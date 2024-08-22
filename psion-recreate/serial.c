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

#include "psion_recreate_all.h"

int keypress = 0;
int parameter = 0;
unsigned int address = 0;

void serial_help(void);
void prompt(void);
void cli_interactive(void);

int interactive_done = 0;

////////////////////////////////////////////////////////////////////////////////
//
  
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

// Another digit pressed, update the parameter variable
void cli_digit(void)
{
  unsigned int  n = 0;

#if DB_DIGIT
  printf("\nK:%02X", keypress);
  printf("\nn:%02X", n);
#endif
  if( keypress > '9' )
    {
      n = keypress - 'a' + 10;
    }
  else
    {
      n = keypress - '0';
    }

#if DB_DIGIT
  printf("\nn:%02X", n);
#endif
  
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
  pk_fmat();
  printf("\nDone...");
  
}

void cli_catalog(void)
{
  int rc = 0;
  uint8_t rectype;
  int bytes_free;
  int num_recs;
  PAK_ADDR first_free;
  
  char filename[32];
  printf("\nCatalog of %c:\n", 'A'+pkb_curp);

  fl_size(&bytes_free, &num_recs, &first_free);

  printf("\n%d bytes free, first free byte at %08X", bytes_free, first_free);

       fl_catl(FL_OP_OPEN, pkb_curp, filename, &rectype);
  rc = fl_catl(FL_OP_FIRST, pkb_curp, filename, &rectype);

  while(rc)
    {
      fl_rect(rectype);
      fl_size(&bytes_free, &num_recs, &first_free);
      
      printf("\n%-8s    $%02X  Records:%d", filename, rectype, num_recs);
      rc = fl_catl(FL_OP_NEXT, pkb_curp, filename, &rectype);
    }

  fl_catl(FL_OP_CLOSE, pkb_curp, filename, &rectype);
}

void cli_create(void)
{
  int rc = 0;
  uint8_t rectype;
  char *filename;
  
  printf("\nCreate file on A:\n");
  printf("\nEnter filename:");
  filename = serial_get_string();
  
  fl_cret(filename, 0);
  
  printf("\nDone...");
  
}

