////////////////////////////////////////////////////////////////////////////////
//
// Serial CLI Handling
//
////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "pico/bootrom.h"

#include "psion_recreate.h"
#include "emulator.h"
#include "wireless.h"
#include "serial.h"

#include "svc.h"

int keypress = 0;
int parameter = 0;
unsigned int address = 0;

void serial_help(void);
void prompt(void);

void cli_boot_mass(void)
{
  reset_usb_boot(0,0);
}

void cli_set_address(void)
{
  address = parameter;
}

void cli_zero_parameter(void)
{
  parameter = 0;
}


// Dump the language stack

uint16_t read_ram_16(int a)
{
  return((RAMDATA(a)*256)+RAMDATA(a+1));
}

void print_frame(uint16_t a)
{
  uint16_t frame_fp            = read_ram_16(a);
  uint16_t frame_sp            = read_ram_16(a+2);
  uint16_t frame_global_table  = read_ram_16(a-2);
  
}

void cli_dump_language_stack(void)
{
  uint16_t rta_sp   = read_ram_16(0xa5);
  uint16_t bta_sbas = read_ram_16(0x2065);
  uint16_t rta_fp   = read_ram_16(0xa7);
  uint16_t rta_pc   = read_ram_16(0xa9);

  printf("\nbta_sbas: %04X", bta_sbas);
  printf("\nrta_sp:   %04X", rta_sp);
  printf("\nrta_fp:   %04X", rta_fp);
  printf("\nrta_pc:   %04X", rta_pc);

  // Print frame
  print_frame(rta_fp);
}

#define BYTE_WIDTH  16
  
void cli_dump_eeprom(void)
{
  char ascii[BYTE_WIDTH*3+5];
  char ascii_byte[5];
  uint8_t *flash = (uint8_t *)(XIP_BASE + ((uint32_t)((1024+512) * 1024)));
  
  uint8_t data[1024*4];

  read_eeprom(EEPROM_1_ADDR_RD, 0, 1024, (uint8_t *)&data);
  
  // Display memory from address
  printf("\n");

  ascii[0] = '\0';
  
  for(int z = 0; z<1024; z++)
    {
      int byte = 0;
      
      if( (z % BYTE_WIDTH) == 0)
	{
	  printf("  %s", ascii);
	  ascii[0] = '\0';
	  printf("\n%03X: ", z);
	}

#if 0      
      if( z >= ROM_START )
	{
	  byte =  data[z];
	  printf("%02X ", byte);
	}
      else
	{
	  byte =  data[z];
	  printf("%02X ", byte);
	}
#endif

      byte = *(flash++);
      if( isprint(byte) )
	{
	  sprintf(ascii_byte, "%c", byte);
	}
      else
	{
	  sprintf(ascii_byte, ".");
	}
      
      strcat(ascii, ascii_byte);
    }
  
  printf("\n");
}


  
void cli_write_eeprom(void)
{
  char wstr[] = "abcdefghij";
 
  // write
  write_eeprom(EEPROM_1_ADDR_RD, parameter, strlen(wstr), wstr);
}

  

void cli_dump_memory(void)
{
  char ascii[BYTE_WIDTH*3+5];
  char ascii_byte[5];
  
  // Display memory from address
  printf("\n");

  ascii[0] = '\0';
  
  for(unsigned int z = parameter; z<parameter+512; z++)
    {
      int byte = 0;
      
      if( (z % BYTE_WIDTH) == 0)
	{
	  printf("  %s", ascii);
	  ascii[0] = '\0';
	  printf("\n%08X: ", z);
	}

#if 0
      if( z >= ROM_START )
	{
	  byte =  ROMDATA(z-ROM_START);
	  printf("%02X ", byte);
	}
      else
	{
	  byte =  RAMDATA(z);
	  printf("%02X ", byte);
	}
#endif

      byte = *((uint8_t *)z);

      printf(" %02X", byte);
      
      if( isprint(byte) )
	{
	  sprintf(ascii_byte, "%c", byte);
	}
      else
	{
	  sprintf(ascii_byte, ".");
	}
      
      strcat(ascii, ascii_byte);
    }
  
  printf("\n");

}

