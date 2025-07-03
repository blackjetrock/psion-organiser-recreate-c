#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

#include "nopl.h"

////////////////////////////////////////////////////////////////////////////////

#define LPRINT_FN "LPRINT"

FILE *lprintfp;

//------------------------------------------------------------------------------

QC_BYTE_CODE qc_byte_code[] =
  {
   
    {"v",      null_qc_byte_len_fn_2,      qc_byte_prt_fn_v},
    {"V",      null_qc_byte_len_fn_2,      qc_byte_prt_fn_V},
    {"-",      null_qc_byte_fn,            null_qc_byte_prt_fn  },
    {"m",      null_qc_byte_len_fn_2,      qc_byte_prt_fn_m},
    {"f",      null_qc_byte_len_fn_1,      qc_byte_prt_fn_f},
    {"I",      null_qc_byte_len_fn_2,      qc_byte_prt_fn_I},
    {"F",           qc_byte_len_fn_F,      qc_byte_prt_fn_F},
    {"S",           qc_byte_len_fn_S,      qc_byte_prt_fn_S},
    {"B",      null_qc_byte_len_fn_1,      qc_byte_prt_fn_B},
    {"O",      null_qc_byte_len_fn_1,      qc_byte_prt_fn_O},
    {"D",      null_qc_byte_len_fn_2,      qc_byte_prt_fn_D},
    {"f+list", null_qc_byte_fn,       null_qc_byte_prt_fn  },
  };

int qcode_sizeof_qc_byte_code = (sizeof(qc_byte_code)/sizeof(QC_BYTE_CODE));

////////////////////////////////////////////////////////////////////////////////

void dbpfq(const char *caller, char *fmt, ...)
{
  va_list valist;

  va_start(valist, fmt);
  fprintf(exdbfp, "\n(%s)", caller);
  fflush(exdbfp);
    
  vfprintf(exdbfp, fmt, valist);
  va_end(valist);
  fflush(exdbfp);
}

////////////////////////////////////////////////////////////////////////////////

#define MAX_P_STR_LEN (80*5)

char p_str[MAX_P_STR_LEN];

void p_str_start(void)
{
  strcpy(p_str, "");
}

char *p_str_get(void)
{
  return(p_str);
}

void p_str_add(char *fmt, ...)
{
  va_list valist;
  char line[160];
  
  va_start(valist, fmt);

  vsprintf(line, fmt, valist);
  strcat(p_str, line);
  
  va_end(valist);
}

////////////////////////////////////////////////////////////////////////////////

char *decode_qc_txt(int *i,  NOBJ_QCODE **qc)
{
  int found = 0;

  p_str_start();
  
  for(int j=0; j<qcode_sizeof_qcode_decode; j++)
    {
      
      if( qcode_decode[j].qcode == **qc )
	{
	  p_str_add("\n%04X: %02X       %s", *i, **qc, qcode_decode[j].desc);
	  p_str_add("     (bytes code:%s)", qcode_decode[j].bytes);
	  
	  for(int qcb = 0; qcb < qcode_sizeof_qc_byte_code; qcb++)
	    {
	      if( strcmp(qcode_decode[j].bytes, qc_byte_code[qcb].code) == 0 )
		{
		  p_str_add("%s", (*qc_byte_code[qcb].prt_fn)(*i, *qc));
		  *qc +=       (*qc_byte_code[qcb].len_fn)(*i, *qc);
		  *i +=        (*qc_byte_code[qcb].len_fn)(*i, *qc);
		}
	    }
	  
	  found = 1;
	  break;
	}
    }
  
  if( !found )
    {
      p_str_add("\n%04X: %02X ????", *i, **qc);
    }

  return(p_str);
}

////////////////////////////////////////////////////////////////////////////////
//

void qcode_get_string_push_stack(NOBJ_MACHINE *m)
{
  int len;

  len =  m->stack[(m->rta_pc)++];
  push_machine_8(m, len);

  for(int i=0; i<len; i++)
    {
      push_machine_8(m, m->stack[(m->rta_pc)++]);  
    }

}


////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////

uint16_t qcode_next_16(NOBJ_MACHINE *m)
{
  uint16_t r;

  dbq("PC:%04X", m->rta_pc);
  
  r  = (m->stack[(m->rta_pc)++]) << 8;
  r |= (m->stack[(m->rta_pc)++]);

  dbq("val:%04X", r);
  
  return(r);
}

//------------------------------------------------------------------------------
//
// Skips over the fild list and also returns a pointer for it.
// The QCode list must be copied elsewhere as the QCode could be replaced on the
// stack by another procedure.
//

uint8_t *qcode_field_list(NOBJ_MACHINE *m)
{
  uint8_t *addr_fields = &(m->stack[m->rta_pc]);
  uint8_t b;
  
  // Skip the data
  
  b = qcode_next_8(m);  // Get type

  while(b !=  QCO_END_FIELDS )
    {
      // Drop the name
      int len = qcode_next_8(m);
      for(int i=0; i<len; i++)
	{
	  qcode_next_8(m);
	}

      // Type of next field
      b = qcode_next_8(m);
    }

  return(addr_fields);
}

//------------------------------------------------------------------------------

uint8_t qcode_next_8(NOBJ_MACHINE *m)
{
  uint8_t r;

  r  = (m->stack[(m->rta_pc)++]);

  dbq("%s: %02X at %04X", __FUNCTION__, r, (m->rta_pc)-1);
  
  return(r);
}

////////////////////////////////////////////////////////////////////////////////


void qca_null(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
}

//------------------------------------------------------------------------------

void zero_num(NOPL_FLOAT *f)
{
  f->exponent = 0;
  f->sign = NUM_SIGN_POSITIVE;
  
  // Clear float
  for(int i=0; i<NUM_MAX_DIGITS; i++)
    {
      f->digits[i] = 0;
    }
}

//------------------------------------------------------------------------------


void qca_str_ind_con(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int dp = s->ind_ptr;

  // Now stack the string
  s->len = stack_entry_8(m, dp++ );

  int i;
  
  for(i=0; i<s->len; i++)
    {
      s->str[i] = stack_entry_8(m, dp++);
    }
  
  s->str[i] = '\0';
  
  push_machine_string(m, s->len, s->str);

  // Push field
  //push_machine_8(m, 0);
}

//------------------------------------------------------------------------------
//
// get the variable address and push the value as a float

void qca_num_ind_con(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int dp = s->ind_ptr+NUM_BYTE_LENGTH-1;

  // Now stack the float
  for(int i=0; i<NUM_BYTE_LENGTH; i++)
    {
      push_machine_8(m, stack_entry_8(m, dp--));
    }
}

void qca_num_arr_ind_con(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int dp = s->ind_ptr+s->arr_idx*SIZEOF_NUM+NUM_BYTE_LENGTH-1;

  // Now stack the float
  for(int i=0; i<NUM_BYTE_LENGTH; i++)
    {
      push_machine_8(m, stack_entry_8(m, dp--));
    }
}

void qca_int_qc_con(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int h, l;

  h = qcode_next_8(m);
  l = qcode_next_8(m);
  
  // Get int from qcodes and push onto stack
  push_machine_8(m, l);
  push_machine_8(m, h);
}

void qca_str_qc_con(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  
  // Get string from qcodes and push onto stack
  s->len = qcode_next_8(m);
  
  dbq("  Len:%d", s->len);

  int i;
  
  for(i=0; i<s->len; i++)
    {
      s->str[i] = qcode_next_8(m);
    }
  
  s->str[i] = '\0';
  
  push_machine_string(m, s->len, s->str);
}

//------------------------------------------------------------------------------
//
// Get compact float form and push a full float on the stack

void qca_num_qc_con(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int n;
  NOPL_FLOAT num;
  uint8_t digit[NUM_MAX_DIGITS];

  // Sign and length in first byte
  n = qcode_next_8(m);
  num.sign = n & 0x80;
  n &= 0x7f;
  
  dbq("n:%d", n);

  // Clear float
  for(int i=0; i<NUM_MAX_DIGITS; i++)
    {
      digit[i] = 0;
    }
  
  // Copy digit bytes
  // We need to reverse them
  
  for(int i=0; i<n-1; i++)
    {
      // Two digits in each byte
      digit[i] = qcode_next_8(m);
    }

  for(int i=0; i<n-1; i++)
    {
      int j = (n-1-1)-i;
      
      push_machine_8(m, digit[j]);
      
      dbq("Byte %d: j:%d digit:(%02X)", i, j, digit);
    }

  // Zero bytes on stack
  for(int i=0; i<NUM_MAX_DIGITS/2-(n-1); i++)
    {
      push_machine_8(m, 0x00);
    }

  int b = qcode_next_8(m);
  num.exponent = b;
  
  push_machine_8(m, num.exponent);
  push_machine_8(m, num.sign);

  dbq("Sign:%d (%02X) Exponent:%d (%02X)", num.sign, num.sign, num.exponent, num.exponent);
}

//------------------------------------------------------------------------------

void qca_fp(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Get pointer to string
  s->ind_ptr = qcode_next_16(m);
  
  dbq("ind ptr:%04X", s->ind_ptr);
  
  // Add to FP
  s->ind_ptr += m->rta_fp;
  
  dbq("ind_ptr: %04X", s->ind_ptr);
}

//------------------------------------------------------------------------------
//
// Calculator memory
// These are positioned at the start of the stack. The original QCode has the absolute
// address of the float after the qcode

void qca_abs(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Get pointer to float
  s->ind_ptr = qcode_next_16(m);
  
  dbq("ind_ptr: %04X", s->ind_ptr);
}

void qca_iabs(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("ind_ptr: %04X", s->integer);

  if( s->integer < 0 )
    {
      s->integer = -(s->integer);
    }

  s->result = s->integer;
}

//------------------------------------------------------------------------------
// Combination of qca_fp and qca_ind
// Done so we don't have to add another column to table
void qca_fp_ind(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Get pointer to string
  s->ind_ptr = qcode_next_16(m);
  
  dbq("ind ptr:%04X", s->ind_ptr);
  
  // Add to FP
  s->ind_ptr += m->rta_fp;

  // Then that address has the address
  s->ind_ptr = stack_entry_16(m, s->ind_ptr);
  
  dbq("ind_ptr: %04X", s->ind_ptr);
}

//------------------------------------------------------------------------------

void qca_ind(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Then that address has the address
  s->ind_ptr = stack_entry_16(m, s->ind_ptr);

}

//------------------------------------------------------------------------------

void qca_str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Get the maximum size
  s->max_sz = m->stack[s->ind_ptr-1];
  
  // Push string max length
  //push_machine_8(m, s->max_sz);
  
  // Push address of string
  push_machine_16(m, s->ind_ptr);
  
  // Push field flag
  push_machine_8(m, 0);
}

void qca_rept(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int count;
  
  // Get the repeat N  
  count = pop_machine_int(m);

  // Get the string
  pop_machine_string(m, &(s->len2), &(s->str2[0]));

  // Check result isn't too big
  if( (count * strlen(s->str2)) > NOBJ_STRING_MAXLEN)
    {
      runtime_error(ER_LX_ST, "REPT result too long");
    }

  s->str[0] = '\0';
  
  // Build the string
  for(int i=0; i<count; i++)
    {
      strcat(s->str, s->str2);
    }

  s->len = strlen(s->str);
}

//------------------------------------------------------------------------------

void qca_push_ind_addr(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Push address of integer
  push_machine_16(m, s->ind_ptr);
  
  // Push field flag
  push_machine_8(m, 0);
}

void qca_push_int_arr_addr(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Push address of integer
  push_machine_16(m, s->ind_ptr+s->arr_idx*SIZEOF_INT);
  
  // Push field flag
  push_machine_8(m, 0);
}

void qca_push_num_arr_addr(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Push address of num. Address points to first byte of num.
  // ind_ptr points to first byte of array size word.
  push_machine_16(m, s->ind_ptr+(s->arr_idx)*SIZEOF_NUM+2);
  
  // Push field flag
  push_machine_8(m, 0);
}

void qca_push_str_arr_addr(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int max_str;
  
  // Max string size is just before the array
  max_str = stack_entry_8(m, s->ind_ptr-1);

  dbq("Max str:%d idx:%d ind_ptr:%04X", max_str, s->arr_idx, s->ind_ptr);

  // Push address of string
  push_machine_16(m, s->ind_ptr+s->arr_idx*(max_str+1)+2);
  
  // Push field flag
  push_machine_8(m, 0);
}

void qca_push_ind(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Push address we calculated
  push_machine_16(m, s->ind_ptr);
}

// Pop array index
void qca_pop_idx(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Pop the array index
  s->arr_idx = pop_machine_16(m);
}

//------------------------------------------------------------------------------

void qca_push_int_at_ind(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int val = stack_entry_16(m, s->ind_ptr);

  dbq("val=%d (%04X)", val, val);
  
  // Push integer at the address we calculated
  push_machine_16(m,  val);
}

void qca_push_int_arr_at_ind(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int val = stack_entry_16(m, s->ind_ptr+s->arr_idx*SIZEOF_INT);

  dbq("val=%d (%04X)", val, val);
  
  // Push integer at the address we calculated
  push_machine_16(m,  val);
}

//------------------------------------------------------------------------------

void qca_push_num_arr_at_ind(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int np = s->ind_ptr+s->arr_idx*SIZEOF_NUM+2;
  NOPL_FLOAT num = num_from_mem(&(m->stack[np]));
				
  dbq_num("Push array num: ", &num );
  //printf("\narr_idx:%d", s->arr_idx);
  //  printf("\nind_ptr:%04X %s\n", np, num_as_text(&num, ""));
  push_machine_num(m, &num);
}

//------------------------------------------------------------------------------

void qca_push_str_arr_at_ind(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int max_str;
  uint8_t *val;

  // Max string size is just before the array
  max_str = stack_entry_8(m, s->ind_ptr-1);
  val     = &(m->stack[s->ind_ptr+s->arr_idx*(max_str+1)+2]);

  // Push string at the address we calculated
  push_machine_string(m,  *val, val+1);
}

//------------------------------------------------------------------------------

void qca_push_str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Push string at the address we calculated
  push_machine_16(m,  stack_entry_16(m, s->ind_ptr));
}

void to_lower_str(char *str)
{
  while(*str != '\0')
    {
      *str = tolower(*str);
      str++;
    }
}

void qca_push_proc(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("QCO_PROC");

  FILE *fp;
  
  // Get proc name
  s->len = qcode_next_8(m);

  dbq("  Len:%d", s->len);

  int i;
  
  for(i=0; i<s->len; i++)
    {
      s->str[i] = qcode_next_8(m);
    }

  s->str[i] = '\0';
  strcpy(s->procpath, "A:");
  strcat(s->procpath, s->str);
	  
  strcat(s->str, ".OB3");
	  
  // We have the name, open the file
  dbq("Loading PROC %s", s->str);
	  
  // Load the procedure file
  fp = fopen(s->str, "r");
	  
  if( fp == NULL )
    {
      // We failed to open the ob3 file, try a lower case version
      to_lower_str(s->str);

      // Load the procedure file
      fp = fopen(s->str, "r");
      
      if( fp == NULL )
	{
	  runtime_error(ER_RT_PN, "Cannot open '%s'", s->str);
	  dbq("Cannot open '%s'", s->str);
	  exit(-1);
	}
    }
  
  dbq("Loaded '%s'", s->str);
	  
  // Discard header
  read_ob3_header(fp);
	  
  // Initialise the machine
  //init_machine(&machine);
	  
  // Put some parameters on the stack for our test code
  //push_parameters(m);
	  
  // Push proc onto stack
  push_proc(fp, m, s->procpath, 0);

  fclose(fp);
}

void qca_push_qc_byte(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->data8 = qcode_next_8(m);
  push_machine_8(m, s->data8);
}

void qca_push_qc_word(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->integer = qcode_next_16(m);
  push_machine_16(m, s->integer);
}

void qca_raise(NOBJ_MACHINE *m, NOBJ_QCS *s)
{  
  m->error_occurred = 1;
  m->rtb_eror = pop_machine_int(m);
}

void qca_err(NOBJ_MACHINE *m, NOBJ_QCS *s)
{  
  s->result = m->rtb_eror;
}

// Set a flag that will be seen in the main exec loop

void qca_onerr(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->integer = qcode_next_16(m);
  if( s->integer == 0 )
    {
      put_stack_16(m, m->rta_fp+FP_OFF_ONERR+1, 0);
    }
  else
    {
      put_stack_16(m, m->rta_fp+FP_OFF_ONERR+1, s->integer+m->rta_pc - 2);
    }
}

void qca_unwind_proc(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Dump stack for dbq
  dbq("==============================================Unwind============================");
  display_machine(m);
	  
  // Unwind the procedure (remove it from the stack), then
  // stack a zero floating point value
  uint16_t last_fp;

  last_fp = stack_entry_16(m, (m->rta_fp)+FP_OFF_NEXT_FP);

  if( last_fp == 0 )
    {
      // There is not a previous procedure top return to
      // Ignore PC and FP, set the stack to empty
      init_sp(m, 0x3F00);       // For full example 4

      dbq("Stack reset");
    }
  else
    {
      // There is a previous procedure to return to
	      
      // To unwind the stack, set SP to the previous frame base SP
      m->rta_sp = stack_entry_16(m, last_fp+FP_OFF_BASE_SP);
	      
      // Set PC to return PC address
      m->rta_pc = stack_entry_16(m, (m->rta_fp)+FP_OFF_RETURN_PC);
    }
	  
  // Set FP to last FP
  // If zero then this signals the end of the execution
	  
  m->rta_fp = last_fp;
}

