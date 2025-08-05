
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
//#include "hardware/i2c.h"
//#include "hardware/pio.h"
//#include "hardware/clocks.h"
#include "hardware/uart.h"

#include "psion_recreate_all.h"

//#include "psion_recreate.h"
//#include "emulator.h"

//#include "rtc.h"

////////////////////////////////////////////////////////////////////////////////
//
// RTC support for MCP7940

#define ADDR_MCP7940    0xdf
#define MCP_READ_ADDR   (ADDR_MCP7940  | 0x01)
#define MCP_WRITE_ADDR  (ADDR_MCP7940 & 0xFE)

// Changed to use a field based system to avoid overwriting other setting when
// adjusting things

typedef struct _MCP7940_FIELD
{
  int reg;
  int mask;
} MCP7940_FIELD;

// The register fields

MCP7940_FIELD mcp7940_fields[] =
  {
    {0x00, 0x80},    // START
    {0x03, 0x08},    // VBATEN
    {0x03, 0x07},    // WKDAY
  };

#define NUM_FIELDS (sizeof(mcp7940_fields)/sizeof(MCP7940_FIELD))

enum {
  MCP7940_F_START = 0,
  MCP7940_F_VBATEN,
  MCP7940_F_WKDAY,
};

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
  i2c_send_bytes(I2C_BUS_RTC, MCP_WRITE_ADDR, 1, sb);  
  i2c_read_bytes(I2C_BUS_RTC, MCP_READ_ADDR, 1, &b);
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
  i2c_send_bytes(I2C_BUS_RTC, MCP_WRITE_ADDR, 2, data);
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

////////////////////////////////////////////////////////////////////////////////

#define FIELDMASK(FIELDN) mcp7940_fields[FIELDN].mask
#define FIELDREG(FIELDN) mcp7940_fields[FIELDN].reg

int find_shifts(int fieldnum)
{
  int mask = FIELDMASK(fieldnum);
  int shift = 0;

while( (mask & 1) == 0 )
  {
    shift++;
    mask >>= 1;
  }

return(shift);
}

//
// Read and write a field
//

int rtc_read_field(int fieldnum)
{
  int shifts = find_shifts(fieldnum);
  
  // Read the register then mask and shift the data
  return( (read_mcp7940(FIELDREG(fieldnum)) & FIELDMASK(fieldnum)) >> shifts );
}

// Writing is a read, modify write operation

void rtc_write_field(int fieldnum, int value)
{
  int regval = read_mcp7940(FIELDREG(fieldnum));
  int shifts = find_shifts(fieldnum);

  // Shift and mask the data
  int data = (value << shifts) & FIELDMASK(fieldnum);

  // Mask out the field in the register value
  regval &= ~(FIELDMASK(fieldnum));
  
  // Add the field data in
  regval |= data;

  // Write to the register
  write_mcp7940(FIELDREG(fieldnum), regval);
}

////////////////////////////////////////////////////////////////////////////////

// Sets the VBATEN bit

void set_vbaten_bit()
{
  BYTE reg0;

  rtc_write_field(MCP7940_F_VBATEN, 1);
#if 0  
  reg0 = read_mcp7940(MCP_RTCWKDAY_REG);
  reg0 |= MCP_VBATEN_MASK;

  write_mcp7940(MCP_RTCWKDAY_REG, reg0);
#endif
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
  rtc_write_field(MCP7940_F_START, 1);
#if 0
  BYTE reg0;

  reg0 = read_mcp7940(MCP_RTCSEC_REG);
  reg0 |= MCP_ST_MASK;

  write_mcp7940(MCP_RTCSEC_REG, reg0);
#endif
  
}

// Clears the ST bit
void clr_st_bit(void)
{
  rtc_write_field(MCP7940_F_START, 0);
#if 0
  BYTE reg0;

  reg0 = read_mcp7940(MCP_RTCSEC_REG);
  reg0 &= ~MCP_ST_MASK;

  write_mcp7940(MCP_RTCSEC_REG, reg0);
#endif

}

// Clear the MCP_OUT bit

