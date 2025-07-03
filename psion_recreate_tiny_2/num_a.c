////////////////////////////////////////////////////////////////////////////////
//
// Num package
//
// BCD floating point
//
////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
//#include <double.h>
#include <float.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

#include "nopl.h"


#define DB1 1      // num_to_text

////////////////////////////////////////////////////////////////////////////////

FILE *numfp;
char num_text[NOBJ_STRING_MAXLEN+1];

////////////////////////////////////////////////////////////////////////////////
//
// Return text version of a float
//

char *num_as_text(NOPL_FLOAT *n, char *text)
{
  static char line[NUM_AS_TEXT_LEN];
  static char part[20];
  
  if(n->sign)
    {
      sprintf(line, "-", (n->sign & 0xFF));
    }
  else
    {
      sprintf(line, "+", n->sign);
    }
  
  sprintf(part, "%d.%s", n->digits[0], text);
  strcat(line, part);
  
  for(int i=1; i<NUM_MAX_DIGITS; i++)
    {
      sprintf(part, "%d%s", n->digits[i], text);
      strcat(line, part);
    }

  sprintf(part, " E%d", (int)n->exponent);
  strcat(line, part);

  return(line);
}
//------------------------------------------------------------------------------
//

void num_clear(NOPL_FLOAT *n)
{
  n->sign = NUM_SIGN_POSITIVE;
  n->exponent = 0;
  num_clear_digits(NUM_MAX_DIGITS, &(n->digits[0]));
}

//------------------------------------------------------------------------------
//
// Turn a text representation of a float into a float
//
// Form of float:
//
// <sign>*<digit|dot>+
// <sign>*<digit|dot>+E<sign>*<digit|dot>+
//
// No spaces allowed in number between digits and E
//

int check_digit_or_dot(char *p)
{
  switch(*p)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      return(1);
      break;
    }

  return(0);
}

int check_sign(char *p)
{
  dbq("Char:'%c'", *p);
  
  switch(*p)
    {
    case '-':
    case'+':
      dbq("ret(1)");
      return(1);
      break;
    }
  
  dbq("ret(0)");
  return(0);
}

int check_exp(char *p)
{

  switch(*p)
    {
    case 'e':
    case 'E':
      dbq("ret(1)");
      return(1);
      break;
    }

  dbq("ret(0)");
  return(0);
}


int scan_digit_or_dot(char **p, char *digdot)
{
  switch(**p)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      *digdot = **p;
      (*p)++;
      return(1);
      break;
    }

  return(0);
}

int scan_sign(char **p, int *sign_multiplier)
{
  switch(**p)
    {
    case '-':
      *sign_multiplier = -1;
      (*p)++;
      dbq("ret(1) -ve");
      return(1);
      break;
      
    case '+':
      (*p)++;
      *sign_multiplier = 1;
      dbq("ret(1) +ve");
      return(1);
    }
  
  *sign_multiplier = 1;
  dbq("ret(0)");
  return(0);
}

int scan_exp(char **p)
{
  switch(**p)
    {
    case 'e':
    case 'E':
      (*p)++;
      return(1);
      break;
    }
  
  return(0);
}

//------------------------------------------------------------------------------

NOPL_FLOAT num_from_text(char *p)
{
  NOPL_FLOAT f;
  int dig_i = 0;
  int dotpos = 0;
  int dot_seen = 0;  // Only one dot allowed
  int num_digits = 0;
  int sign_mul = 1;
  
  num_clear(&f);
  f.sign = NUM_SIGN_POSITIVE;
  
  if( check_sign(p) )
    {
      scan_sign(&p, &sign_mul);

      switch(sign_mul)
	{
	case 1:
	  f.sign = NUM_SIGN_POSITIVE;
	  break;

	case -1:
	  f.sign = NUM_SIGN_NEGATIVE;
	  break;
	}
    }

  while(check_digit_or_dot(p) )
    {
      char digdot;

      num_digits++;

      // Max digits plus one dot
      if( num_digits > NUM_MAX_DIGITS+1 )
	{
	  runtime_error(ER_MT_IS, "Too many digits in mantissa (%d)", num_digits);
	  num_clear(&f);
	  return(f);
	}
      
      scan_digit_or_dot(&p, &digdot);
      if( digdot == '.' )
	{
	  // Only one dot llowed
	  if( dot_seen )
	    {
	      runtime_error(ER_MT_IS, "Only one dot allowed");
	      num_clear(&f);
	      return(f);
	    }

	  // The number of digits before the dot is counted
	  dotpos = dig_i;
	  dot_seen = 1;
	}
      else
	{
	  f.digits[dig_i++] = digdot-'0';
	}
    }

  // Check for exponent
  num_digits = 0;
  
  if( check_exp(p) )
    {
      int exp_sign = 1;

      scan_exp(&p);

      if( check_sign(p) )
	{
	  scan_sign(&p, &exp_sign);
	}

      int exp_val = 0;
      
      while(check_digit_or_dot(p) )
	{
	  char digdot;

	  num_digits++;

	  if( num_digits > 2 )
	    {
	      runtime_error(ER_MT_IS, "Too many digits in exponent");
	      num_clear(&f);
	      return(f);
	    }

	  scan_digit_or_dot(&p, &digdot);
	  if( digdot == '.' )
	    {
	      // No dots allowed
	      runtime_error(ER_MT_IS, "Only one dot allowed");
	      num_clear(&f);
	      return(f);
	    }
	  else
	    {
	      f.exponent *= 10;
	      f.exponent += digdot-'0';
	    }
	}
      
      f.exponent *= exp_sign;
    }

  f.exponent += dotpos-1;

  num_normalise(&f);
  
  //  printf("dotpos=%d", dotpos);
  return(f);
}


//------------------------------------------------------------------------------
//
// Build a NOPL_FLOAT from bytes formatted as a Psion float
//

NOPL_FLOAT num_from_mem(uint8_t *mp)
{
  NOPL_FLOAT n;
  int b;

  n.sign = *(mp++);
  n.exponent = *(mp++);
  
  // Pop digits
  for(int i = (NUM_MAX_DIGITS/2)-1; i>=0; i--)
    {
      b = *(mp++);
      n.digits[i*2] = b >> 4;
      n.digits[i*2+1] = b & 0xF;
    }

  return(n);
}

//------------------------------------------------------------------------------
//
// Build a psion float from a NOPL_FLOAT

void num_to_mem(NOPL_FLOAT *f, uint8_t *mp)
{
  int b;
  uint8_t *omp = mp;
  
  *(mp++) = f->sign;
  *(mp++) = f->exponent;
  
  for(int i = (NUM_MAX_DIGITS/2)-1; i>=0; i--)
    {
      b =  (f->digits[i*2+0] << 4);
      b |= (f->digits[i*2+1] &  0xF);
      *(mp++) = b;
    }

#if 0
  printf("\nMemdump: ");
  for(int i=0; i<8; i++)
    {
      printf("%02X ", *(omp+i));
    }
  printf("\n");

  printf("\nF dump: ");
  for(int i=0; i<sizeof(NOPL_FLOAT); i++)
    {
      printf("%02X ", *(((uint8_t *)f)+i));
    }
  printf("\n");

  printf("\nFloat to text:'%s'\n", num_to_text(f));
#endif
}

//------------------------------------------------------------------------------
       
void dbq_num_exploded_f(const char *caller, char *text, NOPL_FLOAT *n)
{
  dbpfq(caller, text, num_as_text(n, " "));
}

void dbq_num_f(const char *caller, char *text, NOPL_FLOAT *n)
{
  dbpfq(caller, text, num_as_text(n, ""));
}

//------------------------------------------------------------------------------

void num_print(NOPL_FLOAT *n)
{
  printf("%s", num_as_text(n, ""));
}

////////////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------------

#define MAX_INDEX (NUM_MAX_DIGITS-1)

void digits_right(NOPL_FLOAT *n, int shift)
{
  
  for(int i = MAX_INDEX; i>0; i--)
    {
      n->digits[i] = n->digits[i-1];
    }

  for(int i=0; i<shift; i++)
    {
      n->digits[0] = 0;
    }
}