//------------------------------------------------------------------------------

void qca_ass_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Drop int
  s->integer = pop_machine_int(m);

  // Check for field
  s->field_flag = pop_machine_8(m);

  dbq("Int:%d (%04X) Addr:%04X", s->integer, s->integer, s->addr);

  if( s->field_flag )
    {
      int logfile = pop_machine_8(m);
      char int_str[20];
      
      // Drop field name string
      pop_machine_string(m, &(s->len), s->str);
      
      // Field variable
      sprintf(int_str, "%d", s->integer);
      
      logfile_put_field_as_str(m, current_logfile, s->str, int_str);
      
    }
  else
    {
      // Drop int address
      s->addr = pop_machine_16(m);

      // Assign integer to variable
      m->stack[s->addr+0] = s->integer >> 8;
      m->stack[s->addr+1] = s->integer  & 0xFF;
    }
}

//------------------------------------------------------------------------------

void qca_ass_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  uint8_t  num_bytes[NUM_BYTE_LENGTH];
  
  // Drop float bytes
  for(int i=0; i<NUM_BYTE_LENGTH; i++)
    {
      num_bytes[i] = pop_machine_8(m);
    }
  
  // Check for field
  s->field_flag = pop_machine_8(m);

  if( s->field_flag )
    {
      int logfile = pop_machine_8(m);
      
      // Drop field name string
      pop_machine_string(m, &(s->len), s->str);

      // Field variable
      NOPL_FLOAT f = num_from_mem(num_bytes);

      logfile_put_field_as_str(m, current_logfile, s->str, num_to_text(&f));
    }
  else
    {
      // Drop int address
      s->addr = pop_machine_16(m);
      
      // Copy bytes from stack to memory
      for(int i=0; i<NUM_BYTE_LENGTH; i++)
	{
	  m->stack[s->addr+i] = num_bytes[i];
	}
    }
}

//------------------------------------------------------------------------------

void qca_ass_str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Drop string
  pop_machine_string(m, &(s->len), s->str);
  
  // Check for field
  s->field_flag = pop_machine_8(m);

  dbq("field flg:%d", s->field_flag);

  if( s->field_flag )
    {
      int logfile = pop_machine_8(m);
      
      // Drop field name string
      pop_machine_string(m, &(s->len2), s->str2);

      // Field variable
      //NOPL_FLOAT f = num_from_mem(num_bytes);

      logfile_put_field_as_str(m, current_logfile, s->str2, s->str);
    }
  else
    {
      // Drop string reference
      s->str_addr = pop_machine_16(m);
      
      dbq("Str addr:%04X", s->str_addr);
      
      //  s->len      = pop_machine_8(m);
      
      // Assign string to variable
      // We are pointing at length byte. get the max length and check we won't
      // exceed that
      int max_len = m->stack[(s->str_addr)-1];
      
      dbq("Max len:%d", max_len);
      
      if( s->len > max_len )
	{
	  runtime_error(ER_LX_ST, "String too big (%d > %d)", s->len, max_len);
	  return;
	}
      
      // All OK
      // Copy data
      
      // We copy the length with the data
      // Address points to the length byte, after the max len
      m->stack[s->str_addr] = s->len;
      
      for(int i=0; i<s->len; i++)
	{
	  m->stack[s->str_addr+i+1] = s->str[i];
	}
    }
  
  // No need to zero the remaining space
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------

void qca_at(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
}

//------------------------------------------------------------------------------

void qca_cls(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  for(int i=0; i<4; i++)
    {
      printf("\n");
    }
}

//------------------------------------------------------------------------------
//

void qca_cursor(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  m->cursor_flag = qcode_next_8(m);  // Get type

  dbq("\nCursor flag now set to %d", m->cursor_flag);
}

//------------------------------------------------------------------------------
//

void qca_special(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("QCO_SPECIAL not supported, exiting");
  s->done = 1;
}

//------------------------------------------------------------------------------
//

void qca_break(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("BREAK, exiting");
  s->done = 1;
}

//------------------------------------------------------------------------------
//

void qca_clock(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int prev = m->clock_flag;

  m->clock_flag = s->integer;
  
  dbq("CLOCK: Previous state:%d, state now:%d", prev, m->clock_flag);

  s->result = prev;
}

//------------------------------------------------------------------------------
//

void qca_stop(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("STOP, exiting");
  s->done = 1;
}

//------------------------------------------------------------------------------
//

void qca_trap(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("TRAP, setting flag");
  m->rtb_trap = 1;
}

//------------------------------------------------------------------------------
//
// Allow peeks in the stack
//

#define STACK_ADDRESS(XXX) ((XXX <= NOBJ_MACHINE_STACK_HIGH) && (XXX >= NOBJ_MACHINE_STACK_LOW))

void qca_peekw(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_INT w;
  
  s->result = 0;
  
  if( STACK_ADDRESS(s->integer2) )
    {
      dbq("Stack address");

      w  = 256*(m->stack[s->integer2]);
      w += m->stack[s->integer2+1];

      s->result = w;
    }
  
  dbq("PEEKW(%04X) = %04X", s->integer2, w);

}

void qca_peekb(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_INT w;
  
  s->result = 0;
  
  if( STACK_ADDRESS(s->integer2) )
    {
      dbq("Stack address");

      w = m->stack[s->integer2];

      s->result = w;
    }
  
  dbq("PEEKW(%04X) = %04X", s->integer2, w);
}

//------------------------------------------------------------------------------

void qca_pokew(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_INT w = 0;

  dbq("POKEW(%04X, %04X)", s->integer2, s->integer);
  
  if( STACK_ADDRESS(s->integer2) )
    {
      dbq("Stack address");

      m->stack[s->integer2+0] = s->integer / 256;
      m->stack[s->integer2+1] = s->integer & 0xFF;
    }
}

//------------------------------------------------------------------------------

void qca_pokeb(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_INT w;
  
  dbq("POKEW(%04X, %04X)", s->integer2, s->integer);

  if( (s->integer > 255) || (s->integer <0))
    {
      runtime_error(ER_FN_BA, "POKEB value is not a byte");
      return;
    }
  
  if( STACK_ADDRESS(s->integer2) )
    {
      dbq("Stack address");

      m->stack[s->integer2+0] = s->integer & 0xFF;
    }
}

//------------------------------------------------------------------------------
//

void qca_escape(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  m->rta_escf = qcode_next_8(m);  // Get type

  dbq("\nEscape flag now set to %d", m->rta_escf);
}

//------------------------------------------------------------------------------

void qca_get(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int c;
  
  c = fgetc(stdin);
  
  push_machine_16(m, c);
}

//------------------------------------------------------------------------------

void qca_input_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int intval;
  int scan_ret = 0;
  
  // Get integer from user
  while(scan_ret == 0 )
    {
#if TUI
      wprintw(output_win, "?");
#else
      printf("?");
#endif      
      scan_ret = scanf("%d", &intval);
    }

  fgetc(stdin);
  
  s->integer = intval;
  
  // Check for field
  s->field_flag = pop_machine_8(m);

  dbq("Int:%d (%04X) Addr:%04X", s->integer, s->integer, s->addr);

  if( s->field_flag )
    {
      int logfile = pop_machine_8(m);
      char int_str[20];
      
      // Drop field name string
      pop_machine_string(m, &(s->len), s->str);
      
      // Field variable
      sprintf(int_str, "%d", s->integer);
      
      logfile_put_field_as_str(m, current_logfile, s->str, int_str);
      
    }
  else
    {
      // Drop int address
      s->addr = pop_machine_16(m);

      // Assign integer to variable
      m->stack[s->addr+0] = s->integer >> 8;
      m->stack[s->addr+1] = s->integer  & 0xFF;
    }
}

//------------------------------------------------------------------------------

void qca_input_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT f;
  char inp[20];
  int scan_ret = 0;
  
  // Get float from user
  while(scan_ret == 0 )
    {
#if TUI
      wprintw(output_win, "?");
#else
      printf("?");
#endif      
      
      scan_ret = scanf("%s", inp);
      f = num_from_text(inp);
    }

  fgetc(stdin);
  
  // f is set up for following code
  
  // Check for field
  s->field_flag = pop_machine_8(m);

  if( s->field_flag )
    {
      int logfile = pop_machine_8(m);
      
      // Drop field name string
      pop_machine_string(m, &(s->len), s->str);

      // Field variable
      //      NOPL_FLOAT f = num_from_mem(num_bytes);

      logfile_put_field_as_str(m, current_logfile, s->str, num_to_text(&f));
    }
  else
    {
      // Drop int address
      s->addr = pop_machine_16(m);

      num_to_mem(&f, &(m->stack[s->addr]));

#if 0	
      // Copy bytes from stack to memory
      for(int i=0; i<NUM_BYTE_LENGTH; i++)
	{
	  m->stack[s->addr+i] = num_bytes[i];
	}
#endif      
    }
}

//------------------------------------------------------------------------------

void qca_input_str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int scan_ret = 0;
  char inp[256];
  
  // Drop string
  //  pop_machine_string(m, &(s->len), s->str);
  
  // Get string from user
  //while( fgets(&(inp[0]), 254, stdin) != NULL)
  //   {
  //  }

  while(scan_ret == 0 )
    {
#if TUI
      wprintw(output_win, "?");
#else
      printf("?");
#endif      
      
      //scan_ret = scanf("%s", &(s->str));
      if( fgets(&(inp[0]), 254, stdin) != NULL )
	{
	  //printf("\nstr='%s'", inp);
	  //gets(&(s->str));
	  scan_ret = 1;

	  // Drop trailing newline
	  inp[strlen(inp)-1] = '\0';
	}
    }

  s->len = strlen(inp);
  
  // Check for field
  s->field_flag = pop_machine_8(m);

  dbq("field flg:%d", s->field_flag);

  if( s->field_flag )
    {
      int logfile = pop_machine_8(m);
      
      // Drop field name string
      pop_machine_string(m, &(s->len2), s->str2);

      // Field variable
      //NOPL_FLOAT f = num_from_mem(num_bytes);

      logfile_put_field_as_str(m, current_logfile, s->str2, inp);
    }
  else
    {
      // Drop string reference
      s->str_addr = pop_machine_16(m);
      
      dbq("Str addr:%04X", s->str_addr);
      
      //  s->len      = pop_machine_8(m);
      
      // Assign string to variable
      // We are pointing at length byte. get the max length and check we won't
      // exceed that
      int max_len = m->stack[(s->str_addr)-1];
      
      dbq("Max len:%d", max_len);
      
      if( s->len > max_len )
	{
	  runtime_error(ER_LX_ST, "String too big (%d > %d)", s->len, max_len);
	  return;
	}
      
      // All OK
      // Copy data
      
      // We copy the length with the data
      // Address points to the length byte, after the max len
      m->stack[s->str_addr] = s->len;
      
      for(int i=0; i<s->len; i++)
	{
	  m->stack[s->str_addr+i+1] = inp[i];
	}
    }
  
  // No need to zero the remaining space
}


//------------------------------------------------------------------------------

void qca_pause(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  qca_clock_pause(s->integer);
}


//------------------------------------------------------------------------------

void qca_push_zero(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Now push a zero
  push_machine_8(m, 0);
  push_machine_8(m, 0);
  push_machine_8(m, 0);
  push_machine_8(m, 0);
  push_machine_8(m, 0);
  push_machine_8(m, 0);
  push_machine_8(m, 0);
  push_machine_8(m, 0);
}

void qca_push_null(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Now push a zero
  push_machine_8(m, 0);
  push_machine_8(m, 0);
}

void qca_push_nought(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Now push a nought
  push_machine_8(m, 0);
  push_machine_8(m, 0);
}

void qca_pop_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->num = pop_machine_num(m);
}

void qca_pop_num_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->integer = pop_machine_int(m);
  s->num = pop_machine_num(m);
}

void qca_pop_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->integer = pop_machine_int(m);
}

void qca_pop_byte(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->integer = pop_machine_8(m);
}

void qca_drop_byte(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  pop_machine_8(m);
}

// Pop a variable ref, then push the address
void qca_pop_ref(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int addr;
  int field;
  
  // Field flag
  field = pop_machine_8(m);
  
  // Address
  addr = pop_machine_16(m);
  
  push_machine_16(m, addr);
  
  dbq("Field:%d (%02X) Addr:%d (%04X)", field, field, addr, addr);
}

void qca_pop_2int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("Int:%d (%04X) Int2:%d (%04X)", s->integer, s->integer, s->integer2, s->integer2);
  s->integer  = pop_machine_int(m);
  s->integer2 = pop_machine_int(m);
  dbq("Int:%d (%04X) Int2:%d (%04X)", s->integer, s->integer, s->integer2, s->integer2);
}

void qca_pop_9int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->integer  = pop_machine_int(m);
  s->integer2 = pop_machine_int(m);
  s->integer  = pop_machine_int(m);
  s->integer2 = pop_machine_int(m);
  s->integer  = pop_machine_int(m);
  s->integer2 = pop_machine_int(m);
  s->integer  = pop_machine_int(m);
  s->integer2 = pop_machine_int(m);
  s->integer  = pop_machine_int(m);
}

void qca_pop_2num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->num   = pop_machine_num(m);
  s->num2  = pop_machine_num(m);
}

//------------------------------------------------------------------------------

// LS int field reference
void qca_ls_int_fld(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int logfile;
  logfile = qcode_next_8(m);

  // Push logical file
  push_machine_8(m, logfile);

  // ...and the field flag
  push_machine_8(m, 1);
}

// LS str field reference
void qca_ls_str_fld(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int logfile;
  logfile = qcode_next_8(m);

  // Push logical file
  push_machine_8(m, logfile);

  // ...and the field flag
  push_machine_8(m, 1);
}

//------------------------------------------------------------------------------
//
// Get field and push on to stack as float
void qca_num_fld(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int logfile;
  int fld_n;
  
  logfile = qcode_next_8(m);

  // We have field flag and field name on stack
  pop_machine_string(m, &(s->len), s->str);

  //printf("\nfldname:%s", s->str);

  // Get index of field
  fld_n = logfile_get_field_index(m, logfile, s->str);
  
  if( fld_n != -1 )
    {
      // Get value
      char *fld_val_str = logfile_get_field_as_str(m, logfile, s->str);
      //printf("\nFld:'%s'", fld_val_str);
      
      // Convert to float and push
      NOPL_FLOAT f = num_from_text(fld_val_str);

      push_machine_num(m, &f);
    }
  else
    {
    }
}

//------------------------------------------------------------------------------
//
// Get field and push on to stack as float

void qca_int_fld(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int logfile;
  int fld_n;
  
  logfile = qcode_next_8(m);

  // We have field flag and field name on stack
  pop_machine_string(m, &(s->len), s->str);

  //printf("\nfldname:%s", s->str);

  // Get index of field
  fld_n = logfile_get_field_index(m, logfile, s->str);
  
  if( fld_n != -1 )
    {
      // Get value
      char *fld_val_str = logfile_get_field_as_str(m, logfile, s->str);
      //printf("\nFld:'%s'", fld_val_str);
      
      // Convert to floatint and push
      int intval;
      sscanf(fld_val_str, "%d", &intval);

      push_machine_16(m, intval);
    }
  else
    {
    }
}

//------------------------------------------------------------------------------
//
// Get field and push on to stack as float

void qca_str_fld(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int logfile;
  int fld_n;
  
  logfile = qcode_next_8(m);

  // We have field flag and field name on stack
  pop_machine_string(m, &(s->len), s->str);

  //printf("\nfldname:%s", s->str);

  // Get index of field
  fld_n = logfile_get_field_index(m, logfile, s->str);
  
  if( fld_n != -1 )
    {
      // Get value
      char *fld_val_str = logfile_get_field_as_str(m, logfile, s->str);

      //printf("\nFld:'%s'", fld_val_str);
      
      // Push string
      push_machine_string(m, strlen(fld_val_str), fld_val_str);

      //NOPL_FLOAT f = num_from_text(fld_val_str);

      //push_machine_num(m, &f);
    }
  else
    {
    }
}

//------------------------------------------------------------------------------

void qca_add(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("Int:%d (%04X) Int2:%d (%04X)", s->integer, s->integer, s->integer2, s->integer2);
  s->result = s->integer+s->integer2;
  
  dbq("Res:%d (%04X)", s->result, s->result);
}

void qca_sub(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->result = s->integer2 - s->integer;
}

void qca_mul(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->result = s->integer * s->integer2;
}

void qca_div(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->result = s->integer2 / s->integer;
}

void qca_powint(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  if( s->integer2 == 1 )
    {
      s->result = 1;
      return;
    }

  if( s->integer == 1 )
    {
      s->result = s->integer2;
      return;
    }

  if( s->integer2 == 0 )
    {
      s->result = 1;
    }

  s->result = powl(s->integer2, s->integer);
}

