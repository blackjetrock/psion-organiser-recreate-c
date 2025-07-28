////////////////////////////////////////////////////////////////////////////////
//
// File handling
//
// Line based handling
//
////////////////////////////////////////////////////////////////////////////////


#define FH_IGNORE_ERROR         1
#define FH_DO_NOT_IGNORE_ERROR  0
 
//------------------------------------------------------------------------------

#define FH_LINE_MAX 256

typedef struct _FILE_INFO
{
  int num_lines;
  
} FILE_INFO;

//------------------------------------------------------------------------------

typedef struct _FILE_LINE
{
  struct _FILE_LINE *next;
  struct _FILE_LINE *prev;
  char *text;                   // The text of the line itself
  
} FILE_LINE;

//------------------------------------------------------------------------------

void file_editor(char *filename);
int file_load(char *filename, FILE_INFO *fi);
void file_unload(void);
void file_dump_list(void);
