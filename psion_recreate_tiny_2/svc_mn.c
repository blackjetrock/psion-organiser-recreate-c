
#include <string.h>

#include "nopl.h"

#define MENU_DB 0

////////////////////////////////////////////////////////////////////////////////
//
// Uses menustr to put a menu on the display, returns the index of
// the selected item, or -1 if menu was exited

int mn_menu(char *str)
{
  char sels[MAX_NOPL_MENU_SELS];
  char *sp;
  int num_sels = 0;
  int strx, stry;

  // Build up information about the menu
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
  char selstr[32][32];
  
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
  
#if 1
  printf("\nnumsels:%d", num_sels);
  
  for(int i=0; i<num_sels; i++)
    {
      //printf("\n%2d,", i);
      printf("\n%2d:'%c' %s", i, selchar[i], selstr[i]);
    }
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

  //------------------------------------------------------------------------------
  //
  // Process the menu, in the compiled manner
  //
  //------------------------------------------------------------------------------
  
#if TUI

  // TUI has a display that is more similar to the original Organiser
  
  // Wait for one of the selection letters to be pressed
  int done = 0;
  int done_srch = 0;
  int selnum = 0;
  int key;

  curs_set(3);
  
  // Display the menu, storing the positions of the entries
  for(int i=0; i<num_sels; i++)
    {
      wprintw(output_win, " ");
      getyx(output_win, iposy[i], iposx[i]);
      wprintw(output_win, selstr[i]);
      wprintw(output_win, " ");
    }
  
  wrefresh(output_win);

  keypad(stdscr, TRUE);

  while(!done)
    {
      //printf("\nLoop");
      mvwprintw(output_win, iposy[selnum], iposx[selnum], "");
      wrefresh(output_win);

      key = wgetch(stdscr);
      
      switch(key)
	{
	case ERR:
	  break;

	case 10:
	  done = 1;
	  if( (selnum >=0) && (selnum < num_sels) )
	    {
	      done = 1;
	    }
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
	  done_srch = 0;
	  
	  // We have a key that could be the first letter of an entry
	  int last_i = selnum>0? selnum-1 : num_sels-1;
	  
	  for(int i=selnum; (i != last_i) && !done_srch; i=((i+1) % num_sels))
	    {
	      //printf("\ni:%d selnum:%d", i, selnum);
	      if( key == sels[i] )
		{
		  if( how_many_have_sel[i] == 1 )
		    {
		      selnum = i;
		      done = 1;
		      break;
		    }
		  else
		    {
		      // Rotate around the entries and select with EXE (ENTER)
		      // Find next entry that starts with this letter
		      for(int j=((i+1) % num_sels); (j!=i); j=((j+1) % num_sels))
			{
			  //printf("\nj:%d", j);
			  if( key == sels[j] )
			    {
			      // Found it
			      selnum = j;
			      done_srch = 1;
			      break;
			    }
			}
		    }
		}
	    }
	  break;
	}
    }
  
  nodelay(stdscr, 0);
  
  return(selnum+1);
#else

  // On command line, print the selections and nunbers and allow the number to be selected
  for(int i=0; i<num_sels; i++)
    {
      printf("\n%d:%s", i+1, selstr[i]);
    }

  // Get integer and return it if valid
  char input[64];
  int result = -1;
  int done = 0;
  
  while(!done)
    {
      fgets(input, 63, stdin);
      sscanf(input, "%d", &result);

      if( (result>=1) && (result <= num_sels) )
	{
	  done = 1;
	}
    }
  return(result);
 
#endif

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

  keypad(stdscr, TRUE);

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
	case ERR:
	  break;

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
  
  nodelay(stdscr, 0);
  
  return(selnum);
}