void qca_push_result(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("res:%d (%04X)", s->result, s->result);
  push_machine_16(m, s->result);
}

void qca_push_num_result(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq_num("Push num result: ", &(s->num_result));
  push_machine_num(m, &(s->num_result));
}

void qca_push_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("Num:%d", s->num);
  push_machine_num(m, &(s->num));
}

void qca_umin_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("Num:%d", s->num);

  if( s->num.sign == 0 )
    {
      s->num.sign = 0x80;
    }
  else
    {
      s->num.sign = 0x00;
    }
}

void qca_umin_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("Int:%d", s->integer);
  s->result = - s->integer;
  dbq("-ve Int:%d", s->integer);

}

void qca_pop_str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  pop_machine_string(m, &(s->len), s->str);
}

void qca_pop_str_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->integer = pop_machine_int(m);
  pop_machine_string(m, &(s->len), s->str);
}

void qca_pop_str_int_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->integer = pop_machine_int(m);
  s->integer2 = pop_machine_int(m);
  pop_machine_string(m, &(s->len2), s->str2);
}

void qca_pop_2str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  pop_machine_string(m, &(s->len), s->str);
  pop_machine_string(m, &(s->len2), s->str2);
}

//------------------------------------------------------------------------------
//
// PRINT prints to the screen
//
//

void qca_print_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
#if TUI
  wprintw(output_win, "%d", s->integer);
  wrefresh(output_win);
#else
  printf("%d", s->integer);
#endif


}

void qca_print_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT n = s->num;
#if TUI
  wprintw(output_win, "%s", num_to_text(&n));
  wrefresh(output_win);
#else
  printf("%s", num_to_text(&n));
#endif

}

void qca_print_str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  for(int i=0; i<s->len; i++)
    {
#if TUI
      wprintw(output_win, "%c", s->str[i]);
      wrefresh(output_win);
#else
      printf("%c", s->str[i]);
#endif
    }
}

void qca_print_cr(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
#if TUI
  wprintw(output_win, "\n");
  wrefresh(output_win);
#else
  printf("\n");
#endif
}

void qca_print_sp(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
#if TUI  
  wprintw(output_win, " ");
  wrefresh(output_win);
#else
  printf(" ");
#endif
}

//------------------------------------------------------------------------------
//
// LPRINT prints to a file called 'LPRINT'
// and to the TUI window
//
//

void lprintf(char *fmt, ...)
{
  va_list valist;
  
  va_start(valist, fmt);
  if( lprintfp != NULL )
    {
      vfprintf(lprintfp, fmt, valist);
    }
  va_end(valist);

#if TUI
  va_start(valist, fmt);

  vw_printw(printer_win, fmt, valist);
  wrefresh(printer_win);
  
  va_end(valist);
#endif
  
  fflush(lprintfp);
}

void qca_lprint_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  lprintf("%d", s->integer);
}

void qca_lprint_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT n = s->num;

  lprintf("%s", num_to_text(&n));
}

void qca_lprint_str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  for(int i=0; i<s->len; i++)
    {
      lprintf("%c", s->str[i]);
    }
}

void qca_lprint_cr(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  lprintf("\n");
}

void qca_lprint_sp(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  lprintf(" ");
}

//------------------------------------------------------------------------------

void qca_eq_str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Terminate for C
  s->str[s->len]   = '\0';
  s->str2[s->len2] = '\0';

  if( strcmp(s->str, s->str2) == 0 )
    {
      s->integer = NOBJ_TRUE;
    }
  else
    {
      s->integer = NOBJ_FALSE;
    }
}

void qca_lt_str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Terminate for C
  s->str[s->len]   = '\0';
  s->str2[s->len2] = '\0';
  
  s->integer = strcmp(s->str2, s->str) < 0;
}

void qca_gt_str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Terminate for C
  s->str[s->len]   = '\0';
  s->str2[s->len2] = '\0';
  
  s-> integer = strcmp(s->str2, s->str) > 0;
}

void qca_push_int_result(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  if( s->integer )
    {
      push_machine_16(m, NOBJ_TRUE);
    }
  else
    {
      push_machine_16(m, NOBJ_FALSE);
    }
}

void qca_push_not_int_result(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  if( s->integer )
    {
      push_machine_16(m, NOBJ_FALSE);
    }
  else
    {
      push_machine_16(m, NOBJ_TRUE);
    }
}

//------------------------------------------------------------------------------

void qca_eq_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  dbq("Int:%d (%04X) Int2:%d (%04X)", s->integer, s->integer, s->integer2, s->integer2);

  if( s->integer == s->integer2 )
    {
      res = NOBJ_TRUE;    
    }
  else
    {
      res = NOBJ_FALSE;
    }

  // Push result
  push_machine_16(m, res);
}

void qca_ne_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  if( s->integer != s->integer2 )
    {
      res = NOBJ_TRUE;    
    }
  else
    {
      res = NOBJ_FALSE;
    }

  dbq("Int:%d (%04X) Int2:%d (%04X) res:%d", s->integer, s->integer, s->integer2, s->integer2, res);
  
  // Push result
  push_machine_16(m, res);
}

void qca_gt_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  dbq("Int:%d (%04X) Int2:%d (%04X)", s->integer, s->integer, s->integer2, s->integer2);

  if( s->integer2 > s->integer )
    {
      res = NOBJ_TRUE;    
    }
  else
    {
      res = NOBJ_FALSE;
    }

  // Push result
  push_machine_16(m, res);
}

void qca_lt_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  dbq("Int:%d (%04X) Int2:%d (%04X)", s->integer, s->integer, s->integer2, s->integer2);

  if( s->integer2 < s->integer )
    {
      res = NOBJ_TRUE;    
    }
  else
    {
      res = NOBJ_FALSE;
    }

  // Push result
  push_machine_16(m, res);
}

void qca_gte_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  dbq("Int:%d (%04X) Int2:%d (%04X)", s->integer, s->integer, s->integer2, s->integer2);

  if( s->integer2 >= s->integer )
    {
      res = NOBJ_TRUE;    
    }
  else
    {
      res = NOBJ_FALSE;
    }

  // Push result
  push_machine_16(m, res);
}

void qca_lte_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  dbq("Int:%d (%04X) Int2:%d (%04X)", s->integer, s->integer, s->integer2, s->integer2);

  if( s->integer2 <= s->integer )
    {
      res = NOBJ_TRUE;    
    }
  else
    {
      res = NOBJ_FALSE;
    }

  // Push result
  push_machine_16(m, res);
}

void qca_add_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  s->result = s->integer2 + s->integer;

  dbq("Int:%d (%04X) + Int2:%d (%04X) = %d (%04X)", s->integer, s->integer, s->integer2, s->integer2, s->result, s->result);
}

void qca_and_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  s->result = s->integer2 &  s->integer;

  dbq("Int:%d (%04X) + Int2:%d (%04X) = %d (%04X)", s->integer, s->integer, s->integer2, s->integer2, s->result, s->result);
}

void qca_or_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  s->result = s->integer2 | s->integer;

  dbq("Int:%d (%04X) + Int2:%d (%04X) = %d (%04X)", s->integer, s->integer, s->integer2, s->integer2, s->result, s->result);
}

void qca_not_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  s->result = ~s->integer;

  dbq("Int:%d (%04X) + Int2:%d (%04X) = %d (%04X)", s->integer, s->integer, s->integer2, s->integer2, s->result, s->result);
}

//------------------------------------------------------------------------------

void qca_eq_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  if( num_eq(&(s->num), &(s->num2)) )
    {
      res = NOBJ_TRUE;    
    }
  else
    {
      res = NOBJ_FALSE;
    }

  // Push result
  push_machine_16(m, res);
}

void qca_ne_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  if( num_ne((&s->num), &(s->num2)) )
    {
      res = NOBJ_TRUE;    
    }
  else
    {
      res = NOBJ_FALSE;
    }

  // Push result
  push_machine_16(m, res);
}

void qca_gt_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  if( num_gt((&s->num2), &(s->num)) )
    {
      res = NOBJ_TRUE;    
    }
  else
    {
      res = NOBJ_FALSE;
    }

  // Push result
  push_machine_16(m, res);
}

void qca_lt_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  if( num_lt(&(s->num2), &(s->num)) )
    {
      res = NOBJ_TRUE;    
    }
  else
    {
      res = NOBJ_FALSE;
    }

  // Push result
  push_machine_16(m, res);
}

void qca_gte_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  if( num_gte(&(s->num2), &(s->num)) )
    {
      res = NOBJ_TRUE;    
    }
  else
    {
      res = NOBJ_FALSE;
    }

  // Push result
  push_machine_16(m, res);
}

void qca_lte_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  if( num_lte(&(s->num2), &(s->num)) )
    {
      res = NOBJ_TRUE;    
    }
  else
    {
      res = NOBJ_FALSE;
    }

  // Push result
  push_machine_16(m, res);
}

void qca_and_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;

  s->result = num_and(&(s->num), &(s->num2));

  dbq_num("num: ", &(s->num));
  dbq_num("num2:", &(s->num2));
  dbq_num("res: ", &(s->num_result));
}

void qca_or_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  s->result = num_or(&(s->num), &(s->num2));

  dbq_num("num: ", &(s->num));
  dbq_num("num2:", &(s->num2));
  dbq_num("res: ", &(s->num_result));
}

void qca_not_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  s->result = num_not(&(s->num));

  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_log_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  num_log(&(s->num), &(s->num_result));
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_exp_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  num_exp(&(s->num), &(s->num_result));
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_intf_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  num_intf(&(s->num), &(s->num_result));
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_pi_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  num_pi(&(s->num_result));
  
  dbq_num("res: ", &(s->num_result));
}

void qca_log10_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  num_log10(&(s->num), &(s->num_result));
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_sin_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  num_sin(&(s->num), &(s->num_result));
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_cos_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  num_cos(&(s->num), &(s->num_result));
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_abs_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  num_abs(&(s->num), &(s->num_result));
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_tan_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  dbq_num("num: ", &(s->num));
    
  num_tan(&(s->num), &(s->num_result));
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_asin_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  num_asin(&(s->num), &(s->num_result));
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_acos_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  num_acos(&(s->num), &(s->num_result));
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_atan_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  num_atan(&(s->num), &(s->num_result));
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_sqr_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT res = 0;
  
  num_sqr(&(s->num), &(s->num_result));
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

//------------------------------------------------------------------------------

void qca_randomize(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_INT num_i;
  
  num_num_to_int(NUM_MAX_DIGITS, &(s->num), &num_i);  

  srand(num_i);
  
  dbq_num("num: ", &(s->num));
  dbq_num("res: ", &(s->num_result));
}

void qca_rnd(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT r, rmax = (RAND_MAX & ((1<<(sizeof(NOBJ_INT)*8))-1)/2);
  //printf("\nrmax:%d %X\n", rmax, rmax);
  
  NOPL_FLOAT rf, rmaxf;
  
  r = rand() & rmax;
  num_int_to_num(NUM_MAX_DIGITS, &r, &rf);
  num_int_to_num(NUM_MAX_DIGITS, &rmax, &rmaxf);
  //printf("\nrf:   %s", num_to_text(&rf));
  //printf("\nrmaxf:%s", num_to_text(&rmaxf));
  
  // Scale to [0,1)
  num_div(&rf, &rmaxf, &(s->num_result));
  //printf("\nresult:%s", num_to_text(&(s->num_result)));
  dbq_num("rmaxf: ", &rmaxf);
  dbq_num("r:     ", &rf);
}

//------------------------------------------------------------------------------
//
// Percentage qcodes
//

NOPL_FLOAT calculate_percent_fraction(NOPL_FLOAT p)
{
  NOPL_FLOAT r, f100;

  num_100(&f100);
  
  num_div(&p, &f100, &r);
  
  return(r);
}

void qca_plus_per(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT p, v;

  //printf("\nnum:%s", num_to_text(&(s->num)));
  //printf("\nnum2:%s", num_to_text(&(s->num2)));
  
  p = calculate_percent_fraction(s->num);
  num_mul(&(s->num2), &p, &v);
  
  //printf("\np:%s", num_to_text(&p));
  //printf("\n");
    
  num_add(&(s->num2), &v, &(s->num_result));
}

void qca_minus_per(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT p, v;

  //printf("\nnum:%s", num_to_text(&(s->num)));
  // printf("\nnum2:%s", num_to_text(&(s->num2)));
  
  p = calculate_percent_fraction(s->num);
  num_mul(&(s->num2), &p, &v);
  
  //printf("\np:%s", num_to_text(&p));
  //printf("\n");

  num_sub(&(s->num2), &v, &(s->num_result));
}

void qca_times_per(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT p;

  //printf("\nnum:%s", num_to_text(&(s->num)));
  //printf("\nnum2:%s", num_to_text(&(s->num2)));
  
  p = calculate_percent_fraction(s->num);
  
  //printf("\np:%s", num_to_text(&p));
  //printf("\n");

  num_mul(&(s->num2), &p, &(s->num_result));
}

void qca_div_per(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT p;

  //printf("\nnum:%s", num_to_text(&(s->num)));
  //printf("\nnum2:%s", num_to_text(&(s->num2)));
  
  p = calculate_percent_fraction(s->num);
  
  //printf("\np:%s", num_to_text(&p));
  //printf("\n");

  num_div(&(s->num2), &p, &(s->num_result));
}

void qca_gt_per(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT p, v, f1;

  p = calculate_percent_fraction(s->num);
  num_1(&f1);

  // Add one to get multiple
  num_add(&f1, &p, &v);
  num_div(&(s->num2), &v, &(s->num_result));

}

void qca_lt_per(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT p, v, f1;

  // Calculate the amount before percentage increase
  qca_gt_per(m, s);

  // Subtract that from the amount after increase
  v = s->num_result;
  
  num_sub(&(s->num2), &v, &(s->num_result));

}


//------------------------------------------------------------------------------

void qca_val(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Convert string to float
  s->num_result = num_from_text(s->str);

}

void qca_hex(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Convert string to float
  sprintf(s->str, "%X", s->integer);
  s->len = strlen(s->str);
  s->integer = 4;
  qca_right(m, s);
}

//------------------------------------------------------------------------------

void qca_int_to_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Convert int to float form

  
  num_int_to_num(NUM_MAX_DIGITS, &(s->integer), &(s->num_result));

}

void qca_num_to_int(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Convert float to int.

  num_num_to_int(NUM_MAX_DIGITS, &(s->num), &(s->result));  
}

//------------------------------------------------------------------------------

void qca_add_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  num_add(&(s->num2), &(s->num), &(s->num_result));
  dbq_num("num: ", &(s->num));
  dbq_num("num2:", &(s->num2));
  dbq_num("res: ", &(s->num_result));
}

void qca_sub_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  num_sub(&(s->num2), &(s->num), &(s->num_result));
  dbq_num("num: ", &(s->num));
  dbq_num("num2:", &(s->num2));
  dbq_num("res: ", &(s->num_result));
  dbq_num_exploded("res: ", &(s->num_result));
}

void qca_mul_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  num_mul(&(s->num2), &(s->num), &(s->num_result));
  dbq_num("num: ", &(s->num));
  dbq_num("num2:", &(s->num2));
  dbq_num("res: ", &(s->num_result));
}

void qca_div_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  num_div(&(s->num2), &(s->num), &(s->num_result));
  dbq_num("num: ", &(s->num));
  dbq_num("num2:", &(s->num2));
  dbq_num("res: ", &(s->num_result));
}

void qca_pow_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  num_pow(&(s->num2), &(s->num), &(s->num_result));
  dbq_num("num: ", &(s->num));
  dbq_num("num2:", &(s->num2));
  dbq_num("res: ", &(s->num_result));
}

////////////////////////////////////////////////////////////////////////////////

void qca_asc(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->result = s->str[0];
}

void qca_len(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->result = strlen(s->str);
}

void qca_loc(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  char *answer = strstr(s->str2, s->str);

  if( answer == NULL )
    {
      s->result = 0;
    }
  else
    {
      s->result = answer - s->str2;
    }
}

void qca_bra_false(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOBJ_INT flag;
  int16_t offset;

  // Get flag to test
  flag = pop_machine_16(m);

  // Get offset
  offset = m->stack[m->rta_pc++];
  offset <<= 8;
  offset |= m->stack[m->rta_pc++];
  
  dbq("Flag:(%04X) offset %d (%04X)", flag, offset, offset);

  if( flag == 0 )
    {
      m->rta_pc += offset -2;
    }
}

void qca_goto(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int16_t offset;

  // Get offset
#if 0
  offset = m->stack[m->rta_pc++];
  offset <<= 8;
  offset |= m->stack[m->rta_pc++];
#else
  offset = qcode_next_16(m);
#endif
  dbq("Offset %d (%04X)", offset, offset);

  m->rta_pc += offset -2;
}

//------------------------------------------------------------------------------

void qca_off(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  dbq("OFF: Not supported");
}


////////////////////////////////////////////////////////////////////////////////
//

void qca_chr(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  //
  // We have an int, build a string that has one character with the ascii code
  // of the int.

  s->str[0] = (s->integer) & 0xFF;
  s->len = 1;
}

