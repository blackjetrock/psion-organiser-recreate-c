//
// Common functions that deal with QCodes
//

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdarg.h>

#include "newopl.h"
#include "nopl.h"
#include "nopl_obj.h"

#define DEBUG_PUSH_POP    1

uint16_t swap_uint16(uint16_t n)
{
  int h = (n & 0xFF00) >> 8;
  int l = (n & 0x00FF);

  return((l << 8) + h);
}

int read_item(FILE *fp, void *ptr, int n, size_t size)
{
  int ni = fread(ptr, n, size, fp);
  
  if( feof(fp) || (ni == 0))
    {
      // No more file
      return(0);
    }
  
  return(1);
}

////////////////////////////////////////////////////////////////////////////////
//
// Reads the header of an OB3 file
//
//

char ob3_hdr[10];
uint8_t  ob3_block_file_type = 0;
uint16_t ob3_file_length_word = 0;
uint16_t ob3_block_length_word = 0;

void read_ob3_header(FILE *fp)
{
  int ni;

  strcpy(ob3_hdr, "\0\0\0\0\0\0");
	 
  printf("\nReading OB3 header");
  
  // Drop the ORG string
  ni = read_item(fp, &ob3_hdr, 1, 3);
  printf("\nHdr                  : %s", ob3_hdr);
  
  // File length word
  ni = read_item(fp, &ob3_file_length_word, 1, 2);

  printf("\nOB3 File length word : %X", (int)swap_uint16(ob3_file_length_word));
  
  // Block file type
  ni = fread(&ob3_block_file_type, 1, 1, fp);
  printf("\nBlock file type      : %02X", (int)ob3_block_file_type);

  // Block length
  ni = read_item(fp, &ob3_block_length_word, 1, 2);
  printf("\nOB3 Block length word: %X", (int)swap_uint16(ob3_block_length_word));
}

int debugi = 0;


#define READ_ITEM(FP,DEST,NUM,TYPE,ERROR)			\
  printf("\nLoading %d", debugi++);                             \
  if(!read_item(FP, (void *)&(DEST), NUM, sizeof(TYPE)))	\
    {								\
      printf(ERROR);						\
      return;							\
    }


//------------------------------------------------------------------------------
// Reads a proc file and builds a NOBJ_PROC
// The NOBJ_PROC is used for dumping procs
//------------------------------------------------------------------------------

