////////////////////////////////////////////////////////////////////////////////
// 
// Pack Services
//

// Packs are different to the original, but the same format is used as
// it works well in RAM chips and flash.
//
// A:Pico flash
// B:Recreation serial EEPROM
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate_all.h"

PAK_ID pkt_id = {0,0,0,0,0,0,0,0,0,0};

PK_DRIVER_SET pk_drivers[] =
  {
   {pk_rbyt_pico_flash,     pk_save_pico_flash,      pk_format_pico_flash},
   {pk_rbyt_serial_eeprom,  pk_save_serial_eeprom,   pk_format_serial_eeprom},
  };

////////////////////////////////////////////////////////////////////////////////
//
// Build a pak ID string
//

void pk_build_id_string(PAK_ID result,
			int size,
			int year,
			int month,
			int day,
			int hour,
			int unique1,
			int unique2)
{
  PAK_ID id;

  id[0] = 0x78;
  id[1] = size;
  id[2] = year;
  id[3] = month;
  id[4] = day;
  id[5] = hour;
  id[6] = unique1;
  id[7] = unique2;

  result = id;
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void pk_setp(PAK pak)
{
  // Set up current pak
  pkb_curp = pak;
  pkw_cpad = 0;
  
  // We now switch the drivers to point to the appropriate hardware
  
  // Check the header and see if it is valid
#if DB_PK_SETP
  printf("\n%s:", __FUNCTION__);
#endif
  
}

void pk_save(int len, uint8_t *src)
{
  printf("\n%s:%08X %d", __FUNCTION__, pkw_cpad, len);
  (*pk_drivers[pkb_curp].save)(pkw_cpad, len, src);

  // Update the current address
  pkw_cpad += len;
}

////////////////////////////////////////////////////////////////////////////////
//
// Read bytes from pak

void pk_read(int len, uint8_t *dest)
{
  for(int i=0; i<len; i++)
    {
      *(dest++) = pk_rbyt();
    }
}

uint8_t pk_rbyt(void)
{
  // Branch to the pak drivers
  return( (*pk_drivers[pkb_curp].rbyt)(pkw_cpad++));
}


uint16_t pk_rwrd(void)
{
  uint16_t word = 0;

  word  = pk_rbyt() << 8;
  word |= pk_rbyt();
  
}

void pk_skip(int len)
{
  // Not needed as we have no address counters
  pkw_cpad += len;
}

int pk_qadd(void)
{
  return(pkw_cpad); 
}

void pk_sadd(int addr)
{
  pkw_cpad = addr;
}

void pk_pkof(void)
{
  // Do nothing for now
}

void      pk_fmat(void)
{
  // Branch to the pak drivers
  return( (*pk_drivers[pkb_curp].format)());
}
