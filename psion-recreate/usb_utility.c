
#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"

//#include "macropad.h"
//#include "keyboard.h"
#include "serial.h"
#include "i2c.h"
//#include "led.h"
#include "sleep.h"

#include "psion_recreate_all.h"



// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
#if 0
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance)
{
  (void) instance;
  return desc_hid_report;
}
#endif

void init_usb(void)
{
  //    tusb_init();
      board_init();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

}

//------------------------------------------------------------------------------
//
// Called every 10ms
//

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  int key_to_send;
  
  // skip if hid is not ready yet
  if ( !tud_hid_ready() )
    return;
  
  switch(report_id)
    {
    case REPORT_ID_KEYBOARD:
#if 1
      if( (key_to_send = unqueue_hid_key()) != 0)
	{
	  // Keycode could have a modifier in upper bits
	  uint8_t modifier = (key_to_send >> 8);
	  uint8_t k = (key_to_send & 0xFF);
	  
	  uint8_t keycode[6] = { 0 };
	  keycode[0] = k;
	  printf("\nk:%d %02X mod:%d %02X", k, k, modifier, modifier);
	  
	  tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, keycode);
	}
      else
	{
#endif
	  tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
	}
      
      break;

    case REPORT_ID_MOUSE:
      {
	int8_t const delta = 5;

	// no button, right + down, no scroll, no pan
	tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
      }
      break;

    case REPORT_ID_CONSUMER_CONTROL:
      {
	// use to avoid send multiple consecutive zero report
	static bool has_consumer_key = false;

	if ( btn )
	  {
	    // volume down
	    uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
	    tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
	    has_consumer_key = true;
	  }
	else
	  {
	    // send empty key report (release key) if previously has key pressed
	    uint16_t empty_key = 0;
	    if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
	    has_consumer_key = false;
	  }
      }
      break;

    case REPORT_ID_GAMEPAD:
      {
	// use to avoid send multiple consecutive zero report for keyboard
	static bool has_gamepad_key = false;

	hid_gamepad_report_t report =
	  {
	    .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
	    .hat = 0, .buttons = 0
	  };

	if ( btn )
	  {
	    report.hat = GAMEPAD_HAT_UP;
	    report.buttons = GAMEPAD_BUTTON_A;
	    tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

	    has_gamepad_key = true;
	  }
	else
	  {
	    report.hat = GAMEPAD_HAT_CENTERED;
	    report.buttons = 0;
	    if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
	    has_gamepad_key = false;
	  }
      }
      break;

    default: break;
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
//
////////////////////////////////////////////////////////////////////////////////


void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() )
    {
      // Wake up host if we are in suspend mode
      // and REMOTE_WAKEUP feature is enabled by host
      tud_remote_wakeup();
    }
  else
    {
      // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
      send_hid_report(REPORT_ID_KEYBOARD, btn);
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
    {
      send_hid_report(next_report_id, board_button_read());
    }
}


// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  (void)instance;
  
  if (report_type == HID_REPORT_TYPE_OUTPUT)
    {
      // Set keyboard LED e.g Capslock, Numlock etc...
      if (report_id == REPORT_ID_KEYBOARD)
        {
	  // bufsize should be (at least) 1
	  if (bufsize < 1)
	    {
	      return;
	    }
	  
	  uint8_t const kbd_leds = buffer[0];
	  
	}
    }
}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus

void tud_suspend_cb(bool remote_wakeup_en)
{
  (void)remote_wakeup_en;
  //  gpio_put(PIN_SDB, 0);
  

}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  //gpio_put(PIN_SDB, 1);
}

