////////////////////////////////////////////////////////////////////////////////
//
// The Menu
//
// This is the menu that lives outside the emulation and allows 'meta' tasks
// to be performed.
//
// **********
//
// As it accesses the I2c, it MUST run on the core that is handling the I2C
// which is core1 at th emoment.
//
// **********
//
////////////////////////////////////////////////////////////////////////////////

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
#include "svc_kb.h"

////////////////////////////////////////////////////////////////////////////////
//
// Meta menu
//
//

int scan_keys_off = 0;

MENU  *active_menu = &menu_top;

volatile int menu_done = 0;
volatile int menu_init = 0;

MENU menu_mems;

#if 0

////////////////////////////////////////////////////////////////////////////////
//
// Scan the keyboard for keys

int scan_drive = 0x01;
int scan_state = SCAN_STATE_DRIVE;
int kb_sense = 0;
int saved_latchout1_shadow = 0;
int keycode = 0;


struct _KEYDEF
{
  int  code;
  char ch;
} menukeys[] =
    {
     {0x1002, 'A'},
     {0x1004, 'B'},
     {0x1008, 'C'},
     {0x1040, 'D'},
     {0x1010, 'E'},
     {0x1020, 'F'},
     {0x0802, 'G'},
     {0x0804, 'H'},
     {0x0808, 'I'},
     {0x0840, 'J'},
     {0x0810, 'K'},
     {0x0820, 'L'},
     {0x0402, 'M'},
     {0x0404, 'N'},
     {0x0408, 'O'},
     {0x0440, 'P'},
     {0x0410, 'Q'},
     {0x0420, 'R'},
     {0x0202, 'S'},
     {0x0204, 'T'},
     {0x0208, 'U'},
     {0x0240, 'V'},
     {0x0210, 'W'},
     {0x0220, 'X'},
     {0x0108, 'Y'},
     {0x0140, 'Z'},
     {0x0101, 'm'},
     {0x0120, 'x'},
     {0x0102, 's'},
     {0x0104, '-'},
     {0x0801, 'l'},
     {0x1001, 'r'},
     {0x0201, 'u'},
     {0x0401, 'd'},
     {0x0110, ' '},
    };

#define NUM_KEYCODES ((sizeof(menukeys))/(sizeof(struct _KEYDEF)))

#define SCAN_SKIP  3

int scan_skip = SCAN_SKIP;
int key_count = 0;
int current_key = 0;
#define DEBOUNCE_MAX  10

void scan_keys(void)
{
  return;
  scan_skip--;

  if( scan_skip == 0 )
    {
      scan_skip = SCAN_SKIP;
      return;
    }

  if( scan_keys_off )
    {
      return;
    }
    
  switch(scan_state)
    {
      
    case SCAN_STATE_DRIVE:
      
      // Drive scan lines
      latchout1_shadow &= 0x80;
      latchout1_shadow |= scan_drive;
      write_595(PIN_LATCHOUT1, latchout1_shadow, 8);

      // Store the scan drive for the building of the keycode before
      // scan drive is updated
      keycode = scan_drive;
      
      scan_drive <<= 1;
      if( scan_drive == 0x80 )
	{
	  scan_drive = 0x1;
	}

      //write_display_extra(2, 'a'+scan_drive);
      scan_state = SCAN_STATE_READ;
      break;

      
    case SCAN_STATE_READ:
      // Read port5
      //write_display_extra(2, '5');
      kb_sense = (read_165(PIN_LATCHIN) & 0xFC)>>2;

      if( kb_sense & 0x20 )
	{
	  //write_display_extra(2, 'o');
	  keychar = 'o';
	  gotkey = 1;
	}
      else
	{
	  if( (kb_sense & 0x1F) != 0 )
	    {
	      // Build keycode
	      //i_printxy_hex(5, 2, keycode);
	      //i_printxy_hex(1, 2, kb_sense);

	      //printf("\nKdr:%04X Ksense:%04X", keycode, kb_sense);

	      keycode |= (kb_sense << 8);
	      
	      // Find key from code
	      for(int i=0; i<NUM_KEYCODES; i++)
		{
		  if( menukeys[i].code == keycode )
		    {
		      if( keycode == current_key )
			{
			  // Same key pressed
			  if( key_count == DEBOUNCE_MAX )
			    {
			      // Key registered and still pressed
			      // we won't get here, we will be waiting for release
			    }
			  else
			    {
			      key_count++;
			      if( key_count == DEBOUNCE_MAX )
				{
				  // New key press
				  keychar = menukeys[i].ch;
				  gotkey = 1;

				  // Move to release state
				  scan_state = SCAN_STATE_RELEASE_READ;
				  return;
				}
			      else
				{
				  // Waiting for key to be fully pressed
				}
			    }
			}
		      else
			{
			  // Different key, start debouncing
			  key_count = 0;
			  current_key = keycode;
			}
		    }
		}
	    }
	}
      
      scan_state = SCAN_STATE_DRIVE;
      break;

    case SCAN_STATE_RELEASE_DRIVE:
      break;

    case SCAN_STATE_RELEASE_READ:
      // Read port5
      //write_display_extra(2, '5');
      kb_sense = (read_165(PIN_LATCHIN) & 0xFC)>>2;
      
      if( (kb_sense & 0x3F) != 0 )
	{
	  // Key still pressed
	}
      else
	{
	  // Count down to zero
	  if ( key_count == 0 )
	    {
	      // Already released
	    }
	  else
	    {
	      key_count--;

	      if ( key_count == 0 )
		{
		  // Key finally released
		  scan_state = SCAN_STATE_DRIVE;
		  gotkey = 0;
		  keycode = 0;
		}
	    }
	}
      break;
      
    default:
      break;
    }

  
  //scan_state = (scan_state + 1) % NUM_SCAN_STATES;
}

