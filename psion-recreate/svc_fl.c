//

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

#include "svc_fl.h"

////////////////////////////////////////////////////////////////////////////////

// Device already open when accessed
// first is 1 for first call, 0 thereafter
// return value is 1 if record found, 0 if not
// 
int fl_scan_pack(int first, int device, uint8_t *dest)
{
  uint8_t  length_byte;
  uint8_t  record_type;
  uint16_t block_length;
  
  if(first)
    {
      pk_sadd(0x0a);
    }

  length_byte = pk_rbyt();
  record_type = pk_rbyt();

  if( length_byte == 0 )
    {
      return(ER_FL_NP);
    }

  if( record_type == 0x80 )
    {
      // Long record
      block_length = pk_rwrd();
      pk_skip(block_length);
    }
  else
    {
      if( record_type != 0xFF )
	{
	  // Copy short record
	  for(int i=0; i<length_byte; i++)
	    {
	      *(dest++) = pk_rbyt();	      
	    }
	  
	  return(1);
	}
    }

  if( length_byte == 0xFF )
    {
      return(0);
    }
  else
    {
      printf("\n%s:exit ?", __FUNCTION__);
      return(0);
    }
}

////////////////////////////////////////////////////////////////////////////////

void fl_back(void)
{
}

void fl_bcat(void)
{
}

void fl_bdel(void)
{
}

void fl_bopn(void)
{
}

void fl_bsav(void)
{
}

// Returns filenames in filename,
// return code is 1 if file found, 0 if not

int fl_catl(int first, int device, char *filename, uint8_t *rectype)
{
  int rc = 0;
  uint8_t record_data[256];
  
  // Access device
  pk_setp(device);

  // Scan the records looking for files
  rc = fl_scan_pack(1, device, record_data);

  while(rc == 1)
    {
      // Check record to see if it is a file
      // record type is 0x81

      if( record_data[0] == 0x09 )
	{
	  if( record_data[1] == 0x81 )
	    {
	      // It is a file.
	      *rectype = record_data[10];
	      for(int i=2; i<10; i++)
		{
		  *(filename++) = record_data[i];
		}

	      return(1);
	    }
	}
      
      rc = fl_scan_pack(0, device, record_data);
    }

  return(0);
  
}

void fl_copy(void)
{
}

FL_REC_TYPE fl_cret(char *filename)
{
}

void fl_deln(void)
{
}

void fl_eras(void)
{
}

void fl_ffnd(void)
{
}

void fl_find(void)
{
}

void fl_frec(void)
{
}

void fl_next(void)
{
}

void fl_open(void)
{
}

void fl_pars(void)
{
}

void fl_read(void)
{
}

void fl_rect(void)
{
}

void fl_renm(void)
{
}

void fl_rset(void)
{
}

void fl_setp(void)
{
}

void fl_size(void)
{
}

void fl_writ(void)
{
}

void tl_cpyx(void)
{

}
