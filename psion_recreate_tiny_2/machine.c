////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "nopl.h"

////////////////////////////////////////////////////////////////////////////////
//
// Current machine
//
// The code has been built with the idea of running several OPL machines at
// or saving the state of a machine and continuing later. Having to send the
// machine pointer to all functions is untidy in some cases, so a current
// machine variable is used. This is useful for the runtime_error() function,
// for instance as it avoids having to pass the current machine pointer
// to every function in the call stack even if it isn't used.

NOBJ_MACHINE *current_machine;

char current_proc[NOBJ_FILENAME_MAXLEN+3];

////////////////////////////////////////////////////////////////////////////////


void push_machine_8(NOBJ_MACHINE *m, uint8_t v)
{
#if DEBUG_PUSH_POP
  debug("\n%s:pushing %02X to %04X", __FUNCTION__, v, (m->rta_sp)-1);
#endif

  if( m->rta_sp > 0 )
    {
      m->stack[--(m->rta_sp)] = v;
    }
  else
    {
      error("Attempt to push off end of stack");
    }
}

void push_machine_16(NOBJ_MACHINE *m, int16_t v)
{
  if( m->rta_sp > 0 )
    {
      m->stack[--(m->rta_sp)] = (v &  0xFF);
      m->stack[--(m->rta_sp)] = (v >> 8);
    }
  else
    {
      error("Attempt to push off end of stack");
    }

#if DEBUG_PUSH_POP
  debug("\n%s:pushing %04X to %04X", __FUNCTION__, v, m->rta_sp+2);
#endif

}

////////////////////////////////////////////////////////////////////////////////
//
// Push a string onto the stack. Must be pushed in such a way that the
// length is popped off then the string in character order.
// Must be pushed in reverse, then.

void push_machine_string(NOBJ_MACHINE *m, int len, char *str)
{
#if DEBUG_PUSH_POP
  debug("\n%s:pushing %s to %04X len:%d", __FUNCTION__, str, m->rta_sp, len);
#endif

  // Push the string

  for(int i=0; i< len; i++)
    {
      push_machine_8(m, str[len - i - 1 ]);
    }
  
  // Length last so popped first
  push_machine_8(m, len);
}

NOBJ_INT pop_machine_int(NOBJ_MACHINE *m)
{
  return(pop_machine_16(m));
}

//------------------------------------------------------------------------------

void push_machine_num(NOBJ_MACHINE *m, NOPL_FLOAT *n)
{
#if DEBUG_PUSH_POP
  debug("\n%s:pushing %s to %04X len:%d", __FUNCTION__, str, m->rta_sp, len);
#endif



  // Now push the float on to the stack
  for(int i=0; i<NUM_MAX_DIGITS; i+=2)
    {
      
      push_machine_8(m, (n->digits[i] << 4) + (n->digits[i+1]) );
    }

  push_machine_8(m, n->exponent);
  push_machine_8(m, n->sign);


}

//------------------------------------------------------------------------------

NOPL_FLOAT pop_machine_num(NOBJ_MACHINE *m)
{
  NOPL_FLOAT n;
  int b;

  n.sign = pop_machine_8(m);
  n.exponent = pop_machine_8(m);
  
  // Pop digits
  for(int i = (NUM_MAX_DIGITS/2)-1; i>=0; i--)
    {
      b = pop_machine_8(m);
      n.digits[i*2] = b >> 4;
      n.digits[i*2+1] = b & 0xF;
    }
  
  return(n);
}

//------------------------------------------------------------------------------

void pop_machine_string(NOBJ_MACHINE *m, uint8_t *len, char *str)
{
  uint16_t   orig_sp = m->rta_sp;
  int i;

  // Pop length
  *len = pop_machine_8(m);

  // Pop characters of string
  for(i=0; i<*len; i++)
    {
      str[i] = pop_machine_8(m);
    }
  str[i] = '\0';
  
#if DEBUG_PUSH_POP
  debug("\n%s:Popped '%s' from %04X", __FUNCTION__, str, orig_sp);
#endif
  
}



