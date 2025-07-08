#undef TUI


#define MAX_NOPL_LINE         80
#define PRT_MAX_LINE          80
#define NOPL_MAX_LABEL         12
#define NOPL_MAX_SUFFIX_BYTES   8
#define MAX_COND_FIXUP        100
#define MAX_NOPL_MENU_SELS     16

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
//#include <curses.h>
#include <unistd.h>

#include "newopl.h"

#include "nopl_obj.h"
#include "newopl_types.h"

#include "ff_stdio.h"
void ff_fprintf(FIL *fp, char *fmt, ...);
  
#if 1
#undef assert
#define assert configASSERT
#define fopen  ff_fopen
#define fwrite ff_fwrite
#define fread  ff_fread
#define fclose ff_fclose
#define fseek  ff_fseek
#ifndef FF_DEFINED
#define errno  stdioGET_ERRNO()
#define free   vPortFree
#define malloc pvPortMalloc
#define fflush ff_fflush
#endif
#endif

#include "newopl_exec.h"
#include "newopl_trans.h"

#include "newopl_lib.h"

#include "qcode.h"
#include "errors.h"
#include "qcode_clock.h"
#include "parser.h"

#include "machine.h"

#include "calc.h"
#include "files.h"

#include "svc_pk_base.h"

#include "svc_fl.h"
#include "svc_kb.h"

#include "logical_files.h"
#include "svc_pak_linux.h"

#include "svc_pk.h"
#include "svc_mn.h"

#include "sysvar.h"
#include "nopl_time.h"

#include "serial.h"

#ifdef TUI
#include "tui.h"
#endif

void fprintstr(FF_FILE *fp, char *str);

