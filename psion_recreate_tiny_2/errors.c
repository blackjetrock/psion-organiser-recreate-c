#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdarg.h>

#include "nopl.h"


typedef struct _ERROR_INFO
{
  int code;
  char *desc;
} ERROR_INFO;

ERROR_INFO error_info[] =
  {
    {ER_AL_NC, "NO MORE ALLOCATOR CELLS"},
    {ER_AL_NR, "NO MORE ROOM"},
    {ER_MT_EX, "EXPONENT OVERFLOW (OR UNDERFLOW)"},
    {ER_MT_IS, "CONVERSION FROM STRING TO NUMERIC FAILED"},
    {ER_MT_DI, "DIVIDE BY ZERO"},
    {ER_MT_FL, "CONVERSION FROM NUMERIC TO STRING FAILED"},
    {ER_IM_OV, "BCD STACK OVERFLOW"},
    {ER_IM_UN, "BCD STACK UNDERFLOW"},
    {ER_FN_BA, "BAD ARGUMENT IN FUNCTION CALL"},
    {ER_PK_NP, "NO PACK IN SLOT"},
    {ER_PK_DE, "DATAPACK ERROR (WRITE ERROR)"},
    {ER_PK_RO, "ATTEMPTED WRITE TO READ ONLY PACK"},
    {ER_PK_DV, "BAD DEVICE NAME"},
    {ER_PK_CH, "PACK CHANGED"},
    {ER_PK_NB, "PACK NOT BLANK"},
    {ER_PK_IV, "UNKNOWN PACK TYPE"},
    {ER_FL_PF, "PACK FULL"},
    {ER_FL_EF, "END OF FILE"},
    {ER_FL_BR, "BAD RECORD TYPE"},
    {ER_FL_BN, "BAD FILE NAME"},
    {ER_FL_EX, "FILE ALREADY EXISTS"},
    {ER_FL_NX, "FILE DOES NOT EXIST"},
    {ER_FL_DF, "DIRECTORY FULL"},
    {ER_FL_CY, "PACK NOT COPYABLE"},
    {ER_DV_CA, "INVALID DEVICE CALL"},
    {ER_DV_NP, "DEVICE NOT PRESENT"},
    {ER_DV_CS, "CHECKSUM ERROR"},
    {ER_EX_SY, "SYNTAX ERROR"},
    {ER_EX_MM, "MISMATCHED BRACKETS"},
    {ER_EX_FA, "WRONG NUMBER OF FUNCTION ARGS"},
    {ER_EX_AR, "SUBSCRIPT OR DIMENSION ERROR"},
    {ER_EX_TV, "TYPE VIOLATION"},
    {ER_LX_ID, "IDENTIFIER TOO LONG"},
    {ER_LX_FV, "BAD FIELD VARIABLE NAME"},
    {ER_LX_MQ, "UNMATCHED QUOTES IN STRING"},
    {ER_LX_ST, "STRING TOO LONG"},
    {ER_LX_US, "UNRECOGNISED SYMBOL"},
    {ER_LX_NM, "NON-VALID NUMERIC FORMAT"},
    {ER_TR_PC, "MISSING PROCEDURE DECLARATION"},
    {ER_TR_DC, "ILLEGAL DECLARATION"},
    {ER_TR_IN, "NON-INTEGER DIMENSION"},
    {ER_TR_DD, "NAME ALREADY DECLARED"},
    {ER_TR_ST, "STRUCTURE ERROR"},
    {ER_TR_ND, "NESTING TOO DEEP"},
    {ER_TR_NL, "LABEL REQUIRED"},
    {ER_TR_CM, "MISSING COMMA"},
    {ER_TR_BL, "BAD LOGICAL FILE NAME"},
    {ER_TR_PA, "ARGUMENTS MAY NOT BE TARGET OF ASSIGN"},
    {ER_TR_FL, "TOO MANY FIELDS"},
    {ER_RT_BK, "BREAK KEY"},
    {ER_RT_NP, "WRONG NUMBER OF PARAMETERS"},
    {ER_RT_UE, "UNDEFINED EXTERNAL"},
    {ER_RT_PN, "PROCEDURE NOT FOUND"},
    {ER_RT_ME, "MENU ERROR"},
    {ER_RT_NF, "FIELD NOT FOUND"},
    {ER_PK_BR, "PACK BAD READ ERROR"},
    {ER_RT_FO, "FILE ALREADY OPEN (OPEN/DELETE)"},
    {ER_RT_RB, "RECORD TOO BIG"},
    {ER_LG_BN, "BAD PROCEDURE NAME"},
    {ER_RT_FC, "FILE NOT OPEN (CLOSE)"},
    {ER_RT_IO, "INTEGER OVERFLOW"},
    {ER_GN_BL, "BATTERY TOO LOW"},
    {ER_GN_RF, "DEVICE READ FAIL"},
    {ER_GN_WF, "DEVICE WRITE FAIL"},
    {ER_FL_NP, "ABC"},
    {ER_TM_MN, "Month number out of range"},
  };

