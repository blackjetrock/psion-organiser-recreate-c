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

#define MAT_SCAN_STATE_DRIVE  0
#define MAT_SCAN_STATE_READ  10
#define MAT_SCAN_STATE_NEXT  11

// Called regularly, can be on core1.

// Handles multiple key presses

int mat_scan_drive = 0x01;
int mat_scan_n     = 0;
int mat_scan_last_n = 0;

int mat_scan_state = MAT_SCAN_STATE_DRIVE;
int mat_sense = 0;

// Bit positions within matrix of each key

#define MATRIX_BIT_ON 31

volatile MATRIX_MAP mat_scan_matrix = 0;

////////////////////////////////////////////////////////////////////////////////
//
// Debounces the pressed key matrix
//
// We have a stream of matrices. A 1 is a poressed key. we need to deboune them
// all, which we can do on a bit by bit basis, all at once as integers.
//
// We remove short presses, 
void matrix_debounce(MATRIX_MAP matrix)
{
  
}

////////////////////////////////////////////////////////////////////////////////
// Scans the key matrix and returns a raw bitmap with a 1 for a
// detected keypress and a 0 for no press. There's a small enough
// number of keys for this to fit in a uint32_t

void matrix_scan(void)
{
  sleep_ms(1);
  
  // Use a simple state machine for the scanning
  switch(mat_scan_state)
    {
    case MAT_SCAN_STATE_DRIVE:
      
      // Drive scan lines
      latchout1_shadow &= 0x80;
      latchout1_shadow |= mat_scan_drive;
      write_595(PIN_LATCHOUT1, latchout1_shadow, 8);

      mat_scan_state = MAT_SCAN_STATE_READ;
      break;
      
     
    case MAT_SCAN_STATE_READ:

      mat_sense = (read_165(PIN_LATCHIN) & 0xFC)>>2;
      
      // ON key is different, it's not part of the matrix, we can
      // simply put it into the matrix
      
      if( mat_sense & 0x20 )
	{
	  mat_scan_matrix |= (1<<MATRIX_BIT_ON);
	  
	}
      else
	{
	  // See which keys are pressed, if any
	  if( (mat_sense & 0x1F) != 0 )
	    {
	      for(int m=0; m<5; m++)
		{
		  
		  if( mat_sense & (1 << m) )
		    {
		      // We have a press, add it to the matrix
		      
		      mat_scan_matrix |= (1 << (mat_scan_n*5 + m));
		      
		    }
		}
	    }
	}
      
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

	  //	  printf("\nX:%08X", mat_scan_matrix);
	  matrix_debounce(mat_scan_matrix);
	  mat_scan_matrix = 0;
	}

      mat_scan_state = MAT_SCAN_STATE_DRIVE;
      break;
      
    default:
      break;
    }
}

