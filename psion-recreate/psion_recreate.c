////////////////////////////////////////////////////////////////////////////////
//
// Psion Recreation
//
// Simple C based rereation
//
//
////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
//#include <math.h>
#include "pico/divider.h"
#include "pico/stdlib.h"
//#include "hardware/vreg.h"

#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"

#include "psion_recreate.h"
#include "emulator.h"
#include "wireless.h"
#include "eeprom.h"
#include "menu.h"
#include "rtc.h"
#include "svc_kb.h"

////////////////////////////////////////////////////////////////////////////////

volatile int core1_in_menu = 0;

////////////////////////////////////////////////////////////////////////////////

void initialise_oled(void);
void clear_oled(void);
void serial_loop(void);

////////////////////////////////////////////////////////////////////////////////
//
// GPIOs
//
//
////////////////////////////////////////////////////////////////////////////////

#define PIN_SD0 0
#define PIN_SD1 1
#define PIN_SD2 2
#define PIN_SD3 3
#define PIN_SD4 4
#define PIN_SD5 5
#define PIN_SD6 6
#define PIN_SD7 7

#define PIN_SCLK       10
#define PIN_SOE        11
#define PIN_SMR        12
#define PIN_P57        13

const uint PIN_SDAOUT     = 14;
const uint PIN_LATCHOUT2  = 15;
const uint PIN_I2C_SDA    = 16;
const uint PIN_I2C_SCL    = 17;
const uint PIN_LS_DIR     = 18;
const uint PIN_LATCHIN    = 19;
const uint PIN_SCLKIN     = 20;
const uint PIN_SDAIN      = 21;
const uint PIN_LATCHOUT1  = 22;

const uint PIN_SCLKOUT    = 26;
const uint PIN_VBAT_SW_ON = 27;

volatile uint16_t latch2_shadow    = 0;
volatile uint16_t latchout1_shadow = 0;

uint16_t csum_calc_on_restore = 0;
uint16_t csum_in_eeprom       = 0;
uint16_t csum_calc_on_dump    = 0;

#if TRACE_ADDR

// Trace a number of execution addresses
int tracing            = 0;
u_int16_t trigger_addr = 0x8242;
int addr_trace_i       = 0;

// Trace from a trigger address until trace full
volatile u_int16_t addr_trace_from[NUM_ADDR_TRACE];
volatile u_int8_t  addr_trace_from_flags[NUM_ADDR_TRACE];
volatile u_int8_t  addr_trace_from_a[NUM_ADDR_TRACE];
volatile u_int8_t  addr_trace_from_b[NUM_ADDR_TRACE];
volatile u_int16_t addr_trace_from_x[NUM_ADDR_TRACE];
volatile u_int16_t addr_trace_from_sp[NUM_ADDR_TRACE];

// Trace continuously until trigger address seen
u_int16_t trace_stop_addr = 0xc0f4;
volatile u_int16_t addr_trace_to[NUM_ADDR_TRACE];
volatile u_int8_t  addr_trace_to_flags[NUM_ADDR_TRACE];
volatile u_int8_t  addr_trace_to_a[NUM_ADDR_TRACE];
volatile u_int8_t  addr_trace_to_b[NUM_ADDR_TRACE];
volatile u_int16_t addr_trace_to_x[NUM_ADDR_TRACE];
volatile u_int16_t addr_trace_to_sp[NUM_ADDR_TRACE];

int addr_trace_to_i       = 0;
int tracing_to            = 1;

#endif

void latch2_set_mask(int value)
{
    if( latch2_shadow == 0x2009 )
    {
      volatile int x = 0;

      while(x)
	{
	}
    }

  latch2_shadow |= value;
  write_595(PIN_LATCHOUT2, latch2_shadow, 16);

  if( latch2_shadow == 0x2009 )
    {
      volatile int x = 0;

      while(x)
	{
	}
    }

}

