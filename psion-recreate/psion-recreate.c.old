#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

////////////////////////////////////////////////////////////////////////////////
//
// GPIOs
//
//
////////////////////////////////////////////////////////////////////////////////

const uint PIN_SDAOUT     = 14;
const uint PIN_LATCHOUT2  = 15;
const uint PIN_I2C_SDA    = 16;
const uint PIN_I2C_SCL    = 17;
const uint PIN_LS_DIR     = 18;
const uint PIN_LATCHIN    = 19;
const uint PIN_SCLKIN     = 20;
const uint PIN_SDAIN      = 21;
const uint PIN_LATCHOUT1  = 22;
const uint PIN_SCLKOUT    = 26;
const uint PIN_VBAT_SW_ON = 27;


////////////////////////////////////////////////////////////////////////////////
//
// Write an 8 bit pattern to a 595 latch
//
////////////////////////////////////////////////////////////////////////////////

void write_595(const uint latchpin, int value)
{
  // Latch pin low
  gpio_put(latchpin, 1);
  
  for(int i = 0; i<8; i++)
    {
      // Clock low
      gpio_put(PIN_SCLKIN, 0);
      
      // Set data up
      if( value & 0x80 )
	{
	  gpio_put(PIN_SDAIN, 1);
	}
      else
	{
	  gpio_put(PIN_SDAIN, 0);
	}

      // Clock data in
      gpio_put(PIN_SCLKIN, 1);
    }

  // Now latch value (update outputs)
  gpio_put(latchpin, 1);

}


////////////////////////////////////////////////////////////////////////////////

void oledmain(void);

int main() {

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  
  // Set up the GPIOs
  gpio_init(PIN_VBAT_SW_ON);
  gpio_set_dir(VBAT_SW_ON, GPIO_OUT);

  // Take power on high so we latch on
  gpio_put(PIN_VBAT_SW_ON, 1);

  // Set GPIOs up
  gpio_init(PIN_SDAOUT);
  gpio_init(PIN_LATCHOUT2);
  gpio_init(PIN_LATCHOUT1);
  gpio_init(PIN_SDAIN);
  gpio_init(PIN_SCLKOUT);

  gpio_set_dir(PIN_SDAOUT,    GPIO_OUT);
  gpio_set_dir(PIN_LATCHOUT2, GPIO_OUT);
  gpio_set_dir(PIN_LATCHOUT1, GPIO_OUT);
  gpio_set_dir(PIN_SDAIN,     GPIO_IN);
  gpio_set_dir(PIN_SCLKOUT,   GPIO_OUT);


  // Set up and run OLED code
  oledmain();
  
  bi_decl(bi_program_description("This is a test binary."));
  bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
  stdio_init_all();
#if 0
  while (1) {
    gpio_put(LED_PIN, 0);
    sleep_ms(250);
    gpio_put(LED_PIN, 1);
    puts("Hello World\n");
    sleep_ms(250);
  }
#endif

#if 0
  #if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c/lcd_1602_i2c example requires a board with I2C pins
#else
    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
#endif

    // Main loop
    while(1)
      {
	
      }
    
}