//------------------------------------------------------------------------------

// Make the exponent of n the same as that of r

void make_exp_same(NOPL_FLOAT *n, NOPL_FLOAT *r)
{
  n->exponent = r->exponent;
  digits_right(n, r->exponent - n->exponent);
}

//------------------------------------------------------------------------------


void num_shift_digits_right_n(int n, NOPL_FLOAT_DIG *digits, int8_t *exponent)
{
  // Shift mantissa digits to the 'right'
  for(int i=n-1; i>0; i--)
    {
      digits[i] = digits[i-1];
    }

  digits[0] = 0;
  (*exponent)++;
}

void num_shift_digits_right(NOPL_FLOAT *n)
{
  num_shift_digits_right_n(NUM_MAX_DIGITS, n->digits, &(n->exponent));

}

//------------------------------------------------------------------------------

void num_shift_digits_left_n(int n, NOPL_FLOAT_DIG *digits, int8_t *exponent)
{
  // Shift mantissa digits to the 'left'
  for(int i=0; i<n;  i++)
    {
      digits[i] = digits[i+1];
    }
  
  digits[n-1] = 0;
  (*exponent)--;
}

void num_shift_digits_left(NOPL_FLOAT *n)
{
  num_shift_digits_left_n(NUM_MAX_DIGITS, n->digits, &(n->exponent));
}

//------------------------------------------------------------------------------

void num_sub_digits(int n, NOPL_FLOAT_DIG *a, NOPL_FLOAT_DIG *b, NOPL_FLOAT_DIG *r)
{
  num_mantissa_tens_compl_digits(n, b);
  
  // Add the mantissa digits
  for(int i=n; i>=0; i--)
    {
      r[i] = a[i] + b[i];
    }

  //num_db_digits("\nnum_sub_digits  r:", n, r);

  num_propagate_carry_digits(r, n);
  //num_db_digits("\nnum_sub_digits  r:", n, r);
}

//------------------------------------------------------------------------------
//
// Propagate carries and shift exponents to get a non zero in digit[0]
//
// A digit greater than 9 can be in [0], this indicates a carry/overflow
//

void num_propagate_carry_digits(NOPL_FLOAT_DIG *digits, int num_digits)
{
  // Propagate carry
  for(int i= num_digits-1; i>0; i--)
    {
      if( digits[i] >= 10 )
	{
	  digits[i-1]+= digits[i] /10;
	  digits[i] %= 10;
	}
    }
}

void num_propagate_carry(NOPL_FLOAT *n, int num_digits)
{
  num_propagate_carry_digits(n->digits, NUM_MAX_DIGITS);
}

//------------------------------------------------------------------------------

void num_clear_digits(int n, NOPL_FLOAT_DIG *d)
{
  for(int i=0; i<n; i++)
    {
      d[i] = 0;
    }
}

//------------------------------------------------------------------------------

#define EXPANDED 1

void num_db_digits(char *text, int n, NOPL_FLOAT_DIG *d)
{
  fprintf(exdbfp, "%s", text);
  
  for(int k=0; k<n; k++)
    {
#if EXPANDED
      fprintf(exdbfp, " %d", d[k]);
#else
      fprintf(exdbfp, "%d", d[k]);
#endif
      if( k == (n/2)-1 )
	{
#if EXPANDED	  
	  fprintf(exdbfp, "_");
#endif
	}
    }
  
  fprintf(exdbfp, "\n");
}

//------------------------------------------------------------------------------
//
//
// Add N digit numbers
// Used for larger registers when calculating * and /
//

void num_add_n_digits(int n, NOPL_FLOAT_DIG *a, NOPL_FLOAT_DIG *b, NOPL_FLOAT_DIG *r)
{
  for(int i=0; i<n; i++)
    {
      r[i] = a[i] + b[i];
    }

  // Sort out carry
  num_propagate_carry_digits(r, n);
}

//------------------------------------------------------------------------------

// Add a digit to the end of a number
// digits are right justified, n digits in total

void num_cat_digit(int n, NOPL_FLOAT_DIG *d, int digit)
{
  // Shift digits left and put new one on end.
  for(int i=0; i<n-1; i++)
    {
      d[i] = d[i+1];
    }

  d[0] = digit;

#if 0
  // Find the first zero digit from left and put digit there.
  for(int i=0; i<n-1; i--)
    {
      if( d[i] == 0 )
	{
	  d[i] = digit;
	  return(1);
	}
    }

  
  return(0);
#endif
}

//------------------------------------------------------------------------------

int num_digits_zero(int n, NOPL_FLOAT_DIG *digits)
{
  for(int i=0; i<n; i++)
    {
      if( (*(digits++)) != 0 )
	{
	  return(0);
	}
    }

  return(1);
}

//------------------------------------------------------------------------------

void num_normalise_digits(int n, NOPL_FLOAT_DIG *digits, int8_t *exponent)
{
  // Check for zero and exit
  if( num_digits_zero(n, digits) )
    {
      *exponent = 0;
      return;
    }

  // If digits[0] is 0 then we need to shift left until it is non zero
  while( digits[0] == 0 )
    {
      num_shift_digits_left_n(n, digits, exponent);
    }

  //    dbq_num_exploded("%s After shift", n);
  
  // If we have a carry in the LH digit then shift right until it has gone
  while( digits[0] >= 10 )
    {
      num_shift_digits_right_n(n, digits, exponent);

      // Split the first digit into two
      digits[0] = digits[1] / 10;
      digits[1] %= 10;
    }
  
  //dbq_num_exploded("%s After first dig split:", n);
}

void num_normalise(NOPL_FLOAT *n)
{
  dbq_num_exploded("%s Entry:", n);
  num_normalise_digits(NUM_MAX_DIGITS, &(n->digits[0]), &n->exponent);
  
}

//------------------------------------------------------------------------------
//
// Builds a table of
//  0 x num
//  1 x num
//   .  .
//   .  .
//   .  .
//  8 x num
//  9 x num
//

void num_build_times_table(int n, NOPL_FLOAT_DIG *ttable, NOPL_FLOAT_DIG *num)
{
  char txt[20];
  int8_t exponent = 0;

  dbq("Building times table for %d digits for:", n);
  num_db_digits("the number:", n, num);
	
  // Clear the first entry
  num_clear_digits(n, ttable);

  // Add entries to each other to build up the table
  
  for(int i=1; i<10; i++)
    {
      sprintf(txt, "%3d:", i);
      
      num_add_n_digits(n, &(ttable[((i-1)*n)]), num, &(ttable[i*n]));
      //      num_db_digits(txt, n, &(ttable[i*n]));
      num_propagate_carry_digits(&(ttable[i*n]), n);

      num_db_digits(txt, n, &(ttable[i*n]));
    }

  //num_normalise_digits(n, &(ttable[i*n]), &exponent);
  dbq("Exponent:%d", exponent);
  
}

int num_digits_gte(int n, NOPL_FLOAT_DIG *a, NOPL_FLOAT_DIG *b)
{
  int eq = 1;
  
  for(int i=0; i<n; i++)
    {
      if( a[i] != b[i] )
	{
	  eq = 0;
	}
    }

  if( eq )
    {
      return(1);
    }
  
  for(int i=0; i<n; i++)
    {
      if( a[i] != b[i] )
	{
	  if( a[i] > b[i] )
	    {
	      return(1);
	    }
	  else
	    {
	      return(0);
	    }
	}
    }

  return(0);
}

//------------------------------------------------------------------------------
//
// Does b divide into a?
// n is number of digits and times_table is
// 1..9 times b
// digits is an int, right justified.
//

int num_divides_into(int n, NOPL_FLOAT_DIG *a, NOPL_FLOAT_DIG *b, NOPL_FLOAT_DIG *times_table)
{
  //num_db_digits("a:", n, a);

  
  // If the number is greater than one of the times table entries then
  // the b does divide into a.
  for(int i=9; i>0; i--)
    {

      if( num_digits_gte(n, a, &(times_table[i*n])) )
	{
	  dbq("Does divide (%d)", i);
	  num_db_digits("tt:", n, &(times_table[i*n]));
	  return(i);
	}
    }
  
  dbq("Doesn't divide");
  return(0);
}

