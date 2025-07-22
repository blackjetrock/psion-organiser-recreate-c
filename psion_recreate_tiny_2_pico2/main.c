#include <stdio.h>
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

#include "psion_recreate_all.h"

#include "ssd1309.h"
//#include "nopl.h"

extern void core1_init(void);
extern bool core1_tick(void);

extern void own_task(void);


////////////////////////////////////////////////////////////////////////////////
//
// GPIOs
//
//
////////////////////////////////////////////////////////////////////////////////

#if PSION_RECREATE

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

const uint PIN_SDAOUT     = 14;
const uint PIN_LATCHOUT2  = 15;
const uint PIN_I2C_SDA    = 16;
const uint PIN_I2C_SCL    = 17;
const uint PIN_LS_DIR     = 18;
const uint PIN_LATCHIN    = 19;
const uint PIN_SCLKIN     = 20;
const uint PIN_SDAIN      = 21;
const uint PIN_LATCHOUT1  = 22;

const uint PIN_SCLKOUT    = 26;
const uint PIN_VBAT_SW_ON = 27;

#endif

#if PSION_MINI

// No datapack interface
#define PIN_SD0 7
#define PIN_SD1 7
#define PIN_SD2 7
#define PIN_SD3 7
#define PIN_SD4 7
#define PIN_SD5 7
#define PIN_SD6 7
#define PIN_SD7 7

#define PIN_SCLK       7
#define PIN_SOE        7
#define PIN_SMR        7
#define PIN_P57        7

const uint PIN_SDAOUT     = 27;
const uint PIN_LATCHOUT2  = 15;
const uint PIN_I2C_SDA    = 5;
const uint PIN_I2C_SCL    = 6;
  
const uint PIN_LS_DIR     = 7;
const uint PIN_LATCHIN    = 12;
const uint PIN_SCLKIN     = 13;
const uint PIN_SDAIN      = 17;
const uint PIN_LATCHOUT1  = 14;

const uint PIN_SCLKOUT    = 26;
const uint PIN_VBAT_SW_ON = 11;

#endif

const uint PIN_OLED_RES   = 4;
const uint PIN_OLED_DC    = 3;
const uint PIN_OLED_CS    = 2;

volatile uint16_t latch2_shadow    = 0;
volatile uint16_t latchout1_shadow = 0;

uint16_t csum_calc_on_restore = 0;
uint16_t csum_in_eeprom       = 0;
uint16_t csum_calc_on_dump    = 0;

////////////////////////////////////////////////////////////////////////////////

void latch2_set_mask(int value)
{
  latch2_shadow |= value;
  write_595(PIN_LATCHOUT2, latch2_shadow, 16);
}

void latch2_clear_mask(int value)
{
  latch2_shadow &= ~value;
  write_595(PIN_LATCHOUT2, latch2_shadow, 16);
}

////////////////////////////////////////////////////////////////////////////////
//
// Read an 8 bit value from a 165 latch
//
////////////////////////////////////////////////////////////////////////////////

#define D165 1

uint8_t __not_in_flash_func(read_165)(const uint latchpin)
{
  uint8_t value = 0;
  
  // Latch the data
  gpio_put(latchpin, 0);
  gpio_put(latchpin, 0);
  core1_safe_delay_ms(D165);
  //  sleep_us(D165);
  gpio_put(latchpin, 1);
  //sleep_us(D165);
  core1_safe_delay_ms(D165);
  
  // Clock the data out of the latch
  for(int i=0; i<8; i++)
    {
      
      // Read data
      value <<= 1;
      if( gpio_get(PIN_SDAIN) )
	{
	  value |= 1;
	}

      gpio_put(PIN_SCLKIN, 0);
      core1_safe_delay_ms(D165);
      //      sleep_us(D165);
      gpio_put(PIN_SCLKIN, 1);
      //sleep_us(D165);
      core1_safe_delay_ms(D165);
    }

  return(value);
}

////////////////////////////////////////////////////////////////////////////////
//
// Write an N bit pattern to a 595 latch or latches
//
////////////////////////////////////////////////////////////////////////////////

#define D595 1
volatile int core1_safe_x = 0;
volatile int id;

inline void core1_safe_delay_ms(uint32_t interval_ms)
{
  for(volatile int i=0; i<1000; i++)
    {
    }
}

