///////////////////////////////////////////////////////////////////////////////
//
// NewOPL Translater
//
// Translates NewOPL to byte code.
//
// Processes files: reads file, translates it and writes bytecode file.
//
////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "psion_recreate_all.h"

FIL *icfp  = NULL;
FIL *ofp   = NULL;
FIL *chkfp = NULL;
FIL *trfp  = NULL;
FIL *exfp  = NULL;
FIL *shfp  = NULL;

////////////////////////////////////////////////////////////////////////////////
//
//
//
//
////////////////////////////////////////////////////////////////////////////////

int n_lines_ok    = 0;
int n_lines_bad   = 0;
int n_lines_blank = 0;
int n_stack_errors = 0;

// Reads next composite line into buffer

char current_expression[200] PSRAM;
int first_token = 1;

OP_STACK_ENTRY op_stack[NOPL_MAX_OP_STACK+1] PSRAM;

int op_stack_ptr = 0;


NOBJ_VARTYPE exp_type_stack[MAX_EXP_TYPE_STACK] PSRAM;
int exp_type_stack_ptr = 0;

#define SAVE_I     1
#define NO_SAVE_I  0

////////////////////////////////////////////////////////////////////////////////

// Per-expression
// Indices start at 1, 0 is 'no p'
int node_id_index = 1;

EXP_BUFFER_ENTRY exp_buffer[MAX_EXP_BUFFER] PSRAM;
int exp_buffer_i = 0;

EXP_BUFFER_ENTRY exp_buffer2[MAX_EXP_BUFFER] PSRAM;
int exp_buffer2_i = 0;

////////////////////////////////////////////////////////////////////////////////
//
// Translate a file
//
// Lines are one of the following formats:
//
// All lines are treated as expressions.
//
// E.g.
//
// PRINT A%
//
// PRINT is a function.
//
// A% = 23
//
// '=' is a function (that maps to qcode)
//
// A% = SIN(45)
//
// SIN is a function. As (45) is an expression
//
// SIN 45 is also vaid (but not in original OPL
//
//
// Tokens are strings, delimited by spaces and also:
//
// (),:
//
//
// 1 when line defines the procedure (i.e. the first line)
//
////////////////////////////////////////////////////////////////////////////////

int token_is_operator(char *token, char **tokstr);

void modify_expression_type(NOBJ_VARTYPE t);
void op_stack_display(void);
void op_stack_print(void);
char type_to_char(NOBJ_VARTYPE t);
char access_to_char(NOPL_OP_ACCESS a);
NOBJ_VARTYPE char_to_type(char ch);
void dump_exp_buffer(FIL *fp, int bufnum);
NOBJ_VARTYPE convert_type_to_non_array(NOBJ_VARTYPE t);

////////////////////////////////////////////////////////////////////////////////
//
// Markers used as comments, and hints
//
////////////////////////////////////////////////////////////////////////////////

//#define dbprintf(fmt,...) dbpf(__FUNCTION__, fmt, ...)
//#define dbprintf(fmt,...) printf(fmt, ...)

#define MAX_INDENT 30

int indent_level = 0;
char indent_str[MAX_INDENT+1];

void indent_none(void)
{
  indent_level = 0;
}

void indent_more(void)
{
  if( indent_level <(MAX_INDENT-1) )
    {
      indent_level++;
    }
  ff_fprintf(ofp, "\n");
}

#define MAX_DBPF_LINE 500

void dbpf(const char *caller, char *fmt, ...)
{
  va_list valist;
  char line[MAX_DBPF_LINE];
  
  va_start(valist, fmt);

  vsnprintf(line,  MAX_DBPF_LINE, fmt, valist);
  va_end(valist);
  
  indent_str[0] = '\0';
  
  for(int i=0; i<indent_level; i++)
    {
      strcat(indent_str, " ");
    }
  
  ff_fprintf(ofp, "\n%s(%s) %s", indent_str, caller, line);
  //fflush(ofp);
  
    if( (strstr(line, "ret0") != NULL) || (strstr(line, "ret1") != NULL) )
    {

      if( indent_level != 0 )
	{
	  indent_level--;
	}
    }
}

int find_op_info(char *name, OP_INFO *op)
{
  for(int i=0; i<NUM_OPERATORS; i++)
    {
      if( strcmp(op_info[i].name, name) == 0 )
	{
	  *op = op_info[i];
	  return(1);
	}
    }
  
  return(0);
}

// Is type one of those in the list?
int is_a_valid_type(NOBJ_VARTYPE type, OP_INFO *op_info)
{
  for(int i=0; i<MAX_OPERATOR_TYPES; i++)
    {
      if( op_info->type[i] == type )
	{
	  // It is in the list
	  return(1);
	}
    }

  // Not in list
  return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Expression type is reset for each line and also for sub-lines separated by colons
//
// This is a global to avoid passing it down to every function in the translate call stack.
// If translating is ever to be a parallel process then that will have to change.
//

NOBJ_VARTYPE expression_type = NOBJ_VARTYPE_UNKNOWN;


////////////////////////////////////////////////////////////////////////////////


int defline = 1;

int token_is_float(char *token)
{
  int all_digits = 1;
  int decimal_present = 0;
  int retval;
  int len = strlen(token);
  
  for(int i=0; i < len; i++)
    {
      if( !( isdigit(*token) || (*token == '.') || (*token == '-') || (*token == 'E') ))
	{
	  all_digits = 0;
	}

      if( *token == '.' )
	{
	  decimal_present = 1;
	}
      
      token++;
    }

  retval = all_digits && decimal_present;

  return(retval);
}

int token_is_integer(char *token)
{
  int all_digits = 1;
  int len = strlen(token);

  dbprintf(" tok:'%s'", token);

  if( *token == '-' )
    {
      token++;
      len--;
    }
  
  for(int i=0; i<len; i++)
    {
      if( !isdigit(*(token)) )
	{
	  all_digits = 0;
	}

      token++;
    }

  dbprintf(" tok:ret%d", all_digits);
  return(all_digits);
}

////////////////////////////////////////////////////////////////////////////////

// Variable if it ends in $ or %
// Variable if not a function name
// Variables have to be only alpha or alpha followed by alphanum

int token_is_variable(char *token)
{
  int is_var = 0;
  char *tokptr;

  ff_fprintf(ofp, "\n%s: tok:'%s'", __FUNCTION__, token);
  
  if( token_is_operator(token, &tokptr) )
    {
      return(0);
    }

  if( token_is_function(token, &tokptr) )
    {
      return(0);
    }

  // First char has to be alpha

  if( !isalpha(*token) )
    {
      return(0);
    }

  for(int i=0; i<strlen(token)-1; i++)
    {
      if( ! (isalnum(token[i]) || token[i] == '.') )
	{
	  return(0);
	}
    }

  char last_char = token[strlen(token)-1];

  switch(last_char)
    {
    case '$':
    case '%':
      return(1);
      break;
    }

  return(1);
}

int token_is_string(char *token)
{
  return( *token == '"' );
}


////////////////////////////////////////////////////////////////////////////////
//
// tokstr is a constant string that we use in the operator stack
// to minimise memory usage.
//
////////////////////////////////////////////////////////////////////////////////

int token_is_operator(char *token, char **tokstr)
{
  for(int i=0; i<NUM_OPERATORS; i++)
    {
      if( strcmp(token, op_info[i].name) == 0 )
	{
	  *tokstr = &(op_info[i].name[0]);
	  
	  ff_fprintf(ofp, "\n'%s' is operator", token);
	  return(1);
	}
    }
  
  return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// returns the precedence of the token. This comes from the operator info table for operators,
// functions are set the precedence of 0
//

int operator_precedence(char *token)
{
  for(int i=0; i<NUM_OPERATORS; i++)
    {
      if( strcmp(token, op_info[i].name) == 0 )
	{
	  dbprintf("\n%s is operator", token);
	  return(op_info[i].precedence);
	}
    }
  
  return(100);
}

int operator_left_assoc(char *token)
{
  for(int i=0; i<NUM_OPERATORS; i++)
    {
      if( strcmp(token, op_info[i].name) == 0 )
	{
	  dbprintf("\n%s is operator", token);
	  return(op_info[i].left_assoc);
	}
    }
  
  return(0);
}

//------------------------------------------------------------------------------

// Turn operator into unary version if possible

void operator_can_be_unary(OP_STACK_ENTRY *op)
{
  for(int i=0; i<NUM_OPERATORS; i++)
    {
      if( strcmp(op->name, op_info[i].name) == 0 )
	{
	  // Convert to unary if it can be
	  if( op_info[i].can_be_unary )
	    {
	      op->buf_id = EXP_BUFF_ID_OPERATOR_UNARY;
	      strcpy(op->name, op_info[i].unaryop);
	      dbprintf("Operator converted to unary. Now '%s'", op->name);
	      return;
	    }
	}
    }
  
  return;
}

////////////////////////////////////////////////////////////////////////////////
//
// If the entry is a trapped command then generate the TRAP QCode

int  qcode_check_trapped(int qcode_idx, OP_STACK_ENTRY *op)
{
  dbprintf("%s: Trapped:%d", op->name, op->trapped);
  
  if( op->trapped )
    {
      dbprintf("Added TRAP");
      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_TRAP);
    }

  return(qcode_idx);
}

		
////////////////////////////////////////////////////////////////////////////////
//
// Table with simple BUFF_ID to QCode mappings
//

typedef struct _SIMPLE_QC_MAP
{
  int            buf_id;
  char           *name;     // Name of OP
  NOBJ_VARTYPE   optype;    // Type of OP
  NOPL_VAR_CLASS class;
  NOBJ_VARTYPE   type;      // Type of variable
  NOPL_OP_ACCESS access;
  int            qcode;
  int            logical_file_after_opcode;   // Special qcode argument
} SIMPLE_QC_MAP;

// A value that will never match
#define __  200


const SIMPLE_QC_MAP qc_map[] =
  {
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_EXTERNAL,    NOBJ_VARTYPE_INT,    NOPL_OP_ACCESS_READ, QI_INT_SIM_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_EXTERNAL,    NOBJ_VARTYPE_FLT,    NOPL_OP_ACCESS_READ, QI_NUM_SIM_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_EXTERNAL,    NOBJ_VARTYPE_STR,    NOPL_OP_ACCESS_READ, QI_STR_SIM_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_EXTERNAL,    NOBJ_VARTYPE_INTARY, NOPL_OP_ACCESS_READ, QI_INT_ARR_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_EXTERNAL,    NOBJ_VARTYPE_FLTARY, NOPL_OP_ACCESS_READ, QI_NUM_ARR_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_EXTERNAL,    NOBJ_VARTYPE_STRARY, NOPL_OP_ACCESS_READ, QI_STR_ARR_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_PARAMETER,   NOBJ_VARTYPE_INT,    NOPL_OP_ACCESS_READ, QI_INT_SIM_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_PARAMETER,   NOBJ_VARTYPE_FLT,    NOPL_OP_ACCESS_READ, QI_NUM_SIM_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_PARAMETER,   NOBJ_VARTYPE_STR,    NOPL_OP_ACCESS_READ, QI_STR_SIM_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_PARAMETER,   NOBJ_VARTYPE_INTARY, NOPL_OP_ACCESS_READ, QI_INT_ARR_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_PARAMETER,   NOBJ_VARTYPE_FLTARY, NOPL_OP_ACCESS_READ, QI_NUM_ARR_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_PARAMETER,   NOBJ_VARTYPE_STRARY, NOPL_OP_ACCESS_READ, QI_STR_ARR_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_GLOBAL,      NOBJ_VARTYPE_INT,       NOPL_OP_ACCESS_READ, QI_INT_SIM_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_GLOBAL,      NOBJ_VARTYPE_FLT,       NOPL_OP_ACCESS_READ, QI_NUM_SIM_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_GLOBAL,      NOBJ_VARTYPE_STR,       NOPL_OP_ACCESS_READ, QI_STR_SIM_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_GLOBAL,      NOBJ_VARTYPE_INTARY,    NOPL_OP_ACCESS_READ, QI_INT_ARR_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_GLOBAL,      NOBJ_VARTYPE_FLTARY,    NOPL_OP_ACCESS_READ, QI_NUM_ARR_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_GLOBAL,      NOBJ_VARTYPE_STRARY,    NOPL_OP_ACCESS_READ, QI_STR_ARR_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_LOCAL,       NOBJ_VARTYPE_INT,        NOPL_OP_ACCESS_READ, QI_INT_SIM_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_LOCAL,       NOBJ_VARTYPE_FLT,        NOPL_OP_ACCESS_READ, QI_NUM_SIM_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_LOCAL,       NOBJ_VARTYPE_STR,        NOPL_OP_ACCESS_READ, QI_STR_SIM_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_LOCAL,       NOBJ_VARTYPE_INTARY,     NOPL_OP_ACCESS_READ, QI_INT_ARR_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_LOCAL,       NOBJ_VARTYPE_FLTARY,     NOPL_OP_ACCESS_READ, QI_NUM_ARR_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_LOCAL,       NOBJ_VARTYPE_STRARY,     NOPL_OP_ACCESS_READ, QI_STR_ARR_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_EXTERNAL,    NOBJ_VARTYPE_INT,    NOPL_OP_ACCESS_WRITE, QI_LS_INT_SIM_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_EXTERNAL,    NOBJ_VARTYPE_FLT,    NOPL_OP_ACCESS_WRITE, QI_LS_NUM_SIM_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_EXTERNAL,    NOBJ_VARTYPE_STR,    NOPL_OP_ACCESS_WRITE, QI_LS_STR_SIM_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_EXTERNAL,    NOBJ_VARTYPE_INTARY, NOPL_OP_ACCESS_WRITE, QI_LS_INT_ARR_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_EXTERNAL,    NOBJ_VARTYPE_FLTARY, NOPL_OP_ACCESS_WRITE, QI_LS_NUM_ARR_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_EXTERNAL,    NOBJ_VARTYPE_STRARY, NOPL_OP_ACCESS_WRITE, QI_LS_STR_ARR_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_PARAMETER,   NOBJ_VARTYPE_INT,    NOPL_OP_ACCESS_WRITE, QI_LS_INT_SIM_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_PARAMETER,   NOBJ_VARTYPE_FLT,    NOPL_OP_ACCESS_WRITE, QI_LS_NUM_SIM_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_PARAMETER,   NOBJ_VARTYPE_STR,    NOPL_OP_ACCESS_WRITE, QI_LS_STR_SIM_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_PARAMETER,   NOBJ_VARTYPE_INTARY, NOPL_OP_ACCESS_WRITE, QI_LS_INT_ARR_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_PARAMETER,   NOBJ_VARTYPE_FLTARY, NOPL_OP_ACCESS_WRITE, QI_LS_NUM_ARR_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_PARAMETER,   NOBJ_VARTYPE_STRARY, NOPL_OP_ACCESS_WRITE, QI_LS_STR_ARR_IND, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_GLOBAL,      NOBJ_VARTYPE_INT,       NOPL_OP_ACCESS_WRITE, QI_LS_INT_SIM_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_GLOBAL,      NOBJ_VARTYPE_FLT,       NOPL_OP_ACCESS_WRITE, QI_LS_NUM_SIM_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_GLOBAL,      NOBJ_VARTYPE_STR,       NOPL_OP_ACCESS_WRITE, QI_LS_STR_SIM_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_GLOBAL,      NOBJ_VARTYPE_INTARY,    NOPL_OP_ACCESS_WRITE, QI_LS_INT_ARR_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_GLOBAL,      NOBJ_VARTYPE_FLTARY,    NOPL_OP_ACCESS_WRITE, QI_LS_NUM_ARR_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_GLOBAL,      NOBJ_VARTYPE_STRARY,    NOPL_OP_ACCESS_WRITE, QI_LS_STR_ARR_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_LOCAL,       NOBJ_VARTYPE_INT,        NOPL_OP_ACCESS_WRITE, QI_LS_INT_SIM_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_LOCAL,       NOBJ_VARTYPE_FLT,        NOPL_OP_ACCESS_WRITE, QI_LS_NUM_SIM_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_LOCAL,       NOBJ_VARTYPE_STR,        NOPL_OP_ACCESS_WRITE, QI_LS_STR_SIM_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_LOCAL,       NOBJ_VARTYPE_INTARY,     NOPL_OP_ACCESS_WRITE, QI_LS_INT_ARR_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_LOCAL,       NOBJ_VARTYPE_FLTARY,     NOPL_OP_ACCESS_WRITE, QI_LS_NUM_ARR_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_LOCAL,       NOBJ_VARTYPE_STRARY,     NOPL_OP_ACCESS_WRITE, QI_LS_STR_ARR_FP, 0},
    {EXP_BUFF_ID_VARIABLE, "",    __, NOPL_VAR_CLASS_CALC_MEMORY, NOBJ_VARTYPE_FLT,       NOPL_OP_ACCESS_READ,  QI_NUM_SIM_ABS, 0},
    {EXP_BUFF_ID_VARIABLE, "",                          __, NOPL_VAR_CLASS_CALC_MEMORY, NOBJ_VARTYPE_FLT,       NOPL_OP_ACCESS_WRITE, QI_LS_NUM_SIM_ABS, 0},
    {EXP_BUFF_ID_AUTOCON,  "",    NOBJ_VARTYPE_INTARY,  __,                         __,     __,               QCO_NUM_TO_INT, 0},
    {EXP_BUFF_ID_AUTOCON,  "",    NOBJ_VARTYPE_FLTARY,  __,                      __,     __,               QCO_INT_TO_NUM, 0},
    {EXP_BUFF_ID_AUTOCON,  "",    NOBJ_VARTYPE_INT,     __,                      __,     __,               QCO_NUM_TO_INT, 0},
    {EXP_BUFF_ID_AUTOCON,  "",    NOBJ_VARTYPE_FLT,     __,                      __,     __,               QCO_INT_TO_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, ":=",  NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_ASS_INT, 0},
    {EXP_BUFF_ID_OPERATOR, ":=",  NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_ASS_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, ":=",  NOBJ_VARTYPE_STR,     __,                   __,        __,               QCO_ASS_STR, 0},
    {EXP_BUFF_ID_OPERATOR, ":=",  NOBJ_VARTYPE_INTARY,  __,                   __,        __,               QCO_ASS_INT, 0},
    {EXP_BUFF_ID_OPERATOR, ":=",  NOBJ_VARTYPE_FLTARY,  __,                   __,        __,               QCO_ASS_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, ":=",  NOBJ_VARTYPE_STRARY,  __,                   __,        __,               QCO_ASS_STR, 0},
    {EXP_BUFF_ID_OPERATOR, "=",   NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_EQ_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "=",   NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_EQ_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "=",   NOBJ_VARTYPE_STR,     __,                   __,        __,               QCO_EQ_STR, 0},
    {EXP_BUFF_ID_OPERATOR, "=",   NOBJ_VARTYPE_INTARY,  __,                   __,        __,               QCO_EQ_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "=",   NOBJ_VARTYPE_FLTARY,  __,                   __,        __,               QCO_EQ_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "=",   NOBJ_VARTYPE_STRARY,  __,                   __,        __,               QCO_EQ_STR, 0},
    {EXP_BUFF_ID_FUNCTION, "DROP",NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_DROP_WORD, 0},
    {EXP_BUFF_ID_FUNCTION, "DROP",NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_DROP_NUM, 0},
    {EXP_BUFF_ID_FUNCTION, "DROP",NOBJ_VARTYPE_STR,     __,                   __,        __,               QCO_DROP_STR, 0},
    {EXP_BUFF_ID_INPUT,    "INPUT",NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_INPUT_INT, 0},
    {EXP_BUFF_ID_INPUT,    "INPUT",NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_INPUT_NUM, 0},
    {EXP_BUFF_ID_INPUT,    "INPUT",NOBJ_VARTYPE_STR,     __,                   __,        __,               QCO_INPUT_STR, 0},


    {EXP_BUFF_ID_FUNCTION, "ABS",  NOBJ_VARTYPE_FLT,     __,                   __,        __,               RTF_ABS, 0},
    {EXP_BUFF_ID_FUNCTION, "ACOS",              __,     __,                   __,        __,               RTF_ACOS, 0},
    {EXP_BUFF_ID_FUNCTION, "ASIN",              __,     __,                   __,        __,               RTF_ASIN, 0},
    {EXP_BUFF_ID_FUNCTION, "ATAN",              __,     __,                   __,        __,               RTF_ATAN, 0},
    {EXP_BUFF_ID_FUNCTION, "ASC",               __,     __,                   __,        __,               RTF_ASC, 0},
    {EXP_BUFF_ID_FUNCTION, "AT",                __,     __,                   __,        __,               QCO_AT, 0},
    {EXP_BUFF_ID_FUNCTION, "ADDR", NOBJ_VARTYPE_INT,     __,                   __,        __,               RTF_ADDR, 0},
    {EXP_BUFF_ID_FUNCTION, "ADDR", NOBJ_VARTYPE_STR,     __,                   __,        __,               RTF_SADDR, 0},
    {EXP_BUFF_ID_FUNCTION, "APPEND",            __,     __,                   __,        __,               QCO_APPEND, 0},

    {EXP_BUFF_ID_FUNCTION, "BACK",              __,     __,                   __,        __,               QCO_BACK, 0},
    
    {EXP_BUFF_ID_FUNCTION, "COPY",              __,     __,                   __,        __,               QCO_COPY, 0},
    {EXP_BUFF_ID_FUNCTION, "COPYW",             __,     __,                   __,        __,               RTF_COPYW, 0},
    {EXP_BUFF_ID_FUNCTION, "COUNT",  NOBJ_VARTYPE_INT,  __,                   __,        __,               RTF_COUNT, 0},
    {EXP_BUFF_ID_FUNCTION, "CLS",               __,     __,                   __,        __,               QCO_CLS, 0},
    {EXP_BUFF_ID_FUNCTION, "CLOCK",             __,     __,                   __,        __,               RTF_CLOCK, 0},
    {EXP_BUFF_ID_FUNCTION, "CURSOR",            __,     __,                   __,        __,               QCO_CURSOR, 0},
	
    {EXP_BUFF_ID_FUNCTION, "DAY",    NOBJ_VARTYPE_INT,  __,                   __,        __,               RTF_DAY, 0},
    {EXP_BUFF_ID_FUNCTION, "DISP",              __,     __,                   __,        __,               RTF_DISP, 0},
    {EXP_BUFF_ID_FUNCTION, "DAYS",              __,     __,                   __,        __,               RTF_DAYS, 0},
    {EXP_BUFF_ID_FUNCTION, "DELETEW",           __,     __,                   __,        __,               RTF_DELETEW, 0},
    {EXP_BUFF_ID_FUNCTION, "DELETE",            __,     __,                   __,        __,               QCO_DELETE, 0},
    {EXP_BUFF_ID_FUNCTION, "DATIM$",            __,     __,                   __,        __,               RTF_DATIM, 0},
    {EXP_BUFF_ID_FUNCTION, "DIR$",              __,     __,                   __,        __,               RTF_DIR, 0},
    {EXP_BUFF_ID_FUNCTION, "DIRW$",             __,     __,                   __,        __,               RTF_DIRW, 0},
    
    {EXP_BUFF_ID_FUNCTION, "EDIT",              __,     __,                   __,        __,               QCO_EDIT, 0},
    {EXP_BUFF_ID_FUNCTION, "ERR",               __,     __,                   __,        __,               RTF_ERR, 0},
    {EXP_BUFF_ID_FUNCTION, "ERR$",              __,     __,                   __,        __,               RTF_SERR, 0},
    {EXP_BUFF_ID_FUNCTION, "EOF",               __,     __,                   __,        __,               RTF_EOF, 0},
    {EXP_BUFF_ID_FUNCTION, "EXP",               __,     __,                   __,        __,               RTF_EXP, 0},
    {EXP_BUFF_ID_FUNCTION, "ERASE",             __,     __,                   __,        __,               QCO_ERASE, 0},    

    {EXP_BUFF_ID_FUNCTION, "FREE",              __,     __,                   __,        __,               RTF_FREE, 0},
    {EXP_BUFF_ID_FUNCTION, "FIRST",             __,     __,                   __,        __,               QCO_FIRST, 0},
    {EXP_BUFF_ID_FUNCTION, "FIND",              __,     __,                   __,        __,               RTF_FIND, 0},
    {EXP_BUFF_ID_FUNCTION, "FINDW",             __,     __,                   __,        __,               RTF_FINDW, 0},
    {EXP_BUFF_ID_FUNCTION, "FIX$",              __,     __,                   __,        __,               RTF_FIX, 0},
    {EXP_BUFF_ID_FUNCTION, "FLT",               __,     __,                   __,        __,               RTF_FLT, 0},
    
    {EXP_BUFF_ID_FUNCTION, "GET",               __,     __,                   __,        __,               RTF_GET, 0},
    {EXP_BUFF_ID_FUNCTION, "GET$",              __,     __,                   __,        __,               RTF_SGET, 0},

    {EXP_BUFF_ID_FUNCTION, "HEX$",              __,     __,                   __,        __,               RTF_HEX, 0},
    {EXP_BUFF_ID_FUNCTION, "HOUR",   NOBJ_VARTYPE_INT,  __,                   __,        __,               RTF_HOUR, 0},

    {EXP_BUFF_ID_FUNCTION, "KSTAT",             __,     __,                   __,        __,               QCO_KSTAT, 0},

    {EXP_BUFF_ID_FUNCTION, "LAST",              __,     __,                   __,        __,               QCO_LAST, 0},
    {EXP_BUFF_ID_FUNCTION, "LOC",               __,     __,                   __,        __,               RTF_LOC, 0},
    {EXP_BUFF_ID_FUNCTION, "LN",                __,     __,                   __,        __,               RTF_LN, 0},
    {EXP_BUFF_ID_FUNCTION, "LOG",               __,     __,                   __,        __,               RTF_LOG, 0},
    //{EXP_BUFF_ID_FUNCTION, "LOWER",             __,     __,                   __,        __,               RTF_LOWER, 0},
	    
    {EXP_BUFF_ID_FUNCTION, "MAX",               __,     __,                   __,        __,               RTF_MAX, 0},
    {EXP_BUFF_ID_FUNCTION, "MIN",               __,     __,                   __,        __,               RTF_MIN, 0},
    {EXP_BUFF_ID_FUNCTION, "SUM",               __,     __,                   __,        __,               RTF_SUM, 0},
    {EXP_BUFF_ID_FUNCTION, "MENUN",  NOBJ_VARTYPE_INT,  __,                   __,        __,               RTF_MENUN, 0},
    {EXP_BUFF_ID_FUNCTION, "MENU",   NOBJ_VARTYPE_INT,  __,                   __,        __,               RTF_MENU, 0},
    {EXP_BUFF_ID_FUNCTION, "MINUTE", NOBJ_VARTYPE_INT,  __,                   __,        __,               RTF_MINUTE, 0},
    {EXP_BUFF_ID_FUNCTION, "MONTH",  NOBJ_VARTYPE_INT,  __,                   __,        __,               RTF_MONTH, 0},
    {EXP_BUFF_ID_FUNCTION, "MEAN",   NOBJ_VARTYPE_FLT,  __,                   __,        __,               RTF_MEAN, 0},
    
    {EXP_BUFF_ID_FUNCTION, "OFF",               __,     __,                   __,        __,               QCO_OFF, 0},
    {EXP_BUFF_ID_FUNCTION, "OFFX",              __,     __,                   __,        __,               RTF_OFFX, 0},
	
    {EXP_BUFF_ID_FUNCTION, "PAUSE",             __,     __,                   __,        __,               QCO_PAUSE, 0},
    {EXP_BUFF_ID_FUNCTION, "POKEW",             __,     __,                   __,        __,               QCO_POKEW, 0},
    {EXP_BUFF_ID_FUNCTION, "POKEB",             __,     __,                   __,        __,               QCO_POKEB, 0},
    {EXP_BUFF_ID_FUNCTION, "PEEKW",             __,     __,                   __,        __,               RTF_PEEKW, 0},
    {EXP_BUFF_ID_FUNCTION, "PEEKB",             __,     __,                   __,        __,               RTF_PEEKB, 0},

    {EXP_BUFF_ID_FUNCTION, "RENAME",            __,     __,                   __,        __,               QCO_RENAME, 0},
    {EXP_BUFF_ID_FUNCTION, "RECSIZE",           __,     __,                   __,        __,               RTF_RECSIZE, 0},
    {EXP_BUFF_ID_FUNCTION, "RAISE",             __ ,    __,                   __,        __,               QCO_RAISE, 0},

    {EXP_BUFF_ID_FUNCTION, "STD",               __,     __,                   __,        __,               RTF_STD, 0},
    {EXP_BUFF_ID_FUNCTION, "SCI$",              __,     __,                   __,        __,               RTF_SCI, 0},
    {EXP_BUFF_ID_FUNCTION, "STOP",              __,     __,                   __,        __,               QCO_STOP, 0},
    {EXP_BUFF_ID_FUNCTION, "SECOND", NOBJ_VARTYPE_INT,  __,                   __,        __,               RTF_SECOND, 0},
    {EXP_BUFF_ID_FUNCTION, "SPACE",             __,     __,                   __,        __,               RTF_SPACE, 0},

    {EXP_BUFF_ID_FUNCTION, "YEAR",   NOBJ_VARTYPE_INT,  __,                   __,        __,               RTF_YEAR, 0},

    {EXP_BUFF_ID_FUNCTION, "NEXT",              __,     __,                   __,        __,               QCO_NEXT, 0},
    {EXP_BUFF_ID_FUNCTION, "POSITION",          __,     __,                   __,        __,               QCO_POSITION, 0},
    {EXP_BUFF_ID_FUNCTION, "ESCAPE",            __,     __,                   __,        __,               QCO_ESCAPE, 0},
    {EXP_BUFF_ID_FUNCTION, "KEY",               __,     __,                   __,        __,               RTF_KEY, 0},

    {EXP_BUFF_ID_FUNCTION, "KEY$",              __,     __,                   __,        __,               RTF_SKEY, 0},
    {EXP_BUFF_ID_FUNCTION, "CHR$",              __,     __,                   __,        __,               RTF_CHR, 0},
    {EXP_BUFF_ID_FUNCTION, "BEEP",              __,     __,                   __,        __,               QCO_BEEP, 0},
    {EXP_BUFF_ID_FUNCTION, "LEFT$",             __,     __,                   __,        __,               RTF_LEFT, 0},
    {EXP_BUFF_ID_FUNCTION, "RIGHT$",            __,     __,                   __,        __,               RTF_RIGHT, 0},
    {EXP_BUFF_ID_FUNCTION, "GEN$",              __,     __,                   __,        __,               RTF_GEN, 0},
    {EXP_BUFF_ID_FUNCTION, "REPT$",             __,     __,                   __,        __,               RTF_REPT, 0},
    {EXP_BUFF_ID_FUNCTION, "MID$",              __,     __,                   __,        __,               RTF_MID, 0},
    {EXP_BUFF_ID_FUNCTION, "UPPER$",            __,     __,                   __,        __,               RTF_UPPER, 0},
    {EXP_BUFF_ID_FUNCTION, "LOWER$",            __,     __,                   __,        __,               RTF_LOWER, 0},
    {EXP_BUFF_ID_FUNCTION, "LEN",               __,     __,                   __,        __,               RTF_LEN, 0},
    {EXP_BUFF_ID_FUNCTION, "POS",               __,     __,                   __,        __,               RTF_POS, 0},
    {EXP_BUFF_ID_FUNCTION, "NUM$",              __,     __,                   __,        __,               RTF_NUM, 0},
    {EXP_BUFF_ID_FUNCTION, "DOW",               __,     __,                   __,        __,               RTF_DOW, 0},
    {EXP_BUFF_ID_FUNCTION, "DAYNAME$",          __,     __,                   __,        __,               RTF_DAYNAME, 0},
    {EXP_BUFF_ID_FUNCTION, "MONTH$",            __,     __,                   __,        __,               RTF_MONTHSTR, 0},
    {EXP_BUFF_ID_FUNCTION, "VIEW",              __,     __,                   __,        __,               RTF_VIEW, 0},
    {EXP_BUFF_ID_FUNCTION, "RND",               __,     __,                   __,        __,               RTF_RND, 0},
    {EXP_BUFF_ID_FUNCTION, "UDG",               __,     __,                   __,        __,               RTF_UDG, 0},
    {EXP_BUFF_ID_FUNCTION, "EXIST",             __,     __,                   __,        __,               RTF_EXIST, 0},
    {EXP_BUFF_ID_FUNCTION, "CLOSE",             __,     __,                   __,        __,               QCO_CLOSE, 0},
    {EXP_BUFF_ID_FUNCTION, "RANDOMIZE",         __,     __,                   __,        __,               QCO_RANDOMIZE, 0},
    {EXP_BUFF_ID_FUNCTION, "RAD",               __,     __,                   __,        __,               RTF_RAD, 0},
    {EXP_BUFF_ID_FUNCTION, "SIN",               __,     __,                   __,        __,               RTF_SIN, 0},
    {EXP_BUFF_ID_FUNCTION, "COS",               __,     __,                   __,        __,               RTF_COS, 0},
    {EXP_BUFF_ID_FUNCTION, "TAN",               __,     __,                   __,        __,               RTF_TAN, 0},
    {EXP_BUFF_ID_FUNCTION, "SQR",               __,     __,                   __,        __,               RTF_SQR, 0},
    {EXP_BUFF_ID_FUNCTION, "DEG",               __,     __,                   __,        __,               RTF_DEG, 0},

    {EXP_BUFF_ID_FUNCTION, "PI",                __,     __,                   __,        __,               RTF_PI, 0},

    {EXP_BUFF_ID_FUNCTION, "USE",               __,     __,                   __,        __,               QCO_USE, 1},
    {EXP_BUFF_ID_FUNCTION, "USR",               __,     __,                   __,        __,               RTF_IUSR, 0},
    {EXP_BUFF_ID_FUNCTION, "USR$",              __,     __,                   __,        __,               RTF_SUSR, 0},
    {EXP_BUFF_ID_FUNCTION, "UPDATE",            __,     __,                   __,        __,               QCO_UPDATE, 0},
    
    {EXP_BUFF_ID_FUNCTION, "VAR",               __,     __,                   __,        __,               RTF_VAR, 0},
    {EXP_BUFF_ID_FUNCTION, "VAL",               __,     __,                   __,        __,               RTF_VAL, 0},

    {EXP_BUFF_ID_FUNCTION, "WEEK",              __,     __,                   __,        __,               RTF_WEEK, 0},

    {EXP_BUFF_ID_TRAP,     "TRAP",              __,     __,                   __,        __,               QCO_TRAP, 0},
    {EXP_BUFF_ID_FUNCTION, "IABS", NOBJ_VARTYPE_INT,     __,                   __,        __,               RTF_IABS, 0},

    {EXP_BUFF_ID_FUNCTION, "INT",  NOBJ_VARTYPE_INT,     __,                   __,        __,               RTF_INT, 0},
    {EXP_BUFF_ID_FUNCTION, "INTF", NOBJ_VARTYPE_FLT,     __,                   __,        __,               RTF_INTF, 0},
    {EXP_BUFF_ID_FUNCTION, "GCLS",               __,     __,                   __,        __,               QCX_GCLS, 0},
    {EXP_BUFF_ID_FUNCTION, "GLINE",              __,     __,                   __,        __,               QCX_GLINE, 0},
    {EXP_BUFF_ID_FUNCTION, "GPOINT",             __,     __,                   __,        __,               QCX_GPOINT, 0},
    {EXP_BUFF_ID_FUNCTION, "GUPDATE",            __,     __,                   __,        __,               QCX_GUPDATE, 0},


    
    {EXP_BUFF_ID_OPERATOR, "<",   NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_LT_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "<",   NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_LT_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "<",   NOBJ_VARTYPE_STR,     __,                   __,        __,               QCO_LT_STR, 0},
    {EXP_BUFF_ID_OPERATOR, "<",   NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_LT_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "<",   NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_LT_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "<",   NOBJ_VARTYPE_STRARY,     __,                   __,        __,               QCO_LT_STR, 0},
    {EXP_BUFF_ID_OPERATOR, "<=",  NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_LTE_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "<=",  NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_LTE_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "<=",  NOBJ_VARTYPE_STR,     __,                   __,        __,               QCO_LTE_STR, 0},
    {EXP_BUFF_ID_OPERATOR, "<=",  NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_LTE_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "<=",  NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_LTE_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "<=",  NOBJ_VARTYPE_STRARY,     __,                   __,        __,               QCO_LTE_STR, 0},
    {EXP_BUFF_ID_OPERATOR, ">",   NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_GT_INT, 0},
    {EXP_BUFF_ID_OPERATOR, ">",   NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_GT_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, ">",   NOBJ_VARTYPE_STR,     __,                   __,        __,               QCO_GT_STR, 0},
    {EXP_BUFF_ID_OPERATOR, ">",   NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_GT_INT, 0},
    {EXP_BUFF_ID_OPERATOR, ">",   NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_GT_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, ">",   NOBJ_VARTYPE_STRARY,     __,                   __,        __,               QCO_GT_STR, 0},
    {EXP_BUFF_ID_OPERATOR, ">=",  NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_GTE_INT, 0},
    {EXP_BUFF_ID_OPERATOR, ">=",  NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_GTE_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, ">=",  NOBJ_VARTYPE_STR,     __,                   __,        __,               QCO_GTE_STR, 0},
    {EXP_BUFF_ID_OPERATOR, ">=",  NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_GTE_INT, 0},
    {EXP_BUFF_ID_OPERATOR, ">=",  NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_GTE_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, ">=",  NOBJ_VARTYPE_STRARY,     __,                   __,        __,               QCO_GTE_STR, 0},
    {EXP_BUFF_ID_OPERATOR, "<>",  NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_NE_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "<>",  NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_NE_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "<>",  NOBJ_VARTYPE_STR,     __,                   __,        __,               QCO_NE_STR, 0},
    {EXP_BUFF_ID_OPERATOR, "<>",  NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_NE_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "<>",  NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_NE_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "<>",  NOBJ_VARTYPE_STRARY,     __,                   __,        __,               QCO_NE_STR, 0},
    {EXP_BUFF_ID_OPERATOR, "+",   NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_ADD_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "+",   NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_ADD_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "+",   NOBJ_VARTYPE_STR,     __,                   __,        __,               QCO_ADD_STR, 0},
    {EXP_BUFF_ID_OPERATOR, "+",   NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_ADD_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "+",   NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_ADD_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "+",   NOBJ_VARTYPE_STRARY,     __,                   __,        __,               QCO_ADD_STR, 0},
    {EXP_BUFF_ID_OPERATOR, "-",   NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_SUB_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "-",   NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_SUB_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "-",   NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_SUB_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "-",   NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_SUB_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "*",   NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_MUL_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "*",   NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_MUL_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "*",   NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_MUL_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "*",   NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_MUL_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "/",   NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_DIV_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "/",   NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_DIV_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "/",   NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_DIV_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "/",   NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_DIV_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "**",  NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_POW_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "**",  NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_POW_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "**",  NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_POW_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "**",  NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_POW_NUM, 0},

    {EXP_BUFF_ID_OPERATOR_UNARY, "UMIN",NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_UMIN_INT, 0},
    {EXP_BUFF_ID_OPERATOR_UNARY, "UMIN",NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_UMIN_NUM, 0},
    {EXP_BUFF_ID_OPERATOR_UNARY, "UMIN",NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_UMIN_INT, 0},
    {EXP_BUFF_ID_OPERATOR_UNARY, "UMIN",NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_UMIN_NUM, 0},

    {EXP_BUFF_ID_OPERATOR_UNARY, "UNOT", NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_NOT_INT, 0},
    {EXP_BUFF_ID_OPERATOR_UNARY, "UNOT", NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_NOT_NUM, 0},
    {EXP_BUFF_ID_OPERATOR_UNARY, "UNOT", NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_NOT_INT, 0},
    {EXP_BUFF_ID_OPERATOR_UNARY, "UNOT", NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_NOT_NUM, 0},

    {EXP_BUFF_ID_OPERATOR, "AND", NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_AND_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "AND", NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_AND_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "OR",  NOBJ_VARTYPE_INT,     __,                   __,        __,               QCO_OR_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "OR",  NOBJ_VARTYPE_FLT,     __,                   __,        __,               QCO_OR_NUM, 0},

    {EXP_BUFF_ID_OPERATOR, "AND", NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_AND_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "AND", NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_AND_NUM, 0},
    {EXP_BUFF_ID_OPERATOR, "OR",  NOBJ_VARTYPE_INTARY,     __,                   __,        __,               QCO_OR_INT, 0},
    {EXP_BUFF_ID_OPERATOR, "OR",  NOBJ_VARTYPE_FLTARY,     __,                   __,        __,               QCO_OR_NUM, 0},

    {EXP_BUFF_ID_OPERATOR, "+%",  NOBJ_VARTYPE_FLT,        __,                   __,        __,               RTF_PLUSPERCENT, 0},
    {EXP_BUFF_ID_OPERATOR, "-%",  NOBJ_VARTYPE_FLT,        __,                   __,        __,               RTF_MINUSPERCENT, 0},
    {EXP_BUFF_ID_OPERATOR, "*%",  NOBJ_VARTYPE_FLT,        __,                   __,        __,               RTF_TIMESPERCENT, 0},
    {EXP_BUFF_ID_OPERATOR, "/%",  NOBJ_VARTYPE_FLT,        __,                   __,        __,               RTF_DIVIDEPERCENT, 0},
    {EXP_BUFF_ID_OPERATOR, ">%",  NOBJ_VARTYPE_FLT,        __,                   __,        __,               RTF_GTPERCENT, 0},
    {EXP_BUFF_ID_OPERATOR, "<%",  NOBJ_VARTYPE_FLT,        __,                   __,        __,               RTF_LTPERCENT, 0},
  };

