#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate_all.h"

//------------------------------------------------------------------------------
//
// Cursor
#if !PICOCALC
#define CURSOR_CHAR        0x100
#define CURSOR_UNDERLINE   0x101
#endif

#if PICOCALC
#define CURSOR_CHAR        250
#define CURSOR_UNDERLINE   251
#endif

int cursor_on = 0;
uint64_t cursor_upd_time = 1000000L;
uint64_t cursor_last_time = 0;
volatile int cursor_phase = 0;
int cursor_char = 0x101;
int under_cursor_char[DISPLAY_NUM_CHARS][DISPLAY_NUM_LINES];
int under_char;
int cursor_x = 3;
int cursor_y = 0;
int cursor_blink = 0;

int force_cursor_update = 0;

#if 0
void cursor_task(void)
{
  int ch;
  
  if( cursor_on)
    {
      u_int64_t now = time_us_64();
      if( ((now - cursor_last_time) > cursor_upd_time) || force_cursor_update )
	{
	  force_cursor_update = 0;
	  cursor_last_time = now;
	  cursor_phase = !cursor_phase;

	  if( cursor_phase )
	    {
	      if( cursor_blink )
		{
		  //		  saved_char = ch;
		  
		  // Solid blinking block
		  ch = CURSOR_CHAR;
		}
	      else
		{
		  // We have the character code, copy it and underline it in
		  // the underline cursor entry in the font table
		  create_underline_char(under_cursor_char[cursor_x][cursor_y], CURSOR_UNDERLINE);	      
		  
		  //saved_char = ch;
		  // Non blinking underline of character
		  ch = CURSOR_UNDERLINE;
		}
	      
	      print_cursor(cursor_x, cursor_y, ch);
	    }
	  else
	    {
	      // Other phase of cursor
	      // Which cursor?
	      if( cursor_blink )
		{
		  // Underline of char
		  create_underline_char(under_cursor_char[cursor_x][cursor_y], CURSOR_UNDERLINE);	      
		  //saved_char = ch;
		  
		  // Solid blinking block
		  //		  ch = CURSOR_UNDERLINE;
		  ch = under_cursor_char[cursor_x][cursor_y];
		}
	      else
		{
		  // We have the character code, copy it and underline it in
		  // the underline cursor entry in the font table
		  create_underline_char(under_cursor_char[cursor_x][cursor_y], CURSOR_UNDERLINE);	      
		  
		  //saved_char = ch;
		  // Non blinking underline of character
		  // on both phases
		  ch = CURSOR_UNDERLINE;
		}
	      
	      print_cursor(cursor_x, cursor_y, ch);
	    }
	}
    }
}
#endif


#if 1

int last_cursor_x = -1;
int last_cursor_y = -1;

void cursor_task(void)
{
  int ch;
  
  if( cursor_on)
    {
      if( last_cursor_x != -1 )
        {
          if( (cursor_x != last_cursor_x) || (cursor_y != last_cursor_y) )
            {
              // Cursor has moved, put character back
              print_cursor(last_cursor_x, last_cursor_y, under_cursor_char[last_cursor_x][last_cursor_y]);
            }
        }

      u_int64_t now = time_us_64();
      if( ((now - cursor_last_time) > cursor_upd_time) || force_cursor_update )
	{
	  force_cursor_update = 0;
	  cursor_last_time = now;
	  cursor_phase = !cursor_phase;

	  if( cursor_phase )
	    {
	      if( cursor_blink )
		{
		  // Solid blinking block
		  ch = CURSOR_CHAR;
		}
	      else
		{
		  // We have the character code, copy it and underline it in
		  // the underline cursor entry in the font table
		  create_underline_char(under_cursor_char[cursor_x][cursor_y], CURSOR_UNDERLINE);	      
                  //create_underline_char(, CURSOR_UNDERLINE);	      

                  //saved_char = ch;
		  // Non blinking underline of character
		  ch = CURSOR_UNDERLINE;
		}
	      
	      print_cursor(cursor_x, cursor_y, ch);
	    }
	  else
	    {
	      // Other phase of cursor
	      // Which cursor?
	      if( cursor_blink )
		{
		  // Underline of char
		  //create_underline_char(under_cursor_char[cursor_x][cursor_y], CURSOR_UNDERLINE);	      
		  //saved_char = ch;
		  
		  // Solid blinking block
		  //		  ch = CURSOR_UNDERLINE;
		  //ch = under_cursor_char[cursor_x][cursor_y];
                  ch = under_cursor_char[cursor_x][cursor_y];
		}
	      else
		{
		  // We have the character code, copy it and underline it in
		  // the underline cursor entry in the font table
		  create_underline_char(under_cursor_char[cursor_x][cursor_y], CURSOR_UNDERLINE);	      
		  
		  //saved_char = ch;
		  // Non blinking underline of character
		  // on both phases
		  //ch = under_char;
                  ch = under_cursor_char[cursor_x][cursor_y];
		}
	      
	      print_cursor(cursor_x, cursor_y, ch);
	    }
	}
      last_cursor_x = cursor_x;
      last_cursor_y = cursor_y;
    }
}
#endif


// We have to put the original character back then move the cursor then force
// an update

void handle_cursor_key(KEYCODE k)
{
  // Put original character back
  print_cursor(cursor_x, cursor_y, under_cursor_char[cursor_x][cursor_y]);
  cursor_phase = 0;
  
  switch( k )
    {
    case 'B':
      cursor_blink = !cursor_blink;
      break;
	      
    case KEY_LEFT:
      if( cursor_x>0 )
	{
	  cursor_x--;
	}
      break;
	  
    case KEY_UP:
      if( cursor_y>0 )
	{
	  cursor_y--;
	}
      break;

    case KEY_RIGHT:
      if( cursor_x<(DISPLAY_NUM_CHARS-1) )
	{
	  cursor_x++;
	}
      break;
	  
    case KEY_DOWN:
      if( cursor_y<(DISPLAY_NUM_LINES-1) )
	{
	  cursor_y++;
	}
      break;
	  
    }

  force_cursor_update = 1;
}


// Which display mapping we are using
u_int8_t *model_mapping;

void create_underline_char(int ch, int dest_code)
{
  int i;
  
  for(i=0; i<5; i++)
    {
      font_5x7_letters[dest_code*5+i] = font_5x7_letters[ch*5+i] | 0x80;
    }

}
