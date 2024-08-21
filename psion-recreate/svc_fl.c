//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate_all.h"


////////////////////////////////////////////////////////////////////////////////

// Device already open when accessed
// first is 1 for first call, 0 thereafter
// return value is 1 if record found, 0 if not
//



int fl_scan_pack(int first, int device, uint8_t *dest)
{
  uint8_t  length_byte;
  uint8_t  record_type;
  uint16_t block_length;

  // Start at the start of the pack
  if(first)
    {
      pk_sadd(0x0a);
    }

  length_byte = pk_rbyt();
  record_type = pk_rbyt();

#if DB_FL_SCAN_PACK
  printf("\n%s:first:%d len_byte:%02X rectype:%02X", __FUNCTION__, first, length_byte, record_type);
  printf("\nCPAD:%04X", pkw_cpad);
#endif
  
  if( length_byte == 0 )
    {
      return(ER_FL_NP);
    }

  if( record_type == 0x80 )
    {
      // Long record
      block_length = pk_rwrd();
      pk_skip(block_length);

#if DB_FL_SCAN_PACK
  printf("\nLong record:blk_len:%04X", block_length);
#endif

    }
  else
    {
      if( record_type != 0xFF )
	{
#if DB_FL_SCAN_PACK
	  printf("\nShort record:len:%04X", length_byte);
#endif

	  // Copy short record
	  *(dest++) = length_byte;
	  *(dest++) = record_type;
	  
	  for(int i=0; i<length_byte; i++)
	    {
	      *(dest++) = pk_rbyt();	      
	    }
#if DB_FL_SCAN_PACK
  printf("\nExit(1):CPAD:%04X", pkw_cpad);
#endif
	  
	  return(1);
	}
    }

  if( length_byte == 0xFF )
    {
      // Leave the address at the byte after the last used byte
      // We need to move back one byte
      pk_sadd(pkw_cpad-2);

#if DB_FL_SCAN_PACK
  printf("\nExit(0):CPAD:%04X", pkw_cpad);
#endif

      return(0);
    }
  else
    {
      printf("\n%s:exit ?", __FUNCTION__);

#if DB_FL_SCAN_PACK
  printf("\nExit(?):CPAD:%04X", pkw_cpad);
#endif
  
      return(0);
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Position pak address at first unused byte
//

void fl_pos_at_end(void)
{
  int rc = 0;

#if DB_FL_POS_AT_END
  printf("\n%s", __FUNCTION__);
#endif
  
  rc = fl_catl(1, pkb_curp, NULL, NULL);

  while(rc == 1)
    {
      rc = fl_catl(0, pkb_curp, NULL, NULL);
    }

#if DB_FL_POS_AT_END
  printf("\n%s:exit", __FUNCTION__);
#endif

}

////////////////////////////////////////////////////////////////////////////////

void fl_back(void)
{
}

void fl_bcat(void)
{
}

void fl_bdel(void)
{
}

void fl_bopn(void)
{
}

void fl_bsav(void)
{
}

//------------------------------------------------------------------------------
//
// Returns filenames in filename,
// return code is 1 if file found, 0 if not



int fl_catl(int first, int device, char *filename, uint8_t *rectype)
{
  int rc = 1;
  uint8_t record_data[256];

#if DB_FL_CATL
  printf("\n%s:\n", __FUNCTION__);
#endif
  
  // Access device on first call
  if( first )
    {
      pk_setp(device);
    }
  
  // Check record to see if it is a file
  // record type is 0x81

  while( rc )
    {

      rc = fl_scan_pack(first, device, record_data);
      
      if( record_data[0] == 0x09 )
	{
	  if( record_data[1] == 0x81 )
	    {
	      // It is a file.
	      if( rectype != NULL )
		{
		  *rectype = record_data[10];
		}

	      if( filename != NULL )
		{
		  for(int i=2; i<10; i++)
		    {
		      *(filename++) = record_data[i];
		    }
		  *filename = '\0';
		}

	      // Exit with file data
	      break;
	    }

	}
      else
	{
	  // Do another loop, as this wasn't a file
	}
#if DB_FL_CATL
      printf("\n%s:rc:%d recdat[0]:%d", __FUNCTION__, rc, record_data[0]);
#endif

    }

return(rc);
  
}

void fl_copy(void)
{
}

//------------------------------------------------------------------------------
// Create a file

FL_REC_TYPE fl_cret(char *filename)
{
  int rc = 0;
  uint8_t record[11];
  int new_rectype;
  
#define NUM_RECTYPES (0xfe - 0x90 + 1)
  
  uint8_t used_rectypes[NUM_RECTYPES];

  for(int i=0x90; i<=0xfe; i++)
    {
      used_rectypes[i-0x90] = 0;
    }
  
  printf("\n%s:", __FUNCTION__);
  
  // Scan the pak and work out the record types that are used

  uint8_t rectype;
  
  char fn[256];

  printf("\nGetting record types");

  pk_setp(pkb_curp);

  rc = fl_catl(1, pkb_curp, fn, &rectype);

  while(rc == 1)
    {
      printf("\n%s ($%02X) found", fn, rectype);
      used_rectypes[rectype - 0x90] = 1;
      rc = fl_catl(0, pkb_curp, fn, &rectype);
    }
  
  for(int i=0x90; i<=0xfe; i++)
    {
      printf("\n%02X:%d", i, used_rectypes[i-0x90]);
    }

  printf("\nDone...");
  
  // Now find a record type that isn't used
  new_rectype = 0;

  for(int i=0x91; i<=0xfe; i++)
    {
      if( used_rectypes[i-0x90] == 0 )
	{
	  new_rectype = i;
	}
    }

  if( new_rectype == 0 )
    {
      printf("\nNo new rec type available");
    }
  else
    {
      printf("\nNew rectype of %02X available", new_rectype);
    }

  printf("\n");

  
  // Create a file record entry at the end of the file

  // Write the record
  // e.g.  09 81 4D 41 49 4E 20 20 20 20 90  filename "MAIN"
  record[0] = 0x09;
  record[1] = 0x81;
  for(int i=0; i<8; i++)
    {
      record[2+i] = *(filename++);
    }

  record[10] = new_rectype;

   pk_save(11, record);

}

void fl_deln(void)
{
}

void fl_eras(void)
{
}

void fl_ffnd(void)
{
}

void fl_find(void)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// Find the Nth record of the current record type and return
// information about it

PAK_ADDR fl_frec(int n, PAK_ADDR *pak_addr, FL_REC_TYPE *rectype, int *reclen)
{
  int rc = 1;
  int rec_n = 0;
  int first = 1;
  uint8_t record_data[256];
  
  // Check record to see if it is a file
  // record type is 0x81
  printf("\npkfrec\n");
  
  while( rc )
    {
      rc = fl_scan_pack(first, pkb_curp, record_data);

#if DB_FL_FREC
      printf("\n%s:recdat[1]:%d", __FUNCTION__, record_data[1]);
#endif
      
      first = 0;
      
      if( record_data[1] == flb_rect )
	{
	  // This is a record for our file
	  rec_n++;

	  if( rec_n == n )
	    {
	      // This is the record we are looking for
	      // Return data
#if DB_FL_FREC
	      printf("\nFound record");

#endif

	      *pak_addr = pkw_cpad;
	      *rectype  = record_data[1];
	      *reclen   = record_data[0];
	      return(1);    
	    }
	}
      else
	{
	  // Do another loop, as this wasn't a file
	}

#if DB_FL_FREC
      printf("\nrc:%d rec_n:%d n:%d", rc, rec_n, n);
#endif
    }

  // Not found, return error
  return(0);
}

void fl_next(void)
{
}

void fl_open(void)
{
}

void fl_pars(void)
{
}

void fl_read(void)
{
}

// Set default record type
void fl_rect(FL_REC_TYPE rect)
{
  flb_rect = rect;
}

void fl_renm(void)
{
}

void fl_rset(void)
{
}

void fl_setp(int device)
{
  pk_setp(device);
}

void fl_size(void)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// Write data to current file (rec type)
// Append to the file
//

void fl_writ(uint8_t *src, int len)
{
  uint8_t lentype[2];

  if( len > 254 )
    {
      len = 254;
    }

  lentype[0] = (len % 256);
  lentype[1] = flb_rect;
  
  // Find the end of the file (and pack) as the data will be appended
  fl_pos_at_end();

  // Write the length and type
  pk_save(sizeof(lentype), lentype);

  // Write the data
  pk_save(len, src);
  
}

void tl_cpyx(void)
{

}