#define NUM_SIMPLE_QC_MAP (sizeof(qc_map)/sizeof(SIMPLE_QC_MAP))

int add_simple_qcode(int idx, OP_STACK_ENTRY *op, NOBJ_VAR_INFO *vi)
{
  int op_type = NOBJ_VARTYPE_UNKNOWN;
  NOBJ_VAR_INFO nullvi;

  dbprintf("'%s'", op->name);
  dbprintf("Op type:%c op access:%s qcode_type:%c", type_to_char(op->type), var_access_to_str(op->access), type_to_char(op->qcode_type));
  
  nullvi.class = NOPL_VAR_CLASS_UNKNOWN;
  nullvi.type = NOBJ_VARTYPE_UNKNOWN;
  
  if( vi == NULL )
    {
      dbprintf("NULL vi");
      vi = &nullvi;
    }

  // Some operators should have their types forced to the qcode_type as that is based on their inputs.
  // the '.type' field is based on the output and is used for typechecking.
  if( op->qcode_type != NOBJ_VARTYPE_UNKNOWN )
    {
      dbprintf("Type forced to qcode_type");
      op_type = op->qcode_type;
    }
  else
    {
      op_type = op->type;
    }
  
  for(int i=0; i<NUM_SIMPLE_QC_MAP; i++)
    {

      if( op->buf_id == qc_map[i].buf_id )
	{
	  
	  // See if other values match
	  if( ((qc_map[i].class  == vi->class)         || (qc_map[i].class == __))   &&
	      ((qc_map[i].type   == vi->type)          || (qc_map[i].type == __)) &&
	      ((qc_map[i].optype == op_type)           || (qc_map[i].optype == __)) &&
	      ((qc_map[i].access == op->access)        || (qc_map[i].access == __)) &&
	      (((strcmp(qc_map[i].name, op->name) == 0) && (strlen(qc_map[i].name) != 0)) || (strlen(qc_map[i].name) == 0))
	      )
	    {

	      // Add TRAP if needed
	      idx = qcode_check_trapped(idx, op);
	      
	      // We have a match and a qcode
	      idx = set_qcode_header_byte_at(idx, 1, qc_map[i].qcode);

	      return(idx);
	    }
	}
    }

  dbprintf("Not found");
  return(idx);
}

////////////////////////////////////////////////////////////////////////////////
//
// Conditional branch offset fixups

typedef struct _COND_FIXUP_ENTRY
{
  int  offset_idx;   // Where the offset has to be written
  int  target_idx;   // The index that the offset is calculated from (jump target)
  int  buf_id;       // The type of point in the qcode that this is
  int  level;
  char label[NOBJ_VARNAME_MAXLEN+1];    // For label and goto

} COND_FIXUP_ENTRY;


COND_FIXUP_ENTRY cond_fixup[MAX_COND_FIXUP] PSRAM;
int cond_fixup_i = 0;

void add_cond_fixup(int offset_idx, int target_idx, int buf_id, int level)
{
  if( cond_fixup_i < MAX_COND_FIXUP-1 )
    {
      cond_fixup[cond_fixup_i].offset_idx    = offset_idx;
      cond_fixup[cond_fixup_i].target_idx    = target_idx;
      cond_fixup[cond_fixup_i].buf_id        = buf_id;
      cond_fixup[cond_fixup_i].level         = level;
      strcpy(cond_fixup[cond_fixup_i].label, "");
      cond_fixup_i++;
    }
  else
    {
      internal_error("Too many conditionals");
    }
}

//------------------------------------------------------------------------------

// Add a fixup entry with a label
void add_cond_fixup_label(int offset_idx, int target_idx, int buf_id, char *label)
{
  if( cond_fixup_i < MAX_COND_FIXUP-1 )
    {
      cond_fixup[cond_fixup_i].offset_idx      = offset_idx;
      cond_fixup[cond_fixup_i].target_idx      = target_idx;
      cond_fixup[cond_fixup_i].buf_id          = buf_id;
      strcpy(cond_fixup[cond_fixup_i].label, label);
      to_upper_str(cond_fixup[cond_fixup_i].label);
      cond_fixup[cond_fixup_i].level           = 0;
      cond_fixup_i++;
    }
  else
    {
      internal_error("Too many conditionals");
    }
}

//------------------------------------------------------------------------------
//
// Find an offset index given a level and buf_id

int find_offset_idx(int buf_id, int level)
{
  for(int i=0; i<cond_fixup_i; i++)
    {
      if( (buf_id == cond_fixup[i].buf_id) && (level == cond_fixup[i].level) )
	{
	  return(cond_fixup[i].offset_idx);
	}
    }
  return(-1);
}

//------------------------------------------------------------------------------
// Find a target index given a level and buf_id

int find_target_idx(int buf_id, int level)
{
  for(int i=0; i<cond_fixup_i; i++)
    {
      if( (buf_id == cond_fixup[i].buf_id) && (level == cond_fixup[i].level) )
	{
	  return(cond_fixup[i].target_idx);
	}
    }
  return(-1);
}

//------------------------------------------------------------------------------
// Find a target index given a label

int find_target_idx_from_label(char *label)
{
  to_upper_str(label);
  
  for(int i=0; i<cond_fixup_i; i++)
    {
      if( (strcmp(cond_fixup[i].label, label)==0) && (cond_fixup[i].buf_id == EXP_BUFF_ID_LABEL) )
	{
	  dbprintf("i:%d target:%04X", i, cond_fixup[i].target_idx);
	  return(cond_fixup[i].target_idx);
	}
    }
  return(-1);
}

//------------------------------------------------------------------------------
//
// Find a forward target index given a level and buf_id and a starting index

int find_forward_target_idx(int start, int buf_id, int level)
{
  for(int i=start+1; i<cond_fixup_i; i++)
    {
      if( (buf_id == cond_fixup[i].buf_id) && (level == cond_fixup[i].level) )
	{
	  return(cond_fixup[i].target_idx);
	}
    }
  return(-1);
}

//------------------------------------------------------------------------------

void dump_cond_fixup(void)
{
  FIL *cffp = fopen("cond_fixup.txt", "w");

  if( cffp == NULL )
    {
      return;
    }
  
  for(int i=0; i<cond_fixup_i; i++)
    {
      ff_fprintf(cffp, "\n%04d: %-25s Level:%3d Offset:%04X Target:%04X Label:'%s'",
	      i,
	      exp_buffer_id_str[cond_fixup[i].buf_id],
	      cond_fixup[i].level,
	      cond_fixup[i].offset_idx - qcode_start_idx,
	      cond_fixup[i].target_idx - qcode_start_idx,
      cond_fixup[i].label);
    }
  
  ff_fprintf(cffp, "\n");
  fclose(cffp);
}

//------------------------------------------------------------------------------
//
// Fix up conditional branches
//

