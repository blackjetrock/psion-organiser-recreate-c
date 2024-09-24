

////////////////////////////////////////////////////////////////////////////////
//
// OPL
//
// This is the embedding of OPL (in this case NewOPL) into the
// recreation. It's here to avoid the newopl files from having any recreation
// code dependencies. That should make it easier to build a PC based
// version from the NewOpl files.
//
//
////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"

#include "psion_recreate_all.h"


////////////////////////////////////////////////////////////////////////////////


NOBJ_MACHINE opl_machine;