void latch2_clear_mask(int value)
{
  if( value == 0x2009 )
    {
      volatile int x = 0;

      while(x)
	{
	}
    }

  latch2_shadow &= ~value;
  write_595(PIN_LATCHOUT2, latch2_shadow, 16);

  if( latch2_shadow == 0x2009 )
    {
      volatile int x = 0;

      while(x)
	{
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Read an 8 bit value from a 165 latch
//
////////////////////////////////////////////////////////////////////////////////

#define D165 1

uint8_t read_165(const uint latchpin)
{
  uint8_t value = 0;
  
  // Latch the data
  gpio_put(latchpin, 0);
  gpio_put(latchpin, 0);
  core1_safe_delay_ms(D165);
  //  sleep_us(D165);
  gpio_put(latchpin, 1);
  //sleep_us(D165);
  core1_safe_delay_ms(D165);
  
  // Clock the data out of the latch
  for(int i=0; i<8; i++)
    {
      
      // Read data
      value <<= 1;
      if( gpio_get(PIN_SDAIN) )
	{
	  value |= 1;
	}

      gpio_put(PIN_SCLKIN, 0);
      core1_safe_delay_ms(D165);
      //      sleep_us(D165);
      gpio_put(PIN_SCLKIN, 1);
      //sleep_us(D165);
      core1_safe_delay_ms(D165);
    }

  return(value);
}

////////////////////////////////////////////////////////////////////////////////
//
// Write an N bit pattern to a 595 latch or latches
//
////////////////////////////////////////////////////////////////////////////////

#define D595 1
volatile int core1_safe_x = 0;

void core1_safe_delay_ms(uint32_t interval_ms)
{
#if 0
  //      const uint32_t interval_ms = 2;
  uint32_t start_ms = 0;
  
  while ( board_millis() - start_ms < interval_ms)
    {
      core1_safe_x++;
    }
#else
  sleep_ms(interval_ms);
#endif
}

void write_595(const uint latchpin, int value, int n)
{
  
  // Latch pin low
  gpio_put(latchpin, 0);
  //sleep_us(D595);
  core1_safe_delay_ms(D595);
  
  for(int i = 0; i<n; i++)
    {
      // Clock low
      gpio_put(PIN_SCLKOUT, 0);
      //sleep_us(D595);
      core1_safe_delay_ms(D595);
      
      // Set data up
      if( value & (1<< (n-1)) )
	{
	  gpio_put(PIN_SDAOUT, 1);
	}
      else
	{
	  gpio_put(PIN_SDAOUT, 0);
	}

      //sleep_us(D595);
      core1_safe_delay_ms(D595);
	
      // Shift to next data bit
      value <<= 1;
      
      // Clock data in
      gpio_put(PIN_SCLKOUT, 1);
      //sleep_us(D595);
      core1_safe_delay_ms(D595);
    }

  // Now latch value (update outputs)
  gpio_put(latchpin, 1);

}

////////////////////////////////////////////////////////////////////////////////
//
// Test the keyboard
//

void keyboard_test(void)
{
  uint8_t drive = 0;
  uint8_t port5 = 0;

  i_printxy_str(0,0, "Keyboard Test");

    while(1)
    {
      if( drive == 0 )
	{
	  drive = 0x08;
	}

      // Drive KB line (keep display out of reset)
      write_595(PIN_LATCHOUT1, drive | 0xFF, 8);

      // read port5
      port5 = (read_165(PIN_LATCHIN));

      // Display value
      i_printxy_hex(0, 2, port5);
    }
}



////////////////////////////////////////////////////////////////////////////////
//
// To improve performance we use the second core. It performs:
//
// Display update of buffer to OLED over I2C
// Wireless task manager
// RTC tasks
// EEPROM accesses
// EEPROM dump and restore
//
//
// As the core peforms OLED updates over I2C it also has to do all other
// accesses or we'd have to have locks to prevent two cores using the same
// I2C bus.

typedef enum
  {
   MS_ENTER = 1,
   MS_INIT,
   MS_RUNNING,
   MS_LEAVE,
   MS_IDLE,
  } MENU_STATE;
  
MENU_STATE ms = MS_IDLE;

void menu_tasks(void)
{
  switch(ms)
    {
    case MS_IDLE:
      if( core1_in_menu )
	{
	  ms = MS_ENTER;
	}
      break;

    case MS_ENTER:
      menu_enter();
      ms = MS_RUNNING;
      break;

    case MS_RUNNING:
      menu_loop();

      if( menu_done )
	{
	  menu_done = 0;
	  ms = MS_LEAVE;
	}
      break;

    case MS_LEAVE:
      menu_leave();
      core1_in_menu = 0;
      ms = MS_IDLE;
      break;

    }
}

//------------------------------------------------------------------------------
//
// Cursor
#define CURSOR_CHAR 0x100
#define CURSOR_UNDERLINE 0x101

int cursor_on = 0;
uint64_t cursor_upd_time = 1000000L;
uint64_t cursor_last_time = 0;
volatile int cursor_phase = 0;
int cursor_char = 0x101;
int under_cursor_char[DISPLAY_NUM_CHARS][DISPLAY_NUM_LINES];
int cursor_x = 3;
int cursor_y = 0;
int cursor_blink = 0;
int saved_char = 0;
int force_cursor_update = 0;

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

//------------------------------------------------------------------------------
// These are the tasks the menu functions need to perform in order to
// keep the display, wireless and so on running.

void menu_loop_tasks(void)
{
  tud_task();
  matrix_scan();
  cursor_task();
  //  rtc_tasks();
  //  eeprom_tasks();
  //  wireless_taskloop();
  serial_loop();
  hid_task();
}

////////////////////////////////////////////////////////////////////////////////
//
// This is the main function for the second core. It handles
// everything apart from emulationm
//
////////////////////////////////////////////////////////////////////////////////

volatile int matscan_count = 0;
volatile int core1_count = 0;

void core1_main(void)
{
  matscan_count++;
  
  multicore_lockout_victim_init();
  
  while(1)
    {
      core1_count++;
#if 1
      // Handle USB
      //tud_task();
      //busy_wait_ms(10);
      // Scan keyboard every 1ms
      const uint32_t interval_ms = 2;
      static uint32_t start_ms = 0;
      
      if ( board_millis() - start_ms >= interval_ms)
	{
	  //matrix_scan();
	  start_ms += interval_ms;
	  matscan_count++;

	}

      // Handle USB
      //hid_task();

#else
      matscan_count++;
#endif      
    }
}

////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////

int main()
{

  // Set up the GPIOs
  gpio_init(PIN_VBAT_SW_ON);
  gpio_set_dir(PIN_VBAT_SW_ON, GPIO_OUT);

  // Take power on high so we latch on
  gpio_put(PIN_VBAT_SW_ON, 1);

  // ON key
  gpio_init(PIN_P57);
  
  // Set GPIOs up
  gpio_init(PIN_SDAOUT);
  gpio_init(PIN_LATCHOUT2);
  gpio_init(PIN_LATCHOUT1);
  gpio_init(PIN_SDAIN);
  gpio_init(PIN_SCLKOUT);
  gpio_init(PIN_LATCHIN);
  gpio_init(PIN_SCLKIN);
  gpio_init(PIN_LS_DIR);
  
  gpio_init(PIN_SD0);
  gpio_init(PIN_SD1);
  gpio_init(PIN_SD2);
  gpio_init(PIN_SD3);
  gpio_init(PIN_SD4);
  gpio_init(PIN_SD5);
  gpio_init(PIN_SD6);
  gpio_init(PIN_SD7);

  // Turn on pull downs on the data bus
  gpio_pull_down(PIN_SD0);
  gpio_pull_down(PIN_SD1);
  gpio_pull_down(PIN_SD2);
  gpio_pull_down(PIN_SD3);
  gpio_pull_down(PIN_SD4);
  gpio_pull_down(PIN_SD5);
  gpio_pull_down(PIN_SD6);
  gpio_pull_down(PIN_SD7);
  
  gpio_init(PIN_SCLK);
  gpio_init(PIN_SOE);
  gpio_init(PIN_SMR);

  // I2C bus
  gpio_init(PIN_I2C_SCL);
  gpio_init(PIN_I2C_SDA);

  // Use pull ups
  gpio_pull_up(PIN_I2C_SDA);
  gpio_pull_up(PIN_I2C_SCL);
  
  gpio_set_dir(PIN_SDAOUT,    GPIO_OUT);
  gpio_set_dir(PIN_P57,       GPIO_IN);
  gpio_set_dir(PIN_SDAOUT,    GPIO_OUT);
  gpio_set_dir(PIN_LATCHOUT2, GPIO_OUT);
  gpio_set_dir(PIN_LATCHOUT1, GPIO_OUT);
  gpio_set_dir(PIN_SDAIN,     GPIO_IN);
  gpio_set_dir(PIN_SCLKOUT,   GPIO_OUT);
  gpio_set_dir(PIN_LATCHIN,   GPIO_OUT);
  gpio_set_dir(PIN_SCLKIN,    GPIO_OUT);
  gpio_set_dir(PIN_LS_DIR,    GPIO_OUT);
  
  gpio_set_dir(PIN_SCLK, GPIO_OUT);
  gpio_set_dir(PIN_SOE,  GPIO_OUT);
  gpio_set_dir(PIN_SMR,  GPIO_OUT);

  // Unlatch input latch
  //  gpio_put(PIN_SDAIN,  1);
  gpio_put(PIN_SCLKIN, 1);

#if OVERCLOCK
  //set_sys_clock_khz(260000, false);   // Works
  //  set_sys_clock_khz(300000, false);  
  //set_sys_clock_khz(90000, false);  
#endif
  //------------------------------------------------------------------------------
  //
  // Display test
  //
  
#if FN_OLED_DEMO  
  oledmain();
#endif
  //------------------------------------------------------------------------------
  //
  // Initialise display

  // Turn on the 12V supply
  // This signal does nothing now. OLED turned on when 3V3 turned on
  
  latch2_set_mask(LAT2PIN_MASK_DRV_HV);
  
  //  latchout2_shadow |= LAT2PIN_MASK_DRV_HV;
  //write_595(PIN_LATCHOUT2, latchout2_shadow, 16);

  // Initialise I2C
  //i2c_fn_initialise();
  
  // Wait for it to start up
  sleep_ms(10);
  initialise_oled();
  
  printxy_str(0, 0, "***");

  init_usb();
    
  // Clear screen
  clear_oled();
  stdio_init_all();

  // Restore RAM from EEPROM

  // Initialise emulator
  //initialise_emulator();
  
  // Initialise wifi
  wireless_init();

  // EEPROM tests before core 1 starts, so display won't work but tests
  // won't have interference from that core on I2C
  
  //#if MULTI_CORE
  // If multi core then we run the LCD update on the other core
  multicore_launch_core1(core1_main);

    //#endif



    ////////////////////////////////////////////////////////////////////////////////
    //
    // C based main
    //
    // Scan keys, handle keypresses and loop
    //
    ////////////////////////////////////////////////////////////////////////////////

    char *toggle;
    int t = 0;

    //sleep_ms(2000);

    menu_enter();
    
    printf("\nC Based Recreation\n");

    while(1)
      {
	//core1_main();

	t++;
	//menu_loop_tasks();
	
	tud_task();
	matrix_scan();
	menu_loop();
	serial_loop();
	hid_task();
      }
}

