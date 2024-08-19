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
PAK pkb_curp = PAKNONE;

void pk_setp(PAK pak)
{
}

void pk_save(void)
{
}

void pk_read(void)
{
}

void pk_rbyt(void)
{
}

void pk_rwrd(void)
{
}

void pk_skip(void)
{
}

void pk_qadd(void)
{
}

void pk_sadd(void)
{
}

void pk_pkof(void)
{
}


