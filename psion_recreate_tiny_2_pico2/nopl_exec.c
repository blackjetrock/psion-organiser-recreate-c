////////////////////////////////////////////////////////////////////////////////
//
// Mini II Psion OPL exec
//
//
////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate_all.h"

#define debug printf

////////////////////////////////////////////////////////////////////////////////

int do_single_step = 0;
FIL *exdbfp;

////////////////////////////////////////////////////////////////////////////////

NOBJ_MACHINE machine;
FIL *exdbfp;
FIL *fp;
NOBJ_PROC proc;

#define MAX_ARGS 16

typedef struct _ARG_INFO_ENTRY
{
  int type;
  char *value;
} ARG_INFO_ENTRY;

int arg_count = 0;
ARG_INFO_ENTRY arg_info[MAX_ARGS] PSRAM;

void push_parameters(NOBJ_MACHINE *m)
{
  debug("\nPush parameters...");

  
#if 0
  // parameters for the code in xx0.bin
  push_machine_8(m, 'C');
  push_machine_8(m, 'B');
  push_machine_8(m, 'A');
  push_machine_8(m, 0x03);
  push_machine_8(m, 0x02);

  push_machine_8(m, 0x12);
  push_machine_8(m, 0x00);
  push_machine_8(m, 0x00);

  push_machine_8(m, 0x00);
  push_machine_8(m, 0x01);
  push_machine_8(m, 0x17);
  push_machine_8(m, 0x50);
  push_machine_8(m, 0x00);
  push_machine_8(m, 0x00);
  push_machine_8(m, 0x00);
  push_machine_8(m, 0x00);
  push_machine_8(m, 0x01);

  // Number of parameters
  push_machine_8(m, 0x03);
#endif

#if 0
  // Push two floats on to the stack

#endif

#if 0
  push_machine_8(m, 'T');
  push_machine_8(m, 'S');
  push_machine_8(m, 'R');
  push_machine_8(m, 0x03);
  push_machine_8(m, 0x02);
  push_machine_8(m, 0x01);
  
#endif

#if 0
  push_machine_8(m, 'X');
  push_machine_8(m, 'X');
  push_machine_8(m, 'X');
  push_machine_8(m, ':');
  push_machine_8(m, 'A');
  push_machine_8(m, 0x03);
  push_machine_8(m, 0x00);
#endif

  int i;
  NOPL_FLOAT f;
  
  // Push parameters
  for(int p=0; p<arg_count; p++)
    {
      switch(arg_info[p].type)
	{
	case NOBJ_VARTYPE_INT:


	  sscanf(arg_info[p].value, "%d", &i);
	  printf("\ni=%d '%s'", i, arg_info[p].value);
	  push_machine_16(m, (uint16_t)i);

	  // Type
	  push_machine_8(m, NOBJ_VARTYPE_INT);
	  break;

	case NOBJ_VARTYPE_FLT:
	  f = num_from_text(arg_info[p].value);
	  push_machine_num(m, &f);
	  push_machine_8(m, arg_info[p].type);

	  // Type
	  push_machine_8(m, NOBJ_VARTYPE_FLT);
	  break;

	case NOBJ_VARTYPE_STR:
	  push_machine_string(m, strlen(arg_info[p].value), arg_info[p].value);
	  push_machine_8(m, arg_info[p].type);
	  
	  // Type
	  push_machine_8(m, NOBJ_VARTYPE_STR);
	  break;
	}
    }

  // Push number of parameters
  push_machine_8(m, arg_count);
  
  debug("\nPush parameters done.");

}


void print_machine_state(NOBJ_MACHINE *m)
{
  debug("\n  SP:%04X",              m->rta_sp);
  debug("\n  FP:%04X",              m->rta_fp);
  debug("\n  PC:%04X",              m->rta_pc);
  debug("\n");
}

////////////////////////////////////////////////////////////////////////////////
//
//
// Pushes a procedure onto the stack of a machine
//
// The procedure is in the form of an OB3 file, with file header
// and then QCode
//
//
////////////////////////////////////////////////////////////////////////////////