void qca_fix(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int w, decpl;
  
  // Pop arguments
  w = pop_machine_int(m);
  decpl = pop_machine_int(m);
  qca_pop_num(m, s);

  num_round_up(&(s->num), decpl);

  // Convert float to string
  strcpy(s->str, num_to_text(&(s->num)));

  //printf("\nw=%d, dp=%d len=%d, s=%s", w, decpl, s->len, s->str);
  num_set_decimal_places(s->str, decpl);
    
  //printf("\nw=%d, dp=%d len=%d, s=%s\n", w, decpl, s->len, s->str);

  num_force_str_to_width(s->str, w);

  // If in scientific notation then it was too big so force to asterisks
  num_force_sci_asterisk(s->str);
  
  s->len = strlen(s->str);
}

void qca_gen(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT f;
  int a, b;
  
  // Pop arguments
  b = pop_machine_int(m);
  a = pop_machine_int(m);

  qca_pop_num(m, s);
  
  f = s->num;

  // Convert float to string
  strcpy(s->str, num_to_text(&(s->num)));
  s->len = strlen(s->str);

  if( s->len > b )
    {
      s->len = b;

      for(int i= 0; i< s->len; i++)
	{
	  s->str[i] = '*';
	}
    }
}

void qca_sci(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT f;
  int w, decpl;
  
  // Pop arguments
  w     = pop_machine_int(m);
  decpl = pop_machine_int(m);
  qca_pop_num(m, s);
  
  f = s->num;

  // Convert float to string
  strcpy(s->str, num_to_sci_text(&(s->num), decpl));

  //num_set_decimal_places(s->str, decpl);
  num_force_str_to_width(s->str, w);
  
  s->len = strlen(s->str);
}

//------------------------------------------------------------------------------

void qca_left(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  //printf("\nLen:%d n:%d\n", s->len, s->integer);
  if( s->integer > NOBJ_STRING_MAXLEN )
    {
      s->integer = NOBJ_STRING_MAXLEN;
    }
  
  if( s->len > s->integer )
    {
      s->len = s->integer;
    }
}

void qca_right(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  //printf("\nLen:%d n:%d\n", s->len, s->integer);

  if( s->integer > NOBJ_STRING_MAXLEN )
    {
      s->integer = NOBJ_STRING_MAXLEN;
    }

  // Get RH n characters
  
  if( s->len > s->integer )
    {
      // We need only N characters, shift them over, lose the first
      // len - N characters
      int remlen = s->len - s->integer;

      for(int i=0; i<s->integer; i++)
	{
	  s->str[i] = s->str[remlen+i];
	}
      
      s->len = s->integer;
    }
}

void qca_mid(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Pull out integer characters starting at position integer2

  int from, to;

  //printf("\nfrom:%d count:%d", s->integer2, s->integer);
  //printf("\nstr2:'%s'", s->str2);

  for(from=s->integer2-1, to=0; (from < (s->len2)) && (to < s->integer); from++, to++)
    {
      s->str[to] = s->str2[from];
    }

  s->str[to] = '\0';
  s->len = strlen(s->str);
  
}

void qca_num(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  strcpy(s->str, num_to_int_text(&(s->num), s->integer));
  s->len = strlen(s->str);
}

//------------------------------------------------------------------------------

void qca_lower(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  for(int i=0; i<s->len; i++)
    {
      s->str[i] = tolower(s->str[i]);
    }
}

void qca_upper(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  for(int i=0; i<s->len; i++)
    {
      s->str[i] = toupper(s->str[i]);
    }
}


//------------------------------------------------------------------------------

void qca_push_string(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  push_machine_string(m, s->len, s->str);
}

void qca_add_str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  if( (s->len + s->len2) > NOBJ_STRING_MAXLEN )
    {
      runtime_error(ER_LX_ST, "String too long");
    }

  s->len = s->len + s->len2;
  strcat(s->str2, s->str);
  strcpy(s->str, s->str2);
}

////////////////////////////////////////////////////////////////////////////////
//
// Files
//

#define OPEN   (create_nopen == 0 )
#define CREATE (create_nopen == 1 )


//------------------------------------------------------------------------------
//
// Open and create are similar so we use the same function here.
//
// The datapack to use is in the filename before the colon
// the filename on the pack is after the colon.
//
// TRAP used
//

void qca_create_open(int create_nopen, NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  uint8_t  logfile;
  uint8_t  *flist;
  
  // Get the filename etc

  pop_machine_string(m, &(s->len), s->str);

  logfile = qcode_next_8(m);
  
  current_logfile = logfile;
    
  if( logfile >= NOPL_NUM_LOGICAL_FILES )
    {
      trappable_runtime_error(ER_RT_FO, "Bad file ID: %d", logfile);
      return;
    }
  
  flist = qcode_field_list(m);
  
  // Store the field names and file name for later
  logfile_store_field_names(m, logfile, flist);

  // Put some delimiters in the buffer
  int num_fields = logical_file_info[logfile].num_field_names;

  for(int i=0; i<num_fields-1; i++)
    {
      logical_file_info[logfile].buffer[i] = NOPL_FIELD_DELIMITER;
    }

  logical_file_info[logfile].buffer_size = num_fields-1;

#if 0
  // Device id
  logical_file_info[logfile].device_id = *(s->str)-'A';
  
  // Name part
  strcpy(logical_file_info[logfile].name, (s->str)+2);
#else
  strcpy(logical_file_info[logfile].name, (s->str)+0);
#endif
  
  if( OPEN )
    {
      dbq("OPEN");
      dbq("Logfile:%d", logfile);

      if( logical_file_info[logfile].open )
	{
	  trappable_runtime_error(ER_RT_FO, "Bad file ID: %d", logfile);
	}

      // Open the file
      fl_open(logfile);
      
      logical_file_info[logfile].open = 1;
      flb_rect = logical_file_info[logfile].rec_type;
    }
  
  if( CREATE )
    {
      dbq("CREATE");
      dbq("Logfile:%d", logfile);
      
      if( logical_file_info[logfile].open )
	{
	  trappable_runtime_error(ER_RT_FO, "Bad file ID: %d", logfile);
	}
      
      logical_file_info[logfile].rec_type =  fl_cret(logfile, 0);
      flb_rect = logical_file_info[logfile].rec_type;
    }


}

//------------------------------------------------------------------------------

void qca_create(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  qca_create_open(1, m, s);
}

//------------------------------------------------------------------------------

void qca_open(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  qca_create_open(0, m, s);
}

//------------------------------------------------------------------------------

void qca_rtf_pos(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  s->result = fl_rpos();
}

//------------------------------------------------------------------------------

void qca_rtf_exist(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  if( fl_exist(s->str) )
    {
      s->result = NOBJ_TRUE;
    }
  else
    {
      s->result = NOBJ_FALSE;
    }
}

//------------------------------------------------------------------------------

void qca_count(NOBJ_MACHINE *m, NOBJ_QCS *s)
{

  PAK_ADDR first_free;
  int num_recs;
  int bytes_free;

  // Get currently open file details
  fl_size(&bytes_free, &num_recs, &first_free);

  s->result = num_recs;
}

//------------------------------------------------------------------------------

void qca_use(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  uint8_t  logfile;
  
  // set logical file
  logfile = qcode_next_8(m);
  
  current_logfile = logfile;
    
  if( logfile >= NOPL_NUM_LOGICAL_FILES )
    {
      runtime_error(ER_RT_FO, "Bad file ID: %d", logfile);
      return;
    }

  // Get the current record type for the file
  fl_rect(logical_file_info[current_logfile].rec_type);

  // Set current record type
  flb_rect = logical_file_info[logfile].rec_type;

}  

void qca_append(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Save data in file 0x90 (MAIN)
  fl_rect(logical_file_info[current_logfile].rec_type);
  fl_writ(logical_file_info[current_logfile].buffer, logical_file_info[current_logfile].buffer_size);
}

// Position at record 1 then read the record
void qca_first(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  PAK_ADDR    pak_addr;
  FL_REC_TYPE rectype;
  int         reclen;

  // Position at first record
  //fl_frec(1, &pak_addr, &rectype, &reclen);
  flw_crec = 1;
  
  // read record into buffer
  fl_read(logical_file_info[current_logfile].buffer);
}

// Position at record 1 then read the record
void qca_position(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  PAK_ADDR    pak_addr;
  FL_REC_TYPE rectype;
  int         reclen;

  // Position at popped position
  
  flw_crec =  pop_machine_int(m);
  
  // read record into buffer
  fl_read(logical_file_info[current_logfile].buffer);
}

// Position at last record then read the record
void qca_last(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int bytes_free;
  PAK_ADDR first_free;
  int num_recs;
  
  fl_size(&bytes_free, &num_recs, &first_free);
  flw_crec = num_recs;
  
  // read record into buffer
  fl_read(logical_file_info[current_logfile].buffer);
}

void qca_next(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  fl_next();

  // read record into buffer
  fl_read(logical_file_info[current_logfile].buffer);
}

void qca_back(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  fl_back();

  // read record into buffer
  fl_read(logical_file_info[current_logfile].buffer);
}


void qca_close(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
}

//------------------------------------------------------------------------------

void qca_rtf_menu(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  // Drop field name string
  pop_machine_string(m, &(s->len), s->str);

  push_machine_16(m, mn_menu(s->str));
}

void qca_rtf_menun(NOBJ_MACHINE *m, NOBJ_QCS *s)
{

  // Drop field name string
  pop_machine_string(m, &(s->len), s->str);

  // Ignore N for now
  pop_machine_int(m);

  push_machine_16(m, mn_menu(s->str));
}

//------------------------------------------------------------------------------

void qca_rtf_max(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int flist_flag;
  int count;

  NOPL_FLOAT max;
  uint16_t num_i;
  NOPL_FLOAT x;

  zero_num(&max);
  
  flist_flag = pop_machine_8(m);

  dbq("flist_flg:%d", flist_flag);
  
  switch(flist_flag)
    {
    case 0:
      // Array address and count
      count = pop_machine_16(m);
      
      // Lose the field flag, we can't take the max of field variables.
      pop_machine_8(m);
      
      // Ary reference points to array size word, skip over that
      num_i = pop_machine_16(m);

      dbq("Count = %d\n", count);
      
      // Read all the floats and find the maximum value
      for(int i=0; i<count; i++, num_i+=8)
	{
	  x = num_from_mem(&(m->stack[num_i]));
	  
	  dbq_num("n:   %s", &x);
	  dbq_num("MAX: %s", &max);
	  if( num_gt(&x, &max) || (i==0))
	    {
	      max = x;
	    }
	}

      dbq_num("Result MAX:%s", &max);
      s->num_result = max;
      break;
      
    case 1:
      // List of numbers on stack

      // Get count
      count = pop_machine_8(m);
      
      dbq("Count = %d\n", count);
      
      // Read all the floats and find the maximum value
      for(int i=0; i<count; i++, num_i+=8)
	{
	  x = pop_machine_num(m);
	  
	  dbq_num("n:  %s", &x);
	  dbq_num("MAX:%s", &max);
	  if( num_gt(&x, &max) || (i==0))
	    {
	      max = x;
	    }
	}

      dbq_num("Result MAX:%s", &max);
      s->num_result = max;

      break;
    }
}

//------------------------------------------------------------------------------

void qca_rtf_min(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int flist_flag;
  int count;

  NOPL_FLOAT min;
  uint16_t num_i;
  NOPL_FLOAT x;

  zero_num(&min);
  
  flist_flag = pop_machine_8(m);

  dbq("flist_flg:%d", flist_flag);
  
  switch(flist_flag)
    {
    case 0:
      // Array address and count
      count = pop_machine_16(m);
      
      // Lose the field flag, we can't take the min of field variables.
      pop_machine_8(m);
      
      // Ary reference points to array size word, skip over that
      num_i = pop_machine_16(m);

      dbq("Count = %d\n", count);
      
      // Read all the floats and find the minimum value
      for(int i=0; i<count; i++, num_i+=8)
	{
	  x = num_from_mem(&(m->stack[num_i]));
	  
	  dbq_num("n:  %s", &x);
	  dbq_num("MIN:%s", &min);
	  if( num_lt(&x, &min) || (i==0))
	    {
	      min = x;
	    }
	}

      dbq_num("Result MIN:%s", &min);
      s->num_result = min;
      break;
      
    case 1:
      // List of numbers on stack

      // Get count
      count = pop_machine_8(m);
      
      dbq("Count = %d\n", count);
      
      // Read all the floats and find the minimum value
      for(int i=0; i<count; i++, num_i+=8)
	{
	  x = pop_machine_num(m);
	  
	  dbq_num("n:  %s", &x);
	  dbq_num("MIN:%s", &min);
	  if( num_lt(&x, &min) || (i==0))
	    {
	      min = x;
	    }
	}

      dbq_num("Result MIN:%s", &min);
      s->num_result = min;

      break;
    }
}

//------------------------------------------------------------------------------

// Convert from radians to degrees
// multiply by 57.29


void qca_rtf_deg(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT n, a, p;

  a = pop_machine_num(m);
  num_180_div_pi(&p);
  
  num_mul(&a, &p, &n);
  push_machine_num(m, &n);
}

void qca_rtf_rad(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT n, a, p;

  a = pop_machine_num(m);
  num_180_div_pi(&p);
  
  num_div(&a, &p, &n);
  push_machine_num(m, &n);
}

//------------------------------------------------------------------------------
//

void qca_dow(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_INT d, mo, y;

  y = pop_machine_int(m);
  mo = pop_machine_int(m);
  d = pop_machine_int(m);

  push_machine_16(m, time_dow(d, mo, y));
}

char *dayname[7] =
  {
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    "Sun"
  };

void qca_dayname(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_INT d;

  dbq("Day Name");
  
  d = s->integer;

  dbq("d:", d);
  
  if( d < 1 )
    {
      d = 1;
    }

  if( d > 7 )
    {
      d = 7;
    }

  dbq("Dayname='%s'", dayname[d-1]);
  
  strcpy(s->str, dayname[d-1]);
  s->len = strlen(s->str);
}

void qca_week(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_INT d, mo, y;

  y = pop_machine_int(m);
  mo = pop_machine_int(m);
  d = pop_machine_int(m);

  push_machine_16(m, time_week(d, mo, y));
}

////////////////////////////////////////////////////////////////////////////////
//
// Returns a float, despite the examples saying it returns an int. If it was an int then
// 2025 would not be a possible return vaue

void qca_days(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_INT d, mo, y;

  y = pop_machine_int(m);
  mo = pop_machine_int(m);
  d = pop_machine_int(m);

  // Calculate Julian day numbers
  int jdn = time_jdn(d, mo, y);
  int jdn_1_1_1900 = time_jdn(1, 1, 1900);

  // Work out the difference (int32 has sufficient rnge)
  int32_t diff = jdn - jdn_1_1_1900;
  NOPL_FLOAT num_diff;
  
  // Convert to float
  num_int32_to_num(NUM_MAX_DIGITS, &diff, &num_diff);

  dbq_num("num_diff: ", &num_diff);
   
  push_machine_num(m, &num_diff);
}

char *mname[12] =
  {
    "JAN",    
    "FEB",    
    "MAR",    
    "APR",    
    "MAY",    
    "JUN",    
    "JUL",    
    "AUG",    
    "SEP",    
    "OCT",    
    "NOV",    
    "DEC",    
  };

void qca_month_str(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  if( (s->integer < 1) || (s->integer > 12) )
    {
      runtime_error(ER_TM_MN, "Month %d out of range", s->integer);
    }

  strcpy(s->str, mname[s->integer-1]);
  s->len = strlen(s->str);
}

//------------------------------------------------------------------------------
//
// Calculates sum of a list of values.
//
// Used by RTF_MEAN and RTF_VAR
//

void qca_rtf_sum(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  int flist_flag;
  NOPL_FLOAT sum;
  uint16_t num_i;
  NOPL_FLOAT x;
  int count;
  
  zero_num(&sum);
  
  flist_flag = pop_machine_8(m);

  //dbq("flist_flg:%d", flist_flag);

  // Save flag for later codes
  s->flist_flag = flist_flag;
  
  switch(flist_flag)
    {
    case 0:
      // Array address and count
      count = pop_machine_16(m);
      
      // Lose the field flag, we can't take the sum of field variables.
      pop_machine_8(m);
      
      // Ary reference points to array size word, skip over that
      num_i = pop_machine_16(m);

      // Save for use by other RTF codes
      s->num_i = num_i;
      
      //dbq("Count = %d\n", count);
      
      // Read all the floats and find the sumimum value
      for(int i=0; i<count; i++, num_i+=8)
	{
	  x = num_from_mem(&(m->stack[num_i]));
	  
	  //dbq_num("n:   %s", &x);
	  //dbq_num("SUM: %s", &sum);
	  num_add(&x, &sum, &sum);
	}

      //dbq_num("Result SUM:%s", &sum);
      s->integer = count;
      s->num_result = sum;
      break;
      
    case 1:
      // List of numbers on stack

      // Get count
      count = pop_machine_8(m);
      
      dbq("Count = %d\n", count);

      // We store the stack pointer here so that in a subsequent qcode
      // we can re-pop the vlues. This is used in the variance QCode for example
      // which calculates over two passes.

      mark_aux_stack(m);
      
      // Read all the floats and find the sumimum value
      for(int i=0; i<count; i++, num_i+=8)
	{
	  x = pop_machine_num(m);
	  
	  //dbq_num("n:  %s", &x);
	  //dbq_num("SUM:%s", &sum);
	  num_add(&x, &sum, &sum);
	}

      //dbq_num("Result SUM:%s", &sum);
      s->integer = count;
      s->num_result = sum;

      break;
    }

  s->count = count;
}


