/***************************************************
//Web: http://www.buydisplay.com
EastRising Technology Co.,LTD
****************************************************/

// Modified to run on Psion Organiser recreate

#include <stdio.h>
#include "hardware/gpio.h"
#include "psion_recreate_all.h"

#include "ssd.h"
#include "ssd1351.h"

char buffer1351[OLED_WIDTH * OLED_HEIGHT * 2];
#if 0
void command(char cmd)
{
  bcm2835_gpio_write(DC, LOW);
  bcm2835_spi_transfer(cmd);
}

void data(char cmd) {
    bcm2835_gpio_write(DC, HIGH);
    bcm2835_spi_transfer(cmd);
}
#endif

void SSD1351_begin()
{
  //bcm2835_gpio_fsel(RST, BCM2835_GPIO_FSEL_OUTP);
  //bcm2835_gpio_fsel(DC, BCM2835_GPIO_FSEL_OUTP);
#if 0
  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);     //The default
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                  //The default
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);  //The default
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                     //The default
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);     //the default
  
  bcm2835_gpio_write(RST, HIGH);
  bcm2835_delay(10);
  bcm2835_gpio_write(RST, LOW);
  bcm2835_delay(10);
  bcm2835_gpio_write(RST, HIGH);
#endif
  
  // SPI Initialisation

#if SPI_INTERFACE
    gpio_put(PIN_OLED_RES, 1);
    gpio_put(PIN_OLED_CS, 1);
    gpio_put(PIN_OLED_DC, 1);
#endif
    

  command(0xfd);	// command lock
  data(0x12);
  command(0xfd);	// command lock
  data(0xB1);
  
  command(0xae);	// display off
  command(0xa4); 	// Normal Display mode
  
  command(0x15);	//set column address
  data(0x00);     //column address start 00
  data(0x7f);     //column address end 95
  command(0x75);	//set row address
  data(0x00);     //row address start 00
  data(0x7f);     //row address end 63	
  
  command(0xB3);
  data(0xF1);
  
  command(0xCA);	
  data(0x7F);
  
  command(0xa0);  //set re-map & data format
  data(0x65);     //0x74
  
  command(0xa1);  //set display start line
  data(0x00);     //start 00 line
  
  command(0xa2);  //set display offset
  data(0x00);
  
  command(0xAB);	
  command(0x01);	
  
  command(0xB4);	
  data(0xA0);	  
  data(0xB5);  
  data(0x55);    
  
  command(0xC1);	
  data(0xC8);	
  data(0x80);
  data(0xC0);
  
  command(0xC7);	
  data(0x0F);
  
  command(0xB1);	
  data(0x32);
  
  command(0xB2);	
  data(0xA4);
  data(0x00);
  data(0x00);
  
  command(0xBB);	
  data(0x17);
  
  command(0xB6);
  data(0x01);
  
  command(0xb5); // Set GPIO 0 to 1  turn on DC/DC
  data(0x0b);

  command(0xBE);
  data(0x05);
  
  command(0xA6);
  
  SSD1351_clear();
  command(0xaf);	 //display on    

}

void SSD1351_clear() {
    int i;
    for(i = 0; i < sizeof(buffer1351); i++)
    {
        buffer1351[i] = 0;
    }
}

void SSD1351_draw_point(int x, int y, uint16_t hwColor) {
    if(x >= OLED_WIDTH || y >= OLED_HEIGHT)
    {
        return;
    }
    buffer1351[x * 2 + y * OLED_WIDTH * 2] = hwColor >> 8;
    buffer1351[x * 2 + y * OLED_WIDTH * 2 + 1] = hwColor;
}

#if 0
void SSD1351_char1616(uint8_t x, uint8_t y, uint8_t chChar, uint16_t hwColor) {
    uint8_t i, j;
    uint8_t chTemp = 0, y0 = y;

    for (i = 0; i < 32; i ++) {
        chTemp = Font1612[chChar - 0x30][i];
        for (j = 0; j < 8; j ++) {
            if (chTemp & 0x80) {
                SSD1351_draw_point(x, y, hwColor);
            } else {
                SSD1351_draw_point(x, y, 0);
            }
            chTemp <<= 1;
            y++;
            if ((y - y0) == 16) {
                y = y0;
                x++;
                break;
            }
        }
    }
}

