// ED service

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate_all.h"

#if 0
#include "menu.h"
#include "emulator.h"
#include "eeprom.h"
#include "rtc.h"
#include "display.h"
#include "record.h"
#include "svc_kb.h"
#include "svc_dp.h"
#include "svc_ed.h"
#endif

#include "nos.h"

////////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
////////////////////////////////////////////////////////////////////////////////

// Update the display for the epos service.

void display_epos(char *str, char *epos_prompt, int insert_point, int cursor_line, int display_start_index)
{
#if DB_ED_EPOS
  printf("\n%s: Entry InsPt:%d cursline:%d dispstrt:%d", __FUNCTION__, insert_point, cursor_line, display_start_index);
  printf("\n%s: Entry printpos_x:%d printpos_t:%d",      __FUNCTION__, printpos_x, printpos_y);
  printf("\n");
#endif

  i_printxy_str(0, 0, epos_prompt);
    
#if DB_ED_EPOS
  printf("\n%s: Entry InsPt:%d cursline:%d dispstrt:%d", __FUNCTION__, insert_point, cursor_line, display_start_index);
  printf("\n%s: Entry printpos_x:%d printpos_t:%d",      __FUNCTION__, printpos_x, printpos_y);
  printf("\n");
#endif

  dp_prnt(str+display_start_index);

  dp_clr_eos();

#if DB_ED_EPOS
  printf("\n%s: Entry InsPt:%d cursline:%d dispstrt:%d", __FUNCTION__, insert_point, cursor_line, display_start_index);
  printf("\n%s: Entry printpos_x:%d printpos_t:%d",      __FUNCTION__, printpos_x, printpos_y);
  printf("\n");
#endif
  
  cursor_x = printpos_x;
  cursor_y = printpos_y;

#if DB_ED_EPOS
  printf("\n%s: Exit", __FUNCTION__);
#endif
  
}


////////////////////////////////////////////////////////////////////////////////


KEYCODE ed_edit(char *str)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// Standard line editor
//
// str: string to edit
// len: Max length of str
// single_nmulti_line:  1 single line, 0:multi line
//
//
// Returns terminating key code

typedef enum
  {
   ED_STATE_INIT = 1,
   ED_STATE_EDIT,
  } ED_STATE;

ED_STATE ed_state = ED_STATE_INIT;


int epos_px = 0;
int epos_py = 0;
char epos_prompt[DISPLAY_NUM_CHARS*DISPLAY_NUM_LINES];

KEYCODE ed_epos(char *str, int len, int single_nmulti_line, int exit_on_mode)
{
  int done = 0;
  char charstr[2] = "";
  int insert_point        = 0;  // Index where next character should be inserted
  int cursor_line         = 0;  // Which line the cursor is on (0-based)
  int display_start_index = 0;  // The index of the first character in the string that is currently
                                // displayed
  KEYCODE k;
  
  // printpos defines any prompt before the string
#if DB_ED_EPOS
  printf("\n%s: ", __FUNCTION__);
  printf("\n");
  sleep_ms(100);
#endif

  epos_px = printpos_x;
  epos_py = printpos_y;
  
  // Copy prompt
  for(int i=0; i<DISPLAY_NUM_CHARS*DISPLAY_NUM_LINES; i++)
    {
      epos_prompt[i] = under_cursor_char[i%DISPLAY_NUM_CHARS][i/DISPLAY_NUM_CHARS];
    }
  epos_prompt[epos_py*DISPLAY_NUM_CHARS+epos_px] = '\0';

  cursor_on = 1;
  cursor_blink = 1;
  cursor_x = printpos_x;
  cursor_y = printpos_y;
  
  ed_state = ED_STATE_EDIT;

  cursor_line = 0;
  insert_point = 0;
  display_start_index = 0;

  // Display initial string
#if DB_ED_EPOS
  printf("\n%s: Init", __FUNCTION__);
  printf("\n");
  sleep_ms(100);
#endif

  dp_cls();
  display_epos(str, epos_prompt, insert_point, cursor_line, display_start_index);
  
  while(!done)
    {
#if 0
  printf("\n%s: Loop", __FUNCTION__);
  printf("\n");
  sleep_ms(100);
#endif

      // Keep the display updated
      menu_loop_tasks();

      // Handle keypresses
      if( kb_test() != KEY_NONE )
	{
	  k = kb_getk();

	  cursor_phase = 0;
	  force_cursor_update = 1;
	  
	  switch(ed_state)
	    {
	      // Clear display and put string on display
	    case ED_STATE_INIT:
	      
	      break;
	      
	    case ED_STATE_EDIT:
	      switch(k)
		{
		case KEY_ON:
		  cursor_on = 0;
		  done = 1;
		  break;

		case KEY_EXE:
		  done = 1;
		  break;
		  
		  // Insert newline character
		case KEY_DOWN:
		  charstr[0] = CHRCODE_TAB;;
		  strcat(str, charstr);
		  break;
		  
		case KEY_DEL:
		  if( strlen(str) > 0 )
		    {
		      str[strlen(str)-1] = '\0';
		    }
		  break;
		  
		default:
		  charstr[0] = k;
		  strcat(str, charstr);
		  break;
		}
	      break;
	      
	    }

	  // Update the display
	  display_epos(str, epos_prompt, insert_point, cursor_line, display_start_index);
	  
	}
    }

  // return last key pressed if it caused an exit
  return(k);
  
}