void qca_mean(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  NOPL_FLOAT f_count;
  NOPL_FLOAT f_r;
  
  num_int_to_num(NUM_MAX_DIGITS, &(s->integer), &f_count);

  num_div(&(s->num_result), &f_count, &f_r);
  s->num_result = f_r;
}

//------------------------------------------------------------------------------
//
// Calculates the variance, using a previous call to rtf_sum
// from which the mean is calculated

void qca_rtf_var_nstd(NOBJ_MACHINE *m, NOBJ_QCS *s, int var_nstd)
{
  NOPL_FLOAT sum;
  uint16_t num_i;
  NOPL_FLOAT x, d;

  // We must have calculated the sum previously, so calculate the mean
  qca_mean(m, s);

  // Mean is now in num_result

  //printf("\nMean:%s", num_to_text(&(s->num_result)));
  
  zero_num(&sum);
  
  //  s->flist_flag = pop_machine_8(m);

  //printf("flist_flg:%d", s->flist_flag);
  
  switch(s->flist_flag)
    {
    case 0:
      // Array address and count
      //      s->count = pop_machine_16(m);
      
      // Lose the field flag, we can't take the sum of field variables.
      //     pop_machine_8(m);
      
      // Ary reference points to array size word, skip over that
      num_i = s->num_i;

      dbq("Count = %d\n", s->count);
      
      // Read all the floats and find the variance
      for(int i=0; i<s->count; i++, num_i+=8)
	{
	  x = num_from_mem(&(m->stack[num_i]));

	  //printf("\nx:%s", num_to_text(&x));
	  dbq_num("x:   %s", &x);
	  dbq_num("SUM: %s", &sum);

	  // Subtract mean from sample
	  num_sub(&x, &(s->num_result), &d);

	  //printf("\ndiff:%s", num_to_text(&d));
	  
	  // Square
	  num_mul(&d, &d, &x);

	  //printf("\nsq:%s", num_to_text(&x));
	  
	  // Accumulate
	  num_add(&x, &sum, &d);
	  sum = d;
	}

      //printf("\nsumsq:%s", num_to_text(&sum));
      dbq_num("Result SUM:%s", &sum);
      s->integer = s->count;
      s->num_result = sum;
      break;
      
    case 1:
      // List of numbers on stack

      // Get count
      //      s->count = pop_machine_8(m);
      
      dbq("Count = %d\n", s->count);

      // Aux stack has been set up so we can pop the floats off the stack again.
      // This only works if nothing is pushed on to the stack from the mark to
      // this point.

      // Read all the floats and find the sumimum value
      for(int i=0; i<s->count; i++, num_i+=8)
	{
	  x = pop_aux_num(m);
	  
	  dbq_num("x:   %s", &x);
	  dbq_num("SUM: %s", &sum);

	  // Subtract mean from sample
	  num_sub(&x, &(s->num_result), &d);

	  // Square
	  num_mul(&d, &d, &x);

	  // Accumulate
	  num_add(&x, &sum, &d);
	  sum = d;
	}

      dbq_num("Result SUM:%s", &sum);
      s->integer = s->count;
      s->num_result = sum;

      break;
    }

  // Now calculate the variance by dividing by (N-1)
  NOPL_FLOAT f_count, f_nm1;
  NOPL_FLOAT f_one = {0, 0, {1,0,0,0,0,0,0,0,0,0,0,0}};
  NOPL_INT icnt;

  icnt = (NOPL_INT)s->count;
  num_int_to_num(NUM_MAX_DIGITS, &icnt, &f_count);

  num_sub(&f_count, &f_one, &f_nm1);
  dbq_num("N-1:%s", &f_nm1);

  num_div(&sum, &f_nm1, &x);
  dbq_num("Variance:%s", &x);

  printf("\nvar_nstd:%d", var_nstd);
  if( !var_nstd )
    {
      // Calculate the standard deviation
      s->num = x;
      printf("\nX:%s", num_to_text(&x));
      qca_sqr_num(m, s);
      printf("\nX:%s", num_to_text(&(s->num_result)));
    }
  else
    {
      s->num_result = x;
    }
}

void qca_rtf_var(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  qca_rtf_var_nstd(m, s, 1);
}

void qca_rtf_std(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  qca_rtf_var_nstd(m, s, 0);
}

////////////////////////////////////////////////////////////////////////////////
//
//

