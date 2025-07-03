//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nopl.h"

#define DEBUG          0
#define DB_FL_SCAN_PAK 0
#define DB_FL_SIZE     0

////////////////////////////////////////////////////////////////////////////////

int fl_check_op(int op)
{

}
 
#define fl_check_op(XXX) {			\
  static int opened;				\
  switch(op)					\
    {						\
    case FL_OP_OPEN:				\
      opened=1;					\
      break;					\
						\
    case FL_OP_FIRST:				\
      if( !opened )				\
	{					\
	  printf("\n****NOT OPENED*****");	\
	  while(1)				\
	    {					\
	    }					\
	}					\
      break;					\
    }						\
  }


////////////////////////////////////////////////////////////////////////////////
//

void fl_print_len_string(char *str, int len)
{
  for(int i=0; i<len; i++)
    {
      if( isprint(*str) )
	{
	  printf("%c", *(str++));
	}
      else
	{
	  printf("(%02X)", *(str++));
	}
    }
}

////////////////////////////////////////////////////////////////////////////////

char *fl_op_to_text(FL_OP op)
{
  switch(op)
    {
    case FL_OP_OPEN:
      return("FL_OP_OPEN");
      break;

    case FL_OP_CLOSE:
      return("FL_OP_CLOSE");
      break;

    case FL_OP_FIRST:
      return("FL_OP_FIRST");
      break;

    case FL_OP_NEXT:
      return("FL_OP_NEXT");
      break;

    default:
      return("FL_OP_????");
      break;
    }
  
  return("????");
}

////////////////////////////////////////////////////////////////////////////////

// Device already open when accessed
// first is 1 for first call, 0 thereafter
// return value is 1 if record found, 0 if not
//
// Uses a caller-supplied context that allows nested calls to the service
//
// Does not alter pak address
//
// Returns record data
//         pak address of start of record