void push_proc(FIL *fp, NOBJ_MACHINE *m, char *name, int top)
{
  // We build a header for the frame
  int      base_sp           = 0;
  int      previous_fp       = m->rta_fp;
  char     device_ch         = *name;
  uint16_t size_of_variables = 0;
  uint16_t size_of_qcode     = 0;
  uint16_t size_of_global_table     = 0;
  uint16_t size_of_external_table     = 0;
  uint8_t  num_parameters;

  // Store the proc name for debug, it can be used to find the variable info file
  // for the proc, for instance.

  // Strip out the base name of the file
  char *first = name;
  char *last = name + (strlen(name)-1);

  printf("\nname:'%s'\n", name);
  
  for(int i=0; i<strlen(name); i++)
    {
      switch(*(name+i))
	{
	case ':':
	  first = name+i+1;
	  break;
	case '.':
	  last = name+i-1;
	  break;
	}
    }

  
  strncpy(current_proc, first, last-first+1);
  current_proc[last-first+2] = '\0';
  
  printf("\ncur_proc:'%s'\n", current_proc);
    
  debug("\n\n");
  debug("\n===================================Proc=========================================\n\n");
  debug("\nPushing procedure '%s'", name);
  debug("\n  Top:%d",               top);
  debug("\n  onto machine:");

  print_machine_state(m);
  
  // Get sizes from file
  if( !read_item_16(fp, &size_of_variables) )
    {
      // Error
    }

  
  if( !read_item_16(fp, &size_of_qcode) )
    {
      // Error
    }

  // Work out the base SP value
  // This contains data we set up:
  //     the global table
  //     the indirection table
  //
  // and data not in the OB3 file:
  //
  //     space for the variables
  //     space for the qcode
  
  base_sp += m->rta_sp - size_of_variables - 9 - size_of_qcode;

  if( !read_item(fp, &num_parameters, 1, sizeof(uint8_t)) )
    {
      // Error
    }

  debug("\nSize of variables    :%02X %d", size_of_variables, size_of_variables);
  debug("\nSize of QCode        :%02X %d", size_of_qcode, size_of_qcode);
  debug("\nNumber of parameters :%02X %d", num_parameters, num_parameters);
  debug("\nBase SP              :%02X %d", base_sp, base_sp);

  uint8_t par_type;
  uint8_t par_types[16];
  
  for(int np = 0; np<num_parameters; np++)
    {
      if( !read_item(fp, &par_type, 1, sizeof(uint8_t)) )
	{
	  // Error
	}
      
      // Store parameter types
      par_types[np] = par_type;
    }

  // Device
  if( top )
    {
      push_machine_8(m, 0x00);
    }
  else
    {
      push_machine_8(m, device_ch);
    }
  
  //  Return PC
  //  We may adjust this to point to the next qcode to execute.
  // The original doesn't do that, but we don't have the B register
  // to use to increment the PC.
  
  push_machine_16(m, m->rta_pc);

  // ONERR address
  push_machine_16(m, 0x0000);

  // Base SP. Used as SP if this is an ONERR procedure
  push_machine_16(m, base_sp);

  // FP points at the frame we are pushing
  m->rta_fp = m->rta_sp-2;

  debug("\nrta_fp = %04X", m->rta_fp);
  
  // Previous FP
  push_machine_16(m, previous_fp);

  // Get size of global table from file
  if( !read_item_16(fp, &size_of_global_table) )
    {
      // Error
    }
  
  debug("\nSize of global table:%02X (%d)", size_of_global_table, size_of_global_table);

  uint16_t start_of_global_table = m->rta_sp-size_of_global_table-2;

  debug("\nStart of global table:%04X (SP now %04X)", start_of_global_table, m->rta_sp);
  push_machine_16(m, start_of_global_table);

  //------------------------------------------------------------------------------
  // Zero out the variable area on the stack before we use it.
  //------------------------------------------------------------------------------

  // This does not clear the global table as that starts at the start and goes
  // higher in memory. It is fully written by the procedure load so doesn't need
  // clearing

#define ZERO_VALUE 0x22
  //#define END_ZEROES   (m->rta_fp + 9)
#define END_ZEROES   (start_of_global_table - 1)

  debug("\nZeroing stack from base_sp to start of global table");
  debug("\n%04X to %04X", base_sp, END_ZEROES);
  
  
  for(int va = base_sp; va <= END_ZEROES; va++)
    {
      m->stack[va] = ZERO_VALUE;
    }

  // Push global area on to stack Fill global area on stack (not push)
  int k = start_of_global_table;
      
  for(int i=0; i<size_of_global_table;)
    {
      int start_i = i;

      debug("\n%d %d", size_of_global_table, i);
      
      // Read variable data from file
      // Build up the record in the stack
      
      uint8_t gv_name_len;
      if( !read_item(fp, &gv_name_len, 1, sizeof(uint8_t)) )
	{
	  // Error
	}

      m->stack[k++] = gv_name_len;
      i++;
      
      debug("\n  Name length:%d", gv_name_len);

      debug("  '");
      for(int j=0; j<gv_name_len; j++)
	{
	  uint8_t gv_name_char;
	  if( !read_item(fp, &gv_name_char, 1, sizeof(uint8_t)) )
	    {
	      // Error
	    }

	  m->stack[k++] = gv_name_char;
	  i++;
	  
	  debug("%c", gv_name_char);
	}

      debug("' ");
      
      //      k += gv_name_len;

      uint8_t gv_type;
      if( !read_item(fp, &gv_type, 1, sizeof(uint8_t)) )
	{
	  // Error
	}

      m->stack[k++] = gv_type;
      i++;
      
      debug(" Type:%d", gv_type);
      
      uint16_t gv_addr;
      uint16_t gv_eff_addr;
      
      if( !read_item_16(fp, &gv_addr) )
	{
	  // Error
	}

      gv_eff_addr = (uint16_t)m->rta_fp + gv_addr;

      debug("\nEff %04X = %04X - %04X", (int)gv_eff_addr, (int)gv_addr, (int)base_sp);
      debug("  ADDR:%04X -> %04X", gv_addr, gv_eff_addr);

      m->stack[k++] = gv_eff_addr >> 8;
      m->stack[k++] = gv_eff_addr & 0xFF;
      i++;
      i++;
      
      for(int ii=start_i; ii<start_i+4+gv_name_len; ii++)
	{
	  debug("\n%04X: %02X", ii, m->stack[start_of_global_table+ii]);
	}

      //      i = k;
    }

  // Move SP on by size of global table
  m->rta_sp -= size_of_global_table;
  
  // Get size of external table from file
  if( !read_item_16(fp, &size_of_external_table) )
    {
      // Error
    }
  
  debug("\nSize of external table:%02X %d", size_of_external_table, size_of_external_table);

  //------------------------------------------------------------------------------
  // Push external area on to stack
  // The indirection area has all parameters and externals
  // We need to:
  //
  // Work through all parameters, placing the address of the parameter (on the stack)
  // into the table.
  // Work through the external table in the OB3 file and find the address
  // of the variable by looking through the linked frame list.
  //
  // The size of the table is known and accounted for in the allocation of
  // storage on the stack. The number of parameters is known, and externals are in
  // the external table in the object file.
  //
  // Parameters first
  //
  //------------------------------------------------------------------------------
  
  uint16_t par_ptr = m->rta_fp + 9;
  uint8_t  par_stack_type = 0;
  uint8_t  par_stack_num_pars = 0;

  // Check number of parameters correct
  if( m->stack[par_ptr] == num_parameters )
    {
      // Number of parameters is OK
    }
  else
    {
      debug("\nIncorrect number of parameters. Expected %d, got %d. SP:%04X", num_parameters, m->stack[par_ptr], par_ptr);
    }

  par_ptr++;
  for(int np = 0; np<num_parameters; np++)
    {
      
      // Get type, check it matches
      if( m->stack[par_ptr] == par_types[np] )
	{
	  // Type OK
	  debug("Type %d correct. Is %d. Stack %04X", m->stack[par_ptr], par_types[np], par_ptr);
	}
      else
	{
	  debug("Type %d incorrect. Should be %d. Stack %04X", m->stack[par_ptr], par_types[np], par_ptr);
	}

      par_ptr++;

      // We are pointing at the data, put this value in the table
      push_machine_16(m, par_ptr);

      // Skip past the data on the stack
      int par_data_len = datatype_length(par_types[np], m->stack[par_ptr]);
      
    }

  //------------------------------------------------------------------------------
  // Now run through the external table.
  
  debug("\nBuilding Indirection Table");
  debug("\nSearching for externals...");
	 
  for(int i=0; i<size_of_external_table;)
    {
      uint8_t ext_name_len;
      char    ext_name[NOBJ_VARNAME_MAXLEN];
      int ext_type;
      
      // Name length
      if( !read_item(fp, &ext_name_len, 1, sizeof(uint8_t)) )
	{
	  // Error
	}

      i++;

      uint8_t ext_name_char;
      
      // Read the name
      int j;
      
      for(j=0; j < ext_name_len; j++)
	{
	  // Name length
	  if( !read_item(fp, &ext_name_char, 1, sizeof(uint8_t)) )
	    {
	      // Error
	    }
	  
	  i++;
	  
	  ext_name[j] = ext_name_char;
	}

      ext_name[j] = '\0';

      debug("\nExternal:'%s'", ext_name);
      
      // Name length
      if( !read_item(fp, &ext_type, 1, sizeof(uint8_t)) )
	{
	  // Error
	}
      
      i++;

      // See if we can find this external global. Look in all the previous frames
      uint16_t  fpp       = m->rta_fp;
      int       ext_found = 0;
      uint16_t  glob_ptr;

      debug("\nFPP now %04X", fpp);
      
      while( stack_entry_16(m, fpp) != 0 )
	{
	  // Point to next frame
	  fpp = stack_entry_16(m, fpp);

	  debug("\nFPP now %04X", fpp);
	  
	  // Check the global table
	  glob_ptr = stack_entry_16(m, fpp - 2); //m->stack[(fpp - 2)];

	  uint16_t end_of_upper_table = fpp - 2;
	  uint16_t size_of_upper_table = end_of_upper_table - glob_ptr;
	  
	  debug("\n  Global table at %04X - %04X", glob_ptr, end_of_upper_table);
	  debug("\n    (Size %04X)", size_of_upper_table);
	  
	  for(int gi = 0; gi<size_of_upper_table;)
	    {
	      // Get name length
	      uint8_t glob_name_len = m->stack[glob_ptr+gi];
	      char    glob_name[NOBJ_VARNAME_MAXLEN];
	      int     glob_type;

	      gi++;
	      
	      // Read the name
	      int j;
	      
	      for(j=0; j < glob_name_len; j++)
		{
		  glob_name[j] = m->stack[glob_ptr+(gi++)];
		}
	      
	      glob_name[j] = '\0';
			  
	      debug("\n  Global name:'%s'", glob_name);

	      if( strcmp(glob_name, ext_name)==0 )
		{
		  // Found the global, get the address and put it in indirection table
		  debug("\n  Found global");

		  uint16_t glob_addr;

		  gi++;
		  glob_addr = stack_entry_16(m, glob_ptr + gi);
		  gi += 2;
		  
		  //		  glob_addr  = m->stack[glob_ptr+(gi++)] << 8;
		  //glob_addr |= m->stack[glob_ptr+(gi++)] &  0x0F;

		  // Copy to table
		  push_machine_16(m, glob_addr);

		  debug("\n  Address:%04X", glob_addr);

		  ext_found = 1;
		}
	    }

	  if( ext_found )
	    {
	      // All OK
	      debug("\nExternal found:%s", ext_name);
	    }
	  else
	    {
	      debug("\nExternal NOT found:%s", ext_name);

	      // ==========    Add a dummy entry
	      push_machine_16(m, 0x0000);
	    }
	}
    }

  // Variables go here
  // The variable space is already in place, the SP needs to move after it eventually.
  // The variable space has been zeroed.

  // Apply fix-ups
  debug("\nApplying fixups...");
  
  // String fixups
  uint16_t num_fixups;

  if( !read_item_16(fp, &num_fixups) )
    {
      // Error
    }

  debug("\nNumber of string fixups:%04X, %d", num_fixups/3, num_fixups/3);
  
  for(int j=0; j < num_fixups/3; j++)
    {
      // Address (offset from FP)
      uint16_t addr;
      uint8_t fixdata;
      
      if( !read_item_16(fp, &addr) )
	{
	  // Error
	}

      if( !read_item(fp, &fixdata, 1, sizeof(uint8_t)) )
	{
	  // Error
	}

      debug("\nFixup at %04X, data %02X, add:%04X", addr, fixdata, m->rta_fp+addr);

      if( ((uint16_t)(m->rta_fp+addr) < NOBJ_MACHINE_STACK_SIZE) || ((uint16_t)(m->rta_fp+addr) < 0) )
	{
	  m->stack[(uint16_t)(m->rta_fp+addr)] = fixdata;
	}
      else
	{
	  debug("\n*** Bad fixup at %04X", (uint16_t)(m->rta_fp+addr));
	}
    }

  // Array fixups

  if( !read_item_16(fp, &num_fixups) )
    {
      // Error
    }

  debug("\nNumber of array fixups:%04X, %d", num_fixups/4, num_fixups/4);
  
  for(int j=0; j < num_fixups/4; j++)
    {
      // Address (offset from FP)
      uint16_t addr;
      uint16_t fixdata;
      
      if( !read_item_16(fp, &addr) )
	{
	  // Error
	}

      if( !read_item_16(fp, &fixdata) )
	{
	  // Error
	}

      debug("\nFixup at %04X, data %04X, addr %04X", addr, fixdata, (m->rta_fp+addr+0) & 0xFFFF);

      m->stack[(uint16_t)(m->rta_fp+addr+0)] = fixdata << 8;
      m->stack[(uint16_t)(m->rta_fp+addr+1)] = fixdata  & 0x0F;
    }

  // QCode goes here

  for(int i=0; i<size_of_qcode; i++)
    {
      uint8_t qcode_byte;
      
      if( !read_item(fp, &qcode_byte, 1, sizeof(uint8_t)) )
	{
	  // Error
	}

      // Offset of two bytes to match technical manual (seems to be included in qcode size)
      m->stack[base_sp+i-2] = qcode_byte;
    }
  
  // Leave the stack at the end of the procedure
  // Move down by a byte so the stack doesn't overwrite the qcode
  
  m->rta_sp = base_sp;

  // Put the PC at the start of the QCode
  m->rta_pc = base_sp;
  

}