void read_proc_file(FILE *fp, NOBJ_PROC *p)
{
  printf("\nEnter:%s", __FUNCTION__);
  
  READ_ITEM(  fp, p->var_space_size, 1, NOBJ_VAR_SPACE_SIZE, "\nError reading var space size.");
  p->var_space_size.size = swap_uint16(p->var_space_size.size);

  READ_ITEM(fp, p->qcode_space_size,   1,                     NOBJ_QCODE_SPACE_SIZE, "\nError reading qcode space size.");
  p->qcode_space_size.size = swap_uint16(p->qcode_space_size.size);
  
  READ_ITEM(fp, p->num_parameters.num, 1,                     NOBJ_NUM_PARAMETERS,   "\nError reading number of parameters.");
  printf("\nParameters num:%d", p->num_parameters.num);
  READ_ITEM(fp, p->parameter_types,    p->num_parameters.num, NOBJ_PARAMETER_TYPE,   "\nError reading parameter types.");

  printf("\nGlobal varname size");
    
  READ_ITEM( fp,  p->global_varname_size, 1, NOBJ_GLOBAL_VARNAME_SIZE, "\nError reading global varname size.");
  p->global_varname_size.size = swap_uint16(p->global_varname_size.size);

  printf("\nVar space size     :%d\n", p->var_space_size.size);
  printf("\nGlobal varname size:%d\n", p->global_varname_size.size);

  //------------------------------------------------------------------------------
  // Global varname is more complicated to read. Each entry is length
  // prefixed, so read them until the length we have read matches the
  // size we have just read.
  //------------------------------------------------------------------------------
  
  int length_read = 0;
  int num_global_vars = 0;

  while (length_read < p->global_varname_size.size )
    {
      uint8_t len;
      uint8_t varname[NOBJ_VARNAME_MAXLEN];
      NOBJ_VARTYPE vartype;
      NOBJ_ADDR addr;
      vartype = 0;
      addr = 0;

      if(!read_item(fp, (void *)&len, 1, sizeof(len)))
	{
	  printf("\nError reading global varname entry length");
	  return;
	}

      printf("\nGlobal varname entry length: %d", len);
      
      memset(varname, 0, sizeof(varname));
      
      //      printf("\nVarname entry len=%d", len);
      
      length_read += sizeof(len);

      // 3 bytes on end are type and addr, so read rest of data into name field
      
      if(!read_item(fp, (void *)&varname, len, sizeof(uint8_t)))
	{
	  printf("\nError reading global varname entry name field");
	  return;
	}
      
      printf("\nvarname='%s'", varname);

      length_read += len;

      // read type and addr

      if(!read_item(fp, (void *)&vartype, 1, sizeof(NOBJ_VARTYPE)))
	{
	  printf("\nError reading global varname entry type field");
	  return;
	}

      if(!read_item(fp, (void *)&addr, 1, sizeof(NOBJ_ADDR)))
	{
	  printf("\nError reading global varname entry addr field");
	  return;
	}

      printf("\nVarname type=%02X", vartype);
      printf("\nAddr addr=%02X", addr);
      length_read += 3;

      //printf("\nLength read:%d out of %d", length_read, p->global_varname_size.size);

      // Add variable to list of globals
      p->global_varname[num_global_vars].type    = vartype;
      p->global_varname[num_global_vars].address = swap_uint16(addr);
      strcpy(p->global_varname[num_global_vars].varname, varname);
      
      num_global_vars++;      
    }

  p->global_varname_num = num_global_vars;

  printf("\nNum global vars: %d", num_global_vars);
  
  //------------------------------------------------------------------------------
  //
  // Now read the external varname table
  //
  //------------------------------------------------------------------------------

  READ_ITEM( fp,  p->external_varname_size, 1, NOBJ_EXTERNAL_VARNAME_SIZE, "\nError reading external varname size.");
  p->external_varname_size.size = swap_uint16(p->external_varname_size.size);

  length_read = 0;
  int num_external_vars = 0;

  while (length_read < p->external_varname_size.size )
    {
      uint8_t len;
      uint8_t varname[NOBJ_VARNAME_MAXLEN];
      NOBJ_VARTYPE vartype;
      vartype = 0;
      
      if(!read_item(fp, (void *)&len, 1, sizeof(len)))
	{
	  printf("\nError reading external varname entry length");
	  return;
	}

      memset(varname, 0, sizeof(varname));
      
      //      printf("\nVarname entry len=%d", len);
      
      length_read += sizeof(len);

      // 3 bytes on end are type and addr, so read rest of data into name field
      
      if(!read_item(fp, (void *)&varname, len, sizeof(uint8_t)))
	{
	  printf("\nError reading global varname entry name field");
	  return;
	}
      
      //printf("\nvarname='%s'", varname);

      length_read += len;

      // read type and addr

      if(!read_item(fp, (void *)&vartype, 1, sizeof(NOBJ_VARTYPE)))
	{
	  printf("\nError reading global varname entry type field");
	  return;
	}

      //printf("\nVarname type=%02X", vartype);
      length_read += 1;

      //printf("\nLength read:%d out of %d", length_read, p->global_varname_size.size);

      // Add variable to list of globals
      p->external_varname[num_external_vars].type    = vartype;
      strcpy(p->external_varname[num_external_vars].varname, varname);
      
      num_external_vars++;      
    }


  p->external_varname_num = num_external_vars;

  //------------------------------------------------------------------------------
  //
  // Now read the string length fixup table
  //
  //------------------------------------------------------------------------------

  READ_ITEM( fp,  p->strlen_fixup_size, 1, NOBJ_STRLEN_FIXUP_SIZE, "\nError reading string length fixup size.");
  p->strlen_fixup_size.size = swap_uint16(p->strlen_fixup_size.size);
 
  length_read = 0;
  int num_strlen_fixup = 0;

  while (length_read < p->strlen_fixup_size.size )
    {
      NOBJ_ADDR             addr;
      NOBJ_STRLEN_FIXUP_LEN strlen;
      
      // Read address and strlen
      
      if(!read_item(fp, (void *)&addr, 1, sizeof(NOBJ_ADDR)))
	{
	  printf("\nError reading strlen fixup address field");
	  return;
	}

      //printf("\nVarname type=%02X", vartype);
      length_read += sizeof(NOBJ_ADDR);

      if(!read_item(fp, (void *)&strlen, 1, sizeof(NOBJ_STRLEN_FIXUP_LEN)))
	{
	  printf("\nError reading strlen fixup length field");
	  return;
	}

      //printf("\nVarname type=%02X", vartype);
      length_read += sizeof(NOBJ_STRLEN_FIXUP_LEN);

      //printf("\nLength read:%d out of %d", length_read, p->global_varname_size.size);

      // Add variable to list of globals
      p->strlen_fixup[num_strlen_fixup].address  = swap_uint16(addr);
      p->strlen_fixup[num_strlen_fixup].len      = strlen;
      
      num_strlen_fixup++;      
    }


  p->strlen_fixup_num = num_strlen_fixup;

  //------------------------------------------------------------------------------
  //
  // Now read the array size fixup table
  //
  //------------------------------------------------------------------------------

  READ_ITEM( fp,  p->arysz_fixup_size, 1, NOBJ_ARYSZ_FIXUP_SIZE, "\nError reading string length fixup size.");
  p->arysz_fixup_size.size = swap_uint16(p->arysz_fixup_size.size);
 
  length_read = 0;
  int num_arysz_fixup = 0;

  while (length_read < p->arysz_fixup_size.size )
    {
      NOBJ_ADDR             addr;
      NOBJ_ARYSZ_FIXUP_LEN arysz;
      
      // Read address and arysz
      
      if(!read_item(fp, (void *)&addr, 1, sizeof(NOBJ_ADDR)))
	{
	  printf("\nError reading arysz fixup address field");
	  return;
	}

      //printf("\nVarname type=%02X", vartype);
      length_read += sizeof(NOBJ_ADDR);

      if(!read_item(fp, (void *)&arysz, 1, sizeof(NOBJ_ARYSZ_FIXUP_LEN)))
	{
	  printf("\nError reading arysz fixup length field");
	  return;
	}

      //printf("\nVarname type=%02X", vartype);
      length_read += sizeof(NOBJ_ARYSZ_FIXUP_LEN);

      //printf("\nLength read:%d out of %d", length_read, p->global_varname_size.size);

      // Add variable to list of globals
      p->arysz_fixup[num_arysz_fixup].address  = swap_uint16(addr);
      p->arysz_fixup[num_arysz_fixup].len      = arysz;
      
      num_arysz_fixup++;      
    }



  p->arysz_fixup_num = num_arysz_fixup;

  //------------------------------------------------------------------------------
  //
  // QCode
  //
  // The QCode could be large, so malloc an area to store it rather than having a fixed
  // array size in the PROC structure
  //
  //------------------------------------------------------------------------------

  p->qcode = malloc(p->qcode_space_size.size);

  if( p->qcode == NULL )
    {
      printf("\nError mallocing QCode space.");
      return;
    }

  // Read the QCode in
  if(!read_item(fp, (void *)p->qcode, p->qcode_space_size.size, sizeof(uint8_t)))
    {
      printf("\nError reading QCode data");
      return;
    }
  
  printf("\nExit:%s", __FUNCTION__);

}