//------------------------------------------------------------------------------

void num_make_exponent(NOPL_FLOAT *n, int exp)
{
  // The num will be left unnormalised.
  // We shift mantissa to make the expoinent the required value. This may result
  // in all zeros in some cases.

  // There are two directions we may have to shift in.
  
  if( n->exponent < exp )
    {
      dbq("Right Exp:%d", exp);
      while(n->exponent < exp )
	{
	  num_shift_digits_right(n);
	}
    }
  else
    {
      dbq("Left Exp:%d", exp);
      
      while(n->exponent < exp )
	{
	  num_shift_digits_left(n);
	}
    }
}

//------------------------------------------------------------------------------
//
// Make mantissa a ten's complement

void num_mantissa_tens_compl_digits(int n, NOPL_FLOAT_DIG *digits)
{
  dbq("");

  // Nine's complement
  for(int i=0; i<n; i++)
    {
      digits[i] = 9 - digits[i];
    }

  // Add 1 for the ten's complement
  digits[n-1]++;

  num_propagate_carry_digits(digits, n);
}

void num_mantissa_tens_compl(NOPL_FLOAT *n)
{
  num_mantissa_tens_compl_digits(NUM_MAX_DIGITS, &(n->digits[0]));
  dbq_num_exploded("%s before normalise", n);
  //num_normalise(n);

  dbq_num_exploded("%s after 10's compl", n);
}

//------------------------------------------------------------------------------

void num_num_to_int(int n, NOPL_FLOAT *num,  NOBJ_INT *i)
{
  // Check float isn't too big
  
  if( num->exponent > 4 )
    {
      runtime_error(ER_RT_IO, "Float too big (%s)", num_to_text(num));
      *i = 0;
      return;
    }

  if( num->exponent < 0 )
    {
      *i = 0;
      return;
    }

  //------------------------------------------------------------------------------
  // Check that the num isn't out of range
  // Maximum is slightly different between positive and negative
  // integers
  
  if( num->exponent == 4 )
    {
      NOPL_FLOAT_DIG thr[NUM_MAX_DIGITS];

      num_clear_digits(n, thr);

      // Set up the first positive invalid number
      thr[0] = 3;
      thr[1] = 2;
      thr[2] = 7;
      thr[3] = 6;
      thr[4] = 8;

      num_db_digits("\nthr:", n, thr);
  
      if( NUM_IS_POSITIVE(num) )
	{
	  // Nothing to do
	  if( num_digits_gte(NUM_MAX_DIGITS, &(num->digits[0]), thr) )
	    {
	      // Too big
	      runtime_error(ER_RT_IO, "Float too big (%s)", num_to_text(num));
	      *i = 0;
	      return;
	    }
	}
      else
	{
	  thr[4] = 8;

	  if( num_digits_gte(NUM_MAX_DIGITS, &(num->digits[0]), thr) )
	    {
	      // Too big
	      runtime_error(ER_RT_IO, "Float too big (%s)", num_to_text(num));
	      *i = 0;
	      return;
	    }
	}
    }

  // Calculate the integer
  switch(num->exponent)
    {
    case 0:
      *i = num->digits[0];
      break;
    case 1:
      *i = num->digits[0]*10 + num->digits[1];
      break;
    case 2:
      *i = num->digits[0]*100 + num->digits[1]*10 + num->digits[2];
      break;
    case 3:
      *i = num->digits[0]*1000 + num->digits[1]*100 + num->digits[2]*10 + num->digits[3];
      break;
    case 4:
      *i = num->digits[0]*10000 + num->digits[1]*1000 + num->digits[2]*100 + num->digits[3]*10 + num->digits[4];
      break;

    }
  
  // Work out sign
  if( NUM_IS_POSITIVE(num) )
    {
      // Nothing to do
    }
  else
    {
      // Negative, invert
      *i = -*i;
    }

  dbq("i:%d (%04X)", *i, *i);
  
}

//------------------------------------------------------------------------------

// Convert int32 to float

void num_int32_to_num(int n, int32_t *i, NOPL_FLOAT *num)
{
  dbq("Converting %d to float", *i);
  
  num_clear_digits(n, num->digits);
  
  // Set sign up
  if( *i < 0 )
    {
      num->sign = NUM_SIGN_NEGATIVE;
      *i = - *i;
    }
  else
    {
      num->sign = NUM_SIGN_POSITIVE;
    }

  num->exponent   = 11;

#if 1  
  long int div = 1;
  for(int j=0, k=NUM_MAX_DIGITS-1; j<NUM_MAX_DIGITS; j++,k--)
    {
      num->digits[k]  = ((long int)*i / div) % 10;
      div *= 10L;
    }
#else
  
  num->digits[0]  = (*i / 10000000000);
  num->digits[1]  = (*i / 1000000000) % 10;
  num->digits[2]  = (*i / 100000000)  % 10;
  num->digits[3]  = (*i / 10000000)   % 10;
  num->digits[4]  = ( *i / 1000000) % 10;
  num->digits[5]  = (*i / 100000);
  num->digits[6]  = (*i / 10000) % 10;
  num->digits[7]  = (*i / 1000)  % 10;
  num->digits[8]  = (*i / 100)   % 10;
  num->digits[9]  =  *i / 10        % 10;
  num->digits[10] = (*i / 1    ) % 10;
#endif
  
  dbq_num("Raw convert:%s", num);
  num_normalise_digits(n, num->digits, &(num->exponent));
  dbq_num("After normalise:%s", num);
  num_normalise(num);
}

void num_int_to_num(int n, NOBJ_INT *i, NOPL_FLOAT *num)
{
  num_clear_digits(n, num->digits);
  
  // Set sign up
  if( *i < 0 )
    {
      num->sign = NUM_SIGN_NEGATIVE;
      *i = - *i;
    }
  else
    {
      num->sign = NUM_SIGN_POSITIVE;
    }

  num->exponent = 4;
  num->digits[0] = (*i / 10000);
  num->digits[1] = (*i / 1000) % 10;
  num->digits[2] = (*i / 100)  % 10;
  num->digits[3] = (*i / 10)   % 10;
  num->digits[4] =  *i         % 10;
  
  num_normalise_digits(n, num->digits, &(num->exponent));
  
  num_normalise(num);
}


//------------------------------------------------------------------------------
//
// Add positive floats
//
// Ignores signs
//

void num_add_pos(NOPL_FLOAT *a, NOPL_FLOAT *b, NOPL_FLOAT *r)
{
  dbq("ADD POS");
  dbq_num("%s a:", a);
  dbq_num("%s b:", b);
  
  // Adjust the smaller number so we have the same exponent. Shift the mantissa to line up.
  if( a->exponent > b->exponent)
    {
      num_make_exponent(b, a->exponent);
    }
  else
    {
      num_make_exponent(a, b->exponent);
    }

  dbq_num("%s After exp adj a:", a);
  dbq_num("%s After exp adj b:", b);
  
  // Add the mantissa digits
  for(int i=NUM_MAX_DIGITS-1; i>=0; i--)
    {
      r->digits[i] = a->digits[i] + b->digits[i]; 
    }

  dbq_num("%s After digit sub r:", r);
  
  r->exponent = a->exponent;

  dbq_num("%s After digit asgn r:", r);
  
  dbq_num("%s Before normailse:", r);

  // Handle carries
  num_propagate_carry_digits(r->digits, NUM_MAX_DIGITS);

  // Normalise
  num_normalise(r);
  dbq_num("%s After normalise:", r);
}

//------------------------------------------------------------------------------
//
// Subtract positive floats
//
// Ignores signs
//