void clear_out_bit(void)
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

void rtc_dump(void)
{
  char line[10];
  
  // Dump RTC registers
  printf("\nRTC Dump\n");
    
  for(int i=0; i<0x60; i++)
    {
      if( (i % 16) == 0 )
	{
	  printf("\n%04X: ", i);
	}

      BYTE reg = read_mcp7940(i);
      printf("%02X ", reg);
    }
  printf("\n");
}

////////////////////////////////////////////////////////////////////////////////
//
// Read date and time from RTC
//

uint8_t timedate[20];
uint8_t memdata[64];

// Reads first set of registers into timedate[]
// These are timekeeping registers

void read_rtc(void)
{
  for(int i=0; i<9; i++)
    {
      BYTE reg = read_mcp7940(i);
      timedate[i] = reg;      
    }
}

void rtc_read_mem(void)
{
  for(int i=0x20; i<0x60; i++)
    {
      BYTE reg = read_mcp7940(i);
      memdata[i-0x20] = reg;      
    }
}

void rtc_write_mem(void)
{
  for(int i=0x20; i<0x60; i++)
    {
      write_mcp7940(i, memdata[i-0x20]);
    }
}

char rtc_buffer[40];

char *wdayn[8] =
  {
    "???",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    "Sun",
  };

char *rtc_get_time(void)
{
  int seconds, minutes, hours;
  int day, month, year;
  int wday;
  
  read_rtc();
  
  seconds = ((timedate[0] & 0x70) >> 4)* 10 + (timedate[0] & 0xf);
  minutes = ((timedate[1] & 0x70) >> 4)* 10 + (timedate[1] & 0xf);
  hours   = ((timedate[2] & 0x70) >> 4)* 10 + (timedate[2] & 0xf);
  wday    = ((timedate[3] & 0x00) >> 4)* 10 + (timedate[3] & 0x7);
  day     = ((timedate[4] & 0x30) >> 4)* 10 + (timedate[4] & 0xf);
  month   = ((timedate[5] & 0x30) >> 4)* 10 + (timedate[5] & 0xf);
  year    = ((timedate[6] & 0xF0) >> 4)* 10 + (timedate[6] & 0xf);

  sprintf(rtc_buffer, "%02d:%02d:%02d %02d/%02d/%02d %s",hours, minutes, seconds, day, month, year, wdayn[wday]);
  return(rtc_buffer);
}

void rtc_set_seconds(int s)
{
  write_mcp7940( MCP_RTCSEC_REG, s | MCP_ST_MASK);
}

void rtc_set_minutes(int m)
{
  write_mcp7940( MCP_RTCMIN_REG, m);
}

void rtc_set_hours(int h)
{
  write_mcp7940( MCP_RTCHOUR_REG, h);
}

int rtc_get_seconds(void)
{
  return(read_mcp7940(MCP_RTCSEC_REG));
}

int rtc_get_minutes(void)
{
  return(read_mcp7940(MCP_RTCMIN_REG));
}

int rtc_get_hours(void)
{
  return(read_mcp7940(MCP_RTCHOUR_REG));
}

void rtc_set_wday(int s)
{
  rtc_write_field(MCP7940_F_WKDAY, s);
  
  //write_mcp7940( MCP_RTCWKDAY_REG, s | MCP_ST_MASK);
}

void rtc_set_day(int s)
{
  write_mcp7940( MCP_RTCDATE_REG, s | MCP_ST_MASK);
}

void rtc_set_month(int m)
{
  write_mcp7940( MCP_RTCMTH_REG, m);
}

void rtc_set_year(int h)
{
  write_mcp7940( MCP_RTCYEAR_REG, h);
}

int rtc_get_wday(void)
{
  return(read_mcp7940(MCP_RTCWKDAY_REG));
}

int rtc_get_day(void)
{
  return(read_mcp7940(MCP_RTCDATE_REG));
}

int rtc_get_month(void)
{
  return(read_mcp7940(MCP_RTCMTH_REG));
}

int rtc_get_year(void)
{
  return(read_mcp7940(MCP_RTCYEAR_REG));
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
