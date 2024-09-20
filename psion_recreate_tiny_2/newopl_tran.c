// NewOPL Translater

// Translates NewOPL to byte code.

// Processes files: reads file, translates it and writes bytecode file.

#include <stdio.h>
#include <stdlib.h>

#include "nopl.h"

// Translate a file
//
// Lines are one of the following formats:
//
// <command> <args>
// <variable> = <expression>
//
// <args> :: <expression> | <expression> , <expression>
//
// Tokens are case insensitive (strings aren't).
// Delimiting is with spaces or commas
//

void translate_file(FILE *fp, FILE *ofp)
{
  char line[MAX_NOPL_LINE+1];
  
  // Read lines from file and translate each line as a unit
  while(!feof(fp) )
    {
      int n = fread(line, 1, MAX_NOPL_LINE, fp);

      printf("\nL:'%s'", line);
      
      // If nothing read then we are done
      if( n == 0 )
	{
	  break;
	}
    }
}


int main(int argc, char *argv[])
{
  FILE *fp;
  FILE *ofp;
  
  // Open file and process on a line by line basis
  fp = fopen(argv[1], "r");

  if( fp == NULL )
    {
      printf("\nCould not open '%s'", argv[1]);
      exit(-1);
    }

  ofp = fopen("out.opl.tran", "w");
  
  translate_file(fp, ofp);

  fclose(fp);
  fclose(ofp);
}