NOBJ_QCODE_INFO qcode_info[] =
  {
    { QI_INT_SIM_FP,     "QI_INT_SIM_FP",     {qca_fp,           qca_null,        qca_push_int_at_ind}},
    { QI_NUM_SIM_FP,     "QI_NUM_SIM_FP",     {qca_fp,           qca_null,        qca_num_ind_con}},
    { QI_STR_SIM_FP,     "QI_STR_SIM_FP",     {qca_fp,           qca_null,        qca_str_ind_con}},
    { QI_INT_ARR_FP,     "QI_INT_ARR_FP",     {qca_fp,           qca_pop_idx,     qca_push_int_arr_at_ind}},
    { QI_NUM_ARR_FP,     "QI_NUM_ARR_FP",     {qca_fp,           qca_pop_idx,     qca_push_num_arr_at_ind}},
    { QI_STR_ARR_FP,     "QI_STR_ARR_FP",     {qca_fp,           qca_pop_idx,     qca_push_str_arr_at_ind}},
    { QI_NUM_SIM_ABS,    "QI_NUM_SIM_ABS",    {qca_abs,          qca_null,        qca_num_ind_con}},
    { QI_INT_SIM_IND,    "QI_INT_SIM_IND",    {qca_fp,           qca_ind,         qca_push_int_at_ind}},
    { QI_NUM_SIM_IND,    "QI_NUM_SIM_IND",    {qca_fp,           qca_ind,         qca_num_ind_con}},
    { QI_STR_SIM_IND,    "QI_STR_SIM_IND",    {qca_fp,           qca_ind,         qca_str_ind_con}},
    { QI_INT_ARR_IND,    "QI_INT_ARR_IND",    {qca_fp_ind,       qca_pop_idx,     qca_push_int_arr_at_ind}}, // test
    { QI_NUM_ARR_IND,    "QI_NUM_ARR_IND",    {qca_fp_ind,       qca_pop_idx,     qca_push_num_arr_at_ind}}, // test
    { QI_STR_ARR_IND,    "QI_STR_ARR_IND",    {qca_fp_ind,       qca_pop_idx,     qca_push_str_arr_at_ind}}, // test
    { QI_LS_INT_SIM_FP,  "QI_LS_INT_SIM_FP",  {qca_fp,           qca_null,        qca_push_ind_addr }},
    { QI_LS_NUM_SIM_FP,  "QI_LS_NUM_SIM_FP",  {qca_fp,           qca_null,        qca_push_ind_addr}},
    { QI_LS_STR_SIM_FP,  "QI_LS_STR_SIM_FP",  {qca_fp,           qca_null,        qca_str }},
    { QI_LS_INT_ARR_FP,  "QI_LS_INT_ARR_FP",  {qca_fp,           qca_pop_idx,     qca_push_int_arr_addr }},
    { QI_LS_NUM_ARR_FP,  "QI_LS_NUM_ARR_FP",  {qca_fp,           qca_pop_idx,     qca_push_num_arr_addr }},
    { QI_LS_STR_ARR_FP,  "QI_LS_STR_ARR_FP",  {qca_fp,           qca_pop_idx,     qca_push_str_arr_addr }},
    { QI_LS_NUM_ARR_FP,  "QI_LS_NUM_ARR_FP",  {qca_fp,           qca_pop_idx,     qca_push_num_arr_addr }},
    { QI_LS_NUM_SIM_ABS, "QI_LS_NUM_SIM_ABS", {qca_abs,          qca_null,        qca_push_ind_addr }},
    { QI_LS_INT_SIM_IND, "QI_LS_INT_SIM_FP",  {qca_fp,           qca_ind,         qca_push_ind_addr }},
    { QI_LS_NUM_SIM_IND, "QI_LS_NUM_SIM_FP",  {qca_fp,           qca_ind,         qca_push_ind_addr}},
    { QI_LS_STR_SIM_IND, "QI_LS_STR_SIM_IND", {qca_fp,           qca_ind,         qca_str }},
    { QI_LS_INT_ARR_IND, "QI_LS_INT_ARR_IND", {qca_fp_ind,       qca_pop_idx,     qca_push_int_arr_addr }}, // test
    { QI_LS_NUM_ARR_IND, "QI_LS_NUM_ARR_IND", {qca_fp_ind,       qca_pop_idx,     qca_push_num_arr_addr }}, // test
    { QI_LS_STR_ARR_IND, "QI_LS_STR_ARR_IND", {qca_fp_ind,       qca_pop_idx,     qca_push_str_arr_addr }}, // test

    { QI_INT_FLD,        "QI_INT_FLD",        {qca_int_fld,      qca_null,        qca_null }},           // QI_INT_FLD              0x1A
    { QI_NUM_FLD,        "QI_NUM_FLD",        {qca_num_fld,      qca_null,        qca_null }},           // 1B
    { QI_STR_FLD,        "QI_STR_FLD",        {qca_str_fld,      qca_null,        qca_null }},           // QI_STR_FLD              0x1C
    { QI_LS_INT_FLD,     "QI_LS_INT_FLD",     {qca_ls_int_fld,   qca_null,        qca_null }},
    { QI_LS_NUM_FLD,     "QI_LS_NUM_FLD",     {qca_ls_int_fld,   qca_null,        qca_null }},
    { QI_LS_STR_FLD,     "QI_LS_STR_FLD",     {qca_ls_int_fld,   qca_null,        qca_null }},           // QI_LS_STR_FLD           0x1F    
    { QI_STK_LIT_BYTE,   "QI_STK_LIT_BYTE",   {qca_null,         qca_null,        qca_push_qc_byte}},    // QI_STK_LIT_BYTE         0x20
    { QI_STK_LIT_WORD,   "QI_STK_LIT_WORD",   {qca_null,         qca_null,        qca_push_qc_word}},    // QI_STK_LIT_WORD         0x21
    { QI_INT_CON,        "QI_INT_CON",        {qca_null,         qca_null,        qca_int_qc_con}},
    { QI_NUM_CON,        "QI_NUM_CON",        {qca_null,         qca_null,        qca_num_qc_con}},
    { QI_STR_CON,        "QI_STR_CON",        {qca_null,         qca_null,        qca_str_qc_con}},
    { QCO_SPECIAL,       "QCO_SPECIAL",       {qca_special,      qca_null,        qca_null}},            // QCO_SPECIAL             0x25    
    { QCO_BREAK,         "QCO_BREAK",         {qca_break,        qca_null,        qca_null}},            // QCO_BREAK               0x26
    { QCO_LT_INT,        "QI_LT_INT",         {qca_pop_2int,     qca_lt_int,      qca_null}},
    { QCO_LTE_INT,       "QI_LTE_INT",        {qca_pop_2int,     qca_lte_int,     qca_null}},
    { QCO_GT_INT,        "QI_GT_INT",         {qca_pop_2int,     qca_gt_int,      qca_null}},
    { QCO_GTE_INT,       "QI_GTE_INT",        {qca_pop_2int,     qca_gte_int,     qca_null}},
    { QCO_NE_INT,        "QI_NE_INT",         {qca_pop_2int,     qca_ne_int,      qca_null}},
    { QCO_EQ_INT,        "QI_EQ_INT",         {qca_pop_2int,     qca_eq_int,      qca_null}},
    { QCO_ADD_INT,       "QCO_ADD_INT",       {qca_pop_2int,     qca_add,         qca_push_result}},
    { QCO_SUB_INT,       "QCO_SUB_INT",       {qca_pop_2int,     qca_sub,         qca_push_result}},
    { QCO_MUL_INT,       "QCO_MUL_INT",       {qca_pop_2int,     qca_mul,         qca_push_result}},
    { QCO_DIV_INT,       "QCO_DIV_INT",       {qca_pop_2int,     qca_div,         qca_push_result}},
    { QCO_POW_INT,       "QCO_POW_INT",       {qca_pop_2int,     qca_powint,      qca_push_result}},       // QCO_POW_INT             0x31    
    { QCO_UMIN_INT,      "QCO_UMIN_INT",      {qca_pop_int,      qca_umin_int,    qca_push_result}},
    { QCO_NOT_INT,       "QCO_NOT_INT",       {qca_pop_int,      qca_not_int,     qca_push_result}},
    { QCO_AND_INT,       "QCO_AND_INT",       {qca_pop_2int,     qca_and_int,     qca_push_result}},
    { QCO_OR_INT,        "QCO_OR_INT",        {qca_pop_2int,     qca_or_int,      qca_push_result}},
    { QCO_LT_NUM,        "QI_LT_NUM",         {qca_pop_2num,     qca_lt_num,      qca_null}},
    { QCO_LTE_NUM,       "QI_LTE_NUM",        {qca_pop_2num,     qca_lte_num,     qca_null}},
    { QCO_GT_NUM,        "QI_GT_NUM",         {qca_pop_2num,     qca_gt_num,      qca_null}},
    { QCO_GTE_NUM,       "QI_GTE_NUM",        {qca_pop_2num,     qca_gte_num,     qca_null}},
    { QCO_NE_NUM,        "QI_NE_NUM",         {qca_pop_2num,     qca_ne_num,      qca_null}},
    { QCO_EQ_NUM,        "QI_EQ_NUM",         {qca_pop_2num,     qca_eq_num,      qca_null}},
    { QCO_ADD_NUM,       "QCO_ADD_NUM",       {qca_pop_2num,     qca_add_num,     qca_push_num_result}},
    { QCO_SUB_NUM,       "QCO_SUB_NUM",       {qca_pop_2num,     qca_sub_num,     qca_push_num_result}},
    { QCO_MUL_NUM,       "QCO_MUL_NUM",       {qca_pop_2num,     qca_mul_num,     qca_push_num_result}},
    { QCO_DIV_NUM,       "QCO_DIV_NUM",       {qca_pop_2num,     qca_div_num,     qca_push_num_result}},
    { QCO_POW_NUM,       "QCO_POW_NUM",       {qca_pop_2num,     qca_pow_num,     qca_push_num_result}},     // QCO_POW_NUM             0x40    
    { QCO_UMIN_NUM,      "QCO_UMIN_NUM",      {qca_pop_num,      qca_umin_num,    qca_push_num}},
    { QCO_NOT_NUM,       "QCO_NOT_NUM",       {qca_pop_num,      qca_not_num,     qca_push_result}},
    { QCO_AND_NUM,       "QCO_AND_NUM",       {qca_pop_2num,     qca_and_num,     qca_push_result}},
    { QCO_OR_NUM,        "QCO_OR_NUM",        {qca_pop_2num,     qca_or_num,      qca_push_result}},
    { QCO_LT_STR,        "QI_LT_STR",         {qca_pop_2str,     qca_lt_str,      qca_push_int_result}},        // QCO_LT_STR              0x45    
    { QCO_LTE_STR,       "QI_LTE_STR",        {qca_pop_2str,     qca_gt_str,      qca_push_not_int_result}},    // QCO_LTE_STR             0x46    
    { QCO_GT_STR,        "QI_GT_STR",         {qca_pop_2str,     qca_gt_str,      qca_push_int_result}},        // QCO_GT_STR              0x47    
    { QCO_GTE_STR,       "QI_GTE_STR",        {qca_pop_2str,     qca_lt_str,      qca_push_not_int_result}},    // QCO_GTE_STR             0x48    
    { QCO_NE_STR,        "QI_NE_STR",         {qca_pop_2str,     qca_eq_str,      qca_push_not_int_result}},    // QCO_NE_STR              0x49    
    { QCO_EQ_STR,        "QI_EQ_STR",         {qca_pop_2str,     qca_eq_str,      qca_push_int_result}},
    { QCO_ADD_STR,       "QI_ADD_STR",        {qca_pop_2str,     qca_add_str,     qca_push_string}},
    { QCO_AT,            "QI_AT",             {qca_pop_2int,     qca_null,        qca_null}},
    { QCO_BEEP,          "QCO_BEEP",          {qca_pop_2int,     qca_null,        qca_null}},
    { QCO_CLS,           "QCO_CLS",           {qca_cls,          qca_null,        qca_null}},    // QCO_CLS                 0x4E    
    { QCO_CURSOR,        "QCO_CURSOR",        {qca_cursor,       qca_null,        qca_null}},    // QCO_CURSOR              0x4F    
    { QCO_ESCAPE,        "QCO_ESCAPE",        {qca_escape,       qca_null,        qca_null}},    // QCO_ESCAPE              0x50    
    { QCO_GOTO,          "QCO_GOTO",          {qca_goto,         qca_null,        qca_null}},
    { QCO_OFF,           "QCO_OFF",           {qca_off,          qca_null,        qca_null}},    // QCO_OFF                 0x52    
    { QCO_ONERR,         "QCO_ONERR",         {qca_onerr,        qca_null,        qca_null}},    // QCO_ONERR               0x53    
    { QCO_PAUSE,         "QCO_PAUSE",         {qca_pop_int,      qca_pause,       qca_null}},    // QCO_PAUSE               0x54
    { QCO_POKEB,         "QCO_POKEB",         {qca_pop_2int,     qca_pokeb,       qca_null}},    // QCO_POKEB               0x55    
    { QCO_POKEW,         "QCO_POKEW",         {qca_pop_2int,     qca_pokew,       qca_null}},    // QCO_POKEW               0x56    
    { QCO_RAISE,         "QCO_RAISE",         {qca_raise,        qca_null,        qca_null}},    // QCO_RAISE               0x57    
    { QCO_RANDOMIZE,     "QCO_RANDOMIZE",     {qca_pop_num,      qca_randomize,   qca_null}},    // QCO_RANDOMIZE           0x58    
    { QCO_STOP,          "QCO_STOP",          {qca_stop,         qca_null,        qca_null}},    // QCO_STOP                0x59    
    { QCO_TRAP,          "QCO_TRAP",          {qca_trap,         qca_null,        qca_null}},    // QCO_TRAP                0x5A    
    { QCO_APPEND,         "QCO_APPEND",         {qca_append,      qca_null,       qca_null}},    // QCO_APPEND              0x5B    
    { QCO_CLOSE,          "QCO_CLOSE",          {qca_close,       qca_null,       qca_null}},     // QCO_CLOSE               0x5C    
    // QCO_COPY                0x5D    
    { QCO_CREATE,         "QCO_CREATE",         {qca_create,      qca_null,       qca_null}},
    // QCO_DELETE              0x5F    
    // QCO_ERASE               0x60
    { QCO_FIRST,          "QCO_FIRST",          {qca_first,       qca_null,       qca_null}},    // QCO_FIRST               0x61    
    { QCO_LAST,           "QCO_LAST",           {qca_last,        qca_null,       qca_null}},    // QCO_LAST                0x62    
    { QCO_NEXT,           "QCO_NEXT",           {qca_next,        qca_null,       qca_null}},    // QCO_NEXT                0x63    
    { QCO_BACK,           "QCO_BACK",           {qca_back,        qca_null,       qca_null}},    // QCO_BACK                0x64
    { QCO_OPEN,           "QCO_OPEN",           {qca_open,        qca_null,       qca_null}},    // QCO_OPEN                0x65    
    { QCO_POSITION,       "QCO_POSITION",       {qca_position,    qca_null,       qca_null}},    // QCO_POSITION            0x66    
    // QCO_RENAME              0x67    
    // QCO_UPDATE              0x68
    { QCO_USE,            "QCO_USE",            {qca_use,         qca_null,       qca_null}},  // QCO_USE                 0x69    
    // QCO_KSTAT               0x6A    
    // QCO_EDIT                0x6B    
    { QCO_INPUT_INT,      "QCO_INPUT_INT",      {qca_input_int,   qca_null,       qca_null}},    // QCO_INPUT_INT           0x6C    
    { QCO_INPUT_NUM,      "QCO_INPUT_NUM",      {qca_input_num,   qca_null,       qca_null}},    // QCO_INPUT_NUM           0x6D    
    { QCO_INPUT_STR,      "QCO_INPUT_STR",      {qca_input_str,   qca_null,       qca_null}},    // QCO_INPUT_STR           0x6E
    { QCO_PRINT_INT,     "QCO_PRINT_INT",     {qca_pop_int,      qca_print_int,   qca_null}},
    { QCO_PRINT_NUM,     "QCO_PRINT_NUM",     {qca_pop_num,      qca_print_num,   qca_null}},
    { QCO_PRINT_STR,     "QCO_PRINT_STR",     {qca_pop_str,      qca_print_str,   qca_null}},
    { QCO_PRINT_SP,      "QCO_PRINT_SP",      {qca_null,         qca_print_sp,    qca_null}},
    { QCO_PRINT_CR,      "QCO_PRINT_CR",      {qca_null,         qca_print_cr,    qca_null}},
    { QCO_LPRINT_INT,    "QCO_LPRINT_INT",    {qca_pop_int,      qca_lprint_int,  qca_null}},    // QCO_LPRINT_INT          0x74    
    { QCO_LPRINT_NUM,    "QCO_LPRINT_NUM",    {qca_pop_num,      qca_lprint_num,  qca_null}},    // QCO_LPRINT_NUM          0x75    
    { QCO_LPRINT_STR,    "QCO_LPRINT_STR",    {qca_pop_str,      qca_lprint_str,  qca_null}},    // QCO_LPRINT_STR          0x76    
    { QCO_LPRINT_SP,     "QCO_LPRINT_SP",     {qca_null,         qca_lprint_sp,   qca_null}},    // QCO_LPRINT_SP           0x77    
    { QCO_LPRINT_CR,     "QCO_LPRINT_CR",     {qca_null,         qca_lprint_cr,   qca_null}},    // QCO_LPRINT_CR           0x78
    { QCO_RETURN,        "QCO_RETURN",        {qca_unwind_proc,  qca_null,        qca_null}},
    { QCO_RETURN_NOUGHT, "QCO_RETURN_NOUGHT", {qca_unwind_proc,  qca_push_nought, qca_null}},
    { QCO_RETURN_ZERO,   "QCO_RETURN_ZERO",   {qca_unwind_proc,  qca_push_zero,   qca_null}},
    { QCO_RETURN_NULL,   "QCO_RETURN_NULL",   {qca_unwind_proc,  qca_push_null,   qca_null}},
    { QCO_PROC,          "QCO_PROC",          {qca_push_proc,    qca_push_null,   qca_null}},
    { QCO_BRA_FALSE,     "QCO_BRA_FALSE",     {qca_bra_false,    qca_null,        qca_null}},
    { QCO_ASS_INT,       "QCO_ASS_INT",       {qca_ass_int,      qca_null,        qca_null}},
    { QCO_ASS_STR,       "QCO_ASS_STR",       {qca_ass_str,      qca_null,        qca_null}},
    { QCO_ASS_NUM,       "QCO_ASS_NUM",       {qca_ass_num,      qca_null,        qca_null}},
    { QCO_DROP_BYTE,     "QCO_DROP_BYTE",     {qca_drop_byte,    qca_null,        qca_null}},    // QCO_DROP_BYTE           0x82    
    { QCO_DROP_WORD,     "QCO_DROP_WORD",     {qca_pop_int,      qca_null,        qca_null}},
    { QCO_DROP_NUM,      "QCO_DROP_NUM",      {qca_pop_num,      qca_null,        qca_null}},
    { QCO_DROP_STR,      "QCO_DROP_STR",      {qca_pop_str,      qca_null,        qca_null}},
    { QCO_INT_TO_NUM,    "QCO_INT_TO_NUM",    {qca_pop_int,      qca_int_to_num,  qca_push_num_result}},
    { QCO_NUM_TO_INT,    "QCO_NUM_TO_INT",    {qca_pop_num,      qca_num_to_int,  qca_push_result}},
    // QCO_END_FIELDS          0x88      *** Should never be executed 
    // QCO_RUN_ASSEM           0x89      *** Assembly not supported
    { RTF_ADDR,          "RTF_ADDR",          {qca_pop_ref,      qca_null,        qca_null}},
    { RTF_ASC,           "RTF_ASC",           {qca_pop_str,      qca_asc,         qca_push_result}},
    { RTF_DAY,           "RTF_DAY",           {qca_clock_day,    qca_null,        qca_null}},
    // RTF_DISP                0x8D    
    { RTF_ERR,           "RTF_ERR",           {qca_err,          qca_null,        qca_push_result}},    // RTF_ERR    0x8E    
    // RTF_FIND                0x8F    
    // RTF_FREE                0x90    
    { RTF_GET,           "RTF_GET",           {qca_null,         qca_get,         qca_null}},    // RTF_GET                 0x91    
    { RTF_HOUR,          "RTF_HOUR",          {qca_clock_hour,   qca_null,        qca_null}},
    { RTF_IABS,          "RTF_IABS",          {qca_pop_int,      qca_iabs,        qca_push_result}},    // RTF_IABS                0x93    
    { RTF_INT,           "RTF_INT",           {qca_pop_num,      qca_num_to_int,  qca_push_result}},    // RTF_INT                 0x94    
    // RTF_KEY                 0x95
    { RTF_LEN,           "RTF_LEN",           {qca_pop_str,      qca_len,         qca_push_result}},
    { RTF_LOC,           "RTF_LOC",           {qca_pop_2str,     qca_loc,         qca_push_result}},
    { RTF_MENU,          "RTF_MENU",          {qca_rtf_menu,     qca_null,        qca_null}},          // RTF_MENU                0x98
    { RTF_MINUTE,        "RTF_MINUTE",        {qca_clock_minute, qca_null,        qca_null}},
    { RTF_MONTH,         "RTF_MONTH",         {qca_clock_month,  qca_null,        qca_null}},
    { RTF_PEEKB,         "RTF_PEEKB",         {qca_pop_int,      qca_peekb,       qca_push_result}},    // RTF_PEEKB               0x9B    
    { RTF_PEEKW,         "RTF_PEEKW",         {qca_pop_int,      qca_peekw,       qca_push_result}},   // RTF_PEEKW               0x9C    
    // RTF_RECSIZE             0x9D
    { RTF_SECOND,        "RTF_SECOND",        {qca_clock_second, qca_null,        qca_null}},
    // RTF_IUSR                0x9F    
    // RTF_VIEW                0xA0    
    { RTF_YEAR,          "RTF_YEAR",          {qca_clock_year,   qca_null,        qca_null}},
    { RTF_COUNT,         "RTF_COUNT",         {qca_count,        qca_null,        qca_push_result}},    // RTF_COUNT               0xA2    
    // RTF_EOF                 0xA3    
    { RTF_EXIST,         "RTF_EXIST",         {qca_pop_str,      qca_rtf_exist,   qca_push_result}},    // RTF_EXIST               0xA4    
    { RTF_POS,           "RTF_POS",           {qca_rtf_pos,      qca_null,        qca_push_result}},  // RTF_POS                 0xA5
    { RTF_ABS,           "RTF_ABS",           {qca_pop_num,      qca_abs_num,     qca_push_num_result}},
    { RTF_ATAN,          "RTF_ATAN",          {qca_pop_num,      qca_atan_num,    qca_push_num_result}},
    { RTF_COS,           "RTF_COS",           {qca_pop_num,      qca_cos_num,     qca_push_num_result}},
    { RTF_DEG,           "RTF_DEG",           {qca_null,         qca_rtf_deg,     qca_null}},               // RTF_DEG                 0xA9    
    { RTF_EXP,           "RTF_EXP",           {qca_pop_num,      qca_exp_num,     qca_push_num_result}},    // RTF_EXP                 0xAA    
    { RTF_FLT,           "RTF_FLT",           {qca_pop_int,      qca_int_to_num,  qca_push_num_result}},    // RTF_FLT                 0xAB    
    { RTF_INTF,          "RTF_INTF",          {qca_pop_num,      qca_intf_num,    qca_push_num_result}},    // RTF_INTF                0xAC
    { RTF_LN,            "RTF_LN",            {qca_pop_num,      qca_log_num,     qca_push_num_result}},    // RTF_LN                  0xAD 
    { RTF_LOG,           "RTF_LOG",           {qca_pop_num,      qca_log10_num,   qca_push_num_result}},    // RTF_LOG                 0xAE
    { RTF_PI,            "RTF_PI",            {qca_null,         qca_pi_num,      qca_push_num_result}},    // RTF_PI                  0xAF    
    { RTF_RAD,           "RTF_RAD",           {qca_null,         qca_rtf_rad,     qca_null}},               // RTF_RAD                 0xB0    
    { RTF_RND,           "RTF_RND",           {qca_null,         qca_rnd,     qca_push_num_result}},    // RTF_RND                 0xB1    
    { RTF_SIN,           "RTF_SIN",           {qca_pop_num,      qca_sin_num,     qca_push_num_result}},
    { RTF_SQR,           "RTF_SQR",           {qca_pop_num,      qca_sqr_num,     qca_push_num_result}},
    { RTF_TAN,           "RTF_TAN",           {qca_pop_num,      qca_tan_num,     qca_push_num_result}},
    { RTF_VAL,           "RTF_VAL",           {qca_pop_str,      qca_val,         qca_push_num_result}},    // RTF_VAL                 0xB5
    
    // RTF_SPACE               0xB6    
    // RTF_DIR                 0xB7    
    { RTF_CHR,           "RTF_CHR",           {qca_pop_int,      qca_chr,         qca_push_string}},    // RTF_CHR                 0xB8
    // RTF_DATIM               0xB9    
    // RTF_SERR                0xBA    
    { RTF_FIX,           "RTF_FIX",           {qca_null,         qca_fix,         qca_push_string}},         // RTF_FIX                 0xBB    
    { RTF_GEN,           "RTF_GEN",           {qca_pop_num,      qca_gen,         qca_push_string}},         // RTF_GEN                 0xBC    
    // RTF_SGET                0xBD    
    { RTF_HEX,           "RTF_HEX",           {qca_pop_int,      qca_hex,         qca_push_string}},         // RTF_HEX                 0xBE    
    
    // RTF_SKEY                0xBF    
    { RTF_LEFT,           "RTF_LEFT",         {qca_pop_str_int,     qca_left,          qca_push_string}},    // RTF_LEFT                0xC0    
    { RTF_LOWER,          "RTF_LOWER",        {qca_pop_str,         qca_lower,         qca_push_string}},    // RTF_LOWER               0xC1    
    { RTF_MID,            "RTF_MID",          {qca_pop_str_int_int, qca_mid,           qca_push_string}},    // RTF_MID                 0xC2    
    { RTF_NUM,            "RTF_NUM",          {qca_pop_num_int,     qca_num,           qca_push_string}},    // RTF_NUM                 0xC3    
    { RTF_RIGHT,          "RTF_RIGHT",        {qca_pop_str_int,     qca_right,         qca_push_string}},    // RTF_RIGHT               0xC4    
    { RTF_REPT,           "RTF_REPT",         {qca_null,            qca_rept,          qca_push_string}},    // RTF_REPT                0xC5    
    { RTF_SCI,            "RTF_SCI",          {qca_null,            qca_sci,           qca_push_string}},    // RTF_SCI                 0xC6    
    { RTF_UPPER,          "RTF_UPPER",        {qca_pop_str,         qca_upper,         qca_push_string}},    // RTF_UPPER               0xC7    
    // RTF_SUSR                0xC8    
    // RTF_SADDR               0xC9    
    //
    //////////////////////////////// LZ QCode //////////////////////////////
    //

    { RTF_LTPERCENT,          "RTF_LTPERCENT",           {qca_pop_2num,      qca_lt_per,         qca_push_num_result}},    // RTF_LTPERCENT           0xCC
    { RTF_GTPERCENT,          "RTF_GTPERCENT",           {qca_pop_2num,      qca_gt_per,         qca_push_num_result}},    // RTF_GTPERCENT           0xCD
    { RTF_PLUSPERCENT,        "RTF_PLUSPERCENT",         {qca_pop_2num,      qca_plus_per,       qca_push_num_result}},    // RTF_PLUSPERCENT         0xCE
    { RTF_MINUSPERCENT,       "RTF_MINUSPERCENT",        {qca_pop_2num,      qca_minus_per,      qca_push_num_result}},    // RTF_MINUSPERCENT        0xCF
    { RTF_TIMESPERCENT,       "RTF_TIMESPERCENT",        {qca_pop_2num,      qca_times_per,      qca_push_num_result}},    // RTF_TIMESPERCENT        0xD0
    { RTF_DIVIDEPERCENT,      "RTF_DIVIDEPERCENT",       {qca_pop_2num,      qca_div_per,        qca_push_num_result}},    // RTF_DIVIDEPERCENT       0xD1
    // RTF_OFFX                0xD2
    // RTF_COPYW               0xD3
    // RTF_DELETEW             0xD4
    { RTF_UDG,           "RTF_UDG",           {qca_pop_9int,     qca_null,        qca_null}},    // RTF_UDG                 0xD5

    { RTF_CLOCK,         "RTF_CLOCK",         {qca_pop_int,     qca_clock,        qca_push_result}},    // RTF_CLOCK               0xD6
    { RTF_DOW,           "RTF_DOW",           {qca_null,        qca_dow,          qca_null}},                // RTF_DOW                 0xD7
    // RTF_FINDW               0xD8
    { RTF_MENUN,         "RTF_MENUN",         {qca_rtf_menun,    qca_null,        qca_null}},    // RTF_MENUN               0xD9
    { RTF_WEEK,          "RTF_WEEK",          {qca_null,         qca_week,        qca_null}},    // RTF_WEEK                0xDA
    { RTF_ACOS,          "RTF_ACOS",          {qca_pop_num,      qca_acos_num,    qca_push_num_result}},
    { RTF_ASIN,          "RTF_ASIN",          {qca_pop_num,      qca_asin_num,    qca_push_num_result}},
    { RTF_DAYS,          "RTF_DAYS",          {qca_null,         qca_days,        qca_null}},               // RTF_DAYS                0xDD
    { RTF_MAX,           "RTF_MAX",           {qca_rtf_max,      qca_null,        qca_push_num_result}},    // RTF_MAX                 0xDE
    { RTF_MEAN,          "RTF_MEAN",          {qca_rtf_sum,      qca_mean,        qca_push_num_result}},    // RTF_MEAN                0xDF
    { RTF_MIN,           "RTF_MIN",           {qca_rtf_min,      qca_null,        qca_push_num_result}},    // RTF_MIN                 0xE0
    { RTF_STD,           "RTF_STD",           {qca_rtf_sum,      qca_rtf_std,     qca_push_num_result}},    // RTF_STD                 0xE1
    { RTF_SUM,           "RTF_SUM",           {qca_rtf_sum,      qca_null,        qca_push_num_result}},    // RTF_SUM                 0xE2
    { RTF_VAR,           "RTF_VAR",           {qca_rtf_sum,      qca_rtf_var,     qca_push_num_result}},    // RTF_VAR                 0xE3
    { RTF_DAYNAME,       "RTF_DAYNAME",       {qca_pop_int,      qca_dayname,     qca_push_string}},        // RTF_DAYNAME             0xE4
    // RTF_DIRW                0xE5
    { RTF_MONTHSTR,      "RTF_MONTHSTR",      {qca_pop_int,      qca_month_str,   qca_push_string}}         // RTF_MONTHSTR            0xE6
  };

