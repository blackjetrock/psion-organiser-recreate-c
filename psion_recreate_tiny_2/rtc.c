
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
    {0x00, 0x7F},    // SECONDS
    {0x01, 0x7F},    // MINUTES
    {0x02, 0x3F},    // HOURS
    {0x04, 0x3F},    // DAY
    {0x05, 0x1F},    // MONTH
    {0x06, 0xFF},    // YEAR
    {0x03, 0x10},    // PWRFAIL
    {0x18, 0x7F},    // PDMINUTES
    {0x19, 0x3F},    // PDHOURS
    {0x1A, 0x3F},    // PDDAY
    {0x1B, 0x1F},    // PDMONTH
    {0x1B, 0xE0},    // PDWKDAY
    {0x1C, 0x7F},    // PUMINUTES
    {0x1D, 0x3F},    // PUHOURS
    {0x1E, 0x3F},    // PUDAY
    {0x1F, 0x1F},    // PUMONTH
    {0x1F, 0xE0},    // PUWKDAY
  };

#define NUM_FIELDS (sizeof(mcp7940_fields)/sizeof(MCP7940_FIELD))

enum {
  MCP7940_F_START = 0,
  MCP7940_F_VBATEN,
  MCP7940_F_WKDAY,
  MCP7940_F_SECONDS,
  MCP7940_F_MINUTES,
  MCP7940_F_HOURS,
  MCP7940_F_DAY,
  MCP7940_F_MONTH,
  MCP7940_F_YEAR,
  MCP7940_F_PWRFAIL,
  MCP7940_F_PDMINUTES,
  MCP7940_F_PDHOURS,
  MCP7940_F_PDDAY,
  MCP7940_F_PDMONTH,
  MCP7940_F_PDWKDAY,
  MCP7940_F_PUMINUTES,
  MCP7940_F_PUHOURS,
  MCP7940_F_PUDAY,
  MCP7940_F_PUMONTH,
  MCP7940_F_PUWKDAY,
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

void set_vbaten_bit(void)
{
  rtc_write_field(MCP7940_F_VBATEN, 1);
}

void clr_pwrfail_bit(void)
{
  rtc_write_field(MCP7940_F_PWRFAIL, 0);
}

// Sets the ALMPOL 0 bit
void set_almpol0_bit(void)
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

uint8_t timedate[MCP_LAST_BYTE+1];
uint8_t memdata[64];

// Reads first set of registers into timedate[]
// These are timekeeping registers

void read_rtc(void)
{
  for(int i=0; i<=MCP_LAST_BYTE; i++)
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

char *rtc_get_pdtime(void)
{
  int minutes, hours;
  int day, month;
  int wday;
  
  read_rtc();

  minutes = rtc_read_field(MCP7940_F_PDMINUTES);
  hours   = rtc_read_field(MCP7940_F_PDHOURS);
  wday    = rtc_read_field(MCP7940_F_PDWKDAY);
  day     = rtc_read_field(MCP7940_F_PDDAY);
  month   = rtc_read_field(MCP7940_F_PDMONTH);
  
  sprintf(rtc_buffer, "%02X:%02X %02X/%02X %s", hours, minutes,  day, month, wdayn[wday]);
  return(rtc_buffer);
}

char *rtc_get_putime(void)
{
  int minutes, hours;
  int day, month;
  int wday;
  
  read_rtc();

  minutes = rtc_read_field(MCP7940_F_PUMINUTES);
  hours   = rtc_read_field(MCP7940_F_PUHOURS);
  wday    = rtc_read_field(MCP7940_F_PUWKDAY);
  day     = rtc_read_field(MCP7940_F_PUDAY);
  month   = rtc_read_field(MCP7940_F_PUMONTH);
  
  sprintf(rtc_buffer, "%02X:%02X %02X/%02X %s", hours, minutes,  day, month, wdayn[wday]);
  return(rtc_buffer);
}

char *rtc_get_time(void)
{
  int seconds, minutes, hours;
  int day, month, year;
  int wday;
  
  read_rtc();

  seconds = rtc_read_field(MCP7940_F_SECONDS);
  minutes = rtc_read_field(MCP7940_F_MINUTES);
  hours   = rtc_read_field(MCP7940_F_HOURS);
  wday    = rtc_read_field(MCP7940_F_WKDAY);
  day     = rtc_read_field(MCP7940_F_DAY);
  month   = rtc_read_field(MCP7940_F_MONTH);
  year    = rtc_read_field(MCP7940_F_YEAR);
  
  sprintf(rtc_buffer, "%02X:%02X:%02X %02X/%02X/%02X %s", hours, minutes, seconds, day, month, year, wdayn[wday]);
  return(rtc_buffer);
}

void rtc_set_seconds(int s)
{
  rtc_write_field(MCP7940_F_SECONDS, s);
}

void rtc_set_minutes(int m)
{
  rtc_write_field(MCP7940_F_MINUTES, m);
}

void rtc_set_hours(int h)
{
  rtc_write_field(MCP7940_F_HOURS, h);
}

int rtc_get_seconds(void)
{
  return(rtc_read_field(MCP7940_F_SECONDS));
}

int rtc_get_minutes(void)
{
  return(rtc_read_field(MCP7940_F_MINUTES));
}

int rtc_get_hours(void)
{
  return(rtc_read_field(MCP7940_F_HOURS));
}

void rtc_set_wday(int s)
{
  rtc_write_field(MCP7940_F_WKDAY, s);
}

void rtc_set_day(int s)
{
  rtc_write_field(MCP7940_F_DAY, s);
}

void rtc_set_month(int m)
{
  rtc_write_field(MCP7940_F_MONTH, m);
}

void rtc_set_year(int h)
{
  rtc_write_field(MCP7940_F_YEAR, h);
}

int rtc_get_wday(void)
{
  return(rtc_read_field(MCP7940_F_WKDAY));
}

int rtc_get_day(void)
{
  return(rtc_read_field(MCP7940_F_DAY));
}

int rtc_get_month(void)
{
  return(rtc_read_field(MCP7940_F_MONTH));
}

int rtc_get_year(void)
{
  return(rtc_read_field(MCP7940_F_YEAR));
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
