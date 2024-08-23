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
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate_all.h"

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

  
  if( kb_test() != KEY_NONE )
    {
      KEYCODE k= kb_getk();
      
      int e = 0;
     
      while( active_menu->item[e].key != '&' )
	{
	  if( k == active_menu->item[e].key )
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

void init_menu_test_os(void)
{
}

void init_menu_buzzer(void)
{
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

void menu_goto_test_os(void)
{
  goto_menu(&menu_test_os);
}

void menu_goto_buzzer(void)
{
  goto_menu(&menu_buzzer);
}

void menu_goto_mems(void)
{
  goto_menu(&menu_mems);
}

void menu_goto_rtc(void)
{
  goto_menu(&menu_rtc);
}


void menu_epos_test(void)
{
  char line[20];

  i_printxy_str(0, 0, "");
  flowprint("default");
	    
  ed_epos("default", 30, 0, 0);
  
  // Refresh menu on exit
  menu_init = 1;
}

//------------------------------------------------------------------------------


void init_scan_test(void)
{
  display_clear();
  printxy_str(0,0, "Key test    ");
}

void menu_scan_test(void)
{
  char line[20];
  
  init_scan_test();

   while(1)
    {
      // Keep the display updated
      menu_loop_tasks();

      if( kb_test() != KEY_NONE )
	{
	  KEYCODE k = kb_getk();

	  sprintf(line, "'%c' %02X %03d  ", k, k, k);
	  
	  i_printxy_str(0, 1, line);
	  
	  // Exit on ON key, exiting demonstrates it is working...
	  if( k == KEY_ON )
	    {
	      break;
	    }
	}
    }

  // Refresh menu on exit
  menu_init = 1;
}

void menu_cursor_test(void)
{
  char line[20];
  int done = 0;

  display_clear();
  i_printxy_str(0, 0, "Cursor keys to move");
  i_printxy_str(0, 1, "B:Toggle blink");

  cursor_on = 1;
  
   while(!done)
    {
      // Keep the display updated
      menu_loop_tasks();

      if( kb_test() != KEY_NONE )
	{
	  KEYCODE k = kb_getk();
	  
	  switch(k)
	    {
	    case KEY_ON:
	      cursor_on = 0;
	      done = 1;
	      break;
	      
	    default:
	      handle_cursor_key(k);
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
      
      if( kb_test() != KEY_NONE )
	{
	  KEYCODE k = kb_getk();
	  
	  return(k);
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
      
      if( (kb_test() != KEY_NONE) || searching )
	{
	  KEYCODE k = kb_getk();
	  
	  // Exit on ON key, exiting demonstrates it is working...
	  if( k == KEY_ON )
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

	  if ( (k == KEY_EXE) || searching )
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
		      
		      if( kb_test() != KEY_NONE )
			{
			  KEYCODE k = kb_getk();
			  
			  switch(k)
			    {
			    case KEY_EXE:
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
	      keystr[0] = k;
	      strcat(find_str, keystr);
	      
	      display_clear();
	      printxy_str(0, 0, "Find:");
	      printxy_str(5, 0, find_str);
	    }
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
      
      if( kb_test() != KEY_NONE )
	{
	  KEYCODE k = kb_getk();

	  switch(substate)
	    {
	    case SS_SAVE_INIT:
	      switch(k)
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

		case KEY_EXE:
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
		      keystr[0] = k;
		      strcat(save_str, keystr);
		      
		      display_clear();
		      printxy_str(0, 0, "Save:");
		      printxy_str(5, 0, save_str);
		    }
		  break;
		  
		}
	    }
	}
    }
}

void menu_fl_find(void)
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
      
      if( (kb_test() != KEY_NONE) || searching )
	{
	  KEYCODE k = kb_getk();
	  
	  // Exit on ON key, exiting demonstrates it is working...
	  if( k == KEY_ON )
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

	  if ( (k == KEY_EXE) || searching )
	    {
	      int reclen;
	      char r[256];

	      printf("\nStart search");
	      
	      if( !searching )
		{
		  printf("\nNot already in search");
		  flw_crec = 1;
		  searching = 1;
		  
		}
	      
	      // Find the record
	      fl_rect(0x90);
	      if( fl_find(find_str, r, &reclen) )
		{
		  fl_next();
		  
		  printf("\nFound");
		  
		  display_clear();
		  printxy_str(0, 0, "Find:");
		  printxy_str(5, 0, r);

		  int done = 0;
		  
		  while(!done)
		    {
		      // Keep the display updated
		      menu_loop_tasks();
		      
		      if( kb_test() != KEY_NONE )
			{
			  KEYCODE k = kb_getk();
			  
			  switch(k)
			    {
			    case KEY_EXE:
			      done = 1;
			      break;

			    case KEY_DEL:
			      fl_back();
			      fl_eras();
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

	  if( k == KEY_DEL )
	    {
	      find_str[strlen(find_str)-1] = '\0';
	      printxy_str(5, 0, find_str);
	      printxy_str(5+strlen(find_str), 0, "                 ");

	    }
	  else
	    {
	      if ( strlen(find_str) < (MAX_INPUT_STR-2) )
		{
		  char keystr[2] = " ";
		  keystr[0] = k;
		  strcat(find_str, keystr);
		  
		  display_clear();
		  printxy_str(0, 0, "Find:");
		  printxy_str(5, 0, find_str);
		}
	    }
	}
    }
}

void menu_fl_save(void)
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
      
      if( kb_test() != KEY_NONE )
	{
	  KEYCODE k = kb_getk();

	  switch(substate)
	    {
	    case SS_SAVE_INIT:
	      switch(k)
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

		case KEY_EXE:

		  // Save data in file 0x90 (MAIN)
		  fl_rect(0x90);
		  fl_writ(save_str, strlen(save_str));
		  break;

		case KEY_DEL:
		  save_str[strlen(save_str)-1] = '\0';
		  printxy_str(5+strlen(save_str), 0, "                 ");
		  break;
		  
		default:
		  if ( strlen(save_str) < (MAX_INPUT_STR-2) )
		    {
		      char keystr[2] = " ";
		      keystr[0] = k;
		      strcat(save_str, keystr);
		      
		      display_clear();
		      printxy_str(0, 0, "Save:");
		      printxy_str(5, 0, save_str);
		    }
		  break;
		  
		}
	    }
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
//
//
//

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
      //printf("\n%d %d", x, y);
      
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
      if( kb_test() != KEY_NONE )
	{
	  KEYCODE k = kb_getk();
	  
	  // Exit on ON key, exiting demonstrates it is working...
	  if( k == KEY_ON )
	    {
	      break;
	    }
	  
	  if ( k == KEY_RIGHT )
	    {
	      dxa+=100;
	    }

	  if ( k == KEY_LEFT )
	    {
	      dxa-=100;
	    }

	  if ( k == KEY_UP )
	    {
	      dya += 100;
	    }


	  if ( k == KEY_DOWN )
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
////////////////////////////////////////////////////////////////////////////////

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

#define DPVIEW_N 3

char *dpview_data =
  
   "Long test string for dpview that requires scrolling\t"
   "Short string\t"
   "Str";

void menu_dp_view(void)
{
  ed_view(dpview_data);
}

void menu_buz1(void)
{
  printf("\nBuz1");
  for(int i=0; i<50; i++)
    {
      TURN_BUZZER_ON;
      sleep_ms(1);

      TURN_BUZZER_OFF;
      sleep_ms(1);
    }
  printf("\nBuz1 done");
}

////////////////////////////////////////////////////////////////////////////////
//

void menu_bubble(void)
{
  int var_idx = 0;

#define N_IDX 0
#define T_IDX 1
#define H_IDX 2
  
  double tau = 3.1415*2;
  double r   = tau/2.350;
  double h = 0.7;
  
  int    n   = 5;
  int    sz  = 200;
  double sw  = 128.0/2.0;
  double sh  = 32.0/2.0;
  double t   = 20.0;
  double t2   = 1.0;
  
  double u, v, y=0.0, x = 0.0;
  double a, b;
  int ax, by;
  int c = 0;
  int mi = 100;
  int mj = 5;
  double ii, jj;
  double tinc = 0.025;

  printf("\nBubble Universe Approximation");
  printf("\n");
  printf("\nC: Toggle screen clear each loop");
  printf("\n");
  printf("\nPress + or - after one of these keys to adjust the following:");
  printf("\nN: Controls number of dots-ish");
  printf("\nT: Controls time -ish");
  printf("\nH: Controls density of dots-ish");
  
  pixels_clear();
  
  while(1)
    {
      if( c )
	{
	  pixels_clear();
	}
      
      menu_loop_tasks();
      
      for (int i=0; i<mi; i++)
	{
	  
	  double ang1_start = i+t;
	  double ang2_start = r*i+t;
	  double v=0;
	  double u=0;
	  
	  for(int j=0; j<n; j++)
	    {
	      double ang1 = ang1_start+v;
	      double ang2 = ang2_start+u;
	      u = sin(ang1)+sin(ang2);
	      v = cos(ang1)+cos(ang2);

	      a = u/2.0*sw+sw;
	      b = v/2.0*16.0+16.0;
	      ax = (int)a;
	      by = (int)b;
	    
	      plot_point(ax, by, (i*j)>(mi*mj)*h?1:0);
	    	      
	    }
	}
      
      t += tinc;
      
      if( kb_test() != KEY_NONE )
	{
	  KEYCODE k = kb_getk();
	
	  // Exit on ON key, exiting demonstrates it is working...
	  if( k == KEY_ON )
	    {
	      break;
	    }
	  
	  if ( k == KEY_RIGHT )
	    {
	      sw *= 2.0;
	    }

	  if ( k == KEY_LEFT )
	    {
	      sw /= 2.0;
	    }

	  if ( k == KEY_UP )
	    {
	      v += 3.1;
	    }

	  if ( k == 'N' )
	    {
	      var_idx = N_IDX;
	      printf("\nN adjust");
	    }

	  if ( k == 'H' )
	    {
	      var_idx = H_IDX;
	      printf("\nH adjust");
	    }

	  if ( k == 'T' )
	    {
	      var_idx = T_IDX;
	      printf("\nT adjust");
	    }

	  if ( k == '+' )
	    {
	      switch(var_idx)
		{
		case N_IDX:
		  n++;
		  printf("\nN:%d", n);
		  break;
		  
		case T_IDX:
		  tinc+=0.005;
		  printf("\nT:%g", t);
		  break;

		case H_IDX:
		  h += 0.02;
		  printf("\nH:%g", h);
		  break;
		}
	    }

	  if ( k == '-' )
	    {
	      switch(var_idx)
		{
		case N_IDX:
		  n--;
		  printf("\nN:%d", n);
		  break;
		  
		case T_IDX:
		  tinc -= 0.005;
		  printf("\nT:%g", t);
		  break;

		case H_IDX:
		  h -= 0.02;
		  printf("\nH:%g", h);
		  break;
		}
	    }
	  
	  if ( k == ' ' )
	    {
	      c = !c;;
	    }

	
	  if ( k == KEY_DOWN )
	    {
	      v -= 3.2;
	    }
	}
    }
}



////////////////////////////////////////////////////////////////////////////////
	   

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
    {'A', "All",        menu_all},
    {'M', "forMat",     menu_format},
    {'B', "Bubble",     menu_bubble},
    {'T', "Test",       menu_goto_test_os},

    {'F', "Find",       menu_fl_find},
    {'S', "Save",       menu_fl_save},

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
    {'F', "Find",       menu_find},
    {'S', "Save",       menu_save},
    {'&', "",           menu_null},
   }
  };

MENU menu_test_os =
  {
   &menu_top,
   "Test OS",
   init_menu_test_os,   
   {
    {KEY_ON, "",        menu_back},
    {'D', "DispTest",   menu_oled_test},
    {'V', "dpView",     menu_dp_view},
    {'B', "Buzz",       menu_goto_buzzer},
    {'C', "Cursor",     menu_cursor_test},
    {'E', "Epos",       menu_epos_test},
    {'&', "",           menu_null},
   }
  };

MENU menu_buzzer =
  {
   &menu_top,
   "Buzz",
   init_menu_buzzer,   
   {
    {KEY_ON, "",        menu_back},
    {'B', "Buzz1",      menu_buz1},
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