void write_595(const uint latchpin, int value, int n)
{
  
  // Latch pin low
  gpio_put(latchpin, 0);
  //sleep_us(D595);
  core1_safe_delay_ms(D595);
  
  for(int i = 0; i<n; i++)
    {
      // Clock low
      gpio_put(PIN_SCLKOUT, 0);
      //sleep_us(D595);
      core1_safe_delay_ms(D595);
      
      // Set data up
      if( value & (1<< (n-1)) )
	{
	  gpio_put(PIN_SDAOUT, 1);
	}
      else
	{
	  gpio_put(PIN_SDAOUT, 0);
	}

      //sleep_us(D595);
      core1_safe_delay_ms(D595);
	
      // Shift to next data bit
      value <<= 1;
      
      // Clock data in
      gpio_put(PIN_SCLKOUT, 1);
      //sleep_us(D595);
      core1_safe_delay_ms(D595);
    }

  // Now latch value (update outputs)
  gpio_put(latchpin, 1);

}

////////////////////////////////////////////////////////////////////////////////
//
// Periodically displays information
//

int info_tick = 0;

void panel_info_init(void)
{
}

PANEL_T panel_info =
  {
    panel_info_init,
    {
      {1, 2, 15, "Core1 Ticks", PANEL_ITEM_TYPE_INT, (int *)&core1_ticks},
      {1, 1, 15, "",            PANEL_ITEM_TYPE_INT, NULL},
    }
  };

void info_task(void)
{
  
  if( (info_tick % 700)==0 )
    {
      do_panel(&panel_info, ITF_INFO, 1);
    }

  info_tick++;
}

////////////////////////////////////////////////////////////////////////////////
//
// OPL information
//

int opl_tick = 0;

void panel_opl_init(void)
{
}

PANEL_T panel_opl =
  {
    panel_opl_init,
    {
      //      {1, 1, 15, "SP",      PANEL_ITEM_TYPE_INT, (int *)&opl_machine.sp},
      {1, 1, 15, "",        PANEL_ITEM_TYPE_INT, NULL},
    }
  };

void opl_task(void)
{
  
  if( (opl_tick % 700)==0 )
    {
      do_panel(&panel_opl, ITF_OPL, 1);
    }

  opl_tick++;
}

////////////////////////////////////////////////////////////////////////////////
// If the card is physically removed, unmount the filesystem:
static volatile bool card_det_int_pend;
static volatile uint card_det_int_gpio;

static void card_detect_callback(uint gpio, uint32_t events) {
    (void)events;
    // This is actually an interrupt service routine!
    card_det_int_gpio = gpio;
    card_det_int_pend = true;
}

////////////////////////////////////////////////////////////////////////////////

int *lvadp;