void num_sub_pos(NOPL_FLOAT *a1, NOPL_FLOAT *b1, NOPL_FLOAT *r)
{
  // Copy arguments as we don't want to alter them
  NOPL_FLOAT af = *a1;
  NOPL_FLOAT bf = *b1;
  NOPL_FLOAT *a = &af;
  NOPL_FLOAT *b = &bf;

  dbq("SUB POS");
  dbq_num("%s a:", a);
  dbq_num("%s b:", b);

  // Initialise sign
  r->sign = a->sign;
  
  // Adjust the smaller number so we have the same exponent. Shift the mantissa to line up.
  if( a->exponent > b->exponent)
    {
      num_make_exponent(b, a->exponent);
    }
  else
    {
      num_make_exponent(a, b->exponent);
    }

  num_mantissa_tens_compl(b);
  
  // Add the mantissa digits
  for(int i=NUM_MAX_DIGITS-1; i>=0; i--)
    {
      r->digits[i] = a->digits[i] + b->digits[i];
    }
 
  r->exponent = a->exponent;

  num_propagate_carry(r, NUM_MAX_DIGITS);

  dbq_num_exploded("%s Before normalise r:", r);

  // Normalise

  dbq_num_exploded("%s After carry prop r:", r);
    
  // We will always have an overflow in the first digit as we are adding a ten's
  // complement number
  if( r->digits[0] >= 10 )
    {
      // No overflow, sign stays same
      r->digits[0] -= 10;
      r->sign = a->sign;

    }
  else
    {
      // Overflow, the sign changes
      r->sign = NUM_INVERT_SIGN(a->sign);
      num_mantissa_tens_compl(r);
    }

  dbq_num_exploded("%s After adjust r:", r);
  
  // Normalise
  num_normalise(r);

  dbq_num_exploded("%s After normalise r:", r);
  dbq_num("%s Exit r:", r);

}

//------------------------------------------------------------------------------
//
// Add two floats, the sign is handled here.
//

void num_add(NOPL_FLOAT *a1, NOPL_FLOAT *b1, NOPL_FLOAT *r)
{
  // If the signs are the same then we add the mantissas:
  //
  // a+b
  // -a + -b
  //
  // If we have different signs:
  //
  // -a+b
  // a + -b
  //
  // These are subtractions
  //
  dbq("");

  // We cannot alter our parameters
  NOPL_FLOAT af = *a1;
  NOPL_FLOAT bf = *b1;
  NOPL_FLOAT *a = &af;
  NOPL_FLOAT *b = &bf;

  if( (a->sign) == (b->sign) )
    {
      num_add_pos(a, b, r);
      r->sign = a->sign;
    }
  else
    {
      // Different signs, so arrange the sum to be a difference
      
      if( NUM_IS_NEGATIVE(a) )
	{
	  // a negative, b positive
	  // swap to make this an x-y type subtraction
	  num_sub_pos(b, a, r);
	}
      else
	{
	  num_sub_pos(b, a, r);
	}
    }

  // Summarise
  fprintf(numfp, "\n(%s): %s",  __FUNCTION__, num_as_text(a1, ""));
  fprintf(numfp,      " + %s",                num_as_text(b1, ""));
  fprintf(numfp,      " = %s",                num_as_text(r, ""));
}

//------------------------------------------------------------------------------
//
//
//   x -  y   => x - y
//   x - -y   => x + y
//  -x -  y   => -(x+y)
//  -x - -y   => y - x
//

#define SIG_P_P 0
#define SIG_P_N 1
#define SIG_N_P 2
#define SIG_N_N 3


void num_sub(NOPL_FLOAT *a1, NOPL_FLOAT *b1, NOPL_FLOAT *r)
{
  int signsig = 0;

  // We cannot alter our parameters
  NOPL_FLOAT af = *a1;
  NOPL_FLOAT bf = *b1;
  NOPL_FLOAT *a = &af;
  NOPL_FLOAT *b = &bf;

  dbq_num_exploded("%s a:", a);
  dbq_num_exploded("%s b:", b);

  if( NUM_IS_NEGATIVE(a) )
    {
      dbq("a -ve");
      signsig |= 2;
    }
  else
    {
      dbq("a +ve (%d %02X", a->sign, a->sign);
    }

  if( NUM_IS_NEGATIVE(b) )
    {
      dbq("b -ve");
      signsig |= 1;
    }
  else
    {
      dbq("b +ve");
    }

  dbq("Sign sig:%d", signsig);

  switch(signsig)
    {
    case SIG_P_P:
      num_sub_pos(a, b, r);
      break;

    case SIG_P_N:
      num_add_pos(a, b, r);
      r->sign = NUM_SIGN_POSITIVE;
      break;

    case SIG_N_P:
      // a negative, b positive
      // swap to make this an x-y type subtraction
      num_add_pos(a, b, r);
      r->sign = NUM_SIGN_NEGATIVE;
      break;

    case SIG_N_N:
      // b is positive so force it
      b->sign = NUM_SIGN_POSITIVE;
      num_sub_pos(b, a, r);
      break;
    }

  // Summarise
  fprintf(numfp, "\n(%s): %s",  __FUNCTION__, num_as_text(a1, ""));
  fprintf(numfp,      " - %s",                num_as_text(b1, ""));
  fprintf(numfp,      " = %s",                num_as_text(r, ""));

}

////////////////////////////////////////////////////////////////////////////////
//
// Multiply two floats
//


void num_mul(NOPL_FLOAT *a1, NOPL_FLOAT *b1, NOPL_FLOAT *r)
{
  // We cannot alter our parameters
  NOPL_FLOAT af = *a1;
  NOPL_FLOAT bf = *b1;
  NOPL_FLOAT *a = &af;
  NOPL_FLOAT *b = &bf;

  int d_digits[NUM_MAX_DIGITS*2];

  for(int i=0; i<NUM_MAX_DIGITS*2; i++)
    {
      d_digits[i] = 0;
    }
  
  dbq("MUL");
  dbq_num("%s a:", a);
  dbq_num("%s b:", b);

  // Sort out the signs
  //
  if( a->sign == b-> sign )
    {
      r->sign = NUM_SIGN_POSITIVE;
    }
  else
    {
      r->sign = NUM_SIGN_NEGATIVE;
    }

  // Sort out exponent
  r->exponent = a->exponent + b->exponent;

  dbq_num_exploded("%s a:", a);
  dbq_num_exploded("%s b:", b);

  // Now multiply mantissas, we use a double sized mantissa for the result
  
  for(int i=NUM_MAX_DIGITS-1; i>=0; i--)
    {
      for(int j=NUM_MAX_DIGITS-1; j>=0; j--)
	{
	  dbq("%i:d j:%d, p:%d  a:%d b:%d", i, j, a->digits[j]*b->digits[i], a->digits[j], b->digits[i]);
	  //	  d_digits[i+j+NUM_MAX_DIGITS] += a->digits[j]*b->digits[i];
	  d_digits[i+j] += a->digits[j]*b->digits[i];
	  num_propagate_carry_digits(d_digits, NUM_MAX_DIGITS*2);
	}

      fprintf(exdbfp, "\n");

      // Now propagate the carry
      num_db_digits("d_digits before c:", NUM_MAX_DIGITS*2, d_digits);
      num_propagate_carry_digits(d_digits, NUM_MAX_DIGITS*2);
      num_db_digits("d_digits after  c:", NUM_MAX_DIGITS*2, d_digits);
    }

  // Build result
  // Normalise digits, keeping track of exponent adjustment
  int exponent_adjust = 0;
  
  while( d_digits[0] == 0 )
    {
      // Shift mantissa digits to the 'left'
      for(int i=0; i<NUM_MAX_DIGITS*2;  i++)
	{
	  d_digits[i] = d_digits[i+1];
	}
      
      d_digits[NUM_MAX_DIGITS*2-1] = 0;

      exponent_adjust++;
    }
  
  for(int i=0; i<NUM_MAX_DIGITS; i++)
    {
      r->digits[i] = d_digits[i];
    }

  exponent_adjust = 0 - exponent_adjust;
  
  //dbq("exp adj:%d", exponent_adjust);

  r->exponent += exponent_adjust;
  num_normalise(r);

  dbq_num_exploded("%s result", r);

  // Summarise
  fprintf(numfp, "\n(%s): %s",  __FUNCTION__, num_as_text(a1, ""));
  fprintf(numfp,      " + %s",                num_as_text(b1, ""));
  fprintf(numfp,      " = %s",                num_as_text(r, ""));
}

