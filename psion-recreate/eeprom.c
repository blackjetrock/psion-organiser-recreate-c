////////////////////////////////////////////////////////////////////////////////
//
// Serial EEPROM
//
////////////////////////////////////////////////////////////////////////////////
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"

#include "psion_recreate.h"
#include "eeprom.h"
#include "i2c.h"

// Read a block of data from the EEPROM
// 
// Returns:
// 1 for success
// 0 for failure
//

int read_eeprom(int slave_addr, int start, int len, BYTE *dest)
{
  BYTE a[2];

  i2c_set_delay_value(I2C_DELAY_EEPROM);
  
  a[0] = start >> 8;
  a[1] = start & 0xFF;
  
  // Set up the write address

  i2c_send_bytes(slave_addr & 0xFE, 2, a);

  return(i2c_read_bytes(slave_addr | 0x01, len, dest));
}

int write_eeprom(int slave_addr, int start, int len, BYTE *src)
{
  BYTE a[2];
  int i;

  i2c_set_delay_value(I2C_DELAY_EEPROM);
  
  // Force slave address to have correct RDWR bit
  slave_addr &= 0xFE;
  
  a[0] = start >> 8;
  a[1] = start & 0xFF;

  i2c_start();

  // Send slave address with write bit
  i2c_send_byte(slave_addr);

  // Address
  i2c_send_byte(a[0]);
  i2c_send_byte(a[1]);

  // Write the data
    for (i = 0; i < len; i++)
    {
      i2c_send_byte(*(src++));
    }

  i2c_stop();

  // Delay for the eeprom write time
  sleep_ms(4);
}


////////////////////////////////////////////////////////////////////////////////
//
// test mode
//
#define TEST_LEN 4
#define TEST_START 0x1000

void eeprom_test(void)
{
  BYTE data[TEST_LEN];
  int i;
  int j;

  //DEBUG_STOP;
  while(1)
    {
      dump_lcd();
      
      j++;
      
      for(i=0; i<TEST_LEN; i++)
	{
	  data[i] = 10+i+j*5;
	}
      
      printxy_str(0,0,"EEPROM Test");
      
      // Write some bytes to an eeprom, then read it back
      write_eeprom(EEPROM_1_ADDR_WR, TEST_START, TEST_LEN, data);

      sleep_ms(1000);
      for(i=0; i<TEST_LEN; i++)
	{
	  printxy_hex(i*3, 1, data[i]);
	}
      
      for(i=0; i<TEST_LEN; i++)
	{
	  data[i] = 0xAA;
	}
      
      read_eeprom(EEPROM_1_ADDR_RD, TEST_START, TEST_LEN, data);
      
      for(i=0; i<TEST_LEN; i++)
	{
	  printxy_hex(i*3, 2, data[i]);
	}

      sleep_ms(1000);
     }
}


// This test writes 128 bytes and reads it back then checks it
#define TEST2_LEN  128

void eeprom_test2(void)
{
  BYTE write_data[TEST2_LEN];
  BYTE read_data[TEST2_LEN];
  
  int i;
  int j;

  printxy_str(1, 0, "EEPROM Test");
  dump_lcd();
  
  //DEBUG_STOP;

  // Set up test data
  for(int i=0; i<TEST2_LEN; i++)
    {
      write_data[i] = 10+i;
    }

  // Write some bytes to an eeprom
  write_eeprom(EEPROM_1_ADDR_WR, TEST_START, TEST2_LEN, write_data);

  sleep_ms(100);

  // read it back
  read_eeprom(EEPROM_1_ADDR_RD, TEST_START, TEST2_LEN, read_data);
  

  // Compare it
  for(int i=0; i<TEST2_LEN; i++)
    {
      if( write_data[i] == read_data[i] )
	{
	  // All ok
	}
      else
	{
	  // Bad
	  printxy_str(1,1,"Bad");
	  dump_lcd();
	  DEBUG_STOP;
	}
    }
  
  printxy_str(1,1,"Good");
  dump_lcd();
  DEBUG_STOP;
}


// This test writes RAM to EEPROM and then checks it has written correctly

