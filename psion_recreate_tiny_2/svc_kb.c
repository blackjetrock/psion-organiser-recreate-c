// Keyboard service
// Keyboard matrix scan is here as well

// Code tries to followoriginal Psion in key scanning, but
// is a totally different implementation.

// matrix_scan() scans the keyboard matrix and the matrix
// state is passed to the debouncer.
// If keys are pressed then they are sent to the translator
// that handles the shift and caps keys and so on and puts its
// output in the keyboard buffer which is then handled by the
// 'foreground' code.


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
#include "svc.h"
#include "switches.h"

#define MAT_SCAN_STATE_DRIVE  0
#define MAT_SCAN_STATE_READ   5
#define MAT_SCAN_STATE_NEXT   (MAT_SCAN_STATE_READ+1)

// This allows other code to insert keys into the key queue. Note: Core1
// has to be the code that does this, and i tmust run on core1. So, put a key in this
// location and wait for it to tuirn to KEY_NONE before putting another one there.
// If multiple code needs ot do this, then add more variables and add insertion in the KB code.

KEYCODE kb_external_key = KEY_NONE;

// Called regularly, can be on core1.

// Handles multiple key presses

int mat_scan_drive = 0x01;
int mat_scan_n     = 0;
int mat_scan_last_n = 0;

int mat_scan_state = MAT_SCAN_STATE_DRIVE;
int mat_sense = 0;

// Last time a key was presed (or we reset). used to turn off after some time delay
uint64_t last_key_press_time = 0;
int init_last_key = 1;

// How long we wait before turning off if no keys pressed
uint64_t kb_inactivity_timeout = 20*60*1000*1000;
uint64_t last_tick_time = 0;
uint64_t tick_interval = 5*1000*1000;
int tick = 0;

// Bit positions within matrix of each key

#define MATRIX_BIT_CAP    1
#define MATRIX_BIT_NUM    2
#define MATRIX_BIT_UP     1
#define MATRIX_BIT_DOWN   2
#define MATRIX_BIT_LEFT   3
#define MATRIX_BIT_RIGHT  4
#define MATRIX_BIT_A      9
#define MATRIX_BIT_B     14
#define MATRIX_BIT_C     19
#define MATRIX_BIT_D     34
#define MATRIX_BIT_E     24
#define MATRIX_BIT_F     29
#define MATRIX_BIT_G      8
#define MATRIX_BIT_H     13
#define MATRIX_BIT_I     18
#define MATRIX_BIT_J     33
#define MATRIX_BIT_K     23
#define MATRIX_BIT_L     28
#define MATRIX_BIT_M      7
#define MATRIX_BIT_N     12
#define MATRIX_BIT_O     17
#define MATRIX_BIT_P     32
#define MATRIX_BIT_Q     22
#define MATRIX_BIT_R     27
#define MATRIX_BIT_S      6
#define MATRIX_BIT_T     11
#define MATRIX_BIT_U     16
#define MATRIX_BIT_V     31
#define MATRIX_BIT_W     21
#define MATRIX_BIT_X     26
#define MATRIX_BIT_Y     15
#define MATRIX_BIT_Z     30
#define MATRIX_BIT_EXE   25
#define MATRIX_BIT_SPACE 20
#define MATRIX_BIT_MODE   0
#define MATRIX_BIT_DEL   10

#define MATRIX_BIT_SHIFT  5
#define MATRIX_BIT_ON    63


volatile MATRIX_MAP mat_scan_matrix = 0;

typedef struct _KEYMAP
{
  uint64_t mask;
  char c;
} KEYMAP;