////////////////////////////////////////////////////////////////////////////////

// Check all lower digits except the top one is zero

int all_lower_zero(int n, NOPL_FLOAT_DIG *lb)
{
  int all_zero = 1;
  
  for(int i=n/2+1; i<n-1; i++)
    {
      if( lb[i] != 0 )
	{
	  return(0);
	}
    }
  return(1);
}

////////////////////////////////////////////////////////////////////////////////
//
// Divide two floats
//
// Uses long division algorithm.
//
// Builds double length numbers for division, leaves one zero in MS position
// to prevent times table overflow.
//

void num_div(NOPL_FLOAT *a1, NOPL_FLOAT *b1, NOPL_FLOAT *r)
{
  NOPL_FLOAT_DIG d_digits[NUM_MAX_DIGITS*2];

  // We cannot alter our parameters
  NOPL_FLOAT af = *a1;
  NOPL_FLOAT bf = *b1;
  NOPL_FLOAT *a = &af;
  NOPL_FLOAT *b = &bf;

  dbq("DIV");
  dbq_num("%s a:", a);
  dbq_num("%s b:", b);

  // Don't divide by zero
  int is_zero = 1;
  
  for(int i=0; i<NUM_MAX_DIGITS; i++)
    {
      if( b->digits[i] != 0 )
	{
	  is_zero = 0;
	  break;
	}
    }

  if( is_zero )
    {
      runtime_error(ER_MT_DI, "Divide by zero");
      return;
    }

  // We know the answer when dividing zero by anything
  is_zero = 1;
  
  for(int i=0; i<NUM_MAX_DIGITS; i++)
    {
      //dbq("dig %d:%d",i, a->digits[i]);
      
      if( a->digits[i] != 0 )
	{
	  is_zero = 0;
	  break;
	}
    }

  if( is_zero )
    {
      *r = *a;
      dbq("Dividing zero");
      return;
    }

  //------------------------------------------------------------------------------

  // Zero the result
  for(int i=0; i<NUM_MAX_DIGITS; i++)
    {
      r->digits[i] = 0;
    }

  //------------------------------------------------------------------------------
  
  dbq("DIV");
  dbq_num("%s a:", a);
  dbq_num("%s b:", b);

  // Sort out the signs
  //
  if( a->sign == b-> sign )
    {
      r->sign = NUM_SIGN_POSITIVE;
    }
  else
    {
      r->sign = NUM_SIGN_NEGATIVE;
    }

  // Sort out exponent
  r->exponent = a->exponent - b->exponent;

  //------------------------------------------------------------------------------
  //
  // Long division algorithm.
  //
  
  dbq_num_exploded("%s a:", a);
  dbq_num_exploded("%s b:", b);

  // Now divide mantissas
#define LONGER_DIGITS  (NUM_MAX_DIGITS*2+2)
  
  NOPL_FLOAT_DIG  w[LONGER_DIGITS];    // working register
  NOPL_FLOAT_DIG  res[LONGER_DIGITS];    // result register
  NOPL_FLOAT_DIG  ttable[LONGER_DIGITS*10];  // Times table
  NOPL_FLOAT_DIG  la[LONGER_DIGITS];        // long version of a
  NOPL_FLOAT_DIG  lb[LONGER_DIGITS];        // long version of b
  NOPL_FLOAT_DIG  te[LONGER_DIGITS];        // copy of table entry
  int8_t          exponent = 0;

  //
  // Make a longer version of b
  //
  // To prevent times table overflow, we leave a zero in the MS position
  //
  
  num_clear_digits(LONGER_DIGITS, lb);
  
  for(int i=0; i<NUM_MAX_DIGITS; i++)
    {
      lb[i+NUM_MAX_DIGITS-1] = b->digits[i];
    }

  // Put significant digits in top half
  while( !all_lower_zero(LONGER_DIGITS, lb) )
    {
      num_shift_digits_left_n(LONGER_DIGITS, lb, &exponent);
    }

  r->exponent -= exponent;
  
  // Make a longer version of a
  num_clear_digits(LONGER_DIGITS, la);
  for(int i=0; i<NUM_MAX_DIGITS; i++)
    {
      la[i] = a->digits[i];
    }
  
  num_clear_digits(LONGER_DIGITS, w);
  num_clear_digits(LONGER_DIGITS, res);

  int done = 0;
  int a_digit_pos = 0;
  int divides_by = 0;
  	    
  num_db_digits("\nw:",   LONGER_DIGITS, w);
  num_db_digits("\nres:", LONGER_DIGITS, res);

  num_build_times_table(LONGER_DIGITS, ttable, lb);
  
  while(!done)
    {
      if( a_digit_pos >= LONGER_DIGITS )
	{
	  done = 1;
	  continue;
	}
      
      // Bring digit down into the w register, add to the end of any number in w
      //num_cat_digit(LONGER_DIGITS, w, la[a_digit_pos]);

      // Shift top half left and put digit in the last position of the top half
      for(int i=0; i<LONGER_DIGITS/2+1; i++)
	{
	  w[i] = w[i+1];
	}

      w[LONGER_DIGITS/2+1] = la[a_digit_pos];

      num_db_digits("test w :\n",   LONGER_DIGITS, w);
      //num_db_digits("\ntest tt:",   LONGER_DIGITS, ttable);
      
      // Is it possible to divide b into w?
      if( divides_by = num_divides_into(LONGER_DIGITS, w, lb, ttable) )
	{
	  // We have another digit of the result
	  res[a_digit_pos] = divides_by;

	  // Leave remainder in w
	  // Use a copy of table entry as tens complement writes into that array
	  for(int i=0; i<LONGER_DIGITS; i++)
	    {
	      te[i] = ttable[(LONGER_DIGITS*divides_by)+i];
	    }
		
	  //	  num_sub_digits(LONGER_DIGITS, w, &(ttable[(LONGER_DIGITS*divides_by)]), w);
	  num_sub_digits(LONGER_DIGITS, w, &(te[0]), w);
	  
	  // Remove overflow
	  if( w[0] > 10 )
	    {
	      w[0] -= 10;
	    }
	}
      else
	{
	  // Result digit is zero
	  res[a_digit_pos] = 0;
	}

      dbq(" ***** Divides_by: %d", divides_by);
      num_db_digits("\nres:", LONGER_DIGITS, res);
      num_db_digits("\nw:  ", LONGER_DIGITS, w);
      a_digit_pos++;
    }

  // Shift so first significant digit is in res[0], so we get
  // full resolutio of significant digits in result.

  exponent = 0;
  
  while( res[0] == 0 )
    {
      num_shift_digits_left_n(LONGER_DIGITS, res, &(r->exponent));
    }

  // Adjust exponent due to larger double length calculations
  (r->exponent)+=3;
  
  num_db_digits("\nres after shift:", LONGER_DIGITS, res);

  // Round the last digit up
  if( res[NUM_MAX_DIGITS] >=5 )
    {
      res[NUM_MAX_DIGITS-1]++;
    }

  // Now propagate any carry
  num_propagate_carry_digits(res, LONGER_DIGITS);

  num_db_digits("\nres after round:", LONGER_DIGITS, res);

  for(int i=0; i<NUM_MAX_DIGITS; i++)
    {
      r->digits[i] = res[i];
    }

  num_normalise(r);
  dbq_num_exploded("%s result", r);
}

////////////////////////////////////////////////////////////////////////////////
//
// Comparisons
//
////////////////////////////////////////////////////////////////////////////////

int num_eq(NOPL_FLOAT *a, NOPL_FLOAT *b)
{
  if( a->exponent == b->exponent )
    {
      if( a->sign == b->sign )
	{
	  for(int i=0; i<NUM_MAX_DIGITS; i++)
	    {
	      if( a->digits[i] != b->digits[i] )
		{
		  return(0);
		}
	    }
	  return(1);
	}
    }
  
  return(0);
}

//------------------------------------------------------------------------------
//
// a > b?
//

