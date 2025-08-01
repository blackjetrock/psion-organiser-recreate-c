#include <stdio.h>

#include "psion_recreate_all.h"

#include "svc_kb.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "switches.h"


// .----------------------------------------------------------.
// | Core 1 code                                              |
// `----------------------------------------------------------'

volatile int core1_ticks = 0;

volatile int core1_in_ram = 0;
volatile int core1_goto_ram = 0;
volatile int core1_exit_ram = 0;

#define LED 25

void __not_in_flash_func(core1_ram_wait)(void)
{
#if DB_CORE1
  printf("\n%s:", __FUNCTION__);
#endif

  core1_in_ram = 1;
  core1_exit_ram = 0;
  core1_goto_ram = 0;
  while( !core1_exit_ram )
    {
    }

  core1_in_ram = 0;
}

void put_core1_into_ram(void)
{
#if CORE1_UNUSED
#else
  
#if DB_CORE1
  printf("\n%s:", __FUNCTION__);
#endif

  core1_goto_ram = 1;

  while( !core1_in_ram )
    {
    }
#endif
}

void take_core1_out_of_ram(void)
{
#if DB_CORE1
  printf("\n%s:", __FUNCTION__);
#endif
  
  core1_exit_ram = 1;
}

void __not_in_flash_func(core1_code)(void)
{
  //multicore_lockout_victim_init();

#if DB_CORE1
  printf("\n%s:", __FUNCTION__);
#endif

  while (true) {

    if( core1_goto_ram )
      {
#if DB_CORE1
  printf("\n%s:Entering RAM", __FUNCTION__);
#endif

	core1_ram_wait();
      }
      
      
    core1_ticks++;

    if( (core1_ticks % 2000) == 0 )
      {
        matrix_scan();
      }
  }
}

// .----------------------------------------------------------.
// | Core 1 launcher                                          |
// `----------------------------------------------------------'

void core1_init(void)
{
  multicore_launch_core1(core1_code);
}

// .----------------------------------------------------------.
// | Core 1 tick detector                                     |
// `----------------------------------------------------------'

uint32_t last_count = 0;

bool core1_tick(void)
{
  if (core1_ticks == last_count)
    {
      return false;
    }
  else
    {
      last_count = core1_ticks;
      return true;
    }
}