void do_cond_fixup(void)
{
  int target_idx;
  int offset_idx;
  int branch_offset;
  int until_offset;

  dbprintf("Conditional fixup");
  
  for(int i=0; i<cond_fixup_i; i++)
    {
      dbprintf("Fixing %d %s" , i, exp_buffer_id_str[cond_fixup[i].buf_id]);
      
      switch(cond_fixup[i].buf_id)
	{
	case EXP_BUFF_ID_LABEL:
	  // Do nothing
	  break;

	case EXP_BUFF_ID_GOTO:
	case EXP_BUFF_ID_ONERR:
	  // Find the label, get the offset and fill it in
	  target_idx = find_target_idx_from_label(cond_fixup[i].label);

	  // Calculate offset
	  branch_offset = (target_idx - cond_fixup[i].offset_idx);
	  until_offset = branch_offset;
	  dbprintf("GOTO %s %04X %04X", cond_fixup[i].label, target_idx, branch_offset);
	  
	  // Fill in the offset
	  set_qcode_header_byte_at(cond_fixup[i].offset_idx+0, 1, (until_offset) >> 8);
	  set_qcode_header_byte_at(cond_fixup[i].offset_idx+1, 1, (until_offset) & 0xFF);
	  break;

	case EXP_BUFF_ID_BREAK:
	  // We need to either find a matching DO and jump after the UNTIL or
	  // find a matching WHILE and jump after the ENDWH

	  // Is there a DO?
	  target_idx = find_target_idx(EXP_BUFF_ID_UNTIL, cond_fixup[i].level);

	  if( target_idx == -1 )
	    {
	      // No DO, is there a WHILE?
	      target_idx = find_target_idx(EXP_BUFF_ID_ENDWH, cond_fixup[i].level);
	      
	      if( target_idx == -1 )
		{
		  // No WHILE, error
		  
		  syntax_error("No matching DO or WHILE for BREAK");
		  return;
		}
	      else
		{
		  // There is a WHILE
		  // Calculate offset
		  branch_offset = (target_idx - cond_fixup[i].offset_idx);
		  until_offset = branch_offset;
		  
		  // Fill in the offset
		  set_qcode_header_byte_at(cond_fixup[i].offset_idx+0, 1, (until_offset) >> 8);
		  set_qcode_header_byte_at(cond_fixup[i].offset_idx+1, 1, (until_offset) & 0xFF);
		}
	      
	      //	      return;
	    }
	  else
	    {
	      // There is a DO
	      // Calculate offset
	      // Calculate offset (we add 2 as we want to branch after the UNTIL goto and the target is the
	      // fixed up branch offset
	      
	      branch_offset = (target_idx - cond_fixup[i].offset_idx) + 2;
	      until_offset = branch_offset;
	      
	      // Fill in the offset
	      set_qcode_header_byte_at(cond_fixup[i].offset_idx+0, 1, (until_offset) >> 8);
	      set_qcode_header_byte_at(cond_fixup[i].offset_idx+1, 1, (until_offset) & 0xFF);
	    }
	  break;

	case EXP_BUFF_ID_CONTINUE:
	  // We need to either find a matching DO and jump to the UNTIL test or
	  // find a matching WHILE and jump to the WHILE test

	  // Is there a TEST_EXPR for this level?
	  target_idx = find_target_idx(EXP_BUFF_ID_TEST_EXPR, cond_fixup[i].level);

	  if( target_idx == -1 )
	    {
	      syntax_error("No matching DO or WHILE for CONTINUE");
	      return;
	    }

	  // There is a TEST_EXPR
	  // Calculate offset
	  branch_offset = (target_idx - cond_fixup[i].offset_idx);
	  until_offset = branch_offset;
	  
	  // Fill in the offset
	  set_qcode_header_byte_at(cond_fixup[i].offset_idx+0, 1, (until_offset) >> 8);
	  set_qcode_header_byte_at(cond_fixup[i].offset_idx+1, 1, (until_offset) & 0xFF);
	  break;
	  
	case EXP_BUFF_ID_UNTIL:
	  // Find matching DO and get idx
	  
	  target_idx = find_target_idx(EXP_BUFF_ID_DO, cond_fixup[i].level);

	  if( target_idx == -1 )
	    {
	      syntax_error("Bad UNTIL");
	      return;
	    }
	  
	  // Calculate offset
	  branch_offset = (target_idx - cond_fixup[i].offset_idx);
	  until_offset = branch_offset;
	  
	  // Fill in the offset
	  set_qcode_header_byte_at(cond_fixup[i].offset_idx+0, 1, (until_offset) >> 8);
	  set_qcode_header_byte_at(cond_fixup[i].offset_idx+1, 1, (until_offset) & 0xFF);
	  break;

	case EXP_BUFF_ID_IF:
	case EXP_BUFF_ID_ELSEIF:
	  // Find earliest token after the IF that is an endif, else or elseif and branch there
	  
	  if( (target_idx = find_forward_target_idx(i, EXP_BUFF_ID_BRAENDIF, cond_fixup[i].level)) != -1 )
	    {
	    }
	  else
	    {
	      if( (target_idx = find_forward_target_idx(i, EXP_BUFF_ID_ELSE, cond_fixup[i].level)) != -1 )
		{
		}
	      else
		{
		  if( (target_idx = find_forward_target_idx(i, EXP_BUFF_ID_ENDIF, cond_fixup[i].level)) != -1 )
		    {
		    }
		  else
		    {
		      syntax_error("Bad IF");
		      return;
		    }
		}
	    }

	  // Calculate offset
	  branch_offset = (target_idx - cond_fixup[i].offset_idx);
	  until_offset = branch_offset;
	  
	  // Fill in the offset
	  set_qcode_header_byte_at(cond_fixup[i].offset_idx+0, 1, (until_offset) >> 8);
	  set_qcode_header_byte_at(cond_fixup[i].offset_idx+1, 1, (until_offset) & 0xFF);
	  break;

	case EXP_BUFF_ID_BRAENDIF:
	case EXP_BUFF_ID_ELSE:
	  // Find the ENDIF	  
	  if( (target_idx = find_target_idx(EXP_BUFF_ID_ENDIF, cond_fixup[i].level)) != -1 )
	    {
	      // Calculate offset
	      branch_offset = (target_idx - cond_fixup[i].offset_idx);
	      until_offset = branch_offset;
	      
	      // Fill in the offset
	      set_qcode_header_byte_at(cond_fixup[i].offset_idx+0, 1, (until_offset) >> 8);
	      set_qcode_header_byte_at(cond_fixup[i].offset_idx+1, 1, (until_offset) & 0xFF);
	    }
	  else
	    {
	      syntax_error("No ENDIF for ELSE");
	      return;
	    }
	  break;

	case EXP_BUFF_ID_WHILE:
	  // Find matching ENDWH and get idx
	  
	  target_idx = find_target_idx(EXP_BUFF_ID_ENDWH, cond_fixup[i].level);

	  if( target_idx == -1 )
	    {
	      syntax_error("Bad WHILE");
	      return;
	    }
	  
	  // Calculate offset
	  branch_offset = (target_idx - cond_fixup[i].offset_idx);
	  until_offset = branch_offset;
	  
	  // Fill in the offset
	  set_qcode_header_byte_at(cond_fixup[i].offset_idx+0, 1, (until_offset) >> 8);
	  set_qcode_header_byte_at(cond_fixup[i].offset_idx+1, 1, (until_offset) & 0xFF);
	  break;

	case EXP_BUFF_ID_ENDWH:
	  // Find the WHILE	  
	  if( (target_idx = find_target_idx(EXP_BUFF_ID_WHILE, cond_fixup[i].level)) != -1 )
	    {
	      // Calculate offset
	      branch_offset = (target_idx - cond_fixup[i].offset_idx);
	      until_offset = branch_offset;
	      
	      // Fill in the offset
	      set_qcode_header_byte_at(cond_fixup[i].offset_idx+0, 1, (until_offset) >> 8);
	      set_qcode_header_byte_at(cond_fixup[i].offset_idx+1, 1, (until_offset) & 0xFF);
	    }
	  else
	    {
	      syntax_error("No WHILE for ENDWH");
	      return;
	    }
	  break;

	  
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
//

int qcode_add_length_prefixed_string(int qcode_idx, char *str)
{
  dbprintf("str='%s'", str);
  
  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  strlen(str));
  
  for(int s=0; s<strlen(str); s++)
    {
      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  toupper(str[s]));
    }

  return(qcode_idx);
}

////////////////////////////////////////////////////////////////////////////////
//
// Convert float (in string form) to comapct qcode format and add it to
// the qcode stream.
//
//  1.23456
// -1.23456
//  1.23456E12
// -1.23456E12
//  1.23456E-12
// -1.23456E-12

#define NORM_MANT_LEN 12

int convert_to_compact_float(int qcode_idx, char *fltstr)
{
  int len       = 1;   // Exponent always present
  int exponent  = 0;
  int sign      = 0;
  char normalised_mantissa[NORM_MANT_LEN+1+1+1];

  dbprintf("INPUT:Idx:%d fltstr:'%s'", qcode_idx, fltstr);

  // First extract any exponent
  for(int i=0; i<strlen(fltstr); i++)
    {
      if( fltstr[i] == 'E' )
	{
	  // found exponent
	  sscanf(&(fltstr[i]), "E%d", &exponent);
	  dbprintf("Exponent:E%d found", exponent);

	  // End the string at the 'E'
	  fltstr[i] = '\0';
	}
      else
	{
	  dbprintf("No 'E' found");
	}
    }
  
  dbprintf("Exponent:%d", exponent);
  dbprintf("Idx:%d fltstr:'%s'", qcode_idx, fltstr);

  // Remove sign
  int start;
  if( fltstr[0] == '-' )
    {
      sign = 1;
      start = 1;
    }
  else
    {
      sign = 0;
      start = 0;
    }

  dbprintf("Sign:%d start:%d", sign, start);
  
  // Remove leading zeroes
  while( fltstr[start] == '0' )
    {
      start++;
    }
  

  // Check for various forms of zero
  // If we only have dot and zero digits up to end or E then its zero
  int is_zero = 1;
  int done = 0;
  
  for(int i=0; (i<strlen(&(fltstr[start]))) && (!done); i++)
    {
      switch(fltstr[start+i])
	{
	case '0':
	case '.':
	  // All ok
	  break;

	case 'E':
	  // All done
	  done = 1;
	  break;
	  
	default:
	  // Not zero
	  is_zero = 0;
	  break;
	}
    }

  if( is_zero )
    {
      dbprintf("Detected zero");
      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x02);
      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
      return(qcode_idx);
    }
  
  dbprintf("Normalising:'%s'", &(fltstr[start]));
  normalised_mantissa[0] = '\0';
  
  // Normalise the mantissa, adjusting the exponent as we normalise
  if( fltstr[start] == '.' )
    {
      dbprintf("Need to make larger");
      
      // Need to make larger to normalise, find first significant digit
      for(int i=start; i<strlen(fltstr); i++)
	{
	  if( fltstr[i] == '.' )
	    {
	      continue;
	    }
	  
	  if( fltstr[i] == '0' )
	    {
	      // Not found sig dig
	      exponent--;
	    }
	  else
	    {
	      int j;
	      
	      // Found sig dig, copy over to normlised string
	      for(j=0; i<strlen(fltstr) && (j < NORM_MANT_LEN); i++,j++)
		{
		  normalised_mantissa[j] = fltstr[i];
		}
	      normalised_mantissa[j] = '\0';
	    }
	  
	  dbprintf("normalised='%s'", normalised_mantissa);
	}

      // Correct exponent
      exponent--;
      dbprintf("Exponent:%d", exponent);
      
    }
  else
    {
      int j = 0;
      int mod_exp = 1;

      dbprintf("Need to make smaller");
      
      // Need to make smaller to normalise
      for(int i=start; i<strlen(fltstr); i++)
	{
	  if( fltstr[i] == '.' )
	    {
	      // We are done modifying the exponent
	      mod_exp = 0;
	      continue;
	    }
	  
	  normalised_mantissa[j++] = fltstr[i];
	  
	  if( mod_exp )
	    {
	      exponent++;
	    }
	  
	}
      
      normalised_mantissa[j] = '\0';

      dbprintf("normalised='%s'", normalised_mantissa);
      
      // Correct exponent
      exponent--;
    }

  // Drop trailing zeros
  dbprintf("Drop trailing zeros");
  
  for(int t=strlen(normalised_mantissa)-1; t>=0; t--)
    {
      if( normalised_mantissa[t] == '0' )
	{
	  // Remove trailing zero
	  normalised_mantissa[t] = '\0';
	}
      else
	{
	  // Not a zero, all done
	  break;
	}
    }

  dbprintf("normalised='%s'", normalised_mantissa);
  
  // If mantissa has an odd number of digits, add a zero back in
  if( (strlen(normalised_mantissa) % 2) != 0 )
    {
      // This zero allows the byte packing to work properly
      strcat(normalised_mantissa, "0");
    }

  dbprintf("After adding zero back for odd number of digits: normalised='%s'", normalised_mantissa);
	
  // Now build the qcode compact form
  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, strlen(normalised_mantissa)/2+1);
  
  for(int k=strlen(normalised_mantissa)-1; k>=0; k-=2)
    {
      int byte = 0;

      byte = normalised_mantissa[k-1]-'0';
      byte <<=4;
      byte += normalised_mantissa[k]-'0';

      dbprintf("%02X", byte);
      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, byte);

    }

  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, (int8_t)exponent);
  
  dbprintf("RESULT: Input:'%s' Norm mant:'%s' Sign:%d Exponent:%d", fltstr, normalised_mantissa, sign, exponent);
  
  return(qcode_idx);
}


////////////////////////////////////////////////////////////////////////////////
//
// QCode output stream
//
//
//
// The text based tokens from the output stream are converted here to the final
// qcodes
//
// Most codes are translated directly
// Variable offsets have to be inserted instea dof variable names.
// Tables of variables are maintained
//
// Fixed size tables for variables
//
// This is called after each line is processed, That generates enough intermediate
// code to generate QCode for one line.
//
// NOTE:
// As we want to reduce the amount of RAM needed to generate Qcode, we want to generate
// code for a line at a time so we don't have to hold all the intermediate code for
// the entire PROC in memory at once. One problem is that we need to genrate the output
// (OB3) header before the QCode can be generated. We need variable offsets and types
// before the qcode is created. This is fine for the locals and globals and parameters as
// they are always declared at the start of the PROC. Externals aren't, however, we find
// them as we translate the PROC lines. That means we can't generate the qcode until we
// have translated and found al the externals. To get around this the translation is done twice.
// The first pass is to collect the externals, although error will also be found then. The entire
// variable table is built and kept for the second pass.
// The second pass then generates the header and qcode, as it has all the information it needs.
// There's a pass number that tells the code what it needs to do

typedef struct _VAR_INFO
{
  char name[NOBJ_VARNAME_MAXLEN];
  int is_array;
  int is_integer;
  int is_float;
  int is_string;
  int max_array;
  int max_string;
  NOBJ_VARTYPE type;
  uint16_t offset;    // Offset from FP
} VAR_INFO;

VAR_INFO local_info[NOPL_MAX_LOCAL] PSRAM;
VAR_INFO global_info[NOPL_MAX_GLOBAL] PSRAM;

int local_info_index  = 0;
int global_info_index = 0;

void output_qcode_for_line(void)
{
  NOBJ_VAR_INFO *vi = NULL;
  
  // Do nothing on first pass
  if( pass_number == 1 )
    {
      return;
    }

  // We have to take the header off to get back to the start of the line
  ff_fprintf(icfp, "\nQCode idx:%04X", qcode_idx-13);
  ff_fprintf(icfp, "  --++  %s  --++", last_line);
  
  dump_exp_buffer(icfp, 2);

  //------------------------------------------------------------------------------
  // We are now able to append qcode to the qcode output
  // Run through the exp_buffer and convert the tokens into QCode...
  //------------------------------------------------------------------------------
  
  dbprintf("================================================================================");
  dbprintf("Generating QCode     Pass:%d Buf2_i:%d qcode_idx:%04X", pass_number, exp_buffer2_i, qcode_idx);
  dbprintf("================================================================================");

  // Things need to be detected so qcode can be generated earlier than the normal token generation
  // would have created them.
  int elseif_present = 0;
  int is_an_assignment = 0;
  
  for(int i=0; i<exp_buffer2_i; i++)
    {
      if( (exp_buffer2[i].op.buf_id == EXP_BUFF_ID_OPERATOR) && (strcmp(exp_buffer2[i].op.name, ":=")==0) )
	{
	  is_an_assignment = 1;
	}
      
      if( (exp_buffer2[i].op.buf_id == EXP_BUFF_ID_META) && (strcmp(exp_buffer2[i].op.name, "BRAENDIF")==0) )
	{
	  elseif_present = 1;

	  // This line is for an elseif, so we need to put a goto here to skip over the code we
	  // are about to generate
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_GOTO);

	  // Add an entry to the qcode fixups to fill in the goto offset
	  // Add an index for the IF to branch to, make it look like an else
	  add_cond_fixup(qcode_idx, qcode_idx+2, EXP_BUFF_ID_BRAENDIF, exp_buffer2[i].op.level);	  
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  break;
	}
    }

  // We need to know where the line starts for the WHILE
  // todo: Check this is correct in all cases. If the parser uses a finalise call then the OPL line
  // could be split across multiple intcode lines.
  
  int start_of_line_qcode_idx = qcode_idx;
  
  for(int i=0; i<exp_buffer2_i; i++)
    {
      EXP_BUFFER_ENTRY token = exp_buffer2[i];
      NOBJ_QCODE qc;

       last_line_is_return = 0;
       
#define tokop token.op
      
      dbprintf("QC: i:%d", i);
      
      if( (exp_buffer2[i].op.buf_id < 0) || (exp_buffer2[i].op.buf_id > EXP_BUFF_ID_MAX) )
	{
	  dbprintf("N%d buf_id invalid", token.node_id);
	}
      
      switch(token.op.buf_id)
	{
	case EXP_BUFF_ID_PROC_CALL:
	  // We generate qcode to stack the parameter types, then number of parameters, then the QCO_PROC
	  // then a string which is the proc name
#if 0
	  for(int i=token.op.num_parameters-1; i>=0; i--)
	    {
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  QI_STK_LIT_BYTE);
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  token.op.parameter_type[i]);
	    }
#endif
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  QI_STK_LIT_BYTE);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  token.op.num_parameters);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  QCO_PROC);

	  // Now the proc name, length prefixed.
	  char procname[NOBJ_VARNAME_MAXLEN];
	  strcpy(procname, token.op.name);
	  //	  procname[strlen(procname)-1] = '\0';
	  
	  qcode_idx = qcode_add_length_prefixed_string(qcode_idx, procname);
	  break;
	  
	case EXP_BUFF_ID_META:
	  // On pass 2 when we see the PROCDEF we generate the qcode header,
	  // each line then generates qcodes after that
	  dbprintf("QC:META '%s'", tokop.name);

	  if( (strcmp(exp_buffer2[i].name, "TEST_EXPR") == 0) )
	    {
	      // Add conditional fixup information for the test expressions of
	      // WHILE and UNTIL. This is for the CONTINUE command
	      add_cond_fixup(qcode_idx, qcode_idx, EXP_BUFF_ID_TEST_EXPR, exp_buffer2[i].op.level);
	    }

	  if( (strcmp(exp_buffer2[i].name, "BYTE") == 0) )
	    {
	      // Put a byte in the QCode stream
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  exp_buffer2[i].op.integer);
	    }
	  
	  if( (strcmp(exp_buffer2[i].name, " CREATE") == 0) || (strcmp(exp_buffer2[i].name, " OPEN") == 0) )
	    {
	      // Work out the qcode for the CREATE or OPEN
	      if( (strcmp(exp_buffer2[i].name, " CREATE") == 0) )
		{
		  qc =  QCO_CREATE;
		}
	      else
		{
		  qc =  QCO_OPEN;
		}

	      qcode_idx = qcode_check_trapped(qcode_idx, &tokop);
	      
	      // Filename is already stacked as an expression
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, qc);

	      // Add the logical file name
	      i++;
	      if( i > (MAX_EXP_BUFFER-1) )
		{
		  internal_error("exp_buffer2 overflow");
		  return;
		}
		
	      if( exp_buffer2[i].op.buf_id == EXP_BUFF_ID_LOGICALFILE )
		{
		  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, exp_buffer2[i].op.name[0] - 'A');
		}
	      else
		{
		  syntax_error("Logical file missing");
		}
	      
	      i++;
	      if( i > (MAX_EXP_BUFFER-1) )
		{
		  internal_error("exp_buffer2 overflow");
		  return;
		}

	      // Now add the field variables
	      while( !((exp_buffer2[i].op.buf_id == EXP_BUFF_ID_META) && (strcmp(exp_buffer2[i].op.name, "ENDFIELDS")==0)) )
		{
		  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, exp_buffer2[i].op.type);

		  // Put field variable name without the logical file character next
		  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, strlen(exp_buffer2[i].op.name)-2);
		  for(int n=0;n<strlen(exp_buffer2[i].op.name)-2;n++)
		    {
		      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, exp_buffer2[i].op.name[n+2]);
		    }
		  i++;
		  if( i > (MAX_EXP_BUFFER-1) )
		    {
		      internal_error("exp_buffer2 overflow");
		      return;
		    }
		}
	      
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_END_FIELDS);
	      return;
	    }
	  
	  if( pass_number == 2 )
	    {
	      if( strcmp(exp_buffer2[i].name, "PROCDEF")==0 )
		{
		  dbprintf("QC:Building QCode header");
		  qcode_idx = 0;
		  build_qcode_header();
		  ff_fprintf(icfp, "Header built qcode_idx:%04X", qcode_idx);
		  //		  exit(0);
		  // Do not process line further
		  return;
		}

	      if( strcmp(tokop.name, " LOCAL")==0 )
		{
		  return;
		}

	      if( strcmp(tokop.name, " GLOBAL")==0 )
		{
		  return;
		}
	    }

	  // Proc call parameters need a type on the stack
	  // Procedure calls cannot have arrays passed to them, so all array types
	  // need to be turned into their non-array versions.

	  if( (strcmp(exp_buffer2[i].name, "PAR_TYPE") == 0) )
	    {
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  QI_STK_LIT_BYTE);
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  convert_type_to_non_array(token.op.type));
	      continue;
	    }
	  
	  // Check the simple mapping table
	  qcode_idx = add_simple_qcode(qcode_idx, &(token.op), vi);

	  break;

	  // If there's a return keyword then there would have been a return value that should have been stacked.
	  // The qcode for this situation is  QCO_RETURN
	  // If there's no RETURN then a code that stacks a zero/null has to be added once the translating ends.
	case EXP_BUFF_ID_RETURN:
	  procedure_has_return = 1;
	  last_line_is_return = 1;
	  if( token.op.access == NOPL_OP_ACCESS_EXP )
	    {
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  QCO_RETURN);
	    }
	  else
	    {
	      switch(procedure_type)
		{
		case NOBJ_VARTYPE_INT:
		  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  QCO_RETURN_NOUGHT);
		  break;

		case NOBJ_VARTYPE_FLT:
		  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  QCO_RETURN_ZERO);
		  break;

		case NOBJ_VARTYPE_STR:
		  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  QCO_RETURN_NULL);
		  break;
		}
	    }
	  break;
	  
	case EXP_BUFF_ID_VARIABLE:

	  // If the variable is a field variable then we generate different QCode
	  if( tokop.name[1] == '.' )
	    {
	      // Push the name as a string literal
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  QI_STR_CON);
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  strlen(tokop.name)-2);

	      for(int s=0; s<strlen(tokop.name)-2; s++)
		{
		  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  toupper(tokop.name[s+2]));
		}

	      // Now the reference push
	      switch( token.op.access )
		{
		case NOPL_OP_ACCESS_WRITE:
		  
		  switch(token.op.type)
		    {
		    case NOBJ_VARTYPE_INT:
		      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,   QI_LS_INT_FLD);
		      break;
		      
		    case NOBJ_VARTYPE_FLT:
		      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,   QI_LS_NUM_FLD);
		      break;
		      
		    case NOBJ_VARTYPE_STR:
		      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,   QI_LS_STR_FLD);
		      break;
		    }
		  break;

		case NOPL_OP_ACCESS_READ:

		  switch(token.op.type)
		    {
		    case NOBJ_VARTYPE_INT:
		      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,   QI_INT_FLD);
		      break;
		      
		    case NOBJ_VARTYPE_FLT:
		      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,   QI_NUM_FLD);
		      break;
		      
		    case NOBJ_VARTYPE_STR:
		      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,   QI_STR_FLD);
		      break;
		    }
		}
	      
	      // Now logical file
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  tokop.name[0]-'A');
	    }
	  else
	    {
	      // Find the info about this variable
	      
	      vi = find_var_info(tokop.name, tokop.type);
	      
	      switch(vi->class)
		{
		case NOPL_VAR_CLASS_CALC_MEMORY:
		  qcode_idx = add_simple_qcode(qcode_idx, &(token.op), vi);
		  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, ((vi->offset)*8) >> 8);
		  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, ((vi->offset)*8) & 0xFF);
		  break;
		  
		default:
		  qcode_idx = add_simple_qcode(qcode_idx, &(token.op), vi);
		  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, (vi->offset) >> 8);
		  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, (vi->offset) & 0xFF);
		  break;
		}
	    }
	  break;

	case EXP_BUFF_ID_LOGICALFILE:
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, tokop.name[0] - 'A');
	  break;
	  
	case EXP_BUFF_ID_INTEGER:
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QI_INT_CON);

	  // Convert integer and add to qcode
	  int intval;

	  sscanf(tokop.name, "%d", &intval);
	  
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, (intval) >> 8);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, (intval) & 0xFF);
	  break;

	case EXP_BUFF_ID_BYTE:
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QI_STK_LIT_BYTE);

	  // Convert integer and add to qcode

	  sscanf(tokop.name, "%d", &intval);
	  dbprintf("Name:'%s' intval:%d", tokop.name, intval);	  
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, (intval) & 0xFF);
	  break;

	case EXP_BUFF_ID_FLT:
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QI_NUM_CON);

	  // Convert to 'compact' float form
	  // Float may or may not have a decimal point.
	  // We can handle up to 12 significant digits
	  
	  qcode_idx = convert_to_compact_float(qcode_idx, tokop.name);
#if 0
	  dbprintf("fltval:%f %lf", float_val, float_val);
	  
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, (intval) >> 8);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, (intval) & 0xFF);
#endif
	  break;

	  
	case EXP_BUFF_ID_PRINT_NEWLINE:
	  dbprintf("QC:PRINT");
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_PRINT_CR);
	  break;

	case EXP_BUFF_ID_PRINT_SPACE:
	  dbprintf("QC:PRINT");
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_PRINT_SP);
	  break;
	  
	case EXP_BUFF_ID_PRINT:
	  dbprintf("QC:PRINT");
	  switch(token.op.type)
	    {
	    case NOBJ_VARTYPE_INT:
	    case NOBJ_VARTYPE_INTARY:
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_PRINT_INT);
	      break;

	    case NOBJ_VARTYPE_FLT:
	    case NOBJ_VARTYPE_FLTARY:
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_PRINT_NUM);
	      break;

	    case NOBJ_VARTYPE_STR:
	    case NOBJ_VARTYPE_STRARY:
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_PRINT_STR);
	      break;
	    }
	  break;

	case EXP_BUFF_ID_LPRINT_NEWLINE:
	  dbprintf("QC:LPRINT");
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_LPRINT_CR);
	  break;

	case EXP_BUFF_ID_LPRINT_SPACE:
	  dbprintf("QC:LPRINT");
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_LPRINT_SP);
	  break;
	  
	case EXP_BUFF_ID_LPRINT:
	  dbprintf("QC:LPRINT");
	  switch(token.op.type)
	    {
	    case NOBJ_VARTYPE_INT:
	    case NOBJ_VARTYPE_INTARY:
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_LPRINT_INT);
	      break;

	    case NOBJ_VARTYPE_FLT:
	    case NOBJ_VARTYPE_FLTARY:
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_LPRINT_NUM);
	      break;

	    case NOBJ_VARTYPE_STR:
	    case NOBJ_VARTYPE_STRARY:
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_LPRINT_STR);
	      break;
	    }
	  break;

	  // Put the offset of the label into the fixup table
	case EXP_BUFF_ID_LABEL:
	  add_cond_fixup_label(qcode_idx, qcode_idx, token.op.buf_id, token.name);
	  break;

#if 1
	case EXP_BUFF_ID_ONERR:
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_ONERR);

	  if( strcmp(token.name, "0")==0 )
	    {
	    }
	  else
	    {
	      add_cond_fixup_label(qcode_idx, qcode_idx, token.op.buf_id, token.name);
	    }
	  
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  break;
#endif
	  // Put an entry into the fixup table to fill in the offset
	case EXP_BUFF_ID_GOTO:
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_GOTO);
	  add_cond_fixup_label(qcode_idx, qcode_idx, token.op.buf_id, token.name);
	  
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  break;

	case EXP_BUFF_ID_CONTINUE:
	case EXP_BUFF_ID_BREAK:
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_GOTO);
	  add_cond_fixup(qcode_idx, qcode_idx, token.op.buf_id, token.op.level);
	  
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  break;
	  
	case EXP_BUFF_ID_UNTIL:
	  // We put zero in as a dummy jump offset and add it to the conditionals fixup table
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_BRA_FALSE);
	  add_cond_fixup(qcode_idx, qcode_idx, token.op.buf_id, token.op.level);
	  
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  break;
	  	  
	case EXP_BUFF_ID_WHILE:
	  // Branch to skip the while clause if test false
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_BRA_FALSE);

	  // Add fixup entry
	  add_cond_fixup(qcode_idx, start_of_line_qcode_idx, token.op.buf_id, token.op.level);

	  // We find the offset of the next item
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  break;

	case EXP_BUFF_ID_ENDWH:
	  // Use a goto to loop back to the WHILE
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_GOTO);

	  // Add an entry to the qcode fixups to fill in the goto offset
	  // Add an index for the IF to branch to
	  add_cond_fixup(qcode_idx, qcode_idx+2, token.op.buf_id, token.op.level);	  
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  break;

	case EXP_BUFF_ID_ELSEIF:
	  // Put a branch in to skip over the IF clause code if the test fails
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_BRA_FALSE);

	  // Add an entry to the qcode fixups
	  add_cond_fixup(qcode_idx, qcode_idx+2, token.op.buf_id, token.op.level);

	  // We find the offset of the next item
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  break;
	  
	case EXP_BUFF_ID_IF:
	  // Put a branch in to skip over the IF clause code if the test fails
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_BRA_FALSE);

	  // Add an entry to the qcode fixups
	  add_cond_fixup(qcode_idx, qcode_idx, token.op.buf_id, token.op.level);

	  // We find the offset of the next item
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  break;

	case EXP_BUFF_ID_ELSE:
	  // Use a goto for the previous IF or ELSEIF code to jump over the ELSE clause.
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QCO_GOTO);

	  // Add an entry to the qcode fixups to fill in the goto offset
	  // Add an index for the IF to branch to
	  add_cond_fixup(qcode_idx, qcode_idx+2, token.op.buf_id, token.op.level);	  
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, 0x00);
	  break;
	  
	case EXP_BUFF_ID_DO:
	case EXP_BUFF_ID_ENDIF:
	  // No Qcode, we just create a point where UNTIL can branch back to
	  add_cond_fixup(qcode_idx, qcode_idx, token.op.buf_id, token.op.level);
	  break;
	  
	case EXP_BUFF_ID_STR:
	  // String literal
	  dbprintf("QC:%d String Literal '%s' %s", i, exp_buffer2[i].name, exp_buffer_id_str[exp_buffer2[i].op.buf_id]);
	  
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, QI_STR_CON);
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, strlen(exp_buffer2[i].name)-2);
	  
	  for(int j=1; j<strlen(exp_buffer2[i].name)-1; j++)
	    {
	      qcode_idx = set_qcode_header_byte_at(qcode_idx, 1, exp_buffer2[i].name[j]);
	    }
	  break;