void cli_info(void)
{
  printf("\nBank RAM Offset:%08X", ram_bank_off);
  printf("\nBank ROM Offset:%08X", rom_bank_off);

  printf("\nProcessor State");
  printf("\n===============");

  printf("\nPC:%04X", REG_PC);
  printf("\n%04X      A:%02X B:%02X X:%04X", REG_A, REG_B, REG_X);

}


// Another digit pressed, update the parameter variable
void cli_digit(void)
{
  unsigned int  n = 0;

  printf("\nK:%02X", keypress);
  printf("\nn:%02X", n);
  if( keypress > '9' )
    {
      n = keypress - 'a' + 10;
    }
  else
    {
      n = keypress - '0';
    }

  printf("\nn:%02X", n);
  parameter <<= 4;
  parameter |= n;
}

void cli_trace_dump_from(void)
{
  for(int i=0; i<addr_trace_i; i++)
    {
      printf("\n%04X:%04X      A:%02X B:%02X X:%04X SP:%04X FLAGS:%02X (%s)",	     
	     i,
	     addr_trace_from[i],
	     addr_trace_from_a[i],
	     addr_trace_from_b[i],
	     addr_trace_from_x[i],
	     addr_trace_from_sp[i],
	     addr_trace_from_flags[i],
	     decode_flag_value(addr_trace_from_flags[i])
	     );
    }
  
  printf("\n");
}

void cli_trace_dump_to(void)
{
  int j = addr_trace_to_i;
  
  for(int i=0; i<NUM_ADDR_TRACE; i++)
    {
      printf("\n%04X:%04X      A:%02X B:%02X X:%04X SP:%04X FLAGS:%02X (%s)",
	     i,
	     addr_trace_to[j],
	     addr_trace_to_a[j],
	     addr_trace_to_b[j],
	     addr_trace_to_x[j],
	     addr_trace_to_sp[j],
	     addr_trace_to_flags[j],
	     decode_flag_value(addr_trace_to_flags[j])
	     );
      j = ((j + 1) % NUM_ADDR_TRACE);
    }
  
  printf("\n");
}

////////////////////////////////////////////////////////////////////////////////

void cli_format(void)
{
  printf("\nFormatting A:");
  pk_setp(0);
  pk_fmat();
  printf("\nDone...");
  
}

void cli_catalog(void)
{
  int rc = 0;
  uint8_t rectype;
  
  char filename[32];
  printf("\nCatalog of A:\n");
  pk_setp(0);

  
  rc = fl_catl(1, 0, filename, &rectype);

  while(rc == 1)
    {
      printf("\n%s ($%02X)", filename, rectype);
      rc = fl_catl(0, 0, filename, &rectype);
    }

  printf("\nDone...");
  
}

void cli_create(void)
{
  int rc = 0;
  uint8_t rectype;
  char *filename;
  
  printf("\nCreate file on A:\n");
  printf("\nEnter filename:");
  filename = serial_get_string();
  
  pk_setp(0);
  
  fl_cret(filename);
  
  printf("\nDone...");
  
}

////////////////////////////////////////////////////////////////////////////////
//
// Allows the code to be driven from USB
//

int serial_terminal_mode = 0;
int serial_graphics_mode = 0;

void cli_terminal(void)
{
  // Clear screen
  printf("%c[2J", 27);

  printf("\n\n\n\n\n\n\n\n\n\n");

    // Set top and bottom margins
  // DECSTBM	ESC [ Pt ; Pb r

  printf("%c[6;0r", 27);

  
  serial_terminal_mode = !serial_terminal_mode;

  printf("\n========================================");
  printf("\n");
  printf("\nterminal mode is %s", serial_terminal_mode?"on.":"off.");
  printf("\n");
  printf("\nESC ESC to exit");
  printf("\nESC o   for ON");
  printf("\nESC m   for MODE");
  printf("\nArrow keys should work");
  printf("\n(Uses VT-102 codes)");
  printf("\n");
  printf("\n========================================");
  printf("\n");
  
}

