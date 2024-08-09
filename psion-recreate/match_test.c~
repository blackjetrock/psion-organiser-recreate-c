////////////////////////////////////////////////////////////////////////////////
//
// Tests the match function.
//
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

#include "match.h"

#define DEBUG 0

typedef struct _TEST
{
  char *a;
  char *b;
  int correct_result;
  int cns;                        // correct num scanned , -1 means use length of test string
  int int_arg[MAX_INT_ARGS];
} TEST;

TEST testlist[] =
  {
   {"abc",    "%%abc",   0, -1, {0}},
   {"abc",    "abc%%",   0, -1, {0}},
   {"%abc",    "%%abc",   1, -1, {0}},
   {"a%bc",    "a%%bc",   1, -1, {0}},
   {"ab%c",    "ab%%c",   1, -1, {0}},
   {"abx",    "abc",   0, -1, {0}},
   
   {"abcx",    "abc",   1, 3, {0}},
   {"abcxy",   "abc",   1, 3, {0}},
   {"abcxyz",   "abc",   1, 3, {0}},
   

   {"1",    "%d",   1, -1, {1}},
   {"12",    "%d",   1, -1, {12}},
   {"123",    "%d",   1, -1, {123}},
   
   {"abc",    "abc",   1, -1, {0}},
   {"abc",    "abc",   1, -1, {0}},
   {"abc",    "bbc",   0, -1, {0}},
   {"abc9",   "abc%d", 1, -1, {9}},
   {"9abc",   "%dabc", 1, -1, {9}},
   {"98abc",  "%dabc", 1, -1, {98}},
   {"989abc", "%dabc", 1, -1, {989}},
   
   {"abc",    "%dabc", 0, -1, {0}},
   {"abc",    "a%dbc", 0, -1, {0}},
   {"abc",    "ab%dc", 0, -1, {0}},
   {"abc",    "abc%d", 0, -1, {0}},
   
   {"a1bc",   "a%dbc", 1, -1, {1}},
   {"ab1c",   "ab%dc", 1, -1, {1}},

   {"9",      "%d", 1, -1, {9}},
   {"0",      "%d", 1, -1, {0}},
   {"12",     "%d", 1, -1, {12}},
   {"123",    "%d", 1, -1, {123}},
   {"1234",   "%d", 1, -1, {1234}},
   {"12345",  "%d", 1, -1, {12345}},

   {"x12y34",  "x%dy%d", 1, -1, {12,34}},
   {"12x34y",  "%dx%dy", 1, -1, {12,34}},
   {"x12y34z",  "x%dy%dz", 1, -1, {12,34}},
   {"12x34y56",  "%dx%dy%d", 1, -1, {12,34,56}},
   {"x12x34y56",  "x%dx%dy%d", 1, -1, {12,34,56}},
   {"12xyz34y56",  "%dxyz%dy%d", 1, -1, {12,34,56}},
   {"12x34y56xyz",  "%dx%dy%d", 1, 8, {12,34,56}},
   {"xyz12x34y56",  "xyz%dx%dy%d", 1, -1, {12,34,56}},

   {"GET /",  "GET /", 1, -1, {0}},
   {"AT+CIPSERVER=1,80",  "AT+CIPSERVER=%d,%d", 1, -1, {1,80}},
   {"GET /otherstuff",  "GET /", 1, 5, {0}},
   {"AT+CIPSERVER=1,80otherstuff",  "AT+CIPSERVER=%d,%d", 1, 17, {1,80}},

   {"AT+CIPSEND=1,568OK>",  "AT+CIPSEND=%d,%dOK>", 1, 19, {1,568}},
   {"AT+CIPSEND=1,5",  "AT+CIPSEND=%d,%dOK>", 0, 19, {1,5}},

   {"12\n34",   "12 34", 1, -1, {0}},
   {"12\n\r34",   "12 34", 1, -1, {0}},
   {"12\n\r 34",   "12 34", 1, -1, {0}},
   {"\r\n1234",   " 1234", 1, -1, {0}},
   {"1234\r\n",   " 1234 ", 1, -1, {0}},
   

   {"1234",   " 1234 ", 1, -1, {0}},
   {"1234",   "12 34 ", 1, -1, {0}},
   {"",       "1234 ", 0, -1, {0}},
   {"",       " 1234 ", 0, -1, {0}},

   {"x21y41",  "x%xy%x", 1, -1, {33,65}},
   {"x1000yffff",  "x%xy%x", 1, -1, {4096,65535}},
   {"x1000yFFFF",  "x%xy%x", 1, -1, {4096,65535}},
   {"GET /memory/FFFF",  "GET /memory/%x", 1, -1, {65535}},
   {"GET /memory/FFFF stuff",  "GET /memory/%x", 1, 16, {65535}},
   

  };

#define NUM_TESTS (sizeof(testlist)/sizeof(TEST))

int main(void)
{
  int good_tests = 0;
  int bad_tests = 0;
  int result = 0;
  
  for(int i=0; i<NUM_TESTS; i++)
    {
      int overall = 1;
      int correct_num_scanned = (testlist[i].cns==-1)?strlen(testlist[i].a):testlist[i].cns;
      
      result = match(testlist[i].a, testlist[i].b);

      printf("\n%20s %20s res=%d cor=%d cns=%d ns=%d", testlist[i].a, testlist[i].b, result, testlist[i].correct_result, correct_num_scanned, match_num_scanned);


      if( (match_num_scanned == correct_num_scanned) || (result == 0) )
	{
	  printf("  ");
	}
      else
	{
	  overall = 0;
	  printf(" *");
	}

      if( result == testlist[i].correct_result )
	{
	  printf("  ");
	}
      else
	{
	  overall = 0;
	  printf(" *");
	}


      int ok = 1;
      for(int j=0; j<MAX_INT_ARGS; j++)
	{
	  if(match_int_arg[j] == testlist[i].int_arg[j] )
	    {
	    }
	  else
	    {
	      ok = 0;
	    }
	  printf(" (%d,%d)", match_int_arg[j], testlist[i].int_arg[j]);
	}

      if( ok || (result == 0) )
	{
	  printf("   ");
	}
      else
	{
	  overall = 0;
	  printf(" * ");
	}

      if( overall )
	{
	  good_tests++;
	}
      else
	{
	  bad_tests++;
	}

      printf("  %s %s %d", match_err, match_err_parm, match_err_int_parm);
    }

  printf("\n\nGood tests:%d", good_tests);
  printf("\n\nBad  tests:%d", bad_tests);
  printf("\n");
}