KEYMAP base_map[] =
  {
   { (uint64_t)1 << MATRIX_BIT_ON,     1 },
   { (uint64_t)1 << MATRIX_BIT_UP,     3 },
   { (uint64_t)1 << MATRIX_BIT_DOWN,   4 },
   { (uint64_t)1 << MATRIX_BIT_LEFT,   5 },
   { (uint64_t)1 << MATRIX_BIT_RIGHT,  6 },
   { (uint64_t)1 << MATRIX_BIT_A,     'A' },
   { (uint64_t)1 << MATRIX_BIT_B,     'B' },
   { (uint64_t)1 << MATRIX_BIT_C,     'C' },
   { (uint64_t)1 << MATRIX_BIT_D,     'D' },
   { (uint64_t)1 << MATRIX_BIT_E,     'E' },
   { (uint64_t)1 << MATRIX_BIT_F,     'F' },
   { (uint64_t)1 << MATRIX_BIT_G,     'G' },
   { (uint64_t)1 << MATRIX_BIT_H,     'H' },
   { (uint64_t)1 << MATRIX_BIT_I,     'I' },
   { (uint64_t)1 << MATRIX_BIT_J,     'J' },
   { (uint64_t)1 << MATRIX_BIT_K,     'K' },
   { (uint64_t)1 << MATRIX_BIT_L,     'L' },
   { (uint64_t)1 << MATRIX_BIT_M,     'M' },
   { (uint64_t)1 << MATRIX_BIT_N,     'N' },
   { (uint64_t)1 << MATRIX_BIT_O,     'O' },
   { (uint64_t)1 << MATRIX_BIT_P,     'P' },
   { (uint64_t)1 << MATRIX_BIT_Q,     'Q' },
   { (uint64_t)1 << MATRIX_BIT_R,     'R' },
   { (uint64_t)1 << MATRIX_BIT_S,     'S' },
   { (uint64_t)1 << MATRIX_BIT_T,     'T' },
   { (uint64_t)1 << MATRIX_BIT_U,     'U' },
   { (uint64_t)1 << MATRIX_BIT_V,     'V' },
   { (uint64_t)1 << MATRIX_BIT_W,     'W' },
   { (uint64_t)1 << MATRIX_BIT_X,     'X' },
   { (uint64_t)1 << MATRIX_BIT_Y,     'Y' },
   { (uint64_t)1 << MATRIX_BIT_Z,     'Z' },
   { (uint64_t)1 << MATRIX_BIT_EXE,   13 },
   { (uint64_t)1 << MATRIX_BIT_SPACE, ' ' },
   { (uint64_t)1 << MATRIX_BIT_MODE,   2 },
   { (uint64_t)1 << MATRIX_BIT_DEL,    8 },
   { 0,                '-' }
  };

KEYMAP caps_map[] =
  {
   { (uint64_t)1 << MATRIX_BIT_ON,     1 },
   { (uint64_t)1 << MATRIX_BIT_UP,     3 },
   { (uint64_t)1 << MATRIX_BIT_DOWN,   4 },
   { (uint64_t)1 << MATRIX_BIT_LEFT,   5 },
   { (uint64_t)1 << MATRIX_BIT_RIGHT,  6 },
   { (uint64_t)1 << MATRIX_BIT_A,     'a' },
   { (uint64_t)1 << MATRIX_BIT_B,     'b' },
   { (uint64_t)1 << MATRIX_BIT_C,     'c' },
   { (uint64_t)1 << MATRIX_BIT_D,     'd' },
   { (uint64_t)1 << MATRIX_BIT_E,     'e' },
   { (uint64_t)1 << MATRIX_BIT_F,     'f' },
   { (uint64_t)1 << MATRIX_BIT_G,     'g' },
   { (uint64_t)1 << MATRIX_BIT_H,     'h' },
   { (uint64_t)1 << MATRIX_BIT_I,     'i' },
   { (uint64_t)1 << MATRIX_BIT_J,     'j' },
   { (uint64_t)1 << MATRIX_BIT_K,     'k' },
   { (uint64_t)1 << MATRIX_BIT_L,     'l' },
   { (uint64_t)1 << MATRIX_BIT_M,     'm' },
   { (uint64_t)1 << MATRIX_BIT_N,     'n' },
   { (uint64_t)1 << MATRIX_BIT_O,     'o' },
   { (uint64_t)1 << MATRIX_BIT_P,     'p' },
   { (uint64_t)1 << MATRIX_BIT_Q,     'q' },
   { (uint64_t)1 << MATRIX_BIT_R,     'r' },
   { (uint64_t)1 << MATRIX_BIT_S,     's' },
   { (uint64_t)1 << MATRIX_BIT_T,     't' },
   { (uint64_t)1 << MATRIX_BIT_U,     'u' },
   { (uint64_t)1 << MATRIX_BIT_V,     'v' },
   { (uint64_t)1 << MATRIX_BIT_W,     'w' },
   { (uint64_t)1 << MATRIX_BIT_X,     'x' },
   { (uint64_t)1 << MATRIX_BIT_Y,     'y' },
   { (uint64_t)1 << MATRIX_BIT_Z,     'z' },
   { (uint64_t)1 << MATRIX_BIT_EXE,    13 },
   { (uint64_t)1 << MATRIX_BIT_SPACE,  ' ' },
   { (uint64_t)1 << MATRIX_BIT_MODE,   2 },
   { (uint64_t)1 << MATRIX_BIT_DEL,    7 },
   { 0,                '-' }
  };