////////////////////////////////////////////////////////////////////////////////
//
// Decode variable types
//
////////////////////////////////////////////////////////////////////////////////

char res[7];

char *decode_vartype(NOBJ_VARTYPE t)
{
  switch(t)
    {
    case 0x00:
      strcpy(res, "INT   ");
      break;
    case 0x01:
      strcpy(res, "FLT   ");
      break;
    case 0x02:
      strcpy(res, "STR   ");
      break;
    case 0x03:
      strcpy(res, "INTARY");
      break;
    case 0x04:
      strcpy(res, "FLTARY");
      break;
    case 0x05:
      strcpy(res, "STRARY");
      break;
    default:
      strcpy(res, "??????");
      break;
      
    }
  
  return(res);
}

//------------------------------------------------------------------------------
//
// Initialises a mahine
//
//------------------------------------------------------------------------------

void init_machine(NOBJ_MACHINE *m)
{
  printf("\nInit machine...");
  
  // Set stack pointer
  // Stack grows from max index towards index 0
  // On the original organiser OPL the stack grows downwards, we do the same
  
  // Addresses of variables are referenced from the start of the proc area
  // on the stack anyway, so addresses have to be manipulated.

  m->sp = NOBJ_MACHINE_STACK_SIZE;
  printf("\nInit machine done.");
}

void error(char *fmt, ...)
{
  va_list valist;

  va_start(valist, fmt);

  vprintf(fmt, valist);
  va_end(valist);
  
  exit(-1);
}

////////////////////////////////////////////////////////////////////////////////

