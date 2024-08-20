#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate_all.h"

// Current pak
PAK          pkb_curp = PAKNONE;
PAK_ADDR     pkw_cpad = 0;
FL_REC_TYPE  flb_rect = 0;
