////////////////////////////////////////////////////////////////////////////////
//
// File Handling
//
// We need to access files, but want to keep stack use low.
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

////////////////////////////////////////////////////////////////////////////////
//
// Load a file into memory
// Lines are then accessed by asking for them by line number
// We want to use little stack, and lines are going to be of varying lengths, probably small
// so we have to store them as variable length.
//
// Linked list of lines
//

FILE_LINE *first_line;
FIL *linfp;
char linline[FH_LINE_MAX+1] = "";
FILE_LINE *lnode;
char *line_text;
FILE_LINE *curline;
FILE_LINE *fh_last_line = NULL;
FILE_LINE *linep;

int is_first_line;

int file_load(char *filename, FILE_INFO *fi)
{
#if DB_FILE_LOAD
  printf("\n%s:", __FUNCTION__);
#endif

  run_mount(0, argv_null);
     
  is_first_line = 1;

  // Open the file and read N lines in.
  linfp = fopen(filename, "r");
	  
  if( linfp == NULL )
    {
#if DB_FILE_LOAD
      printf("Cannot open '%s'", filename);
#endif
      return(false);    
    }
  
    while( !ff_feof(linfp) )
    {
      ff_fgets(linline, FH_LINE_MAX, linfp);

      // Lose the newline at the end
      linline[strlen(linline)-1] = '\0';
      
#if DB_FILE_LOAD
      printf("\n%s", linline);
#endif

      // Build node and add to list
      curline = malloc(sizeof(FILE_LINE));
      
      if( curline != NULL )
        {
#if DB_FILE_LOAD
          printf("   curline malloc OK (%p)", curline);
#endif

          curline->next = NULL;
          curline->prev = NULL;
          
          // line data
          line_text = malloc(strlen(linline)+1);

          if( line_text != NULL)
            {
#if DB_FILE_LOAD
      printf("  line_text malloc OK");
#endif

              strcpy(line_text, linline);
              
              curline->text = line_text;

              // Add to list
              if( is_first_line )
                {
                  is_first_line = 0;
                  first_line = curline;
                  curline->prev = NULL;
                  fh_last_line = curline;
                }
              else
                {
                  // Link into list
#if DB_FILE_LOAD
                  printf("\n%s:Linking into list fh_last_line=%p", fh_last_line);
#endif

                  curline->prev = fh_last_line;
                  fh_last_line->next = curline;

                  fh_last_line = curline;
                }
            }
        }
    }

  fclose(linfp);
  
  return(true);
}

//------------------------------------------------------------------------------
//
// free up the file line list
void file_unload(void)
{
  if( first_line == NULL )
    {
      // Nothing to do
      return;
    }

  // Run through the list freeing every thing
  linep = first_line;

  while(linep != NULL )
    {
#if DB_FILE_LOAD
      printf("\n%s: FreeHeap:%d addr:%p n:%p p:%p '%s'", __FUNCTION__, getFreeHeap(), linep, linep->next, linep->prev, linep->text);
#endif

      free(linep->text);

      // Get pointer before we free it
      curline = linep->next;
#if DB_FILE_LOAD
  printf("   Freeing:%p", linep);
#endif

      free(linep);
      linep = curline;
    }
  
#if DB_FILE_LOAD
  printf("\n%s: FreeHeap:%d", __FUNCTION__, getFreeHeap());
#endif

  first_line = NULL;
  return;
}

FILE_LINE *linep = NULL;

void file_dump_list(void)
{
  printf("\nFile handling line list:\n");
  
  if( first_line == NULL )
    {
      printf("\nLine list empty\n");
      return;
    }

  linep = first_line;
  int i = 1;
  
  while(linep != NULL )
    {
      printf("\n%d: addr:%p n:%p p:%p '%s'", i++, linep, linep->next, linep->prev, linep->text);
      linep = linep->next;
    }

  printf("\nEnd of list\n");
  
}


//------------------------------------------------------------------------------
//
// We want a file editor, but don't want to use a lot of stack. Create a pool of
// lines in the external RAM that we load the file into. This is a simple N lines
// for now, but a window of lines could be maintained later, allowing large files
// to be edited. The problem is rewriting lines that have changed in size.
//
// Note lines are delimited by newlines in the text file but tabs in the internal
// code.

FILE_INFO finfo;

void file_editor(char *filename)
{
  int k;
  int done = 0;
  
#if DB_FILE_EDITOR
  printf("\n%s:", __FUNCTION__);
#endif

  // Load file lines
  file_load(filename, &finfo);

  // Display and allow editing
  dp_cls();
  printxy_str(0, 0, "");

  curline = first_line;
  
  while(!done)
    {
      // Edit line in a larger buffer so text can be added, then create a new node if
      // it has increased in size after edit
      strcpy(linline, curline->text);
      
      k = ed_epos(linline, FH_LINE_MAX, 1, 0);

      switch(k)
        {
        case KEY_ON:
          done = 1;
          break;
          
        case KEY_EXE:
          break;
        }
    }
  
  // Unload the file and free memory etc.
  file_unload();
}
