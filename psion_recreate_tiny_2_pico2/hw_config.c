
/* hw_config.c
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

   http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
*/

/*

This file should be tailored to match the hardware design.

See 
  https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/tree/main#customizing-for-the-hardware-configuration

There should be one element of the spi[] array for each RP2040 hardware SPI used.

There should be one element of the spi_ifs[] array for each SPI interface object.
* Each element of spi_ifs[] must point to an spi_t instance with member "spi".

There should be one element of the sdio_ifs[] array for each SDIO interface object.

There should be one element of the sd_cards[] array for each SD card slot.
* Each element of sd_cards[] must point to its interface with spi_if_p or sdio_if_p.
*/

/* Hardware configuration for Pico SD Card Development Board
See https://oshwlab.com/carlk3/rp2040-sd-card-dev

See https://docs.google.com/spreadsheets/d/1BrzLWTyifongf_VQCc2IpJqXWtsrjmG7KnIbSBy-CPU/edit?usp=sharing,
tab "Dev Brd", for pin assignments assumed in this configuration file.
*/

#include <assert.h>
//
#include "hw_config.h"

#include "psion_recreate_all.h"

spi_t spis[] = {  // One for each SPI.
    {
        // spis[0]
        .hw_inst = spi0,  // RP2040 SPI component
        .sck_gpio = 18,    // GPIO number (not Pico pin number)
        .mosi_gpio = 19,
        .miso_gpio = 16,
        .spi_mode = 0,
        
        /* GPIO reset drive strength is 4 mA.
         * When set_drive_strength is true,
         * the drive strength is set to 2 mA unless otherwise specified. */
        .set_drive_strength = true,
        .mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,
        .sck_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,
        .no_miso_gpio_pull_up = false,
        // .baud_rate = 125 * 1000 * 1000 / 10 // 12500000 Hz
        // .baud_rate = 125 * 1000 * 1000 / 8  // 15625000 Hz
        .baud_rate = 125 * 1000 * 1000 / 10  // 20833333 Hz
        // .baud_rate = 125 * 1000 * 1000 / 4  // 31250000 Hz

        // .DMA_IRQ_num = DMA_IRQ_0
    }
};
/* SPI Interfaces */
static sd_spi_if_t spi_ifs[] = {
    {   // spi_ifs[0]
        .spi = &spis[0],  // Pointer to the SPI driving this card
#if PICOCALC
        .ss_gpio = 17,     // The SPI slave select GPIO for this SD card
#else
        .ss_gpio = 22,     // The SPI slave select GPIO for this SD card
#endif
        .set_drive_strength = true,
        .ss_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA
    },
};

// Hardware Configuration of the SD Card "objects"
 sd_card_t sd_cards[] = {  // One for each SD card
    {
        .type = SD_IF_SPI,
        .spi_if_p = &spi_ifs[0],  // Pointer to the SPI interface driving this card
        // SD Card detect:
        .use_card_detect = false,
        .card_detect_gpio = 9,  
        .card_detected_true = 0, // What the GPIO read returns when a card is
                                 // present.
        .card_detect_use_pull = true,
        .card_detect_pull_hi = true                                 
    }
};

// See http://elm-chan.org/fsw/ff/doc/config.html#volumes
// and ffconf.h
const char *VolumeStr[FF_VOLUMES] = {"sd0"};	/* Pre-defined volume ID */

/* ********************************************************************** */

//size_t sd_get_num() { return count_of(sd_cards); }
size_t sd_get_num() { return 1; }

/**
 * @brief Get a pointer to an SD card object by its number.
 *
 * @param[in] num The number of the SD card to get.
 *
 * @return A pointer to the SD card object, or @c NULL if the number is invalid.
 */
sd_card_t *sd_get_by_num(size_t num) {
    assert(num < sd_get_num());
    if (num < sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}

/* [] END OF FILE */