KEYMAP shifted_map[] =
  {
   { (uint64_t)1 << MATRIX_BIT_ON,     1 },
   { (uint64_t)1 << MATRIX_BIT_UP,     3 },
   { (uint64_t)1 << MATRIX_BIT_DOWN,   4 },
   { (uint64_t)1 << MATRIX_BIT_LEFT,   5 },
   { (uint64_t)1 << MATRIX_BIT_RIGHT,  6 },
   { (uint64_t)1 << MATRIX_BIT_A,     '<' },
   { (uint64_t)1 << MATRIX_BIT_B,     '>' },
   { (uint64_t)1 << MATRIX_BIT_C,     '(' },
   { (uint64_t)1 << MATRIX_BIT_D,     ')' },
   { (uint64_t)1 << MATRIX_BIT_E,     '%' },
   { (uint64_t)1 << MATRIX_BIT_F,     '/' },
   { (uint64_t)1 << MATRIX_BIT_G,     '=' },
   { (uint64_t)1 << MATRIX_BIT_H,     '"' },
   { (uint64_t)1 << MATRIX_BIT_I,     '7' },
   { (uint64_t)1 << MATRIX_BIT_J,     '8' },
   { (uint64_t)1 << MATRIX_BIT_K,     '9' },
   { (uint64_t)1 << MATRIX_BIT_L,     '*' },
   { (uint64_t)1 << MATRIX_BIT_M,     ',' },
   { (uint64_t)1 << MATRIX_BIT_N,     '$' },
   { (uint64_t)1 << MATRIX_BIT_O,     '4' },
   { (uint64_t)1 << MATRIX_BIT_P,     '5' },
   { (uint64_t)1 << MATRIX_BIT_Q,     '6' },
   { (uint64_t)1 << MATRIX_BIT_R,     '-' },
   { (uint64_t)1 << MATRIX_BIT_S,     ';' },
   { (uint64_t)1 << MATRIX_BIT_T,     ':' },
   { (uint64_t)1 << MATRIX_BIT_U,     '1' },
   { (uint64_t)1 << MATRIX_BIT_V,     '2' },
   { (uint64_t)1 << MATRIX_BIT_W,     '3' },
   { (uint64_t)1 << MATRIX_BIT_X,     '+' },
   { (uint64_t)1 << MATRIX_BIT_Y,     '0' },
   { (uint64_t)1 << MATRIX_BIT_Z,     '.' },
   { (uint64_t)1 << MATRIX_BIT_EXE,    13 },
   { (uint64_t)1 << MATRIX_BIT_SPACE,  ' ' },
   { (uint64_t)1 << MATRIX_BIT_MODE,   2 },
   { (uint64_t)1 << MATRIX_BIT_DEL,    7 },
   { 0,                '-' }
  };