////////////////////////////////////////////////////////////////////////////////

uint16_t stack_entry_16(NOBJ_MACHINE *m, uint16_t ptr)
{
  uint16_t ret = 0;
  
  ret =  (uint16_t)(m->stack[ptr+0]) << 8;
  ret |=            m->stack[ptr+1];

  return(ret);
  
}

uint8_t stack_entry_8(NOBJ_MACHINE *m, uint16_t ptr)
{
  uint8_t ret = 0;
  
  ret =  (uint16_t)(m->stack[ptr+0]);

  return(ret);
  
}

//------------------------------------------------------------------------------
//
// Put 16 bit value in stack
void put_stack_16(NOBJ_MACHINE *m, int sp, uint16_t v)
{
  m->stack[sp-0] = (v &  0xFF);
  m->stack[sp-1] = (v >> 8);
}

//------------------------------------------------------------------------------
//
// Gets a 16 bit value from the machine stack


uint16_t get_machine_16(NOBJ_MACHINE *m, uint16_t sp)
{
  uint16_t value = 0;
  
  if( sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to get from empty stack");
    }

  //printf("\n   from %04X", sp);
  
  value  =  m->stack[sp+1];
  value |= (m->stack[sp+0]) << 8;
  
  return(value);  
}

uint16_t get_machine_8(NOBJ_MACHINE *m, uint16_t sp, uint8_t *v)
{
  uint8_t value = 0;
  
  if( sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to get from empty stack");
    }

  value  =  m->stack[sp++];
  *v = value;
  
  return(sp);  
}


//------------------------------------------------------------------------------
// Pop byte off stack and return new stack pointer value.

uint16_t pop_sp_8(NOBJ_MACHINE *m, uint16_t sp, uint8_t *val)
{
  if( sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to pop from empty stack");
    }
  
  *val = m->stack[sp++];

#if DEBUG_PUSH_POP
  debug("\n%s:Popped %02X from %04X", __FUNCTION__, *val, (m->rta_sp)-1);
#endif

  return(sp);  
}

//------------------------------------------------------------------------------
// Pop byte off stack

uint8_t pop_machine_8(NOBJ_MACHINE *m)
{
  uint8_t val8;
  
  if( m->rta_sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to pop from empty stack");
    }
  
  val8 = m->stack[(m->rta_sp)++];

#if DEBUG_PUSH_POP
  debug("\n%s:Popped %02X from SP:%04X", __FUNCTION__, val8, (m->rta_sp)-1);
#endif

  return(val8);  
}

uint16_t pop_machine_16(NOBJ_MACHINE *m)
{
  uint16_t val16;
  
  if( m->rta_sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to pop from empty stack");
    }
  
  val16  = (m->stack[(m->rta_sp)++]);
  val16 <<= 8;
  val16 |= (m->stack[(m->rta_sp)++]);

#if DEBUG_PUSH_POP
  debug("\n%s:Popped %04X from SP:%04X", __FUNCTION__, val16, (m->rta_sp)-2);
#endif

  return(val16);  
}
////////////////////////////////////////////////////////////////////////////////

uint16_t pop_discard_sp_int(NOBJ_MACHINE *m, uint16_t sp)
{
  if( sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to pop from empty stack");
    }

  for(int i=0; i<2; i++)
    {
      ++sp;
      
#if DEBUG_PUSH_POP
      debug("\n%s:Popped from SP:%04X", __FUNCTION__, (m->rta_sp)-1);
#endif
    }
  

  return(sp);  
}

uint16_t pop_discard_sp_float(NOBJ_MACHINE *m, uint16_t sp)
{
  if( sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to pop from empty stack");
    }

  for(int i=0; i<8; i++)
    {
      ++sp;
#if DEBUG_PUSH_POP
      debug("\n%s:Popped and discarded from SP:%04X", __FUNCTION__, (m->rta_sp)-1);
#endif
      
    }

  return(sp);  
}

