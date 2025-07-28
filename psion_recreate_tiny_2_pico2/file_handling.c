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

void file_handling_error(char *msg)
{
  dp_cls();
  i_printxy_str(0, 0, msg);
  sleep_ms(3000);
}

////////////////////////////////////////////////////////////////////////////////
//

void file_delete(char *filename, int ignore_error)
{
  FRESULT fr = f_unlink(filename);

  if( (fr != FR_OK) && !ignore_error )
    {
      file_handling_error("Unknown file");
      return;
    }

  // File has been deleted, or not
}

void file_rename(char *existing_filename, char *filename)
{
  FRESULT fr = f_rename(existing_filename, filename);

  if( fr != FR_OK )
    {
      file_handling_error("Could not rename");
      return;
    }

  // File has been renamed
}


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
FILE_LINE *line_ptr;
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

//------------------------------------------------------------------------------
//
// Write the file data to a file
//
FIL *outfp;

void file_write(char *filename)
{
  // Open sd card 
  run_mount(0, argv_null);
  
  // Open the file
  outfp = ff_fopen(filename, "w");

  if( first_line == NULL )
    {
      // Nothing to do
      ff_fclose(outfp);
      return;
    }

  // Run through the list freeing every thing
  linep = first_line;

  while(linep != NULL )
    {
      // Write line
      ff_fprintf(outfp, "%s\n", linep->text);

      linep = linep->next;
    }

  ff_fclose(outfp);
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

////////////////////////////////////////////////////////////////////////////////
//
// If line buffer is different then build a new node with the new details
// and insert it into the line linked list
//
// curline updated to point to new node
//

void file_edit_new_node_if_different(FILE_LINE **curlinep, char *linline)
{
  FILE_LINE *new_node;
  FILE_LINE *curline = *curlinep;
  
  if( strcmp(linline, curline->text) != 0 )
    {
      new_node = malloc(sizeof(FILE_LINE));

      if( new_node == NULL )
        {
          // Out of memory
          file_handling_error("Out of memory");
          return;
        }

      // Text for the node
      new_node->text = malloc(strlen(linline));

      if( new_node->text == NULL )
        {
          // Out of memory
          file_handling_error("Out of text memory");
          free(new_node);
          return;
        }
      
      // Fill in new node
      strcpy(new_node->text, linline);
      
      // Link it into the list
      if( curline->next == NULL )
        {
        }
      else
        {
          curline->next->prev = new_node;
        }

      new_node->next = curline->next;
          
      if( curline->prev == NULL )
        {
          first_line = new_node;
        }
      else
        {
          curline->prev->next = new_node;
        }

      new_node->prev = curline->prev;

      // Data copied, and new node linked in to the list
      // Free up the old node and data
      free(curline->text);
      free(curline);

      *curlinep = new_node;
    }
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
int curlineno = 0;
// Filename plus '.bak'
char backup_filename[NOBJ_FILENAME_MAXLEN+1+5];

void file_editor(char *filename)
{
  int k;
  int move_in_list = 0;
  int done = 0;
  int cursor_line = 0;
  
#if DB_FILE_EDITOR
  printf("\n%s:", __FUNCTION__);
#endif

  // Load file lines
  file_load(filename, &finfo);

  // Display and allow editing
  dp_cls();
  i_printxy_str(0, 0, "");

  printpos_x = 0;
  printpos_y = 0;
  
#if DB_FILE_EDITOR
  printf("\n%s:printpos_x:%d printpos_y:%d", __FUNCTION__, printpos_x, printpos_y);
#endif

  // The line that we are editing, we just display the other lines around it if
  // possible
  curline = first_line;
  
  while(!done)
    {
      // Edit line in a larger buffer so text can be added, then create a new node if
      // it has increased in size after edit
      strcpy(linline, curline->text);

      // Display lines to make curline appear at the cursor_line
      dp_cls();
      
      line_ptr = curline->prev;
      for(int i=0; i<cursor_line;i++)
        {
          if( line_ptr != NULL )
            {
              i_printxy_str(0, cursor_line-i-1, "");
              display_epos(line_ptr->text, "", 0, cursor_line-i-1, 0, ED_SINGLE_LINE);
              line_ptr = line_ptr->prev;
            }
        }

      line_ptr = curline->next;
      for(int i=cursor_line+1; i<display_num_lines();i++)
        {
          if( line_ptr != NULL )
            {
              i_printxy_str(0, i, "");
              display_epos(line_ptr->text, "", 0, i, 0, ED_SINGLE_LINE);
              line_ptr = line_ptr->next;
            }
        }

      i_printxy_str(0, cursor_line, "");
      k = ed_epos(linline, FH_LINE_MAX, ED_SINGLE_LINE, 0, cursor_line);

      switch(k)
        {
        case KEY_ON:
          done = 1;
          break;
          
        case KEY_EXE:
          break;

        case KEY_DOWN:
          // About to move to another line, check this one and see
          // if the editing buffer is different to th eline nodem if
          // it is then something has changed, so we create a new node
          // in case the size has changed and insert it into the list
          file_edit_new_node_if_different(&curline, linline);
          
          move_in_list = 1;
          if( cursor_line < display_num_lines()-1 )
            {
              cursor_line++;
            }
          break;
          
        case KEY_UP:
          // About to move to another line, check this one and see
          // if the editing buffer is different to th eline nodem if
          // it is then something has changed, so we create a new node
          // in case the size has changed and insert it into the list
          file_edit_new_node_if_different(&curline, linline);

          move_in_list = -1;
          if( cursor_line > 0 )
            {
              cursor_line--;
            }
          break;
        }

      if( move_in_list != 0 )
        {
          // Move forward along the linked list of lines
          if( move_in_list == 1 )
            {
              if( curline->next != NULL )
                {
                  curline = curline->next;
                }
              else
                {
                  // Moving off the end of the file
                  // Add a blank line at end of file

                  // Build node and add to list
                  FILE_LINE *new_node = malloc(sizeof(FILE_LINE));
      
                  if( new_node != NULL )
                    {
#if DB_FILE_EDITOR
                      printf("   new_node malloc OK (%p)", curline);
#endif
                      new_node->next = NULL;
                      new_node->prev = curline;
          
                      // line data
                      new_node->text = malloc(sizeof(""));

                      if( new_node->text != NULL)
                        {
#if DB_FILE_EDITOR
                          printf("  text malloc OK");
#endif
                          strcpy(new_node->text, "");

                          // Add to list
                          curline->next = new_node;
                        }
                    }
                }
            }
          
          // Move back along th elist of lines
          if( move_in_list == -1 )
            {
              if( curline->prev != NULL )
                {
                  curline = curline->prev;
                }
            }

          move_in_list = 0;
        }
    }

  // Exit from editing loop
  // Delete any backup
  strcpy(backup_filename, filename);
  strcat(backup_filename, ".bak");
  
  file_delete(backup_filename, FH_IGNORE_ERROR);
  file_rename(filename, backup_filename);
  
  // Write the new version of the file out
  file_write(filename);
  
  // Unload the file and free memory etc.
  file_unload();
}


////////////////////////////////////////////////////////////////////////////////
//
// Create a file
//
// We don't handle totally empty files at the moment, so create one with
// a blank line in it.

void file_create(char *filename)
{
  FIL *fp;

  // Check it doesn't already exist
  fp = fopen(filename, "r");

  if( fp != NULL )
    {
      fclose(fp);
      file_handling_error("File exists");
      return;
    }

  fp = fopen(filename, "w");
  ff_fprintf(fp, "%s:", filename);
  fclose(fp);
}
