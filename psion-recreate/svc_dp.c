#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate.h"

#include "menu.h"
#include "emulator.h"
#include "eeprom.h"
#include "rtc.h"
#include "display.h"
#include "record.h"
#include "svc.h"
#include "svc_kb.h"
#include "svc_dp.h"

// Display string on line and scroll so it is all viewable.
// return when key is pressed

KEYCODE dp_view(char *str, int line)
{
  int scroll_dir = 1;
  int offset = 0;
  int done = 0;
  char ch;
  char w_str[DP_MAX_STR+3];
  int offset2 = 0;
  
  strcpy(w_str, str);
  strcat(w_str, "  ");
  
  i_printxy_str(0, line, w_str);

  int swlen = strlen(w_str);
  int slen = strlen(str);
  
  sleep_ms(500);
  
  while(!done)
    {
      offset2 = 0;

      // Don't scroll strings that fit on a line
      if( slen <= DP_NUM_CHARS )
	{
	  scroll_dir = 0;
	}
      
      // Display DP_LEN_CHARS characters from the string
      for(int i=0; i<DP_NUM_CHARS; i++)
	{
	  //printf("\ni:%d off:%d scrldir:%d", i, offset, scroll_dir);
	  
	  if( (i+offset) >= swlen )
	    {
	      if( swlen > DP_NUM_CHARS )
		{
		  ch = w_str[(offset2++) % swlen];
		}
	      else
		{
		  ch = ' ';
		}
	    }
	  else
	    {
	      ch = w_str[(i+offset) % swlen];
	    }
	  
	  i_printxy(i, line, ch);
	}
      
      if( kb_test() != KEY_NONE )
	{
	  KEYCODE k = kb_getk();
	  //	  printf("\nKC:%d", k);
	  
	  switch(k)
	    {
	    case KEY_NONE:
	      break;
	      
	    case KEY_LEFT:
	      switch(scroll_dir)
		{
		case -1:
		  break;

		case 1:
		  scroll_dir = 0;
		  break;

		case 0:
		  scroll_dir = -1;
		  break;
		}
	      break;
	      
	    case KEY_RIGHT:
	      switch(scroll_dir)
		{
		case 1:
		  break;

		case -1:
		  scroll_dir = 0;
		  break;

		case 0:
		  scroll_dir = 1;
		  break;
		}
	      break;
	      
	    default:

	      // Re-display string in starting position
	      i_printxy_str(0, line, w_str);
	      return(k);
	      break;
	    }
	}

      sleep_ms(200);
      
      offset += scroll_dir;

      if( offset < 0 )
	{
	  offset = swlen-1;
	}
      
      if( offset >= swlen )
	{
	  offset = 0;
	}
    }
  
  return(KEY_NONE);  
}