int num_gt(NOPL_FLOAT *a, NOPL_FLOAT *b)
{
  dbq("NUM GT");
  dbq_num("a: %s", a);
  dbq_num("b: %s", b);

    // Different signs then positive is greater
  if( a->sign != b->sign )
    {
      if( NUM_POSITIVE(a) )
	{
	  dbq("sign chk ret1");
	  return(1);
	}
      else
	{
	  dbq("sign chk ret0");
	  return(0);
	}
    }
  
  // Signs the same, but negative numbers have opposite sense
  // so swap them
  if( NUM_NEGATIVE(a) )
    {
      NOPL_FLOAT *t;

      t = a;
      a = b;
      b = t;
    }


  // Check exponents
  if( a->exponent > b->exponent )
    {
      dbq("exp chk ret1");
      return(1);
    }

  if( a->exponent < b->exponent )
    {
      dbq("exp chk ret0");
      return(0);
    }

  // Test mantissas as exponents are equal
  
  
  for(int i=0; i<NUM_MAX_DIGITS; i++)
    {
       if( a->digits[i] == b->digits[i] )
	 {
	   continue;
	 }
       
      if( a->digits[i] > b->digits[i] )
	{
	  dbq("dig chk ret1");
	  return(1);
	}
     else
	{
	  dbq("dig chk ret0");
	  return(0);
	}
    }
  
  dbq("dig chk ret0");
  return(0);
}

//------------------------------------------------------------------------------
//
// Take interger part of float
//

void num_intf(NOPL_FLOAT *a, NOPL_FLOAT *r)
{
  // If exp>NUM_MAX_DIGITS then there's no fractional part
  if( a->exponent >= NUM_MAX_DIGITS )
    {
      *r = *a;
      return;
    }

  // Otherwise zero the last NUM_MAX_DIGITS-exponent digits-1
  for(int i = 0; i<NUM_MAX_DIGITS-(a->exponent)-1; i++)
    {
      a->digits[NUM_MAX_DIGITS-1-i] = 0;
    }
  
  *r = *a;
}

void num_sqr(NOPL_FLOAT *a, NOPL_FLOAT *r)
{
  long double d;

  d = num_to_double(a);
  d = sqrtl(d);
  num_from_double(r, d);
}

void num_log(NOPL_FLOAT *a, NOPL_FLOAT *r)
{
  long double d;

  d = num_to_double(a);
  d = logl(d);
  num_from_double(r, d);
}

void num_exp(NOPL_FLOAT *a, NOPL_FLOAT *r)
{
  long double d;

  d = num_to_double(a);
  d = expl(d);
  num_from_double(r, d);
}

void num_log10(NOPL_FLOAT *a, NOPL_FLOAT *r)
{
  long double d;

  d = num_to_double(a);
  d = log10l(d);
  num_from_double(r, d);
}

void num_sin(NOPL_FLOAT *a, NOPL_FLOAT *r)
{
  long double d;

  d = num_to_double(a);
  d = sinl(d);
  num_from_double(r, d);
}

void num_pow(NOPL_FLOAT *a, NOPL_FLOAT *b, NOPL_FLOAT *r)
{
  long double da;
  long double db;

  da = num_to_double(a);
  db = num_to_double(b);
  da = powl(da, db);
  num_from_double(r, da);
}

void num_pi(NOPL_FLOAT *r)
{
  r->exponent = 0;
  r->sign = NUM_SIGN_POSITIVE;
  r->digits[0]  = 3;
  r->digits[1]  = 1;
  r->digits[2]  = 4;
  r->digits[3]  = 1;
  r->digits[4]  = 5;
  r->digits[5]  = 9;
  r->digits[6]  = 2;
  r->digits[7]  = 6;
  r->digits[8]  = 5;
  r->digits[9]  = 3;
  r->digits[10] = 5;
  r->digits[11] = 9;
  
}

void num_100(NOPL_FLOAT *r)
{
  r->exponent = 2;
  r->sign = NUM_SIGN_POSITIVE;
  r->digits[0]  = 1;
  r->digits[1]  = 0;
  r->digits[2]  = 0;
  r->digits[3]  = 0;
  r->digits[4]  = 0;
  r->digits[5]  = 0;
  r->digits[6]  = 0;
  r->digits[7]  = 0;
  r->digits[8]  = 0;
  r->digits[9]  = 0;
  r->digits[10] = 0;
  r->digits[11] = 0;
}

void num_1(NOPL_FLOAT *r)
{
  r->exponent = 0;
  r->sign = NUM_SIGN_POSITIVE;
  r->digits[0]  = 1;
  r->digits[1]  = 0;
  r->digits[2]  = 0;
  r->digits[3]  = 0;
  r->digits[4]  = 0;
  r->digits[5]  = 0;
  r->digits[6]  = 0;
  r->digits[7]  = 0;
  r->digits[8]  = 0;
  r->digits[9]  = 0;
  r->digits[10] = 0;
  r->digits[11] = 0;
}

void num_180_div_pi(NOPL_FLOAT *r)
{
  r->exponent = 1;
  r->sign = NUM_SIGN_POSITIVE;
  r->digits[0]  = 5;
  r->digits[1]  = 7;
  r->digits[2]  = 2;
  r->digits[3]  = 9;
  r->digits[4]  = 5;
  r->digits[5]  = 7;
  r->digits[6]  = 7;
  r->digits[7]  = 9;
  r->digits[8]  = 5;
  r->digits[9]  = 1;
  r->digits[10] = 3;
#if 0
  r->digits[11] = 1;
#else
  r->digits[11] = 0;
#endif
}

void num_cos(NOPL_FLOAT *a, NOPL_FLOAT *r)
{
  long double d;

  d = num_to_double(a);
  d = cosl(d);
  num_from_double(r, d);
}

void num_tan(NOPL_FLOAT *a, NOPL_FLOAT *r)
{
  long double d;

  d = num_to_double(a);
  d = tanl(d);
  num_from_double(r, d);
}

void num_abs(NOPL_FLOAT *a, NOPL_FLOAT *r)
{
  long double d;

  *r = *a;
  r->sign = 0;
}

void num_asin(NOPL_FLOAT *a, NOPL_FLOAT *r)
{
  long double d;

  d = num_to_double(a);
  d = asinl(d);
  num_from_double(r, d);
}

void num_acos(NOPL_FLOAT *a, NOPL_FLOAT *r)
{
  long double d;

  d = num_to_double(a);
  d = acosl(d);
  num_from_double(r, d);
}

void num_atan(NOPL_FLOAT *a, NOPL_FLOAT *r)
{
  long double d;

  d = num_to_double(a);
  d = atanl(d);
  num_from_double(r, d);
}

//------------------------------------------------------------------------------

int num_zero(NOPL_FLOAT *a)
{
  for(int i = 0; i< NUM_MAX_DIGITS; i++)
    {
      if( a->digits[i] != 0 )
	{
	  return(0);
	}
    }
  
  return(1);
}

int num_not(NOPL_FLOAT *a)
{
  if( num_zero(a) )
    {
      return(NOBJ_TRUE);
    }
  
  return(NOBJ_FALSE);
}

int num_true(NOPL_FLOAT *a)
{
  if( num_zero(a) == NOBJ_FALSE )
    {
      return(1);
    }
  
  return(0);
}


int num_and(NOPL_FLOAT *a, NOPL_FLOAT *b)
{
  if( num_true(a) && num_true(b) )
    {
      return(NOBJ_TRUE);
    }
  
  return(NOBJ_FALSE);
}

int num_or(NOPL_FLOAT *a, NOPL_FLOAT *b)
{
  if( num_true(a) || num_true(b) )
    {
      return(NOBJ_TRUE);
    }
  
  return(NOBJ_FALSE);
}

//------------------------------------------------------------------------------

int num_ne(NOPL_FLOAT *a, NOPL_FLOAT *b)
{
  return(!num_eq(a, b));
}

int num_lt(NOPL_FLOAT *a, NOPL_FLOAT *b)
{
  return(num_gt(b, a) && num_ne(a,b));
}

