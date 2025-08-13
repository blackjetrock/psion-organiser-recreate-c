
#include <string.h>

#include "psion_recreate_all.h"

////////////////////////////////////////////////////////////////////////////////
//
// Uses menustr to put a menu on the display, returns the index of
// the selected item, or -1 if menu was exited

// Avoid stack use by not making these local

char sels[MAX_NOPL_MENU_SELS];
char *sp;
int num_sels = 0;
int strx, stry;

int ipos[32];
int iposx[32];
int iposy[32];
char selchar[32];
int how_many_have_sel[32];  
char frag[2] = " ";
char selstr[32][32];

int mn_menu(char *str)
{
  int i=0;
  num_sels = 0;
  
  // Build up information about the menu
  // Find the selection letters
  sp = str;

  for(int i=0; i<32; i++)
    {
      how_many_have_sel[i] = 0;
    }
  
  // Find each of the menu items, print them and store their first characters
  // and the cursor positions of those characters.
  int delta = 0;
  int start = 0;
  
  while( *sp != '\0')
    {
      if( (i==0) || (*sp == ',') || (*sp == ' ') )
	{
	  if( i==0 )
	    {
	      delta = 0;
	      start = 0;
	      
	    }
	  else
	    {
	      delta = 1;

	      // Copy entry string
	      strncpy(&(selstr[num_sels-1][0]),  str+start, i-start);
	      selstr[num_sels-1][i-start] = '\0';
	      start = i+1;
	    }

	  //printf("\nFound:%c numsels:%d", *(sp+delta), num_sels);
	  
	  selchar[num_sels] = *(sp+delta);
	  sels[num_sels] = *(sp+delta);
	  how_many_have_sel[num_sels] = 1;
	  
	  for(int h=0; h<num_sels; h++)
	    {
	      if( *(sp+delta) == selchar[h] )
		{
		  how_many_have_sel[h]++;
		}
	    }

	  num_sels++;
	}
      else
	{
	}
      
      sp++;
      i++;
    }

  strncpy(&(selstr[num_sels-1][0]),  str+start, i-start);
  selstr[num_sels-1][i-start] = '\0';
  
  // Correct the totals for the first characters of the entries
  for(int i=0; i<num_sels; i++)
    {
      for(int j=0; j<num_sels;j++)
	{
	  if(selchar[i] == selchar[j] )
	    {
	      if( how_many_have_sel[i] > how_many_have_sel[j] )
		{
		  how_many_have_sel[j] = how_many_have_sel[i];
		}
	    }
	}
    }
  
#if DB_MN_MENU
  printf("\nnumsels:%d", num_sels);
  
  for(int i=0; i<num_sels; i++)
    {
      //printf("\n%2d,", i);
      printf("\n%2d:'%c' %s", i, selchar[i], selstr[i]);
    }
#endif
  
#if DB_MN_MENU
  // Dump all the data gathered
  printf("\nStr:'%s'", str);
  
  for(int s=0; s<num_sels; s++)
    {
      printf("\n%02i:selchar:'%c' X,y:%d,%d", s,  selchar[s], iposx[s], iposy[s]);
    }
  printf("\n...\n");

#endif

  //------------------------------------------------------------------------------
  //
  // Process the menu, in the compiled manner
  //
  //------------------------------------------------------------------------------

  // Wait for one of the selection letters to be pressed
  int done = 0;
  int done_srch = 0;
  unsigned int selnum = 0;
  int key;

  dp_cls();
  
  //  curs_set(3);
  
  // Display the menu, storing the positions of the entries
  for(int i=0; i<num_sels; i++)
    {
      dp_prnt(" ");
      iposy[i] = printpos_y;
      iposx[i] = printpos_x;
      
      dp_prnt(selstr[i]);
      //dp_prnt(" ");
    }
  
  //  keypad(stdscr, TRUE);

  // Cursor on
  cursor_on();
  cursor_blink = 1;
  
  while(!done)
    {
      tight_loop_tasks();
      
      cursor_y = iposy[selnum];
      cursor_x = iposx[selnum];
      
      if( (kb_test() != KEY_NONE) )
        {
          key = kb_getk();
          
          switch(key)
            {
            case KEY_EXE:
              done = 1;
              if( (selnum >=0) && (selnum < num_sels) )
                {
                  done = 1;
                }
              break;
              
            case KEY_ON:
              // We add one to selnum for return value
              selnum = -1;
              done = 1;
#if DB_MN_MENU
              printf("\nKey:KEY_ON");
#endif
              break;

            case KEY_RIGHT:
              selnum = (selnum + 1) % num_sels;
              break;

            case KEY_LEFT:
              selnum = (selnum - 1) % num_sels;
              break;

              
            default:
              done_srch = 0;

              // We have a key that could be the first letter of an entry
              int last_i = selnum>0? selnum-1 : num_sels-1;

#if DB_MN_MENU
              printf("\nKey:%c", key);
              printf("\nlast_i:%d selnum:%d", last_i, selnum);
#endif
              
              for(int i=selnum; !done_srch; i=((i+1) % num_sels))
                {
#if DB_MN_MENU
                  printf("\ni:%d selnum:%d", i, selnum);
#endif
                  if( key == sels[i] )
                    {
                      if( how_many_have_sel[i] == 1 )
                        {
                          selnum = i;
                          done_srch = 1;
                          done = 1;
                          break;
                        }
                      else
                        {
                          // Rotate around the entries and select with EXE (ENTER)
                          // Find next entry that starts with this letter
                          for(int j=((i+1) % num_sels); (j!=i); j=((j+1) % num_sels))
                            {
#if DB_MN_MENU
                              printf("\nj:%d", j);
#endif
                              if( key == sels[j] )
                                {
                                  // Found it
#if DB_MN_MENU
                                  printf("\nFound key");
#endif

                                  selnum = j;
                                  done_srch = 1;
                                  break;
                                }
                            }
                        }
                    }

                  // have we processed the last entry we will look at?
                  if( i == (last_i) )
                    {
                      done_srch = 1;
                      done = 1;
                    }
                }
              break;
            }
        }
    }

#if DB_MN_MENU
  printf("\nSelected:%d", selnum+1);
#endif

  // Cursor off again
  cursor_off();
  
  // Menu command returns 1..n
  return(selnum+1);
}