int fl_scan_pak(FL_SCAN_PAK_CONTEXT *context, FL_OP op, int device, uint8_t *dest, PAK_ADDR *recstart)
{
  uint8_t  length_byte;
  uint8_t  record_type;
  uint16_t block_length;
  PAK_ADDR rec_start = 0;

#if DB_FL_SCAN_PAK
  dbq("ENTRY:op:%s len_byte:%02X rectype:%02X", fl_op_to_text(op), length_byte, record_type);
  dbq("CPAD:%04X",  pk_qadd());
#endif

  fl_check_op();
  
  // Start at the start of the pack
  switch(op)
    {
    case FL_OP_OPEN:
      // Save the pak address, and restore it when needed
      context->save_pak_addr = pk_qadd();
      return(1);
      break;

    case FL_OP_CLOSE:
      // Done, so restore
      pk_sadd(context->save_pak_addr);
      return(1);
      break;

    case FL_OP_FIRST:
      pk_sadd(0xa);
      break;

    case FL_OP_NEXT:
      //pk_sadd(context->save_pak_addr);
      break;
    }

  // Start of record is here
  rec_start = pk_qadd();
  
  length_byte = pk_rbyt();
  record_type = pk_rbyt();

#if DB_FL_SCAN_PAK
  dbq("Length/type:%02X %02X", length_byte, record_type);
  dbq("CPAD:%04X", pk_qadd());
#endif
  
  if( length_byte == 0 )
    {
      dbq("Length byte zero, exiting");
      pk_sadd(context->save_pak_addr);
      return(ER_FL_NP, 0);
    }

  if( record_type == 0x80 )
    {
      // Long record
      block_length = pk_rwrd();
      pk_skip(block_length);

#if DB_FL_SCAN_PAK
      dbq("Long record:blk_len:%04X", block_length);
#endif

    }
  else
    {
      if( record_type != 0xFF )
	{
#if DB_FL_SCAN_PAK
	  dbq("Short record:type:%02X len:%04X", record_type, length_byte);
#endif

	  // Copy record start
	  *recstart = rec_start;
	  
	  // Copy short record
	  *(dest++) = length_byte;
	  *(dest++) = record_type;
	  
	  for(int i=0; i<length_byte; i++)
	    {
	      *(dest++) = pk_rbyt();	      
	    }

#if DB_FL_SCAN_PAK
	  dbq("\n%s:Exit(1):CPAD:%04X", __FUNCTION__, pkw_cpad);
#endif
	  
	  return(1);
	}
    }

  if( length_byte == 0xFF )
    {
      // Leave the address at the byte after the last used byte
      // We need to move back one byte
      pk_sadd(pkw_cpad-2);

#if DB_FL_SCAN_PAK
      dbq("\n%s:Exit(0):CPAD:%04X\n", __FUNCTION__, pkw_cpad);
#endif

      return(0);
    }
  else
    {
      
#if DB_FL_SCAN_PAK
      dbq("\n%s:Exit(?):CPAD:%04X\n", __FUNCTION__, pkw_cpad);
#endif
  
      return(0);
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Position pak address at first unused byte
//
#if 0
void fl_pos_at_end(void)
{
  int rc = 0;

#if DB_FL_POS_AT_END
  dbq("\n%s", __FUNCTION__);
#endif
  
  rc = fl_catl(1, pkb_curp, NULL, NULL);

  while(rc == 1)
    {
      rc = fl_catl(0, pkb_curp, NULL, NULL);
    }

#if DB_FL_POS_AT_END
  dbq("\n%s:exit Addr now:%04X", __FUNCTION__, pkw_cpad);
#endif

}

#else

void fl_pos_at_end(void)
{
  int rc = 0;
  PAK_ADDR first_free;
  int num_recs;
  int bytes_free;

  fl_size(&bytes_free, &num_recs, &first_free);
  pk_sadd(first_free);
  }

#endif
  
////////////////////////////////////////////////////////////////////////////////

void fl_back(void)
{
  int recno;
  PAK_ADDR pak_addr;
  FL_REC_TYPE rectype;
  int reclen;
  int done = 0;
  uint8_t recdata[256];
  
  // Get the current position and move to the next record
  recno = fl_rpos();

  // Now load records and search them
  while(!done)
    {
      recno--;
      
#if DB_FL_FIND
      dbq("\n%s:Recno:%d rect:%d", __FUNCTION__, recno, rectype);
#endif
      
      if( fl_frec(recno, &pak_addr, &rectype, &reclen) )
	{
	  if( rectype == flb_rect )
	    {
	      // Got next one
	      fl_rset(recno);
	      return;
	    }
	}
      else
	{
	  // If off end of file, leaves pointer there so other code
	  // can see that
	  
	  fl_rset(recno);
	  // No more records
	  done = 1;
	}
    }
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



int fl_catl(FL_OP op, int device, char *filename, uint8_t *rectype)
{
  int rc = 1;
  uint8_t record_data[256];
  PAK_ADDR recstart;
  FL_SCAN_PAK_CONTEXT context;
  
#if DB_FL_CATL
  dbq("\n%s:\n", __FUNCTION__);
#endif
  
  // Access device on first call
  switch(op)
    {
    case FL_OP_OPEN:
    case FL_OP_CLOSE:
      fl_scan_pak(&context, op, device, record_data, &recstart);
      return(1);
      break;

    case FL_OP_FIRST:
    case FL_OP_NEXT:
      break;
    }

  // Check record to see if it is a file
  // record type is 0x81
  // We have a slightly different format here, allowing longer file names
  
  while( rc )
    {
      rc = fl_scan_pak(&context, op, device, record_data, &recstart);
      
      if( record_data[1] == 0x81 )
	{
	  if( record_data[1] == 0x81 )
	    {
	      int rec_data_len = record_data[0];
	      
	      // It is a file.
	      if( rectype != NULL )
		{
		  *rectype = record_data[2+rec_data_len];
		}

	      if( filename != NULL )
		{
		  for(int i=2; i<rec_data_len-1; i++)
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
      dbq("\n%s:rc:%d recdat[0]:%d", __FUNCTION__, rc, record_data[0]);
#endif

    }

  return(rc);
  
}

void fl_copy(void)
{
}

//------------------------------------------------------------------------------
//
// Create a file.
// If type is 0 then find next available


FL_REC_TYPE fl_cret(int logfile, FL_REC_TYPE type)
{
  int     rc = 0;
  uint8_t record[NOBJ_FILENAME_MAXLEN];
  int     new_rectype;
  char    *filename = logical_file_info[logfile].name;
  
#define NUM_RECTYPES (0xfe - 0x90 + 1)
  
  uint8_t used_rectypes[NUM_RECTYPES];

  for(int i=0x90; i<=0xfe; i++)
    {
      used_rectypes[i-0x90] = 0;
    }

#if DEBUG  
  dbq("");
#endif
  
  // Scan the pak and work out the record types that are used
  // We have to do this to find the end of the pack data
  // even if we don't use the type.
  
  uint8_t rectype;
  
  char fn[256];

  dbq("Getting record types");
  
  pk_setp(pkb_curp);

  dbq("Creating pack");
  pk_create(logfile);
  
  // Format pak (temporary call)
  dbq("Formatting pak");
  
  pk_fmat(logfile);

  // Start after header
  dbq("Finding rec types in use");
  pk_sadd(0xa);
  rc = fl_catl(1, pkb_curp, fn, &rectype);

  while(rc == 1)
    {
      used_rectypes[rectype - 0x90] = 1;
      rc = fl_catl(0, pkb_curp, fn, &rectype);
    }

  dbq("Rec types");
  
  for(int i=0x90; i<=0xfe; i++)
    {
      dbq("%02X:%d", i, used_rectypes[i-0x90]);
    }

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
      dbq("No new rec type available");
    }
  else
    {
      dbq("New rectype of %02X available", new_rectype);
    }

  if( type != 0 )
    {
      new_rectype = type;
    }

  dbq("File record type:%02X", new_rectype);
  
  // Create a file record entry at the end of the file
  // +2 as we include the device letter and colon
  int total_reclen = strlen(filename)+2;

  // Ensure we don't overflow record
  if( total_reclen >= sizeof(record) )
    {
      total_reclen = sizeof(record)-1;
    }
  
  dbq("Creating file record entry of len:%02X", total_reclen);
  
  // Write the file record
  // e.g.  09 81 4D 41 49 4E 20 20 20 20 90  filename "MAIN"
  record[0] = total_reclen-2;
  record[1] = 0x81;

  int i;
  for(i=0; i<strlen(filename); i++)
    {
      record[2+i] = *(filename+i);
    }
  
  record[2+i] = new_rectype;

  dbq("Record:");
  for(int i=0; i<total_reclen; i++)
    {
      dbq("%02X ", record[i]);
    }
  
  PAK_ADDR pak_addr;
  int bytes_free;
  int num_recs;
  
  // Append record
  fl_size(&bytes_free, &num_recs, &pak_addr);
  pk_sadd(pak_addr);
  
  pk_save(total_reclen, record);

  dbq("\nFile record type:%02X", new_rectype);
  return(new_rectype);
}

void fl_deln(void)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// Erase the current record
//

void fl_eras(void)
{
  PAK_ADDR pak_addr;
  uint8_t id0;
  FL_REC_TYPE rectype;
  int reclen;
  int found = 0;
  uint8_t dest[2];
  
  pak_addr = pk_qadd();
  
  // Is this an eprom pack? i.e. do we just clear the top bit of the record type?
  // If a RAM pack then we can move the data around and recover the space.

  pk_read(1, &id0);

#if DB_FL_ERAS
  dbq("\n%s:id0=%02X", __FUNCTION__, id0);
#endif
  
  if( id0 & 0x02,1 )
    {
      // EPROM pack
#if DB_FL_ERAS
  dbq("\n%s:EPROM pak", __FUNCTION__);
#endif

      // Clear top bit of record type of current record
      
      // Find the record
      found = fl_frec(flw_crec, &pak_addr, &rectype, &reclen);
      
      // Read record if found
      if( found )
	{
	  // Copy data
	  // Skip the two byte header so we read just the data
#if DB_FL_ERAS
	  dbq("\n%s:Addr;%04X", __FUNCTION__, pak_addr);
#endif

	  pk_sadd(pak_addr);
	  pk_read(2, dest);
	  
#if DB_FL_ERAS
	  dbq("\nRead data:\n");
	  db_dump(dest, 2);
	  dbq("\n");
#endif

	  // Clear the bit
	  dest[1] &= 0x7f;

	  // Write back
	  pk_sadd(pak_addr);
	  pk_save(2, dest);

#if DB_FL_ERAS
	  dbq("\nWrite data:\n");
	  db_dump(dest, 2);
	  dbq("\n");
#endif

	}
    }
  else
    {
    }
  
  // restore pak address
  pk_sadd(pak_addr);
}

void fl_ffnd(void)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// Find the next record that contains the search string
//
// Searches the current record and onwards.
// Leaves the record number at the found record, the caller has to move it on for
// subsequent searches.
//

int fl_find(char *srch, char *dest, int *len)
{
  int recno;
  PAK_ADDR pak_addr;
  PAK_ADDR save_pak_addr;
  FL_REC_TYPE rectype;
  int reclen;
  int done = 0;
  uint8_t recdata[256];

#if DB_FL_FIND
  dbq("\n%s:Entry", __FUNCTION__);
#endif
      
  // Get the current position and move to the next record
  recno = flw_crec;
  rectype= flb_rect;
  
  // Now load records and search them
  while(!done)
    {
#if DB_FL_FIND
      dbq("\n%s:Recno:%d rect:%d", __FUNCTION__, recno, rectype);
#endif
      
      if( fl_frec(recno, &pak_addr, &rectype, &reclen) )
	{
	  // Got a record, load the data and see if it contains the search
	  // string.
	  save_pak_addr = pk_qadd();
	  pk_sadd(pak_addr);
#if DB_FL_FIND
	  dbq("\n%s:Reading %d bytes", __FUNCTION__, reclen+2);
#endif
	  pk_read(reclen+2, recdata);
	  
#if DB_FL_FIND
	  dbq("\n%s:Recdata:", __FUNCTION__);
	  fl_print_len_string(recdata+2, reclen);
#endif
	  pk_sadd(save_pak_addr);

#if DB_FL_FIND
	  dbq("\n%s:TESTING %s against Recdata (%d bytes)", __FUNCTION__, srch, reclen);
	  fl_print_len_string(recdata+2, reclen);
#endif

	  int found = 0;
	  
	  for(int i=0; i<reclen; i++)
	    {
	      if( (strncmp(&(recdata[i]), srch, strlen(srch)) == 0) || (strlen(srch) == 0) )
		{
#if DB_FL_FIND
		  dbq("\n%s:**FOUND**", __FUNCTION__);
#endif
	  
		  // Yes, found it
		  found = 1;
		  break;
		}
	    }

	  if( found )
	    {
	      // Copy the data over
	      memcpy(dest, recdata+2, reclen);
	      //sdbq(dest, "dummy");
	      // Terminate string
	      *(dest+reclen) = '\0';
	      
	      *len = reclen;

	      // Leave record number pointing at this record.
	      fl_rset(recno);

	      return(1);	      
	    }
	  else
	    {
	      // No match, keep looking
	      recno++;
	    }
	}
      else
	{
	  // No record, we are done
#if DB_FL_FIND
	  dbq("\n%s:exit(0)", __FUNCTION__);
#endif
	  return(0);
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Find the Nth record of the current record type and return
// information about it

int fl_frec(int n, PAK_ADDR *pak_addr, FL_REC_TYPE *rectype, int *reclen)
{
  int rc = 1;
  int rec_n = 0;
  int op = FL_OP_FIRST;
  uint8_t record_data[256];
  PAK_ADDR recstart;
  FL_SCAN_PAK_CONTEXT context;

  // Validate record number
  if( n <= 0 )
  {
    return(0);
  }
  
  // Check record to see if it is a file
  // record type is 0x81

  fl_scan_pak(&context, FL_OP_OPEN, pkb_curp, record_data, &recstart);
  
  while( rc )
    {
      rc = fl_scan_pak(&context, op, pkb_curp, record_data, &recstart);

      if( !rc )
	{
	  continue;
	}
      
#if DB_FL_FREC
      dbq("\n%s:n:%d recno:%d rect:%d recstart:%04X", __FUNCTION__, n, rec_n, record_data[1], recstart);
#endif
      
      op = FL_OP_NEXT;
      
      if( record_data[1] == flb_rect )
	{
	  // This is a record for our file
	  rec_n++;

	  if( rec_n == n )
	    {
	      // This is the record we are looking for
	      // Return data
#if DB_FL_FREC
	      dbq("\n%s:Found record (recdata ", __FUNCTION__);
	      fl_print_len_string(record_data, record_data[0]);
	      dbq(" )");
#endif
	      *pak_addr = recstart;
	      *rectype  = record_data[1];
	      *reclen   = record_data[0];
	      
	      fl_scan_pak(&context, FL_OP_CLOSE, pkb_curp, record_data, &recstart);
	      return(1);    
	    }
	}
      else
	{
	  // Do another loop, as this wasn't a file
	}

#if DB_FL_FREC
      dbq("\n%s:rc:%d rec_n:%d n:%d", __FUNCTION__, rc, rec_n, n);
#endif
    }

  fl_scan_pak(&context, FL_OP_CLOSE, pkb_curp, record_data, &recstart);
    
  // Not found, return error
#if DB_FL_FREC
  dbq("\n%s:NOT FOUND rc=0", __FUNCTION__);
#endif

  return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Move to next record of current record type
//

void fl_next(void)
{
  int recno;
  PAK_ADDR pak_addr;
  FL_REC_TYPE rectype;
  int reclen;
  int done = 0;
  uint8_t recdata[256];
  
  // Get the current position and move to the next record
  recno = fl_rpos();

  // Now load records and search them
  while(!done)
    {
      recno++;
      
#if DB_FL_FIND
      dbq("\n%s:Recno:%d rect:%d", __FUNCTION__, recno, rectype);
#endif
      
      if( fl_frec(recno, &pak_addr, &rectype, &reclen) )
	{
	  if( rectype == flb_rect )
	    {
	      // Got next one
	      fl_rset(recno);
	      return;
	    }
	}
      else
	{
	  // If off endof file, leaves pointer there so other code
	  // can see that
	  
	  fl_rset(recno);
	  // No more records
	  done = 1;
	}
    }
}

int fl_rpos(void)
{
  return(flw_crec);
}
  
////////////////////////////////////////////////////////////////////////////////

void fl_open(int logfile)
{
  // Open current file
  pk_open(logfile);
}

////////////////////////////////////////////////////////////////////////////////

int fl_exist(char *filename)
{
  pk_exist(filename);
}

////////////////////////////////////////////////////////////////////////////////

void fl_pars(void)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// Reads a record
//
// Current record is set before call
// Current record type is set before call
//
// Only reads the data, not the record header
//

int fl_read(uint8_t *dest)
{
  PAK_ADDR pak_addr;
  FL_REC_TYPE rectype;
  int reclen;
  int found = 0;
  
  // Find the record
  found = fl_frec(flw_crec, &pak_addr, &rectype, &reclen);

  // Read record if found
  if( found )
    {
      // Copy data
      // Skip the two byte header so we read just the data
      pk_sadd(pak_addr+2);
      pk_read(reclen, dest);

#if DB_FL_READ
      dbq("\nRead data:\n");
      db_dump(dest, reclen);
      dbq("\n");
#endif
      
      return(1);
    }
  
  return(0);
}

// Set default record type
void fl_rect(FL_REC_TYPE rect)
{
  flb_rect = rect;
}

void fl_renm(void)
{
}

void fl_rset(int recno)
{
  if( recno == 0 )
    {
      runtime_error(ER_FL_BR, "Bad recno (0)");
    }
  
  flw_crec = recno;
}

void fl_setp(int device)
{
  pk_setp(device);
}

////////////////////////////////////////////////////////////////////////////////

void fl_size(int *bytes_free, int *num_recs, PAK_ADDR *first_free)
{
  // Count the number of records of current record type
  uint8_t recdat[256];
  int rc = 1;
  int op = FL_OP_FIRST;
  PAK_ADDR recstart;
  int rec_cnt = 0;
  PAK_ADDR addr_save;
  uint8_t id[10];
  FL_SCAN_PAK_CONTEXT context;
  PAK_ADDR save_addr;
  
#if DB_FL_SIZE
  dbq("\n%s:", __FUNCTION__);
#endif

  fl_scan_pak(&context, FL_OP_OPEN, pkb_curp, recdat, &recstart);
  
  while(rc)
    {
      rc = fl_scan_pak(&context, op, pkb_curp, recdat, &recstart);
      op = FL_OP_NEXT;
      
#if DB_FL_SIZE
      dbq("\n%s:rc:%d rcnt:%d", __FUNCTION__, rc, rec_cnt);
#endif
  
      if( rc )
	{
	  if( recdat[1] == flb_rect )
	    {
	      rec_cnt++;
	    }
	}
    }

  // Read pak ID for the size of the pack
  save_addr = pk_qadd();
  pk_sadd(0);
  pk_read(10, id);
  pk_sadd(save_addr);
  
  *bytes_free = (id[1] * 8192) - pkw_cpad;
  *first_free = pkw_cpad;
  *num_recs = rec_cnt;

  fl_scan_pak(&context, FL_OP_CLOSE, pkb_curp, recdat, &recstart);
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

  dbq("Len:%d [0]=%d [1]=%d flb_rect=%02X", len, lentype[0], lentype[1], flb_rect);
  
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
