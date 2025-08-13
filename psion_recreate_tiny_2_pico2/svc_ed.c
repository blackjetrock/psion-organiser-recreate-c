// ED service

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate_all.h"

#include "nos.h"

////////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
////////////////////////////////////////////////////////////////////////////////

// Update the display for the epos service.
//
// Prompt at start of display
// Then the line being edited (which can be on multiple lines, delimited by tabs)
//
//

char line1[DP_MAX_CHARS+1];

void display_epos(char *str_in, char *epos_prompt, int insert_point, int cursor_line, int display_start_index, int single_nmulti_line)
{
  char *str;
  int px, py;

  px = printpos_x;
  py = printpos_y;
  
#if DB_ED_EPOS
  printf("\n%s: Entry InsPt:%d cursline:%d dispstrt:%d", __FUNCTION__, insert_point, cursor_line, display_start_index);
  printf("\n%s: Entry printpos_x:%d printpos_y:%d",      __FUNCTION__, printpos_x, printpos_y);
  printf("\n%s: Epos prompt:'%s'", __FUNCTION__, epos_prompt);
  printf("\n");
#endif

#if 0
  i_printxy_str(0, cursor_line, epos_prompt);
#else
  //i_printxy_str(0, py, epos_prompt);
#endif
  
  // Ensure single line doesn't go onto other lines
  if( single_nmulti_line )
    {
      // Single line mode, ensure we stay on the current line
      int i;
      
      for(i=0; (i<DP_MAX_CHARS) && str_in[i] != '\0'; i++)
        {
          line1[i] = str_in[i+display_start_index];
        }

      line1[i] = '\0';
    }
  else
    {
      // Multi line, can display on all lines
      strcpy(line1, str_in);
    }

  str = line1;
  
#if DB_ED_EPOS
  printf("\n%s: Entry InsPt:%d cursline:%d dispstrt:%d", __FUNCTION__, insert_point, cursor_line, display_start_index);
  printf("\n%s: Entry printpos_x:%d printpos_t:%d",      __FUNCTION__, printpos_x, printpos_y);
  printf("\n%s: Str:'%s'", __FUNCTION__, str);
#endif

  printpos_y = cursor_line;
  printpos_x = px;
  cursor_x = px+insert_point;

  dp_prnt(str+display_start_index);

  if( single_nmulti_line,1 )
    {
    dp_clr_eol();
    }
  
  //cursor_x = printpos_x;
  cursor_y = printpos_y;  
  
  //dp_clr_eos();

#if DB_ED_EPOS
  printf("\n%s: Entry InsPt:%d cursline:%d dispstrt:%d", __FUNCTION__, insert_point, cursor_line, display_start_index);
  printf("\n%s: Entry printpos_x:%d printpos_t:%d",      __FUNCTION__, printpos_x, printpos_y);
  printf("\n");
#endif

  // Cursor is at the insert_point position
#if 0
  cursor_x = printpos_x;
  cursor_y = printpos_y;
#endif
  int cursor_i = /*strlen(epos_prompt)+*/insert_point;

  //cursor_x = cursor_i % display_num_chars();
  //cursor_x = px;
  
#if 0
  cursor_y = cursor_i / display_num_chars();
#else
  cursor_y = cursor_line;
#endif
  
  //printf("\n%s:cursor_x:%d cursor_y:%d",      __FUNCTION__, cursor_x, cursor_y);

  printpos_x = px;
  printpos_y = py;
  
#if DB_ED_EPOS
  printf("\n%s: Exit", __FUNCTION__);
#endif
  
}


////////////////////////////////////////////////////////////////////////////////


