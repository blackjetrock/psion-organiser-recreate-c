

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

#include <stdarg.h>

static int32_t any(int32_t itf)
{
  if (itf >= 0 && itf < CFG_TUD_CDC)
    {
      if (tud_cdc_n_connected(itf))
	{
	  return tud_cdc_n_available(itf);
        }
    }
  return 0;
}

static int32_t rxbyte(int32_t itf)
{
  // Calling 'any(itf)' checks we are connected and 'itf' is in range
  // so we don't need to explicitly check either here.
  if (any(itf))
    {
      uint8_t c;
      tud_cdc_n_read(itf, &c, 1);
      return c;
    }
  return -1;
}

void txbyte(int32_t itf, int32_t txb)
{
  if (itf >= 0 && itf < CFG_TUD_CDC)
    {
      if (tud_cdc_n_connected(itf))
	{
	  uint8_t c = txb & 0xFF;
	  while (tud_cdc_n_write_available(itf) < 1)
	    {
	      tud_task();
	      tud_cdc_n_write_flush(itf);
            }
	  tud_cdc_n_write(itf, &c, 1);
	  tud_cdc_n_write_flush(itf);
        }
    }
}

static void txtext(int32_t itf, const char *ptr)
{
  if (itf >= 0 && itf < CFG_TUD_CDC)
    {
      if (tud_cdc_n_connected(itf))
	{
	  size_t len = strlen(ptr);
	  while (len > 0)
	    {
	      size_t available = tud_cdc_n_write_available(itf);
	      if (available == 0)
		{
		  // No free space so let's try and create some
		  tud_task();
                }
	      else if (len > available)
		{
		  // Too much to send in one go so we send as much as we can
		  size_t sent = tud_cdc_n_write(itf, ptr, available);
		  ptr += sent;
		  len -= sent;
                }
	      else
		{
		  // We have space to send it so send it all
		  tud_cdc_n_write(itf, ptr, len);
		  len = 0;
                }
	      tud_cdc_n_write_flush(itf);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void cdc_printf(int itf, const char *format, ...)
{
  va_list args;
  va_start(args, format);
    
  char buffer[CDC_MAX_LINE];
  
  vsprintf(buffer, format, args);
  va_end(args);
  
  tud_cdc_n_write(itf, buffer, strlen(buffer));
  tud_cdc_n_write_flush(itf);
  
}


void cdc_printf_xy(int itf, int x, int y, const char *format, ...)
{
  va_list args;
  va_start(args, format);
    
  char buffer[CDC_MAX_LINE];

  cdc_printf(itf, "%c7", 27);
  cdc_printf(itf, "%c[%d;%dH", 27, y+2, x+2);
  
  vsprintf(buffer, format, args);
  cdc_printf(itf, buffer);
  
  va_end(args);

  cdc_printf(itf, "%c8", 27);
}


void cdc_cls(int itf)
{
  // Clear screen
  cdc_printf(itf, "%c[2J", 27);
}
