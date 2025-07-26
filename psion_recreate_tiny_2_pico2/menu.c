////////////////////////////////////////////////////////////////////////////////
//
// The Menu
//
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

#define CALC_FSM_DB 1

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

//------------------------------------------------------------------------------
// These are the tasks the menu functions need to perform in order to
// keep the display, wireless and so on running.

u_int64_t now[NUM_STATS];
int menu_loop_count = 0;


void menu_loop_tasks(void)
{
  if( !((menu_loop_count++) % 10) == 0 )
    {
      return;
    }
  
  tud_task();

#if CORE0_SCAN
  matrix_scan();
#endif
  
  cursor_task();
  serial_loop();
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
      dp_cls();
      
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
}

void init_menu_format(void)
{
}

void init_menu_eeprom(void)
{
}

void init_menu_test_os(void)
{
}

void init_menu_prog(void)
{
}

void init_menu_calc(void)
{
}

void init_menu_buzzer(void)
{
}

void init_menu_rtc(void)
{
}

void init_menu_mems(void)
{
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

void menu_goto_format(void)
{
  goto_menu(&menu_format);
}

void menu_goto_test_os(void)
{
  goto_menu(&menu_test_os);
}

void menu_goto_prog(void)
{
  goto_menu(&menu_prog);
}

void menu_goto_calc(void)
{
  goto_menu(&menu_calc);
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

////////////////////////////////////////////////////////////////////////////////

void menu_test_file(void)
{
  size_t argc = 0;
  
  // Open and read the file 'TEST.TXT' in the root directory of the SD card
  run_mount(0, argv_null);
  run_cd(argc, argv);
  argc = 1;
  argv[0]  = "TEST.TXT";
  run_cat(argc, argv);
  run_unmount(0, argv_null);
}

char e_buffer[64] = "";

void menu_prog_translate(void)
{
  dp_stat(0, 0, DP_STAT_CURSOR_OFF, 0);

  dp_prnt("Default:");

  ed_epos(e_buffer, 30, 0, 0);

  // Refresh menu on exit
  menu_init = 1;

  // Mount the SD card and translate the OPL file
  run_mount(0, argv_null);

  // Add suffix, as it has to be an ob3 file
  strcat(e_buffer, ".opl");
  nopl_trans(e_buffer);
  
  run_unmount(0, argv_null);

}

void menu_prog_run(void)
{
  dp_stat(0, 0, DP_STAT_CURSOR_OFF, 0);

  dp_prnt("Run:");

  ed_epos(e_buffer, 30, 0, 0);

  // Refresh menu on exit
  menu_init = 1;

  // Mount the SD card and translate the OPL file
  run_mount(0, argv_null);

  // Add suffix, as it has to be an ob3 file
  strcat(e_buffer, ".ob3");
  nopl_exec(e_buffer);
  
  run_unmount(0, argv_null);


}


void menu_epos_test(void)
{
  char line[20];
  char e_buffer[64];

  dp_stat(0, 0, DP_STAT_CURSOR_OFF, 0);

  dp_prnt("Default:");

  strcpy(e_buffer, "DATA");
  ed_epos(e_buffer, 30, 0, 0);

  // Refresh menu on exit
  menu_init = 1;
}

void menu_flowtext_test(void)
{
  char line[20];
  char e_buffer[64];
  char test_str[64];

  sprintf(test_str, "Line one%cLine 2%cEnd", CHRCODE_TAB, CHRCODE_TAB);

  dp_stat(0, 0, DP_STAT_CURSOR_OFF, 0);

  dp_prnt(test_str);
  dp_clr_eos();
  
  kb_getk();
  
  // Refresh menu on exit
  menu_init = 1;
}

//------------------------------------------------------------------------------


void init_scan_test(void)
{
  dp_cls();
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

	  // Send to host as keypress
	  queue_hid_key(translate_to_hid(k));
	  
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

  dp_cls();
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
void handle_power_off(void)
{
  printf("\nPower off...");
  
  gpio_put(PIN_VBAT_SW_ON, 0);
  
#if ALLOW_POWER_OFF

  // Store time in RC
  rtc_set_registers = 1;

  // Wait for RTC to update
  sleep_ms(500);
  
  // Turn power off
  gpio_put(PIN_VBAT_SW_ON, 0);
#else

  // Sit in a loop so we don't repeatedly dump
  while(1)
    {
    }
#endif
  
}

void menu_instant_off(void)
{
#if 0
  if( do_menu_init() )
    {
      dp_cls();
      printxy_str(0, 0, "Instant Off");
    }
#endif
  
  handle_power_off();
}

//------------------------------------------------------------------------------

void menu_eeprom_invalidate(void)
{
#if 0
  // This is run after a restore so we have the correct cheksum in ram
  // we can just adjust it to get a bad csum, and also fix a value
  ramdata[EEPROM_CSUM_L] = 0x11;
  ramdata[EEPROM_CSUM_H]++;
  
  // Write the checksum to the EEPROM copy
  write_eeprom(EEPROM_0_ADDR_WR , EEPROM_CSUM_H, EEPROM_CSUM_LEN, &(ramdata[EEPROM_CSUM_H]));

#endif
  
  dp_cls();
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
  dp_cls();
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

void menu_fmat(void)
{
  dp_cls();
  dp_stat(0, 0, DP_STAT_CURSOR_OFF, 0);
  
  dp_prnt("Formatting...");
  
  pk_fmat(0);
  menu_back();
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
  #if 0
    write_eeprom(EEPROM_1_ADDR_RD,  EEPROM_OFF_SAVED_MEMS_0_START, EEPROM_LEN_COPY_MEMS, &(ramdata[RTT_NUMB]));
    #endif
    
}

void menu_eeprom_load_mems(void)
{
#if 0
  // Read memories from RAM image in EEPROM 0
  read_eeprom(EEPROM_1_ADDR_RD,  EEPROM_OFF_SAVED_MEMS_0_START, EEPROM_LEN_COPY_MEMS, &(ramdata[RTT_NUMB]));
  #endif
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
  dp_cls();
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
		      
		      dp_cls();
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
		      
		      dp_cls();
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

#if 1

typedef enum
  {
   MF_STATE_INIT = 1,
   MF_STATE_SRCH_START,
   MF_STATE_SRCH_NEXT,
  } MF_STATE;

MF_STATE mf_state;

void menu_fl_find(void)
{
  char      find_str[MAX_INPUT_STR];
  int       searching = 0;
  int       cur_rec   = 0;
  int       ed$pos    = 2;
  int       done      = 0;
  KEYCODE   k;
  int       reclen;
  char      r[256];
  int view_line = 0;
  
  find_str[0] ='\0';
 
  mf_state = MF_STATE_INIT;
  
  while(!done)
    {
      switch(mf_state)
	{
	case MF_STATE_INIT:
	  dp_cls();
	  printxy_str(0, 0, "Find:");
	  //printxy_str(5,0, find_str);
	  
	  k = ed_epos(find_str, 64, 0, 0);
	  
	  switch(k)
	    {
	    case KEY_ON:
	      if( strlen(find_str) == 0)
		{
		  // Refresh menu on exit
		  menu_init = 1;
		  done = 1;
		}
	      else
		{
		  find_str[0] ='\0';
		  
		  dp_cls();
		  printxy_str(0, 0, "Find:");
		  printxy_str(5, 0, find_str);
		}
	      break;

	    case KEY_EXE:
	      mf_state = MF_STATE_SRCH_START;
	      break;
	    }
	  break;

	case MF_STATE_SRCH_START:
	  printf("\nNot already in search");
	  flw_crec = 1;
	  searching = 1;

	  mf_state = MF_STATE_SRCH_NEXT;
	  break;

	case MF_STATE_SRCH_NEXT:
	  // Find the record
	  fl_rect(0x90);
	  if( fl_find(find_str, r, &reclen) )
	    {
	      printf("\n%s:Found flw_crec:%d", __FUNCTION__, flw_crec);
	      
	      dp_cls();
	      printxy_str(0, 0, "Find:");
	      k = ed_view(r, view_line);

	      switch(k)
		{
		case KEY_EXE:
		  fl_next();
		  break;
		  
		case KEY_DEL:
		  //fl_back();
		  fl_eras();
		  break;
		  
		case KEY_UP:
		  if( view_line > 0 )
		    {
		      view_line--;
		    }
		  break;

		case KEY_DOWN:
		  if( view_line < (DISPLAY_NUM_LINES-1) )
		    {
		      view_line++;
		    }
		  break;
		  
		case KEY_ON:
		  if( strlen(find_str) != 0 )
		    {
		      find_str[0] ='\0';
		      
		      mf_state = MF_STATE_INIT;
		    }
		  else
		    {
		      done = 1;
		    }
		  
		  break;
		  
		default:
		  //		  fl_next();
		  break;
		}
	    }
	  else
	    {
	      printf("\nNo more records");
	      
	      //find_str[0] ='\0';

	      mf_state = MF_STATE_INIT;
	      //	      mf_state = MF_STATE_WAIT_END;
	    }
	  break;
	  
	}
    }
  
    dp_stat(0, 0, DP_STAT_CURSOR_OFF, 0);
}

#else


void menu_fl_find_xxx(void)
{
  char find_str[MAX_INPUT_STR];
  int searching = 0;
  int cur_rec = 0;
  int ed$pos = 2;
  
  find_str[0] ='\0';
  dp_cls();
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
		  
		  dp_cls();
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
		  
		  dp_cls();
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
			      printf("\nErasing");
			      fl_back();
			      fl_eras();
			      break;
			      
			      
			    case KEY_ON:
			      done = 1;
			      find_str[0] ='\0';
			      
			      dp_cls();
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
		  
		  dp_cls();
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
		  
		  dp_cls();
		  printxy_str(0, 0, "Find:");
		  printxy_str(5, 0, find_str);
		}
	    }
	}
    }
}


#endif

////////////////////////////////////////////////////////////////////////////////
//
//


void menu_fl_save(void)
{
  char save_str[MAX_INPUT_STR];
  int searching = 0;
  int cur_rec = 0;
  int substate = SS_SAVE_INIT;
  int recnum;
  RECORD r;
  int done = 0;
  KEYCODE   k;
  
  save_str[0] ='\0';
  dp_cls();
  printxy_str(0, 0, "Save:");
  print_str(save_str);
  


  while(!done)
    {
      menu_loop_tasks();
      
      k = ed_epos(save_str, 64, 0, 0);
      
      switch(k)
	{
	case KEY_ON:
	  // Exit on ON key, exiting demonstrates it is working...
	  if( strlen(save_str) == 0)
	    {
	      // Refresh menu on exit
	      menu_init = 1;
	      done = 1;
	    }
	  else
	    {
	      save_str[0] ='\0';
	      
	      dp_cls();
	      printxy_str(0, 0, "Save:");
	      printxy_str(5, 0, save_str);
	    }
	  
	  break;
	  
	case KEY_EXE:
	  
	  // Save data in file 0x90 (MAIN)
	  fl_rect(0x90);
	  fl_writ(save_str, strlen(save_str));

	  save_str[0] ='\0';
	  
	  dp_cls();
	  printxy_str(0, 0, "Save:");
	  printxy_str(5, 0, save_str);
	  
	  break;
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
  int maxx = dd_get_x_size();
  int maxy = dd_get_y_size();
  int gy = 8;
  int gx = 6;

  
  int mx = (maxx / gx) * gx;
  int my = (maxy / gy) * gy;
  
  for(int y=0; y<=my; y+=gy)
    {
      for(int x=0; x<mx; x++)
	{
	  dd_plot_point(x, y, 1);
	}
    }
  
  for(int x=0; x<=mx; x+=gx)
    {
      for(int y=0; y<my; y++)
	{
	  dd_plot_point(x, y, 1);
	}
    }

  dd_update();
  
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
      menu_loop_tasks();
      dd_update();
#if 1
      
      dd_plot_point(x+0, y+0, 0);
      dd_plot_point(x+1, y-1, 0);
      dd_plot_point(x+1, y+1, 0);
      dd_plot_point(x-1, y-1, 0);
      dd_plot_point(x-1, y+1, 0);

      bx=bx+dxa;
      by=by+dya;
      //printf("\n%d %d", x, y);
      
      x = bx / 100;
      y = by / 100;
      
      dd_plot_point(x+0, y+0, 1);
      dd_plot_point(x+1, y-1, 1);
      dd_plot_point(x+1, y+1, 1);
      dd_plot_point(x-1, y-1, 1);
      dd_plot_point(x-1, y+1, 1);

      if( bx > (dd_get_x_size()-1)*100 )
	{
	  dxa= -dxa;
	  dxa = (dxa * 9989)/10000+2;
	  bx = (dd_get_x_size()-1)*100;
	}

      if( by > (dd_get_y_size()-1)*100 )
	{
	  dya = -dya;
	  dya = (dya * 9998)/10000+2;
	  by = (dd_get_y_size()-1)*100;
	}

      if( bx < 300 )
	{
	  dxa= -dxa;
	  dxa = (dxa * 9990)/10000+2;
	  bx = 300;
	}

      if( by <300 )
	{
	  dya = -dya;
	  dya = (dya * 9998)/10000+2;
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
  //dp_cls();

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
//
// Hex calculator
//

void display(int v1, int v2, int hex_ndec)
{
  char hexbuf[80];

  dp_cls();
  
  if( hex_ndec )
    {
      sprintf(hexbuf, "%08Xh", v1);
      printxy_str(0,0,hexbuf);
      sprintf(hexbuf, "%08Xh", v2);
      printxy_str(0,1,hexbuf);
    }
  else
    {
      sprintf(hexbuf, "%dd", v1);
      printxy_str(0,0,hexbuf);
      sprintf(hexbuf, "%dd", v2);
      printxy_str(0,1,hexbuf);
    }
}

void menu_hex(void)
{
  int done = 0;
  int v1=0, v2=0;
  int hex_ndec = 1;

  display(v1, v2, hex_ndec);
  
  while(!done)
    {
      
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

	    case ' ':
	      hex_ndec = !hex_ndec;
	      break;
	      
	    case 'A':
	    case 'B':
	    case 'C':
	    case 'D':
	    case 'E':
	    case 'F':
	      if( hex_ndec )
		{
		  v2 <<= 4;
		  v2 += (k-'A'+0xA);
		}
	      break;

	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
	      if( hex_ndec )
		{
		  v2 <<= 4;
		  v2 += (k-'0');
		}
	      else
		{
		  v2 *= 10;
		  v2 += (k-'0');
		}
	      break;

	    case KEY_EXE:
	      v1 = v2;
	      v2 = 0;
	      break;
	      
	    case KEY_DEL:
	      v2 = 0;
	      break;
	      
	    case '+':
	      v2 = v1 + v2;
	      v1 = v2;
	      break;

	    case '-':
	      v2 = v1 - v2;
	      v1 = v2;
	      break;

	    case '*':
	      v2 = v1 * v2;
	      v1 = v2;
	      break;

	    case '/':
	      v2 = v1 / v2;
	      v1 = v2;
	      break;

	    case '<':
	      v2 <<= 1;
	      break;

	    case '>':
	      v2 >>= 1;
	      break;
	    }

	  display(v1, v2, hex_ndec);
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
//

#define MAX_STACK          4
#define MAX_OPERATOR_LEN  10
#define MAX_NUM_LEN       15

char entry[200];
double stack[MAX_STACK];
int fsp = 0;
double number;
char str_num[MAX_NUM_LEN+1];
char str_num2[MAX_NUM_LEN+1];
char operator[MAX_OPERATOR_LEN+1];

void fac_add(void)
{
  printf("\nAdd\n");
  stack[0] = stack[0] + stack[1];

  for(int i=1; i<MAX_STACK-1; i++)
    {
      stack[i] = stack[i+1];
    }

  fsp = 0;
  
}

void fac_div(void)
{
  printf("\nDiv\n");
  stack[0] = stack[1] / stack[0];

  for(int i=1; i<MAX_STACK-1; i++)
    {
      stack[i] = stack[i+1];
    }
  
  fsp = 0;
}

void fac_mul(void)
{
  printf("\nDiv\n");
  stack[0] = stack[0] * stack[1];

  for(int i=1; i<MAX_STACK-1; i++)
    {
      stack[i] = stack[i+1];
    }
  
  fsp = 0;
}


void fac_minus(void)
{
  printf("\nDiv\n");
  stack[0] = stack[1] - stack[0];

  for(int i=1; i<MAX_STACK-1; i++)
    {
      stack[i] = stack[i+1];
    }
  
  fsp = 0;
}

void fac_sin(void)
{
  printf("\nDiv\n");
  stack[0] = sin(stack[0]);

  for(int i=1; i<MAX_STACK-1; i++)
    {
      stack[i] = stack[i+1];
    }
  
  fsp = 0;
}

void fac_cos(void)
{
  printf("\nDiv\n");
  stack[0] = cos(stack[0]);

  for(int i=1; i<MAX_STACK-1; i++)
    {
      stack[i] = stack[i+1];
    }
  
  fsp = 0;
}

void fac_tan(void)
{
  stack[0] = tan(stack[0]);

  for(int i=1; i<MAX_STACK-1; i++)
    {
      stack[i] = stack[i+1];
    }
  
  fsp = 0;
}

void fac_sqrt(void)
{
  stack[0] = sqrt(stack[0]);
}

void fac_chs(void)
{
  stack[0] = -stack[0];
}

void fac_swp(void)
{
  double t;

  t = stack[0];
  stack[0] = stack[1];
  stack[1] = t;
}

typedef void (*FCMD_ACTION)(void);

typedef struct
{
  char *name;
  FCMD_ACTION action;  
} FCMD_ENTRY;

FCMD_ENTRY fcmds[] =
  {
    {"+", fac_add},
    {"-", fac_minus},
    {"*", fac_mul},
    {"/", fac_div},
    {"SIN", fac_sin},
    {"COS", fac_cos},
    {"TAN", fac_tan},
    {"SQRT", fac_sqrt},
    {"CHS", fac_chs},
    {"SWP", fac_swp},
  };

#define NUM_FCMDS (sizeof(fcmds)/sizeof(FCMD_ENTRY))


void display_forth(char *v1)
{
  char forthbuf[80];

  dp_cls();
  
  sprintf(forthbuf, "%s", entry);
  strcat(forthbuf, "                  ");
  forthbuf[16] = '\0';
  
  printxy_str(0,0,forthbuf);
  cursor_y = 0;
  cursor_x = strlen(entry);
  
  for(int i=0; i<MAX_STACK; i++)
    {
      sprintf(forthbuf, "%g", stack[i]);
      printxy_str(0,i+1,forthbuf);
    }
}

void execute_cmd(char *cmd)
{
  for(int i=0; i<NUM_FCMDS; i++)
    {
      if( strcmp(fcmds[i].name, cmd)==0 )
        {
          (*fcmds[i].action)();
        }
    }
}

void move_stack_up(void)
{
  for(int i=MAX_STACK-1; i>0; i--)
    {
      stack[i] = stack[i-1];
    }
}

void cpa_conv_push_num(char c)
{
  sscanf(str_num, "%lf", &number);
  move_stack_up();
  stack[0] = number;
}

void cpa_push_num(char c)
{
  move_stack_up();
  stack[0] = number;
}

// Parse the expression using a state machine

void cfsm_status(void)
{
#if CALC_FSM_DB
  printf("\nNumber   : %g", number);
  printf("\nstr_num  : '%s'", str_num);
  printf("\noperator : '%s'", operator);
#endif
}

void cpa_num(char c)
{
  char frag[2] = " ";

  frag[0] = c;
  strcat(str_num, frag);
  
}

void cpa_do_op(char c)
{
  execute_cmd(operator);

  // Clear the entry
  
}

void cpa_build_op(char c)
{
  char frag[2] = " ";

  frag[0] = c;
  strcat(operator, frag);
  
}

void cpa_neg_num(char c)
{
  char frag[2] = " ";

  frag[0] = c;
  strcat(str_num, frag);
  sprintf(str_num2, "-%s", str_num);
  strcpy(str_num, str_num2);

}

void cpa_do_minus(char c)
{
  execute_cmd("-");
}

void cpa_num_dig(char c)
{
}

void cpa_null(char c)
{
}

#define EA_DB \
  if( CALC_FSM_DB )               \
    {  \
  printf("\nEA_DB: %s\n", __FUNCTION__);  \
    }

void cpae_init(char c)
{
  EA_DB;
  
  number = 0.0;
  operator[0] ='\0';
  str_num[0] = '\0';
}

void cpae_in_num(char c)
{
  EA_DB;

  printf("\nEA is %s", __FUNCTION__);
}

void cpae_in_op(char c)
{
  EA_DB;

  printf("\nEA is %s", __FUNCTION__);
}

void cpae_neg(char c)
{
  EA_DB;
  printf("\nEA is %s", __FUNCTION__);
}

void cpae_null(char c)
{
  EA_DB;
}

typedef void (*CP_ACTION)(char c);

typedef struct _CP_STATE_ENTRY
{
  char             *c;            // Character in string  _ is '\0'
  CP_ACTION        action;         // NULL if none
  struct _CP_STATE *next_state;
} CP_STATE_ENTRY;

typedef struct _CP_STATE
{
  char             *name;
  CP_ACTION         entry_action;         // Executed on entry to state
  CP_STATE_ENTRY    transition[5];
} CP_STATE;


CP_STATE cps_neg;
CP_STATE cps_in_num;
CP_STATE cps_in_op;

// Start state
CP_STATE cps_init =
  {
    "cps_init",
    cpae_init,
    {
      { "-",                             cpa_null,     &cps_neg },   // minus, could be number or operator
      {"0123456789e.",                   cpa_num,      &cps_in_num},
      {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-*/", cpa_build_op, &cps_in_op},
      { "",                              cpa_null,     NULL},
    }
  };

CP_STATE cps_neg =
  {
    "cps_neg",
    cpae_neg,
    {
      {"0123456789e.", cpa_neg_num,  &cps_in_num},
      {" ",            cpa_do_minus, &cps_init},
      {"_",            cpa_do_minus, &cps_init},
      { "",            cpa_null,     NULL},
    }
  };

CP_STATE cps_in_num =
  {
    "cps_in_num",
    cpae_in_num,
    {
      {"0123456789e.", cpa_num,            &cps_in_num},
      {" ",            cpa_conv_push_num,  &cps_init},
      {"_",            cpa_conv_push_num,  &cps_init},
      { "",            cpa_null,     NULL},
    }
  };

CP_STATE cps_in_op =
  {
    "cps_in_op",
    cpae_in_op,
    {
      {"0123456789.",                    cpa_build_op, &cps_in_op},
      {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-*/", cpa_build_op, &cps_in_op},
      {" ",                              cpa_do_op,    &cps_init},
      {"_",                              cpa_do_op,    &cps_init},
      { "",                              cpa_null,     NULL},
    }
  };

// Evaluate the entry string using an FSM to do the parsing.
#define CALL_CUR_STATE_EA (current_state->entry_action)('\0')

void forth_eval_fsm(char *e)
{
  CP_STATE *current_state;
  CP_STATE *last_state;
  int done = 0;
  
  current_state = &cps_init;
  
  // Just entered the state so cll the entry action
  CALL_CUR_STATE_EA;
  
  while( !done )
    {
      char ch = *e;
      
      // Process current state
      if( *e == '\0' )
        {
          ch = '_';

          // Exit after processing the end of line
          done = 1;
        }
      else
        {
          ch = *e;
        }

      int i = 0;

#if CALC_FSM_DB
      printf("\nProcessing ch:'%c'", ch);
#endif

      while( strlen(current_state->transition[i].c) != 0 )
        {
          if( strchr(current_state->transition[i].c, ch) != NULL )
            {
#if CALC_FSM_DB
              printf("\nAction");
              cfsm_status();
#endif

              if( current_state->transition[i].action != NULL )
                {
#if CALC_FSM_DB
              printf(" called");
#endif
                  (current_state->transition[i].action)(ch);
                }
              else
                {
#if CALC_FSM_DB
              printf(" is NULL");
#endif
                }
              
              // Move states
              last_state = current_state;
              current_state = current_state->transition[i].next_state;

              // If we have entered a new state then call entry action
              if( last_state != current_state )
                {
#if CALC_FSM_DB
                  printf("\nEntering state %s", current_state->name);
#endif
                  CALL_CUR_STATE_EA;
                }
            }
          
          i++;
        }

      e++;
    }
}

void forth_eval(char *e)
{
  char num[50];
  char cmd[50];
  char frag[2] = " ";
  int isnum = 0;
  int iscmd = 0;

  printf("\nEval:%s", entry);
  
  num[0] = '\0';
  cmd[0] = '\0';
  
  while( strlen(e) > 0 )
    {
      switch(*e)
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
          frag[0] = *e;
          strcat(num, frag);
          isnum = 1;

          if( strlen(e) == 1 )
            {
              move_stack_up();
              sscanf(num, "%lf", &(stack[0]));
              printf("\nNum:%s", num);
              num[0] = '\0';
              isnum = 0;
            }
          break;

        case ' ':
          // Push numbers on stack or execute cmds
          if( isnum )
            {
              move_stack_up();
              sscanf(num, "%lf", &(stack[0]));
              printf("\nNum:%s", num);
              num[0] = '\0';
              isnum = 0;
            }

          if( iscmd )
            {
              // Execute command
              execute_cmd(cmd);
              printf("\nCmd:%s", cmd);
              cmd[0] = '\0';
              iscmd = 0;
            }
          break;
          
        default:
          frag[0] = *e;
          strcat(cmd, frag);
          iscmd = 1;
          if( strlen(e) == 1)
            {
              // Execute command
              execute_cmd(cmd);
              printf("\nCmd:%s", cmd);
              cmd[0] = '\0';
              iscmd = 0;
            }
          break;
        }

      e++;
    }
  
  entry[0] = '\0';
}

void menu_forth_core(int type)
{
  int done = 0;
  int v1=0, v2=0;
  int hex_ndec = 1;

  char frag[2] = " ";
  
  cursor_on = 1;
  cursor_blink = 1;
  
  entry[0] = '\0';

  for(int i=0; i<MAX_STACK; i++)
    {
      stack[i] = 0.0;
    }
  
  display_forth(entry);
  
  while(!done)
    {
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

              frag[0] = k;
              strcat(entry, frag);
	      break;

	    case KEY_EXE:
              // Evaluate
              switch(type)
                {
                case 1:
                  forth_eval(entry);
                  break;

                case 2:
                  forth_eval_fsm(entry);
                  break;
                }

              // Clear the entry just processed
              entry[0] = '\0';
	      break;
	      
	    case KEY_DEL:
	      // Remove last character
              if( strlen(entry) > 0 )
                {
                  entry[strlen(entry)-1] = '\0';
                }
              
	      break;
	    }

	  display_forth(entry);
	}
    }
}

void menu_calc1(void)
{
  menu_forth_core(1);
}

void menu_calc2(void)
{
  menu_forth_core(2);
}


////////////////////////////////////////////////////////////////////////////////

#define DPVIEW_N 3

char *dpview_data =
  
   "Long test string for dpview that requires scrolling\t"
   "Short string\t"
   "Str";

void menu_dp_view(void)
{
  KEYCODE k;
  int done = 0;
  int view_line = 1;
  
  while(!done)
    {
      k = ed_view(dpview_data, view_line);

      switch(k)
	{
	case KEY_UP:
	  if( view_line > 0 )
	    {
	      view_line--;
	    }
	  break;
	  
	case KEY_DOWN:
	  if( view_line < (DISPLAY_NUM_LINES-1) )
	    {
	      view_line++;
	    }
	  break;

	case KEY_ON:
	  done = 1;
	  break;
	}
    }
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

#if PSION_MINI
#define SW (128/2.0)
#define SH (64.0/2.0)
#else
#define SW (128/2.0)
#define SH (32.0/2.0)
#endif

void menu_bubble(void)
{
  int var_idx = 0;

#define N_IDX 0
#define T_IDX 1
#define H_IDX 2
  
  double tau = 3.1415*2;
  double r   = tau/2.350;
  double h = 0.7;
  int cx, cy;
  
  int    n   = 5;
  int    sz  = 200;
  double sw  = SW;
  double sh  = SH;
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
	      b = v/2.0*sh+sh;
	      ax = (int)a;
	      by = (int)b;

	      cx = ax;
	      cy = by;
	      dd_plot_point(ax, by, (i*j)>(mi*mj)*h?1:0);
	    }
	}
      
      dd_update();
	  
      t += tinc;

      if( t > (2.0 * 3.14159265358) )
	{
	  t -= (2.0 * 3.14159265358);
	}
      
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

	  if( k == 'I' )
	    {
	      // Dump information
	      printf("\ncx,cy=(%d,%d)", cx, cy);
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

    {'O', "Off",        menu_instant_off},
    {'B', "Bubble",     menu_bubble},
    {'T', "Test",       menu_goto_test_os},
    {'H', "Hex",        menu_hex},
    {'F', "Find",       menu_fl_find},
    {'S', "Save",       menu_fl_save},
    {'M', "forMat",     menu_goto_format},
    {'P', "Prog",       menu_goto_prog},
    {'C', "Calc",       menu_goto_calc},
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

MENU menu_format =
  {
   &menu_top,
   "Format Y/N?",
   init_menu_format,   
   {
    {KEY_ON, "",        menu_back},
    {'Y', "Yes",        menu_fmat},
    {'N', "No",         menu_back},
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
    {'F', "Flowtext",   menu_flowtext_test},
    {'K', "Keytest",    menu_scan_test},
    {'&', "",           menu_null},
   }
  };

MENU menu_prog =
  {
   &menu_top,
   "Program",
   init_menu_prog,   
   {
    {KEY_ON, "",        menu_back},
    {'T', "Translate",  menu_prog_translate},
    {'R', "Run",        menu_prog_run},
    {'&', "",           menu_null},
   }
  };

MENU menu_calc =
  {
   &menu_top,
   "Calc",
   init_menu_calc,   
   {
    {KEY_ON, "",        menu_back},
    {'C', "Calc",       menu_calc1},
    {'F', "FSM Calc",   menu_calc2},
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