////////////////////////////////////////////////////////////////////////////////
//
// Displays the contents of a machine 
//
////////////////////////////////////////////////////////////////////////////////

// Displays from the last procedure pushed on the stack

void display_machine_procs(NOBJ_MACHINE *m)
{
  uint16_t fp = m->rta_fp;
  uint16_t fp_next;
  
  int sp = m->rta_sp;
  uint16_t dp;
  uint16_t val;
  uint8_t val8;

  debug("\n================================================================================\n");

  print_machine_state(m);
 
  debug("\n\nMachine Procedures:");
	
  while(fp != 0 )
    {
      // Get procedure name
      
      // Get data about this proc

      fp_next = stack_entry_16(m, fp);
      debug("\n%04X:  Previous FP        : %04X", fp+FP_OFF_NEXT_FP,    fp_next);
      debug("\n%04X:  Base SP            : %04X", fp+FP_OFF_BASE_SP,    stack_entry_16(m, fp+FP_OFF_BASE_SP));
      debug("\n%04X:  ONERR Address      : %04X", fp+FP_OFF_ONERR,      stack_entry_16(m, fp+FP_OFF_ONERR));
      debug("\n%04X:  Return PC          : %04X", fp+FP_OFF_RETURN_PC,  stack_entry_16(m, fp+FP_OFF_RETURN_PC));
      debug("\n%04X:  Device             : %02X", fp+FP_OFF_DEVICE,     stack_entry_8 (m, fp+FP_OFF_DEVICE));
      debug("\n%04X:  Global Table Start : %04X", fp+FP_OFF_GLOB_START, stack_entry_16(m, fp+FP_OFF_GLOB_START));
      debug("\n");
	     
      // Move to next procedure
      fp = fp_next;
    }
  
  debug("\n================================================================================\n");
}