#endif

////////////////////////////////////////////////////////////////////////////////

void goto_menu(MENU *menu)
{
  menu_init = 1;
  active_menu = menu;
}

int do_menu_init(void)
{
  if( menu_init )
    {
      menu_init = 0;
      return(1);
    }
  
  return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Menu handling
//

void menu_process(void)
{
  // Initialise?  
  if( do_menu_init() )
    {
      display_clear();
      
      //      i_printxy_str(0, 0, active_menu->name);
      //print_nl();
      
      int e = 0;
      while( active_menu->item[e].key != '&' )
	{
	  print_nl_if_necessary(active_menu->item[e].item_text);
	  
	  print_str(active_menu->item[e].item_text);
	  print_str(" ");
	  e++;
	}

      (*active_menu->init_fn)();
    }

  
  if( gotkey )
    {
      int e = 0;
      printf("\nMenu got key");
      gotkey = 0;
      
      while( active_menu->item[e].key != '&' )
	{
	  if( keychar == active_menu->item[e].key )
	    {
	      // Call the function
	      (*active_menu->item[e].do_fn)();

	      menu_init = 1;
	      break;
	    }
	  e++;
	}
    }
}

////////////////////////////////////////////////////////////////////////////////

void init_menu_top(void)
{
  //  printxy_str(0, 0, "Meta");
}

void init_menu_eeprom(void)
{
  //printxy_str(0, 0, "EEPROM");
}

void init_menu_rtc(void)
{
  //printxy_str(0, 0, "EEPROM");
}

void init_menu_mems(void)
{
  //print_str("Memories");
}

//------------------------------------------------------------------------------

void menu_null(void)
{
}

void menu_exit(void)
{
  menu_done = 1;
}

void menu_back(void)
{
  // Wait for key to be released
  
  goto_menu(active_menu->last);
}


//------------------------------------------------------------------------------

void menu_goto_eeprom(void)
{
  goto_menu(&menu_eeprom);
}

void menu_goto_mems(void)
{
  goto_menu(&menu_mems);
}

void menu_goto_rtc(void)
{
  goto_menu(&menu_rtc);
}


//------------------------------------------------------------------------------


void init_scan_test(void)
{
  display_clear();
  printxy_str(0,0, "Key test    ");
}

void menu_scan_test(void)
{
  init_scan_test();

   while(1)
    {
      // Keep the display updated
      menu_loop_tasks();
      
      if( gotkey,1 )
	{
	  i_printxy(2, 1, '=');
	  i_printxy(3, 1, keychar);
	  
	  gotkey = 0;
	  
	  // Exit on ON key, exiting demonstrates it is working...
	  if( keychar == KEY_ON )
	    {
	      break;
	    }
	}
    }

  // Refresh menu on exit
  menu_init = 1;
}

//------------------------------------------------------------------------------

// Turn off immediately, with no dump

void menu_instant_off(void)
{
#if 0
  if( do_menu_init() )
    {
      display_clear();
      printxy_str(0, 0, "Instant Off");
    }
#endif
  
  handle_power_off();
}

//------------------------------------------------------------------------------

void menu_eeprom_invalidate(void)
{
  // This is run after a restore so we have the correct cheksum in ram
  // we can just adjust it to get a bad csum, and also fix a value
  ramdata[EEPROM_CSUM_L] = 0x11;
  ramdata[EEPROM_CSUM_H]++;
  
  // Write the checksum to the EEPROM copy
  write_eeprom(EEPROM_0_ADDR_WR , EEPROM_CSUM_H, EEPROM_CSUM_LEN, &(ramdata[EEPROM_CSUM_H]));

  display_clear();
  printxy_str(0, 0, "Invalidated");

  menu_loop_tasks();
  
  sleep_ms(3000);

  // We don't exit back to the eeprom menu, as we are still in it
  menu_init = 1;
}

//
//------------------------------------------------------------------------------
//


// Save and find records

void get_record(int n, RECORD *record_data)
{
  read_eeprom(EEPROM_1_ADDR_RD, RECORD_LENGTH*n, RECORD_LENGTH, (uint8_t *)record_data);
}

void put_record(int n, RECORD *record_data)
{
  write_eeprom(EEPROM_1_ADDR_WR, RECORD_LENGTH*n, RECORD_LENGTH, (uint8_t *)record_data);
}

//------------------------------------------------------------------------------
//
// Find records matching the key
// return index of record found or NO_RECORD if none found.
// Optionally start at 'start'

int find_record(char *key, RECORD *recout, int start)
{
  RECORD r;
  
  if( start == NO_RECORD )
    {
      start = 0;
    }

  for(int i=start; i<NUM_RECORDS; i++)
    {
      get_record(i,&r);

      if( strcmp(key, r.key)==0 )
	{
	  *recout = r;
	  return(i);
	}
    }

  return(NO_RECORD);
}

int is_empty(uint8_t flag)
{
  return(flag != '*');
}

////////////////////////////////////////////////////////////////////////////////

// Find the first free slot, using the flag field
// '*' full
// Anything else empty

int find_free_record(void)
{
  RECORD r;
  
  for(int i=0; i<NUM_RECORDS; i++)
    {
      get_record(i, &r);

      if( is_empty(r.flag))
	{
	  printf("\nFound empty slot at %d", i);
	  return(i);
	}
    }

  printf("\nNo empty slot");
  return(NO_RECORD);
}


////////////////////////////////////////////////////////////////////////////////

int display_record(RECORD *r)
{
  display_clear();
  printxy_hex(0, 0, r->flag);
  printxy_str(5, 0, &(r->key[0]));
  printxy_str(0, 1, &(r->data[0]));

  printf("\nRecord");
  printf("\nFlag:%c",   r->flag);
  printf("\nKey:'%s'",  &(r->key[0]));
  printf("\nData:'%s'", &(r->data[0]));
  
  while(1)
    {
      // Keep the display updated
      menu_loop_tasks();
      
      // We are on core 1 so a loop will cause 
      //      dump_lcd();
      
      if( gotkey )
	{
	  gotkey = 0;
	  
	  return(keychar);
	}
      
    }
}

////////////////////////////////////////////////////////////////////////////////

void menu_format(void)
{
  RECORD r;
  char line[32];
  
  display_clear();
  
  for(int i=0; i<NUM_RECORDS; i++)
    {
      sprintf(line, "REC %c", i);
      printxy_str(0, 0, line);
      
      r.flag = '-';
      sprintf(&(r.key[0]), "K%c", 'A'+(i%10));
      sprintf(&(r.data[0]), "Data for rec %02X", i);
      put_record(i, &r);
      display_record(&r);
    }
}

////////////////////////////////////////////////////////////////////////////////
//

// Run through all records

void menu_all(void)
{
  RECORD r;
  int key = 0;
  
  for(int i=0; i<NUM_RECORDS; i++)
    {
      get_record(i, &r);
      key = display_record(&r);

      if( key == KEY_ON )
	{
	  return;
	}
    }
}

//------------------------------------------------------------------------------
//
// Memories
//

void menu_eeprom_save_mems(void)
{
    write_eeprom(EEPROM_1_ADDR_RD,  EEPROM_OFF_SAVED_MEMS_0_START, EEPROM_LEN_COPY_MEMS, &(ramdata[RTT_NUMB]));
}

void menu_eeprom_load_mems(void)
{
  // Read memories from RAM image in EEPROM 0
  read_eeprom(EEPROM_1_ADDR_RD,  EEPROM_OFF_SAVED_MEMS_0_START, EEPROM_LEN_COPY_MEMS, &(ramdata[RTT_NUMB]));
  
}

// Extract the memories from the RAM copy in EEPROM.
// Copy it to memory slot 0 in the other EEPROM

void menu_eeprom_extract_mems(void)
{
  BYTE mems[EEPROM_LEN_COPY_MEMS];
  
  // Read memories from RAM image in EEPROM 0
  read_eeprom(EEPROM_0_ADDR_RD,  EEPROM_OFF_COPY_MEMS_START, EEPROM_LEN_COPY_MEMS, mems);

  // Write it to saved memory slot 0
  write_eeprom(EEPROM_1_ADDR_WR,  EEPROM_OFF_SAVED_MEMS_0_START, EEPROM_LEN_COPY_MEMS, mems);
}

////////////////////////////////////////////////////////////////////////////////

#define MAX_INPUT_STR   32

void menu_find(void)
{
  char find_str[MAX_INPUT_STR];
  int searching = 0;
  int cur_rec = 0;
  int ed$pos = 2;
  
  find_str[0] ='\0';
  display_clear();
  printxy_str(0, 0, "Find:");
  print_str(find_str);

  while(1)
    {
      // Keep the display updated
      menu_loop_tasks();
      
      if( gotkey || searching )
	{
	  gotkey = 0;
	  
	  // Exit on ON key, exiting demonstrates it is working...
	  if( keychar == KEY_ON )
	    {
	      if( strlen(find_str) == 0)
		{
		  // Refresh menu on exit
		  menu_init = 1;
		  return;
		}
	      else
		{
		  find_str[0] ='\0';
		  
		  display_clear();
		  printxy_str(0, 0, "Find:");
		  printxy_str(5, 0, find_str);
		  continue;
		}
	    }

	  if ( (keychar == 'x') || searching )
	    {
	      int recnum;
	      RECORD r;

	      printf("\nStart search");
	      
	      if( !searching )
		{
		  printf("\nNot already in search");
		  cur_rec = 0;
		  searching = 1;
		  
		}
	      
	      // Find the record
	      recnum = find_record(find_str, &r, cur_rec);
	      cur_rec = recnum+1;
	      
	      if( recnum != -1 )
		{
		  printf("\nFound");
		  
		  display_clear();
		  printxy_str(0, 0, "Find:");
		  printxy_str(5, 0, &(r.key[0]));
		  printxy_str(0, 1, &(r.data[0]));

		  int done = 0;
		  
		  while(!done)
		    {
		      // Keep the display updated
		      menu_loop_tasks();
		      
		      if( gotkey )
			{
			  gotkey = 0;
			  
			  switch(keychar)
			    {
			    case 'x':
			      done = 1;
			      break;

			    case KEY_ON:
			      done = 1;
			      find_str[0] ='\0';
			      
			      display_clear();
			      printxy_str(0, 0, "Find:");
			      printxy_str(5, 0, find_str);
			      continue;
			      
			      break;
			    }
			}
		    }
		  continue;
		}
	      else
		{
		  printf("\nNo more records");
		  
		  // No more records
		  searching = 0;

		  find_str[0] ='\0';
		  
		  display_clear();
		  printxy_str(0, 0, "Find:");
		  printxy_str(5, 0, find_str);
		  continue;
		}
	    }
	  
	  if ( strlen(find_str) < (MAX_INPUT_STR-2) )
	    {
	      char keystr[2] = " ";
	      keystr[0] = keychar;
	      strcat(find_str, keystr);
	      
	      display_clear();
	      printxy_str(0, 0, "Find:");
	      printxy_str(5, 0, find_str);
	    }
	  
	  gotkey = 0;
	  
	}
    }
}


#define SS_SAVE_INIT   1
#define SS_SAVE_ENTER  2
#define SS_SAVE_EXIT   3

void menu_save(void)
{
  char save_str[MAX_INPUT_STR];
  int searching = 0;
  int cur_rec = 0;
  int substate = SS_SAVE_INIT;
  int recnum;
  RECORD r;
    
  save_str[0] ='\0';
  display_clear();
  printxy_str(0, 0, "Save:");
  print_str(save_str);

  int done = 0;
  
  while(!done)
    {
      // Keep the display updated
      menu_loop_tasks();
      
      if( gotkey )
	{
	  gotkey = 0;

	  switch(substate)
	    {
	    case SS_SAVE_INIT:
	      switch(keychar)
		{
		case KEY_ON:
		  // Exit on ON key, exiting demonstrates it is working...
		  if( strlen(save_str) == 0)
		    {
		      // Refresh menu on exit
		      menu_init = 1;
		      return;
		    }
		  else
		    {
		      save_str[0] ='\0';
		      
		      display_clear();
		      printxy_str(0, 0, "Save:");
		      printxy_str(5, 0, save_str);
		      continue;
		    }
				  
		  break;

		case 'x':
		  printf("\nFind empty search");
		  
		  // Find the record slot to save in to
		  recnum = find_free_record();
		  
		  if( recnum != -1 )
		    {
		      // Store data
		      r.flag = '*';
		      
		      strcpy(&(r.key[0]), save_str);
		      put_record(recnum, &r);
		      done = 1;
		    }
		  else
		    {
		      // No room
		      done = 1;
		    }
		  break;

		default:
		  if ( strlen(save_str) < (MAX_INPUT_STR-2) )
		    {
		      char keystr[2] = " ";
		      keystr[0] = keychar;
		      strcat(save_str, keystr);
		      
		      display_clear();
		      printxy_str(0, 0, "Save:");
		      printxy_str(5, 0, save_str);
		    }
		  break;
		  
		}
	    }
	  
	  gotkey = 0;
	}
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// Oled

void menu_oled_test(void)
{

#if 0
  oledmain();
#endif

#if 1
  pixels_clear();
  //return;
  
  for(int y=0; y<31; y+=10)
    {
      for(int x=0; x<121; x++)
	{
	  plot_point(x,   y, 1);
	}
    }
  
  for(int x=0; x<121; x+=10)
    {
      for(int y=0; y<31; y++)
	{
	  plot_point(x, y, 1);
	}
    }

  sleep_ms(2000);
  
#endif

  int bx = 5000;
  int by = 5000;

  int x = 0;
  int y = 0;
  int dxa = 314;
  int dya = 577;
  int dxb = 357;
  int dyb = 591;

  pixels_clear();
  
  while(1)
    {
#if 1
      
      plot_point(x+0, y+0, 0);
      plot_point(x+1, y-1, 0);
      plot_point(x+1, y+1, 0);
      plot_point(x-1, y-1, 0);
      plot_point(x-1, y+1, 0);

      bx=bx+dxa;
      by=by+dya;
      printf("\n%d %d", x, y);
      
      x = bx / 100;
      y = by / 100;
      
      plot_point(x+0, y+0, 1);
      plot_point(x+1, y-1, 1);
      plot_point(x+1, y+1, 1);
      plot_point(x-1, y-1, 1);
      plot_point(x-1, y+1, 1);

      if( bx > 12500 )
	{
	  dxa= -dxa;
	  dxa = (dxa * 9989)/10000;
	  bx = 12500;
	}

      if( by > 3200 )
	{
	  dya = -dya;
	  dya = (dya * 9998)/10000;
	  by = 3200;
	}

      if( bx < 300 )
	{
	  dxa= -dxa;
	  dxa = (dxa * 9990)/10000;
	  bx = 300;
	}

      if( by <300 )
	{
	  dya = -dya;
	  dya = (dya * 9998)/10000;
	  by = 300;
	}
      
#endif
      // Keep the display updated
      menu_loop_tasks();
      
      if( gotkey )
	{
	  gotkey = 0;
	  
	  // Exit on ON key, exiting demonstrates it is working...
	  if( keychar == KEY_ON )
	    {
	      break;
	    }
	  
	  if ( keychar == 'r' )
	    {
	      dxa+=100;
	    }

	  if ( keychar == 'l' )
	    {
	      dxa-=100;
	    }

	  if ( keychar == 'u' )
	    {
	      dya += 100;
	    }


	  if ( keychar == 'd' )
	    {
	      dxb -= 100;
	    }
	  
	}
     }
}

////////////////////////////////////////////////////////////////////////////////
//
// RTC
//
//

void menu_rtc_varset(void)
{
  rtc_set_variables = 1;
}

void menu_rtc_regset(void)
{
  rtc_set_registers = 1;
}

////////////////////////////////////////////////////////////////////////////////
//
//
// Start the menu running
//
//

void menu_enter(void)
{
  // Save the bit pattern the serial latch is generating so keyboard
  // scan doesn't affect the organiser code
  //  saved_latchout1_shadow = latchout1_shadow;
  
  // Save the display for the menu exit
  //display_save();

  // Clear the display
  //display_clear();

  // Draw the menu
  //  printxy_str(0,0, "Menu");

  menu_done = 0;

  goto_menu(&menu_top);
}

void menu_loop(void)
{
  check_keys();
  menu_process();
}

void menu_leave(void)
{
  display_restore();
  
  // Restore latch data
  //write_595(PIN_LATCHOUT1, saved_latchout1_shadow, 8);
  //latchout1_shadow = saved_latchout1_shadow;
  
}

//------------------------------------------------------------------------------
//
// Menu detection
//
// How many presses of the SHIFT key have been seen
// Let the organiser scan the keyboard and check the flag in memory
//
// This code uses the Organiser to scan the keyboard, it just looks in
// RAM of the emulated 6303 to see if the organiser has seens a SHIFT key
// or not. This does mean that if the emulated code isn't looking at the keyboard
// then the meta menu can't be entered.
// If we didn't do this then we'd have to stop the emulation for a while while
// we scanned the keyboard and that impacts emulation speed.
//
// This does mean that if the processor TRAPs then it doesn't scan the keyboard and
// we can't get to th emenu. If dump/restore is on then when the organiser restores
// it goes back to the TRAP and you are stuck.
//

int shift_edge_counter = 0;
int last_shift[NUM_LAST] = { 2, 2, 2, 2, 2, };
int shift = 0;
int shift_counter = 0;

void check_menu_launch(void)
{
  shift_counter++;
  
  if( (shift_counter % META_MENU_SCAN_COUNT) == 0 )
    {
      if( ramdata[KBB_STAT] & 0x80 )
	{
	  // Shift pressed
	  shift = 1;
	  //	  printxy(0,2, '*');
	}
      else
	{
	  // Shift not pressed
	  shift = 0;
	  //	  printxy(0,2, ' ');
	}
      
      for(int i = 1; i<NUM_LAST; i++)
	{
	  last_shift[i-1] = last_shift[i];
	}
      
      last_shift[NUM_LAST-1] = shift;
       
      if( (last_shift[NUM_LAST-1] == 1) &&
	  (last_shift[NUM_LAST-2] == 1) &&
	  (last_shift[NUM_LAST-3] == 0) &&
	  (last_shift[NUM_LAST-4] == 0)
	  )
	{
	  // Another edge
	  shift_edge_counter++;
	  //printxy_hex(0,1, shift_edge_counter);
	  
	  if( shift_edge_counter == 4 )
	    {
#if 0	      
	      menu_enter();
#else
	    // get core 1 to run the menu
	    core1_in_menu = 1;

	    // Wait for the core to exit the menu before we continue
	    while(core1_in_menu)
	      {
	      }
#endif
	      shift_edge_counter = 0;
	    }
	}
    }
      
  // Any other key pressed?
  if( ramdata[KBB_NKEYS] )
    {
      // Another key pressed, reset shift_edge_counter
      shift_edge_counter = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
//


MENU menu_top =
  {
   &menu_top,
   "Meta",
   init_menu_top,   
   {
    {'K', "Keytest",    menu_scan_test},
    {'O', "Off",        menu_instant_off},
    {'E', "Eeprom",     menu_goto_eeprom},
    {'R', "RTC",        menu_goto_rtc},
    {'D', "DispTest",   menu_oled_test},
    {'F', "Find",       menu_find},
    {'S', "Save",       menu_save},
    {'A', "All",        menu_all},
    {'M', "forMat",     menu_format},
    {'&', "",           menu_null},
   }
  };

MENU menu_eeprom =
  {
   &menu_top,
   "EEPROM",
   init_menu_eeprom,   
   {
    {KEY_ON, "",           menu_back},
    {'I', "Invalidate", menu_eeprom_invalidate},
    {'M', "Mem",        menu_goto_mems},
    {'&', "",           menu_null},
   }
  };

MENU menu_rtc =
  {
   &menu_top,
   "RTC",
   init_menu_rtc,   
   {
    {KEY_ON, "",           menu_back},
    {'V', "VarSet",     menu_rtc_varset},
    {'R', "RegSet",     menu_rtc_regset},
    {'&', "",           menu_null},
   }
  };

MENU menu_mems =
  {
   &menu_eeprom,
   "Memories",
   init_menu_mems,   
   {
    {KEY_ON, "",           menu_back},
    {'S', "Save",       menu_eeprom_save_mems},
    {'L', "Load",       menu_eeprom_load_mems},
    {'E', "Extract",    menu_eeprom_extract_mems},
    {'&', "",           menu_null},
   }
  };
