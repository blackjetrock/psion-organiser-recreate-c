////////////////////////////////////////////////////////////////////////////////
//
// A shell
//
// Usable on the USB serial port and also on the OLED display and KB
//
////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"

#include "psion_recreate_all.h"

void cli_interactive(void)
{
  char *cmd;
  int rect;
  
  interactive_done = 0;
  printf("\nInteractive mode.\n");
  
  while(!interactive_done)
    {
      tight_loop_tasks();
      
      printf("\n%c: RECT:%d RECNO:%d ADD:%08X LVADP:%p>", 'A'+pkb_curp, flb_rect, flw_crec, pkw_cpad, lvadp);
      
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