int num_gte(NOPL_FLOAT *a, NOPL_FLOAT *b)
{
  return(num_lt(b, a));
}

int num_lte(NOPL_FLOAT *a, NOPL_FLOAT *b)
{
  return(num_gt(b, a));
}

////////////////////////////////////////////////////////////////////////////////

long double num_to_double(NOPL_FLOAT *a)
{
  long double res = 0.0;

  for(int i=0; i< NUM_MAX_DIGITS; i++)
    {
      res *= 10.0L;
      res += (long double)(a->digits[i]);
      dbq("Mantissa:%Lf digit:%d", res, a->digits[i]);
    }

  dbq("Mantissa:%Lf", res);
  res /= NUM_MAX_DIGITS_POWER10;
  res *= 10.0L;

  dbq("Adjusted%Lf", res);
    
  if( a->sign )
    {
      res = -res;
    }

  dbq("Signed:%Lf", res);
  res *= powl(10.0, a->exponent);

  dbq("Exp added:%Lf", res);
  
  dbq_num("in:", a);
  dbq("Out:%Lf", res);
  return(res);
}

//------------------------------------------------------------------------------

void num_from_double(NOPL_FLOAT *a, long double d)
{
  char dstr[40];
  int m0;
  char m[40];
  int exp;
  int nscanf = 0;
  
  sprintf(dstr, "%LE", d);
  dbq("dstr:'%s'", dstr);

  if( dstr[0] == '-' )
    {
      dbq("Negative");
      if( (nscanf = sscanf(dstr, "-%d.%[^E]E%d", &m0, m, &exp))==3 )
	{
	  dbq("scanf: - %d . %s E %d", m0, m, exp);
	}
      else
	{
	  dbq("sscanf didn't work");
	  dbq("scanf n(%d): - %d . %s E %d", nscanf, m0, m, exp);
	  
	  return;
	}
    }
  else
    {
      dbq("Positive");
      if( (nscanf = sscanf(dstr, "%d.%[^E]E%d", &m0, m, &exp))==3 )
	{
	  dbq("scanf: %d . %s E %d", m0, m, exp);
	}
      else
	{
	  dbq("sscanf didn't work");
	  dbq("scanf n(%d): %d . %s E %d", nscanf, m0, m, exp);
	  
	  return;
	}
    }
  
  dbq("dstr:%s", dstr);
  
  if( d < 0.0 )
    {
      a->sign = NUM_SIGN_NEGATIVE;
    }
  else
    {
      a->sign = 0;
    }

  a->exponent = exp;


  for(int i=0; i<NUM_MAX_DIGITS; i++)
    {
      a->digits[i+1] = 0;
    }

  a->digits[0] = m0;

  for(int i=0; i<strlen(m); i++)
    {
      a->digits[i+1] = m[i]-'0';
    }
  
  dbq_num("res:", a);
}

//------------------------------------------------------------------------------

NOPL_FLOAT num_float_from_str(char *str)
{

}

//------------------------------------------------------------------------------


int where_char(char *s, char c)
{
  for(int i = 0; i<strlen(s); i++)
    {
      if( s[i] == c )
	{
	  return(i);
	}
    }

  return(-1);
}

int has_char(char *s, char c)
{
  if( where_char(s, c)== -1 )
    {
      return(0);
    }
  return(1);
}

void insert_char(char *s, char c)
{
  char *lastc = s+strlen(s);

  dbq("Insert '%c' at '%s'", c, s);
  
  while(lastc >= s)
    {
      *(lastc+1) = *lastc;
      lastc--;
    }

  *s = c;
  dbq("After insert:'%s'", s);  
}
//------------------------------------------------------------------------------
//
// Negative width is right justification signal
//
// If data won't fit in field, field becomes asterisks
//

void num_force_str_to_width(char *s, int w)
{
  int right_just = 0;

  dbq("Forcing '%s' to width %d", s, w);
  
  if( w < 0 )
    {
      right_just = 1;
      w = -w;
    }

  dbq("%s justified.", right_just?"Right":"Left");
  
  // Check for too large 
  if( strlen(s) > w )
    {
      dbq("Too wide");
      for(int i= 0; i<w; i++)
	{
	  s[i] = '*';
	}
      
      s[w] = '\0';
    }

  // Pad
  // Now force the value to be the required width
  if( strlen(s) < w )
    {
      dbq("Too narrow, padding");
      if( !right_just )
	{
	  while( strlen(s) < w )
	    {
	      strcat(s, " ");
	    }
	}
      else
	{
	  // Add spaces to front of string
	  while( strlen(s) < w )
	    {
	      insert_char(s, ' ');
	    }
	}
    }

  dbq("Now '%s'", s);
}

//------------------------------------------------------------------------------
//
// Round up
//

void num_round_up(NOPL_FLOAT *n, int numdp)
{
  NOPL_FLOAT delta, n1;

  n1 = *n;
  
  num_clear(&delta);
  delta.digits[0] = 5;
  delta.sign = n->sign;
  delta.exponent = -(numdp+1);

  num_add(&n1, &delta, n);
}

//------------------------------------------------------------------------------
//
// Set number of decimal places.
//
// Has to handle scientific notoation and floats and integers
//
// Integers: No DP, zero DPs added
// Sci:      Fix DPS between dot and E
// floats:   Fix between dot and end of string

void num_set_decimal_places(char *ns, int dp)
{
  dbq("Setting '%s' to %d decimal places", ns, dp);
  
  // Work out what type of number we have
  if( has_char(ns, 'E'),0 )
    {
      // Scientific notation
      dbq("Scientific notation, exiting");
      dbq("Now '%s'", ns);
      return;
    }

  if( has_char(ns, '.') )
    {
      // Float
      int i = where_char(ns, '.');
      int num_dp = 0;
      
      dbq("Has decimal point at %d", i);            
      i++;
#if 0
      while( (isdigit(ns[i]) && (dp > 0)) || (num_dp < dp) )
	{
	  dbq("Digit %02X at %d, dp:%d num_dp:%d", ns[i], i, dp, num_dp);
	  num_dp++;
	  dp--;
	  i++;
	}

      i--;
      dp++;
#endif

      while( isdigit(ns[i]) )
	{
	  dbq("Digit %02X at %d, num_dp:%d", ns[i], i, num_dp);
	  num_dp++;
	  i++;
	}
      
      // At first non-digit
      dbq("Digit %02X at %d, num_dp:%d", ns[i], i, num_dp);
      if( num_dp < dp )
	{
	  // Add zeros
	  dbq("Digits end at %d, inserting chars if needed", i);
	  
	  while( num_dp < dp )
	    {
	      insert_char(&(ns[i]), '0');
	      num_dp++;
	    }
	}
      else
	{
	  dbq("Truncating");
	  
	  // Truncate string
	  if( ns[i-num_dp+dp] != 'E' )
	    {
	      ns[i+-num_dp+dp] = '\0';
	    }
	}

      // If last char is a dot then remove it
      if( ns[strlen(ns)-1] == '.' )
	{
	  ns[strlen(ns)-1] = '\0';
	  dbq("Removed trailing dot");
	}

      // End of DPs is here
      dbq("Now '%s'", ns);
      return;
    }

  // Must be integer, we add zeroes on the end
  if( (strlen(ns) + dp + 1) > NOBJ_STRING_MAXLEN )
    {
      runtime_error(ER_LX_ST, "FIX$ result too long");
    }

  dbq("Integer");
  
  strcat(ns, ".");
  for(int i=0; i<dp; i++)
    {
      strcat(ns, "0");
    }

  dbq("Now '%s'", ns);
}

//------------------------------------------------------------------------------

void num_force_sci_asterisk(char *s)
{
  // If there's an E in the number then turn into asterisks
  for(int i=0; i<strlen(s); i++)
    {
      if( s[i] == 'E' )
	{
	  for(int i=0; i<strlen(s); i++)
	    {
	      s[i] = '*';
	    }
	}
    }
}

//------------------------------------------------------------------------------
//
// Remove training zeros given a string float and the exponent
//
// For positive exponent don't drop zeros
//