void push_machine_8(NOBJ_MACHINE *m, uint8_t v)
{
#if DEBUG_PUSH_POP
  printf("\n%s:pushing %02X SP:%04X", __FUNCTION__, v, m->sp);
#endif
  
  if( m->sp > 0 )
    {
      m->stack[(m->sp)--] = v;
    }
  else
    {
      error("Attempt to push off end of stack");
    }
}

// Pop byte off stack and return new stack pointer value.

uint16_t pop_sp_8(NOBJ_MACHINE *m, uint16_t sp, uint8_t *val)
{
  if( sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to pop from empty stack");
    }
  
  *val = m->stack[++sp];

#if DEBUG_PUSH_POP
  printf("\n%s:Popped %02X from SP:%04X", __FUNCTION__, *val, (m->sp)-1);
#endif

  return(sp);  
}

uint16_t pop_discard_sp_int(NOBJ_MACHINE *m, uint16_t sp)
{
  if( sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to pop from empty stack");
    }

  for(int i=0; i<2; i++)
    {
      ++sp;
      
#if DEBUG_PUSH_POP
      printf("\n%s:Popped from SP:%04X", __FUNCTION__, (m->sp)-1);
#endif
    }
  

  return(sp);  
}

uint16_t pop_discard_sp_float(NOBJ_MACHINE *m, uint16_t sp)
{
  if( sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to pop from empty stack");
    }

  for(int i=0; i<8; i++)
    {
      ++sp;
#if DEBUG_PUSH_POP
      printf("\n%s:Popped and discarded from SP:%04X", __FUNCTION__, (m->sp)-1);
#endif
      
    }

  return(sp);  
}

uint16_t pop_discard_sp_str(NOBJ_MACHINE *m, uint16_t sp)
{
  if( sp == NOBJ_MACHINE_STACK_SIZE )
    {
      error("\nAttempting to pop from empty stack");
    }

  uint8_t len_str;
  
  sp = pop_sp_8(m, sp, &len_str);
  
  for(int i=0; i<len_str; i++)
    {
      ++sp;
#if DEBUG_PUSH_POP
      printf("\n%s:Popped and discarded from SP:%04X", __FUNCTION__, (m->sp)-1);
#endif
      
    }

  return(sp);  
}

//------------------------------------------------------------------------------
//
// Takes a proc file and pushes it onto a machine language stack ready for
// execution. This is exactly what the original organiser does.
//
// 
//------------------------------------------------------------------------------