void cli_query(void)
{
  printf("\n");
  printf("\nCurrent Pak:%d Address:%04X", pkb_curp, pk_qadd());
  printf("\n");

  
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
   {
    'q',
    "Query",
    cli_query,
   },
   {
    'i',
    "Interactive Mode",
    cli_interactive,
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

#define MAX_GET_STRING 128

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

////////////////////////////////////////////////////////////////////////////////
//
// Command based interactive interface
//

void ic_test(char *str, char *fmt)
{
  printf("\nTest command, '%s'",str);
}

void ic_exit(char *str, char *fmt)
{
  interactive_done = 1;
}

void ic_catalog(char *str, char *fmt)
{
  cli_catalog();
}

void ic_createfile(char *str, char *fmt)
{
  char filename[20];
  sscanf(str, fmt, filename);

  fl_cret(filename, 0);

}

void ic_createmain(char *str, char *fmt)
{
  fl_cret("MAIN", 0x90);
}

void ic_boot_mass(char *str, char *fmt)
{
  reset_usb_boot(0,0);
}

void ic_format(char *str, char *fmt)
{
  printf("\nFormatting %d", pkb_curp);
  pk_fmat();
}

void ic_erase(char *str, char *fmt)
{
  printf("\nErasing %d", flw_crec);
  fl_eras();
}

void ic_dump(char *str, char *fmt)
{
  parameter = 0x10180000;
  cli_dump_memory();
}


void ic_write(char *str, char *fmt)
{
  char s[254];
  int rt;

  sscanf(str, "write %d %[^@]", &rt, s);
  
  printf("\nWriting '%s' to rec type %d", s, rt);
  fl_rect(rt);
  fl_writ(s, strlen(s));
}

void ic_read(char *str, char *fmt)
{
  char s[254];
  int recno;
  int rect;
  
  sscanf(str, "read", &recno, &rect);
  
  printf("\nReading record %d from rec type %d", flw_crec, flb_rect);

  fl_read(s);
}

void ic_find(char *str, char *fmt)
{
  char s[254];
  char srch[64];
  int len;

  srch[0] = '\0';
  
  sscanf(str, fmt, &srch);
  
  while( fl_find(srch, s, &len) )
    {
      printf("\n%d (%04X): '%s'", flw_crec, pk_qadd(), s);
      fl_next();
    }
}


void ic_device(char *str, char *fmt)
{
  int device;

  sscanf(str, fmt, &device);

  pk_setp(device);
  
  printf("\nDevice now %d", pkb_curp);

}

void ic_recno(char *str, char *fmt)
{
  int recno;

  sscanf(str,  fmt, &recno);

  fl_rset(recno);
}

void ic_next(char *str, char *fmt)
{
  fl_next();
}

void ic_back(char *str, char *fmt)
{
  fl_back();
}

void ic_rect(char *str, char *fmt)
{
  int rect;
  FL_REC_TYPE rt;
  
  sscanf(str, fmt, &rect);

  rt = rect;
  fl_rect(rt);
}


void ic_flfrec(char *str, char *fmt)
{
  PAK_ADDR pak_addr;
  int len;
  int device = 0;
  int n = 1;
  int  rectype = 20;
  FL_REC_TYPE rt;
  int rc = 0;
  
#if DB_FL_FREC
  printf("\n%s:\n", __FUNCTION__);
#endif
    
  sscanf(str, fmt,  &device, &rectype, &n);
  rt = rectype;

  fl_rect(rt);
  
#if DB_FL_FREC
  printf("\nDev:%d rectype:%d n:%d\n", device, rectype, n);
#endif
  
  pk_setp(device);

  rc = fl_frec(n, &pak_addr, &rt, &len);

  if( rc )
    {
      printf("\nrectype: %d", rectype);
      printf("\npak_addr:%04X", pak_addr);
      printf("\nlen:     %d", len);
      
      printf("\nDevice now %d", pkb_curp);
    }
  else
    {
      printf("\nRecord not found");
    }

}

void ic_help(char *str, char *fmt);

typedef void (*CMD_FN)(char *str, char *fmt);
  
struct _IC_CMD
{
  char *cmd;
  char *fmt;
  CMD_FN fn;  
} ic_cmds[] =
  {
   {"help",       "",                ic_help},
   {"dev",        "dev %d",          ic_device},
   {"catalog",    "",                ic_catalog},
   {"createfile", "createfile %s",   ic_createfile},
   {"createmain", "createmain %s",   ic_createmain},
   {"flfrec",     "flfrec %d %d %d", ic_flfrec},
   {"format",     "",                ic_format},
   {"test",       "",                ic_test},
   {"dump",       "",                ic_dump},
   {"erase",       "",               ic_erase},
   {"write",      "write %d %[^@]",  ic_write},
   {"read",       "",                ic_read},
   {"recno",      "recno %d",        ic_recno},
   {"rect",       "rect %d",         ic_rect},
   {"find",       "find %s",         ic_find},
   {"find",       "",                ic_find},

   {"next",       "",                ic_next},
   {"back",       "",                ic_back},
   {"exit",       "",                ic_exit},
   {"!",          "",                ic_boot_mass},
   {"r",          "r %d",            ic_recno},
  };

#define NUM_IC_CMD (sizeof(ic_cmds)/sizeof(struct _IC_CMD))

void ic_help(char *str, char *fmt)
{
  printf("\n");
  
  for(int i=0; i<NUM_IC_CMD; i++)
    {
      printf("\n%-10s %s", ic_cmds[i].cmd, ic_cmds[i].fmt);
    }
  
  printf("\n");
}

void cli_interactive(void)
{
  char *cmd;
  int rect;
  
  interactive_done = 0;
  
  while(!interactive_done)
    {
      printf("\n%c: RECT:%d RECNO:%d ADD:%08X>", 'A'+pkb_curp, flb_rect, flw_crec, pkw_cpad);
      
      cmd = serial_get_string();
      
      for(int i=0; i<NUM_IC_CMD; i++)
	{
	  //	  printf("\ntest '%s' '%s'",cmd , ic_cmds[i].cmd);
	  if( strncmp(ic_cmds[i].cmd, cmd, strlen(ic_cmds[i].cmd)) == 0 )
	    {
	      (*ic_cmds[i].fn)(cmd, ic_cmds[i].fmt);
	      break;
	    }
	}
    }
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
