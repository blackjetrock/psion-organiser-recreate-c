////////////////////////////////////////////////////////////////////////////////
//
//
// EEPROM accessed as a block of pages
//
// We use the EEPROM as a paged data store. The eeprom is implemented
// as having pages. We store one ecord of data in each one.
//
// This is a quick and simple version of the organiser files.
// It gives us a quick way to implement FIND and SAVE
//
//
////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"

#include "psion_recreate.h"
#include "eeprom.h"
#include "i2c.h"


void write_page(int n, uint8_t *data)
{
  write_eeprom(EEPROM_0_ADDR_WR , n, PAGE_SIZE, data);
}