#define SIZEOF_QCODE_INFO (sizeof(qcode_info)/sizeof(NOBJ_QCODE_INFO))

////////////////////////////////////////////////////////////////////////////////
  
QCODE_DESC qcode_decode[] =
  {
    //        Inline  Pull    Push
    //
    {0x00,	"v",	"-",	"I",	"Push local/global integer variable value"},
    {0x01,	"v",	"-",	"F",	"Push local/global float variable value"},
    {0x02,	"v",	"-",	"S",	"Push local/global string variable value"},
    {0x03,	"v",	"I",	"I",	"Pop integer index and push local/global integer array variable value"},
    {0x04,	"v",	"I",	"F",	"Pop integer index and push local/global float array variable value"},
    {0x05,	"v",	"I",	"S",	"Pop integer index and push local/global string array variable value"},
    {0x06,	"m",	"-",	"F",	"Push calculator memory. Is followed by a byte indicating which of the 10 memories."},
    {0x07,	"V",	"-",	"I",	"Push parameter/external integer variable value"},
    {0x08,	"V",	"-",	"F",	"Push parameter/external float variable value"},
    {0x09,	"V",	"-",	"S",	"Push parameter/external string variable value"},
    {0x0A,	"V",	"I",	"I",	"Pop integer index and push parameter/external integer array variable value"},
    {0x0B,	"V",	"I",	"F",	"Pop integer index and push parameter/external float array variable value"},
    {0x0C,	"V",	"I",	"S",	"Pop integer index and push parameter/external string array variable value"},
    {0x0D,	"v",	"-",	"i",	"Push local/global integer variable reference"},
    {0x0E,	"v",	"-",	"f",	"Push local/global float variable reference"},
    {0x0F,	"v",	"-",	"s",	"Push local/global string variable reference"},
    {0x10,	"v",	"I",	"i",	"Pop integer index and push local/global integer array variable reference"},
    {0x11,	"v",	"I",	"f",	"Pop integer index and push local/global float array variable reference"},
    {0x12,	"v",	"I",	"s",	"Pop integer index and push local/global string array variable reference"},
    {0x13,	"m",	"-",	"f",	"Push calculator memory reference. Is followed by a byte indicating which of the 10 memories."},
    {0x14,	"V",	"-",	"i",	"Push parameter/external integer variable reference"},
    {0x15,	"V",	"-",	"f",	"Push parameter/external float variable reference"},
    {0x16,	"V",	"-",	"s",	"Push parameter/external string variable reference"},
    {0x17,	"V",	"I",	"i",	"Pop integer index and push parameter/external integer array variable reference"},
    {0x18,	"V",	"I",	"f",	"Pop integer index and push parameter/external float array variable reference"},
    {0x19,	"V",	"I",	"s",	"Pop integer index and push parameter/external string array variable reference"},
    {0x1A,	"f",	"S",	"I",	"Push file field as integer. Is followed by 1 byte logical file name (0-3 for A-D)"},
    {0x1B,	"f",	"S",	"F",	"Push file field as float. Is followed by 1 byte logical file name (0-3 for A-D)"},
    {0x1C,	"f",	"S",	"S",	"Push file field as string. Is followed by 1 byte logical file name (0-3 for A-D)"},
    {0x1D,	"f",	"S",	"I",	"Push reference of file integer field. Is followed by 1 byte logical file name (0-3 for A-D)"},
    {0x1E,	"f",	"S",	"F",	"Push reference of file float field. Is followed by 1 byte logical file name (0-3 for A-D)"},
    {0x1F,	"f",	"S",	"S",	"Push reference of file string field. Is followed by 1 byte logical file name (0-3 for A-D)"},
    {0x20,	"B",	"-",	"B",	"Push byte literal"},
    {0x21,	"I",	"-",	"I",	"Push word literal (same as integer literal)"},
    {0x22,	"I",	"-",	"I",	"Push integer literal"},
    {0x23,	"F",	"-",	"F",	"Push float literal"},
    {0x24,	"S",	"-",	"S",	"Push string literal"},
    {0x25,	"-",	"-",	"-",	"Special call to machine code. Not used by the organiser's compiler."},
    {0x26,	"-",	"-",	"-",	"Calls UT$LEAV, which quits OPL?? Not used by the organiser's compiler."},
    {0x27,	"-",	"II",	"I",	"Less than integer"},
    {0x28,	"-",	"II",	"I",	"Less than equal integer"},
    {0x29,	"-",	"II",	"I",	"Greater than integer"},
    {0x2A,	"-",	"II",	"I",	"Greater than equal integer"},
    {0x2B,	"-",	"II",	"I",	"Not equal integer"},
    {0x2C,	"-",	"II",	"I",	"Equal integer"},
    {0x2D,	"-",	"II",	"I",	"Plus integer"},
    {0x2E,	"-",	"II",	"I",	"Minus integer"},
    {0x2F,	"-",	"II",	"I",	"Multiply integer"},
    {0x30,	"-",	"II",	"I",	"Divide integer"},
    {0x31,	"-",	"II",	"I",	"Power integer"},
    {0x32,	"-",	"I",	"I",	"unary minus integer"},
    {0x33,	"-",	"I",	"I",	"NOT integer"},
    {0x34,	"-",	"II",	"I",	"AND integer"},
    {0x35,	"-",	"II",	"I",	"OR integer"},
    {0x36,	"-",	"FF",	"I",	"Less than float"},
    {0x37,	"-",	"FF",	"I",	"Less than equal float"},
    {0x38,	"-",	"FF",	"I",	"Greater than float"},
    {0x39,	"-",	"FF",	"I",	"greater than equal float"},
    {0x3A,	"-",	"FF",	"I",	"Not equal float"},
    {0x3B,	"-",	"FF",	"I",	"Equal float"},
    {0x3C,	"-",	"FF",	"F",	"Plus float"},
    {0x3D,	"-",	"FF",	"F",	"Minus float"},
    {0x3E,	"-",	"FF",	"F",	"Multiply float"},
    {0x3F,	"-",	"FF",	"F",	"Divide float"},
    {0x40,	"-",	"FF",	"F",	"Power float"},
    {0x41,	"-",	"F",	"F",	"Unary minus float"},
    {0x42,	"-",	"F",	"I",	"NOT float"},
    {0x43,	"-",	"FF",	"I",	"AND float"},
    {0x44,	"-",	"FF",	"I",	"OR float"},
    {0x45,	"-",	"SS",	"I",	"Less than string"},
    {0x46,	"-",	"SS",	"I",	"Less than equal string"},
    {0x47,	"-",	"SS",	"I",	"Greater than string"},
    {0x48,	"-",	"SS",	"I",	"Greater than equal string"},
    {0x49,	"-",	"SS",	"I",	"Not equal string"},
    {0x4A,	"-",	"SS",	"I",	"Equal string"},
    {0x4B,	"-",	"SS",	"S",	"Plus string"},
    {0x4C,	"-",	"II",	"-",	"AT"},
    {0x4D,	"-",	"II",	"-",	"BEEP"},
    {0x4E,	"-",	"-",	"-",	"CLS"},
    {0x4F,	"O",	"-",	"-",	"CURSOR"},
    {0x50,	"O",	"-",	"-",	"ESCAPE"},
    {0x51,	"D",	"-",	"-",	"GOTO"},
    {0x52,	"-",	"-",	"-",	"OFF"},
    {0x53,	"D",	"-",	"-",	"ONERR"},
    {0x54,	"-",	"I",	"-",	"PAUSE"},
    {0x55,	"-",	"II",	"-",	"POKEB"},
    {0x56,	"-",	"II",	"-",	"POKEW"},
    {0x57,	"-",	"I",	"-",	"RAISE"},
    {0x58,	"-",	"F",	"-",	"RANDOMIZE"},
    {0x59,	"-",	"-",	"-",	"STOP"},
    {0x5A,	"-",	"-",	"-",	"TRAP"},
    {0x5B,	"-",	"-",	"-",	"APPEND"},
    {0x5C,	"-",	"-",	"-",	"CLOSE"},
    {0x5D,	"-",	"SS",	"-",	"COPY"},
    {0x5E,	"f+list",	"S",	"-",	"CREATE"},
    {0x5F,	"-",	"S",	"-",	"DELETE"},
    {0x60,	"-",	"-",	"-",	"ERASE"},
    {0x61,	"-",	"-",	"-",	"FIRST"},
    {0x62,	"-",	"-",	"-",	"LAST"},
    {0x63,	"-",	"-",	"-",	"NEXT"},
    {0x64,	"-",	"-",	"-",	"BACK"},
    {0x65,	"f+list",	"S",	"-",	"OPEN"},
    {0x66,	"-",	"I",	"-",	"POSITION"},
    {0x67,	"-",	"SS",	"-",	"RENAME"},
    {0x68,	"-",	"-",	"-",	"UPDATE"},
    {0x69,	"f",	"-",	"-",	"USE"},
    {0x6A,	"-",	"I",	"-",	"KSTAT"},
    {0x6B,	"-",	"s",	"-",	"EDIT"},
    {0x6C,	"-",	"i",	"-",	"INPUT integer"},
    {0x6D,	"-",	"f",	"-",	"INPUT float"},
    {0x6E,	"-",	"s",	"-",	"INPUT string"},
    {0x6F,	"-",	"I",	"-",	"PRINT integer"},
    {0x70,	"-",	"F",	"-",	"PRINT float"},
    {0x71,	"-",	"S",	"-",	"PRINT string"},
    {0x72,	"-",	"-",	"-",	"PRINT ,"},
    {0x73,	"-",	"-",	"-",	"PRINT newline"},
    {0x74,	"-",	"I",	"-",	"LPRINT integer"},
    {0x75,	"-",	"F",	"-",	"LPRINT float"},
    {0x76,	"-",	"S",	"-",	"LPRINT string"},
    {0x77,	"-",	"-",	"-",	"LPRINT ,"},
    {0x78,	"-",	"-",	"-",	"LPRINT newline"},
    {0x79,	"-",	"I/F/S",	"-",	"RETURN"},
    {0x7A,	"-",	"-",	"-",	"RETURN (integer 0)"},
    {0x7B,	"-",	"-",	"-",	"RETURN (float 0)"},
    {0x7C,	"-",	"-",	"-",	"RETURN (string "")"},
    {0x7D,	"S",	"params",	"F/I/S",	"Procedure call."},
    {0x7E,	"D",	"I",	"-",	"Branch if false"},
    {0x7F,	"-",	"iI",	"-",	"Assign integer"},
    {0x80,	"-",	"fF",	"-",	"Assign float"},
    {0x81,	"-",	"sS",	"-",	"Assign string"},
    {0x82,	"-",	"B",	"-",	"drop byte from stack"},
    {0x83,	"-",	"I",	"-",	"drop integer from stack"},
    {0x84,	"-",	"F",	"-",	"drop float from stack"},
    {0x85,	"-",	"S",	"-",	"drop string from stack"},
    {0x86,	"-",	"I",	"F",	"autoconversion int to float"},
    {0x87,	"-",	"F",	"I",	"autoconversion float to int"},
    {0x88,	"-",	"-",	"-",	"End of field list for CREATE/OPEN"},
    {0x89,	"code",	"-",	"-",	"Inline assembly. Not used by the organiser's compiler."},
    {0x8A,	"-",	"i/f",	"I",	"ADDR"},
    {0x8B,	"-",	"S",	"I",	"ASC"},
    {0x8C,	"-",	"-",	"I",	"DAY"},
    {0x8D,	"-",	"IS",	"I",	"DISP"},
    {0x8E,	"-",	"-",	"I",	"ERR"},
    {0x8F,	"-",	"S",	"I",	"FIND"},
    {0x90,	"-",	"-",	"I",	"FREE"},
    {0x91,	"-",	"-",	"I",	"GET"},
    {0x92,	"-",	"-",	"I",	"HOUR"},
    {0x93,	"-",	"I",	"I",	"IABS"},
    {0x94,	"-",	"F",	"I",	"INT"},
    {0x95,	"-",	"-",	"I",	"KEY"},
    {0x96,	"-",	"S",	"I",	"LEN"},
    {0x97,	"-",	"SS",	"I",	"LOC"},
    {0x98,	"-",	"S",	"I",	"MENU"},
    {0x99,	"-",	"-",	"I",	"MINUTE"},
    {0x9A,	"-",	"-",	"I",	"MONTH"},
    {0x9B,	"-",	"I",	"I",	"PEEKB"},
    {0x9C,	"-",	"I",	"I",	"PEEKW"},
    {0x9D,	"-",	"-",	"I",	"RECSIZE"},
    {0x9E,	"-",	"-",	"I",	"SECOND"},
    {0x9F,	"-",	"II",	"I",	"USR"},
    {0xA0,	"-",	"II",	"I",	"VIEW"},
    {0xA1,	"-",	"-",	"I",	"YEAR"},
    {0xA2,	"-",	"-",	"I",	"COUNT"},
    {0xA3,	"-",	"-",	"I",	"EOF"},
    {0xA4,	"-",	"S",	"I",	"EXIST"},
    {0xA5,	"-",	"-",	"I",	"POS"},
    {0xA6,	"-",	"F",	"F",	"ABS"},
    {0xA7,	"-",	"F",	"F",	"ATAN"},
    {0xA8,	"-",	"F",	"F",	"COS"},
    {0xA9,	"-",	"F",	"F",	"DEG"},
    {0xAA,	"-",	"F",	"F",	"EXP"},
    {0xAB,	"-",	"F",	"F",	"FLT"},
    {0xAC,	"-",	"F",	"F",	"INTF"},
    {0xAD,	"-",	"F",	"F",	"LN"},
    {0xAE,	"-",	"F",	"F",	"LOG"},
    {0xAF,	"-",	"-",	"F",	"PI"},
    {0xB0,	"-",	"F",	"F",	"RAD"},
    {0xB1,	"-",	"-",	"F",	"RND"},
    {0xB2,	"-",	"F",	"F",	"SIN"},
    {0xB3,	"-",	"F",	"F",	"SQR"},
    {0xB4,	"-",	"F",	"F",	"TAN"},
    {0xB5,	"-",	"S",	"F",	"VAL"},
    {0xB6,	"-",	"-",	"F",	"SPACE"},
    {0xB7,	"-",	"S",	"S",	"DIR$"},
    {0xB8,	"-",	"I",	"S",	"CHR$"},
    {0xB9,	"-",	"-",	"S",	"DATIM$"},
    {0xBA,	"-",	"-",	"S",	"ERR$"},
    {0xBB,	"-",	"FII",	"S",	"FIX$"},
    {0xBC,	"-",	"FI",	"S",	"GEN$"},
    {0xBD,	"-",	"-",	"S",	"GET$"},
    {0xBE,	"-",	"I",	"S",	"HEX$"},
    {0xBF,	"-",	"-",	"S",	"KEY$"},
    {0xC0,	"-",	"SI",	"S",	"LEFT$"},
    {0xC1,	"-",	"S",	"S",	"LOWER$"},
    {0xC2,	"-",	"SII",	"S",	"MID$"},
    {0xC3,	"-",	"FI",	"S",	"NUM$"},
    {0xC4,	"-",	"SI",	"S",	"RIGHT$"},
    {0xC5,	"-",	"SI",	"S",	"REPT$"},
    {0xC6,	"-",	"FII",	"S",	"SCI$"},
    {0xC7,	"-",	"S",	"S",	"UPPER$"},
    {0xC8,	"-",	"II",	"S",	"USR$"},
    {0xC9,	"-",	"s",	"I",	"ADDR (string)"},
    {0xCA,	"SI",	"-",	"-",	"Used in .LNO files by the Developer Emulator to store procedure debug info."},
    {0xCB,	"II",	"-",	"-",	"Used in .LNO files by the Developer Emulator to store line and columns number of a statement."},
    {0xCC,	"-",	"FF",	"F",	"<%"},
    {0xCD,	"-",	"FF",	"F",	">%"},
    {0xCE,	"-",	"FF",	"F",	"+%"},
    {0xCF,	"-",	"FF",	"F",	"-%"},
    {0xD0,	"-",	"FF",	"F",	"*%"},
    {0xD1,	"-",	"FF",	"F",	"/%"},
    {0xD2,	"-",	"I",	"-",	"OFFX"},
    {0xD3,	"-",	"SS",	"-",	"COPYW"},
    {0xD4,	"-",	"S",	"-",	"DELETEW"},
    {0xD5,	"-",	"IIIIIIIII",	"-",	"UDG"},
    {0xD6,	"-",	"I",	"I",	"CLOCK"},
    {0xD7,	"-",	"III",	"I",	"DOW"},
    {0xD8,	"-",	"S",	"I",	"FINDW"},
    {0xD9,	"-",	"IS",	"I",	"MENUN"},
    {0xDA,	"-",	"III",	"I",	"WEEK"},
    {0xDB,	"-",	"F",	"F",	"ACOS"},
    {0xDC,	"-",	"F",	"F",	"ASIN"},
    {0xDD,	"-",	"III",	"I",	"DAYS"},
    {0xDE,	"-",	"Flist",	"F",	"MAX"},
    {0xDF,	"-",	"Flist",	"F",	"MEAN"},
    {0xE0,	"-",	"Flist",	"F",	"MIN"},
    {0xE1,	"-",	"Flist",	"F",	"STD"},
    {0xE2,	"-",	"Flist",	"F",	"SUM"},
    {0xE3,	"-",	"Flist",	"F",	"VAR"},
    {0xE4,	"-",	"I",	"S",	"DAYNAME$"},
    {0xE5,	"-",	"S",	"S",	"DIRW$"},
    {0xE6,	"-",	"I",	"S",	"MONTH$"},
  };

