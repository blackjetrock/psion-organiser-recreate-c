//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate_all.h"



#define DB_BYTE_WIDTH 16

void db_dump(uint8_t *mem, int len)
{
  char ascii[DB_BYTE_WIDTH*3+5];
  char ascii_byte[5];
  
  // Display memory from address
  printf("\n");

  ascii[0] = '\0';
  
  for(unsigned int z = 0; z<len; z++)
    {
      int byte = 0;
      
      if( (z % DB_BYTE_WIDTH) == 0)
	{
	  printf("  %s", ascii);
	  ascii[0] = '\0';
	  printf("\n%08X: ", z);
	}

      byte = *(mem++);

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
