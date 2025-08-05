#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate_all.h"

////////////////////////////////////////////////////////////////////////////////
//
// Display string on line and scroll so it is all viewable.
// return when key is pressed

uint64_t scroll_after_this_time = 0;
 
KEYCODE dp_view(char *str, int line)
{
  int scroll_dir = 1;
  int offset = 0;
  int done = 0;
  char ch;
  char w_str[DP_MAX_STR+3];
  int offset2 = 0;
  
#if DB_DP_VIEW
  printf("\n%s:Entry", __FUNCTION__);
#endif
  
  strcpy(w_str, str);
  strcat(w_str, "  ");
  
  //  i_printxy_str(0, line, w_str);

  int swlen = strlen(w_str);
  int slen = strlen(str);
  uint64_t now = time_us_64 ();
    
#if DB_DP_VIEW
  printf("\n%s:Loop entry", __FUNCTION__);
#endif

  while(!done)
    {
      menu_loop_tasks();
      
      offset2 = 0;

      // Don't scroll strings that fit on a line
      if( slen <= DP_NUM_CHARS )
	{
	  scroll_dir = 0;
	}

      now = time_us_64 ();
 
      if( (now > scroll_after_this_time) )
        {
          // Scroll every 200ms
          scroll_after_this_time += 200000;
          
          // Display DP_LEN_CHARS characters from the string
          for(int i=0; i<DP_NUM_CHARS; i++)
            {
#if DB_DP_VIEW
              //printf("\n%s:i:%d off:%d scrldir:%d", __FUNCTION__, i, offset, scroll_dir);
#endif
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
      
      if( kb_test() != KEY_NONE )
	{
	  KEYCODE k = kb_getk();

#if DB_DP_VIEW
	  printf("\n%s:Key:%d", __FUNCTION__, k);
#endif
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
              int len = strlen(w_str);
              
              for(int j=0; j<DP_NUM_CHARS; j++)
                {
                  if( j >= len )
                    {
                      i_printxy(j, line, ' ');
                    }
                  else
                    {
                      i_printxy(j, line, w_str[j]);
                    }
                }
	      //i_printxy_str(0, line, w_str);
	      return(k);
	      break;
	    }
	}

    }

#if DB_DP_VIEW
  printf("\n%s:Exit", __FUNCTION__);
#endif
  
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
  printf("\n%s: printpos_x:%d printpos_y:%d", printpos_x, printpos_y);
  
  int save_pp_x = printpos_x;
  int save_pp_y = printpos_y;
  
  for(int i=printpos_x; i<DISPLAY_NUM_CHARS-1; i++)
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
//
// Does not disrupt printpos_x,y
//

void dp_cls(void)
{
  //int sx, sy;
  //sx = printpos_x;
  //sy = printpos_y;
  
  for(int x=0; x<DISPLAY_NUM_CHARS; x++)
    {
      for(int y=0; y<DISPLAY_NUM_LINES; y++)
	{
	  i_printxy(x, y, ' ');
	}
    }
  
  print_home();

  printpos_x = 0;
  printpos_y = 0;
  cursor_x = 0;
  cursor_y = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Emit one character
//
// Updates the print position
//

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

void dp_newline(void)
{
  next_printpos_line();
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