KEYCODE ed_edit(char *str, int len, int exit_on_mode)
{
  //printpos_x = 0;
  ed_epos(str, len, ED_SINGLE_LINE, exit_on_mode, printpos_y);
  return(KEY_NONE);
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


KEYCODE ed_epos(char *str, int len, int single_nmulti_line, int exit_on_mode, int cursor_line)
{
  int done = 0;
  char charstr[2] = "";
  int insert_point        = 0;  // Index in str where next character should be inserted
  int display_start_index = 0;  // The index of the first character in the string that is currently
                                // displayed
  KEYCODE k;
  
  // printpos defines any prompt before the string
#if DB_ED_EPOS
  printf("\n%s: Entry", __FUNCTION__);
  printf("\n");
  sleep_ms(100);
#endif

  epos_px = printpos_x;
  epos_py = printpos_y;

  //dp_cls();
  
#if 0
  // Copy prompt
  for(int i=0; i<DISPLAY_NUM_CHARS*DISPLAY_NUM_LINES; i++)
    {
      epos_prompt[i] = under_cursor_char[i%DISPLAY_NUM_CHARS][i/DISPLAY_NUM_CHARS];
    }
  
  epos_prompt[epos_py*DISPLAY_NUM_CHARS+epos_px] = '\0';
#endif
  
#if DB_ED_EPOS
  printf("\n%s:Epos prompt:'%s'", __FUNCTION__, epos_prompt);
#endif


  cursor_on();
  cursor_blink = 1;

#if 1
  cursor_x = printpos_x;
  cursor_y = printpos_y;
#endif

#if 1
  int cursor_i = strlen(epos_prompt)+insert_point;
#endif
  
#if DB_ED_EPOS
  printf("\n%s:InsPt:%d cursline:%d dispstrt:%d", __FUNCTION__, insert_point, cursor_line, display_start_index);
  printf("\n%s:printpos_x:%d printpos_y:%d",      __FUNCTION__, printpos_x, printpos_y);
  printf("\n%s:cursor_x:%d cursor_y:%d",          __FUNCTION__, cursor_x, cursor_y);
  printf("\n%s:cursor_i:%d",          __FUNCTION__, cursor_i);
  printf("\n");
#endif

#if 0
  cursor_x = cursor_i % display_num_chars();
  cursor_y = cursor_i / display_num_lines();
#endif
  
#if DB_ED_EPOS
  printf("\n%s:InsPt:%d cursline:%d dispstrt:%d", __FUNCTION__, insert_point, cursor_line, display_start_index);
  printf("\n%s:printpos_x:%d printpos_y:%d",      __FUNCTION__, printpos_x, printpos_y);
  printf("\n%s:cursor_x:%d cursor_y:%d",      __FUNCTION__, cursor_x, cursor_y);
  printf("\n");
#endif

  //i_printxy_str(cursor_x, cursor_y, "");
 
  ed_state = ED_STATE_EDIT;

  //  cursor_line = 0;
  insert_point = 0;
  display_start_index = 0;

  // Display initial string
#if DB_ED_EPOS
  printf("\n%s: Init", __FUNCTION__);
  printf("\n");
  sleep_ms(100);
#endif
  
  display_epos(str, epos_prompt, insert_point, cursor_line, display_start_index, single_nmulti_line);
  
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
                  cursor_off();
                  done = 1;
                  break;
              
                case KEY_EXE:
                  done = 1;
                  break;

                case KEY_SHIFT_DEL:
                  done = 1;
                  break;
                  
                case KEY_LEFT:
                  if( insert_point > 0)
                    {
                      insert_point--;
                    }
              
                  if( insert_point < display_start_index )
                    {
                      display_start_index = insert_point;
                    }
                  break;
              
                case KEY_RIGHT:
                  if( insert_point < strlen(str))
                    {
                      insert_point++;
                    }
              
                  if( insert_point > display_start_index+display_num_chars()-3 )
                    {
                      display_start_index = insert_point-display_num_chars()+3;
                    }
                  break;
                  
                case KEY_DEL:
                  if( insert_point > 0 )
                    {
                      // Move string over insert_point
                      insert_point--;
                      int end = strlen(str)-1;
                  
                      for(int i=insert_point; i<=end; i++)
                        {
                          str[i] = str[i+1];
                        }
                      str[end] = '\0';
                    }
                  break;

                case KEY_DOWN:
                case KEY_UP:
                  return(k);
                  break;
                  
                default:
                  // Insert character at insert point
                  charstr[0] = k;
              
                  // Shift up
                  for(int i=strlen(str); i>=insert_point; i--)
                    {
                      str[i+1] = str[i];
                    }
                  str[insert_point++] = k;
              
                  //strcat(str, charstr);
                  break;
                }
              break;
            }
      
          // Update the display
          printpos_x = epos_px;
          printpos_y = epos_py;
          
          display_epos(str, epos_prompt, insert_point, cursor_line, display_start_index, single_nmulti_line);
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

////////////////////////////////////////////////////////////////////////////////
//
// Import string into line buffers, splitting at TAB characters.
//

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

////////////////////////////////////////////////////////////////////////////////

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
      int len = strlen(ed_edit_buffer[i]);
                       
      for(int j=0; j<DP_NUM_CHARS; j++)
        {
          if( j >= len )
            {
              i_printxy(j, i, ' ');
            }
          else
            {
              i_printxy(j, i, ed_edit_buffer[i][j]);
            }
        }
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


  
