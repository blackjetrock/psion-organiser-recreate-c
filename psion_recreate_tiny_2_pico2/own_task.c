
// **************************************************************************
// *                                                                        *
// *    Hooks into TinyUSB                                                  *
// *                                                                        *
// **************************************************************************

#include "tusb.h"

static int32_t any(int32_t itf) {
  if (itf >= 0 && itf < CFG_TUD_CDC) {
    if (tud_cdc_n_connected(itf)) {
      return tud_cdc_n_available(itf);
    }
  }
  return 0;
}

static int32_t rxbyte(int32_t itf) {
  // Calling 'any(itf)' checks we are connected and 'itf' is in range
  // so we don't need to explicitly check either here.
  if (any(itf)) {
    uint8_t c;
    tud_cdc_n_read(itf, &c, 1);
    return c;
  }
  return -1;
}

void txbyte(int32_t itf, int32_t txb) {
  if (itf >= 0 && itf < CFG_TUD_CDC) {
    if (tud_cdc_n_connected(itf)) {
      uint8_t c = txb & 0xFF;
      while (tud_cdc_n_write_available(itf) < 1) {
	tud_task();
	tud_cdc_n_write_flush(itf);
      }
      tud_cdc_n_write(itf, &c, 1);
      tud_cdc_n_write_flush(itf);
    }
  }
}

static void txtext(int32_t itf, const char *ptr) {
  if (itf >= 0 && itf < CFG_TUD_CDC) {
    if (tud_cdc_n_connected(itf)) {
      size_t len = strlen(ptr);
      while (len > 0) {
	size_t available = tud_cdc_n_write_available(itf);
	if (available == 0) {
	  // No free space so let's try and create some
	  tud_task();
	} else if (len > available) {
	  // Too much to send in one go so we send as much as we can
	  size_t sent = tud_cdc_n_write(itf, ptr, available);
	  ptr += sent;
	  len -= sent;
	} else {
	  // We have space to send it so send it all
	  tud_cdc_n_write(itf, ptr, len);
	  len = 0;
	}
	tud_cdc_n_write_flush(itf);
      }
    }
  }
}

// **************************************************************************
// *                                                                        *
// *    Our own task							    *
// *                                                                        *
// **************************************************************************

void own_task(void) {
  int32_t c;
  // Echo 'dev/ttyACM1' to '/dev/ttyACM2'
  while ((c = rxbyte(1)) >= 0)
    {
      txbyte(2, c);
      if (c == 0x0D)
	{
	  txbyte(2, 0x0A);
	}
    }
  // Echo 'dev/ttyACM2' to '/dev/ttyACM1'
  while ((c = rxbyte(2)) >= 0)
    {
      txbyte(1, c);
      if (c == 0x0D)
	{
	  txbyte(1, 0x0A);
	}
    }
}
