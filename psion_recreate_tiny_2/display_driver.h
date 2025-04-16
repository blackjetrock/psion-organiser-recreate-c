
typedef enum
  {
    DD_UNKNOWN = 0, 
    DD_I2C_SSD,
    DD_SPI_SSD1309,
    DD_SPI_SSD1351,
  } DD_TYPE;

void dd_init(DD_TYPE type);
void dd_clear(void);
void dd_update(void);
void dd_char_at_xy(int x, int y, int ch);
void dd_plot_point(int x, int y, int mode);
void dd_clear_graphics(void);

void i2c_ssd_clear_oled(void);
extern DD_TYPE current_dd;
