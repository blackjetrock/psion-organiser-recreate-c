#include "match.h"

typedef uint8_t BYTE;

#define uint unsigned int
#define uchar unsigned char

#define DEBUG_STOP {volatile int x = 1; while (x) {} }

typedef enum _KEYCODE
  {
   KEY_ON        = 1,
   KEY_MODE      = 2,
   KEY_UP        = 3,
   KEY_DOWN      = 4,
   KEY_LEFT      = 5,
   KEY_RIGHT     = 6,
   KEY_SHIFT_DEL = 7,
   KEY_DEL       = 8,
   KEY_TAB       = 9,
   KEY_EXE       = 13,
   NOS_KEY_NONE  = -1,
   KEY_NONE      = -1,
  } KEYCODE;

// Character codes

typedef enum _CHRCODE
  {
   CHRCODE_LEFT     = 8,
   CHRCODE_TAB      = 9,
   CHRCODE_LF       = 10,
   CHRCODE_HOME     = 11,
   CHRCODE_CLS      = 12,
   CHRCODE_CR       = 13,
   CHRCODE_CLL1     = 14,
   CHRCODE_CLL2     = 15,
   CHRCODE_BEEP     = 16,
   CHRCODE_RFSH_12  = 17,
   CHRCODE_RFSH_1   = 18,
   CHRCODE_RFSH_2   = 19,
   CHRCODE_RFSH_3   = 20,
   CHRCODE_RFSH_4   = 21,
   CHRCODE_CLL3     = 22,
   CHRCODE_CLL4     = 23,
   CHRCODE_DASHES   = 24,
   CHRCODE_CLREOL   = 25,
   CHRCODE_  = 26,

  } CHRCODE;


////////////////////////////////////////////////////////////////////////////////
// The model we are emulating chnages the display layout

#define MODEL_XP             0
#define MODEL_LZ             1

// Initial state of warm start flag 0x80 to have warm start
#if DISABLE_RESTORE_ONLY     
#define WARM_FLAG_INITIAL    0x00
#else
#define WARM_FLAG_INITIAL    0x80
#endif

// The model we are emulating
extern int model;

// The value t set model to
#define MODEL_AT_START       MODEL_LZ
//#define MODEL_AT_START       MODEL_XP

#define FN_OLED_DEMO         0
#define FN_KEYBOARD_TEST     0
#define FN_FLASH_LED         0
#define SLOT_TEST            0
#define SLOT_TEST_MASK       LAT2PIN_MASK_SS2
#define SLOT_TEST_G          0
#define SLOT_TEST_GPIO       PIN_SD0
#define TEST_PORT2           0
#define PACK_TEST            0
#define WIFI_TEST            0
#define RTC_TEST             0
#define EEPROM_TEST          0
#define EEPROM_TEST2         0
#define EEPROM_TEST3         0    // Test RAM dump and restore

#define NEW_I2C              1    // Better I2C, not demo code
#define BUZZER_TEST          0
#define UART_INTERRUPTS      1     // Interrupt for UART data collection
#define I2C_DELAY            15    // Default, can be over-ridden
#define I2C_DELAY_OLED       1     // OLED I2C delay value
#define I2C_DELAY_EEPROM     15    // EEPROM I2C delay

#define ALLOW_POWER_OFF      1     // Do we allow the power to be turned off?
                                   // If 0 then sit in a loop on power off so
                                   // we can debug where the request to turn
                                   // off came from
#define WIFI                 1
#define BLUETOOTH            0     // Enable bluetooth comms
#define BLUETOOTH_M          0     // Operate BT in master mode
#define BLUETOOTH_S          1     // Operate BT in slave mode

#define ENABLE_1S_TICK       1     // Needed by restore from EEPROM code,
                                   // otherwise hangs
#define DISABLE_AUTO_OFF     0     // Disable auto off feature
#define TRACE_ADDR           1     // Trace execution addresses
#define TRACE_TO_TRAP        0     // Trace until a TRAP
#define NUM_ADDR_TRACE       300   // How many addresses to trace

#define RAM_RESTORE          1    // Enable dump/restore RAM to/from EEPROM
#define DISABLE_RESTORE_ONLY 0    // Disable the restore part, still dump
#define EEPROM_DUMP_CHECK    0    // Do we check the dumped contents?
#define DISABLE_DMP_WR       0    // Run dump code but don't write anything
                                  // We can use one image over and over
