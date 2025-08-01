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

////////////////////////////////////////////////////////////////////////////////
//
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
  
  //sleep_ms(500);
  
  while(!done)
    {
      menu_loop_tasks();
      
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
	  printf("\n%s:KC:%d", __FUNCTION__, k);
	  
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
      else
	{
	  // Delay for the scrolling lines
	  
	  sleep_ms(200);	  
	}
      
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

////////////////////////////////////////////////////////////////////////////////

void dp_stat(int x, int y, int curs_on, int cursor_block)
{
  printpos_x       = 0;
  printpos_y       = 0;
  printpos_at_end  = 0;

  cursor_on = curs_on;
  cursor_blink = cursor_block;
}

////////////////////////////////////////////////////////////////////////////////
//
// Clear to end of line
// Does not affect cursor position
//

void dp_clr_eol(void)
{
  int save_pp_x = printpos_x;
  int save_pp_y = printpos_y;
  
  while( save_pp_y == printpos_y )
    {
      dp_prnt(" ");
      //i_printxy(printpos_x++, printpos_y, ' ');
    }
  
  printpos_x = save_pp_x;
  printpos_y = save_pp_y;

}

////////////////////////////////////////////////////////////////////////////////
//
// Clear to end of screen
// Does not affect cursor position
//

void dp_clr_eos(void)
{
  int save_pp_x = printpos_x;
  int save_pp_y = printpos_y;
  
  while( !printpos_at_end )
    {
      dp_prnt(" ");
      //i_printxy(printpos_x, printpos_y, ' ');
    }
  
  printpos_x = save_pp_x;
  printpos_y = save_pp_y;
}

////////////////////////////////////////////////////////////////////////////////
//
// Clears the display
//
////////////////////////////////////////////////////////////////////////////////

void dp_cls(void)
{
  for(int x=0; x<DISPLAY_NUM_CHARS; x++)
    {
      for(int y=0; y<DISPLAY_NUM_LINES; y++)
	{
	  i_printxy(x, y, ' ');
	}
    }
  
  print_home();
}

////////////////////////////////////////////////////////////////////////////////

void dp_emit(int ch)
{
  switch(ch)
    {
    case CHRCODE_TAB:
      dp_clr_eol();
      next_printpos_line();
      
      break;
      
    default:
      i_printxy(printpos_x, printpos_y, ch);
      break;
    }
}

////////////////////////////////////////////////////////////////////////////////


void dp_prnt(char *s)
{
  while(*s != '\0')
    {
      dp_emit(*s);
      
      s++;
    }
}
