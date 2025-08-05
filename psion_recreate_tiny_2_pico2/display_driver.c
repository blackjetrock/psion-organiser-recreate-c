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
     case DD_I2C_SSD:
       printf("\nI2C SSD driver");
       sleep_ms(10);
       initialise_oled();
      break;
      
    case DD_SPI_SSD1309:
      printf("\nSSD1309 driver");
      SSD1309_begin();
      break;

    case DD_SPI_SSD1351:
      printf("\nSSD1351 driver");
      SSD1351_begin();
      break;

    case DD_PICOCALC:
      printf("\nPicocalc driver");
      picocalc_lcd_init();
      picocalc_lcd_clear();
      break;
    }
}

void dd_clear(void)
{
  switch(current_dd)
    {
     case DD_I2C_SSD:
      i2c_ssd_clear_oled();
      break;

    case DD_SPI_SSD1309:
      SSD1309_clear();
      SSD1309_display();
      break;

    case DD_SPI_SSD1351:
      SSD1351_clear();
      SSD1351_display();
      break;

    case DD_PICOCALC:
      picocalc_lcd_clear();
      break;
    }
}

void dd_char_at_xy(int x, int y, int ch)
{
  switch(current_dd)
    {
    case DD_I2C_SSD:
      i2c_ssd(127-(x * 6), 3-y, ch);
      break;

    case DD_SPI_SSD1309:
      SSD1309_char(x*6, y*12, ch, 12, 1);
      break;

    case DD_SPI_SSD1351:
      SSD1351_char(x*6, y*12, ch, 12, 1, 0x00FF00);
      break;

    case DD_PICOCALC:
      picocalc_current_x = x*9;
      picocalc_current_y = y*13;
      //      picocalc_lcd_print_char(PICOCALC_GREEN, PICOCALC_BLACK, ch, ORIENT_NORMAL);
      picocalc_lcd_putc(ch);
      break;

    }
}

// Copy a character dot pattern into the UDG
void dd_set_udg_as_char(int ch)
{
  switch(current_dd)
    {
    case DD_I2C_SSD:
      //i2c_ssd(ch);
      break;

    case DD_SPI_SSD1309:
      SSD1309_set_udg_as(ch, 12, 1);
      break;

    case DD_SPI_SSD1351:
      //SSD1351_char(x*6, y*12, ch, 12, 1, 0x00FF00);
      break;

    case DD_PICOCALC:
      break;
    }
}

void dd_clear_graphics(void)
{
  switch(current_dd)
    {
    case DD_I2C_SSD:
      i2c_ssd_clear_oled();
      break;

    case DD_SPI_SSD1309:
      SSD1309_clear();
      SSD1309_display();
      break;

    case DD_SPI_SSD1351:
      SSD1351_clear();
      SSD1351_display();
      break;

    case DD_PICOCALC:
      picocalc_lcd_clear();
      break;

    }
}

// Mode can be colour

void dd_plot_point(int x, int y, int mode)
{
  //printf("\ndd_PLOT_POINT: x:%d, y:%d mopde:%d", x, y, mode);

  switch(current_dd)
    {
    case DD_I2C_SSD:
      i2c_ssd_plot_point(x, y, mode);
      break;
      
    case DD_SPI_SSD1309:
      SSD1309_pixel(x, y, mode);
      break;

    case DD_SPI_SSD1351:
      SSD1351_draw_point(x, y, mode);
      break;

    case DD_PICOCALC:
      picocalc_set_pixel(x, y, mode);
      break;

    }

}

void dd_update(void)
{
  switch(current_dd)
    {
    case DD_I2C_SSD:
      //SSD1309_display();
      break;

    case DD_SPI_SSD1309:
      SSD1309_display();
      break;

    case DD_SPI_SSD1351:
      SSD1351_display();
      break;
    }
}

int dd_get_x_size(void)
{
  switch(current_dd)
    {
    case DD_I2C_SSD:
      return(128);
      break;

    case DD_SPI_SSD1309:
      return(128);
      break;

    case DD_SPI_SSD1351:
      return(128);
      break;

    case DD_PICOCALC:
      return(320);
      break;

    }
  
  return(10);
}

int dd_get_y_size(void)
{
  switch(current_dd)
    {
    case DD_I2C_SSD:
      return(32);
      break;

    case DD_SPI_SSD1309:
      return(64);
      break;

    case DD_SPI_SSD1351:
      return(32);
      break;

    case DD_PICOCALC:
      return(320);
      break;

    }

  
  return(10);
}

