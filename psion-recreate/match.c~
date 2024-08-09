// This is a replacement for sscanf as the sscanf in the Pico SDk seems
// to have problems.
// Hopefully this is a bit quicker too as it doesn't do as much
//
// Arguments are placed in order in
//
// match_int_arg[]
//
//
// ' ' matches any number of whitespace
//
// fmt can have %d in it which matches positive integers
//
// return value:   1 if it matches and 0 if not
//
// If there's a coding error then match fails and error vars are set up
//
////////////////////////////////////////////////////////////////////////////////

#include "match.h"

#define DEBUG 0

#if DEBUG
#include <stdio.h>
#define return_fail(XXX) printf(XXX); return(0)
#else
#define return_fail(XX) return(0)
#endif


char *match_err = "no_error";
char *match_err_parm = "none";
int match_err_int_parm = 0;

int match_num_scanned = 0;
int match_int_arg[MAX_INT_ARGS];
int match_int_arg_i = 0;

int add_int_arg(int value, char *fmt)
{
  match_int_arg[match_int_arg_i++] = value;
  
  if( match_int_arg_i >= MAX_INT_ARGS )
    {
      match_err = "Too many %d";
      match_err_parm = fmt;
      match_err_int_parm = match_int_arg_i;
      return_fail("\nToo many args\n");
    }
  
  return(1);
}

int match(char *str, char *fmt)
{
  int si = 0;
  int fi = 0;
  int argval;
  unsigned long largval;
  
  match_int_arg_i = 0;
  match_num_scanned = 0;

  for(int i=0; i<MAX_INT_ARGS; i++)
    {
      match_int_arg[i] = 0;      
    }
  
  // Continue until we reach the end of either string
  while((fmt[fi] != '\0') && (str[si] != '\0') )
    {
#if DEBUG
      printf("\nfmt = '%s' fi=%d si=%d", fmt, fi, si);
#endif
      
      switch(fmt[fi])
	{
	case ' ':
	  fi++;
	  while( isspace(str[si]) )
	    {
	      si++;
	      match_num_scanned++;
	    }
	  break;
	  
	case '%':
	  fi++;
	  if( fmt[fi] == '\0' )
	    {
	      // Error in fmt string
	      match_err = "Bad fmt";
	      match_err_parm = &(fmt[0]);
	      return_fail("\nBad fmt\n");
	    }
	  
	  switch(fmt[fi])
	    {
	    case '%':
	      // We must match a %
	      if( str[si++] == '%' )
		{
		  // All ok so far
		}
	      else
		{
		  // Mismatch
		  return_fail("\nMismatch\n");
		}

	      match_num_scanned++;
	      
	      // Move to next fmt char
	      fi++;
	      break;

	      // We match a string of digits
	    case 'd':
	    case 'x':
	      
	      if( str[si] == '\0' )
		{
		  return_fail("\nEndof string:d\n");
		}
	      
	      argval = 0;
	      largval =0;
	      
	      while( ((fmt[fi] == 'd') && isdigit(str[si])) || ((fmt[fi] == 'x') && isxdigit(str[si])) )
		{
		  char digit[2] = " ";
		  digit [0] = str[si];

		  switch(fmt[fi])
		    {
		    case 'd':
		      argval *= 10;
		      argval += atoi(digit);
		      break;

		    case 'x':
		      argval *= 16;
		      argval += (int)strtoul(digit, NULL, 16);
		      //		      argval = (int) largval;
		      break;
		    }
		  
		  si++;
		  match_num_scanned++;

		  if( str[si] == '\0' )
		    {
		      // If fmt not completed then fail
		      if( fmt[fi+1] != '\0' )
			{
			  return_fail("\nFmt not completed:1\n");
			}
		      
		      // All digits and at end so success
		      if( add_int_arg(argval, fmt) )
			{
			}
		      else
			{
			  // Error adding the arg
			  return_fail("\nError adding arg:1\n");
			}
		      
		      return(1);
		    }
		}

	      if( si == 0 )
		{
		  // Must be a mismatch
		  return_fail("\nMismatch:2\n");
		}
	      
	      if( ((fmt[fi] == 'd') && isdigit(str[si-1])) || ((fmt[fi] == 'x') && isxdigit(str[si-1])) )
		{
		  // All ok
		  if( add_int_arg(argval, fmt) )
		    {
		    }
		  else
		    {
		      // Error adding the arg
		      return_fail("\nError adding arg:2\n");
		    }
		}
	      else
		{
		  // mismatch
		  return_fail("\nMismatch:3\n");
		}
	      fi++;
	      break;

	    default:
	      break;
	    }
	  break;

	default:
	  // Literal char must match
	  if( str[si++] == fmt[fi++] )
	    {
	    }
	  else
	    {
	      return_fail("\nLiteral mismatch:4\n");
	    }
	  match_num_scanned++;
	  break;
	}
    }
  
  // If we get here then we have a match if the format string has
  // also been fully parsed
  if( fmt[fi] == '\0' )
    {
      return(1);
    }
  else
    {
      // We are not at the end of the fmt string, but it could be a space at the end
      // which is OK
      switch(fmt[fi])
	{
	case ' ':
	  // trailing space is ok, as long as there is nothing after it
	  if( fmt[fi+1] == '\0' )
	    {
	      return(1);
	    }
	  else
	    {
	      return(0);
	    }
	  break;

	default:
	  return_fail("\nEnd fail 1\n");
	  break;
	}
    }
}