#if 1
	case EXP_BUFF_ID_OPERATOR_UNARY:
	  dbprintf("BUFF_ID_OPERATOR_UNARY");
	  OP_INFO op_info;
	  
	  if( find_op_info(exp_buffer2[i].name, &op_info) )
	    {
	      dbprintf("Found operator %s", exp_buffer2[i].name);
	      
	      // If we need to change the qcode based on the argument type then we do that here
	      if( op_info.qcode_type_from_arg )
		{
		  // Set the qcode type from the argument type
		  // Only one argument for unary operators...
		  EXP_BUFFER_ENTRY *e = find_buf2_entry_with_node_id(exp_buffer2[i].p[0]);

		  if( e != NULL )
		    {
		      token.op.type = op_info.output_type;
		      token.op.qcode_type = e->op.type;
		    }
		  else
		    {
		      internal_error("Could not find entry for N%03d", exp_buffer2[i].p[0]);
		    }
		}
	    }
	  else
	    {
	      internal_error("Could not find op_info for '%s'", exp_buffer2[i].name);
	    }
	  
	  qcode_idx = add_simple_qcode(qcode_idx, &(token.op), vi);	  
	  break;
#endif
	  
	default:
	  // Check the simple mapping table
	  qcode_idx = add_simple_qcode(qcode_idx, &(token.op), vi);
	  break;
	}
    }

  qcode_len = qcode_idx - qcode_start_idx;
}

void output_qcode_suffix(void)
{
  dbprintf("Has return:%d last line return:%d", procedure_has_return, last_line_is_return);
  
  if( !last_line_is_return )
    {
      switch(procedure_type)
	{
	case NOBJ_VARTYPE_INT:
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  QCO_RETURN_NOUGHT);
	  break;
	  
	case NOBJ_VARTYPE_FLT:
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  QCO_RETURN_ZERO);
	  break;
	  
	case NOBJ_VARTYPE_STR:
	  qcode_idx = set_qcode_header_byte_at(qcode_idx, 1,  QCO_RETURN_NULL);
	  break;
	}
    }
  
  qcode_len = qcode_idx - qcode_start_idx;
}


////////////////////////////////////////////////////////////////////////////////
//
// Variable name parsing
//
// The type of the variable is determined.
//

char *gns_ptr;
char gn_str[NOBJ_VARNAME_MAXLEN];

void init_get_name(char *s)
{
  gns_ptr = s;
  dbprintf("\n%s:'%s'", __FUNCTION__, gns_ptr);
  
  // Skip leading spaces
  while( ((*gns_ptr) != '\0') && isspace(*gns_ptr) )
    {
      gns_ptr++;
    }

  dbprintf("\n%s:'%s'", __FUNCTION__, gns_ptr);
}


char *get_name(char *n, NOBJ_VARTYPE *t)
{
  int i = 0;
  
  while (*gns_ptr != '\0')
    {
      if( i>=(NOBJ_VARNAME_MAXLEN-1) )
	{
	  internal_error("gn_str overflow");
	  return("");
	}
      
      gn_str[i++] = *gns_ptr;
      switch(*gns_ptr)
	{
	case '$':
	  *t = NOBJ_VARTYPE_STR;
	  gn_str[i] = '\0';
	  dbprintf("\n%s:gn:'%s'", __FUNCTION__, gn_str);
	  return(gn_str);
	  break;

	case '%':

	  *t = NOBJ_VARTYPE_INT;
	  gn_str[i] = '\0';
	  dbprintf("\n%s:gn:'%s'", __FUNCTION__, gn_str);
	  return(gn_str);
	  break;
	  
	case ',':
	  break;

	default:
	  
	  break;
	}

      gns_ptr++;
    }

  gn_str[i] = '\0';
  dbprintf("\n%s:gn:'%s'", __FUNCTION__, gn_str);
  *t = NOBJ_VARTYPE_FLT;
  return(gn_str);
}

void output_qcode_variable(char *def)
{
  char vname[NOBJ_VARNAME_MAXLEN];
  NOBJ_VARTYPE type;
  
  dbprintf("\n%s: %s", __FUNCTION__, def);

  if( strstr(def, "GLOBAL") != NULL )
    {
#if 0
      // Get variable names
      init_get_name(def);

      if( get_name(vname, &type) )
	{
	  modify_expression_type(type);
	  type = expression_type;
	}
#endif
    }

  if( strstr(def, "LOCAL") != NULL )
    {
    }

  
}

////////////////////////////////////////////////////////////////////////////////

char type_to_char(NOBJ_VARTYPE t)
{
  char c;
  
  switch(t)
    {
    case NOBJ_VARTYPE_INT:
      c = 'i';
      break;

    case NOBJ_VARTYPE_FLT:
      c = 'f';
      break;

    case NOBJ_VARTYPE_STR:
      c = 's';
      break;

    case NOBJ_VARTYPE_INTARY:
      c = 'I';
      break;

    case NOBJ_VARTYPE_FLTARY:
      c = 'F';
      break;

    case NOBJ_VARTYPE_STRARY:
      c = 'S';
      break;

    case NOBJ_VARTYPE_VAR_ADDR:
      c = 'V';
      break;

    case NOBJ_VARTYPE_UNKNOWN:
      c = 'U';
      break;

    case NOBJ_VARTYPE_VOID:
      c = 'v';
      break;
      
    default:
      c = '?';
      break;
    }
  
  return(c);
}

//------------------------------------------------------------------------------

char access_to_char(NOPL_OP_ACCESS a)
{
  char c;
  
  switch(a)
    {
    case NOPL_OP_ACCESS_UNKNOWN:
      c = 'U';
      break;

    case NOPL_OP_ACCESS_READ:
      c = 'R';
      break;

    case NOPL_OP_ACCESS_WRITE:
      c = 'W';
      break;

    case NOPL_OP_ACCESS_EXP:
      c = 'X';
      break;

    case NOPL_OP_ACCESS_NO_EXP:
      c = 'x';
      break;
      
    default:
      c = '?';
      break;
    }
  
  return(c);
}

//------------------------------------------------------------------------------
//
// Turn any array type to its non-array version
//

NOBJ_VARTYPE convert_type_to_non_array(NOBJ_VARTYPE t)
{
  switch(t)
    {
	case NOBJ_VARTYPE_INTARY:
	  return(NOBJ_VARTYPE_INT);
	  break;
	  
	case NOBJ_VARTYPE_FLTARY:
	  return(NOBJ_VARTYPE_FLT);
	  break;

	case NOBJ_VARTYPE_STRARY:
	  return(NOBJ_VARTYPE_STR);
	  break;
    }

  return(t);
}

//------------------------------------------------------------------------------
//
// A new token has appeared in an expression. Modify the expression based on
// this new type.
//
//

void modify_expression_type(NOBJ_VARTYPE t)
{
  ff_fprintf(ofp, "\n%s:Inittype:%c", __FUNCTION__, type_to_char(expression_type));
  switch(expression_type)
    {
    case NOBJ_VARTYPE_UNKNOWN:
      expression_type = t;
      break;

    case NOBJ_VARTYPE_INT:
      switch(t)
	{
	case NOBJ_VARTYPE_INT:
	  break;
	  
	case NOBJ_VARTYPE_FLT:
	  // Move to float type
	  expression_type = t;
	  break;

	case NOBJ_VARTYPE_STR:
	  // Syntax error
	  break;
	}
      break;

    case NOBJ_VARTYPE_FLT:
      switch(t)
	{
	case NOBJ_VARTYPE_INT:
	  // Type conversion
	  break;
	  
	case NOBJ_VARTYPE_FLT:
	  break;

	case NOBJ_VARTYPE_STR:
	  // Syntax error
	  break;
	}
      break;

    case NOBJ_VARTYPE_STR:
      switch(t)
	{
	case NOBJ_VARTYPE_INT:
	case NOBJ_VARTYPE_FLT:
	  // Syntax error
	  //expression_type = t;
	  break;

	case NOBJ_VARTYPE_STR:
	  expression_type = t;
	  break;
	}
      break;
    }
  
  ff_fprintf(ofp, " Intype:%c Outtype:%c", type_to_char(t), type_to_char(expression_type));
}

////////////////////////////////////////////////////////////////////////////////
//
// Output expression processing
//
// Each expression is placed in this buffer and processed to create the
// corresponding QCode.
//
// The buffer holds RPN codes with functions and operands in text form. This is
// executed as RPN on a stack, but with no function. This is done to check the
// typing of the functions and operands, and also as a general way to insert the
// automatic type conversion codes.
//
// The reason that this is done is that the operands for an operator may be
// arbitrarily far from the operator.
//
// For instance:
//
//     B = 2 * (3 * ( 4 + (1 + 1)))
//
//  is the following in RPN:
//
//
//    B
//    2      <=== First operand
//    3
//    4
//    1
//    1
//    +
//    +
//    *
//    *      <==== operator
//    =
//
// From this you can see that the first operand of the * operator is
// many entries in the stack away from the operator. Working out if an auto
// conversion code should be inserted when that operand is emitted to the qcode
// stream requires a look-ahead of many stack entries. Executing the code as RPN allows the
// positions of auto conversion codes to be determined in a general way.
//
// 
////////////////////////////////////////////////////////////////////////////////
//
//
// Typechecking execution stack
//

void type_check_stack_print(void);



EXP_BUFFER_ENTRY type_check_stack[MAX_TYPE_CHECK_STACK] PSRAM;
int type_check_stack_ptr = 0;

void type_check_stack_push(EXP_BUFFER_ENTRY entry)
{
  dbprintf(" %s: '%s'", __FUNCTION__, entry.name);
  ff_fprintf(exfp, "\nPush:'%s'", entry.name);
 
  if( type_check_stack_ptr < MAX_TYPE_CHECK_STACK )
    {
      type_check_stack[type_check_stack_ptr++] = entry;
    }
  else
    {
      ff_fprintf(ofp, "\n%s: Operator stack full", __FUNCTION__);
      ff_fprintf(exfp, "\nOperator stack full");
      typecheck_error("Stack full");
      n_stack_errors++;
    }
  
  type_check_stack_print();

}

// Copies data into string

EXP_BUFFER_ENTRY type_check_stack_pop(void)
{
  EXP_BUFFER_ENTRY o;
  
  if( type_check_stack_ptr == 0 )
    {
      ff_fprintf(ofp, "\n%s: Operator stack empty", __FUNCTION__);
      ff_fprintf(exfp, "\nOperator stack empty");
      
      typecheck_error("Stack empty");
      n_stack_errors++;
      return(o);
    }
  
  type_check_stack_ptr --;

  o = type_check_stack[type_check_stack_ptr];
  
  dbprintf("  %s: '%s'", __FUNCTION__, o.name);
  ff_fprintf(exfp, "\nPop: '%s'", o.name);
  
  type_check_stack_print();
  return(o);
}

void type_check_stack_display(void)
{
  char *s;
  NOBJ_VARTYPE type;
  
  dbprintf("Type Check Stack (%d)", type_check_stack_ptr);

  for(int i=0; i<type_check_stack_ptr; i++)
    {
      s = type_check_stack[i].name;
      type = type_check_stack[i].op.type;
      
      dbprintf("%03d: '%s' type:%c (%d), %%:%d", i, s, type_to_char(type), type, type_check_stack[i].op.percent);
    }
}

void type_check_stack_fprint(FIL *fp)
{
  char *s;
  NOBJ_VARTYPE type;
  
  //ff_fprintf(fp, "\n                    Type Check Stack (%d)", type_check_stack_ptr);

  for(int i=0; i<type_check_stack_ptr; i++)
    {
      s = type_check_stack[i].name;
      type = type_check_stack[i].op.type;
      
      ff_fprintf(fp, "\n                    %03d: '%s' type:%c (%d), %%:%d", i, s, type_to_char(type), type, type_check_stack[i].op.percent);
    }
}

int type_check_stack_only_field_data(void)
{
  char *s;
  NOBJ_VARTYPE type;
  int only_field_data = 1;
  
  dbprintf("Type Check Stack ptr:(%d)", type_check_stack_ptr);

  for(int i=0; i<type_check_stack_ptr; i++)
    {
      s = type_check_stack[i].name;
      type = type_check_stack[i].op.type;
      
      dbprintf("%03d: '%s' type:%c (%d)", i, s, type_to_char(type), type);
      switch(type_check_stack[i].op.buf_id )
	{
	case EXP_BUFF_ID_FIELDVAR:
	case EXP_BUFF_ID_LOGICALFILE:
	  break;

	default:
	  only_field_data = 0;
	  break;
	}
    }
  
  return(only_field_data);
}

void type_check_stack_print(void)
{
  char *s;

  dbprintf("------------------");
  dbprintf("Type Check Stack     (%d)\n", type_check_stack_ptr);
  
  for(int i=0; i<type_check_stack_ptr; i++)
    {
      s = type_check_stack[i].name;
      dbprintf(" N%03d: '%s' type:%d %%:%d", type_check_stack[i].node_id, s, type_check_stack[i].op.type, type_check_stack[i].op.percent );
    }

  dbprintf("------------------\n");
}

void type_check_stack_init(void)
{
  type_check_stack_ptr = 0;
}



//------------------------------------------------------------------------------
//

void clear_exp_buffer(void)
{
  exp_buffer_i = 0;
}

void add_exp_buffer_entry(OP_STACK_ENTRY op, int id)
{
  if( exp_buffer_i >= (MAX_EXP_BUFFER-1) )
    {
      internal_error("exp buffer overflow");
      return;
    }
  
  exp_buffer[exp_buffer_i].op = op;
  exp_buffer[exp_buffer_i].op.buf_id = id;
  strcpy(&(exp_buffer[exp_buffer_i].name[0]), op.name);
  exp_buffer_i++;
}

void add_exp_buffer2_entry(OP_STACK_ENTRY op, int id)
{
  if( exp_buffer2_i >= (MAX_EXP_BUFFER-1) )
    {
      internal_error("exp buffer2 overflow");
      return;
    }

  exp_buffer2[exp_buffer2_i].op = op;
  exp_buffer2[exp_buffer2_i].op.buf_id = id;
  strcpy(&(exp_buffer2[exp_buffer2_i].name[0]), op.name);
  exp_buffer2_i++;
}

////////////////////////////////////////////////////////////////////////////////

void dump_exp_buffer(FIL *fp, int bufnum)
{
  char *idstr;
  int exp_buff_len = 0;
  char levstr[10];
  
  switch(bufnum)
    {
    case 1:
      exp_buff_len = exp_buffer_i;
      break;
      
    case 2:
      exp_buff_len = exp_buffer2_i;
      break;
    }

  if( exp_buff_len == 0 )
    {
      return;
    }

  for(int i=0; i<exp_buff_len; i++)
    {
      EXP_BUFFER_ENTRY token;

	switch(bufnum)
	  {
	  case 1:
	    token = exp_buffer[i];
	    break;

	  case 2:
	    token = exp_buffer2[i];
	    break;
	  }

	if( token.op.level == 0 )
	  {
	    sprintf(levstr, "  %5s", "");
	  }
	else
	  {
	    sprintf(levstr, "L:%-5d", token.op.level);
	  }

	
	ff_fprintf(fp, "\nN%03d %10s %-30s %s ty:%c qcty:%c '%s' npar:%d nidx:%d trapped:%d %%:%d",
		token.node_id,
		var_access_to_str(token.op.access),
		exp_buffer_id_str[token.op.buf_id],
		levstr,
		type_to_char(token.op.type),
		type_to_char(token.op.qcode_type),
		token.name,
		token.op.num_parameters,
		token.op.vi.num_indices,
		token.op.trapped,
		token.op.percent);
      
      ff_fprintf(fp, "  %d:", token.p_idx);

      for(int pi=0; pi< MAX_EXP_BUF_P/*token.p_idx*/; pi++)
      //for(int pi=0; pi< token.p_idx; pi++)
	{
	  ff_fprintf(fp, " %d", token.p[pi]);
	}

      ff_fprintf(fp, "  nb %d:(", token.op.num_bytes);
      
      for(int i=0; (i<token.op.num_bytes) & (i < NOPL_MAX_SUFFIX_BYTES); i++)
	{
	  ff_fprintf(fp, " %02X", token.op.bytes[i]);
	}
      ff_fprintf(fp, ")");
      //fflush(fp);
    }

  ff_fprintf(fp, "\n");
  //fflush(fp);
}

////////////////////////////////////////////////////////////////////////////////
//
// Expression tree
//
// Once we have an RPN version of the expression we then build a tree. This
// allows the auto type conversion to be done in a general way by traversing
// the tree looking for type conflict and inserting auto conversion nodes.
//
//
////////////////////////////////////////////////////////////////////////////////

EXP_BUFFER_ENTRY *find_buf2_entry_with_node_id(int node_id)
{
  // Find the entry with the given node_id
  dbprintf("Find entry with node id N%03d", node_id);
  
  for(int i= 0; i<exp_buffer2_i; i++)
    {
      if( exp_buffer2[i].node_id == node_id )
	{
	  ff_fprintf(ofp, "\n   Found at i:%d", i);
	  return(&(exp_buffer2[i]));
	}
    }
  return(NULL);
}

//------------------------------------------------------------------------------
//
// Copy buf back to buf
// This is done after the syntax processing so that we can run the type check
// This could be optimised away.

void copy_buf2_to_buf(void)
{
    for(int i= 0; i<exp_buffer2_i; i++)
      {
	exp_buffer[i] = exp_buffer2[i];
      }

    exp_buffer_i = exp_buffer2_i;
}

// Insert a buffer2 entry into the list, after the entry with the given
// node_id.

int insert_buf2_entry_after_node_id(int node_id, EXP_BUFFER_ENTRY e)
{
  int j;

  if( exp_buffer2_i >= (MAX_EXP_BUFFER-1) )
    {
      internal_error("exp buffer 2 overflow (insert)");
      return(-1);
    }

  dump_exp_buffer(ofp, 2);
  e.node_id = node_id_index++;
				  
  // Find the entry with the given node_id
  ff_fprintf(ofp, "\n Insert after %d exp_buffer2_i:%d", node_id, exp_buffer2_i);
  
  for(int i= 0; i<exp_buffer2_i; i++)
    {
      if( exp_buffer2[i].node_id == node_id )
	{
	  ff_fprintf(ofp, "\n   Found at i:%d", i);
	  
	  // Found the entry, move all after it on by one entry
	  for(j=exp_buffer2_i; j>=i+2; j--)
	    {
	      exp_buffer2[j] = exp_buffer2[j-1];
	      ff_fprintf(ofp, "\n   Copied %d to %d:", j-1, j);
	    }
	  
	  exp_buffer2[i+1] = e;
	  exp_buffer2_i++;

	  dump_exp_buffer(ofp, 2);
	  return(-1);	  
	}
    }
  
  return(0);
}

int insert_buf2_entry_before_node_id(int node_id, EXP_BUFFER_ENTRY e)
{
  int j;

  if( exp_buffer2_i >= (MAX_EXP_BUFFER-1) )
    {
      internal_error("exp buffer 2 overflow (insert)");
      return(0);
    }

  dump_exp_buffer(ofp, 2);
  e.node_id = node_id_index++;
				  
  // Find the entry with the given node_id
  ff_fprintf(ofp, "\n Insert before N%03d exp_buffer2_i:%d", node_id, exp_buffer2_i);
  
  for(int i= 0; i<exp_buffer2_i; i++)
    {
      if( exp_buffer2[i].node_id == node_id )
	{
	  ff_fprintf(ofp, "\n   Found at i:%d", i);
	  
	  // Found the entry, move it and all after it on by one entry
	  for(j=exp_buffer2_i; j>=i+1; j--)
	    {
	      exp_buffer2[j] = exp_buffer2[j-1];
	      ff_fprintf(ofp, "\n   Copied %d to %d:", j-1, j);
	    }
	  
	  exp_buffer2[i] = e;
	  exp_buffer2_i++;

	  dump_exp_buffer(ofp, 2);
	  return(1);	  
	}
    }
  
  return(0);
}


//------------------------------------------------------------------------------
//
// Set access of a node, given the node id
//

void set_node_access(int node_id, NOPL_OP_ACCESS access)
{
  for(int i= 0; i<exp_buffer2_i; i++)
    {
      if( exp_buffer2[i].node_id == node_id )
	{
	  exp_buffer2[i].op.access = access;
	  return;
	}
    }

  // Not found
  internal_error("Did not find node to set access");
}

////////////////////////////////////////////////////////////////////////////////
//
// Return a type that can be reached by as few type conversions as possible.
//
////////////////////////////////////////////////////////////////////////////////


NOBJ_VARTYPE type_with_least_conversion_from(NOBJ_VARTYPE t1, NOBJ_VARTYPE t2)
{
  NOBJ_VARTYPE ret = t1;

  if( (t1 == NOBJ_VARTYPE_INT) && (t2 == NOBJ_VARTYPE_INT) )
    {
      ret = NOBJ_VARTYPE_INT;
    }

  if( (t1 == NOBJ_VARTYPE_FLT) && (t2 == NOBJ_VARTYPE_FLT) )
    {
      ret = NOBJ_VARTYPE_FLT;
    }

  if( (t1 == NOBJ_VARTYPE_FLT) && (t2 == NOBJ_VARTYPE_INT) )
    {
      ret = NOBJ_VARTYPE_FLT;
    }

  if( (t1 == NOBJ_VARTYPE_INT) && (t2 == NOBJ_VARTYPE_FLT) )
    {
      ret = NOBJ_VARTYPE_FLT;
    }

  if( (t1 == NOBJ_VARTYPE_INTARY) && (t2 == NOBJ_VARTYPE_INTARY) )
    {
      ret = NOBJ_VARTYPE_INTARY;
    }

  if( (t1 == NOBJ_VARTYPE_FLTARY) && (t2 == NOBJ_VARTYPE_FLTARY) )
    {
      ret = NOBJ_VARTYPE_FLTARY;
    }

  if( (t1 == NOBJ_VARTYPE_FLTARY) && (t2 == NOBJ_VARTYPE_INTARY) )
    {
      ret = NOBJ_VARTYPE_FLTARY;
    }

  if( (t1 == NOBJ_VARTYPE_INTARY) && (t2 == NOBJ_VARTYPE_FLTARY) )
    {
      ret = NOBJ_VARTYPE_FLTARY;
    }

  ff_fprintf(ofp, "\n%s: %c %c => %c",  __FUNCTION__, type_to_char(t1), type_to_char(t2), type_to_char(ret));
  return(ret);
}

