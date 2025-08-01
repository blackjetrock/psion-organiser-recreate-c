// Different i2c busses can be handled, the bus is set up as a bus ID

typedef struct _I2C_BUS
{
  int pin_scl;
  int pin_sda;
} I2C_BUS;

typedef enum _I2C_BUS_ID
  {
    I2C_BUS_OLED = 0,
    I2C_BUS_RTC  = 1,
  } I2C_BUS_ID;

//------------------------------------------------------------------------------

#if PSION_RECREATE
#define PIN_I2C_SDA         16
#define PIN_I2C_SCL         17
#endif

#if PSION_MINI
#define PIN_I2C_SDA         5
#define PIN_I2C_SCL         6
#endif

#define PIN_I2C_RTC_SDA      0
#define PIN_I2C_RTC_SCL      1

//------------------------------------------------------------------------------

void i2c_set_delay_value(int delay);
void i2c_release(I2C_BUS_ID id);
void i2c_delay(I2C_BUS_ID id);
void i2c_sda_low(I2C_BUS_ID id);
void i2c_sda_high(I2C_BUS_ID id);
void i2c_scl_low(I2C_BUS_ID id);
void i2c_scl_high(I2C_BUS_ID id);
void i2c_start(I2C_BUS_ID id);
void i2c_stop(I2C_BUS_ID id);
int i2c_send_byte(I2C_BUS_ID id, BYTE b);
int i2c_read_bytes(I2C_BUS_ID id, BYTE slave_addr, int n, BYTE *data);
void i2c_send_bytes(I2C_BUS_ID id, BYTE slave_addr, int n, BYTE *data);

