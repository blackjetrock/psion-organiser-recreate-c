/**
 * PicoCalc Hello World
 * https://www.clockworkpi.com/
 */
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "i2ckbd.h"
#include "lcdspi.h"
//#include "psram_spi.h"
//#include "pwm_sound.h"

#include "psion_recreate_all.h"

const uint LEDPIN = 25;

#if 0
psram_spi_inst_t* async_spi_inst;

int psram_test(psram_spi_inst_t*psram_spi){
    uint32_t psram_begin, psram_elapsed;
    float psram_speed;
    char buf[128];
    sprintf(buf,"Testing PSRAM...\n");
    printf("%s",buf);
    lcd_print_string(buf);

    // **************** 8 bits testing ****************
    psram_begin = time_us_32();
    for (uint32_t addr = 0; addr < (8 * 1024 * 1024); ++addr) {
        psram_write8(psram_spi, addr, (addr & 0xFF));
    }
    psram_elapsed = time_us_32() - psram_begin;
    psram_speed = 1000000.0 * 8 * 1024.0 * 1024 / psram_elapsed;
    sprintf(buf,"8 bit: PSRAM write 8MB in %d us, %d B/s\n", psram_elapsed, (uint32_t)psram_speed);
    printf("%s", buf);
    lcd_print_string(buf);

    psram_begin = time_us_32();
    for (uint32_t addr = 0; addr < (8 * 1024 * 1024); ++addr) {
        psram_write8_async(psram_spi, addr, (addr & 0xFF));
    }
    psram_elapsed = time_us_32() - psram_begin;
    psram_speed = 1000000.0 * 8 * 1024.0 * 1024 / psram_elapsed;
    sprintf(buf,"8 bit: PSRAM write async 8MB in %d us, %d B/s\n", psram_elapsed, (uint32_t)psram_speed);
    printf("%s", buf);
    lcd_print_string(buf);

    psram_begin = time_us_32();
    for (uint32_t addr = 0; addr < (8 * 1024 * 1024); ++addr) {
        uint8_t result = psram_read8(psram_spi, addr);
        if ((uint8_t)(addr & 0xFF) != result) {
            sprintf(buf,"\nPSRAM failure at address %x (%x != %x)\n", addr, addr & 0xFF, result);
            printf("%s", buf);
            lcd_print_string(buf);
            return 1;
        }
    }
    psram_elapsed = time_us_32() - psram_begin;
    psram_speed = 1000000.0 * 8 * 1024.0 * 1024 / psram_elapsed;
    sprintf(buf,"8 bit: PSRAM read 8MB in %d us, %d B/s\n", psram_elapsed, (uint32_t)psram_speed);
    printf("%s", buf);
    lcd_print_string(buf);

    // **************** 16 bits testing ****************
    psram_begin = time_us_32();
    for (uint32_t addr = 0; addr < (8 * 1024 * 1024); addr += 2) {
        psram_write16(psram_spi, addr, (((addr + 1) & 0xFF) << 8) | (addr & 0xFF));
    }
    psram_elapsed = time_us_32() - psram_begin;
    psram_speed = 1000000.0 * 8 * 1024.0 * 1024 / psram_elapsed;
    sprintf(buf,"16 bit: PSRAM write 8MB in %d us, %d B/s\n", psram_elapsed, (uint32_t)psram_speed);
    printf("%s", buf);
    lcd_print_string(buf);

    psram_begin = time_us_32();
    for (uint32_t addr = 0; addr < (8 * 1024 * 1024); addr += 2) {
        uint16_t result = psram_read16(psram_spi, addr);
        if ((uint16_t)(
                (((addr + 1) & 0xFF) << 8) |
                (addr & 0xFF)) != result
                ) {
            sprintf(buf,"PSRAM failure at address %x (%x != %x) ", addr, (
                    (((addr + 1) & 0xFF) << 8) |
                    (addr & 0xFF)), result
            );
            printf("%s", buf);
            lcd_print_string(buf);
            return 1;
        }
    }
    psram_elapsed = (time_us_32() - psram_begin);
    psram_speed = 1000000.0 * 8 * 1024 * 1024 / psram_elapsed;
    sprintf(buf,"16 bit: PSRAM read 8MB in %d us, %d B/s\n", psram_elapsed, (uint32_t)psram_speed);
    printf("%s", buf);
    lcd_print_string(buf);

    // **************** 32 bits testing ****************
    psram_begin = time_us_32();
    for (uint32_t addr = 0; addr < (8 * 1024 * 1024); addr += 4) {
        psram_write32(
                psram_spi, addr,
                (uint32_t)(
                        (((addr + 3) & 0xFF) << 24) |
                        (((addr + 2) & 0xFF) << 16) |
                        (((addr + 1) & 0xFF) << 8)  |
                        (addr & 0XFF))
        );
    }
    psram_elapsed = time_us_32() - psram_begin;
    psram_speed = 1000000.0 * 8 * 1024.0 * 1024 / psram_elapsed;
    sprintf(buf,"32 bit: PSRAM write 8MB in %d us, %d B/s\n", psram_elapsed, (uint32_t)psram_speed);
    printf("%s", buf);
    lcd_print_string(buf);

    psram_begin = time_us_32();
    for (uint32_t addr = 0; addr < (8 * 1024 * 1024); addr += 4) {
        uint32_t result = psram_read32(psram_spi, addr);
        if ((uint32_t)(
                (((addr + 3) & 0xFF) << 24) |
                (((addr + 2) & 0xFF) << 16) |
                (((addr + 1) & 0xFF) << 8)  |
                (addr & 0XFF)) != result
                ) {
            sprintf(buf,"PSRAM failure at address %x (%x != %x) ", addr, (
                    (((addr + 3) & 0xFF) << 24) |
                    (((addr + 2) & 0xFF) << 16) |
                    (((addr + 1) & 0xFF) << 8)  |
                    (addr & 0XFF)), result
            );
            printf("%s", buf);
            lcd_print_string(buf);
            return 1;
        }
    }
    psram_elapsed = (time_us_32() - psram_begin);
    psram_speed = 1000000.0 * 8 * 1024 * 1024 / psram_elapsed;
    sprintf(buf,"32 bit: PSRAM read 8MB in %d us, %d B/s\n", psram_elapsed, (uint32_t)psram_speed);
    printf("%s", buf);
    lcd_print_string(buf);
    // **************** n bits testing ****************
    uint8_t write_data[256];
    for (size_t i = 0; i < 256; ++i) {
        write_data[i] = i;
    }
    psram_begin = time_us_32();
    for (uint32_t addr = 0; addr < (8 * 1024 * 1024); addr += 256) {
        for (uint32_t step = 0; step < 256; step += 16) {
            psram_write(psram_spi, addr + step, write_data + step, 16);
        }
    }
    psram_elapsed = time_us_32() - psram_begin;
    psram_speed = 1000000.0 * 8 * 1024.0 * 1024 / psram_elapsed;
    sprintf(buf,"128 bit: PSRAM write 8MB in %d us, %d B/s\n", psram_elapsed, (uint32_t)psram_speed);
    printf("%s", buf);
    lcd_print_string(buf);

    psram_begin = time_us_32();
    uint8_t read_data[16];
    for (uint32_t addr = 0; addr < (8 * 1024 * 1024); addr += 256) {
        for (uint32_t step = 0; step < 256; step += 16) {
            psram_read(psram_spi, addr + step, read_data, 16);
            if (memcmp(read_data, write_data + step, 16) != 0) {
                sprintf(buf,"PSRAM failure at address %x", addr);
                printf("%s", buf);
                lcd_print_string(buf);
                return 1;
            }
        }
    }
    psram_elapsed = time_us_32() - psram_begin;
    psram_speed = 1000000.0 * 8 * 1024.0 * 1024 / psram_elapsed;
    sprintf(buf,"128 bit: PSRAM read 8MB in %d us, %d B/s\n", psram_elapsed, (uint32_t)psram_speed);
    printf("%s", buf);
    lcd_print_string(buf);

    lcd_print_string("PSRAM testing done\n");
}


