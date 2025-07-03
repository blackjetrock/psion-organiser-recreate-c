 #define dbprintf(fmt...) dbpf(__FUNCTION__, fmt)
void dbpf(const char *caller, char *fmt, ...);
extern FILE *ptfp;


NOBJ_VARTYPE char_to_type(char ch);
char type_to_char(NOBJ_VARTYPE t);
extern NOBJ_VARTYPE expression_type;

#define MAX_EXP_TYPE_STACK  100

extern char last_line[MAX_NOPL_LINE];
extern char cline[MAX_NOPL_LINE];
extern int cline_i;
extern FILE *ofp;
extern FILE *chkfp;
extern NOBJ_VARTYPE exp_type_stack[MAX_EXP_TYPE_STACK];
extern int exp_type_stack_ptr;
extern char current_expression[200];

#define MAX_EXP_BUFFER   2000
enum
  {
    EXP_BUFF_ID_TKN = 1,
    EXP_BUFF_ID_NONE,
    EXP_BUFF_ID_SUB_START,
    EXP_BUFF_ID_SUB_END,
    EXP_BUFF_ID_VARIABLE,
    EXP_BUFF_ID_INTEGER,
    EXP_BUFF_ID_FLT,
    EXP_BUFF_ID_STR,
    EXP_BUFF_ID_BYTE,
    EXP_BUFF_ID_FUNCTION,
    EXP_BUFF_ID_OPERATOR,
    EXP_BUFF_ID_OPERATOR_UNARY,
    EXP_BUFF_ID_AUTOCON,
    EXP_BUFF_ID_COMMAND,
    EXP_BUFF_ID_PROC_CALL,
    EXP_BUFF_ID_ASSIGN,
    EXP_BUFF_ID_CONDITIONAL,
    EXP_BUFF_ID_RETURN,
    EXP_BUFF_ID_VAR_ADDR_NAME,
    EXP_BUFF_ID_PRINT,
    EXP_BUFF_ID_PRINT_SPACE,
    EXP_BUFF_ID_PRINT_NEWLINE,
    EXP_BUFF_ID_LPRINT,
    EXP_BUFF_ID_LPRINT_SPACE,
    EXP_BUFF_ID_LPRINT_NEWLINE,
    EXP_BUFF_ID_IF,
    EXP_BUFF_ID_ENDIF,
    EXP_BUFF_ID_ELSE,
    EXP_BUFF_ID_ELSEIF,
    EXP_BUFF_ID_DO,
    EXP_BUFF_ID_UNTIL,
    EXP_BUFF_ID_WHILE,
    EXP_BUFF_ID_ENDWH,
    EXP_BUFF_ID_GOTO,
    EXP_BUFF_ID_TRAP,
    EXP_BUFF_ID_LABEL,
    EXP_BUFF_ID_CONTINUE,
    EXP_BUFF_ID_BREAK,
    EXP_BUFF_ID_META,
    EXP_BUFF_ID_BRAENDIF,              // Only used in qcode generation
    EXP_BUFF_ID_TEST_EXPR,             // Only used in qcode generation
    EXP_BUFF_ID_FIELDVAR,
    EXP_BUFF_ID_LOGICALFILE,
    EXP_BUFF_ID_ONERR,
    EXP_BUFF_ID_INPUT,
    EXP_BUFF_ID_MAX,
  };

extern char *exp_buffer_id_str[];

extern int node_id_index;
extern EXP_BUFFER_ENTRY exp_buffer[MAX_EXP_BUFFER];
extern int exp_buffer_i;
extern EXP_BUFFER_ENTRY exp_buffer2[MAX_EXP_BUFFER];
extern int exp_buffer2_i;

//------------------------------------------------------------------------------


#define MAX_OPERATOR_TYPES 6
#define IMMUTABLE_TYPE     1
#define   MUTABLE_TYPE     0

typedef struct _OP_INFO
{
  char          *name;
  int           returns_result;           // Some operators do not create a result (e.g. ':=')
  int           precedence;
  NOBJ_VARTYPE  output_type;              // If not unknown then returns this type
  int           can_be_unary;             // Can be a unary operator
  int           qcode_type_from_arg;      // The qcode type comes from the arg type
  char          *unaryop;                 // Unary version of operator name
  int           left_assoc;
  int           immutable;                // Is the operator output type mutable?
  int           assignment;               // Special code for assignment
  NOBJ_VARTYPE  type[MAX_OPERATOR_TYPES];  // Acceptable argument types.
  //int           qcode;                     // Easily translatable qcodes
  int           percent_convertible;       // Can be turned into a percent operator
} OP_INFO;