void eeprom_test3(void)
{
  BYTE buffer[PAGE_SIZE];
  
  printxy_str(1, 0, "EEPROM Dmp Test");
  dump_lcd();
  
  //DEBUG_STOP;

  // repeat test N times
  for(int k = 0; k< 10; k++)
    {
      // Run twice with different data each time
      for(int p=0; p<2; p++)
	{
	  
	  printxy_str(1, 1, "Setting");
	  dump_lcd();
	  
	  for(int i = 0; i<RAM_SIZE; i++)
	    {
	      ramdata[i] = i*23*p;
	    }
	  
	  printxy_str(1, 1, "Writing");
	  dump_lcd();
	  
	  // Write in 128 byte pages to the eeprom
	  for(int i=0; i<RAM_SIZE; i+=PAGE_SIZE)
	    {
	      // Write the page to EEPROM
	      write_eeprom(EEPROM_0_ADDR_WR , i, PAGE_SIZE, &(ramdata[i]));
	      sleep_us(10);
	    }
#if 0
	  printxy_str(1, 1, "Wiping ");
	  dump_lcd();
	  
	  for(int i = 0; i<RAM_SIZE; i++)
	    {
	      ramdata[i] = 0xee;
	    }
#endif
	  printxy_str(1, 1, "Checking");
	  dump_lcd();
	  
	  // Read EEPROM and check RAM matches
	  for(int i=0; i<RAM_SIZE; i+=PAGE_SIZE)
	    {
	      //DEBUG_STOP;
	      read_eeprom(EEPROM_0_ADDR_RD , i, PAGE_SIZE, &(buffer[0]));
	      
	      // Check the data read is the same as that in RAM
	      for(int j=0; j<PAGE_SIZE; j++)
		{
		  
		  if( ramdata[i+j] == buffer[j] )
		    {
		      // All ok 
		    }
		  else
		    {
		      // Error, record where and halt for debug
		      printxy_str(1, 1, "BAD   ");
		      dump_lcd();
		      
		      DEBUG_STOP;
		    }
		}
	    }
	}
    }
  
  printxy_str(1, 1, "Good  ");
  dump_lcd();
  DEBUG_STOP;
}


////////////////////////////////////////////////////////////////////////////////
//
// EEPROM RAM Dump and restore
//
////////////////////////////////////////////////////////////////////////////////

// The RAM is dumped to an eeprom
//
// We have a few areas in the RAM which we can use as a signature
// to indicate that a valid RAM dump is in the EEPROM
// The two bytes at 0x0190 and 0x0191 have a known value after a dump is
// performed.

void eeprom_ram_dump(void)
{
  u_int16_t csum = 0;
  
#if DISABLE_DMP_WR
  return;
#endif
  // Zero checksum
  ramdata[EEPROM_CSUM_L] = 0;
  ramdata[EEPROM_CSUM_H] = 0;
  
  // Write in 128 byte pages to the eeprom
  for(int i=0; i<RAM_SIZE; i+=PAGE_SIZE)
    {
      // Update checksum
      for(int j=0; j<PAGE_SIZE; j++)
	{
	  csum += ramdata[i+j];
	}

      // Write the page to EEPROM
      write_eeprom(EEPROM_0_ADDR_WR , i, PAGE_SIZE, &(ramdata[i]));
    }

  // Store csum for debug
  csum_calc_on_dump = csum;

  // Store sum in ramdata
  ramdata[EEPROM_CSUM_L] = csum & 0xFF;
  ramdata[EEPROM_CSUM_H] = csum >> 8;

  // Write the checksum to the EEPROM copy
  write_eeprom(EEPROM_0_ADDR_WR , EEPROM_CSUM_H, EEPROM_CSUM_LEN, &(ramdata[EEPROM_CSUM_H]));
  
  // We have written the data, do we check it?
#if EEPROM_DUMP_CHECK
  eeprom_ram_check();
#endif
}

