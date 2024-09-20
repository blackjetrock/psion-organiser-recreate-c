////////////////////////////////////////////////////////////////////////////////
//
// Serial CDC Panels
//
////////////////////////////////////////////////////////////////////////////////


#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"

#include "psion_recreate_all.h"


////////////////////////////////////////////////////////////////////////////////
//
// Process a panel.
//

void do_panel(PANEL_T *panel, int itf, int cls)
{
  if( cls )
    {
      cdc_cls(itf);
    }

  // Initialse the panel
  if( (panel->init_fn) != NULL )
    {
      (*panel->init_fn)();
    }

  // Now display the items
  for(int ii = 0; ; ii++)
    {
      if( ((panel->items[ii].value) == NULL) &&
	  ((panel->items[ii].fn) == NULL) )
	{
	  break;
	}

      int title_len = strlen(panel->items[ii].title);
      int data_len  = panel->items[ii].length;
      int x = panel->items[ii].x;
      int y = panel->items[ii].y;
      
      cdc_printf_xy(itf, x, y, "%*s", data_len + title_len + 2, "");
      cdc_printf_xy(itf, x, y, "%s",  panel->items[ii].title);

      switch(panel->items[ii].type)
	{
	case PANEL_ITEM_TYPE_INT:
	  cdc_printf_xy(itf, x + title_len + 2, y, "%d", *(panel->items[ii].value));
	  break;

	case PANEL_ITEM_TYPE_FN:
	  (*panel->items[ii].fn)();
	  break;
	}
    }
}