#include "sample.h"
int wav_position = 0;

void pwm_interrupt_handler() {
    int slice_l = pwm_gpio_to_slice_num(AUDIO_PIN_L);
    int slice_r = pwm_gpio_to_slice_num(AUDIO_PIN_R);

    pwm_clear_irq(slice_l);
    pwm_clear_irq(slice_r);

    if (wav_position < (WAV_DATA_LENGTH << 3) - 1) {
        pwm_set_chan_level(pwm_gpio_to_slice_num(AUDIO_PIN_L), PWM_CHAN_A, WAV_DATA[wav_position >> 3]);
        pwm_set_chan_level(pwm_gpio_to_slice_num(AUDIO_PIN_R), PWM_CHAN_B, WAV_DATA[wav_position >> 3]);

        wav_position++;
    } else {
        wav_position = 0; // Stop
        pwm_set_chan_level(slice_l, PWM_CHAN_A, 0);
        pwm_set_chan_level(slice_r, PWM_CHAN_B, 0);

        pwm_set_irq_enabled(slice_l, false);
        pwm_set_irq_enabled(slice_r, false);
        irq_remove_handler(PWM_IRQ_WRAP, pwm_interrupt_handler);
        //
    }
}
void test_battery(){
    char buf[64];
    int bat_pcnt = read_battery();
    bat_pcnt = bat_pcnt>>8;
    int bat_charging = bitRead(bat_pcnt,7);
    bitClear(bat_pcnt,7);
    sprintf(buf,"battery percent is %d\n",bat_pcnt);
    printf("%s", buf);
    lcd_print_string(buf);
    if(bat_charging ==0 ){
        sprintf(buf,"battery is not charging\n");
    }else{
        sprintf(buf,"battery is charging\n");
    }
    printf("%s", buf);
    lcd_print_string(buf);
}