void SSD1351_char3216(uint8_t x, uint8_t y, uint8_t chChar, uint16_t hwColor) {
    uint8_t i, j;
    uint8_t chTemp = 0, y0 = y; 

    for (i = 0; i < 64; i++) {
        chTemp = Font3216[chChar - 0x30][i];
        for (j = 0; j < 8; j++) {
            if (chTemp & 0x80) {
                SSD1351_draw_point(x, y, hwColor);
            } else {
                SSD1351_draw_point(x, y, 0);
            }

            chTemp <<= 1;
            y++;
            if ((y - y0) == 32) {
                y = y0;
                x++;
                break;
            }
        }
    }
}
#endif

void SSD1351_char(uint8_t x, uint8_t y, char acsii, char size, char mode, uint16_t hwColor) {
#if 0
  uint8_t i, j, y0=y;
    char temp;
    uint8_t ch = acsii - ' ';
    for(i = 0;i<size;i++) {
        if(size == 12)
        {
            if(mode)temp=Font1206[ch][i];
            else temp = ~Font1206[ch][i];
        }
        else 
        {            
            if(mode)temp=Font1608[ch][i];
            else temp = ~Font1608[ch][i];
        }
        for(j =0;j<8;j++)
        {
            if(temp & 0x80) SSD1351_draw_point(x, y, hwColor);
            else SSD1351_draw_point(x, y, 0);
            temp <<=1;
            y++;
            if((y-y0)==size)
            {
                y = y0;
                x ++;
                break;
            }
        }
    }
#endif
}

void SSD1351_string(uint8_t x, uint8_t y, const char *pString, uint8_t Size, uint8_t Mode, uint16_t hwColor) {
    while (*pString != '\0') {       
        if (x > (OLED_WIDTH - Size / 2)) {
            x = 0;
            y += Size;
            if (y > (OLED_HEIGHT - Size)) {
                y = x = 0;
            }
        }
        
        SSD1351_char(x, y, *pString, Size, Mode, hwColor);
        x += Size / 2;
        pString ++;
    }
}

void SSD1351_mono_bitmap(uint8_t x, uint8_t y, const uint8_t *pBmp, char chWidth, char chHeight, uint16_t hwColor) {
    uint8_t i, j, byteWidth = (chWidth + 7) / 8;
    for(j = 0; j < chHeight; j++) {
        for(i = 0; i <chWidth; i ++) {
            if(*(pBmp + j * byteWidth + i / 8) & (128 >> (i & 7))) {
                SSD1351_draw_point(x + i, y + j, hwColor);
            }
        }
    }        
}

void SSD1351_bitmap24(uint8_t x, uint8_t y, uint8_t *pBmp, char chWidth, char chHeight) {
    uint8_t i, j;
    uint16_t hwColor;
    uint32_t temp;

    for(j = 0; j < chHeight; j++) {
        for(i = 0; i < chWidth; i ++) {
            temp = *(unsigned int*)(pBmp + i * 3 + j * 3 * chWidth);
            hwColor = RGB(((temp >> 16) & 0xFF),
                          ((temp >> 8) & 0xFF),
                           (temp & 0xFF));
            SSD1351_draw_point(x + i, y + chHeight - 1 - j, hwColor);
        }
    }
}

void SSD1351_display() {
    command(SSD1351_CMD_SETCOLUMN);
    command(0);         //cloumn start address
    command(OLED_WIDTH - 1); //cloumn end address
    command(SSD1351_CMD_SETROW );
    command(0);         //page atart address
    command(OLED_HEIGHT - 1); //page end address
    command(SSD1351_CMD_WRITERAM);
#if 0
    bcm2835_gpio_write(DC, HIGH);
    bcm2835_spi_transfern(buffer1351, sizeof(buffer1351));
#else
    for(int i=0; i<sizeof(buffer1351); i++)
    {
      data(buffer1351[i]);
    }
#endif
}

void SSD1351_clear_screen(uint16_t hwColor) {
    uint16_t i, j;
    for(i = 0; i < OLED_HEIGHT; i++) {
        for(j = 0; j < OLED_WIDTH; j ++) {
            SSD1351_draw_point(j, i, hwColor);
        }
    }
}