uint16_t pop_discard_sp_str(NOBJ_MACHINE *m, uint16_t sp)
{
  if( sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to pop from empty stack");
    }

  uint8_t len_str;
  
  sp = pop_sp_8(m, sp, &len_str);
  
  for(int i=0; i<len_str; i++)
    {
      ++sp;
#if DEBUG_PUSH_POP
      debug("\n%s:Popped and discarded from SP:%04X", __FUNCTION__, (m->rta_sp)-1);
#endif
      
    }

  return(sp);  
}

////////////////////////////////////////////////////////////////////////////////
//
// Auxiliary stack manipulation
//
// This allows a mark to be set up in the stack and the stack items to be
// 're-popped' off the stack, after they have actually been popped off.
// Obviously nothing can push over the data before it is re-popped.
//
////////////////////////////////////////////////////////////////////////////////

uint16_t aux_sp;

void mark_aux_stack(NOBJ_MACHINE *m)
{
  // Store the current stack pointer
  aux_sp = m->rta_sp;
}

//------------------------------------------------------------------------------

uint8_t pop_aux_8(NOBJ_MACHINE *m)
{
  uint8_t val8;
  
  if( aux_sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to pop from empty stack");
    }
  
  val8 = m->stack[aux_sp++];

#if DEBUG_PUSH_POP
  debug("\n%s:Popped %02X from SP:%04X", __FUNCTION__, val8, (aux_sp)-1);
#endif

  return(val8);  
}

//------------------------------------------------------------------------------

NOPL_FLOAT pop_aux_num(NOBJ_MACHINE *m)
{
  NOPL_FLOAT n;
  int b;

  n.sign = pop_aux_8(m);
  n.exponent = pop_aux_8(m);
  
  // Pop digits
  for(int i = (NUM_MAX_DIGITS/2)-1; i>=0; i--)
    {
      b = pop_aux_8(m);
      n.digits[i*2] = b >> 4;
      n.digits[i*2+1] = b & 0xF;
    }

  return(n);
}

////////////////////////////////////////////////////////////////////////////////
//
// Run a function for all frames on the stack
// If function return true then exit
// Return zero if function never fires, otherwise 1
//
// f_fp holds FP of matching frame on exit, or 0

int machine_for_each_frame(NOBJ_MACHINE *m, MACHINE_CHECK_FRAME_FN fn, int *f_fp)
{
  int fp;
  
  for(fp = m->rta_fp; fp != 0; fp = get_machine_16(m, fp))
    {
      //printf("\nSRCH:%04X", fp);
      
      if( (*fn)(m, fp) )
	{
	  *f_fp = fp;
	  //printf("\nret1");
	  return(1);
	}
    }
  
  *f_fp = 0;
  //printf("\nret0");
  return(0);
}

//------------------------------------------------------------------------------
//
// Find any defined onerr handler
//
// The stack frames are searched for a non-zero onerr handler
//
//

int machine_nonzero_onerr(NOBJ_MACHINE *m, int fp)
{
  if( get_machine_16(m, fp+FP_OFF_ONERR) != 0 )
    {
      //printf("\n%s:ret1  ONERR:%04X", __FUNCTION__, get_machine_16(m, fp+FP_OFF_ONERR));
      return(1);
    }
  
  //printf("\n%s:ret0", __FUNCTION__);
  return(0);
}

int machine_onerr_handler(NOBJ_MACHINE *m)
{
  int f_fp;     // FP if frame found
  
  // Look through frames
  if( machine_for_each_frame(m, machine_nonzero_onerr, &f_fp) )
    {
      //printf("\n%s: %04X", __FUNCTION__, get_machine_16(m, f_fp+FP_OFF_ONERR));
      return(get_machine_16(m, f_fp+FP_OFF_ONERR));
    }

  // None defined
  return(0);
}
