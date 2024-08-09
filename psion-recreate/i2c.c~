////////////////////////////////////////////////////////////////////////////////
//
// Functions that provide I2C communication
//
//
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"

#include "psion_recreate.h"

// The delay for th ei2c routines. This can be altered before accessing a device
//

//int i2c_delay_value = I2C_DELAY;

////////////////////////////////////////////////////////////////////////////////
//
// I2C functions
//
////////////////////////////////////////////////////////////////////////////////

//void i2c_fn_set_delay(int delay)
//{
//  i2c_delay_value = delay;
//}

// Release the bus
void i2c_release(void)
{
  // All inputs
  gpio_set_dir(PIN_I2C_SDA, GPIO_OUT);
  gpio_set_dir(PIN_I2C_SCL, GPIO_OUT);
  gpio_put(PIN_I2C_SDA, 0);
  gpio_put(PIN_I2C_SCL, 0);
}

// Delay to slow down to I2C bus rates
void i2c_delay(void)
{
#if 0  
  sleep_us(1);
  return;
#else
  volatile int i;

  for(i=0; i<20; i++)
    {
    }
#endif
}

void i2c_sda_low(void)
{
  // Take SCL low by driving a 0 on to the bus
  gpio_set_dir(PIN_I2C_SDA, GPIO_OUT);
  gpio_put(PIN_I2C_SDA, 0);
}

void i2c_sda_high(void)
{
  // Make sure bit is an input
  gpio_set_dir(PIN_I2C_SDA,GPIO_IN);
}

void i2c_scl_low(void)
{
  gpio_set_dir(PIN_I2C_SCL,GPIO_OUT);
  gpio_put(PIN_I2C_SCL, 0);
}

void i2c_scl_high(void)
{
  // Make sure bit is an input
  gpio_set_dir(PIN_I2C_SCL, GPIO_IN);
}

// Read ACK bit

int i2c_read_sda(void)
{
  return(gpio_get(PIN_I2C_SDA));
}

// I2C start condition

void i2c_start(void)
{
  i2c_sda_low();

  i2c_delay();
  i2c_scl_low();
  i2c_delay();
}

void i2c_stop(void)
{
  i2c_sda_low();
  i2c_delay();
  i2c_scl_high();
  i2c_delay();
  i2c_sda_high();
  i2c_delay();

}

// Send 8 bits and read ACK
// Returns number of acks received

int i2c_send_byte(BYTE b)
{
  int i;
  int ack=0;
  int retries = 100;
  int rc =1;

  for (i = 0; i < 8; i++)
    {
      // Set up data
      if ((b & 0x80) == 0x80)
	{
	  i2c_sda_high();
	}
      else
	{
	  i2c_sda_low();
	}

      // Delay
      i2c_delay();

      // Take clock high and then low
      i2c_scl_high();

      // Delay
      i2c_delay();

      // clock low again
      i2c_scl_low();

      // Delay
      i2c_delay();

      // Shift next data bit in
      b <<= 1;
    }

  // release data line
  i2c_sda_high();

  // Now get ACK
  i2c_scl_high();

  i2c_delay();

  // read ACK

  while( ack = i2c_read_sda() ) // @suppress("Assignment in condition")
    {
      retries--;

      if ( retries == 0 )
	{
	  rc = 0;
	  break;
	}
    }

  i2c_scl_low();

  i2c_delay();
  return (rc);
}


// Receive 8 bits and set ACK
// Ack as specified
void i2c_recv_byte(BYTE *data, int ack)
{
  int i, b;

  b = 0;

  // Make data an input
  i2c_sda_high();

  for (i = 0; i < 8; i++)
    {
      // Delay
      i2c_delay();

      // Take clock high and then low
      i2c_scl_high();

      // Delay
      i2c_delay();

      // Shift next data bit in
      b <<= 1;
      b += (i2c_read_sda() & 0x1);

      // clock low again
      i2c_scl_low();

      // Delay
      i2c_delay();

    }

  // ACK is as we are told 
  if ( ack )
    {
      // Data low for ACK
      i2c_sda_low();
    }
  else
    {
      i2c_sda_high();
    }

  // Now send ACK
  i2c_scl_high();

  i2c_delay();

  i2c_scl_low();

  i2c_delay();

  *data = b;

}

// Reads a block of bytes from a slave

int i2c_read_bytes(BYTE slave_addr, int n, BYTE *data)
{
  int i;
  BYTE byte;

  i2c_start();

  // Send slave address with read bit
  if ( !i2c_send_byte(slave_addr ) )
    {
      i2c_stop();
      return(0);
    }


  //
  for (i = 0; i < n; i++)
    {
      i2c_recv_byte( &byte, (i==(n-1))? 0 : 1);
      *(data++) = byte;
    }

  i2c_stop();

  return(1);
}

// Sends a block of data to I2C slave
void i2c_send_bytes(BYTE slave_addr, int n, BYTE *data)
{
  int i;

  i2c_start();

  // Send slave address with read bit
  i2c_send_byte(slave_addr);

  //
  for (i = 0; i < n; i++)
    {
      i2c_send_byte(*(data++));
    }

  i2c_stop();
}

void i2c_fn_initialise(void)
{
  gpio_init(PIN_I2C_SDA);
  gpio_init(PIN_I2C_SCL);
}
