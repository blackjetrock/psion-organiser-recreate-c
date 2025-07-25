void SSD1351_begin(void);
void SSD1351_clear(void);
void SSD1351_draw_point(int x, int y, uint16_t hwColor);


void SSD1351_char(uint8_t x, uint8_t y, char acsii, char size, char mode, uint16_t hwColor);
void SSD1351_display(void);
void SSD1351_clear_screen(uint16_t hwColor);

    
