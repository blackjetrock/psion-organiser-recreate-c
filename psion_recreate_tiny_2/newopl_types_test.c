////////////////////////////////////////////////////////////////////////////////
//
//
// Tests of the types funtions
//
//
////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "newopl.h"
#include "nopl_obj.h"
#include "newopl_types.h"

uint8_t t1[]= {0x35, 0x0b};

void do_t1(void)
{
  int x = psion_int(t1);
  
  printf("\n%s: %s %d", __FUNCTION__, (x==13579)?"PASS":"FAIL", x);
}


typedef struct _FLOAT_TEST
  {
   uint8_t data[8];
   double result;
  } FLOAT_TEST;

FLOAT_TEST float_test[] =
  {
   {{0x00, 0x90, 0x78, 0x56, 0x34, 0x12, 2,  0x80},  -123.456789},
   {{0x00, 0x90, 0x78, 0x56, 0x34, 0x12, 2,  0x00},  123.456789},
   {{0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0,  0x00},    4.5},
   {{0x00, 0x00, 0x00, 0x00, 0x00, 0x10,  99, 0x00},   1.0e99},
   {{0x00, 0x00, 0x00, 0x00, 0x00, 0x10,  99, 0x80},  -1.0e99},
   {{0x00, 0x00, 0x00, 0x00, 0x00, 0x10, -99, 0x00},   1.0e-99},
   {{0x00, 0x00, 0x00, 0x00, 0x00, 0x10, -99, 0x80},  -1.0e-99},
  };

#define NUM_FLOAT_TEST (sizeof(float_test)/sizeof(FLOAT_TEST))

void do_t2(void)
{
  int pass = 1;
  NOPL_FLOAT x;
  
  for(int t=0; t<NUM_FLOAT_TEST; t++)
    {
      printf("\n============= T2 %d test =============", t);

  
      x = psion_float(&(float_test[t].data[0]));

      if( fabs(nopl_float_to_double(&x) - float_test[t].result) < (1.0e-13) )
	{
	  double diff = (double)(nopl_float_to_double(&x) - float_test[t].result);

	  printf("\n%s: PASS %s diff:%f", __FUNCTION__, nopl_float_str(&x), diff);
	}
      else
	{
	  pass = 0;
	  double diff = (double)(nopl_float_to_double(&x) - float_test[t].result);
	  printf("\nA %s", nopl_float_str(&x));
	  printf("\nB %f",float_test[t].result);
	  printf("\nC %f",diff);
	  
	  printf("\n%s: FAIL  %s (result) <> %f (desired value) difference: %f",
		 __FUNCTION__,
		 nopl_float_str(&x),
		 float_test[t].result,
		 diff);
	}
    }
}

int main(void)
{
  printf("\n=============================\n");
  do_t1();
  printf("\n=============================\n");
  do_t2();

  printf("\n");
}