#define MENU_ENABLED         1    // Meta menu enabled

#define OVERCLOCK            1    // Overclock the RP2040
#define OVERCLOCK_RESTORE    0    // Overclock for eeprom restore

#define META_MENU_SCAN_COUNT    10

typedef u_int8_t BYTE;

// Do we use two cores?
// If yes then the second core handles:
//    Display update

#define MULTI_CORE        1

#define PIN_SD0 0
#define PIN_SD1 1
#define PIN_SD2 2
#define PIN_SD3 3
#define PIN_SD4 4
#define PIN_SD5 5
#define PIN_SD6 6
#define PIN_SD7 7

#define PIN_SCLK       10
#define PIN_SOE        11
#define PIN_SMR        12
#define PIN_P57        13

extern const uint PIN_SDAOUT;
extern const uint PIN_LATCHOUT2;
extern const uint PIN_I2C_SDA;
extern const uint PIN_I2C_SCL;
extern const uint PIN_LS_DIR;
extern const uint PIN_LATCHIN;
extern const uint PIN_SCLKIN;
extern const uint PIN_SDAIN;
extern const uint PIN_LATCHOUT1;
extern const uint PIN_SCLKOUT;
extern const uint PIN_VBAT_SW_ON;

extern volatile uint16_t latchout1_shadow;
extern volatile uint16_t latch2_shadow;

void latch2_set_mask(int value);
void latch2_clear_mask(int value);

uint8_t read_165(const uint latchpin);
void write_595(const uint latchpin, int value, int n);

// Latch pins

// Pseudo pins, code needed to handle them
#define PSEUDO_PIN             100

#define PIN_5V_ON              100
#define PIN_SS1                101
#define PIN_SS2                102
#define PIN_SS3                103
#define PIN_SPGM               104

#define LAT1PIN_MASK_K1        0x01
#define LAT1PIN_MASK_K2        0x02
#define LAT1PIN_MASK_K3        0x04
#define LAT1PIN_MASK_K4        0x08
#define LAT1PIN_MASK_K5        0x10
#define LAT1PIN_MASK_K6        0x20
#define LAT1PIN_MASK_K7        0x40
#define LAT1PIN_MASK_OLED_RES  0x80

#define LAT2PIN_MASK_DRV_HV             0x0001
#define LAT2PIN_MASK_ESP_ON             0x0002
#define LAT2PIN_MASK_BUZZER             0x0004
#define LAT2PIN_MASK_5V_ON              0x0008
#define LAT2PIN_MASK_P_SPGM             0x0010
#define LAT2PIN_MASK_VBSW_ON            0x0020
#define LAT2PIN_MASK_SD_OE              0x0040
#define LAT2PIN_MASK_SC_OE              0x0080

#define LAT2PIN_MASK_VPP_ON             0x0100
#define LAT2PIN_MASK_VPP_VOLT_SELECT    0x0200
#define LAT2PIN_MASK_SS1                0x0800
#define LAT2PIN_MASK_SS2                0x1000
#define LAT2PIN_MASK_SS3                0x2000

#define MAX_DDRAM 0xFF
#define MAX_CGRAM (5*16)
#define DISPLAY_NUM_LINES   4
#define DISPLAY_NUM_CHARS  21
#define DISPLAY_NUM_EXTRA   4

extern unsigned char font_5x7_letters[];
extern char lcd_display_buffer[MAX_DDRAM+2];
extern char lcd_display[MAX_DDRAM+2];;
extern char display_line[DISPLAY_NUM_LINES][DISPLAY_NUM_CHARS+1];

////////////////////////////////////////////////////////////////////////////////
//
// Cursor
//

extern int cursor_on;
extern uint64_t cursor_upd_time;
extern uint64_t cursor_last_time;
extern volatile int cursor_phase;
extern int force_cursor_update;
extern int cursor_char;
extern int under_cursor_char[DISPLAY_NUM_CHARS][DISPLAY_NUM_LINES];
extern int cursor_x;
extern int cursor_y;
extern int cursor_blink;
extern int saved_char;

void handle_cursor_key(KEYCODE k);

////////////////////////////////////////////////////////////////////////////////


void _nop_(void);

void put_display_char(int x,int y, int ch);

void write_port2(u_int8_t value);
u_int8_t read_port2(void);


////////////////////////////////////////////////////////////////////////////////

