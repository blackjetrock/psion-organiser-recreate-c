#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psion_recreate_all.h"

#include "serial.h"

////////////////////////////////////////////////////////////////////////////////
//
// Display driver
//
// Routes calls to appropriate hardware drivers
//
////////////////////////////////////////////////////////////////////////////////


DD_TYPE current_dd = DD_UNKNOWN;

void dd_init(DD_TYPE type)
{
  printf("\nDD Init");
  current_dd = type;
  switch(type)
    {
    case DD_SPI_SSD1309:
      SSD1309_begin();
      break;
    }
}

void dd_clear(void)
{
  switch(current_dd)
    {
    case DD_SPI_SSD1309:
      SSD1309_clear();
      SSD1309_display();
      break;
    }
}

void dd_char_at_xy(int x, int y, int ch)
{
  printf("\nCur dd:%d", current_dd);
  switch(current_dd)
    {
    case DD_SPI_SSD1309:
      SSD1309_char(x*6, y*12, ch, 12, 1);
      break;
    }
}

void dd_clear_graphics(void)
{
  switch(current_dd)
    {
    case DD_SPI_SSD1309:
      SSD1309_clear();
      SSD1309_display();
      break;
    }
}

void dd_plot_point(int x, int y, int mode)
{
  //printf("\ndd_PLOT_POINT: x:%d, y:%d mopde:%d", x, y, mode);

  switch(current_dd)
    {
    case DD_SPI_SSD1309:
      SSD1309_pixel(x, y, mode);
      break;
    }
}

void dd_update(void)
{
  switch(current_dd)
    {
    case DD_SPI_SSD1309:
      SSD1309_display();
      break;
    }
}