// This table picksa key map from the current values
// of shift_pressed, caps_on and num_on
#define CAPS_ON    1
#define NUM_ON     2
#define SHIFT_ON   4
#define CAPS_OFF   0
#define NUM_OFF    0
#define SHIFT_OFF  0

typedef struct _KEY_MAP_MAP
{
  int modes;
  KEYMAP *map;  
} KEY_MAP_MAP;

KEY_MAP_MAP key_map_map[8] =
  {
   {SHIFT_OFF | CAPS_OFF| NUM_OFF,  base_map},
   {SHIFT_OFF | CAPS_OFF| NUM_ON,   shifted_map},
   {SHIFT_OFF | CAPS_ON | NUM_OFF,  caps_map},
   {SHIFT_OFF | CAPS_ON | NUM_ON,   shifted_map},
   {SHIFT_ON  | CAPS_OFF| NUM_OFF,  shifted_map},
   {SHIFT_ON  | CAPS_OFF| NUM_ON,   base_map},
   {SHIFT_ON  | CAPS_ON | NUM_OFF,  shifted_map},
   {SHIFT_ON  | CAPS_ON | NUM_ON,   base_map},
  };

#define NUM_MODES ((sizeof(key_map_map))/(sizeof(KEY_MAP_MAP)))
KEYMAP *key_map;

////////////////////////////////////////////////////////////////////////////////
//
// Debounces the pressed key matrix
//
// We have a stream of matrices. A 1 is a pressed key. we need to deboune them
// all, which we can do on a bit by bit basis, all at once as integers.
//
// We remove short presses,
//

#define MAX_INPUT_QUEUE  5
#define MAX_PRESS_QUEUE  5

MATRIX_MAP  input_queue[MAX_INPUT_QUEUE];
MATRIX_MAP  pressed_queue[MAX_PRESS_QUEUE];
MATRIX_MAP  released_queue[MAX_PRESS_QUEUE];
MATRIX_MAP  pressed_edges;
MATRIX_MAP  released_edges;
MATRIX_MAP  key_states = 0LL;

int caps_mode     = 0;
int num_mode      = 0;
int shift_pressed = 0;

//------------------------------------------------------------------------------
// The key buffer 

volatile char nos_key_buffer[NOS_KEY_BUFFER_LEN];

// Keys in and out pointers
volatile int nos_key_in  = 0;
volatile int nos_key_out = 0;

#define NEXT_KEY_IDX(IDX) ((IDX+1)%NOS_KEY_BUFFER_LEN)

void __not_in_flash_func(nos_put_key)(char key)
{
  // Is buffer full?
  if( NEXT_KEY_IDX(nos_key_in) == nos_key_out )
    {
      // Full, drop the key
      // Original beeps here
      return;
    }

  nos_key_buffer[nos_key_in] = key;
  nos_key_in = NEXT_KEY_IDX(nos_key_in);
}

char nos_get_key(void)
{
  char k = NOS_KEY_NONE;
  
  if( nos_key_in == nos_key_out )
    {
      return(k);
    }

  k = nos_key_buffer[nos_key_out];

  nos_key_out = NEXT_KEY_IDX(nos_key_out);
  return(k);
}

//------------------------------------------------------------------------------

#define KEY_PRESSED(MAP,BITNO) ((MAP) & ((uint64_t)1 << BITNO))

uint64_t inactivity_now;
uint64_t inactivity_time;
int i, k;
int cur_modes;