void set_st_bit();
void set_vbaten_bit();
void write_mcp7940(int r, BYTE data);
int read_mcp7940(int r);
void Delay1(uint n);
void Write_number(uchar *n,uchar k,uchar station_dot);
void display_Contrast_level(uchar number);
void adj_Contrast(void);
void Delay(uint n);
void Set_Page_Address(unsigned char add);
void Set_Column_Address(unsigned char add);
void Set_Contrast_Control_Register(unsigned char mod);
void initialise_oled(void);
void Display_Chess(unsigned char value);
void Display_Chinese(unsigned char ft[]);
void Display_Chinese_Column(unsigned char ft[]);
void Display_Picture(unsigned char pic[]);
void SentByte(unsigned char Byte);
void Check_Ack(void);//Acknowledge
void Stop(void);
void Start(void);
void Send_ACK(void);
unsigned char ReceiveByte(void);
void clear_oled(void);
void print_cursor(int x, int y, int ch);
void oledmain(void);


void dump_lcd(void);

// Core 1
void core1_main(void);
void menu_loop_tasks(void);
    
// Emulator
void initialise_emulator(void);
void after_ram_restore_init(void);
void loop_emulator(void);

// RTC tasks
void rtc_tasks(void);
extern int rtc_set_st;

//#define RAM_SIZE 32*1024
#define RAM_SIZE 64*1024
//#define RAM_SIZE 96*1024
//#define RAM_SIZE 128*1024

#define ROM_SIZE (sizeof(romdata))
#define ROM_START (0x8000)

extern u_int8_t ramdata[RAM_SIZE];

// I2C functions
//void i2c_fn_initialise(void);
//void i2c_fn_set_delay(int delay);

void i2c_release(void);
void i2c_delay(void);
void i2c_sda_low(void);
void i2c_sda_high(void);
void i2c_scl_low(void);
void i2c_scl_high(void);
void i2c_start(void);
void i2c_stop(void);
int i2c_send_byte(BYTE b);
int i2c_read_bytes(BYTE slave_addr, int n, BYTE *data);
void i2c_send_bytes(BYTE slave_addr, int n, BYTE *data);

// EEPROM

#define EEPROM_0_ADDR_WR   (0xA0)
#define EEPROM_0_ADDR_RD   (0xA1)
#define EEPROM_1_ADDR_WR   (0xA2)
#define EEPROM_1_ADDR_RD   (0xA3)

int read_eeprom(int slave_addr, int start, int len, u_int8_t *dest);
int write_eeprom(int slave_addr, int start, int len, BYTE *src);
void eeprom_test(void);
void eeprom_ram_restore(void);
void eeprom_ram_dump(void);
void eeprom_ram_check(void);
void eeprom_perform_dump(void);


extern volatile int core1_in_menu;

////////////////////////////////////////////////////////////////////////////////
//
// Tracing
//

extern int tracing_to;

#if TRACE_ADDR
// Trace a number of execution addresses
extern int tracing;
extern u_int16_t trigger_addr;
extern int addr_trace_i;

// Trace from a trigger address until trace full
extern volatile u_int16_t addr_trace_from[NUM_ADDR_TRACE];
extern volatile u_int8_t  addr_trace_from_flags[NUM_ADDR_TRACE];
extern volatile u_int8_t  addr_trace_from_a[NUM_ADDR_TRACE];
extern volatile u_int8_t  addr_trace_from_b[NUM_ADDR_TRACE];
extern volatile u_int16_t addr_trace_from_x[NUM_ADDR_TRACE];
extern volatile u_int16_t addr_trace_from_sp[NUM_ADDR_TRACE];

// Trace continuously until trigger address seen
extern u_int16_t trace_stop_addr;
extern volatile u_int16_t addr_trace_to[NUM_ADDR_TRACE];
extern volatile u_int8_t  addr_trace_to_flags[NUM_ADDR_TRACE];
extern volatile u_int8_t  addr_trace_to_a[NUM_ADDR_TRACE];
extern volatile u_int8_t  addr_trace_to_b[NUM_ADDR_TRACE];
extern volatile u_int16_t addr_trace_to_x[NUM_ADDR_TRACE];
extern volatile u_int16_t addr_trace_to_sp[NUM_ADDR_TRACE];
extern int addr_trace_to_i;

#endif

////////////////////////////////////////////////////////////////////////////////
//
// Checksums
//

extern uint16_t csum_calc_on_restore;
extern uint16_t csum_in_eeprom;
extern uint16_t csum_calc_on_dump;

#include "font.h"