#define NUM_ERROR_INFO (sizeof(error_info)/sizeof(ERROR_INFO))


////////////////////////////////////////////////////////////////////////////////
#if 0
void runtime_error(int error code, char *fmt, ...)
{
  va_list valist;
  char line[80];
  
  va_start(valist, fmt);

  vsprintf(line, fmt, valist);
  va_end(valist);

  dbq("Internal compiler error ***");
  dbq("\n\n\n*** Internal compiler error ***\n");

  printf("\n%s\n", line);
}
#else

void runtime_error(int error_code, char *fmt, ...)
{
  // Signal to the QCode loop that an error has occurred
  current_machine->error_occurred = 1;
  current_machine->rtb_eror = error_code;

  va_list valist;
  char line[80];
  
  va_start(valist, fmt);
  
  vsprintf(line, fmt, valist);
  va_end(valist);

  strcpy(current_machine->error_string, line);
}

//------------------------------------------------------------------------------

// Runtime error only if not trapped, otherwise just set up error code

void trappable_runtime_error(int error_code, char *fmt, ...)
{
  if( !current_machine->rtb_trap )
    {
      // Signal to the QCode loop that an error has occurred
      current_machine->error_occurred = 1;
    }

  current_machine->rtb_eror = error_code;

  va_list valist;
  char line[80];
  
  va_start(valist, fmt);
  
  vsprintf(line, fmt, valist);
  va_end(valist);

  strcpy(current_machine->error_string, line);
}

void runtime_error_print(void)
{
  dbq("Runtime error: error code: %d ***", current_machine->rtb_eror);
  dbq("\n*** Internal compiler error ***\n");
  dbq("\n%s\n", error_text(current_machine->rtb_eror));
  dbq("\n%s\n", current_machine->error_string);
  
  printf("\nRuntime error: error code: %d ***", current_machine->rtb_eror);
  printf("\n*** Internal compiler error ***");
  printf("\n%s", error_text(current_machine->rtb_eror));
  printf("\n%s\n", current_machine->error_string);
}

void runtime_error_msg(int error_code, char *fmt, ...)
{
  va_list valist;
  char line[80];

  va_start(valist, fmt);

  vsprintf(line, fmt, valist);
  va_end(valist);

  dbq("Runtime error: error code: %d ***", error_code);
  dbq("\n*** Internal compiler error ***\n");
  dbq("\n%s\n", error_text(error_code));
  dbq("\n%s\n", line);
  
  printf("\nRuntime error: error code: %d ***", error_code);
  printf("\n*** Internal compiler error ***");
  printf("\n%s", error_text(error_code));
  printf("\n%s\n", line);
}
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Return the string that describes the error
//
// This is the string you see on the Psion
//

char *error_text(int error_code)
{
  for(int i=0; i<NUM_ERROR_INFO; i++)
    {
      if( error_code == error_info[i].code )
	{
	  return(error_info[i].desc);
	}
    }
  return("???");
}