void __not_in_flash_func(matrix_debounce)(MATRIX_MAP matrix)
{
  //  printf("\n  DB");
  
  // Move queue along and insert new value
  for( i=1; i<MAX_INPUT_QUEUE;i++)
    {
      input_queue[i] = input_queue[i-1];
    }

  input_queue[0] = matrix;

  // shift the pressed and unpressed queues so a new entry
  // can be added
  for( i=1; i<MAX_PRESS_QUEUE;i++)
    {
      pressed_queue[i]  = pressed_queue[i-1];
      released_queue[i] = released_queue[i-1];
    }
  
  pressed_queue[0]  = ~((uint64_t)0);
  released_queue[0] = ~((uint64_t)0);
  
  // AND the input and also the negation of the input
  // This gives us a stream of pressed and released edges.
  for(i=0; i<MAX_INPUT_QUEUE;i++)
    {
      // All keys held for max_queue samples
      pressed_queue[0] &= input_queue[i];
      released_queue[0] &= ~(input_queue[i]);
    }

  // Now find pressed edge events and released edge events
  pressed_edges  =  pressed_queue[0] & ~pressed_queue[1];
  released_edges = ~pressed_queue[0] &  pressed_queue[1];

  if ( pressed_edges | released_edges)
    {
#if 0
      printf("\nPed:%016llX Red:%016llX", pressed_edges, released_edges);
      printf("\n");
#endif
      for(i=0; i<64; i++)
	{
	  if( ((uint64_t)1<<i) & pressed_edges )
	    { 
#if 0
	      printf(" %d", i);
#endif
	    }
	}
    }

  // Now set and clear bits in the state matrix which will give the current
  // states of each key

  // Pressed keys
  key_states |= pressed_edges;

  // released keys
  key_states &= ~(released_edges);

  //printf("\n%016llX", key_states);

  // We now have the current states of the keys. 
  // We have pressed and released edges, so we can generate pressed and released
  // events, and also work out which key map to use from the states of the
  // SHIFT, NUM and CAPS keys.

  // The key queue is made up of pressed events only

  shift_pressed = key_states & ((uint64_t)1<< MATRIX_BIT_SHIFT);
  
  // We need to handle the shift key for upper/lower case
  // and also the CAPS and NUM states
  
  if( KEY_PRESSED(pressed_edges,MATRIX_BIT_CAP) && shift_pressed)
    {
      caps_mode = !caps_mode;
      //printf("\nCAPS lock:%d", caps_mode);
    }
  
  if( KEY_PRESSED(pressed_edges,MATRIX_BIT_NUM) && shift_pressed)
    {
      num_mode = !num_mode;
      //printf("\nNUM lock:%d", num_mode);
    }

  // Find the key map to use
  cur_modes = 0;
  if( shift_pressed )
    {
      cur_modes |= SHIFT_ON;
    }

  if( caps_mode )
    {
      cur_modes |= CAPS_ON;
    }

  if( num_mode )
    {
      cur_modes |= NUM_ON;
    }

  key_map = base_map;
  
  for(k=0; k<NUM_MODES; k++)
    {
      if( cur_modes == key_map_map[k].modes )
	{
	  key_map = key_map_map[k].map;
	  break;
	}
    }
  
  // Now generate pressed events and put them into a queue
  i = 0;
  
  while( key_map[i].mask != 0 )
    {
      if( (key_map[i].mask) & pressed_edges )
	{
	  // Key pressed
	  //printf("\nC:%c", key_map[i].c);

	  // Update inactivity timeout
	  last_key_press_time = time_us_64();
	  
	  // Put key pressed event into key buffer
	  nos_put_key(key_map[i].c);
	  
	  //txbyte(2, key_map[i].c );
	  return;
	}
      i++;
    }

  // We may have keys from other sources, accept those as well
  if( kb_external_key != KEY_NONE )
    {
      // Update inactivity timeout
      last_key_press_time = time_us_64();


      nos_put_key(kb_external_key);
      kb_external_key = KEY_NONE;
    }

  // Inactivity processing
#if 0
  // Initialise the last key time as global initialisation has to be constant
  if( init_last_key )
    {
      init_last_key = 0;
      last_key_press_time = time_us_64();
      last_tick_time =  last_key_press_time;
    }

  inactivity_now = time_us_64();
  inactivity_time = (inactivity_now - last_key_press_time);

  if( (inactivity_now - last_tick_time) > tick_interval )
    {
      last_tick_time = inactivity_now;
      tick = 1;
    }
  
 
  if( inactivity_time > kb_inactivity_timeout )
    {
      // Inactive for too long, turn off;
      handle_power_off();
    }
  else
    {
      if( inactivity_time > (kb_inactivity_timeout / 10 * 7) )
	{
	  if( tick )
	    {
	      tick = 0;
	      //printf("\nInactive:turning off in %lld seconds", (kb_inactivity_timeout - inactivity_time)/1000000);
	    }
	}
    }
  #endif
  
}

