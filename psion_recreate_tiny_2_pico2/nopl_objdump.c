//
// Dumps an object file to stdout
//
//

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

//#include "nopl.h"

#include "psion_recreate_all.h"

////////////////////////////////////////////////////////////////////////////////

//FIL *exdbfp;

FIL *op;

////////////////////////////////////////////////////////////////////////////////

char *type_to_string(NOBJ_VARTYPE t)
{
  switch(t)
    {
    case NOBJ_VARTYPE_INT:
      return("Integer");
      break;

    case NOBJ_VARTYPE_FLT:
      return("Float");
      break;

    case NOBJ_VARTYPE_STR:
      return("String");
      break;

    case NOBJ_VARTYPE_INTARY:
      return("Integer Array");
      break;

    case NOBJ_VARTYPE_FLTARY:
      return("Float Array");
      break;

    case NOBJ_VARTYPE_STRARY:
      return("String Array");
      break;

    case NOBJ_VARTYPE_VAR_ADDR:
      return("Variable Address");
      break;

    case NOBJ_VARTYPE_UNKNOWN:
      return("Unknown");
      break;

    case NOBJ_VARTYPE_VOID:
      return("Void");
      break;
      
    default:
      return("????");
      break;
    }
  
  return("????");
}

////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////

void dump_proc(NOBJ_PROC *proc)
{
  printf("\nEnter:%s", __FUNCTION__);

  pr_var_space_size(&(proc->var_space_size));
  pr_qcode_space_size(&(proc->qcode_space_size));
  pr_num_parameters(&(proc->num_parameters));
  pr_parameter_types(proc);
  pr_global_varname_size(&proc->global_varname_size);

  printf("\nGlobal variables (%d)", proc->global_varname_num);
  for(int i=0; i<proc->global_varname_num; i++)
    {
      printf("\n%2d: %-16s %s (%02X) %04X",
	     i,
	     proc->global_varname[i].varname,
	     decode_vartype(proc->global_varname[i].type),
	     proc->global_varname[i].type,
	     proc->global_varname[i].address
	     );
    }
  printf("\n");

  pr_external_varname_size(&proc->external_varname_size);

  printf("\nExternal variables (%d)", proc->external_varname_num);
  for(int i=0; i<proc->external_varname_num; i++)
    {
      printf("\n%2d: %-16s %s (%02X)",
	     i,
	     proc->external_varname[i].varname,
	     decode_vartype(proc->external_varname[i].type),
	     proc->external_varname[i].type
	     );
    }
  printf("\n");

  printf("\nString length fixups (%d)", proc->strlen_fixup_num);
  
  for(int i=0; i<proc->strlen_fixup_num; i++)
    {
      printf("\n%2d: %04X %02X",
	     i,
	     proc->strlen_fixup[i].address,
	     proc->strlen_fixup[i].len
	     );
    }
  printf("\n");

  printf("\nArray size fixups (%d)", proc->arysz_fixup_num);
  
  for(int i=0; i<proc->arysz_fixup_num; i++)
    {
      printf("\n%2d: %04X %04X",
	     i,
	     proc->arysz_fixup[i].address,
	     proc->arysz_fixup[i].len
	     );
    }
  printf("\n");
  
  // Now display the QCode
  NOBJ_QCODE *qc = proc->qcode;

  printf("\nQCode\n");
  if ( qc == 0 )
    {
      printf("\nNo QCode");
    }
  else
    {
      for(int i=0; i<proc->qcode_space_size.size; qc++, i++)
	{
	  int found = 0;
	  
	  // The fields that are after certain file qcodes needs a different decode
	  switch(*qc)
	    {
	    case QCO_OPEN:
	    case QCO_CREATE:
	      decode_qc(&i, &qc);

	      // Decode the field information
	      qc++;
	      i++;
	      printf("\n      Logical file:%02X (%c)", *qc, 'A'+*qc);
	      qc++;
	      i++;
	      
	      while( (*qc) != 0x88 )
		{
		  printf("\n        Type:%02X (%s)", *qc, type_to_string(*qc));
		  qc++;
		  i++;
		  printf("\n        ");

		  int len = *(qc++);
		  i++;
		  for(int ii=0; ii<len; ii++)
		    {
		      printf("%c", *(qc++));
		      i++;
		    }
		}
	      
	      decode_qc(&i, &qc);
	      break;

	    default:
	      decode_qc(&i, &qc);
	      break;
	    }
	}
    }
      
  printf("\n");
      
  printf("\nQCode Data\n");
      
  qc = proc->qcode;
      
  for(int i=0; i<proc->qcode_space_size.size; qc++, i++)
    {
      if( (i % 16)==0 )
	{
	  printf("\n%04X:", i);
	}
	  
      printf("%02X ", *qc);
    }
      
  printf("\n");
}


////////////////////////////////////////////////////////////////////////////////
//
// Dump an ob3 file and write the output to a xxx.od file
//

char filename[100] PSRAM;
char extension[10] PSRAM;
char name[100]     PSRAM;
char outfile[200]  PSRAM;
char buf[200]      PSRAM;

void nopl_objdump(char *filenm)
{
  NOBJ_PROC proc;
  FIL *fp;

  dp_cls();
  
  exdbfp = fopen("exec_db.txt", "w");

  if( exdbfp == NULL )
    {
      printf("\nCould not open 'exec_db.txt'\n");
      i_printxy_str(0, 0, "Could not open 'exec_db.txt'");
      kb_getk();
      return;
    }

  strcpy(filename, filenm);
  sscanf(filename, "%[^.].%s", name, extension);

  sprintf(outfile, "%s.%s.od", name, extension);

  sprintf(buf, "In :%s", filename);
  i_printxy_str(0, 0, buf);
  printf("\nIn : %s", filename);
  
  sprintf(buf, "Out:%s", outfile);
  i_printxy_str(0, 1, buf);
  printf("\nIn : %s", outfile);
      
#if 0
  if( (argc == 3) && (strcmp(argv[2], "--no-filenames")!=0),1 )
    {
      printf("\nFilename:'%s'", name);
      printf("\nFile ext:'%s", extension);
    }
#endif
  
  
  fp = fopen(filename, "r");
  
  if( fp == NULL )
    {
      printf("\nCannot open '%s'n", filename);
      return;
    }

  // Open oputput file
  op = fopen(outfile, "w");

  if( op == NULL )
    {
      printf("\nCannot open outfile '%s'n", outfile);
      return;
    }
  
  // If this is an OB3 file then drop the header
  if( strcmp(extension, "ORG") || strcmp(extension, "OB3") == 0 )
    {
      printf("\nDropping OB3 header...");
      read_ob3_header(fp);
    }

  printf("\nreading proc file\n");
  
  // Read the object file
  read_proc_file(fp, &proc);

  printf("\nDumping proc\n");
  
  // Dump the proc information
  dump_proc(&proc);
  
 
 fclose(fp);
 fclose(exdbfp);
 
}

#if 0
////////////////////////////////////////////////////////////////////////////////
//
// Dummy functions so that qcode.c can be linked in

void push_proc(FIL *fp, NOBJ_MACHINE *m, char *name, int top)
{
}

void display_machine(NOBJ_MACHINE *m)
{
}
#endif
