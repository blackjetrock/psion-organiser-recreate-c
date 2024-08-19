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

#include "psion_recreate.h"

#include "menu.h"
#include "emulator.h"
#include "eeprom.h"
#include "rtc.h"
#include "display.h"
#include "record.h"
#include "svc.h"

// Current pak
PAK      pkb_curp = PAKNONE;
PAK_ADDR pkw_cpad = 0;

PAK_ID pkt_id = {0,0,0,0,0,0,0,0,0,0};

PK_DRIVER_SET pk_drivers[] =
  {
   {pk_rbyt_pico_flash,     pk_save_pico_flash,      pk_format_pico_flash},
   {pk_rbyt_serial_eeprom,  pk_save_serial_eeprom,   pk_format_serial_eeprom},
  };


void pk_setp(PAK pak)
{
  // Set up current pak
  pkb_curp = pak;
  pkw_cpad = 0;
  
  // We now switch the drivers to point to the appropriate hardware
  
  // Check the header and see if it is valid
  
  
}

void pk_save(int pak_addr, int len, uint8_t *src)
{
  (*pk_drivers[pkb_curp].save)(pak_addr, len, src);
}

////////////////////////////////////////////////////////////////////////////////
//
// Read bytes from pak

void pk_read(PAK_ADDR pak_addr, int len, uint8_t *dest)
{
  for(int i=0; i<len; i++)
    {
      pk_rbyt(pak_addr+i);
    }


  
}

uint8_t pk_rbyt(PAK_ADDR pak_addr)
{
  // Branch to the pak drivers
  pkw_cpad++;
  return( (*pk_drivers[pkb_curp].rbyt)(pak_addr));
}


uint16_t pk_rwrd(PAK_ADDR pak_addr)
{
  uint16_t word = 0;

  word  = pk_rbyt(pak_addr) << 8;
  word |= pk_rbyt(pak_addr+1);
  
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


