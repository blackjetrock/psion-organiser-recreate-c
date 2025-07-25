////////////////////////////////////////////////////////////////////////////////
//
// Serial EEPROM flash services
//
// provides the read and save services that allow paks to be
// located on the Pico flash
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate_all.h"

#define SERIAL_EEPROM_SIZE (1024*64)

//------------------------------------------------------------------------------
// Read a byte from the flash pak

uint8_t pk_rbyt_serial_eeprom(PAK_ADDR pak_addr)
{
  uint8_t byte;
  
  read_eeprom(EEPROM_1_ADDR_RD, pak_addr, 1, (uint8_t *)&byte);
  
  return(byte);
}

//------------------------------------------------------------------------------
//
// Write a block of data to the pak
//
// The pak needs to be pre-erased, that is done during
// formatting, so all we do here is write the data.


void pk_save_serial_eeprom(PAK_ADDR pak_addr, int len, uint8_t *src)
{
  write_eeprom(EEPROM_1_ADDR_WR, pkw_cpad, len, src);
}

//------------------------------------------------------------------------------
//
// Format the serial eeprom pak
//
// We 0xFF the header
//

void pk_format_serial_eeprom(void)
{
  uint8_t blank_hdr[10] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  
  write_eeprom(EEPROM_1_ADDR_WR, 0, SERIAL_EEPROM_SIZE, blank_hdr);
}


