////////////////////////////////////////////////////////////////////////////////
//
// Clock support
//
// Provides clock services when running on the Pico
//
//
////////////////////////////////////////////////////////////////////////////////


#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>

#include "psion_recreate_all.h"

//#include "nopl.h"

typedef enum
  {
    CC_SECOND = 1,
    CC_MINUTE,
    CC_HOUR,
    CC_DAY,
    CC_MONTH,
    CC_YEAR,
  } CLK_COMP;


void qca_clock_component(NOBJ_MACHINE *m, NOBJ_QCS *s, CLK_COMP comp)
{
  time_t now;
  struct tm *tm;
  int value = 0;
   
  now = time(0);
  tm = localtime (&now);

  if( tm != NULL )
    {
	
      switch(comp)
	{
	case CC_SECOND:
	  value = tm->tm_sec;
	  break;

	case CC_MINUTE:
	  value = tm->tm_min;
	  break;

	case CC_HOUR:
	  value = tm->tm_hour;
	  break;

	case CC_DAY:
	  value = tm->tm_mday;
	  break;

	case CC_MONTH:
	  value = tm->tm_mon;
	  break;

	case CC_YEAR:
	  value = tm->tm_year;
	  break;

	default:
	  value = 0;
	  break;
	}
    }
  else
    {
      // Return zero
      value = 0;
    }

  push_machine_16(m, value);
      
}

void qca_clock_second(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  qca_clock_component(m, s, CC_SECOND);
}

void qca_clock_minute(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  qca_clock_component(m, s, CC_MINUTE);
}

void qca_clock_hour(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  qca_clock_component(m, s, CC_HOUR);
}

void qca_clock_day(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  qca_clock_component(m, s, CC_DAY);
}

void qca_clock_month(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  qca_clock_component(m, s, CC_MONTH);
}

void qca_clock_year(NOBJ_MACHINE *m, NOBJ_QCS *s)
{
  qca_clock_component(m, s, CC_YEAR);
}

void qca_clock_pause(int del)
{
  sleep_ms(del*1000);
}