////////////////////////////////////////////////////////////////////////////////
//
// ed$view service
//
// Takes a copy of the string and splits it on tabs if there are any.
// Then allows viewing of string, with scrolling of longer lines
//
// Editing is performed on characters in the edit buffer. the string
// passed in is copied ot the edit buffer and split on tabs. On exit
// the edit buffer has the edited text in it, that has to be fetched
// to get the data, that fetch will turn the lines into a single
// TAB-delimited string.

char ed_edit_buffer[ED_NUM_LINES][ED_NUM_CHARS];

void ed_dump_edit_buffer(void)
{
  printf("\nEdit Buffer\n");
  
  for( int line=0; line< ED_NUM_LINES; line++)
    {
      printf("\n%02d: '%s'", line, ed_edit_buffer[line]);
    }

  printf("\n");
}

void ed_import_str(char *str)
{
  int line = 0;
  char s[2] = " ";
  
  for(int l=0; l<ED_NUM_LINES; l++)
    {
      ed_edit_buffer[l][0] = '\0';
    }

  while(*str != '\0' )
    {
      switch(*str)
	{
	case '\n':
	case KEY_TAB:
	  line++;

	  if( line >= ED_NUM_LINES )
	    {
	      // Truncate, no space left
	      printf("\n%s:Too many lines", __FUNCTION__);
	      
	      return;
	    }
	  break;

	default:
	  s[0] = *str;	  
	  strcat(ed_edit_buffer[line], s);
	  break;
	}
      str++;
    }
}

KEYCODE ed_view(char *str, int ln)
{
  int done = 0;
  unsigned int line = ln;
  KEYCODE k = KEY_NONE;

  // Import the string into the edit buffer
  ed_import_str(str);
  ed_dump_edit_buffer();

  // Display the strings initially
  for(int i=0; i<DP_NUM_LINE; i++)
    {
      i_printxy_str(0, i, ed_edit_buffer[i]); 
    }
  
  while(!done)
    {
      menu_loop_tasks();
      
      k = dp_view(ed_edit_buffer[line], line);

      switch(k)
	{
	case KEY_EXE:
	case KEY_ON:
	  done = 1;
	  break;

	case KEY_DEL:
	  done = 1;
	  break;
	  
 	case KEY_DOWN:
	  done = 1;
	  //	  line = (line + 1) % ED_VIEW_N;
	  break;

 	case KEY_UP:
	  done = 1;
	  //line = (line - 1) % ED_VIEW_N;
	  break;
	}
    }

  return(k);
}


  