void eeprom_ram_restore(void)
{
  u_int16_t csum = 0;

#if OVERCLOCK_RESTORE
  set_sys_clock_khz(260000, false);
#endif
    
  // Read EEPROM and restore RAM
  for(int i=0; i<RAM_SIZE; i+=PAGE_SIZE)
    {
      read_eeprom(EEPROM_0_ADDR_RD , i, PAGE_SIZE, &(ramdata[i]));

      // Update checksum
      for(int j=0; j<PAGE_SIZE; j++)
	{
	  csum += ramdata[i+j];
	}
    }

#if OVERCLOCK_RESTORE
  set_sys_clock_khz(130000, false);
#endif

  // When written, the data had 0 in the checksum location, so adjust it
  csum -= ramdata[EEPROM_CSUM_H];
  csum -= ramdata[EEPROM_CSUM_L];

  // Save checksums
  csum_in_eeprom = (ramdata[EEPROM_CSUM_H] << 8) + ramdata[EEPROM_CSUM_L];
  csum_calc_on_restore = csum;


}

////////////////////////////////////////////////////////////////////////////////
//
// Checks that the values in eeprom match those in RAM as a check that
// the write occurred correctly

void eeprom_ram_check(void)
{
  BYTE buffer[PAGE_SIZE];
  
  // Read EEPROM and check RAM matches
  for(int i=0; i<RAM_SIZE; i+=PAGE_SIZE)
    {
      //DEBUG_STOP;
      read_eeprom(EEPROM_0_ADDR_RD , i, PAGE_SIZE, &(buffer[0]));

      // Check the data read is the same as that in RAM
      for(int j=0; j<PAGE_SIZE; j++)
	{
	  if( ramdata[i+j] == buffer[j] )
	    {
	      // All ok 
	    }
	  else
	    {
	      // Error, record where and halt for debug
	      DEBUG_STOP;
	    }
	}
    }

  // The checksum needs to match as well
  if( csum_calc_on_restore != csum_in_eeprom )
    {
      //DEBUG_STOP;
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Function that starts a dump process running and waits for it to finish.
// Must be run on Core0 if core1 is doing the dumping due to I2C conflicts

void eeprom_perform_restore(void)
{
  // Ask core1 to restore the eeprom
  eeprom_done_restore = 0;
  eeprom_do_restore = 1;
  while(!eeprom_done_restore)
    {
    }
}

void eeprom_perform_dump(void)
{
  // Ask core1 to restore the eeprom
  eeprom_done_dump = 0;
  eeprom_do_dump = 1;
  while(!eeprom_done_dump)
    {
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// EEPROM tasks
//
// Dump and restore run in tight loops and use flags to action them
// and flags to signal the job is done
//
// This is designed to run on core1 (or whatever core performs I2C
// work) as we cannot have two cores performing I2C as the GPIO
// writes will interfere.
//

volatile int eeprom_do_invalidate = 0;
volatile int eeprom_do_dump = 0;
volatile int eeprom_do_restore = 0;
volatile int eeprom_done_dump = 0;
volatile int eeprom_done_restore = 0;
volatile int eeprom_done_invalidate = 0;

void eeprom_tasks(void)
{
  if( eeprom_do_dump )
    {
      printxy_str(1,1, "Dumping...");
      dump_lcd();

      eeprom_do_dump = 0;
      eeprom_ram_dump();
      eeprom_done_dump = 1;
      printxy_str(3,1, "          ");
      dump_lcd();
      
    }
  
  if( eeprom_do_restore )
    {
      printxy_str(1,1, "Restoring...");
      dump_lcd();
      
      eeprom_do_restore = 0;
      
#if !DISABLE_RESTORE_ONLY
      eeprom_ram_restore();
#endif
      eeprom_done_restore = 1;
      
      printxy_str(3,1, "            ");
      dump_lcd();
    }

#if 0  
  if( eeprom_do_invalidate )
    {
      eeprom_do_invalidate = 0;
      
      // This is run after a restore so we have the correct cheksum in ram
      // we can just adjust it to get a bad csum
      ramdata[EEPROM_CSUM_L]++;
      ramdata[EEPROM_CSUM_H]++;
      
      // Write the checksum to the EEPROM copy
      write_eeprom(EEPROM_0_ADDR_WR , EEPROM_CSUM_H, EEPROM_CSUM_LEN, &(ramdata[EEPROM_CSUM_H]));

      eeprom_done_invalidate = 1;
    }
#endif
}


