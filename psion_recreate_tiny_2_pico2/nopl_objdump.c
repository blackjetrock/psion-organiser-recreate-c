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


//------------------------------------------------------------------------------

void pr_uint8(uint8_t n)
{
  int l = (n & 0x00FF);

  ff_fprintf(op, "%02X", l);
}

void pr_var_space_size(NOBJ_VAR_SPACE_SIZE *x)
{
  ff_fprintf(op, "\nVar Space Size:%04X", x->size);
}

void pr_qcode_space_size(NOBJ_QCODE_SPACE_SIZE *x)
{
  ff_fprintf(op, "\nQCode Space Size:%04X", x->size);
}

void pr_global_varname_size(NOBJ_GLOBAL_VARNAME_SIZE *x)
{
  ff_fprintf(op, "\nGlobal varname Size:%04X", x->size);
}

void pr_external_varname_size(NOBJ_EXTERNAL_VARNAME_SIZE *x)
{
  ff_fprintf(op, "\nExternal varname Size:%04X", x->size);
}

void pr_num_parameters(NOBJ_NUM_PARAMETERS *x)
{
  ff_fprintf(op, "\nNumber of parameters:");
  pr_uint8(x->num);
}

void pr_parameter_types(NOBJ_PROC *p)
{
  ff_fprintf(op, "\nParameter types:");
  
  for(int i=0; i<p->num_parameters.num; i++)
    {
      ff_fprintf(op, "\n%2d %s (%d)",
	     i,
	     decode_vartype(p->parameter_types[i]),
	     p->parameter_types[i]
	     );
    }
}


void decode_qc(int *i,  NOBJ_QCODE **qc)
{
  ff_fprintf(op, "%s", decode_qc_txt(i, qc));
}

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
  ff_fprintf(op, "\nEnter:%s", __FUNCTION__);

  pr_var_space_size(&(proc->var_space_size));
  pr_qcode_space_size(&(proc->qcode_space_size));
  pr_num_parameters(&(proc->num_parameters));
  pr_parameter_types(proc);
  pr_global_varname_size(&proc->global_varname_size);

  ff_fprintf(op, "\nGlobal variables (%d)", proc->global_varname_num);
  for(int i=0; i<proc->global_varname_num; i++)
    {
      ff_fprintf(op, "\n%2d: %-16s %s (%02X) %04X",
	     i,
	     proc->global_varname[i].varname,
	     decode_vartype(proc->global_varname[i].type),
	     proc->global_varname[i].type,
	     proc->global_varname[i].address
	     );
    }
  ff_fprintf(op, "\n");

  pr_external_varname_size(&proc->external_varname_size);

  ff_fprintf(op, "\nExternal variables (%d)", proc->external_varname_num);
  for(int i=0; i<proc->external_varname_num; i++)
    {
      ff_fprintf(op, "\n%2d: %-16s %s (%02X)",
	     i,
	     proc->external_varname[i].varname,
	     decode_vartype(proc->external_varname[i].type),
	     proc->external_varname[i].type
	     );
    }
  ff_fprintf(op, "\n");

  ff_fprintf(op, "\nString length fixups (%d)", proc->strlen_fixup_num);
  
  for(int i=0; i<proc->strlen_fixup_num; i++)
    {
      ff_fprintf(op, "\n%2d: %04X %02X",
	     i,
	     proc->strlen_fixup[i].address,
	     proc->strlen_fixup[i].len
	     );
    }
  ff_fprintf(op, "\n");

  ff_fprintf(op, "\nArray size fixups (%d)", proc->arysz_fixup_num);
  
  for(int i=0; i<proc->arysz_fixup_num; i++)
    {
      ff_fprintf(op, "\n%2d: %04X %04X",
	     i,
	     proc->arysz_fixup[i].address,
	     proc->arysz_fixup[i].len
	     );
    }
  ff_fprintf(op, "\n");
  
  // Now display the QCode
  NOBJ_QCODE *qc = proc->qcode;

  ff_fprintf(op, "\nQCode\n");
  if ( qc == 0 )
    {
      ff_fprintf(op, "\nNo QCode");
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
	      ff_fprintf(op, "\n      Logical file:%02X (%c)", *qc, 'A'+*qc);
	      qc++;
	      i++;
	      
	      while( (*qc) != 0x88 )
		{
		  ff_fprintf(op, "\n        Type:%02X (%s)", *qc, type_to_string(*qc));
		  qc++;
		  i++;
		  ff_fprintf(op, "\n        ");

		  int len = *(qc++);
		  i++;
		  for(int ii=0; ii<len; ii++)
		    {
		      ff_fprintf(op, "%c", *(qc++));
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
      
  ff_fprintf(op, "\n");
      
  ff_fprintf(op, "\nQCode Data\n");
      
  qc = proc->qcode;
      
  for(int i=0; i<proc->qcode_space_size.size; qc++, i++)
    {
      if( (i % 16)==0 )
	{
	  ff_fprintf(op, "\n%04X:", i);
	}
	  
      ff_fprintf(op, "%02X ", *qc);
    }
      
  ff_fprintf(op, "\n");
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
      ff_fprintf(op, "\nCould not open 'exec_db.txt'\n");
      i_printxy_str(0, 0, "Could not open 'exec_db.txt'");
      kb_getk();
      return;
    }

  strcpy(filename, filenm);
  sscanf(filename, "%[^.].%s", name, extension);

  sprintf(outfile, "%s.%s.od", name, extension);

  ff_fprintf(op, buf, "In :%s", filename);
  i_printxy_str(0, 0, filename);
  ff_fprintf(op, "\nIn : %s", filename);
  
  ff_fprintf(op, buf, "Out:%s", outfile);
  i_printxy_str(0, 1, outfile);
  ff_fprintf(op, "\nIn : %s", outfile);
      
#if 0
  if( (argc == 3) && (strcmp(argv[2], "--no-filenames")!=0),1 )
    {
      ff_fprintf(op, "\nFilename:'%s'", name);
      ff_fprintf(op, "\nFile ext:'%s", extension);
    }
#endif
  
  
  fp = fopen(filename, "r");
  
  if( fp == NULL )
    {
      ff_fprintf(op, "\nCannot open '%s'n", filename);
      return;
    }

  // Open oputput file
  op = fopen(outfile, "w");

  if( op == NULL )
    {
      ff_fprintf(op, "\nCannot open outfile '%s'n", outfile);
      return;
    }
  
  // If this is an OB3 file then drop the header
  if( strcmp(extension, "ORG") || strcmp(extension, "OB3") == 0 )
    {
      ff_fprintf(op, "\nDropping OB3 header...");
      read_ob3_header(fp);
    }

  ff_fprintf(op, "\nreading proc file\n");
  
  // Read the object file
  read_proc_file(fp, &proc);

  ff_fprintf(op, "\nDumping proc\n");
  
  // Dump the proc information
  dump_proc(&proc);
  
 
 fclose(fp);
 fclose(exdbfp);
 fclose(op);
 
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
