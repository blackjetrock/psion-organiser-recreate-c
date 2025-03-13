#ifndef _SSD1309_H_
#define _SSD1309_H_
int main_er_oledm015_2(void);

#define SSD1309_SETCONTRAST 0x81
#define SSD1309_DISPLAYALLON_RESUME 0xA4
#define SSD1309_DISPLAYALLON 0xA5
#define SSD1309_NORMALDISPLAY 0xA6
#define SSD1309_INVERTDISPLAY 0xA7
#define SSD1309_DISPLAYOFF 0xAE
#define SSD1309_DISPLAYON 0xAF
#define SSD1309_SETDISPLAYOFFSET 0xD3
#define SSD1309_SETCOMPINS 0xDA
#define SSD1309_SETVCOMDETECT 0xDB
#define SSD1309_SETDISPLAYCLOCKDIV 0xD5
#define SSD1309_SETPRECHARGE 0xD9
#define SSD1309_SETMULTIPLEX 0xA8
#define SSD1309_SETLOWCOLUMN 0x00
#define SSD1309_SETHIGHCOLUMN 0x10
#define SSD1309_SETSTARTLINE 0x40
#define SSD1309_MEMORYMODE 0x20
#define SSD1309_COLUMNADDR 0x21
#define SSD1309_PAGEADDR 0x22
#define SSD1309_COMSCANINC 0xC0
#define SSD1309_COMSCANDEC 0xC8
#define SSD1309_SEGREMAP 0xA0
#define SSD1309_CHARGEPUMP 0x8D
#define SSD1309_EXTERNALVCC 0x01
#define SSD1309_SWITCHCAPVCC 0x02

//Scrolling constants
#define SSD1309_ACTIVATE_SCROLL 0x2F
#define SSD1309_DEACTIVATE_SCROLL 0x2E
#define SSD1309_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1309_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1309_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1309_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1309_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

void SSD1309_begin();
void SSD1309_display();
void SSD1309_clear();
void SSD1309_pixel(int x,int y,char color);
void SSD1309_bitmap(unsigned char x, unsigned char y, const unsigned char *pBmp, unsigned char chWidth, unsigned char chHeight);
void SSD1309_string(uint8_t x, uint8_t y, const char *pString, uint8_t Size, uint8_t Mode);
void SSD1309_char1616(uint8_t x, uint8_t y, uint8_t chChar);
void SSD1309_char3216(uint8_t x, uint8_t y, uint8_t chChar);
//void I2C_Write_Byte(uint8_t value, uint8_t Cmd);
void SSD1309_char(unsigned char x, unsigned char y, char acsii, char size, char mode);


#endif