int main(void) {
  int lvad;

  lvadp = &lvad;
  
  // Set up the GPIOs
  gpio_init(PIN_VBAT_SW_ON);
  gpio_set_dir(PIN_VBAT_SW_ON, GPIO_OUT);

  // Take power on high so we latch on
  gpio_put(PIN_VBAT_SW_ON, 1);

  // ON key
  gpio_init(PIN_P57);
  
  // Set GPIOs up
  gpio_init(PIN_SDAOUT);
  gpio_init(PIN_LATCHOUT2);
  gpio_init(PIN_LATCHOUT1);
  gpio_init(PIN_SDAIN);
  gpio_init(PIN_SCLKOUT);
  gpio_init(PIN_LATCHIN);
  gpio_init(PIN_SCLKIN);
  gpio_init(PIN_LS_DIR);
  
  gpio_init(PIN_SD0);
  gpio_init(PIN_SD1);
  gpio_init(PIN_SD2);
  gpio_init(PIN_SD3);
  gpio_init(PIN_SD4);
  gpio_init(PIN_SD5);
  gpio_init(PIN_SD6);
  gpio_init(PIN_SD7);

  // Turn on pull downs on the data bus
  gpio_pull_down(PIN_SD0);
  gpio_pull_down(PIN_SD1);
  gpio_pull_down(PIN_SD2);
  gpio_pull_down(PIN_SD3);
  gpio_pull_down(PIN_SD4);
  gpio_pull_down(PIN_SD5);
  gpio_pull_down(PIN_SD6);
  gpio_pull_down(PIN_SD7);
  
  gpio_init(PIN_SCLK);
  gpio_init(PIN_SOE);
  gpio_init(PIN_SMR);

  // I2C bus

  gpio_init(PIN_I2C_SCL);
  gpio_init(PIN_I2C_SDA);

#if PSION_MINI
  gpio_init(PIN_OLED_RES);
  gpio_init(PIN_OLED_DC);
  gpio_init(PIN_OLED_CS);

  gpio_set_dir(PIN_OLED_RES, GPIO_OUT);
  gpio_set_dir(PIN_OLED_DC,  GPIO_OUT);
  gpio_set_dir(PIN_OLED_CS,  GPIO_OUT);

  gpio_put(PIN_OLED_CS,  0);
  gpio_put(PIN_OLED_DC,  0);
  gpio_put(PIN_OLED_RES, 1);
  
  sleep_ms(10);
  gpio_put(PIN_OLED_RES, 0);
  sleep_ms(10);
  gpio_put(PIN_OLED_RES, 1);
#endif
  
  // Use pull ups
  gpio_pull_up(PIN_I2C_SDA);
  gpio_pull_up(PIN_I2C_SCL);

  //i2c_stop();

  gpio_set_dir(PIN_I2C_SCL, GPIO_OUT);
  gpio_set_dir(PIN_I2C_SDA, GPIO_OUT);
  
  gpio_set_dir(PIN_SDAOUT,    GPIO_OUT);
  gpio_set_dir(PIN_P57,       GPIO_IN);
  gpio_set_dir(PIN_SDAOUT,    GPIO_OUT);
  gpio_set_dir(PIN_LATCHOUT2, GPIO_OUT);
  gpio_set_dir(PIN_LATCHOUT1, GPIO_OUT);
  gpio_set_dir(PIN_SDAIN,     GPIO_IN);
  gpio_set_dir(PIN_SCLKOUT,   GPIO_OUT);
  gpio_set_dir(PIN_LATCHIN,   GPIO_OUT);
  gpio_set_dir(PIN_SCLKIN,    GPIO_OUT);
  gpio_set_dir(PIN_LS_DIR,    GPIO_OUT);
  
  gpio_set_dir(PIN_SCLK, GPIO_OUT);
  gpio_set_dir(PIN_SOE,  GPIO_OUT);
  gpio_set_dir(PIN_SMR,  GPIO_OUT);

  // Unlatch input latch
  //  gpio_put(PIN_SDAIN,  1);
  gpio_put(PIN_SCLKIN, 1);

  // SPI for SD card, this should be done by the SD card library
  gpio_init(22);
  gpio_set_dir(22, GPIO_OUT);
  gpio_init(18);
  gpio_set_dir(18, GPIO_OUT);
  gpio_init(19);
  gpio_set_dir(19, GPIO_OUT);
  gpio_init(16);
  gpio_set_dir(16, GPIO_IN);

#if SWITCH_EXTERNAL_PSRAM

  // Pico2 external memory
  gpio_set_function(PIMORONI_PICO_PLUS2_PSRAM_CS_PIN, GPIO_FUNC_XIP_CS1); // CS for PSRAM
  xip_ctrl_hw->ctrl|=XIP_CTRL_WRITABLE_M1_BITS;
#endif
  
  board_init();
  tusb_init();
#if 0
  sd_init_driver();
  sdcard_init();
#endif

#if 0
  for (size_t i = 0; i < sd_get_num(); ++i)
    {
      sd_card_t *sd_card_p = sd_get_by_num(i);
      if (!sd_card_p) 
        continue;
      if (sd_card_p->use_card_detect)
        {
          // Set up an interrupt on Card Detect to detect removal of the card
          // when it happens:
          gpio_set_irq_enabled_with_callback(
                                             sd_card_p->card_detect_gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                             true, &card_detect_callback);
        }
    }
#endif
  
#if 1
  ////////////////////////////core1_init();
  clear_oled();
#endif
  
  stdio_init_all();
  //  sleep_ms(400);

#if !PSION_MINI
  sleep_ms(10);
  initialise_oled();
#endif

  nopl_init();
  
  int tick = 0;

  menu_enter();
  
  int main_ticks = 0;

  printf("\nAbout to init...");
  
#if PSION_MINI
  dd_init(DD_SPI_SSD1309);
  //dd_init(DD_SPI_SSD1351);
  //main_er_oledm015_2();
#else
  dd_init(DD_I2C_SSD);
#endif

  // Implicitly called by disk_initialize,
  // but called here to set up the GPIOs
  // before enabling the card detect interrupt:
  sd_init_driver();

  printf("\nInit done");

  
  //sleep_ms(3000);
  dd_clear();
  
  while(true)
    {
      matrix_scan();
      menu_loop_tasks();

#if PSION_MINI
      menu_loop();
#else
      menu_loop();
#endif
      
      tud_task();

      opl_task();
      info_task();      
      sleep_ms(1);

      main_ticks++;

      if( main_ticks == 2000)
	{
	  for(int i = 0; i< 20; i++)
	    {
	      printf("\n");
	    }
	  
	  printf("\nC Based Recreation");
	  printf("\n------------------");
	  printf("\n");
	  serial_help();
	}
    }
}