int mn_menu2(char *str)
{
  char sels[MAX_NOPL_MENU_SELS];
  char *sp;
  int num_sels = 0;
  int strx, stry;

#if TUI
  //getyx(output_win, stry, strx);
 
  // Display menu string

  //wprintw(output_win, "%s\n", str);

  //wrefresh(output_win);
#else
#if MENU_DB
  printf("\n%s\n", str);
#endif
#endif
  
  // Find the selection letters
  sp = str;
  //  sels[num_sels++] = *sp;
  
  int i=0;
  int ipos[32];
  int iposx[32];
  int iposy[32];
  char selchar[32];
  int how_many_have_sel[32];  
  char frag[2] = " ";

  for(int i=0; i<32; i++)
    {
      how_many_have_sel[i] = 0;
    }
  
  // Find each of the menu items, print them and store their first characters
  // and the cursor positions of those characters.
  int delta = 0;
  
  while( *sp != '\0')
    {
      if( (i==0) || (*sp == ',') || (*sp == ' ') )
	{
	  if( i==0 )
	    {
	      delta = 0;
	    }
	  else
	    {
	      delta = 1;
	    }

	  //printf("\nFound:%c numsels:%d", *(sp+delta), num_sels);
	  
	  selchar[num_sels] = *(sp+delta);
	  sels[num_sels] = *(sp+delta);
	  how_many_have_sel[num_sels] = 1;
	  
	  for(int h=0; h<num_sels; h++)
	    {
	      if( *(sp+delta) == selchar[h] )
		{
		  how_many_have_sel[h]++;
		}
	    }

#if TUI
	  getyx(output_win, iposy[num_sels], iposx[num_sels]);
#endif
	  frag[0] = *sp;
#if TUI
	  wprintw(output_win, "%s", frag);
#else
	  printf("%s", frag);
#endif
	  num_sels++;
	}
      else
	{
	  frag[0] = *sp;
#if TUI
	  wprintw(output_win, "%s", frag);
#else
	  printf("%s", frag);
#endif
	}
      
      sp++;
      i++;
    }

  // Correct the totals for the first characters of the entries
  for(int i=0; i<num_sels; i++)
    {
      for(int j=0; j<num_sels;j++)
	{
	  if(selchar[i] == selchar[j] )
	    {
	      if( how_many_have_sel[i] > how_many_have_sel[j] )
		{
		  how_many_have_sel[j] = how_many_have_sel[i];
		}
	    }
	}
    }
  
#if 1
  printf("\nnumsels:%d", num_sels);
  
  for(int i=0; i<num_sels; i++)
    {
      //printf("\n%2d,", i);
      printf("\n%2d:'%c'%d", i, selchar[i], how_many_have_sel[i]);
    }
#endif
  
#if TUI
  wprintw(output_win, "\n");
  wrefresh(output_win);
#else
  printf("\n");
#endif
  
#if MENU_DB
  // Dump all the data gathered
  printf("\nStr:'%s'", str);
  
  for(int s=0; s<num_sels; s++)
    {
      printf("\n%02i:selchar:'%c' X,y:%d,%d", s,  selchar[s], iposx[s], iposy[s]);
    }
  printf("\n...\n");

#endif
  
  // Wait for one of the selection letters to be pressed
  int done = 0;
  int selnum = 0;
  int key;

#if TUI
  keypad(stdscr, TRUE);
#endif
  
  while(!done)
    {
#if TUI
      mvprintw(stry, strx+ipos[selnum], "");
      wrefresh(output_win);
#endif
      
#if TUI
      key = wgetch(stdscr);
#else
      key = fgetc(stdin);
#endif
      
      switch(key)
	{
#if TUI          
	case ERR:
	  break;
#endif
          
	case 13:
	  printf("\nEnter");
	  break;
	  
	case 27:
	  selnum = 0;
	  done = 1;
	  break;

	default:
#if 0
	  wprintw(output_win, "\nKey:%02X", key);
	  wrefresh(output_win);
#endif
	  for(int i=selnum; (i!=selnum-1)&&!done; i=((i+1) % num_sels))
	    {
	      if( key == sels[i] )
		{
		  if( how_many_have_sel[i] == 1 )
		    {
		      selnum = i;
		      done = 1;
		    }
		  else
		    {
		      // rotate around the entries and select with EXE (ENTER)
		      
		    }
		}
	    }
	  break;
	}
    }

#if TUI
  nodelay(stdscr, 0);
#endif
  
  return(selnum);
}
