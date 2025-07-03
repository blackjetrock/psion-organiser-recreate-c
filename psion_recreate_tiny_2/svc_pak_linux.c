////////////////////////////////////////////////////////////////////////////////
//
// Linux Pack Services
//
// 
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "nopl.h"

#define DEBUG 0

////////////////////////////////////////////////////////////////////////////////
//


// Info for each logical drive

LINUX_FILE_INFO linux_file_info[NOPL_NUM_LOGICAL_FILES];

#define LINFI    linux_file_info[logfile]
#define LINCURFI linux_file_info[current_logfile]

////////////////////////////////////////////////////////////////////////////////
//
// Pack is in a file and uses the Psion file system
//
//------------------------------------------------------------------------------
// Read a byte from the flash pak
//
////////////////////////////////////////////////////////////////////////////////


#define PACK_FN "datapack.bin"
//#define FLASH_PAK_SIZE    ((uint32_t)(1024*1024*1))
#define FLASH_PAK_SIZE    ((uint32_t)(1024*64))

//------------------------------------------------------------------------------
//
// Open for reading
//

void pk_open_linux(int logfile, char *filename)
{
  LINFI.fp = fopen(filename, "rb+");

  if( LINFI.fp == NULL )
    {
      trappable_runtime_error(ER_FL_NX, "File does not exist:'%s'", filename);
    }

#if DEBUG
  printf("\n%s:fp=%X name:'%s' logfile=%d", __FUNCTION__, LINFI.fp, filename, logfile);
#endif
}

//------------------------------------------------------------------------------
//
// Open for creating
//

void pk_create_linux(int logfile, char *filename)
{
  LINFI.fp = fopen(filename, "wb+");

  if( LINFI.fp == NULL )
    {
      trappable_runtime_error(ER_FL_NX, "File cannot be created:'%s'", filename);
    }

#if DEBUG
  printf("\n%s:fp=%X name:'%s' logfile=%d", __FUNCTION__, LINFI.fp, filename, logfile);
#endif
}

void pk_close_linux(int logfile, char *filename)
{
#if DEBUG
  printf("\n%s:", __FUNCTION__);
#endif
  if( LINFI.fp != NULL )
    {
      fclose(LINFI.fp);
    }
}

int pk_exist_linux(char *filename)
{
  FILE *fp;

  fp = fopen(filename, "r");

  if( fp == NULL )
    {
      return(0);
    }
  return(1);
}

//------------------------------------------------------------------------------

uint8_t pk_rbyt_linux(PAK_ADDR pak_addr)
{
  uint8_t byte=0xaa;
  long int offset = pak_addr;
  int rc;

#if 1
  rc = fseek(LINCURFI.fp, offset, SEEK_SET);
  if( rc != 0 )
    {  
      //printf("\nfseek error:%d", errno);
      //perror("error");
    }
  
  fread(&byte, 1, 1, LINCURFI.fp);
  //perror("error");
#else
  FILE *fp;

  fp = fopen("TESTFILE", "r");

  if( fp == NULL )
    {
      printf("\nOpen failed");
      exit(-1);
    }
  
  rc = fseek(fp, offset, SEEK_SET);
  if( rc != 0 )
    {
      //printf("\nfseek error:%d", errno);
      //perror("error");
    }
  
  fread(&byte, 1, 1, fp);
  fclose(fp);
#endif

#if DEBUG
  printf("\nrbyte %02X @ %06X fp=%x curlogfile:%d fn:'%s'", (int)byte, (int)pak_addr,
	 LINCURFI.fp,
	 current_logfile,
	 logical_file_info[current_logfile].name);
#endif  
  return(byte);
}

//------------------------------------------------------------------------------
//
// Write a block of data to the pak
//

void pk_save_linux(PAK_ADDR pak_addr, int len, uint8_t *src)
{
  fseek(LINCURFI.fp, pak_addr, SEEK_SET);
  fwrite(src, len, 1, LINCURFI.fp);
}

//------------------------------------------------------------------------------
//
// Format the flash pak
//
// Write a pak header and also sets the rest of the pack to FF
//

void pk_format_linux(int logfile)
{
  PAK_ID  pak_id;
  uint8_t byte = 0xFF;

#if DEBUG
  printf("\n%s:Formatting PAK_SIZE:%X\n", __FUNCTION__, FLASH_PAK_SIZE);
#endif
  
  // Write 0xff to the entire pack
  for(int i=0; i< FLASH_PAK_SIZE;i++)
    {
      pk_save_linux(i, 1, &byte);
    }
  
  // Now write a pak header
  pk_build_id_string(pak_id, FLASH_PAK_SIZE, 24, 8, 21, 8,  0xaabbccdd);
  
  pk_save_linux(0, 10, pak_id);

#if DEBUG
  printf("\nDone");
#endif
}