int types_identical(NOBJ_VARTYPE t1, NOBJ_VARTYPE t2)
{
  if( t1 == t2 )
    {
      return(1);
    }

  if( (t1 == NOBJ_VARTYPE_INT) && (t2 == NOBJ_VARTYPE_INTARY) )
    {
      return(1);
    }

  if( (t1 == NOBJ_VARTYPE_FLT) && (t2 == NOBJ_VARTYPE_FLTARY) )
    {
      return(1);
    }

  if( (t1 == NOBJ_VARTYPE_STR) && (t2 == NOBJ_VARTYPE_STRARY) )
    {
      return(1);
    }

  if( (t2 == NOBJ_VARTYPE_INT) && (t1 == NOBJ_VARTYPE_INTARY) )
    {
      return(1);
    }

  if( (t2 == NOBJ_VARTYPE_FLT) && (t1 == NOBJ_VARTYPE_FLTARY) )
    {
      return(1);
    }

  if( (t2 == NOBJ_VARTYPE_STR) && (t1 == NOBJ_VARTYPE_STRARY) )
    {
      return(1);
    }

  return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Convert exp_buffer into an infix expression
//
////////////////////////////////////////////////////////////////////////////////
//
// This is useful for checking the translator.
//
////////////////////////////////////////////////////////////////////////////////

#if INFIX_FROM_RPN

#define MAX_INFIX_STACK 500
#define MAX_INFIX_STR   400

char infix_stack[MAX_INFIX_STACK][MAX_INFIX_STR] PSRAM;
int infix_stack_ptr = 0;
char result[MAX_INFIX_STR*20] PSRAM;

void infix_stack_push(char *entry)
{
  ff_fprintf(ofp, "\n%s: '%s'", __FUNCTION__, entry);
  
  if( infix_stack_ptr < MAX_INFIX_STACK )
    {
      strcpy(infix_stack[infix_stack_ptr++], entry);
    }
  else
    {
      ff_fprintf(ofp, "\n%s: Operator stack full", __FUNCTION__);
      typecheck_error("Operator stack full");
      return;
    }
}

// Copies data into string

void infix_stack_pop(char *entry)
{
  EXP_BUFFER_ENTRY o;
  
  if( infix_stack_ptr == 0 )
    {
      ff_fprintf(ofp, "\n%s: Operator stack empty", __FUNCTION__);
      typecheck_error("%s: Operator stack empty", __FUNCTION__);
      return;
    }
  
  infix_stack_ptr --;

  strcpy(entry, infix_stack[infix_stack_ptr]);
  
  ff_fprintf(ofp, "\n%s: '%s'", __FUNCTION__, entry);
}

////////////////////////////////////////////////////////////////////////////////

char *infix_from_rpn(void)
{
  EXP_BUFFER_ENTRY be;
  char op1[MAX_INFIX_STR], op2[MAX_INFIX_STR];
  char newstr[MAX_INFIX_STR*20+5];

  char newstr2[MAX_INFIX_STR*20+5];
  int numarg;
  
  infix_stack_ptr = 0;
  
  for(int i=0; i<exp_buffer2_i; i++)
    {
      
      be = exp_buffer2[i];

      dbprintf("(%s)", be.name);
      
      switch(be.op.buf_id)
	{
	case EXP_BUFF_ID_VAR_ADDR_NAME:
	  ff_fprintf(ofp, "\nVar ADDR Name: %s", be.name);
	  infix_stack_push(be.name);
	  break;
	  
	case EXP_BUFF_ID_VARIABLE:
	  ff_fprintf(ofp, "\nVar: %s %s NumIdx:%d", be.name,
		  type_to_str(be.op.vi.type),
		  be.op.vi.num_indices);

	  if( var_type_is_array(be.op.vi.type) )
	    {
	      // Pop the number of array indices the variable has off the stack
	      // and build the variable name plus indices.
	      strcpy(newstr2, be.name);
	      strcat(newstr2, "(");

	      switch(be.op.vi.num_indices)
		{
		case 1:
		  infix_stack_pop(op1);
		  strcat(newstr2, op1);
		  break;

		case 2:
		  infix_stack_pop(op1);
		  infix_stack_pop(op2);
		  strcat(newstr2, op2);
		  strcat(newstr2, ",");
		  strcat(newstr2, op1);
		  break;
		}
	      
	      strcat(newstr2, ")");
	      infix_stack_push(newstr2);
	    }
	  else
	    {
	      // Non array variables are just pushed on the stack
	      infix_stack_push(be.name);
	    }
	  break;

	  // Just put the name in the output
	case EXP_BUFF_ID_UNTIL:

	case EXP_BUFF_ID_ELSEIF:
	case EXP_BUFF_ID_WHILE:
	case EXP_BUFF_ID_IF:
	  infix_stack_pop(op1);
	  ff_fprintf(ofp, "\n%s", be.name);
	  snprintf(newstr2, MAX_INFIX_STR, "%s %s", be.name, op1);
	  infix_stack_push(newstr2);
	  break;

	  
	case EXP_BUFF_ID_DO:
	case EXP_BUFF_ID_TRAP:
	case EXP_BUFF_ID_ELSE:
	case EXP_BUFF_ID_ENDIF:
	case EXP_BUFF_ID_ENDWH:
	  dbprintf("%s", be.name);
	  snprintf(newstr2, MAX_INFIX_STR, "%s", be.name);
	  infix_stack_push(newstr2);
	  dbprintf("endif done");
	  break;

	case EXP_BUFF_ID_LABEL:
	  dbprintf("%s::", be.name);
	  snprintf(newstr2, MAX_INFIX_STR, "%s::", be.name);
	  infix_stack_push(newstr2);
	  dbprintf("Label %s done", be.name);
	  break;
	  
	case EXP_BUFF_ID_GOTO:
	  dbprintf("GOTO %s", be.name);
	  snprintf(newstr2, MAX_INFIX_STR, "GOTO %s", be.name);
	  infix_stack_push(newstr2);
	  dbprintf("GOTO done");
	  break;
	  
	case EXP_BUFF_ID_PRINT:
	case EXP_BUFF_ID_LPRINT:
	  
	  ff_fprintf(ofp, "\n%s", be.name);

	  // Pop what we will print
	  infix_stack_pop(op1);

	  snprintf(newstr2, MAX_INFIX_STR, "%s(%s)", be.name, op1);
	  infix_stack_push(newstr2);

	  break;

	case EXP_BUFF_ID_PRINT_SPACE:
	  infix_stack_push("< > ");
	  break;

	case EXP_BUFF_ID_PRINT_NEWLINE:
	  infix_stack_push("<nl>");
	  break;

	case EXP_BUFF_ID_LPRINT_SPACE:
	  infix_stack_push("L< > ");
	  break;

	case EXP_BUFF_ID_LPRINT_NEWLINE:
	  infix_stack_push("L<nl>");
	  break;
	  
	case EXP_BUFF_ID_INTEGER:
	case EXP_BUFF_ID_FLT:
	  infix_stack_push(be.name);
	  break;
	  
	case EXP_BUFF_ID_STR:
	  sprintf(newstr2, "%s", be.name);
	  infix_stack_push(newstr2);
	  break;

	case EXP_BUFF_ID_COMMAND:
	case EXP_BUFF_ID_FUNCTION:
	  // See how many arguments to pop
	  numarg = function_num_args(be.name);

	  strcpy(newstr, "");
	  
	  for(int i=0; i<numarg; i++)
	    {
	      infix_stack_pop(op1);

	      sprintf(newstr2, "%s %s", op1, newstr);
	      strcpy(newstr, newstr2);
	    }
	  
	  snprintf(newstr2, MAX_INFIX_STR, "%s(%s)", be.name, newstr);
	  infix_stack_push(newstr2);
	  break;
	  
	case EXP_BUFF_ID_OPERATOR:
	  infix_stack_pop(op1);
	  infix_stack_pop(op2);
	  sprintf(newstr, "(%s %s %s)", op2, be.name, op1);
	  infix_stack_push(newstr);
	  break;

	case EXP_BUFF_ID_OPERATOR_UNARY:
	  infix_stack_pop(op1);
	  sprintf(newstr, "(%s %s)", be.name, op1);
	  infix_stack_push(newstr);
	  break;
	  
	case EXP_BUFF_ID_AUTOCON:
	  break;

	case EXP_BUFF_ID_PROC_CALL:
	  // Pop the procname and then the parameters
	  // Push the proc call

	  sprintf(newstr, "%s", be.name);
	  
	  for(int i=0; i<be.op.num_parameters; i++)
	    {
	      infix_stack_pop(op1);

	      strcat(newstr, "(");
	      strcat(newstr, op1);
	      if( i != be.op.num_parameters-1 )
		{
		  strcat(newstr, ", ");
		}

	      strcat(newstr, ")");
	    }
	  
	  infix_stack_push(newstr);
	  break;
	}
    }

  ff_fprintf(ofp, "\nDone\n");
  
  // There may not be a result if there was just a command
  result[0] = '\0';
  
  if( infix_stack_ptr != 0 )
    {
      infix_stack_pop(result);

      sprintf(newstr2, "%s", result);
      strcpy(newstr, newstr2);

      ff_fprintf(ofp, "\nInfix stack result %s", newstr);

    }
  else
    {
      ff_fprintf(ofp, "\nInfix stack empty");
    }

  // If the first character of the result is an open bracket then remove the brackets
  // that surround the result, they are redundant.
  char *res = result;
  
  if( *result == '(' )
    {
      res = result+1;
      result[strlen(result)-1] = '\0';
    }
  
  ff_fprintf(chkfp, "\n----------------------------------------infix-----------------------------------\n");
  ff_fprintf(chkfp, "\n%s\n", res);

  ff_fprintf(trfp, "\n%s", res);
  
  dbprintf("exit  '%s'", res);  
  return(result+1);
}

#endif

////////////////////////////////////////////////////////////////////////////////


void typecheck_operator_immutable(EXP_BUFFER_ENTRY be, OP_INFO op_info, EXP_BUFFER_ENTRY op1, EXP_BUFFER_ENTRY op2)
{
#if 0
  if( (op1.op.type ==  op_info.type[0]) )
    {
#endif 
      // Types correct, push a dummy result so we have a correct execution stack
      
      // Push dummy result if there is one
      if( op_info.returns_result )
	{
	  EXP_BUFFER_ENTRY res;
	  init_exp_buffer_entry(&res);
	  
	  res.node_id = be.node_id;          // Result id is that of the operator
	  res.p_idx = 2;
	  res.p[0] = op1.node_id;
	  res.p[1] = op2.node_id;
	  strcpy(res.name, "000");
	  
	  // Now set up output type
	  if( op_info.output_type == NOBJ_VARTYPE_UNKNOWN )
	    {
	      res.op.type      = op1.op.type;
	    }
	  else
	    {
	      res.op.type      = op_info.output_type;
	    }
	  
	  type_check_stack_push(res);
	}		    
#if 0
    }
  
  else
    {
      // Error
      ff_fprintf(ofp, "\nType of %s or %s is not %c", op1.name, op2.name, type_to_char(op_info.type[0]));
      internal_error("Type of %s or %s is not %c", op1.name, op2.name, type_to_char(op_info.type[0]));
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////
//
// Can autocon be used with these type?

struct _CAN_USE
{
  NOBJ_VARTYPE f;
  NOBJ_VARTYPE t;
}
  ft_types[] =
  {
    {NOBJ_VARTYPE_INT, NOBJ_VARTYPE_FLT},
    {NOBJ_VARTYPE_INT, NOBJ_VARTYPE_FLTARY},
    {NOBJ_VARTYPE_FLT, NOBJ_VARTYPE_INT},
    {NOBJ_VARTYPE_FLT, NOBJ_VARTYPE_INTARY},
    {NOBJ_VARTYPE_INTARY, NOBJ_VARTYPE_FLT},
    {NOBJ_VARTYPE_INTARY, NOBJ_VARTYPE_FLTARY},
    {NOBJ_VARTYPE_FLTARY, NOBJ_VARTYPE_INT},
    {NOBJ_VARTYPE_FLTARY, NOBJ_VARTYPE_INTARY},
  };

#define NUM_CAN_USE (sizeof(ft_types)/sizeof(struct _CAN_USE))

int can_use_autocon(NOBJ_VARTYPE from_type, NOBJ_VARTYPE to_type)
{
  for(int i=0; i<NUM_CAN_USE; i++)
    {
      if( (from_type == ft_types[i].f) && (to_type == ft_types[i].t))
	{
	  return(1);
	}
    }
  
  return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Take the expression buffer and process it as a syntax tree
//
// Tree-like information about the nodes in the RPN is built up, this allows
// processing of nodes to insert nodes based on node ID and not buffer index.
// The syntax processing is done as a separate step from the type checking
// as syntax tree processing may insert a node before the bnode currently being
// processed and that will be in a part of the tree that has already been typechecked.
//
// Rules:
//
// Only syntax nodes can be inserted by this function. No autocon nodes should
// be inserted here.
//
////////////////////////////////////////////////////////////////////////////////

void process_syntax_tree(void)
{
  EXP_BUFFER_ENTRY be;
  EXP_BUFFER_ENTRY autocon;
  EXP_BUFFER_ENTRY ft;
  OP_INFO          op_info;
  EXP_BUFFER_ENTRY op1;
  EXP_BUFFER_ENTRY op2;
  NOBJ_VARTYPE     op1_type, op2_type;
  NOBJ_VARTYPE     op1_reqtype, op2_reqtype;
  NOBJ_VARTYPE     ret_type;
  int              copied;
  NOBJ_VAR_INFO    *vi;
  char             t_name[NOBJ_VARNAME_MAXLEN+1];
  
  dbprintf("Pass:%d", pass_number);

  init_exp_buffer_entry(&autocon);
  
  // Initialise
  init_op_stack_entry(&(op1.op));
  init_op_stack_entry(&(op2.op));

  // We copy results over to a second buffer, this allows easy insertion of
  // needed extra codes

  exp_buffer2_i = 0;
  
  // We can check for an assignment and adjust the assignment token to
  // differentiate it from the equality token.

  type_check_stack_init();

  for(int i=0; i<exp_buffer_i; i++)
    {
      // Execute
      be = exp_buffer[i];
      copied = 0;

      // Give every entry a node id
      be.node_id = node_id_index++;

      
      dbprintf("*********Processing :%s   *************", be.name);
		  
      switch(be.op.buf_id)
	{
	  // Not used
	case EXP_BUFF_ID_TKN:
	  break;

	  // No type, marker
	case EXP_BUFF_ID_SUB_START:
	  break;

	  // No type, marker
	case EXP_BUFF_ID_SUB_END:
	  break;

	case EXP_BUFF_ID_META:
	  if( (strcmp(be.op.name, " OPEN") == 0) || (strcmp(be.op.name, " CREATE") == 0) )
	    {
	      // Pop file name off stack
	      op1 = type_check_stack_pop();
	      type_check_stack_push(be);	      
	    }
	  
	  if( (strcmp(be.op.name, "ENDFIELDS") == 0) )
	    {
	      // We need to unstack the arguments to this command

	      // File name
	      op1 = type_check_stack_pop();

	      // Logical file name
	      op1 = type_check_stack_pop();

	      do
		{
		  op1 = type_check_stack_pop();
		}
	      while( !((strcmp(op1.name, " CREATE") == 0) || (strcmp(op1.name, " OPEN") == 0)) );
	    }

	  // PAR_TYPE is used to insert a stacking of the parameter types of
	  // PROCs just before a PROC_CALL. Here we need to set the type of the
	  // PAR_TYPE so the correct type code can be stacked in the qcode.
	  
	  if( (strcmp(be.op.name, "PAR_TYPE") == 0) )
	    {
	      // We need to unstack the parameter, find th etyope, update the
	      // PAR_TYPE type and re-stack the parameter

	      // Parameter
	      op1 = type_check_stack_pop();

	      // set type
	      be.op.type = op1.op.type;

	      // Set node id
	      be.node_id = op1.node_id;

	      // Re-stack the parameter
	      type_check_stack_push(op1);
	    }

	  // Percent is an operator that is after an expression. We set a flag in the
	  // popped entry so that the opertors that can be % versions (+,-,*,/,<,>)
	  // can see that they have a percent argument and change into the percent
	  // version
	  
	  if( strcmp(be.op.name, "PERCENT") == 0 )
	    {
	      // Pop the argument
	      op1 = type_check_stack_pop();

	      // Set the percent flag in the syntax tree
	        EXP_BUFFER_ENTRY *e = find_buf2_entry_with_node_id(op1.node_id);

		  if( e != NULL )
		    {
		      // Set the flag in the EXP buffer entry and the op that is pushed back on the stack.
		      e->op.percent = 1;
		      op1.op.percent = 1;
		    }
		  else
		    {
		      internal_error("Could not find entry for N%03d", exp_buffer2[i].p[0]);
		    }
		  
	      // Put it back
	      type_check_stack_push(op1);
	    }
	  break;

	case EXP_BUFF_ID_VARIABLE:
	  // If the variable is an array then we need to pop the index
	  // The index is an integer, we need to convert from a float if its not an int

	  if( pass_number == 2,1 )
	    {
	      // If the variable is a field then we don't look for it in the var info
	      if( be.name[1] == '.' )
		{
		}
	      
	      vi = find_var_info(be.name, be.op.type);

	      if( vi == NULL )
		{
		  dbprintf("\nCould not find variable '%s'", be.name);
		  return;
		}
	      
	      if( var_type_is_array(vi->type) )
		{
		  dbprintf(" Array type, checking index");
		  op1 = type_check_stack_pop();
		}
	    }
      
	  be.p_idx = 0;
	  type_check_stack_push(be);
	  break;

	case EXP_BUFF_ID_VAR_ADDR_NAME:
	  be.p_idx = 0;
	  type_check_stack_push(be);
	  break; 

	  // Field variable name
	case EXP_BUFF_ID_LOGICALFILE:
	case EXP_BUFF_ID_FIELDVAR:
	  be.p_idx = 0;
	  type_check_stack_push(be);
	  break;

	case EXP_BUFF_ID_RETURN:

	  // the RETURN may or may not have an expression after it
	  
	  if( be.op.access == NOPL_OP_ACCESS_EXP )
	    {
	      // Pop the return value off the stack
	      op1 = type_check_stack_pop();
	    }
	  else
	    {
	      //No expression, so we will use a 'null' return qcode
	    }
	  break;
	  
	  // These need to pop a value off the stack to keep the stack
	  // correct for cleaning up at the end with a drop code.
	case EXP_BUFF_ID_IF:
	case EXP_BUFF_ID_ELSEIF:
	case EXP_BUFF_ID_WHILE:
	case EXP_BUFF_ID_UNTIL:
	  dbprintf("%d args", function_num_args(be.name));
	  op1 = type_check_stack_pop();
	  break;
	  
	case EXP_BUFF_ID_PRINT:
	  dbprintf("PRINT type adjust", function_num_args(be.name));
	  op1 = type_check_stack_pop();
	  be.op.type = op1.op.type;
	  break;

	case EXP_BUFF_ID_LPRINT:
	  dbprintf("LPRINT type adjust", function_num_args(be.name));
	  op1 = type_check_stack_pop();
	  be.op.type = op1.op.type;
	  break;

	case EXP_BUFF_ID_INPUT:
	  // Pop and discard the input argument
	  op1 = type_check_stack_pop();
	  break;
	  
	case EXP_BUFF_ID_ENDIF:
	case EXP_BUFF_ID_GOTO:
	case EXP_BUFF_ID_ENDWH:
	  break;
	  
	case EXP_BUFF_ID_FLT:
	case EXP_BUFF_ID_INTEGER:
	case EXP_BUFF_ID_STR:
	  be.p_idx = 0;
	  type_check_stack_push(be);
	  break;

	case EXP_BUFF_ID_FUNCTION:
	  // Functions also require certain types, for instance USR reuires
	  // all integers. Any floats in the arguments require conversion codes.
	  
	  // Set up the function return value
	  ret_type = function_return_type(be.name);

	  ff_fprintf(ofp, "\nret_type;%d %c", ret_type, type_to_char(ret_type));
	  ff_fprintf(ofp, "\n%s:Ret type of %s : %c", __FUNCTION__, be.name, type_to_char(ret_type));

	  
	  // Build an argument list (constructs part of the syntax tree)
	  // If the num_parameters field is zero then use the table to get the number of
	  // arguments to the function
	  int num_args = be.op.num_parameters;

	  if( num_args == 0 )
	    {
	      num_args = function_num_args(be.name);
	    }

	  dbprintf("%d args", num_args);

	  // We have some special functions that use the Flist argument format. This is
	  // either Array, Int
	  // or
	  // a list of floats
	  //
	  // We know how many arguments the function has so we can test the arguments here to work out which
	  // form the args are using.

	  int flist_type = 0;
	  
	  if( function_arg_parse(be.name) == 'L' )
	    {
	      // Flist parsing of arguments
	      // If there are two arguments and the second one is an array then it's type 0
	      dbprintf("Flist type args");
	      
	      if( num_args == 2 )
		{
		  // Get the two args
		  op1 = type_check_stack_pop();
		  op2 = type_check_stack_pop();

		  // Put the args back
		  type_check_stack_push(op2);
		  type_check_stack_push(op1);
		  
		  if( op2.op.type == NOBJ_VARTYPE_FLTARY )
		    {
		      flist_type = 0;
		    }
		  else
		    {
		      flist_type = 1;
		    }
		}
	      else
		{
		  // Flist type 1
		  // A list that should all be floats
		  flist_type = 1;
		}

	      // Now re-arrange the arguments and also add autocons if needed

	      
	      // We also push an int indicating the flist type
	      switch(flist_type)
		{
		case 0:
		  // A reference to the first element of an array followed by an integer that is the number of
		  // elements to use

		  // Get the args
		  op1 = type_check_stack_pop();
		  op2 = type_check_stack_pop();

		  // Check the types
		  if( (op2.op.type == NOBJ_VARTYPE_FLTARY) &&
		      ((op1.op.type == NOBJ_VARTYPE_INT) || (op1.op.type == NOBJ_VARTYPE_FLT)) )
		    {
		      // All OK
		      // Build args to RTF

		      // The array variable needs to be a reference
		      set_node_access(op2.node_id, NOPL_OP_ACCESS_WRITE);
		      
		      // flist type pushed later
		    }
		  break;

		case 1:
		  // A list of floats.
		  // We know how many as the parser provided that information,
		  // so we check all are floats, or autocon any that aren't floats
		  dbprintf("flist type 1. Num args: %d", be.op.num_parameters);

		  for(int i=0; i<be.op.num_parameters; i++)
		    {
		      op1 = type_check_stack_pop();
		    }

		  // Insert a byte which is the number of arguments
		  init_op_stack_entry(&(ft.op));

		  ft.node_id = node_id_index++;
		  ft.op.buf_id = EXP_BUFF_ID_BYTE;
		  ft.p_idx = 1;
		  ft.p[0] = be.node_id;
		  sprintf(t_name, "%d", be.op.num_parameters);
		  strcpy(ft.name, t_name);
		  strcpy(ft.op.name, t_name);
		  ft.op.type      = NOBJ_VARTYPE_INT;

		  // Function is going to be processed next, so we just add this INT to the end of the buffer
		  if( exp_buffer2_i >= (MAX_EXP_BUFFER-1) )
		    {
		      internal_error("exp buffer 2 overflow numargs");
		      return;
		    }

		  exp_buffer2[exp_buffer2_i++] = ft;
		  //		  insert_buf2_entry_after_node_id(be.node_id, ft);
		  
		  break;
		}

	      // Now push the flist type
	      
	      if( exp_buffer2_i >= (MAX_EXP_BUFFER-1) )
		{
		  internal_error("exp buffer 2 overflow numargs");
		  return;
		}

	      init_op_stack_entry(&(ft.op));
	      
	      ft.node_id = node_id_index++;
	      ft.op.buf_id = EXP_BUFF_ID_BYTE;
	      ft.p_idx = 1;
	      ft.p[0] = be.node_id;
	      sprintf(t_name, "%d", flist_type);
	      strcpy(ft.name, t_name);
	      strcpy(ft.op.name, t_name);
	      ft.op.type      = NOBJ_VARTYPE_INT;

	      // Function is going to be processed next, so we just add this INT to the end of the buffer
	      exp_buffer2[exp_buffer2_i++] = ft;
	      //insert_buf2_entry_before_node_id(be.node_id, ft);

	      //type_check_stack_push(ft);
	    }
	  else
	    {
	      // Normal parsing of arguments
	      be.p_idx = 0;
	      for(int i=num_args-1; i>=0; i--)
		{
		  NOBJ_VARTYPE this_arg_type = function_arg_type_n(be.name, i);
	      
		  // Pop an argument off and check it
		  op1 = type_check_stack_pop();

		  // Force the args to write if needed
		  if( function_access_force_write(be.name) )
		    {
		      dbprintf("Forced arg access to write");
		  
		      set_node_access(op1.node_id, NOPL_OP_ACCESS_WRITE);
		    }
	     
		  // Add to list of arguments
		  if( be.p_idx >= ( MAX_EXP_BUF_P-1) )
		    {
		      internal_error("Argument list full");
		      return;
		    }
		  
		  be.p[be.p_idx++] = op1.node_id;
	      
		  dbprintf("FN ARG %d type:%c %s %d(%c)", i,
			   type_to_char(this_arg_type),
			   op1.name,
			   op1.op.type,
			   type_to_char(op1.op.type));
		}
	    }
	  
	  // Only push a result if the function is non-void
	  if( ret_type != NOBJ_VARTYPE_VOID )
	    {
	      // Push dummy result
	      EXP_BUFFER_ENTRY res;
	      init_exp_buffer_entry(&res);
	      
	      res.node_id = be.node_id;          // Result id is that of the operator
	      res.p_idx = function_num_args(be.name);
	      res.p[0] = op1.node_id;
	      res.p[1] = op2.node_id;
	      strcpy(res.name, "000");
	      res.op.type      = ret_type;
	      type_check_stack_push(res);
	    }

	  // The ADDR function is a bit odd. It needs to have a reference to its argument
	  // and also needs to be a string type if its argument is a string
	  // as there's a different QCode for ADDR(string)

	  if( strcmp(be.name, "ADDR")==0 )
	    {
	      // Make argument a reference (parser tries, but this is a better way)
	      EXP_BUFFER_ENTRY *a = find_buf2_entry_with_node_id(be.p[0]);
	      a->op.access = NOPL_OP_ACCESS_WRITE;
	    }

	  // The return type opf the function is known
	  be.op.type = ret_type;
	  break;

	case EXP_BUFF_ID_PROC_CALL:
	  // Procedure calls are like functions, except that no auto conversion of parameters is done.
	  // Mismatched parameter types cause a run time error
	  
	  ff_fprintf(ofp, "\nPROC CALL: %d parameters", be.op.num_parameters);
	  
	  // Set up the function return value
	  ret_type = be.op.type;

	  dbprintf("Ret type of %s : %c", be.name, type_to_char(ret_type));
	  
	  // Build an argument list (constructs part of the syntax tree)
	  be.p_idx = 0;
	  for(int i=be.op.num_parameters-1; i>=0; i--)
	    {
	      // Pop a parameter off
	      op1 = type_check_stack_pop();

	      if( be.p_idx >= ( MAX_EXP_BUF_P-1) )
		{
		  internal_error("Argument list full");
		  return;
		}

	      // Add to list of arguments
	      be.p[be.p_idx++] = op1.node_id;
	      be.op.parameter_type[i] = op1.op.type;
	      
	      dbprintf("PROC PAR %d %s Type:%d(%c)",
		       i,
		       op1.name,
		       op1.op.type,
		       type_to_char(op1.op.type));
	    }
	  
	  // Only push a result if the function is non-void
	  if( ret_type != NOBJ_VARTYPE_VOID )
	    {
	      // Push dummy result
	      EXP_BUFFER_ENTRY res;
	      init_exp_buffer_entry(&res);
	      
	      res.node_id = be.node_id;          // Result id is that of the operator
	      res.p_idx = be.op.num_parameters;
	      res.p[0] = op1.node_id;
	      res.p[1] = op2.node_id;
	      strcpy(res.name, "000");
	      res.op.type      = ret_type;
	      type_check_stack_push(res);
	    }
	  
	  // The return type opf the function is known
	  be.op.type = ret_type;
	  break;

	  //------------------------------------------------------------------------------
	  //
	  // Operators have to be typed correctly depending on their
	  // operands. Some of them are mutable (polymorphic) and we have to bind them to their
	  // type here.
	  // Some are immutable and cause errors if their operators are not correct
	  // Some have a fixed output type (>= for example, but still have mutable inputs)
	  // The assignment operator type is determined by the variable being assigned to
	  
	case EXP_BUFF_ID_OPERATOR:
	  // Check that the operands are correct, i.e. all of them are the same and in
	  // the list of acceptable types
	  dbprintf("BUFF_ID_OPERATOR");

	  if( find_op_info(be.name, &op_info) )
	    {
	      dbprintf("Found operator %s %%conv:%d", be.name, op_info.percent_convertible);

	      // We only handle binary operators here
	      // Pop arguments off stack, this is an analogue of execution of the operator
	      
	      op1 = type_check_stack_pop();
	      op2 = type_check_stack_pop();

	      op1_type = op1.op.type;
	      op2_type = op2.op.type;

	      dbprintf("op1 type:%c op2 type:%c %%conv:%d", type_to_char(op1.op.type), type_to_char(op2.op.type), op_info.percent_convertible);
	      
	      // The percentage operators need to be created here. Only some operators convert here
	      // and only if the first argument to the operator has its percent flag set?
	      if( op_info.percent_convertible )
		{
		  dbprintf("Percent convertible operator op1 (N%03d) percent:%d", op1.node_id, op1.op.percent);
		  if( op1.op.percent )
		    {
		      dbprintf("Converting to percent operator");
		      
		      // Convert to percentage operator
		      strcat(be.name,    "%");
		      strcat(be.op.name, "%");
		      be.op.qcode_type = op_info.output_type;
		    }
		}
	      
	      // Get the node ids of the argumenmts so we can find them if we need to
	      // adjust them.
	      
	      be.p_idx = 2;
	      be.p[0] = op1.node_id;
	      be.p[1] = op2.node_id;
	      
	      // Check all operands are of correct type.
	      if( op_info.immutable )
		{
		  dbprintf("Immutable type");
		  
		  // Immutable types for this operator so we don't do any
		  // auto conversion here. Just check that the correct type
		  // is present, if not, it's an error
		  
		  typecheck_operator_immutable(be, op_info, op1, op2);
		}
	      else
		{
		  // Mutable type is dependent on the arguments, e.g.
		  //  A$ = "RTY"
		  // requires that a string equality is used, similarly
		  // INT and FLT need the correctly typed operator.
		  //
		  // INT and FLT have an additional requirement where INT is used
		  // as long as possible, and also assignment can turn FLT into INT
		  // or INT into FLT
		  // For our purposes here, arrays are the same as their element type
		  
		  dbprintf("Mutable type (%s) %c %c", op1.name, type_to_char(op1.op.type), type_to_char(op2.op.type));
		  
		  // Check input types are valid for this operator
		  if( is_a_valid_type(op1.op.type, &op_info) && is_a_valid_type(op2.op.type, &op_info))
		    {
		      // We have types here. We need to insert auto type conversion qcodes here
		      // if needed.
		      //
		      // Int -> float if float required
		      // Float -> int if int required
		      // Expressions start as integer and turn into float if a float is found.
		      //

		      // If the operator result type is unknown then we use the types of the arguments
		      // Unknown types arise when brackets are used.
		      if( be.op.type == NOBJ_VARTYPE_UNKNOWN)
			{
			  be.op.type = type_with_least_conversion_from(op1.op.type, op2.op.type);
			}

		      if( (be.op.type == NOBJ_VARTYPE_INT) || (be.op.type == NOBJ_VARTYPE_INTARY))
			{
			  be.op.type = type_with_least_conversion_from(op1.op.type, op2.op.type);
			}

		      // Types are both OK
		      // If they are the same then we will bind the operator type to that type
		      // as long as they are both the required type, if not then if types aren't
		      // INT or FLT then it's an error
		      // INT or FLT can be auto converted to the required type
		      
		      if( types_identical(op1.op.type, op2.op.type) )
			{
			  dbprintf("Same type");

			  // The input types of the operands are the same as the required type, all ok
			  be.op.type = op1.op.type;
			  
			  // Now set up output type
			  if( op_info.output_type == NOBJ_VARTYPE_UNKNOWN )
			    {
			      // Do not force
			    }
			  else
			    {
			      dbprintf("(A) Forced type to %c", type_to_char(op1.op.type));
			      be.op.type      = op_info.output_type;
			      be.op.qcode_type = op1.op.type;
			    }
			}
		      else
			{
			  dbprintf(" Autoconversion");
			  dbprintf(" --------------");
			  dbprintf(" Op1: type:%c", type_to_char(op1.op.type));
			  dbprintf(" Op2: type:%c", type_to_char(op2.op.type));
			  dbprintf(" BE:  type:%c",  type_to_char(be.op.type));
		 
			  // We insert auto conversion nodes to force the type of the arguments to match the
			  // operator type. For INT and FLT we can force the operator to FLT if required
			  // Do that before inserting auto conversion nodes.
			  // Special treatment for assignment operator
			  if( op_info.assignment )
			    {
			      dbprintf("Assignment");
			      
			      // Operator type follows the second operand, which is the variable we
			      // are assigning to

			      // Now set up output type
			      if( op_info.output_type == NOBJ_VARTYPE_UNKNOWN )
				{
				  // Do not force
				}
			      else
				{
				  dbprintf("(C) Forced type to %c", type_to_char(op2.op.type));
				  be.op.type       = op_info.output_type;
				  be.op.qcode_type = op2.op.type;
				}

			      be.op.type = op2.op.type;

			      // Now set up output type
			      if( op_info.output_type == NOBJ_VARTYPE_UNKNOWN )
				{
				  // Do not force
				}
			      else
				{
				  dbprintf("(D) Forced type to %c", type_to_char(op_info.output_type));
				  be.op.type      = op_info.output_type;
				}
			    }
			  else
			    {
			      // Must be INT/FLT or FLT/INT
			      if( (op1.op.type == NOBJ_VARTYPE_FLT) || (op2.op.type == NOBJ_VARTYPE_FLT) )
				{
				  // Force operator to FLT
				  be.op.type = NOBJ_VARTYPE_FLT;
				}
			      else if( (op1.op.type == NOBJ_VARTYPE_FLTARY) || (op2.op.type == NOBJ_VARTYPE_FLTARY) )
				{
				  // Force operator to FLT
				  be.op.type = NOBJ_VARTYPE_FLT;
				}
			    }

			  if( !op_info.assignment )
			    {
			      // If type of operator is unknown, 
			      
			      // Now set up output type
			      if( op_info.output_type == NOBJ_VARTYPE_UNKNOWN )
				{
				  // Do not force
				}
			      else
				{
				  dbprintf("(D) Forced type to %c", type_to_char(op_info.output_type));
				  be.op.type      = op_info.output_type;
				}
			    }
			}
		      
		      if( op_info.returns_result )
			{
			  EXP_BUFFER_ENTRY res;
			  init_exp_buffer_entry(&res);
			  
			  strcpy(res.name, "000");
			  res.node_id = be.node_id;   //Dummy result carries the operator node id as that is the tree node
			  res.p_idx = 2;
			  res.p[0] = op1.node_id;
			  res.p[1] = op2.node_id;
			  res.op.type      = be.op.type;
			  type_check_stack_push(res);
			}
		    }
		  else
		    {
		      // Unknown required types exist, this probably shoudn't happen is a syntax error
		      dbprintf("Syntax error at node N%d, unknown required type", be.node_id);
		      type_check_stack_display();

		      dump_exp_buffer(ofp, 1);
		      internal_error("Syntax error at node N%d, unknown required type", be.node_id);
		      //exit(-1);
		    }
		}		
	    }
	  else
	    {
	      // Error, not found
	    }
	  break;

	case EXP_BUFF_ID_OPERATOR_UNARY:
	  dbprintf("BUFF_ID_OPERATOR_UNARY");
	  
	  if( find_op_info(be.name, &op_info) )
	    {
	      dbprintf("Found unary operator %s", be.name);

	      // Unary operators don't change the type, just the value
	      op1 = type_check_stack_pop();
	      op1_type = op1.op.type;

	      dbprintf("op1 type:%c", type_to_char(op1.op.type));

	      // Set the copied entry to the same type
	      be.op.type = op1.op.type;

	      // Create a link to the argument.
	      be.p_idx = 1;
	      be.p[0] = op1.node_id;
	      
	      // Push result
	      EXP_BUFFER_ENTRY res;

	      init_exp_buffer_entry(&res);
	      
	      strcpy(res.name, "000");
	      res.node_id = be.node_id;   //Dummy result carries the operator node id as that is the tree node
	      res.p_idx = 1;
	      res.p[0] = op1.node_id;
	      res.op.type      = be.op.type;
	      type_check_stack_push(res);

	    }
	  else
	    {
	      dbprintf("Unkown unary operator '%s'", be.name);
	      syntax_error("Unkown unary operator '%s'", be.name);
	    }
	  break;
	  
	default:
	  ff_fprintf(ofp, "\ndefault buf_id");
	  break;
	}
      
      // If entry not copied over, copy it
      if( !copied )
	{
	  if( exp_buffer2_i >= (MAX_EXP_BUFFER-1) )
	    {
	      internal_error("exp buffer 2 overflow (insert)");
	      return;
	    }
	  
	  exp_buffer2[exp_buffer2_i++] = be;
	}

      type_check_stack_display();
    }

  // Do we have a value stacked that isn't going to be used?
  if( (pass_number == 2) && (type_check_stack_ptr > 0) )
    {
      // If we only have logical file names or field variables then we ignore those
      if( type_check_stack_only_field_data() )
	{
	  dbprintf("Only field data left, so no DROP needed");
	}
      else
	{
	  dbprintf("Value left stacked so DROP needed");
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Take the expression buffer and execute it for types
//
// Tree-like information about the nodes in the RPN is present, this allows
// auto conversion tokens to be inserted higher up the tree when needed.
// Auto conversion is only inserted to convert inputs to the required type, it
// isn't used on the output or result of an operator or function. This is to
// avoid double application of conversion tokens.
//
// Using a function that leaves an unused stacked value can be detected,
//   e.g.  GET
// That caused a drop to be added
//
// Rules:
//
// Only autocon nodes can be inserted by this function
//
////////////////////////////////////////////////////////////////////////////////

void typecheck_expression(void)
{
  EXP_BUFFER_ENTRY be;
  EXP_BUFFER_ENTRY autocon;
  EXP_BUFFER_ENTRY ft;
  OP_INFO          op_info;
  EXP_BUFFER_ENTRY op1;
  EXP_BUFFER_ENTRY op2;
  EXP_BUFFER_ENTRY opt;
  NOBJ_VARTYPE     op1_type, op2_type;
  NOBJ_VARTYPE     op1_reqtype, op2_reqtype;
  NOBJ_VARTYPE     ret_type;
  NOBJ_VARTYPE     last_known_type = NOBJ_VARTYPE_UNKNOWN;
  int              copied;
  NOBJ_VAR_INFO    *vi;
  char             t_name[NOBJ_VARNAME_MAXLEN+1];
  
  dbprintf("Pass:%d", pass_number);
  
  // Initialise
  init_op_stack_entry(&(autocon.op));
  init_op_stack_entry(&(op1.op));
  init_op_stack_entry(&(op2.op));

  
  // We copy results over to a second buffer, this allows easy insertion of
  // needed extra codes

  exp_buffer2_i = 0;
  
  // We can check for an assignment and adjust the assignment token to
  // differentiate it from the equality token.

  type_check_stack_init();

  ff_fprintf(exfp, "\n====================\n");
  
  for(int i=0; i<exp_buffer_i; i++)
    {
      // Execute
      be = exp_buffer[i];
      copied = 0;
      
#if 0      
      // Give every entry a node id
      be.node_id = node_id_index++;
#endif

      if( be.op.type != NOBJ_VARTYPE_UNKNOWN )
	{
	  last_known_type = be.op.type;
	}

      dbprintf(" *** BE:%s    **********", be.name);

      ff_fprintf(exfp, "\n%s %s", exp_buffer_id_str[be.op.buf_id], be.name);
      
      switch(be.op.buf_id)
	{
	  // Not used
	case EXP_BUFF_ID_TKN:
	  break;

	  // No type, marker
	case EXP_BUFF_ID_SUB_START:
	  break;

	  // No type, marker
	case EXP_BUFF_ID_SUB_END:
	  break;

	case EXP_BUFF_ID_META:
	  if( (strcmp(be.op.name, " OPEN") == 0) || (strcmp(be.op.name, " CREATE") == 0) )
	    {
	      // Pop file name off stack
	      op1 = type_check_stack_pop();
	      type_check_stack_push(be);	      
	    }
	  
	  if( (strcmp(be.op.name, "ENDFIELDS") == 0) )
	    {
	      // We need to unstack the arguments to this command

	      // File name
	      op1 = type_check_stack_pop();

	      // Logical file name
	      op1 = type_check_stack_pop();

	      do
		{
		  op1 = type_check_stack_pop();
		}
	      while( !((strcmp(op1.name, " CREATE") == 0) || (strcmp(op1.name, " OPEN") == 0)) );
	    }

	  // PAR_TYPE is used to insert a stacking of the parameter types of
	  // PROCs just before a PROC_CALL. Here we need to set the type of the
	  // PAR_TYPE so the correct type code can be stacked in the qcode.
	  
	  if( (strcmp(be.op.name, "PAR_TYPE") == 0) )
	    {
	      // We need to unstack the parameter, find the type, update the
	      // PAR_TYPE type and re-stack the parameter

	      // Parameter
	      op1 = type_check_stack_pop();

	      // set type
	      be.op.type = op1.op.type;

	      // Set node id
	      be.node_id = op1.node_id;

	      // Re-stack the parameter
	      type_check_stack_push(op1);
	    }
	  
	  break;

	case EXP_BUFF_ID_VARIABLE:
	  // If the variable is an array then we need to pop the index
	  // The index is an integer, we need to convert from a float if its not an int

	  if( pass_number == 2,1 )
	    {
	      // If the variable is a field then we don't look for it in the var info
	      if( be.name[1] == '.' )
		{
		}
	      
	      vi = find_var_info(be.name, be.op.type);

	      if( vi == NULL )
		{
		  dbprintf("\nCould not find variable '%s'", be.name);
		  return;
		}
	      
	      if( var_type_is_array(vi->type) )
		{
		  op1 = type_check_stack_pop();

		  dbprintf(" Array type, checking index");
		  
		  // If index isn't an INT then see if we can type convert it.
		  if( op1.op.type == NOBJ_VARTYPE_INT)
		    {
		      dbprintf("  Index ok");
		      // All OK
		    }
		  else
		    {
		      dbprintf("  Index not OK");
		      
		      sprintf(autocon.name, "autocon (index) %c->%c", type_to_char(op1.op.type), type_to_char(NOBJ_VARTYPE_INT));
		      
		      // Can we use an auto conversion?
		      // There's only one option, FLT -> INT
		      if( (op1.op.type == NOBJ_VARTYPE_FLT) )
			{
			  autocon.op.buf_id = EXP_BUFF_ID_AUTOCON;
			  
			  autocon.p_idx = 0;
			  autocon.op.type      = NOBJ_VARTYPE_INT;
			  //autocon.op.req_type  = NOBJ_VARTYPE_INT;
			  
			  insert_buf2_entry_after_node_id(op1.node_id, autocon);
			}
		    }
		}
	    }
      
	  be.p_idx = 0;
	  type_check_stack_push(be);
	  break;

	case EXP_BUFF_ID_VAR_ADDR_NAME:
	  be.p_idx = 0;
	  type_check_stack_push(be);
	  break; 

	  // Field variable name
	case EXP_BUFF_ID_LOGICALFILE:
	case EXP_BUFF_ID_FIELDVAR:
	  be.p_idx = 0;
	  type_check_stack_push(be);
	  break;

	case EXP_BUFF_ID_RETURN:

	  // the RETURN may or may not have an expression after it
	  
	  if( be.op.access == NOPL_OP_ACCESS_EXP )
	    {
	      // Pop the return value off the stack
	      op1 = type_check_stack_pop();
	      
	      // We may need to auto convert if the value we popped doesn't match the return
	      // type of the proc, which is the same as the type of the return
	      if( (op1.op.type != be.op.type) )
		{
		  // We have a type error or need to autocon
		  autocon.op.buf_id = EXP_BUFF_ID_AUTOCON;
		  autocon.op.type   = be.op.type;
		  autocon.p_idx     = 0;
		  ff_fprintf(ofp, "  Return type not OK");
		  
		  sprintf(autocon.name, "autocon %c->%c (return)", type_to_char(op1.op.type), type_to_char(be.op.type));

		  if( ( can_use_autocon(op1.op.type, be.op.type)))
		    {
		      insert_buf2_entry_after_node_id(op1.node_id, autocon);
		    }
		  //		  insert_buf2_entry_after_node_id(op1.node_id, autocon);
		  
		}
	    }
	  else
	    {
	      //No expression, so we will use a 'null' return qcode
	    }
	  break;
	  
	  // These need to pop a value off the stack to keep the stack
	  // correct for cleaning up at the end with a drop code.
	  // As they test an integer, if the argument is a float we add some code to get an integer
	  // from the float
	case EXP_BUFF_ID_IF:
	case EXP_BUFF_ID_ELSEIF:
	case EXP_BUFF_ID_WHILE:
	case EXP_BUFF_ID_UNTIL:
	  dbprintf("%d args", function_num_args(be.name));
	  op1 = type_check_stack_pop();

	  EXP_BUFFER_ENTRY fconv;
	  
	  switch(op1.op.type)
	    {
	    case NOBJ_VARTYPE_FLT:
	    case NOBJ_VARTYPE_FLTARY:

	      init_exp_buffer_entry(&fconv);
	      
	      strcpy(fconv.name, "<>");
	      strcpy(fconv.op.name, "<>");
	      fconv.op.buf_id = EXP_BUFF_ID_OPERATOR;
	      insert_buf2_entry_after_node_id(op1.node_id, fconv);
	      strcpy(fconv.name, "0.0");
	      strcpy(fconv.op.name, "0.0");
	      fconv.op.buf_id = EXP_BUFF_ID_FLT;
	      insert_buf2_entry_after_node_id(op1.node_id, fconv);

	      break;
	    }
	  break;
	  
	case EXP_BUFF_ID_PRINT:
	  dbprintf("PRINT type adjust", function_num_args(be.name));
	  op1 = type_check_stack_pop();
	  be.op.type = op1.op.type;
	  break;

	case EXP_BUFF_ID_LPRINT:
	  dbprintf("LPRINT type adjust", function_num_args(be.name));
	  op1 = type_check_stack_pop();
	  be.op.type = op1.op.type;
	  break;

	case EXP_BUFF_ID_INPUT:
	  // Pop and discard the input argument
	  op1 = type_check_stack_pop();
	  break;
	  
	case EXP_BUFF_ID_ENDIF:
	case EXP_BUFF_ID_GOTO:
	case EXP_BUFF_ID_ENDWH:
	  break;

	case EXP_BUFF_ID_BYTE:
	case EXP_BUFF_ID_FLT:
	case EXP_BUFF_ID_INTEGER:
	case EXP_BUFF_ID_STR:
	  be.p_idx = 0;
	  type_check_stack_push(be);
	  break;

	case EXP_BUFF_ID_FUNCTION:
	  // Functions also require certain types, for instance USR reuires
	  // all integers. Any floats in the arguments require conversion codes.
	  
	  // Set up the function return value
	  ret_type = function_return_type(be.name);

	  ff_fprintf(ofp, "\nret_type;%d %c", ret_type, type_to_char(ret_type));
	  ff_fprintf(ofp, "\n%s:Ret type of %s : %c", __FUNCTION__, be.name, type_to_char(ret_type));
	  
	  // Now insert auto convert nodes if required

	  autocon.op.buf_id = EXP_BUFF_ID_AUTOCON;
	  autocon.p_idx = 0;

	  // Now check that all arguments have the correct type or
	  // can with an auto type conversion

	  // Build an argument list (constructs part of the syntax tree)
	  // If the num_parameters field is zero then use the table to get the number of
	  // arguments to the function
	  int num_args = be.op.num_parameters;

	  if( num_args == 0 )
	    {
	      num_args = function_num_args(be.name);
	    }

	  dbprintf("%d args", num_args);

	  // We have some special functions that use the Flist argument format. This is
	  // either Array, Int
	  // or
	  // a list of floats
	  //
	  // We know how many arguments the function has so we can test the arguments here to work out which
	  // form the args are using.

	  int flist_type = 0;
	  
	  if( function_arg_parse(be.name) == 'L' )
	    {
	      // Flist parsing of arguments
	      // If there are two arguments and the second one is an array then it's type 0
	      dbprintf("Flist type args");

#if 0
	      if( num_args == 2 )
		{

		  // Get the flist type byte
		  opt = type_check_stack_pop();
		  
		  // Get the two args
		  op1 = type_check_stack_pop();
		  op2 = type_check_stack_pop();

		  // Put the args back
		  //type_check_stack_push(op2);
		  //type_check_stack_push(op1);
		  //type_check_stack_push(opt);
		  
		  if( op2.op.type == NOBJ_VARTYPE_FLTARY )
		    {
		      flist_type = 0;
		    }
		  else
		    {
		      flist_type = 1;
		    }
		}
	      else
		{
		  // Flist type 1
		  // A list that should all be floats
		  flist_type = 1;
		}
#endif
	      // pop the flist type
	      opt = type_check_stack_pop();

	      sscanf(opt.op.name, "%d", &flist_type);

	      dbprintf("Flist type:%d", flist_type);
	      
	      // Now re-arrange the arguments and also add autocons if needed
	      
	      // We also push an int indicating the flist type
	      switch(flist_type)
		{
		case 0:
		  // A reference to the first element of an array followed by an integer that is the number of
		  // elements to use

		  // get flist type byte
		  //		  opt = type_check_stack_pop();
		  
		  // Get the args
		  op1 = type_check_stack_pop();
		  op2 = type_check_stack_pop();

		  be.p_idx = 2;
		  be.p[0] = op1.node_id;
		  be.p[1] = op2.node_id;

		  // Check the types
		  if( (op2.op.type == NOBJ_VARTYPE_FLTARY) &&
		      ((op1.op.type == NOBJ_VARTYPE_INT) || (op1.op.type == NOBJ_VARTYPE_FLT)) )
		    {
		      // All OK
		      dbprintf("Arg types OK");
		      
		      // Build args to RTF

		      // Array needs an index of 1

		      init_op_stack_entry(&(ft.op));
		      
		      ft.node_id = node_id_index++;
		      ft.op.buf_id = EXP_BUFF_ID_INTEGER;
		      ft.p_idx = 1;
		      ft.p[0] = be.node_id;
		      strcpy(ft.name, "1");
		      strcpy(ft.op.name, "1");
		      ft.op.type      = NOBJ_VARTYPE_INT;
		      //		      insert_buf2_entry_before_node_id(op2.node_id, ft);

		      //type_check_stack_push(ft);
		      //type_check_stack_push(op2);
		      //type_check_stack_push(op1);
		      
		      if( ( can_use_autocon(op1.op.type, NOBJ_VARTYPE_INT)))
			{
			  dbprintf("Autocon added");
			  
			  sprintf(autocon.name, "autocon %c->%c (flist idx)", type_to_char(op1.op.type), type_to_char(NOBJ_VARTYPE_INT));
			  
			  autocon.op.buf_id = EXP_BUFF_ID_AUTOCON;
			  autocon.p_idx = 1;
			  autocon.p[0] = op1.node_id;
			  autocon.op.type      = NOBJ_VARTYPE_INT;
			  
			  insert_buf2_entry_after_node_id(op1.node_id, autocon);
			}

		      // The array variable needs to be a reference
		      set_node_access(op2.node_id, NOPL_OP_ACCESS_WRITE);
		      
		      // flist type pushed later
		    }
		  break;

		case 1:
		  // A list of floats.
		  // We know how many as the parser provided that information,
		  // so we check all are floats, or autocon any that aren't floats
		  dbprintf("flist type 1. Num args: %d", be.op.num_parameters);

		  // Get the number of arguments 
		  op2 = type_check_stack_pop();
		  
		  // Pop the args
		  for(int i=0; i<be.op.num_parameters; i++)
		    {
		      op1 = type_check_stack_pop();

		      
		      // We need to insert an autocon if this isn't a FLT
		      if( can_use_autocon(op1.op.type, NOBJ_VARTYPE_FLT) )
			{
			  dbprintf("Autocon added");
			  
			  sprintf(autocon.name, "autocon %c->%c (flist idx)", type_to_char(op1.op.type), type_to_char(NOBJ_VARTYPE_FLT));
			  
			  autocon.op.buf_id = EXP_BUFF_ID_AUTOCON;
			  autocon.p_idx = 1;
			  autocon.p[0] = op1.node_id;
			  autocon.op.type      = NOBJ_VARTYPE_FLT;
			  
			  insert_buf2_entry_after_node_id(op1.node_id, autocon);
			}
		    }
		  break;
		}

	      // Now push the flist type
	      init_op_stack_entry(&(ft.op));
	      
	      ft.node_id = node_id_index++;
	      ft.op.buf_id = EXP_BUFF_ID_BYTE;
	      ft.p_idx = 1;
	      ft.p[0] = be.node_id;
	      sprintf(t_name, "%d", flist_type);
	      strcpy(ft.name, t_name);
	      strcpy(ft.op.name, t_name);
	      ft.op.type      = NOBJ_VARTYPE_INT;

	      // Function is going to be processed next, so we just add this INT to the end of the buffer
	      //	      exp_buffer2[exp_buffer2_i++] = ft;
	      //insert_buf2_entry_before_node_id(be.node_id, ft);

	      // No point in pushing this as we need no args before we check the function result for
	      // autocon
	      
	      //type_check_stack_push(ft);
	    }
	  else
	    {
	      // Normal parsing of arguments
	      be.p_idx = 0;
	      for(int i=num_args-1; i>=0; i--)
		{
		  NOBJ_VARTYPE this_arg_type = function_arg_type_n(be.name, i);
	      
		  // Pop an argument off and check it
		  op1 = type_check_stack_pop();

		  // Force the args to write if needed
		  if( function_access_force_write(be.name) )
		    {
		      dbprintf("Forced arg access to write");
		  
		      set_node_access(op1.node_id, NOPL_OP_ACCESS_WRITE);
		    }
	      
		  // Add to list of arguments
		  // Add to list of arguments
		  if( be.p_idx >= ( MAX_EXP_BUF_P-1) )
		    {
		      internal_error("Argument list full");
		      return;
		    }

		  be.p[be.p_idx++] = op1.node_id;
	      
		  dbprintf("FN ARG %d type:%c %s %d(%c)", i,
			   type_to_char(this_arg_type),
			   op1.name,
			   op1.op.type,
			   type_to_char(op1.op.type));

		  // If there's an autocon then the type of it is the argument type for this argument
		  // of the function
	      
		  autocon.op.type      = this_arg_type;
	      
		  if( op1.op.type == this_arg_type)
		    {
		      ff_fprintf(ofp, "  Arg ok");
		      // All OK
		    }
		  else
		    {
		      ff_fprintf(ofp, "  Arg not OK");

		      sprintf(autocon.name, "autocon (Arg) %c->%c", type_to_char(op1.op.type), type_to_char(this_arg_type));

		      if( ( can_use_autocon(op1.op.type, this_arg_type)))
			{
			  insert_buf2_entry_after_node_id(op1.node_id, autocon);
			}
		    }
		}
	    }
	  
	  // Only push a result if the function is non-void
	  if( ret_type != NOBJ_VARTYPE_VOID )
	    {
	      // Push dummy result
	      EXP_BUFFER_ENTRY res;
	      init_exp_buffer_entry(&res);

	      res.node_id = be.node_id;          // Result id is that of the operator
	      res.p_idx = function_num_args(be.name);
	      res.p[0] = op1.node_id;
	      res.p[1] = op2.node_id;
	      strcpy(res.name, "000");
	      res.op.type      = ret_type;
	      type_check_stack_push(res);
	    }

	  // The ADDR function is a bit odd. It needs to have a reference to its argument
	  // and also needs to be a string type if its argument is a string
	  // as there's a different QCode for ADDR(string)

	  if( strcmp(be.name, "ADDR")==0 )
	    {
	      // Make argument a reference (parser tries, but this is a better way)
	      EXP_BUFFER_ENTRY *a = find_buf2_entry_with_node_id(be.p[0]);

	      // If the argument is a string then make the ADDR a string too
	      switch( a->op.type )
		{
		case NOBJ_VARTYPE_STR:
		case NOBJ_VARTYPE_STRARY:
		  ret_type = NOBJ_VARTYPE_STR;
		  break;
		}
	    }

	  // The return type opf the function is known
	  be.op.type = ret_type;
	  break;

	  // The return type opf the function is known
	  be.op.type = ret_type;
	  break;

	case EXP_BUFF_ID_PROC_CALL:
	  // Procedure calls are like functions, except that no auto conversion of parameters is done.
	  // Mismatched parameter types cause a run time error
	  
	  ff_fprintf(ofp, "\nPROC CALL: %d parameters", be.op.num_parameters);
	  
	  // Set up the function return value
	  ret_type = be.op.type;

	  dbprintf("Ret type of %s : %c", be.name, type_to_char(ret_type));
	  
	  // Build an argument list (constructs part of the syntax tree)
	  be.p_idx = 0;
	  for(int i=be.op.num_parameters-1; i>=0; i--)
	    {
	      // Pop a parameter off
	      op1 = type_check_stack_pop();

	      // Procedure calls cannot have arrays passed to them, so all array types
	      // need to be turned into their non-array versions.
	      op1.op.type = convert_type_to_non_array(op1.op.type);
	      
	      // Add to list of arguments
	      if( be.p_idx >= ( MAX_EXP_BUF_P-1) )
		{
		  internal_error("Argument list full");
		  return;
		}

	      be.p[be.p_idx++] = op1.node_id;
	      be.op.parameter_type[i] = op1.op.type;
	      
	      dbprintf("PROC PAR %d %s Type:%d(%c)",
		       i,
		       op1.name,
		       op1.op.type,
		       type_to_char(op1.op.type));
	    }
	  
	  // Only push a result if the function is non-void
	  if( ret_type != NOBJ_VARTYPE_VOID )
	    {
	      // Push dummy result
	      EXP_BUFFER_ENTRY res;
	      init_exp_buffer_entry(&res);
		  
	      res.node_id = be.node_id;          // Result id is that of the operator
	      res.p_idx = be.op.num_parameters;
	      res.p[0] = op1.node_id;
	      res.p[1] = op2.node_id;
	      strcpy(res.name, "000");
	      res.op.type      = ret_type;
	      type_check_stack_push(res);
	    }
	  
	  // The return type opf the function is known
	  be.op.type = ret_type;
	  break;

	  //------------------------------------------------------------------------------
	  //
	  // Operators have to be typed correctly depending on their
	  // operands. Some of them are mutable (polymorphic) and we have to bind them to their
	  // type here.
	  // Some are immutable and cause errors if their operators are not correct
	  // Some have a fixed output type (>= for example, but still have mutable inputs)
	  // The assignment operator type is determined by the variable being assigned to
	  
	case EXP_BUFF_ID_OPERATOR:
	  // Check that the operands are correct, i.e. all of them are the same and in
	  // the list of acceptable types
	  dbprintf("BUFF_ID_OPERATOR");
	  
	  if( find_op_info(be.name, &op_info) )
	    {
	      dbprintf("Found operator %s", be.name);

	      // We only handle binary operators here
	      // Pop arguments off stack, this is an analogue of execution of the operator
	      
	      op1 = type_check_stack_pop();
	      op2 = type_check_stack_pop();

	      op1_type = op1.op.type;
	      op2_type = op2.op.type;

	      dbprintf("op1 type:%c op2 type:%c", type_to_char(op1.op.type), type_to_char(op2.op.type));
	      
	      // Get the node ids of the argumenmts so we can find them if we need to
	      // adjust them.
	      
	      be.p_idx = 2;
	      be.p[0] = op1.node_id;
	      be.p[1] = op2.node_id;
	      
	      // Check all operands are of correct type.
	      if( op_info.immutable )
		{
		  dbprintf("Immutable type");
		  
		  // Immutable types for this operator so we don't do any
		  // auto conversion here. Just check that the correct type
		  // is present, if not, it's an error
		  
		  typecheck_operator_immutable(be, op_info, op1, op2);

		  // Force return type
		  be.op.type        = op_info.output_type;
		  be.op.qcode_type  = op_info.output_type;

		  // We need to check the arguments and autocon if necessary

		  autocon.op.buf_id = EXP_BUFF_ID_AUTOCON;
		  autocon.p_idx = 2;
		  autocon.p[0] = be.node_id;

		  autocon.op.type      = be.op.type;
		  
		  if( can_use_autocon(op1.op.type, be.op.type) )
		    {
		      autocon.p[1] = op1.node_id;
		      sprintf(autocon.name, "autocon %c->%c (operator 1)", type_to_char(op1.op.type), type_to_char(be.op.type));
		      insert_buf2_entry_after_node_id(op1.node_id, autocon);
		    }
		  
		  if( can_use_autocon(op2.op.type, be.op.type) )
		    {
		      autocon.p[1] = op2.node_id;
		      sprintf(autocon.name, "autocon %c->%c (operator 2)", type_to_char(op2.op.type), type_to_char(be.op.type));
		      insert_buf2_entry_after_node_id(op2.node_id, autocon);
		    }
		}
	      else
		{
		  // Mutable type is dependent on the arguments, e.g.
		  //  A$ = "RTY"
		  // requires that a string equality is used, similarly
		  // INT and FLT need the correctly typed operator.
		  //
		  // INT and FLT have an additional requirement where INT is used
		  // as long as possible, and also assignment can turn FLT into INT
		  // or INT into FLT
		  // For our purposes here, arrays are the same as their element type
		  
		  dbprintf("Mutable type (%s) %c %c", be.name, type_to_char(op1.op.type), type_to_char(op2.op.type));
		  
		  // Check input types are valid for this operator
		  if( is_a_valid_type(op1.op.type, &op_info) && is_a_valid_type(op2.op.type, &op_info))
		    {
		      // We have types here. We need to insert auto type conversion qcodes here
		      // if needed.
		      //
		      // Int -> float if float required
		      // Float -> int if int required
		      // Expressions start as integer and turn into float if a float is found.
		      //

		      // If the operator result type is unknown then we use the types of the arguments
		      // Unknown types arise when brackets are used.
		      if( op_info.output_type != NOBJ_VARTYPE_UNKNOWN )
			{
			  be.op.type = op_info.output_type;
			}
		      else
			{
			  if( be.op.type == NOBJ_VARTYPE_UNKNOWN)
			    {
			      be.op.type = type_with_least_conversion_from(op1.op.type, op2.op.type);
			    }
			  
			  if( (be.op.type == NOBJ_VARTYPE_INT) || (be.op.type == NOBJ_VARTYPE_INTARY))
			    {
			      be.op.type = type_with_least_conversion_from(op1.op.type, op2.op.type);
			    }
			}
		      
		      // Types are both OK
		      // If they are the same then we will bind the operator type to that type
		      // as long as they are both the required type, if not then if types aren't
		      // INT or FLT then it's an error
		      // INT or FLT can be auto converted to the required type
		      
		      if( types_identical(op1.op.type, op2.op.type) )
			{
			  dbprintf("Same type");

			  // The input types of the operands are the same as the required type, all ok
			  be.op.type = op1.op.type;
			  
			  // Now set up output type
			  if( op_info.output_type == NOBJ_VARTYPE_UNKNOWN )
			    {
			      // Do not force
			    }
			  else
			    {
			      dbprintf("(A) Forced type to %c", type_to_char(op1.op.type));
			      be.op.type      = op_info.output_type;
			      be.op.qcode_type = op1.op.type;
			    }
			}
		      else
			{
			  dbprintf(" Autoconversion");
			  dbprintf(" --------------");
			  dbprintf(" Op1: type:%c", type_to_char(op1.op.type));
			  dbprintf(" Op2: type:%c", type_to_char(op2.op.type));
			  dbprintf(" BE:  type:%c",  type_to_char(be.op.type));
		 
			  // We insert auto conversion nodes to force the type of the arguments to match the
			  // operator type. For INT and FLT we can force the operator to FLT if required
			  // Do that before inserting auto conversion nodes.
			  // Special treatment for assignment operator
			  if( op_info.assignment )
			    {
			      dbprintf("Assignment");
			      
			      // Operator type follows the second operand, which is the variable we
			      // are assigning to

			      // Now set up output type
			      if( op_info.output_type == NOBJ_VARTYPE_UNKNOWN )
				{
				  // Do not force
				}
			      else
				{
				  dbprintf("(C) Forced type to %c", type_to_char(op2.op.type));
				  be.op.type       = op_info.output_type;
				  be.op.qcode_type = op2.op.type;
				}

			      be.op.type = op2.op.type;

			      dbprintf(" Assignment Autoconversion");
			      dbprintf(" --------------");
			      dbprintf(" Op1: type:%c", type_to_char(op1.op.type));
			      dbprintf(" Op2: type:%c", type_to_char(op2.op.type));
			      dbprintf(" BE:  type:%c",  type_to_char(be.op.type));

			      // Now insert auto convert node
			      // Only convert the value being assigned.
			      sprintf(autocon.name, "autocon %c->%c (assignment target)", type_to_char(op2.op.type), type_to_char(op1.op.type));

			      autocon.op.buf_id = EXP_BUFF_ID_AUTOCON;
			      autocon.p_idx = 2;
			      autocon.p[0] = op2.node_id;
			      autocon.p[1] = op1.node_id;
			      autocon.op.type      = be.op.type;

			      //autocon.node_id = node_id_index++;   //Dummy result carries the operator node id as that is the tree node
			      sprintf(autocon.name, "autocon %c->%c (assign target)", type_to_char(op1.op.type), type_to_char(op2.op.type));
			      insert_buf2_entry_after_node_id(op1.node_id, autocon);

			      
			      // Now set up output type
			      if( op_info.output_type == NOBJ_VARTYPE_UNKNOWN )
				{
				  // Do not force
				}
			      else
				{
				  dbprintf("(D) Forced type to %c", type_to_char(op_info.output_type));
				  be.op.type      = op_info.output_type;
				  be.op.qcode_type = autocon.op.type;
				}
			    }
			  else
			    {
			      // Must be INT/FLT or FLT/INT
			      if( (op1.op.type == NOBJ_VARTYPE_FLT) || (op2.op.type == NOBJ_VARTYPE_FLT) )
				{
				  // Force operator to FLT
				  be.op.type = NOBJ_VARTYPE_FLT;
				}
			      else if( (op1.op.type == NOBJ_VARTYPE_FLTARY) || (op2.op.type == NOBJ_VARTYPE_FLTARY) )
				{
				  // Force operator to FLT
				  be.op.type = NOBJ_VARTYPE_FLT;
				}
			    }

			  if( !op_info.assignment )
			    {
			      // If type of operator is unknown, 
			      
			      // Now insert auto convert nodes if required
			      sprintf(autocon.name, "autocon ?->?"); //type_to_char(op1.op.type), type_to_char(be.op.req_type));
			      autocon.op.buf_id = EXP_BUFF_ID_AUTOCON;

			      autocon.p_idx = 2;
			      autocon.p[0] = op1.node_id;
			      autocon.p[1] = op2.node_id;
			      autocon.op.type      = be.op.type;
			      
			      if( can_use_autocon(op1.op.type, be.op.type) )
				{
				  sprintf(autocon.name, "autocon %c->%c (operator 1)", type_to_char(op1.op.type), type_to_char(be.op.type));
				  insert_buf2_entry_after_node_id(op1.node_id, autocon);
				}
			      
			      if( can_use_autocon(op2.op.type, be.op.type) )
				{
				  sprintf(autocon.name, "autocon %c->%c (operator 2)", type_to_char(op2.op.type), type_to_char(be.op.type));
				  insert_buf2_entry_after_node_id(op2.node_id, autocon);
				}
			      
			      // Now set up output type
			      if( op_info.output_type == NOBJ_VARTYPE_UNKNOWN )
				{
				  // Do not force
				}
			      else
				{
				  dbprintf("(D) Forced type to %c", type_to_char(op_info.output_type));
				  be.op.type      = op_info.output_type;
				  be.op.qcode_type = autocon.op.type;
				}
			    }
			}
		      
		      if( op_info.returns_result )
			{
			  EXP_BUFFER_ENTRY res;
			  init_exp_buffer_entry(&res);
				  
			  strcpy(res.name, "000");
			  res.node_id = be.node_id;   //Dummy result carries the operator node id as that is the tree node
			  res.p_idx = 2;
			  res.p[0] = op1.node_id;
			  res.p[1] = op2.node_id;
			  res.op.type      = be.op.type;
			  type_check_stack_push(res);
			}
		    } // if()Valid types
		  else
		    {
		      // Unknown required types exist, this probably shoudn't happen is a syntax error
		      dbprintf("Syntax error at node N%d, unknown required type", be.node_id);
		      type_check_stack_display();

		      dump_exp_buffer(ofp, 1);
		      internal_error("Syntax error at node N%d, unknown required type", be.node_id);
		      //exit(-1);
		    }
		} // if mutable


	      // Mutable, or immutable we need to check th argument types and insert autocon if needed
	    }
	  else
	    {
	      // Error, not found
	    }
	  break;

	case EXP_BUFF_ID_OPERATOR_UNARY:
	  dbprintf("BUFF_ID_OPERATOR_UNARY");
	  
	  if( find_op_info(be.name, &op_info) )
	    {
	      dbprintf("Found unary operator %s", be.name);

	      // Some unary operators, like UMIN, don't change the type of their argument.
	      // Their output follows that of their argument.
	      // others, like UNOT have a fixed output type. 

	      
	      op1 = type_check_stack_pop();
	      op1_type = op1.op.type;

	      dbprintf("op1 type:%c", type_to_char(op1.op.type));

	      if( op_info.output_type != NOBJ_VARTYPE_UNKNOWN )
		{
		  be.op.type = op_info.output_type;
		}
	      else
		{
		  // Set the copied entry to the same type
		  be.op.type = op1.op.type;
		}
	      
	      // Create a link to the argument.
	      be.p_idx = 1;
	      be.p[0] = op1.node_id;
	      
	      // Push result
	      EXP_BUFFER_ENTRY res;
	      init_exp_buffer_entry(&res);

	      strcpy(res.name, "000");
	      res.node_id = be.node_id;   //Dummy result carries the operator node id as that is the tree node
	      res.p_idx = 1;
	      res.p[0] = op1.node_id;
	      res.op.type      = be.op.type;
	      type_check_stack_push(res);

	    }
	  else
	    {
	      dbprintf("Unkown unary operator '%s'", be.name);
	      syntax_error("Unkown unary operator '%s'", be.name);
	    }
	  break;
	  
	default:
	  ff_fprintf(ofp, "\ndefault buf_id");
	  break;
	}
      
      // If entry not copied over, copy it
      if( !copied )
	{
	  if( exp_buffer2_i >= (MAX_EXP_BUFFER-1) )
	    {
	      internal_error("exp buffer 2 overflow (insert)");
	      return;
	    }

	  exp_buffer2[exp_buffer2_i++] = be;
	}

      type_check_stack_display();
      type_check_stack_fprint(exfp);
    }

  ff_fprintf(exfp, "\n--------------------\n");

  // Do we have a value stacked that isn't going to be used?
  if( (pass_number == 2) && (type_check_stack_ptr > 0) )
    {
      // If we only have logical file names or field variables then we ignore those
      if( type_check_stack_only_field_data() )
	{
	  dbprintf("Only field data left, so no DROP needed");
	}
      else
	{
	  dbprintf("Value left stacked so DROP needed");
	  
	  // We want a drop qcode to be generated to remove any value left on the stack. This is typed
	  // so to avoid duplicating code the internal command DROP is used. That uses the structure we have for
	  // translating functions and commands to qcode, taking the type into account.
	  // The type needs to be the type of the last function or command in the line.
	  
	  //	  EXP_BUFFER_ENTRY res;
	  strcpy(be.name, "DROP");
	  strcpy(be.op.name, be.name);
	  be.op.buf_id = EXP_BUFF_ID_FUNCTION;
	  be.p_idx = 0;
	  be.op.type      = last_known_type;

	  if( exp_buffer2_i >= (MAX_EXP_BUFFER-1) )
	    {
	      internal_error("exp buffer 2 overflow (insert)");
	      return;
	    }

	  exp_buffer2[exp_buffer2_i++] = be;
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Processes the RPN tree
//
// The RPN is 'executed' but no function is actually performed. The execution
// is done so a tree can be built up with the nodes of the RPN (with types
// associated with the nodes). This allows auto type conversion nodes to be
// added in the tree where required.
//
////////////////////////////////////////////////////////////////////////////////

void expression_tree_process(char *expr)
{
  node_id_index = 1;
  
  dump_exp_buffer(ofp, 1);
  process_syntax_tree();
  copy_buf2_to_buf();
  typecheck_expression();
  
  dump_exp_buffer(ofp, 1);
  dump_exp_buffer(ofp, 2);
}

////////////////////////////////////////////////////////////////////////////////

// String display of type stack

char tss[400];

char *type_stack_str(void)
{
  char tmps[20];
  
  sprintf(tss, "[%c,(", type_to_char(expression_type));
  
  for(int i=0; i<exp_type_stack_ptr; i++)
    {
      sprintf(tmps, "%c", type_to_char(exp_type_stack[i]));
      strcat(tss, tmps);
    }
  strcat(tss, ")]");
  return(tss);
}

////////////////////////////////////////////////////////////////////////////////

void init_op_stack_entry(OP_STACK_ENTRY *op)
{
  op->buf_id = EXP_BUFF_ID_NONE;
  op->name[0]        = '\0';
  op->num_bytes      = 0;
  op->level          = 0;
  op->num_parameters = 0;
  op->vi.num_indices = 0;
  op->trapped        = 0;
  op->percent        = 0;
  
  op->access         = NOPL_OP_ACCESS_READ;  // Default to reading things
  op->qcode_type     = NOBJ_VARTYPE_UNKNOWN; // Ignored if UNKNOWN, only some operators use this
  
  for(int i=0; i<NOPL_MAX_SUFFIX_BYTES; i++)
    {
      op->bytes[i] = 0xCC;
    }
}

void init_exp_buffer_entry(EXP_BUFFER_ENTRY *e)
{
  init_op_stack_entry(&(e->op));
  strcpy(e->name, "");
  e->p_idx = 0;
  e->node_id = 0;
  
}

void output_float(OP_STACK_ENTRY token)
{
  ff_fprintf(ofp, "\n(%16s) %s %c %s", __FUNCTION__, type_stack_str(), type_to_char(token.type), token.name);
  add_exp_buffer_entry(token, EXP_BUFF_ID_FLT);
}

void output_integer(OP_STACK_ENTRY token)
{
  ff_fprintf(ofp, "\n(%16s) %s %c %s", __FUNCTION__, type_stack_str(), type_to_char(token.type), token.name);
  add_exp_buffer_entry(token, EXP_BUFF_ID_INTEGER);
}

// Operators can be unary
void output_operator(OP_STACK_ENTRY op)
{
  char *tokptr;

  op.type = expression_type;

  dbprintf("%s %c %s", type_stack_str(), type_to_char(op.type), op.name);
  add_exp_buffer_entry(op, op.buf_id);
}

void output_function(OP_STACK_ENTRY op)
{
  ff_fprintf(ofp, "\n(%16s) %s %c %s", __FUNCTION__, type_stack_str(), type_to_char(op.type), op.name);
  add_exp_buffer_entry(op, EXP_BUFF_ID_FUNCTION);
}

void output_variable(OP_STACK_ENTRY op)
{
  expression_type = op.type;
  
  ff_fprintf(ofp, "\n(%16s) %s %c %s", __FUNCTION__, type_stack_str(), type_to_char(op.type), op.name);
  add_exp_buffer_entry(op, EXP_BUFF_ID_VARIABLE);
}

void output_var_addr_name(OP_STACK_ENTRY op)
{
  ff_fprintf(ofp, "\n(%16s) %s %c %s", __FUNCTION__, type_stack_str(), type_to_char(op.type), op.name);
  add_exp_buffer_entry(op, EXP_BUFF_ID_VAR_ADDR_NAME);
}

void output_string(OP_STACK_ENTRY op)
{
  ff_fprintf(ofp, "\n(%16s) %s %c %s", __FUNCTION__, type_stack_str(), type_to_char(op.type), op.name);
  // Always a string type
  add_exp_buffer_entry(op, EXP_BUFF_ID_STR);
}

void output_return(OP_STACK_ENTRY op)
{
  //op.type = expression_type;
  
  ff_fprintf(ofp, "\n(%16s) %s %c %s", __FUNCTION__, type_stack_str(), type_to_char(op.type), op.name); 
  add_exp_buffer_entry(op, EXP_BUFF_ID_RETURN);
}

void output_print(OP_STACK_ENTRY op)
{
  op.type = expression_type;

  ff_fprintf(ofp, "\n(%16s) %s %c %s", __FUNCTION__, type_stack_str(), type_to_char(op.type), op.name); 
  add_exp_buffer_entry(op, op.buf_id);
}

void output_proc_call(OP_STACK_ENTRY op)
{
  ff_fprintf(ofp, "\n(%16s) %s %c %s", __FUNCTION__, type_stack_str(), type_to_char(op.type), op.name); 
  add_exp_buffer_entry(op, EXP_BUFF_ID_PROC_CALL);
}

void output_if(OP_STACK_ENTRY op)
{
  ff_fprintf(ofp, "\n(%16s) %s %c %s", __FUNCTION__, type_stack_str(), type_to_char(op.type), op.name); 
  add_exp_buffer_entry(op, EXP_BUFF_ID_IF);
}

void output_generic(OP_STACK_ENTRY op, char *name, int buf_id)
{
  char line[20];
  
  strcpy(op.name, name);
  op.buf_id = buf_id;
  op.type = expression_type;
  
  dbprintf("%s %c %s exp_type:%c", type_stack_str(), type_to_char(op.type), op.name, type_to_char(expression_type) ); 
  add_exp_buffer_entry(op, buf_id);
}

void output_fieldvar(OP_STACK_ENTRY op, char *name, int buf_id)
{
  char line[20];
  
  strcpy(op.name, name);
  op.buf_id = buf_id;
  //  op.type = expression_type;
  
  dbprintf("%s %c %s exp_type:%c", type_stack_str(), type_to_char(op.type), op.name, type_to_char(expression_type) ); 
  add_exp_buffer_entry(op, buf_id);
}

void output_endif(OP_STACK_ENTRY op)
{
  printf("\nop if");
  ff_fprintf(ofp, "\n(%16s) %s %c %s", __FUNCTION__, type_stack_str(), type_to_char(op.type), op.name); 
  add_exp_buffer_entry(op, EXP_BUFF_ID_ENDIF);
}

// Markers used as comments, and hints
void output_marker(char *marker, ...)
{
  va_list valist;
  char line[80];
  
  va_start(valist, marker);

  vsprintf(line, marker, valist);
  va_end(valist);

  ff_fprintf(ofp, "\n(%16s) %s", __FUNCTION__, line);
}

void output_sub_start(void)
{
  OP_STACK_ENTRY op;
  init_op_stack_entry(&op);
  
  ff_fprintf(ofp, "\n(%16s)", __FUNCTION__);

  strcpy(op.name,  "");
  op.type = NOBJ_VARTYPE_UNKNOWN;
  add_exp_buffer_entry(op, EXP_BUFF_ID_SUB_START);
}

void output_sub_end(void)
{
  OP_STACK_ENTRY op;

  init_op_stack_entry(&op);
    
  ff_fprintf(ofp, "\n(%16s)", __FUNCTION__);

  strcpy(op.name, "");
  op.type = NOBJ_VARTYPE_UNKNOWN;
  add_exp_buffer_entry(op, EXP_BUFF_ID_SUB_END);
}

void output_expression_start(char *expr)
{
  strcpy(current_expression, expr);
  
  if( strlen(expr) > 0 )
    {
      ff_fprintf(ofp, "\n%s", expr);
      ff_fprintf(ofp, "\n========================================================");

      ff_fprintf(ofp, "\n(%16s)", __FUNCTION__);
      
      // We have a new expression, process the previous one which will be in the
      // buffer
      
      //  expression_tree_process(expr);
      
    }

  // Clear operator stack
  op_stack_ptr = 0;
  
  // Clear the buffer ready for the new expression that has just come in
  clear_exp_buffer();

  first_token = 1;
  expression_type = NOBJ_VARTYPE_UNKNOWN;
	
}

////////////////////////////////////////////////////////////////////////////////
//
// Stack function in operator stack
//
////////////////////////////////////////////////////////////////////////////////


void op_stack_push(OP_STACK_ENTRY entry)
{
  ff_fprintf(ofp,"\n Push:'%s'", entry.name);

  
  if( op_stack_ptr < NOPL_MAX_OP_STACK )
    {
      op_stack[op_stack_ptr++] = entry;
    }
  else
    {
      ff_fprintf(ofp, "\n%s: Operator stack full", __FUNCTION__);
      internal_error("Operator stack full");
      //exit(-1);
    }
  op_stack_print();

}

// Copies data into string

OP_STACK_ENTRY op_stack_pop(void)
{
  OP_STACK_ENTRY o;
  
  if( op_stack_ptr == 0 )
    {
      ff_fprintf(ofp, "\n%s: Operator stack empty", __FUNCTION__);
      typecheck_error("%s:Operator stack empty", __FUNCTION__);
      return(o);
    }
  
  op_stack_ptr --;

  o = op_stack[op_stack_ptr];
  dbprintf("Pop '%s' type:%c ", o.name, type_to_char(o.type));
  op_stack_print();
  return(o);
}

// Return a pointer to the top entry of the stack, empty string if empty
OP_STACK_ENTRY op_stack_top(void)
{
  if( op_stack_ptr == 0 )
    {
      OP_STACK_ENTRY o;
      
      strcpy(o.name, "");
      o.type = NOBJ_VARTYPE_UNKNOWN;
      
      return(o);
    }

  return( op_stack[op_stack_ptr-1] );
}

void op_stack_display(void)
{
  char *s;
  
  ff_fprintf(ofp, "\n\nOperator Stack\n");
  
  for(int i=0; i<op_stack_ptr-1; i++)
    {
      s = op_stack[i].name;
      ff_fprintf(ofp, "\n%03d: %s type:%d", i, s, op_stack[i].type);
    }
}

void op_stack_fprint(FIL *fp)
{
  char *s;
  
  for(int i=0; i<op_stack_ptr; i++)
    {
      s = op_stack[i].name;
      ff_fprintf(fp, "\n          %03d:type:%c '%s'", i, type_to_char(op_stack[i].type), s);
    }
  ff_fprintf(fp, "\n");
  
}

void op_stack_print(void)
{
  char *s;

  dbprintf("------------------");
  dbprintf("Operator Stack     (%d)\n", op_stack_ptr);
  
  for(int i=0; i<op_stack_ptr; i++)
    {
      s = op_stack[i].name;
      dbprintf("%03d: %s type:%c id:%s",
	       i,
	       s,
	       type_to_char(op_stack[i].type),
	       exp_buffer_id_str[op_stack[i].buf_id]);
    }

  dbprintf("------------------\n");
}

////////////////////////////////////////////////////////////////////////////////
//
// End of shunting algorithm, flush the stack
// Processing the RPN as a tree (adds auto conversion) is done after we flush
// the stack (shunting operator stack)
//
////////////////////////////////////////////////////////////////////////////////

void op_stack_finalise(void)
{
  OP_STACK_ENTRY o;

  dbprintf("Finalise stack");

  while( strlen(op_stack_top().name) != 0 )
    {
      o = op_stack_pop();

      dbprintf("Popped:%s %c", o.name, type_to_char(o.type));
      if( o.type == NOBJ_VARTYPE_UNKNOWN )
	{
	  //	  o.req_type = expression_type;
	  o.type = expression_type;
	}

      // PRINT commands need to match the expression type
      if( o.buf_id == EXP_BUFF_ID_PRINT )
	{
	  // Force the type
	  //	  o.req_type = expression_type;
	  o.type = expression_type;
	}
      output_operator(o);
    }

}

////////////////////////////////////////////////////////////////////////////////

void process_expression_types(void)
{
  char *infix;

  dbprintf("\n%s:", __FUNCTION__);

  // TODO needed?
  if( strlen(current_expression) > 0,1 )
    {
      // Process the RPN as a tree
      expression_tree_process(current_expression);

#if INFIX_FROM_RPN      
      dbprintf("\n==INFIX==\n",0);
      dbprintf("==%s==", infix = infix_from_rpn());
#endif
      dbprintf("\n\n",0);
      
      // Generate the QCode from the tree output, but only on pass 2
      if( pass_number == 2 )
	{
	  output_qcode_for_line();
	}
    }
}

void init_output(void)
{
  ofp   = fopen("output.txt", "w");
  chkfp = fopen("check.txt", "w");
  trfp  = fopen("translated.opl", "w");
  icfp  = fopen("intcode.txt", "w");
}

void uninit_output(void)
{
  op_stack_display();
  fclose(ofp);
  fclose(icfp);
}

////////////////////////////////////////////////////////////////////////////////
//
// Expression type stack
//
// Used when processing sub expressions as we need to have an expression type for
// all sub expressions but not lose the current one when the sub expression finishes
// processing. The current expression type is saved by pushing it onto this stack
//
//
////////////////////////////////////////////////////////////////////////////////


void exp_type_push(NOBJ_VARTYPE t)
{
  if( exp_type_stack_ptr <= (MAX_EXP_TYPE_STACK - 1))
    {
      exp_type_stack[exp_type_stack_ptr++] = t;
    }
  else
    {
      ff_fprintf(ofp, "\nSub expression stack full");
      typecheck_error("Sub expression stack full");

      dbprintf("Expression stack: %s", type_stack_str());
      return;
    }
}

NOBJ_VARTYPE exp_type_pop(void)
{
  if( exp_type_stack_ptr > 0 )
    {
      return(exp_type_stack[--exp_type_stack_ptr]);
    }
  else
    {
      ff_fprintf(ofp, "\nExp stack empty on pop");
      typecheck_error("Expression stack empty on pop");
      return(NOBJ_VARTYPE_UNKNOWN);
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Process another token using the shunting algorithm.
//
// This converts the infix expression (as in OPL) to an RPN expression.
// All OPL statements are treated as expressions. This works well with the
// QCode assignment operator, and OPL functions and commands (functions
// with no return value)
//
////////////////////////////////////////////////////////////////////////////////

// For unary operators (we only have '-')
// The next token is unary (if it can be) if it is at the start of th einput, or follows an operator
// or follows a left parenthesis.

int unary_next = 0;

#define CAN_BE_UNARY (first_token || unary_next)

void process_token(OP_STACK_ENTRY *token)
{
  char *tokptr;
  OP_STACK_ENTRY o1;
  OP_STACK_ENTRY o2;
  int opr1, opr2;
  
  dbprintf("   Frst:%d T:'%s' toktype:%c exptype:%c bufid:'%s'",
	  first_token, token->name,
	   type_to_char(token->type),
	  type_to_char(expression_type),
	  exp_buffer_id_str[token->buf_id]);

  o1 = *token;


  ff_fprintf(shfp, "\n%s", token->name);
  
  //strcpy(o1.name, token);
  if( o1.type == NOBJ_VARTYPE_UNKNOWN )
    {
      o1.type = expression_type;
    }
  o1.buf_id = token->buf_id;
  
  // Another token has arrived, process it using the shunting algorithm
  // First, check the stack for work to do

  o2 = op_stack_top();
  opr1 = operator_precedence(o1.name);
  opr2 = operator_precedence(o2.name);

  if( strcmp(o1.name, "(")==0 )
    {
      OP_STACK_ENTRY o;

      init_op_stack_entry(&o);
	
      //output_marker("--------- Sub 1");
      output_sub_start();
      
      strcpy(o.name, "(");
      o.type = NOBJ_VARTYPE_UNKNOWN;
      o.buf_id = EXP_BUFF_ID_SUB_START;
      op_stack_push(o);

      // Sub expression, push (save) the expression type and process the sub expression
      // as a new one
      exp_type_push(expression_type);
      expression_type = NOBJ_VARTYPE_UNKNOWN;
      first_token = 0;
      unary_next = 1;
      op_stack_fprint(shfp);

      return;
    }

  if( strcmp(o1.name, ")")==0 )
    {
      //output_marker("----------- Sub E 1");
	    
      while(  (strcmp(op_stack_top().name, "(") != 0) && (strlen(op_stack_top().name)!=0) )
	{
	  dbprintf("\nPop 3");
	  o2 = op_stack_pop();
	  output_operator(o2);
	}

      if( strlen(op_stack_top().name)==0 )
	{
	  // Mismatched parentheses
	  dbprintf("\nMismatched parentheses");
	  op_stack_fprint(shfp);
	  return;
	}

      // Pop the open bracket off the operator stack
      ff_fprintf(ofp, "\nPop 4");
      o2 = op_stack_pop();
      
      //output_marker("-------- Sub E 2");
      output_sub_end();
      
      if( strcmp(o2.name, "(") != 0 )
	{
	  dbprintf("\n**** Should be left parenthesis");
	  op_stack_fprint(shfp);
  
	  return;
	}
      
      expression_type = exp_type_pop();

      output_sub_end();
      first_token = 0;
      unary_next = 0;
      op_stack_fprint(shfp);
      return;
    }

  dbprintf("Before switch, bufid:'%s'",
	  exp_buffer_id_str[token->buf_id]);

  switch( o1.buf_id )
    {

    case EXP_BUFF_ID_PRINT:
    case EXP_BUFF_ID_PRINT_SPACE:
    case EXP_BUFF_ID_PRINT_NEWLINE:
    case EXP_BUFF_ID_LPRINT:
    case EXP_BUFF_ID_LPRINT_SPACE:
    case EXP_BUFF_ID_LPRINT_NEWLINE:
      ff_fprintf(ofp, "\nBuff id print");
      
      NOBJ_VARTYPE vt = NOBJ_VARTYPE_UNKNOWN;
      
      // The type of the function is known, use that, not the expression type
      // which is more of a hint.
      //strcpy(o1.name, tokptr);
      
      ff_fprintf(ofp, "\n%s: '%s' t=>%c", __FUNCTION__, o1.name, type_to_char(vt));

      o1.type = NOBJ_VARTYPE_UNKNOWN;
      //      o1.req_type = NOBJ_VARTYPE_UNKNOWN;
      op_stack_push(o1);
      first_token = 0;
      unary_next = 0;
      op_stack_fprint(shfp);

      return;
      break;

    
    case EXP_BUFF_ID_RETURN:
      ff_fprintf(ofp, "\nBuff id return");

      // RETURN needs to change depending on the type of the expression we are to return.
      // The type of that expression must also match that of the procedure we are translating
      //      o1.req_type = expression_type;
      output_return(o1);
      unary_next = 0;
      op_stack_fprint(shfp);

      return;
      break;
      
    case EXP_BUFF_ID_PROC_CALL:
      ff_fprintf(ofp, "\nBuff id proc call");
      
      // Parser supplies type
      //      o1.req_type = expression_type;
      output_proc_call(o1);
      unary_next = 0;
  op_stack_fprint(shfp);

      return;
      break;

    case EXP_BUFF_ID_WHILE:
    case EXP_BUFF_ID_UNTIL:
    case EXP_BUFF_ID_IF:
    case EXP_BUFF_ID_ENDIF:
    case EXP_BUFF_ID_ENDWH:
    case EXP_BUFF_ID_TRAP:
    case EXP_BUFF_ID_GOTO:
    case EXP_BUFF_ID_META:

    case EXP_BUFF_ID_LOGICALFILE:
      dbprintf("Buff id %s", o1.name);
      
      // Parser supplies type
      //      o1.req_type = expression_type;
      output_generic(o1, o1.name, o1.buf_id);
      unary_next = 0;
  op_stack_fprint(shfp);

      return;
      break;
      
    case EXP_BUFF_ID_FIELDVAR:
      dbprintf("Buff id %s", o1.name);

      // Get variable names
      init_get_name(o1.name);

      NOBJ_VARTYPE type;
      
      if( get_name(o1.name, &type) )
	{
	  o1.type = type;
	  //	  modify_expression_type(type);
	}
      
      //      o1.req_type = expression_type;
      output_fieldvar(o1, o1.name, o1.buf_id);
      unary_next = 0;
  op_stack_fprint(shfp);

      return;
      break;
	    
    case EXP_BUFF_ID_VAR_ADDR_NAME:
      ff_fprintf(ofp, "\nBuff id var addr name");
      
      // Parser supplies type
      //      o1.req_type = NOBJ_VARTYPE_VAR_ADDR;
      o1.type     = NOBJ_VARTYPE_VAR_ADDR;
      output_var_addr_name(o1);
      unary_next = 0;
  op_stack_fprint(shfp);
      return;
      break;
    }

#define OP_PREC(OP) (operator_precedence(OP.name))
  
  if( token_is_operator(o1.name, &(tokptr)) )
    {
      dbprintf("\nToken is operator o1 name:%s o2 name:%s", o1.name, o2.name);
      dbprintf("\nopr1:%d opr2:%d", opr1, opr2);
      
      // Turn token into unary version if we can
      if( CAN_BE_UNARY )
	{
	  operator_can_be_unary(&o1);
	  dbprintf("Operator turned into unary version");
	  opr1 = operator_precedence(o1.name);

	}
      
      unary_next = 1;
#define O2_HAS_GRTR_PREC_THAN_O1     ( OP_PREC(op_stack_top()) > opr1  )
#define O1_AND_O1_HAVE_SAME_PREC     ( opr1 == OP_PREC(op_stack_top()) )
#define O1_IS_LEFT_ASSOC             ( operator_left_assoc(o1.name)    )
#if 0
      while( (strlen(op_stack_top().name) != 0) && (strcmp(op_stack_top().name, "(") != 0 ) &&
	     ( OP_PREC(op_stack_top()) > opr1) || ((opr1 == OP_PREC(op_stack_top()) && operator_left_assoc(o1.name)))
#endif
	     while( (strlen(op_stack_top().name) != 0) && (strcmp(op_stack_top().name, "(") != 0 ) &&
		    ( O2_HAS_GRTR_PREC_THAN_O1 || (O1_AND_O1_HAVE_SAME_PREC && O1_IS_LEFT_ASSOC) )
	     )
	{
	  ff_fprintf(ofp, "\nPop 1");
	  
	  o2 = op_stack_pop();
	  opr1 = operator_precedence(o1.name);
	  opr2 = operator_precedence(o2.name);

	  output_operator(o2);
	}

      dbprintf("Push %s", exp_buffer_id_str[o1.buf_id]);
      
      //strcpy(o1.name, tokptr);

      o1.type = expression_type;
      //      o1.req_type = expression_type;
      op_stack_push(o1);
      first_token = 0;
  op_stack_fprint(shfp);
      return;
    }

  if( token_is_float(o1.name) || (o1.buf_id == EXP_BUFF_ID_FLT) )
    {

      o1.type = NOBJ_VARTYPE_FLT;

      modify_expression_type(o1.type);

      //      o1.req_type = expression_type;
      
      output_float(o1);
      first_token = 0;
      unary_next = 0;
  op_stack_fprint(shfp);
      return;
    }

  if( token_is_integer(o1.name) )
    {
      o1.type = NOBJ_VARTYPE_INT;
      
      modify_expression_type(o1.type);
      //      o1.req_type = expression_type;
      output_integer(o1);
      first_token = 0;
      unary_next = 0;
  op_stack_fprint(shfp);
      return;
    }

    if( token_is_function(o1.name, &tokptr) )
    {
      NOBJ_VARTYPE vt;

      // The type of the function is known, use that, not the expression type
      // which is more of a hint.
      strcpy(o1.name, tokptr);
      vt = function_return_type(o1.name);

      // A few functions require a write access (e.g. EDIT)
      // INPUT does as well but has its own parsing
      
      if( function_access_force_write(o1.name) )
	{
	  o1.access = NOPL_OP_ACCESS_WRITE;
	}
      
      ff_fprintf(ofp, "\n%s: '%s' t=>%c", __FUNCTION__, o1.name, type_to_char(vt));

      o1.type = vt;
      //      o1.req_type = vt;
      op_stack_push(o1);
      first_token = 0;
      unary_next = 0;
      op_stack_fprint(shfp);
      return;
    }

  if( token_is_variable(o1.name) )
    {
      NOBJ_VARTYPE type, new_type;

      // Get variable names
      init_get_name(o1.name);

      if( get_name(o1.name, &type) )
	{
	  // Valid variable name
	  // Perform type checking. We need to be sure that this is a
	  // valid type (e.g. A% = B$ is invalid) and also the expression type
	  // may need to change (e.g.  A% + 10 * B needs to be type FLT for the * and a
	  // type conversion token needs to be inserted.

	  // If the first token is a variable then we don't want to update the expression type
	  // as this is an assignment and we want the assignment to become a float, for instance
	  // only based on the calculation not the assignment variable type. If we didn't then
	  // expressionms like:
	  //
	  // A= 10*20
	  //
	  // would be calculated as floats, which isn't what the original does. The type is therefore set
	  // to int if it's a float or int
	  //
	  //
	  // Arrays need to have their indices calculated with expressions. An array dereference
	  // operator is inserted to ensure that the index expressions are bound to the variables
	  // correctly through the shunting algorithm.
	  
	  if( first_token )
	    {
	      // If a float variable then we start with INT as a type. Auto conversion will
	      // handle the int/float type issues.
	      if (type == NOBJ_VARTYPE_FLT)
		{
		  modify_expression_type(NOBJ_VARTYPE_INT);
		}
	      else
		{
		  modify_expression_type(type);
		}
	      
	      //	      o1.req_type = type;
	      o1.type = type;
	    }
	  else
	    {
	      ff_fprintf(ofp, "\n%s:type:%c", __FUNCTION__, type_to_char(o1.type));
	      modify_expression_type(type);
	      //o1.req_type = expression_type;
	      //o1.type = expression_type;
	    }
	}
      else
	{
	  // Syntax error
	}

      // The type of the variable will affect the expression type
      
      output_variable(o1);
      first_token = 0;
      unary_next = 0;

  op_stack_fprint(shfp);
      
      return;
    }

  if( token_is_string(o1.name) )
    {
      o1.type = NOBJ_VARTYPE_STR;
      output_string(o1);
      modify_expression_type(o1.type);
      first_token = 0;
      unary_next = 0;
      op_stack_fprint(shfp);
      return;
    }
  
  first_token = 0;
  unary_next = 0;
  
  dbprintf("**Unknown token **      '%s'", o1.name);
  op_stack_fprint(shfp);
}

int is_op_delimiter(char ch)
{
  switch(ch)
    {
    case ':':
    case ';':
    case ',':
    case '(':
    case ')':
    case '"':
    case '+':
    case'@':
    case '-':
    case '*':
    case '/':
    case '>':
    case '<':
      //    case '$':
    case '=':
  op_stack_fprint(shfp);
      return(1);
      break;
    }
  op_stack_fprint(shfp);

  return 0;
  
}

int is_delimiter(char ch)
{
  switch(ch)
    {
    case ' ':
    case '\n':
    case '\r':
    case ':':
    case ';':
    case ',':
    case '@':
    case '+':
    case '-':
    case '*':
    case '/':
    case '(':
    case ')':
    case '>':
    case '<':
      //case '$':
    case '=':
    case '"':
  op_stack_fprint(shfp);
      return(1);
      break;
    }

  op_stack_fprint(shfp);
  return 0;
  
}

////////////////////////////////////////////////////////////////////////////////
//
// Scan and check for command
// 
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////

void finalise_expression(void)
{
  dbprintf("Finalise expression Buf i:%d buf2 i:%d", exp_buffer_i, exp_buffer2_i);

  // Now finalise the translation
  op_stack_finalise();
  process_expression_types();
  
  dbprintf("Finalise expression done.");
}


////////////////////////////////////////////////////////////////////////////////
//
// Translates a file
//
////////////////////////////////////////////////////////////////////////////////

// This is really a scan_file(0 function. It parses the overall structure of the file
//
// The procedure definition has to be the first line in the file, then the LOCAL
// and GLOBAL statements.
//
// These requirements could be relaxed.
//

char line[MAX_NOPL_LINE+1];

void translate_file(FIL *fp, FIL *ofp)
{
  int idx;
  char tag = 'A';
  
  dbprintf("********************************************************************************");
  dbprintf("**                                                                            **");
  dbprintf("**                 TRANSLATE FILE                                             **");
  dbprintf("**                                                                            **");
  dbprintf("********************************************************************************");

  //  printf("\n%s:%c", __FUNCTION__, tag++);
  
  // Initialise the line supplier
  initialise_line_supplier(fp);
  //printf("\n%s:%c", __FUNCTION__, tag++);

  idx = cline_i;
  
  // Now translate the file
  pull_next_line();
  //printf("\n%s:%c", __FUNCTION__, tag++);
  
  if( scan_procdef() )
    {
      n_lines_ok++;
      dbprintf("\ncline scanned OK");
      //finalise_expression();
    }
  else
    {
      n_lines_bad++;
      dbprintf("\ncline failed scan");
    }
  //printf("\n%s:%c", __FUNCTION__, tag++);

  pull_next_line();
  //printf("\n%s:%c", __FUNCTION__, tag++);

  indent_none();

  LEVEL_INFO levels;

  levels.if_level = 0;

  while( 1 )
    {
      // Keep things alive
      tight_loop_tasks();

      //printf("\nloop\n");
      
      if( !scan_line(levels) )
	{
	  dbprintf("Scan line failed");
	  break;
	}

      //      printf("\nloop B\n");

      dbprintf("********************************************************************************");
      dbprintf("********************************************************************************");
      dbprintf("Scan line ok");
      
      n_lines_ok++;
      
      idx = cline_i;
      
      if ( check_literal(&idx," :") )
	{
	  dbprintf("Dropping colon");
	  ff_fprintf(chkfp, "  dropping colon");
	  cline_i = idx;
	  //scan_literal(" :");
	}

      //printf("\nloop B\n");
            
      indent_none();

      //printf("\nloop C\n");

    }

  //printf("\nloop D\n");

  finalise_expression();

  //printf("\nloop E\n");
        
  // Done
  dbprintf("Done");
  dbprintf("");
}

////////////////////////////////////////////////////////////////////////////////
//
// Dump the variable table.
//

void dump_vars(FIL *fp)
{

  ff_fprintf(fp, "\nVariables");
  ff_fprintf(fp, "\n");

  for(int i=0; i<num_var_info; i++)
    {
      ff_fprintf(fp, "\n%4d:  ", i);
      fprint_var_info(fp, &(var_info[i]));
    }
  ff_fprintf(fp, "\n");
}


////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////

#if 0
void dbpfq(const char *caller, char *fmt, ...)
{
  va_list valist;
  char line[400];
  
  va_start(valist, fmt);
  ff_fprintf(ofp, "\n(%s)", caller);
  //fflush(ofp);
    
  vsprintf(line, fmt, valist);

  fprintstr(ofp, line);
  
  va_end(valist);
  //fflush(ofp);
}
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Translate an OPL program
//
////////////////////////////////////////////////////////////////////////////////

char trans_res[30] PSRAM;

int nopl_trans(char *filename)
{
  FIL f;
  FIL *fp = &f;
  FIL *vfp = NULL;

  // This is run multiple times before exiting when on a device. When on the PC it runs and exits,
  // so we have to cear these variables, or they retain their values from previous translations.

  // Conditional information
  cond_fixup_i = 0;

  first_token = 1;
  op_stack_ptr = 0;
  exp_type_stack_ptr = 0;

  node_id_index = 1;
  exp_buffer_i = 0;
  exp_buffer2_i = 0;

  indent_level = 0;
  defline = 1;

  local_info_index  = 0;
  global_info_index = 0;

  // Clear variables
  num_var_info = 0;

  n_lines_ok    = 0;
  n_lines_bad   = 0;
  n_lines_blank = 0;
  n_stack_errors = 0;
  
  init_output();
  
  ptfp = ff_fopen("parse_text.txt",  "w");
  exfp = ff_fopen("expressions.txt", "w");
  shfp = ff_fopen("shunting.txt",    "w");
    
  parser_check();

  dp_cls();

  
  // Perform two passes of translation and qcode generation
  for(pass_number = 1; pass_number<=2; pass_number++)
    {
      printf("\n%s:Pass %d\n", __FUNCTION__, pass_number);

      sprintf(trans_res, "Pass %d", pass_number);
      i_printxy_str(0, 0, trans_res);
      
      // Keep things alive
      tight_loop_tasks();

      dbprintf("********************************************************************************");
      dbprintf("**                         Pass %d                                             **", pass_number);
      dbprintf("********************************************************************************");
      
      // Open file and process on a line by line basis
      //fp = ff_fopen(filename, "r");
      FRESULT fr = f_open(&f, filename, FA_READ);
      
      if( fr != FR_OK )
	{
	  ff_fprintf(ofp, "\nCould not open '%s'", filename);
	  printf("\nCould not open '%s' (%d)", filename, fr);
          ff_fclose(ofp);
	  return(-1);
	}

      translate_file(fp, ofp);
      printf("\n%s:Pass done %d\n", __FUNCTION__, pass_number);
            
      f_close(&f);
    }

  output_qcode_suffix();

  // Fill in the conditional offsets
  do_cond_fixup();
  
  // Fill in the qcode length field
  set_qcode_header_byte_at(size_of_qcode_idx,   1, qcode_len >> 8);
  set_qcode_header_byte_at(size_of_qcode_idx+1, 1, qcode_len &  0xFF);

  dump_exp_buffer(ofp, 2);
  dump_qcode_data(filename);
  dump_cond_fixup();
  
  fclose(chkfp);
  fclose(trfp);

  vfp = fopen("vars.txt", "w");
  dump_vars(vfp);
  fclose(vfp); 
  fclose(ptfp);
  fclose(exfp);
  fclose(shfp);
  
  dbprintf("\n");
  dbprintf("\n %d lines scanned OK",       n_lines_ok);
  dbprintf("\n %d lines scanned failed",   n_lines_bad);
  dbprintf("\n %d lines blank",            n_lines_blank);
  dbprintf("\n %d variables",              num_var_info);
  dbprintf("\n");

  printf("\n %d lines scanned Ok",       n_lines_ok);
  printf("  %d lines scanned failed",    n_lines_bad);
  printf("  %d variables",               num_var_info);
  printf("  %d lines blank\n",           n_lines_blank);

  dp_cls();
  sprintf(trans_res, "%d lines",       n_lines_ok);
  i_printxy_str(0, 0, trans_res);
  sprintf(trans_res, "%d failed",    n_lines_bad);
  i_printxy_str(0, 1, trans_res);
  sprintf(trans_res, "%d vars",               num_var_info);
  i_printxy_str(0, 2, trans_res);

  uninit_output();

  kb_getk();
  
  return(0);
}

