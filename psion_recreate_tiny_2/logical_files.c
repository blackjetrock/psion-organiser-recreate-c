////////////////////////////////////////////////////////////////////////////////
//
// Files
//
////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "nopl.h"

#define DEBUG 0

////////////////////////////////////////////////////////////////////////////////
//
// Logical files
//
////////////////////////////////////////////////////////////////////////////////

int current_logfile;
NOPL_LOGICAL_FILE logical_file_info[NOPL_NUM_LOGICAL_FILES];

//------------------------------------------------------------------------------

void init_logical_files(void)
{
  for(int logfile=0; logfile<NOPL_NUM_LOGICAL_FILES; logfile++)
    {
      logical_file_info[logfile].open = 0;
      strcpy(logical_file_info[logfile].name, "");
    }
}

//------------------------------------------------------------------------------
//
// We build a list of field names, as pointers to chars.
// The strings themselves are stored in a pool of characters (char array)

#define LFI logical_file_info[logfile]

void logfile_store_field_names(NOBJ_MACHINE *m, int logfile, uint8_t *flist)
{
  uint8_t b;
  
  LFI.num_field_names = 0;
  LFI.fname_pool_i = 0;

  // Scan the QCode list and build the field names
  b = *(flist++);

#if DEBUG
  printf("\nlfs: b:%d", b);
#endif
  
  while(b !=  QCO_END_FIELDS )
    {
      int len;

      // We have a start for the string
      LFI.field_name[LFI.num_field_names] = &(LFI.field_name_charpool[LFI.fname_pool_i]);

      // Type
      LFI.field_type[LFI.num_field_names] = b;

      len = *(flist++);

#if DEBUG
      printf("\nlfs: len:%d", len);
#endif
      
      for(int i=0; i<len; i++)
	{
	  LFI.field_name_charpool[(LFI.fname_pool_i)++] = *(flist++);
#if DEBUG
	  printf("\nlfs: ch:%c", LFI.field_name_charpool[(LFI.fname_pool_i-1)]);
#endif
	}

      LFI.field_name_charpool[(LFI.fname_pool_i)++] = '\0';
      (LFI.num_field_names)++;

      b = *(flist++);
    }
}

//------------------------------------------------------------------------------

int logfile_get_field_index(NOBJ_MACHINE *m, int logfile, char *field_name)
{
  for(int i=0; i<LFI.num_field_names; i++)
    {
      if( strcmp(LFI.field_name[i], field_name)==0 )
	{
	  return(i);
	}
    }
  return(-1);
}

//------------------------------------------------------------------------------

char *address_tab_n(int n, int logfile)
{
  char *p = &(logical_file_info[logfile].buffer[0]);
  char *buf_start = p;
  char *buf_end = buf_start + logical_file_info[logfile].buffer_size - 1; 
  int n_found = 0;

#if 0
  printf("\n\n%s n:%d buf_start:%p buf_end:%p", __FUNCTION__, n, buf_start, buf_end);
#endif
  
  if( n == 0 )
    {
      return(p);
    }

  for(int i=0; (n_found<n) && (i<logical_file_info[logfile].buffer_size); i++, p++)
    {
      if( *p == NOPL_FIELD_DELIMITER )
	{
	  n_found++;

	  if(n_found == n )
	    {
	      return(p);
	    }
	}
    }

  if( p > buf_end )
    {
      p = buf_end;
    }

  return(p);
}

//------------------------------------------------------------------------------

char *logfile_field_start(int n, int logfile)
{
  if( n == 0 )
    {
      return( &(logical_file_info[logfile].buffer[0]));
    }
  
  if( n == LFI.num_field_names - 1 )
    {
      return( &(logical_file_info[logfile].buffer[logical_file_info[logfile].buffer_size-1]));
    }
}

//------------------------------------------------------------------------------

void dump_buffer(int logfile)
{
#if DEBUG
  printf("\nBuffer size:%d\n", logical_file_info[logfile].buffer_size);
  for(int i=0; i< logical_file_info[logfile].buffer_size; i++)
    {
      printf("%c%02X", logical_file_info[logfile].buffer[i]==9?'*':' ', i);
    }
  printf("\n");

  for(int i=0; i< logical_file_info[logfile].buffer_size; i++)
    {
      printf(" %02X", ((long int)&(logical_file_info[logfile].buffer[i])) & 0xFF);
    }
  printf("\n");

  for(int i=0; i< logical_file_info[logfile].buffer_size; i++)
    {
      printf(" %02X", logical_file_info[logfile].buffer[i]);
    }
  printf("\n");

#endif
}

//------------------------------------------------------------------------------

void find_field_bounds(int logfile, int field_num, char **start, char **end)
{
  // Find current field and delete contents
  if( field_num == 0 )
    {
      *start = &(logical_file_info[logfile].buffer[0]);
    }
  else
    {
      *start = address_tab_n(field_num, logfile)+1;
    }

  if( field_num ==  (logical_file_info[logfile].num_field_names)-1)
    {
      *end = &(logical_file_info[logfile].buffer[(logical_file_info[logfile].buffer_size -1)]);
    }
  else
    {
      *end = address_tab_n(field_num+1, logfile)-1;
    }
}

//------------------------------------------------------------------------------

char field_val[256];

char *logfile_get_field_as_str(NOBJ_MACHINE *m, int logfile, char *field_name)
{
  int field_num;
  char *start, *end;

  
  if( (field_num = logfile_get_field_index(m, logfile, field_name)) != -1 )
    {
      // Get field value
      // Find nth field delimited by FIELD_DELIMITER character

      find_field_bounds(logfile, field_num, &start, &end);
      
      // Copy field value
      int i;
      for(i = 0; i<end-start+1; i++)
	{
	  field_val[i] = *(start+i);
	}

      field_val[i] = '\0';
    }

  return(field_val);
}

//------------------------------------------------------------------------------

void logfile_put_field_as_str(NOBJ_MACHINE *m, int logfile, char *field_name, char *field_val)
{
  int field_num;
  char *start, *end;
  int new_field_len = strlen(field_val);
  
  if( (field_num = logfile_get_field_index(m, logfile, field_name)) == -1 )
    {
      // unknown field, exit
      return;
    }

#if DEBUG
  printf("\n----------------------------------\n%s: buf:%p field: num:%d name:'%s', len:%d",
	 __FUNCTION__,
	 &(logical_file_info[logfile].buffer[0]),
	 field_num,
	 field_name,
	 new_field_len
	 );
#endif
  dump_buffer(logfile);

  find_field_bounds(logfile, field_num, &start, &end);

  // Remove this fields contents, if there are any
  int field_len = end - start + 1;
  
#if DEBUG
  printf("\nStart:%p End:%p Field_len:%d New_field_len:%d", start, end, field_len, new_field_len);
#endif
   
  if( field_len > 0 )
    {
      // Move everything down
      memmove(start, end+1, field_len);
    }

  logical_file_info[logfile].buffer_size -= field_len;
  dump_buffer(logfile);

  // Now make space for the new data
  memmove(start+new_field_len, start, new_field_len);
  logical_file_info[logfile].buffer_size += new_field_len;
  dump_buffer(logfile);
	  
  // Now put the new contents into the field
  for(int i=0; i<strlen(field_val); i++)
    {
      *(start+i) = *(field_val+i);
    }
  
  dump_buffer(logfile);

}

