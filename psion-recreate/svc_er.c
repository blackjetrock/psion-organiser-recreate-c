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

// error

void er_error(char *errstr)
{
  printf("\n**** %s *****\n", errstr);
}

char *er_lkup(ER_ERRORCODE e)
{
}


void er_mess(ER_ERRORCODE e)
{
}