////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////

void display_machine(NOBJ_MACHINE *m)
{
  char dch;
  uint8_t b;
  


  // Dump the stack
  for(int i= NOBJ_MACHINE_STACK_SIZE; i>=(m->rta_sp); i--)
    {
      b = m->stack[i];
      
      if( isprint(b) )
	{
	  dch = b;
	}
      else
	{
	  dch = '.';
	}
      
      debug("\n%04X: %02X %c", i, b, dch);
    }
  
  debug("\n");

  display_machine_procs(m);
}

void nopl_init(void)
{
  printf("\nInitialising newopl...");
  return;
  
  //  run_mount();
  
  exdbfp = fopen("exec_db.txt", "w");

  if( exdbfp == NULL )
    {
      printf("\nCould not open 'exec_db.txt'\n");
    }
}

////////////////////////////////////////////////////////////////////////////////
//
//
//
//
////////////////////////////////////////////////////////////////////////////////

int nopl_exec(char *filename)
{
  FIL *fp;

  printf("\nExecuting %s", filename);
  
  exdbfp = ff_fopen("exec_db.txt", "w");

  if( exdbfp == NULL )
    {
      printf("\nCould not open 'exec_db.txt'\n");
      return(0);
    }

  printf("\nSize of NOBJ_PROC:%ld", sizeof(NOBJ_PROC));
  
  // Load the procedure file
  fp = ff_fopen(filename, "r");

  if( fp == NULL )
    {
      printf("\nCannot open '%s'", filename);
      return(0);
    }

  debug("\nLoaded '%s'", filename);

#if 0
  if( argc > 2 )
    {
      switch(argv[2][0])
	{
	case 't':
	  break;
	  
	case 'n':
	  do_single_step = 1;
	  break;

	case 'p':
	  // Parameters, list ends with a '.' or end of args
	  
	  break;
	}
    }
#endif

#if 0
  // File name of .ob3 to run is followed, optionally by parameter values
  // Build parameter table
  // Parameters are prefixed by a type character so we can have things like:
  //
  //  a string parameter "1.2"
  //
  // This is s1.2
  //
  // Similarly:
  //
  // i10
  // f123.45
  // f100
  //
      
  printf("\nParameters");
  
  for(int a=2; a<argc; a++)
    {
      // Find type of arg
      printf("\n<%s>", argv[a]);

      switch(argv[a][0])
	{
	case 'i':
	  arg_info[arg_count].type = NOBJ_VARTYPE_INT;
	  break;
	  
	case 'f':
	  arg_info[arg_count].type = NOBJ_VARTYPE_FLT;
	  break;

	case 's':
	  arg_info[arg_count].type = NOBJ_VARTYPE_STR;
	  break;
	  
	default:
	  printf("\nUnknown parameter type. Prefix parameter vaues with type char");
	  printf("\ne.g.   f1.2, i100, sabc, s1.2");
	  break;
	}
      
      arg_info[arg_count].value = &(argv[a][1]);
      arg_count++;
    }

#endif
  
  printf("\n%d parameters loaded", arg_count);
  for(int p=0; p<arg_count; p++)
    {
      printf("\n%d: %s TYPE:%d", p, arg_info[p].value, arg_info[p].type);
    }

  printf("\n");
  
#ifdef TUI
  printf("tui init");
  tui_init();
#endif
  
  // Discard header
  read_ob3_header(fp);

  num_init();
  qcode_init();
  
  // Initialise the machine
  init_machine(&machine);
  init_logical_files();
  
  // Put some parameters on the stack for our test code
  push_parameters(&machine);

  // Push proc onto stack
  push_proc(fp, &machine, filename, 1);

  // Execute it
  current_machine = &machine;

  debug("\n\n");

  // Clear screen before qcode is run
  dp_cls();
  
  execute_qcode(&machine, do_single_step);
  
  debug("\n\n");
  
  display_machine(&machine);
  
  debug("\n");

  //ff_fclose(exdbfp);

#ifdef TUI
  tui_end();
#endif

  num_uninit();
  
  ff_fclose(exdbfp);
  ff_fclose(fp);
  
  return(1);
}