////////////////////////////////////////////////////////////////////////////////
// Scans the key matrix and returns a raw bitmap with a 1 for a
// detected keypress and a 0 for no press. There's a small enough
// number of keys for this to fit in a uint32_t

int m;

void __not_in_flash_func(matrix_scan)(void)
{
#if 0
  printf("\n%s:State:%d Drive:%d", __FUNCTION__, mat_scan_state, mat_scan_drive);
#endif

  // Use a simple state machine for the scanning
  switch(mat_scan_state)
    {
    case MAT_SCAN_STATE_DRIVE:
      
      // Drive scan lines
      latchout1_shadow &= 0x80;
      latchout1_shadow |= mat_scan_drive;
      latchout1_shadow |= 0x80;
      write_595(PIN_LATCHOUT1, latchout1_shadow, 8);

      mat_scan_state = MAT_SCAN_STATE_READ;
      //mat_scan_state = MAT_SCAN_STATE_DRIVE;
      break;
      
     
    case MAT_SCAN_STATE_READ:

      mat_sense = (read_165(PIN_LATCHIN) & 0xFC)>>2;
      //mat_sense = 0xFF;
      
      // ON key is different, it's not part of the matrix, we can
      // simply put it into the matrix
      
      if( mat_sense & 0x20 )
	{
	  mat_scan_matrix |= ((uint64_t)1<<MATRIX_BIT_ON);
	  //printf("\nON");
	}
      else
	{

	  // See which keys are pressed, if any
	  if( (mat_sense & 0x1F) != 0 )
	    {

	      for(m=0; m<5; m++)
		{
		  if( mat_sense & ((uint64_t)1 << m) )
		    {
		      // We have a press, add it to the matrix
		      
		      mat_scan_matrix |= ((uint64_t)1 << (mat_scan_n*5 + m));
		      
		    }
		}
	    }
	}
      
      //mat_scan_state = MAT_SCAN_STATE_DRIVE;
	          mat_scan_state = MAT_SCAN_STATE_NEXT;
      break;
      
      // Move to next drive line
    case MAT_SCAN_STATE_NEXT:
      mat_scan_drive <<= 1;
      mat_scan_n++;
      if( mat_scan_drive == 0x80 )
	{
	  mat_scan_drive = 0x1;
	  mat_scan_n = 0;

	  // Before we clear the matrix for another scan, we send the
	  // matrix to the debouncer
	  //	  printf("\nMSM: %08X", mat_scan_matrix);

	  //printf("\nX:%08X", mat_scan_matrix);
	  matrix_debounce(mat_scan_matrix);
	  mat_scan_matrix = 0;
	}

      mat_scan_state = MAT_SCAN_STATE_DRIVE;
      break;
      
    default:
      break;
    }
}

////////////////////////////////////////////////////////////////////////////////

KEYCODE kb_getk(void)
{
  KEYCODE k;

#if DB_KB_GETK
  printf("\n%s::", __FUNCTION__);
#endif
  
  while(1)
    {
      // Keep the display updated
      menu_loop_tasks();
      
      if( kb_test() != KEY_NONE )
	{
	  k = nos_get_key();

#if DB_KB_GETK
	  printf("\n%s::exit:%d", __FUNCTION__, k);
#endif
	  return(k);
	}
    }

#if DB_KB_GETK  
  printf("\nExit:KEY_NONE");
#endif
  
  return(KEY_NONE);
}