extern OP_INFO  op_info[];
extern int num_operators(void);

//#define NUM_OPERATORS (sizeof(op_info)/sizeof(struct _OP_INFO))
#define NUM_OPERATORS num_operators()

void init_op_stack_entry(OP_STACK_ENTRY *op);

#define IGNORE_COMMA  1
#define HEED_COMMA    0

extern int n_lines_ok;
extern int n_lines_bad;
extern int n_lines_blank;
extern int n_stack_errors;

typedef struct _LEVEL_INFO
{
  int if_level;
  int do_level;
  int while_level;
} LEVEL_INFO;

// scan_command can scan for just trappable commands
#define SCAN_ALL          0
#define SCAN_TRAPPABLE    1




  
extern NOBJ_VAR_INFO var_info[MAX_VAR_INFO];
extern int num_var_info;

////////////////////////////////////////////////////////////////////////////////

int scan_addr_name(void);
int check_addr_name(int *index);
int scan_literal(char *lit);
int check_literal(int *index, char *lit);
int scan_line(LEVEL_INFO levels);

void indent_none(void);
int scan_expression(int *num_commas, int ignore_comma);
int check_function(int *index);
int scan_function(char *cmd_dest);
int next_composite_line(FILE *fp);
int scan_procdef(void);
int scan_cline(void);
int scan_integer(int *intdest);
int check_expression(int *index, int ignore_comma);
int check_declare(int *index);
int scan_declare(void);
void syntax_error(char *fmt, ...);
NOBJ_VARTYPE function_arg_type_n(char *fname, int n);
int function_num_args(char *fname);
NOBJ_VARTYPE function_return_type(char *fname);
int token_is_function(char *token, char **tokstr);
int token_is_float(char *token);
int token_is_integer(char *token);
int token_is_variable(char *token);
int token_is_string(char *token);
int check_expression_list(int *index, int *num_elements);
int is_all_spaces(int idx);

void finalise_expression(void);
void output_expression_start(char *expr);
void process_token(OP_STACK_ENTRY *token);
void parser_check(void);
void indent_more(void);
int scan_expression_list(int *num_expressions, int insert_types);
void initialise_line_supplier(FILE *fp);
int pull_next_line(void);
void op_stack_finalise(void);
void output_return(OP_STACK_ENTRY op);
void output_if(OP_STACK_ENTRY op);
void output_generic(OP_STACK_ENTRY op, char *name, int buf_id);
int strn_match(char *s1, char *s2, int n);
int check_proc_call(int *index);
int scan_proc_call(void);
void typecheck_error(char *fmt, ...);
void fprint_var_info(FILE *fp, NOBJ_VAR_INFO *vi);
void internal_error(char *fmt, ...);
char *type_to_str(NOBJ_VARTYPE t);
void make_var_type_array(NOBJ_VARTYPE *vt);
int var_type_is_array(NOBJ_VARTYPE vt);
void dump_qcode_data(char *opl_filename);
void build_qcode_header(void);
int set_qcode_header_byte_at(int idx, int len, int val);
int set_qcode_header_string_at(int idx, char *str);
NOBJ_VAR_INFO *find_var_info(char *name, NOBJ_VARTYPE type);
int check_operator(int *index, int *is_comma, int ignore_comma);
int function_access_force_write(char * fname);
void to_upper_str(char *str);
char *var_access_to_str(NOPL_OP_ACCESS va);
char function_arg_parse(char *fname);
EXP_BUFFER_ENTRY *find_buf2_entry_with_node_id(int node_id);
int check_eitem(int *index, int *is_comma, int ignore_comma);
int scan_eitem(int *num_commas, int ignore_comma);
void init_exp_buffer_entry(EXP_BUFFER_ENTRY *e);

extern int qcode_idx;
extern int pass_number;
extern int qcode_header_len;
extern int qcode_start_idx;
extern int qcode_len;
extern int size_of_qcode_idx;
extern int procedure_has_return;
extern int last_line_is_return;
extern NOBJ_VARTYPE  procedure_type;
void dump_vars(FILE *fp);


#define FLT_INCLUDES_SIGN   0
