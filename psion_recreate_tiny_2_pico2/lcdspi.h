#ifndef LCDSPI_H
#define LCDSPI_H
#include "pico/multicore.h"
#include <hardware/spi.h>

//#define LCD_SPI_SPEED   6000000
#define LCD_SPI_SPEED   25000000
//#define LCD_SPI_SPEED 50000000

#define Pico_LCD_SCK 10 //
#define Pico_LCD_TX  11 // MOSI
#define Pico_LCD_RX  12 // MISO
#define Pico_LCD_CS  13 //
#define Pico_LCD_DC  14
#define Pico_LCD_RST 15

#define ILI9488  1
#ifdef ILI9488
#define LCD_WIDTH 320
#define LCD_HEIGHT 320
#endif

#define PIXFMT_BGR 1

#define TFT_SLPOUT 0x11
#define TFT_INVOFF 0x20
#define TFT_INVON 0x21

#define TFT_DISPOFF 0x28
#define TFT_DISPON 0x29
#define TFT_MADCTL 0x36

#define ILI9341_MEMCONTROL 	0x36
#define ILI9341_MADCTL_MX  	0x40
#define ILI9341_MADCTL_BGR 	0x08

#define ILI9341_COLADDRSET      0x2A
#define ILI9341_PAGEADDRSET     0x2B
#define ILI9341_MEMORYWRITE     0x2C
#define ILI9341_RAMRD           0x2E

#define ILI9341_Portrait        ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR

#define ORIENT_NORMAL       0

#define PICOCALC_RGB(red, green, blue) ((unsigned int) ( ((red & 0b11111111) << 16) | ((green  & 0b11111111) << 8) | (blue & 0b11111111) ))
#define PICOCALC_WHITE               PICOCALC_RGB(255,  255,  255) //0b1111
#define PICOCALC_YELLOW              PICOCALC_RGB(255,  255,    0) //0b1110
#define PICOCALC_LILAC               PICOCALC_RGB(255,  128,  255) //0b1101
#define PICOCALC_BROWN               PICOCALC_RGB(255,  128,    0) //0b1100
#define PICOCALC_FUCHSIA             PICOCALC_RGB(255,  64,   255) //0b1011
#define PICOCALC_RUST                PICOCALC_RGB(255,  64,     0) //0b1010
#define PICOCALC_MAGENTA             PICOCALC_RGB(255,  0,    255) //0b1001
#define PICOCALC_RED                 PICOCALC_RGB(255,  0,      0) //0b1000
#define PICOCALC_CYAN                PICOCALC_RGB(0,    255,  255) //0b0111
#define PICOCALC_GREEN               PICOCALC_RGB(0,    255,    0) //0b0110
#define PICOCALC_CERULEAN            PICOCALC_RGB(0,    128,  255) //0b0101
#define PICOCALC_MIDGREEN            PICOCALC_RGB(0,    128,    0) //0b0100
#define PICOCALC_COBALT              PICOCALC_RGB(0,    64,   255) //0b0011
#define PICOCALC_MYRTLE              PICOCALC_RGB(0,    64,     0) //0b0010
#define PICOCALC_BLUE                PICOCALC_RGB(0,    0,    255) //0b0001
#define PICOCALC_BLACK               PICOCALC_RGB(0,    0,      0) //0b0000
#define PICOCALC_BROWN               PICOCALC_RGB(255,  128,    0)
#define PICOCALC_GRAY                PICOCALC_RGB(128,  128,    128)
#define PICOCALC_LITEGRAY            PICOCALC_RGB(210,  210,    210)
#define PICOCALC_ORANGE            	PICOCALC_RGB(0xff,	0xA5,	0)
#define PICOCALC_PINK				PICOCALC_RGB(0xFF,	0xA0,	0xAB)
#define PICOCALC_GOLD				PICOCALC_RGB(0xFF,	0xD7,	0x00)
#define PICOCALC_SALMON				PICOCALC_RGB(0xFA,	0x80,	0x72)
#define PICOCALC_BEIGE				PICOCALC_RGB(0xF5,	0xF5,	0xDC)

//Pico spi0 or spi1 must match GPIO pins used above.
#define Pico_LCD_SPI_MOD spi1
#define nop asm("NOP")
//xmit_byte_multi == HW1SendSPI


#define PORTCLR             1
#define PORTSET             2
#define PORTINV             3
#define LAT                 4
#define LATCLR              5
#define LATSET              6
#define LATINV              7
#define ODC                 8
#define ODCCLR              9
#define ODCSET              10
#define CNPU                12
#define CNPUCLR             13
#define CNPUSET             14
#define CNPUINV             15
#define CNPD                16
#define CNPDCLR             17
#define CNPDSET             18

#define ANSELCLR            -7
#define ANSELSET            -6
#define ANSELINV            -5
#define TRIS                -4
#define TRISCLR             -3
#define TRISSET             -2

extern void __not_in_flash_func(spi_write_fast)(spi_inst_t *spi, const uint8_t *src, size_t len);
extern void __not_in_flash_func(spi_finish)(spi_inst_t *spi);
extern void hw_read_spi(unsigned char *buff, int cnt);
extern void hw_send_spi(const unsigned char *buff, int cnt);
extern unsigned char __not_in_flash_func(hw1_swap_spi)(unsigned char data_out);

extern void lcd_spi_raise_cs(void);
extern void lcd_spi_lower_cs(void);
extern void spi_write_data(unsigned char data);
extern void spi_write_command(unsigned char data);
extern void spi_write_cd(unsigned char command, int data, ...);
extern void spi_write_data24(uint32_t data);

extern void spi_draw_pixel(uint16_t x, uint16_t y, uint16_t color) ;
extern void picocalc_lcd_putc(uint8_t c);
extern int  lcd_getc(uint8_t devn);
extern void lcd_sleeping(uint8_t devn);

extern char lcd_put_char(char c, int flush);
extern void picocalc_lcd_print_string(char* s);

extern void lcd_spi_init();
extern void picocalc_lcd_init();
extern void picocalc_lcd_clear();
extern void reset_controller(void);
extern void pin_set_bit(int pin, unsigned int offset);
extern void picocalc_lcd_print_char( int fc, int bc, char c, int orientation);
extern void picocalc_set_pixel(int x, int y, int col);

extern short picocalc_current_x, picocalc_current_y;

#endif