int qcode_sizeof_qcode_decode = (sizeof(qcode_decode)/sizeof(QCODE_DESC));


////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////

char *qcode_desc(int qc)
{
  for(int q=0; q<qcode_sizeof_qcode_decode; q++)
    {
      if( qcode_decode[q].qcode == qc )
	{
	  return(qcode_decode[q].desc);
	}
    }
  return("???");
}

char *qcode_name(NOBJ_QCODE qcode)
{
  for(int q=0; q<SIZEOF_QCODE_INFO; q++)
    {
      if( qcode == qcode_info[q].qcode )
	{
	  return(qcode_info[q].name);
	  break;
	}
    }
  
  return("???");
}

////////////////////////////////////////////////////////////////////////////////
//
// Display frame
//
////////////////////////////////////////////////////////////////////////////////


void display_frame(NOBJ_MACHINE *m)
{
  int fp = m->rta_fp;
  
  // rta_fp points to the frame pointer entry in the frame

  int retadd   = stack_entry_16(m, fp+6);
  int onerr    = stack_entry_16(m, fp+4);
  int basesp   = stack_entry_16(m, fp+2);
  int framep   = stack_entry_16(m, fp+0);
  int globtab  = stack_entry_16(m, fp-2);

  printf("\n%04X: Return PC    : %04X", fp+6, retadd  );
  printf("\n%04X: ONERR        : %04X", fp+4, onerr   );
  printf("\n%04X: BASE SP      : %04X", fp+2, basesp  );
  printf("\n%04X: FP           : %04X", fp+0, framep  );
  printf("\n%04X: Global Table : %04X", fp-2, globtab );
  
  printf("\n");
}

////////////////////////////////////////////////////////////////////////////////
//
// Display variables
//
////////////////////////////////////////////////////////////////////////////////


void display_variables(NOBJ_MACHINE *m)
{
  int fp = m->rta_fp;
  FILE *vfp;
  char line[300];
  
  // Read the vars.txt file and display the variables
  vfp = fopen("vars.txt", "r");

  if( vfp == NULL )
    {
      printf("\nCannot open vars.txt");
      return;
    }

  int done = 0;

  int i;
  char vname[200];
  char class[200];
  char type[200];
  char decref[200];
  int max_str;
  int max_ary;
  int num_ind;
  int int_offset;
  int16_t offset;

  while(!done )
    {
      if( fgets(line, 300, vfp) == NULL )
	{
	  // we are done
	  done = 1;
	}

      // Print the line and then the value
      //printf("\n%s   ", line);

      // get information
      int scanret = sscanf(line, "%d: VAR: '%[^']' %s %s %s max_str: %d max_ary: %d num_ind: %d offset:%x",
			   &i,
			   &(vname[0]),
			   &(class[0]),
			   &(type[0]),
			   &(decref[0]),
			   &max_str,
			   &max_ary,
			   &num_ind,
			   &int_offset
			   );

      if( scanret == 9 )
	{
	  offset = (uint16_t)int_offset;

	  printf("\n%d:  VAR: '%s' %7s %10s %10s max_str:  %d max_ary:  %d num_ind:  %d offset:%d    ", 
		 i,
		 vname,
		 class,
		 type,
		 decref,
		 max_str,
		 max_ary,
		 num_ind,
		 offset
		 );

	  printf("  %04X:  ", offset+fp+2);	  
	  
	  for(int i=0; i<8; i++)
	    {
	      //	      printf("%02X ", stack_entry_8(m, fp+2+i+offset));
	      printf("%02X ", m->stack[fp+i+offset]);
	    }
	}
    }

  printf("\n");
  return;
}

////////////////////////////////////////////////////////////////////////////////
//
// Execute one QCode 
//
////////////////////////////////////////////////////////////////////////////////
//
// Single step mode aows execution to be stepped and information queried
// before execution of next instruction
//

void execute_qcode(NOBJ_MACHINE *m, int single_step)
{
  uint8_t    field_flag;
  NOBJ_QCS   s;
  int        found;
  char outline[250];
  
  s.done = 0;
  
  while(!s.done)
    {
#ifdef TUI
      tui_step(m, &(s.done));
#endif
      
      // Get the qcode using the PC from the stack
      
      s.qcode = m->stack[m->rta_pc];

      dbq("\n--------------------------------------------------------------------------------\n");
      dbq("Executing QCode %02X at %04X", s.qcode, m->rta_pc);
      
      (m->rta_pc)++;

      found = 0;
      int qci = 0;
      int q;
      
      for(q=0; q<SIZEOF_QCODE_INFO; q++)
	{
	  if( s.qcode == qcode_info[q].qcode )
	    {
	      qci = q;
	      
	      found = 1;
	      break;
	    }
	}

      if( !found )
	{
	  printf("\n\nNot found QCode: %02X (%s)\n", s.qcode, qcode_desc(s.qcode));
	  dbq("Not found so exit: %02X\n", s.qcode);
	  s.done = 1;
	  continue;
	}

      sprintf(outline, "rta_sp:%04X rta_fp:%04X rta_pc:%04X qcode:%02X %s",
	      m->rta_sp,
	      m->rta_fp,
	      m->rta_pc,
	      s.qcode,
	      qcode_info[qci].name
	      );

      dbq(outline);
      
      if( single_step )
	{
	  printf("\n%s", outline);

	  for(int i = 0; i<40-(strlen(outline) % 40); i++)
	    {
	      printf(" ");
	    }

	  printf(" %04X: ", m->rta_sp-16);
	  
	  for(int i=m->rta_sp-16; i<=m->rta_sp+16; i++)
	    {
	      if( i == m->rta_sp )
		{
		  printf("(%02X) ", m->stack[i]);
		}
	      else
		{
		  printf("%02X ", m->stack[i]);
		}
	    }
	  
	  printf(" :%04X ", m->rta_sp+16);
	  printf("\n");
	  
	  int done = 0;
	  char cmdline[250];
	  while(!done)
	    {
	      fgets(cmdline, 250, stdin);

	      if( cmdline[strlen(cmdline)-1] == '\n' )
		{
		  cmdline[strlen(cmdline)-1] = '\0';
		}
	      
	      if( strlen(cmdline)==0 )
		{
		  done = 1;
		}
	      
	      if( strcmp(cmdline, "cont") == 0 )
		{
		  done = 1;
		}

	      if( strcmp(cmdline, "f") == 0 )
		{
		  // Display frame
		  display_frame(m);
		}

	      if( strcmp(cmdline, "v") == 0 )
		{
		  // Display variables using addresses from the var.txt file
		  // which has to be from a compilation of this .opl file
		  display_variables(m);
		}
	      
	      if( strcmp(cmdline, "s") == 0 )
		{
		  // Print the stack out, around rta_sp
		  printf("\nrta_sp:%04X", m->rta_sp);
		  
		  for(int i=m->rta_sp; i<=0x3F00; i++)
		    {
		      printf("\n%04X: %02X '%c'", i, m->stack[i], isprint(m->stack[i])?m->stack[i]:' ');
		    }
		  printf("\n");
		}
	      
	      if( sscanf(cmdline, "cont") == 1 )
		{
		  done = 1;
		}
	    }
	}

      dbq("Executing %s\n", qcode_info[qci].name);
      
      // Perform actions
      for(int a=0; a<NOBJ_QC_NUM_ACTIONS; a++)
	{
	  qcode_info[qci].action[a](m, &s);
	}

      //------------------------------------------------------------------------------
      // ONERR handling
      //
      // If there is an error and an onerr handler has been defined
      // then jump to it

      int handler_address;
      
      if( m->error_occurred )
	{
	  dbq("Error occurred");

	  handler_address = machine_onerr_handler(m);
	  if( handler_address != 0 )
	    {
	      dbq("ONERR handler defined: %04X", handler_address);
	      m->rta_pc = handler_address;
	    }
	  else
	    {
	      // No handler
	      dbq("No ONERR handler defined");

	      runtime_error_print();
	      //Exit
	      return;
	    }
	  
	  // Clear this error
	  m->error_occurred = 0;
	}

      sprintf(outline, "rta_sp:%04X rta_fp:%04X rta_pc:%04X qcode:%02X %s",
	      m->rta_sp,
	      m->rta_fp,
	      m->rta_pc,
	      s.qcode,
	      qcode_info[qci].name
	      );

      dbq(outline);

      if ( m->rta_fp == 0 )
	{
	  dbq("===  Exit ====");
	  display_machine(m);
	  //exit(0);
	  return;
	  // Exit?
	}
    }
}
// prt fn called first, then len fn

int null_qc_byte_len_fn_2(int i, NOBJ_QCODE *qc)
{
  return(2);
}

int null_qc_byte_len_fn_1(int i, NOBJ_QCODE *qc)
{
  return(1);
}

char prt_res[NOBJ_PRT_MAX_LINE];
int qc_len = 0;

char *qc_byte_prt_fn_v(int i, NOBJ_QCODE *qc)
{
  sprintf(prt_res, "\n%04X: %02X%02X       (%d)", i+1, *(qc+1), *(qc+2), (*(qc+1))*256+*(qc+2));
  return(prt_res);
}


char *qc_byte_prt_fn_V(int i, NOBJ_QCODE *qc)
{
  sprintf(prt_res, "\n%04X: %02X%02X       (%d)", i+1, *(qc+1), *(qc+2), (*(qc+1))*256+*(qc+2));
  return(prt_res);
}

char *qc_byte_prt_fn_B(int i, NOBJ_QCODE *qc)
{
  sprintf(prt_res, "\n%04X: %02X           (%d)", i+1, *(qc+1), *(qc+1));
  return(prt_res);
}

char *qc_byte_prt_fn_O(int i, NOBJ_QCODE *qc)
{
  sprintf(prt_res, "\n%04X: %02X           (%d)", i+1, *(qc+1), *(qc+1));
  return(prt_res);
}

char *qc_byte_prt_fn_f(int i, NOBJ_QCODE *qc)
{
  sprintf(prt_res, "\n%04X: %02X           (%c)", i+1, *(qc+1), 'A'+*(qc+1));
  return(prt_res);
}

char *qc_byte_prt_fn_m(int i, NOBJ_QCODE *qc)
{
  sprintf(prt_res, "\n%04X: %02X%02X       (M%d)", i+1, *(qc+1), *(qc+2), ((*(qc+1))*256+(*(qc+2)))/8 );
  return(prt_res);
}

char *qc_byte_prt_fn_I(int i, NOBJ_QCODE *qc)
{
  sprintf(prt_res, "\n%04X: %02X%02X       (%d)", i+1, *(qc+1), *(qc+2), (*(qc+1))*256+*(qc+2));
  return(prt_res);
}

char *qc_byte_prt_fn_D(int i, NOBJ_QCODE *qc)
{
  // Where we go for a distance of 0
  // Instruction is 3 bytes long and distance of 0002 takes us to next instruction.
  
  int dest = i + 3 - 2;
  int16_t  distance = (*(qc+1))*256+*(qc+2);
  
  dest += distance;
  
  sprintf(prt_res, "\n%04X: Dest:%04X (Dist:%04X)", i+1, dest, distance);

  return(prt_res);
}

char *qc_byte_prt_fn_S(int i, NOBJ_QCODE *qc)
{
  qc_len = *(++qc);
  
  char chs[2];
  chs[1] = '\0';

  sprintf(prt_res, "\n%04X: Len:%d\n%04X: '", i+1, *(qc++), i+2);
  
  for(int j=0; j<qc_len; j++)
    {
      //printf("\nj:%d", j);
      chs[0] = *(qc++);

      strcat(prt_res, chs);
    }

  strcat(prt_res, "'");

  return(prt_res);
}

int qc_byte_len_fn_S(int i, NOBJ_QCODE *qc)
{
  return(qc_len+1);
}

// Compact floating point form

char *qc_byte_prt_fn_F(int i, NOBJ_QCODE *qc)
{
  uint8_t first_byte;
  int8_t exponent;
  uint8_t sign;
  char line[100];
  char digits[30];
  
  first_byte = *(++qc);
  qc_len = first_byte & 0x7F;
  sign = first_byte & 0x80;
  
  sprintf(prt_res, "\n%04X: Len:%d\n%04X: '", i+1, *(qc++), i+2);

  line[0] = '\0';
  
  for(int j=0; j<qc_len-1; j++)
    {
      sprintf(digits, "%02X", *(qc++));
      strcat(line, digits);
    }
  
  exponent = *(qc++);
  
  sprintf(digits, "E%d", exponent);
  strcat(line, digits);

  strcat(prt_res, line);
  return(prt_res);
}

// Floating point compact format
int qc_byte_len_fn_F(int i, NOBJ_QCODE *qc)
{
  return(qc_len+1);
}

char *null_qc_byte_prt_fn(int i, NOBJ_QCODE *qc)
{
  return("");
}

int null_qc_byte_fn(int i, NOBJ_QCODE *qc)
{
  return(0);
}


//------------------------------------------------------------------------------

void pr_uint8(uint8_t n)
{
  int l = (n & 0x00FF);

  printf("%02X", l);
}

void pr_var_space_size(NOBJ_VAR_SPACE_SIZE *x)
{
  printf("\nVar Space Size:%04X", x->size);
}

void pr_qcode_space_size(NOBJ_QCODE_SPACE_SIZE *x)
{
  printf("\nQCode Space Size:%04X", x->size);
}

void pr_global_varname_size(NOBJ_GLOBAL_VARNAME_SIZE *x)
{
  printf("\nGlobal varname Size:%04X", x->size);
}

void pr_external_varname_size(NOBJ_EXTERNAL_VARNAME_SIZE *x)
{
  printf("\nExternal varname Size:%04X", x->size);
}

void pr_num_parameters(NOBJ_NUM_PARAMETERS *x)
{
  printf("\nNumber of parameters:");
  pr_uint8(x->num);
}

void pr_parameter_types(NOBJ_PROC *p)
{
  printf("\nParameter types:");
  
  for(int i=0; i<p->num_parameters.num; i++)
    {
      printf("\n%2d %s (%d)",
	     i,
	     decode_vartype(p->parameter_types[i]),
	     p->parameter_types[i]
	     );
    }
}


void decode_qc(int *i,  NOBJ_QCODE **qc)
{
  printf("%s", decode_qc_txt(i, qc));
}

////////////////////////////////////////////////////////////////////////////////
//
//  Top level execution of QCodes
//
//  Exits with exit QCode or invalid QCode
//
////////////////////////////////////////////////////////////////////////////////

void exec_loop(void)
{

}

////////////////////////////////////////////////////////////////////////////////
//
// Initialise the QCode handling
//
////////////////////////////////////////////////////////////////////////////////

void qcode_init(void)
{
  lprintfp = fopen(LPRINT_FN, "w+");

  if( lprintfp == NULL )
    {
      // Warn that we can't open the LPRINT file
      printf("\nWarning: Can't open %s file", LPRINT_FN);
    }
  
}