void push_proc_on_stack(NOBJ_PROC *p, NOBJ_MACHINE *m)
{
  uint8_t parm_cnt;

  printf("\nEnter:%s", __FUNCTION__);

  // Check if it is a language extension

  // Search for the procedure

  // Check there is sufficient memory
  // Is there more than 255 bytes free?
  if( !(m->sp > 255) )
    {
      error("\nOut of memory");
    }
  
  // Set new rta_sp and rta_fp

  // Check the parameter count on the stack agrees with what the procedure requires
  
  // Save the stack pointer
  uint16_t osp = pop_sp_8(m, m->sp, &parm_cnt);

  uint8_t vartype;
  
  // Check the parameter count
  if( parm_cnt != p->num_parameters.num )
    {
      error("\nParameter count does not match (%d in proc does not match %d on stack)", p->num_parameters.num, parm_cnt);
    }
  
  for(int i=0; i<parm_cnt; i++)
    {
      osp = pop_sp_8(m, osp, &vartype);

      printf("\nChecking parameter %d Type:%s", i, decode_vartype(vartype));
      
      // We just skip past the value 
      switch(vartype)
	{
	case NOBJ_VARTYPE_INT:
	  osp = pop_discard_sp_int(m, osp);
	  break;

	case NOBJ_VARTYPE_FLT:
	  osp = pop_discard_sp_float(m, osp);
	  break;

	case NOBJ_VARTYPE_STR:
	  osp = pop_discard_sp_str(m, osp);
	  break;

	case NOBJ_VARTYPE_INTARY:
	  break;

	case NOBJ_VARTYPE_FLTARY:
	  break;

	case NOBJ_VARTYPE_STRARY:
	  break;

	default:
	  error("\nBad variable type (%02X)", vartype);
	  break;
	}
    }
  
  // Check the parameter types

  // Set up the global variable tables

  // Set up the parameter table

  // Resolve externals and build external table

  // Zero all variable space

  // Fix-up strings

  // Fix up arrays

  // Load code

  // Set new rta_pc

  
  #if 0
  READ_ITEM(  fp, p->var_space_size, 1, NOBJ_VAR_SPACE_SIZE, "\nError reading var space size.");
  p->var_space_size.size = swap_uint16(p->var_space_size.size);

  READ_ITEM(fp, p->qcode_space_size,   1,                     NOBJ_QCODE_SPACE_SIZE, "\nError reading qcode space size.");
  p->qcode_space_size.size = swap_uint16(p->qcode_space_size.size);
  
  READ_ITEM(fp, p->num_parameters.num, 1,                     NOBJ_NUM_PARAMETERS,   "\nError reading number of parameters.");
  READ_ITEM(fp, p->parameter_types,    p->num_parameters.num, NOBJ_PARAMETER_TYPE,   "\nError reading parameter types.");

  READ_ITEM( fp,  p->global_varname_size, 1, NOBJ_GLOBAL_VARNAME_SIZE, "\nError reading global varname size.");
  p->global_varname_size.size = swap_uint16(p->global_varname_size.size);
  
  //  printf("\nGlobal varname size:%d", p->global_varname_size.size);

  //------------------------------------------------------------------------------
  // Global varname is more complicated to read. Each entry is length
  // prefixed, so read them until the length we have read matches the
  // size we have just read.
  //------------------------------------------------------------------------------
  
  int length_read = 0;
  int num_global_vars = 0;
  
  do
    {
      uint8_t len;
      uint8_t varname[NOBJ_VARNAME_MAXLEN];
      NOBJ_VARTYPE vartype;
      NOBJ_ADDR addr;
      vartype = 0;
      addr = 0;
      
      if(!read_item(fp, (void *)&len, 1, sizeof(len)))
	{
	  printf("\nError reading global varname entry length");
	  return;
	}

      memset(varname, 0, sizeof(varname));
      
      //      printf("\nVarname entry len=%d", len);
      
      length_read += sizeof(len);

      // 3 bytes on end are type and addr, so read rest of data into name field
      
      if(!read_item(fp, (void *)&varname, len, sizeof(uint8_t)))
	{
	  printf("\nError reading global varname entry name field");
	  return;
	}
      
      //printf("\nvarname='%s'", varname);

      length_read += len;

      // read type and addr

      if(!read_item(fp, (void *)&vartype, 1, sizeof(NOBJ_VARTYPE)))
	{
	  printf("\nError reading global varname entry type field");
	  return;
	}

      if(!read_item(fp, (void *)&addr, 1, sizeof(NOBJ_ADDR)))
	{
	  printf("\nError reading global varname entry addr field");
	  return;
	}

      //printf("\nVarname type=%02X", vartype);
      //printf("\nAddr addr=%02X", addr);
      length_read += 3;

      //printf("\nLength read:%d out of %d", length_read, p->global_varname_size.size);

      // Add variable to list of globals
      p->global_varname[num_global_vars].type    = vartype;
      p->global_varname[num_global_vars].address = swap_uint16(addr);
      strcpy(p->global_varname[num_global_vars].varname, varname);
      
      num_global_vars++;      
    }

  while (length_read < p->global_varname_size.size );

  p->global_varname_num = num_global_vars;

  //------------------------------------------------------------------------------
  //
  // Now read the external varname table
  //
  //------------------------------------------------------------------------------

  READ_ITEM( fp,  p->external_varname_size, 1, NOBJ_EXTERNAL_VARNAME_SIZE, "\nError reading external varname size.");
  p->external_varname_size.size = swap_uint16(p->external_varname_size.size);

  length_read = 0;
  int num_external_vars = 0;
  
  do
    {
      uint8_t len;
      uint8_t varname[NOBJ_VARNAME_MAXLEN];
      NOBJ_VARTYPE vartype;
      vartype = 0;
      
      if(!read_item(fp, (void *)&len, 1, sizeof(len)))
	{
	  printf("\nError reading external varname entry length");
	  return;
	}

      memset(varname, 0, sizeof(varname));
      
      //      printf("\nVarname entry len=%d", len);
      
      length_read += sizeof(len);

      // 3 bytes on end are type and addr, so read rest of data into name field
      
      if(!read_item(fp, (void *)&varname, len, sizeof(uint8_t)))
	{
	  printf("\nError reading global varname entry name field");
	  return;
	}
      
      //printf("\nvarname='%s'", varname);

      length_read += len;

      // read type and addr

      if(!read_item(fp, (void *)&vartype, 1, sizeof(NOBJ_VARTYPE)))
	{
	  printf("\nError reading global varname entry type field");
	  return;
	}

      //printf("\nVarname type=%02X", vartype);
      length_read += 1;

      //printf("\nLength read:%d out of %d", length_read, p->global_varname_size.size);

      // Add variable to list of globals
      p->external_varname[num_external_vars].type    = vartype;
      strcpy(p->external_varname[num_external_vars].varname, varname);
      
      num_external_vars++;      
    }

  while (length_read < p->external_varname_size.size );

  p->external_varname_num = num_external_vars;

  //------------------------------------------------------------------------------
  //
  // Now read the string length fixup table
  //
  //------------------------------------------------------------------------------

  READ_ITEM( fp,  p->strlen_fixup_size, 1, NOBJ_STRLEN_FIXUP_SIZE, "\nError reading string length fixup size.");
  p->strlen_fixup_size.size = swap_uint16(p->strlen_fixup_size.size);
 
  length_read = 0;
  int num_strlen_fixup = 0;
 
  do
    {
      NOBJ_ADDR             addr;
      NOBJ_STRLEN_FIXUP_LEN strlen;
      
      // Read address and strlen
      
      if(!read_item(fp, (void *)&addr, 1, sizeof(NOBJ_ADDR)))
	{
	  printf("\nError reading strlen fixup address field");
	  return;
	}

      //printf("\nVarname type=%02X", vartype);
      length_read += sizeof(NOBJ_ADDR);

      if(!read_item(fp, (void *)&strlen, 1, sizeof(NOBJ_STRLEN_FIXUP_LEN)))
	{
	  printf("\nError reading strlen fixup length field");
	  return;
	}

      //printf("\nVarname type=%02X", vartype);
      length_read += sizeof(NOBJ_STRLEN_FIXUP_LEN);

      //printf("\nLength read:%d out of %d", length_read, p->global_varname_size.size);

      // Add variable to list of globals
      p->strlen_fixup[num_strlen_fixup].address  = swap_uint16(addr);
      p->strlen_fixup[num_strlen_fixup].len      = strlen;
      
      num_strlen_fixup++;      
    }

  while (length_read < p->strlen_fixup_size.size );

  p->strlen_fixup_num = num_strlen_fixup;

  //------------------------------------------------------------------------------
  //
  // Now read the array size fixup table
  //
  //------------------------------------------------------------------------------

  READ_ITEM( fp,  p->arysz_fixup_size, 1, NOBJ_ARYSZ_FIXUP_SIZE, "\nError reading string length fixup size.");
  p->arysz_fixup_size.size = swap_uint16(p->arysz_fixup_size.size);
 
  length_read = 0;
  int num_arysz_fixup = 0;
 
  do
    {
      NOBJ_ADDR             addr;
      NOBJ_ARYSZ_FIXUP_LEN arysz;
      
      // Read address and arysz
      
      if(!read_item(fp, (void *)&addr, 1, sizeof(NOBJ_ADDR)))
	{
	  printf("\nError reading arysz fixup address field");
	  return;
	}

      //printf("\nVarname type=%02X", vartype);
      length_read += sizeof(NOBJ_ADDR);

      if(!read_item(fp, (void *)&arysz, 1, sizeof(NOBJ_ARYSZ_FIXUP_LEN)))
	{
	  printf("\nError reading arysz fixup length field");
	  return;
	}

      //printf("\nVarname type=%02X", vartype);
      length_read += sizeof(NOBJ_ARYSZ_FIXUP_LEN);

      //printf("\nLength read:%d out of %d", length_read, p->global_varname_size.size);

      // Add variable to list of globals
      p->arysz_fixup[num_arysz_fixup].address  = swap_uint16(addr);
      p->arysz_fixup[num_arysz_fixup].len      = arysz;
      
      num_arysz_fixup++;      
    }

  while (length_read < p->arysz_fixup_size.size );

  p->arysz_fixup_num = num_arysz_fixup;

  //------------------------------------------------------------------------------
  //
  // QCode
  //
  // The QCode could be large, so malloc an area to store it rather than having a fixed
  // array size in the PROC structure
  //
  //------------------------------------------------------------------------------

  p->qcode = malloc(p->qcode_space_size.size);

  if( p->qcode == NULL )
    {
      printf("\nError mallocing QCode space.");
      return;
    }

  // Read the QCode in
  if(!read_item(fp, (void *)p->qcode, p->qcode_space_size.size, sizeof(uint8_t)))
    {
      printf("\nError reading QCode data");
      return;
    }
#endif  
}
