#include <assert.h>
#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint64_t u_int64_t;
typedef unsigned int uint;

//#include "hardware/adc.h"
#include "hardware/clocks.h" 
#include "hardware/structs/xip_ctrl.h"
#include "pico/aon_timer.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <pico/platform.h>

#include "psion_recreate.h"

//#include "nos.h"
//#include "newopl_types.h"
//#include "newopl.h"
//#include "nopl_obj.h"
#include "nopl.h"

#include "newopl_lib.h"

#define LVAD(XXX) printf("\n%s:%p\n", __FUNCTION__, &XXX-lvadp)

#include "emulator.h"
#include "wireless.h"
#include "serial.h"

#include "ssd1309.h"
#include "ssd1351b.h"
#include "display_driver.h"

#include "eeprom.h"
#include "rtc.h"
#include "display.h"
#include "record.h"
#include "font.h"
#include "cursor.h"
#include "cdc.h"
#include "core1.h"
#include "panel.h"
#include "svc.h"
//#include "opl.h"

#include "menu.h"
#include "switches.h"

#include "sysvar.h"

//#include "sdcard.h"
#include "f_util.h"
#include "hw_config.h"
//#include "sd_card.h"
#include "diskio.h" /* Declarations of disk functions */
#include "sd_card_utils.h"

#define PSRAM __attribute__((section(".psram.")))

extern char line[MAX_NOPL_LINE+1];
extern int *lvadp;