void cli_graphics_terminal(void)
{
  // Clear screen
  printf("%c[2J", 27);

  printf("\n\n\n\n\n\n\n\n\n\n");

    // Set top and bottom margins
  // DECSTBM	ESC [ Pt ; Pb r

  printf("%c[40;0r", 27);

  
  serial_terminal_mode = !serial_terminal_mode;
  serial_graphics_mode = !serial_graphics_mode;
  
  printf("\n========================================");
  printf("\n");
  printf("\nterminal mode is %s", serial_terminal_mode?"on.":"off.");
  printf("\n");
  printf("\nESC ESC to exit");
  printf("\nESC o   for ON");
  printf("\nESC m   for MODE");
  printf("\nArrow keys should work");
  printf("\n(Uses VT-102 codes)");
  printf("\n");
  printf("\n========================================");
  printf("\n");
  
}




////////////////////////////////////////////////////////////////////////////////

// Serial loop command structure

typedef void (*SERIAL_FPTR)(void);

typedef struct
{
  char key;
  char *desc;
  SERIAL_FPTR fn;
} SERIAL_COMMAND;

SERIAL_COMMAND serial_cmds[] =
  {
   {
    '?',
    "Serial command help",
    serial_help,
   },
   {
    '!',
    "Boot to mass storage",
    cli_boot_mass,
   },
   {
    '0',
    "*Digit",
    cli_digit,
   },
   {
    '1',
    "*Digit",
    cli_digit,
   },
   {
    '2',
    "*Digit",
    cli_digit,
   },
   {
    '3',
    "*Digit",
    cli_digit,
   },
   {
    '4',
    "*Digit",
    cli_digit,
   },
   {
    '5',
    "*Digit",
    cli_digit,
   },
   {
    '6',
    "*Digit",
    cli_digit,
   },
   {
    '7',
    "*Digit",
    cli_digit,
   },
   {
    '8',
    "*Digit",
    cli_digit,
   },
   {
    '9',
    "*Digit",
    cli_digit,
   },
   {
    'a',
    "*Digit",
    cli_digit,
   },
   {
    'b',
    "*Digit",
    cli_digit,
   },
   {
    'c',
    "*Digit",
    cli_digit,
   },
   {
    'd',
    "*Digit",
    cli_digit,
   },
   {
    'e',
    "*Digit",
    cli_digit,
   },
   {
    'f',
    "*Digit",
    cli_digit,
   },
   {
    'M',
    "Dump Memory",
    cli_dump_memory,
   },
   {
    'z',
    "Zero Parameter",
    cli_zero_parameter,
   },
   {
    'i',
    "Information",
    cli_info,
   },
   {
    'T',
    "Trace To Dump",
    cli_trace_dump_to,
   },
   {
    'F',
    "Trace From Dump",
    cli_trace_dump_from,
   },
   {
    'L',
    "Dump Language Stack",
    cli_dump_language_stack,
   },
   {
    '_',
    "Dump EEPROM",
    cli_dump_eeprom,
   },
   {
    '.',
    "Write to EEPROM",
    cli_write_eeprom,
   },
   {
    't',
    "Terminal",
    cli_terminal,
   },
   {
    'g',
    "Graphics Terminal",
    cli_graphics_terminal,
   },
   {
    '*',
    "Format",
    cli_format,
   },
   {
    '+',
    "Catalog",
    cli_catalog,
   },
   {
    'C',
    "Create File",
    cli_create,
   },
   
  };

int pcount = 0;
int periodic_read = 0;

#define SL_STATE_INIT 0
#define SL_STATE_ESC1 1
#define SL_STATE_ESC2 2
#define SL_STATE_ESC3 3

void queue_key(int key)
{
  // Add key to queue
  kb_external_key = key;
  
  // Wait for it to be processed
  while( kb_external_key != KEY_NONE )
    {
      menu_loop_tasks();
    }
}

//------------------------------------------------------------------------------

#define MAX_GET_STRING 64

char serial_get_string_buffer[MAX_GET_STRING+1];

char *serial_get_string(void)
{
  int done = 0;
  char key_str[2] = " ";
  int key;
  
  serial_get_string_buffer[0] = '\0';
  
  while(!done)
    {

      if( ((key = getchar_timeout_us(100)) != PICO_ERROR_TIMEOUT))
	{
	  switch(key)
	    {
	    case 13:
	    case 27:
	      done = 1;
	      break;

	    default:
	      printf("%c", key);
	      if( strlen(serial_get_string_buffer) < MAX_GET_STRING )
		{
		  key_str[0] = key;
		  strcat(serial_get_string_buffer, key_str);
		}
	      break;
	    }
	}
    }
  
  return(serial_get_string_buffer);
}

