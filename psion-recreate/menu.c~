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

////////////////////////////////////////////////////////////////////////////////
//
// Meta menu
//
//

MENU  *active_menu = &menu_top;

volatile int menu_done = 0;
volatile int menu_init = 0;

MENU menu_mems;

////////////////////////////////////////////////////////////////////////////////
//
// Scan the keyboard for keys

int scan_drive = 0x01;
int scan_state = 0;
int kb_sense = 0;
int saved_latchout1_shadow = 0;
int keycode = 0;
char keychar = 0;      // What the key code is
int gotkey = 0;       // We have a key


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

void scan_keys(void)
{
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
      break;

    case SCAN_STATE_READ:
      // Read port5
      //write_display_extra(2, '5');
      kb_sense = (read_165(PIN_LATCHIN) & 0xFC) >> 2;

      if( kb_sense & 0x20 )
	{
	  //write_display_extra(2, 'o');
	  keychar = 'o';
	  gotkey = 1;
	}
      else
	{
	  if( (kb_sense & 0x3F) != 0 )
	    {
	      // Build keycode
	      printxy_hex(5, 2, keycode);
	      printxy_hex(1, 2, kb_sense);
	      
	      keycode |= kb_sense << 8;
	      
	      // Find key from code
	      for(int i=0; i<NUM_KEYCODES; i++)
		{
		  if( menukeys[i].code == keycode )
		    {
		      keychar = menukeys[i].ch;
		      write_display_extra(2, 'k');
		      gotkey = 1;
		      break;
		    }
		}
	    }
	}
      break;

    default:
      break;
    }
  
  scan_state = (scan_state + 1) % NUM_SCAN_STATES;
}



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
      printxy_str(0, 0, active_menu->name);
      print_nl();
      
      int e = 0;
      while( active_menu->item[e].key != '&' )
	{
	  print_str(active_menu->item[e].item_text);
	  print_str(" ");
	  e++;
	}

      (*active_menu->init_fn)();
    }
  
  if( gotkey )
    {
      int e = 0;

      gotkey = 0;
      
      while( active_menu->item[e].key != '&' )
	{
	  if( keychar == active_menu->item[e].key )
	    {
	      // Call the function
	      (*active_menu->item[e].do_fn)();
	      break;
	    }
	  e++;
	}
    }
}

////////////////////////////////////////////////////////////////////////////////

void init_menu_top(void)
{
  //display_clear();
  //  printxy_str(0, 0, "Meta");
}

void init_menu_eeprom(void)
{
  //display_clear();
  //printxy_str(0, 0, "EEPROM");
}

void init_menu_rtc(void)
{
  //display_clear();
  //printxy_str(0, 0, "EEPROM");
}

void init_menu_mems(void)
{
  //display_clear();
  //print_home();
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

      // We are on core 1 so a loop will cause 
      //      dump_lcd();
      
      if( gotkey )
	{
	  printxy(2, 1, '=');
	  printxy(3, 1, keychar);
	  gotkey = 0;
	  
	  // Exit on ON key, exiting demonstrates it is working...
	  if( keychar == 'o' )
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
  if( do_menu_init() )
    {
      display_clear();
      printxy_str(0, 0, "Instant Off");
    }

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
  saved_latchout1_shadow = latchout1_shadow;
  
  // Save the display for the menu exit
  display_save();

  // Clear the display
  display_clear();

  // Draw the menu
  //  printxy_str(0,0, "Menu");

  menu_done = 0;

  goto_menu(&menu_top);
}

void menu_loop(void)
{
  scan_keys();
  menu_process();
}

void menu_leave(void)
{
  display_restore();
  
  // Restore latch data
  write_595(PIN_LATCHOUT1, saved_latchout1_shadow, 8);
  latchout1_shadow = saved_latchout1_shadow;
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
    {'o', "",           menu_exit},
    {'K', "Keytest",    menu_scan_test},
    {'I', "Instantoff", menu_instant_off},
    {'E', "Eeprom",     menu_goto_eeprom},
    {'R', "RTC",        menu_goto_rtc},
    {'&', "",           menu_null},
   }
  };

MENU menu_eeprom =
  {
   &menu_top,
   "EEPROM",
   init_menu_eeprom,   
   {
    {'o', "",           menu_back},
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
    {'o', "",           menu_back},
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
    {'o', "",           menu_back},
    {'S', "Save",       menu_eeprom_save_mems},
    {'L', "Load",       menu_eeprom_load_mems},
    {'E', "Extract",    menu_eeprom_extract_mems},
    {'&', "",           menu_null},
   }
  };