void test_kbd_backlight(uint8_t val) {
    char buf[64];
    int kbd_backlight = set_kbd_backlight(val);
    kbd_backlight = kbd_backlight>>8;

    sprintf(buf, "kbd backlight %d\n", kbd_backlight);
    printf("%s", buf);
    lcd_print_string(buf);
}

#endif

int hwmain(void)
{
  
#if 1
  set_sys_clock_khz(133000, true);
#endif
  
  stdio_init_all();

  uart_init(uart0, 115200);
  
  uart_set_format(uart0, 8, 1, UART_PARITY_NONE);  // 8-N-1
  uart_set_fifo_enabled(uart0, false);
  
  init_i2c_kbd();
  picocalc_lcd_init();
  
  gpio_init(LEDPIN);
  gpio_set_dir(LEDPIN, GPIO_OUT);
  
  picocalc_lcd_clear();
  picocalc_lcd_print_string("Hello World PicoCalc\n");
  gpio_put(LEDPIN, 1);
  sleep_ms(500);
  gpio_put(LEDPIN, 0);

  //return(0);
  
#if 0
  init_pwm(pwm_interrupt_handler);
  
  psram_spi_inst_t psram_spi = psram_spi_init_clkdiv(pio1, -1,1.0f,true);
  psram_test(&psram_spi);

  // if we need battery information
  // ** we should power on the picocalc only with batteries
    // ** then plug the type c cable
    // otherwise we won't get working battery infos

    sleep_ms(2000);
    test_battery();
    test_kbd_backlight(50);
    sleep_ms(1000);
    test_kbd_backlight(100);
    sleep_ms(1000);
    test_kbd_backlight(255);
    sleep_ms(1000);
    test_kbd_backlight(0);

#endif
    while (1) {
        int c = lcd_getc(0);
        if(c != -1 && c > 0) {
            picocalc_lcd_putc(c);
        }
        sleep_ms(20);
    }
}

