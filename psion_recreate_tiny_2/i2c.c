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

#include "psion_recreate_all.h"

////////////////////////////////////////////////////////////////////////////////
//
// The delay for the i2c routines. This can be altered before accessing a
// device
//
////////////////////////////////////////////////////////////////////////////////

int i2c_delay_value = I2C_DELAY;

////////////////////////////////////////////////////////////////////////////////

I2C_BUS i2c_bus[2] =
  {
    {PIN_I2C_SCL,     PIN_I2C_SDA},
    {PIN_I2C_RTC_SCL, PIN_I2C_RTC_SDA},
  };

////////////////////////////////////////////////////////////////////////////////
//
// I2C functions
//
////////////////////////////////////////////////////////////////////////////////

void i2c_set_delay_value(int delay)
{
  i2c_delay_value = delay;
}

// Release the bus
void i2c_release(I2C_BUS_ID id)
{
  // All inputs
  gpio_set_dir(i2c_bus[id].pin_sda, GPIO_OUT);
  gpio_set_dir(i2c_bus[id].pin_scl, GPIO_OUT);
  gpio_put(i2c_bus[id].pin_sda, 0);
  gpio_put(i2c_bus[id].pin_scl, 0);
}

// Delay to slow down to I2C bus rates
// sleep_us does not work well here.

void i2c_delay(I2C_BUS_ID id)
{
  volatile int i;

  // This is the smallest delay that works with the recreate I2C OLED
  // panel.
  
  for(i=0; i<3; i++)
    {
    }
}

void i2c_sda_low(I2C_BUS_ID id)
{
  // Take SCL low by driving a 0 on to the bus
  gpio_set_dir(i2c_bus[id].pin_sda, GPIO_OUT);
  gpio_put(i2c_bus[id].pin_sda, 0);
}

void i2c_sda_high(I2C_BUS_ID id)
{
  // Make sure bit is an input
  gpio_set_dir(i2c_bus[id].pin_sda,GPIO_IN);
}

void i2c_scl_low(I2C_BUS_ID id)
{
  gpio_set_dir(i2c_bus[id].pin_scl,GPIO_OUT);
  gpio_put(i2c_bus[id].pin_scl, 0);
}

void i2c_scl_high(I2C_BUS_ID id)
{
  // Make sure bit is an input
  gpio_set_dir(i2c_bus[id].pin_scl, GPIO_IN);
}

// Read ACK bit

int i2c_read_sda(I2C_BUS_ID id)
{
  return(gpio_get(i2c_bus[id].pin_sda));
}

// I2C start condition

void i2c_start(I2C_BUS_ID id)
{
  i2c_sda_low(id);
  i2c_delay(id);
  i2c_scl_low(id);
  i2c_delay(id);
}

void i2c_stop(I2C_BUS_ID id)
{
  i2c_sda_low(id);
  i2c_delay(id);
  i2c_scl_high(id);
  i2c_delay(id);
  i2c_sda_high(id);
  i2c_delay(id);
}

// Send 8 bits and read ACK
// Returns number of acks received

int i2c_send_byte(I2C_BUS_ID id, BYTE b)
{
  int i;
  int ack=0;
  int retries = 2;
  int rc =1;

  for (i = 0; i < 8; i++)
    {
      // Set up data
      if ((b & 0x80) == 0x80)
	{
	  i2c_sda_high(id);
	}
      else
	{
	  i2c_sda_low(id);
	}

      // Delay
      i2c_delay(id);

      // Take clock high and then low
      i2c_scl_high(id);

      // Delay
      i2c_delay(id);

      // clock low again
      i2c_scl_low(id);

      // Delay
      i2c_delay(id);

      // Shift next data bit in
      b <<= 1;
    }

  // release data line
  i2c_sda_high(id);

  // Now get ACK
  i2c_scl_high(id);

  i2c_delay(id);

  // read ACK

  while( ack = i2c_read_sda(id) ) // @suppress("Assignment in condition")
    {
      retries--;

      if ( retries == 0 )
	{
	  rc = 0;
	  //printf("\nTimeout");
	  break;
	}
    }

  i2c_scl_low(id);

  i2c_delay(id);
  return (rc);
}


// Receive 8 bits and set ACK
// Ack as specified
void i2c_recv_byte(I2C_BUS_ID id, BYTE *data, int ack)
{
  int i, b;

  b = 0;

  // Make data an input
  i2c_sda_high(id);

  for (i = 0; i < 8; i++)
    {
      // Delay
      i2c_delay(id);

      // Take clock high and then low
      i2c_scl_high(id);

      // Delay
      i2c_delay(id);

      // Shift next data bit in
      b <<= 1;
      b += (i2c_read_sda(id) & 0x1);

      // clock low again
      i2c_scl_low(id);

      // Delay
      i2c_delay(id);

    }

  // ACK is as we are told 
  if ( ack )
    {
      // Data low for ACK
      i2c_sda_low(id);
    }
  else
    {
      i2c_sda_high(id);
    }

  // Now send ACK
  i2c_scl_high(id);

  i2c_delay(id);

  i2c_scl_low(id);

  i2c_delay(id);

  *data = b;

}

// Reads a block of bytes from a slave

int i2c_read_bytes(I2C_BUS_ID id, BYTE slave_addr, int n, BYTE *data)
{
  int i;
  BYTE byte;

  i2c_start(id);

  // Send slave address with read bit
  if ( !i2c_send_byte(id, slave_addr ) )
    {
      i2c_stop(id);
      return(0);
    }

  //
  for (i = 0; i < n; i++)
    {
      i2c_recv_byte(id, &byte, (i==(n-1))? 0 : 1);
      *(data++) = byte;
    }

  i2c_stop(id);

  return(1);
}

// Sends a block of data to I2C slave
void i2c_send_bytes(I2C_BUS_ID id, BYTE slave_addr, int n, BYTE *data)
{
  int i;

  i2c_start(id);

  // Send slave address with read bit
  i2c_send_byte(id, slave_addr);

  //
  for (i = 0; i < n; i++)
    {
      i2c_send_byte(id, *(data++));
    }

  i2c_stop(id);
}

void i2c_fn_initialise(I2C_BUS_ID id)
{
  gpio_init(i2c_bus[id].pin_sda);
  gpio_init(i2c_bus[id].pin_scl);

  i2c_stop(id);
}