void remove_trailing_zeros(char *n)
{
  // If there's no decimal point then don't drop zeros
  int has_dot = 0;

  dbq("Removing trailing zeros from '%s'", n);
  
  if( !has_char(n, '.') )
    {
      dbq("Had no decimal point, exiting");
      dbq("Now '%s'", n);
      return;
    }

  dbq("Removing...");
  for(int i = strlen(n)-1; i>0; i--)
    {
      switch( n[i] )
	{
	case '0':
	  n[i] = '\0';
	  break;

	case '.':
	  n[i+1] = '0';
	  return;
	  break;
	  
	default:
	  return;
	  break;
	}
    }
  
  dbq("Now '%s'", n);
}

int calc_num_sigdigs(NOPL_FLOAT *n)
{
  int sigdigs = NUM_MAX_DIGITS;

  dbq("Calculating number of sig digits");
  
  for(int i = NUM_MAX_DIGITS-1; i>0; i--)
    {
      switch(n->digits[i])
	{
	case 0:
	  sigdigs--;
	  break;

	default:
	  dbq("Sig digits:%d", sigdigs);
	  return(sigdigs);
	  break;
	}
    }

  dbq("Sig digits:%d", sigdigs);
  return(sigdigs);
}

//------------------------------------------------------------------------------
//
// Convert float to text in scientific notation.
//
//

char *num_to_sci_text(NOPL_FLOAT *n, int decpl)
{
  char temp[10];

  dbq("decpl:%d", decpl);

  num_text[0] = '\0';
 
  if(n->sign)
    {
      strcat(num_text, "-");
    }
  
  sprintf(temp, "%d.", n->digits[0]);
  strcat(num_text, temp);

  dbq("%s", num_text);
  
  for(int i=1; i<NUM_MAX_DIGITS; i++)
    {
      sprintf(temp, "%d", n->digits[i]);
      strcat(num_text, temp);
    }

  dbq("'%s'", num_text);
  
  remove_trailing_zeros(num_text);
  dbq("After remove: '%s'", num_text);

  if( decpl >= 0 )
    {
      num_set_decimal_places(num_text, decpl);
      dbq("After set dec places: '%s'", num_text);
    }
  
  char sign;
  int exp = n->exponent;
  
  if( n->exponent < 0 )
    {
      sign = '-';
      exp = -exp;
    }
  else
    {
      sign = '+';
    }
  
  sprintf(temp, "E%c", sign);
  strcat(num_text, temp);

  sprintf(temp, "%02d", exp);
  strcat(num_text, temp);
  
  dbq("%s", num_text);
  return(num_text);
}

//------------------------------------------------------------------------------
//
// Size of this array is number of digits plus extra for signs and exponent.
//
// Scientific notation for exponents > NUM_MAX_DIGITS
// and -exponent < -NUM_MAX_DIGITS


#define EXP (n->exponent)
#define EXP_LIM 14

char *num_to_text(NOPL_FLOAT *n)
{
  char temp[10];
  int num_sigdigs = calc_num_sigdigs(n);
  
  num_text[0] = '\0';
  
  if( ((EXP > 0) && (EXP >  EXP_LIM)) ||
      ((EXP < 0) && (EXP < -EXP_LIM)) ||
      ((EXP < 0) && (-EXP+num_sigdigs > EXP_LIM)))
    {
      
      num_to_sci_text(n, -1);
      
      return(num_text);
    }
  else
    {
      if(n->sign)
	{
	  strcat(num_text, "-");
	}

      // Not scientific format
      if( EXP >= 0 )
	{
	  for(int i=0; i<NUM_MAX_DIGITS; i++)
	    {
	      // Add a digit
	      sprintf(temp, "%d", n->digits[i]);
	      strcat(num_text, temp);

	      // Add a dot if we have the same number of digits as the exponent
	      if( (EXP == i) /* && (num_sigdigs != EXP+1)*/ )
		{
		  strcat(num_text, ".");
		}
	    }
	}
      else
	{
	  // Negative exponent
	  int i = EXP;
	  while(i<0)
	    {
	      strcat(num_text, "0");
	      if( (EXP == i)  )
		{
		  strcat(num_text, ".");
		}
	      i++;
	    }
	  
	  for(int i=0; i<NUM_MAX_DIGITS; i++)
	    {
	      sprintf(temp, "%d", n->digits[i]);
	      strcat(num_text, temp);
	    }
	}

      remove_trailing_zeros(num_text);
      return(num_text);
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Convert a float to a string version integer
//

char *num_to_int_text(NOPL_FLOAT *n, int width)
{
  char frag[3];
  int right_just = 0;

  if( width < 0 )
    {
      right_just = 1;
      width = -width;
    }
  
  //printf("\nnum int exponent:%d", n->exponent);
  
  num_text[0] = '\0';

  if( n->exponent < 0 )
    {
      strcpy(num_text, "0");
      return(num_text);
    }

  //printf("\nnum int 2");
    
  // Put significant figures in the buffer until exponent reaches 0
  int i;
  int j;
  
  for(i=n->exponent, j=0; (i>=0) && (strlen(num_text) < width); i--,j++)
    {
      //printf("\n%s", num_text);
      
      if( j>= NUM_MAX_DIGITS )
	{
	strcat(num_text, "0");
	}
      else
	{
	  sprintf(frag, "%d", n->digits[j]);
	  strcat(num_text, frag);
	}
    }

  printf("\nAfter loop 1: i:%d '%s'", i, num_text);

  if( i > 0 )
    {
      // Field not big enough, fill with *
      for(i=0; i<width;i++)
	{
	  num_text[i] = '*';
	}
      
      num_text[i] = '\0';
      return(num_text);
    }
  
  for(i=0; i<width;i++)
    {
      //printf("\n%s", num_text);
      if( strlen(num_text) < width )
	{
	  strcat(num_text, " ");
	}
    }

  //printf("\ndone: '%s'", num_text);
  return(num_text);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#if 0

////////////////////////////////////////////////////////////////////////////////
//
// Conversion to and from C types
//
////////////////////////////////////////////////////////////////////////////////
//
// Psion datatypes are pointed to and stored in Psion format. Arrays are not
// converted as arrays, the elements have to be accessed and converted individually
//
// For now floats are converted to doubles, really a BCD data type should be used.
//

NOPL_INT psion_int(uint8_t *p)
{
  NOPL_INT x;

  x  = ((int)(*(p++)))<<8;
  x += *p;
  
  return(x);
}

////////////////////////////////////////////////////////////////////////////////
//
// Convert a Psion float to a NOPL_FLOAT
//
////////////////////////////////////////////////////////////////////////////////

NOPL_FLOAT psion_float(uint8_t *p)
{
  NOPL_FLOAT nf;
  
  uint64_t m = 0;
  uint64_t mul = 1;
  
  for(int i = 0; i<6; i++)
    {
      int z = *(p++);
      double y = (double)((z & 0xF) + ((z>>4)*10));

      m +=  y * mul;
      mul *= 100.0;

#if DB_PSION_FLOAT
      printf("\n         m:%lu  mul:%lu z:%d y:%lf", m, mul, z, y);
      
#endif

    }

  nf.m = m;
  
  // Signed byte for exponent, binary
  int es = (int8_t)(*(p++));
  nf.e = es;
  
  nf.s = *p;
    
  return(nf);
}

////////////////////////////////////////////////////////////////////////////////
//
// Convert a NOPL_FLOAT to a double
//
////////////////////////////////////////////////////////////////////////////////

double nopl_float_to_double(NOPL_FLOAT *nf)
{
  double r;

  r = nf->m;
  r *= pow(10, (nf->e)-11);
  if( (nf->s) == 0 )
    {
    }
  else
    {
      r = -r;
    }
  
  return(r);
}

#endif

////////////////////////////////////////////////////////////////////////////////
//
// Initialise the number handling code
//
////////////////////////////////////////////////////////////////////////////////



void num_init(void)
{
  numfp = fopen("num_info.txt", "w");

  fprintf(numfp, "\nSummary of numerical calculations");
}

void num_uninit(void)
{
  fclose(numfp);
}
  