////////////////////////////////////////////////////////////////////////////////
//
// Returns keycode if there is a key in the buffer
// otherwise KEY_NONE.
// Does not take key from buffer
//

KEYCODE kb_test(void)
{

  menu_loop_tasks();

#if DB_KB_TEST
  printf("\n%s:: I:%d O:%d", __FUNCTION__, nos_key_in, nos_key_out);
#endif
  
  if( nos_key_in == nos_key_out )
    {
      // Buffer empty
#if DB_KB_TEST
      printf("\nExit:KEY_NONE");
#endif
      return(KEY_NONE);
    }

#if DB_KB_TEST
  printf("\nExit:%d", nos_key_buffer[nos_key_out]);
#endif
  
  // Return key code but don't remove key from buffer
  return(nos_key_buffer[nos_key_out]);
}


////////////////////////////////////////////////////////////////////////////////
//
// Queue so the Psion can send keys to the host
// as a USB HID keyboard
//
// NOTE: these are USB HID keycodes, not ASCII
//
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// New key to be sent
// 0: not sent
// 1: sent

#define DELIVER_QUEUE_LENGTH  200
int deliver_q_in  = 0;
int deliver_q_out = 0;

int deliver_queue[DELIVER_QUEUE_LENGTH];

#define QUEUE_EMPTY (deliver_q_out == deliver_q_in)
#define QUEUE_FULL (NEXT_Q_PTR(deliver_q_in) == deliver_q_out )
#define QUEUE_FULL2 (NEXT_Q_PTR(NEXT_Q_PTR(deliver_q_in)) == deliver_q_out )
#define NEXT_Q_PTR(PTR) ((PTR+1) % DELIVER_QUEUE_LENGTH)

// A new key to send, queue it
// We can't send two identical keys in consecutive reports as it
// will look like the key is just held down, so we insert a no key
// report between them
#define MATRIX_KEY_NONE 0

int last_key = MATRIX_KEY_NONE;

void queue_hid_key(int k)
{
  // We want two slots so we can insert NO_KEY if needed
  if( QUEUE_FULL2 || QUEUE_FULL )
    {
      //printf("\nQueue full");
      return;
    }

  // Put a no key report before this key so it separates it from the
  // previous key if it was the same as this one.
  //printf("\nLast key:%04X key:%04X", last_key, k);
  if(last_key == k )
    {
      // No key report between pairs of keys
      deliver_queue[deliver_q_in] = MATRIX_KEY_NONE;
      deliver_q_in = NEXT_Q_PTR(deliver_q_in);
      //printf("\nQueued %d %04X ***", MATRIX_KEY_NONE, MATRIX_KEY_NONE);

    }

  deliver_queue[deliver_q_in] = k;
  deliver_q_in = NEXT_Q_PTR(deliver_q_in);


  last_key = k;  
  
  //printf("\nQueued %d %04X", k, k);
  
}

int unqueue_hid_key(void)
{
  int k;
  
  if( QUEUE_EMPTY )
    {
      //printf("\nQueue empty");
      return(MATRIX_KEY_NONE);
    }

  k = deliver_queue[deliver_q_out];
  deliver_q_out = NEXT_Q_PTR(deliver_q_out);
  //printf("\nUnqueued %d", k);
  return(k);
}

#if 0
uint8_t const conv_table[128][2] =  { HID_ASCII_TO_KEYCODE };
#endif

int translate_to_hid(char ch)
{
  int ret = 0;
  uint8_t modifier   = 0;

#if 0
  if ( conv_table[ch][0] )
    {
    modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    }
#endif

#if 0
  ret  = conv_table[ch][1];
  ret |= modifier << 8;
#else

  ret = ch;
#endif
  return(ret);
}