//------------------------------------------------------------------------------
int sl_state = SL_STATE_INIT;

void serial_loop()
{
  int key = KEY_NONE;

  
  int key_queue[3];
  
  if( serial_terminal_mode )
    {
      // Display
  
      if( ((key = getchar_timeout_us(100)) != PICO_ERROR_TIMEOUT))
	{
	  switch(sl_state)
	    {
	    case SL_STATE_INIT:
	      if( key == 27 )
		{
		  sl_state = SL_STATE_ESC1;
		  key_queue[0] = key;
		}
	      else
		{
		  queue_key(key);		  
		}
	      break;

	      
	    case SL_STATE_ESC1:
	      
	      // We have an ESC, see if we may have a control key
	      switch(key)
		{
		case 27:
		  // Exit terminal mode
		  serial_terminal_mode = 0;
		  sl_state = SL_STATE_INIT;
		  printf("%c[0;0r", 27);
		  printf("\nterminal mode off");
		  break;

		  // ON key
		case 'o':
		  // Exit terminal mode

		  queue_key(KEY_ON);		  
		  sl_state = SL_STATE_INIT;
		  break;

		  // MODE key
		case 'm':
		  // Exit terminal mode

		  queue_key(KEY_MODE);		  
		  sl_state = SL_STATE_INIT;
		  break;
	      
		case 91:
		  // Could be a longer sequence
		  key_queue[1] = key;
		  sl_state = SL_STATE_ESC2;
		  break;

		default:
		  // not a sequene, send the previous and this key
		  queue_key(key_queue[0]);
		  queue_key(key);
		  sl_state = SL_STATE_INIT;
		  break;
		}
	      
	      break;

	    case SL_STATE_ESC2:
	      
	      // We have an ESC, see if we may have a control key
	      switch(key)
		{
		case 65:
		  // We have control code, convert
		  queue_key(KEY_UP);
		  sl_state = SL_STATE_INIT;
		  break;

		case 66:
		  // We have control code, convert
		  queue_key(KEY_DOWN);
		  sl_state = SL_STATE_INIT;
		  break;
		  
		case 67:
		  // We have control code, convert
		  queue_key(KEY_RIGHT);
		  sl_state = SL_STATE_INIT;
		  break;
		  
		case 68:
		  // We have control code, convert
		  queue_key(KEY_LEFT);
		  sl_state = SL_STATE_INIT;
		  break;

		default:
		  // not a sequene, send the previous and this key
		  queue_key(key_queue[0]);
		  queue_key(key_queue[1]);
		  queue_key(key);
		  sl_state = SL_STATE_INIT;
		  break;
		}
	      
	      break;
	    }
	}
    }
  else
    {
      if( ((key = getchar_timeout_us(100)) != PICO_ERROR_TIMEOUT))
	{
	  for(int i=0; i<sizeof(serial_cmds)/sizeof(SERIAL_COMMAND);i++)
	    {
	      if( serial_cmds[i].key == key )
		{
		  
		  keypress = key;
		  (*serial_cmds[i].fn)();
		  prompt();
		  break;
		}
	    }
	}
    }
}

void serial_help(void)
{
  printf("\n");
  
  for(int i=0; i<sizeof(serial_cmds)/sizeof(SERIAL_COMMAND);i++)
    {
      if( *(serial_cmds[i].desc) != '*' )
	{
	  printf("\n%c:   %s", serial_cmds[i].key, serial_cmds[i].desc);
	}
    }
  
  printf("\nEnter 0-9,A-F for hex parameter");  
  printf("\n");
}


void prompt(void)
{
  printf("\nP:%08X >", parameter);
}

////////////////////////////////////////////////////////////////////////////////
//
// Display a character at a cursor position
// CUP	ESC [ Pl ; Pc H

void serial_display_xy(int x, int y, char ch)
{
  if( serial_terminal_mode )
    {
      printf("%c7", 27);
      printf("%c[%d;%dH%c", 27, y+2, x+2, ch);
      printf("%c8", 27);
    }
}

void serial_plot_point_byte(int x, int y, uint8_t byte)
{
  if( serial_terminal_mode )
    {
      printf("%c7", 27);

      printf("%c[%d;%dH%c", 27, y+2+5, x+0, byte?'@':' ');
      printf("%c8", 27);
    }
  
}
