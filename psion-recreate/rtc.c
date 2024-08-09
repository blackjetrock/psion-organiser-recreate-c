
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
//#include "hardware/i2c.h"
//#include "hardware/pio.h"
//#include "hardware/clocks.h"
#include "hardware/uart.h"

#include "psion_recreate.h"
#include "emulator.h"

#include "rtc.h"

////////////////////////////////////////////////////////////////////////////////
//
// RTC support for MCP7940

#define ADDR_MCP7940    0xdf
#define MCP_READ_ADDR   (ADDR_MCP7940  | 0x01)
#define MCP_WRITE_ADDR  (ADDR_MCP7940 & 0xFE)



////////////////////////////////////////////////////////////////////////////////
//
//
// Read a register from the RTC
// 

int read_mcp7940(int r)
{
  BYTE b;
  BYTE sb[1] = {r};
  
#if NEW_I2C
  i2c_send_bytes(MCP_WRITE_ADDR, 1, sb);  
  i2c_read_bytes(MCP_READ_ADDR, 1, &b);
#else
  
  Start();
  
  SentByte(MCP_WRITE_ADDR);
  SentByte(r);
  Stop();
  
  Start();
  SentByte(MCP_READ_ADDR);  
  b = ReceiveByte();
  Stop();
#endif
  
  return(b);
}

// Write a register to the RTC
void write_mcp7940(int r, BYTE value)
{
#if NEW_I2C
  BYTE data[2] = {r, value};
  data[1] = value;
  i2c_send_bytes(MCP_WRITE_ADDR, 2, data);
#else
  Start();

  SentByte(MCP_WRITE_ADDR);
  SentByte(r);

  for(int i=0; i<n; i++)
    {
      SentByte(data[i]);      
    }

  Stop();
#endif
}

// Sets the VBATEN bit
void set_vbaten_bit()
{
  BYTE reg0;

  reg0 = read_mcp7940(MCP_RTCWKDAY_REG);
  reg0 |= MCP_VBATEN_MASK;

  write_mcp7940(MCP_RTCWKDAY_REG, reg0);
}

// Sets the ALMPOL 0 bit
void set_almpol0_bit()
{
  BYTE reg0;

  reg0 = read_mcp7940(MCP_ALM0WKDAY_REG);
  reg0 |= MCP_ALMPOL_MASK;

  write_mcp7940(MCP_ALM0WKDAY_REG, reg0);
}

void set_almpol1_bit()
{
  BYTE reg0;

  reg0 = read_mcp7940(MCP_ALM1WKDAY_REG);
  reg0 |= MCP_ALMPOL_MASK;

  write_mcp7940(MCP_ALM1WKDAY_REG, reg0);
}

// Sets the ST bit
void set_st_bit()
{
  BYTE reg0;

  reg0 = read_mcp7940(MCP_RTCSEC_REG);
  reg0 |= MCP_ST_MASK;

  write_mcp7940(MCP_RTCSEC_REG, reg0);

}

// Clear the MCP_OUT bit

void clear_out_bit()
{
  BYTE reg0;

  reg0 = read_mcp7940(MCP_RTCC_CONTROL_REG);
  reg0 &= ~MCP_OUT_MASK;

  write_mcp7940(MCP_RTCC_CONTROL_REG, reg0);

}

// Set the MCP_OUT bit

void set_out_bit()
{
  BYTE reg0;

  reg0 = read_mcp7940(MCP_RTCC_CONTROL_REG);
  reg0 |= MCP_OUT_MASK;

  write_mcp7940(MCP_RTCC_CONTROL_REG, reg0);

}


////////////////////////////////////////////////////////////////////////////////
//
//  RTC tasks concerning the I2C bus are done here.
// 

int read_seconds = 0;
int read_minutes = 0;
int read_hours = 0;
int rtc_set_st = 0;
int rtc_set_variables = 0;
int rtc_set_registers = 0;

int rtc_seconds = 0;
int rtc_minutes = 0;
int rtc_hours = 0;

void rtc_tasks(void)
{
  if( rtc_set_st )
    {
      // Set the ST bit
      set_st_bit();

      // Also set the alarm polarity
      set_almpol0_bit();
      set_almpol0_bit();

      // And set the OUT bit
      clear_out_bit();
      
      rtc_set_st = 0;
    }
  
  if( read_seconds )
    {
      rtc_seconds = read_mcp7940( MCP_RTCSEC_REG) & 0x7f;
      read_seconds = 0;
    }

  if( read_minutes )
    {
      rtc_minutes = read_mcp7940( MCP_RTCMIN_REG);
      read_minutes = 0;
    }

  if( read_hours )
    {
      rtc_hours = read_mcp7940( MCP_RTCHOUR_REG);
      read_hours = 0;
    }

  // Update the Psion organiser time variables with the time from the RTC
  if( rtc_set_variables )
    {
      ramdata[TMB_SECS] = read_mcp7940( MCP_RTCSEC_REG) & 0x7f;
      ramdata[TMB_MINS] = read_mcp7940( MCP_RTCMIN_REG);
      ramdata[TMB_HOUR] = read_mcp7940( MCP_RTCHOUR_REG);
      rtc_set_variables = 0;
    }

  // Set the RTC registers to the current time variables
  if( rtc_set_registers )
    {
      write_mcp7940( MCP_RTCSEC_REG, ramdata[TMB_SECS]);
      write_mcp7940( MCP_RTCMIN_REG, ramdata[TMB_MINS]);
      write_mcp7940( MCP_RTCHOUR_REG, ramdata[TMB_HOUR]);
      rtc_set_registers = 0;
    }
}
