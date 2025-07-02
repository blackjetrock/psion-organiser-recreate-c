////////////////////////////////////////////////////////////////////////////////
//
// SD Card support
//
////////////////////////////////////////////////////////////////////////////////

#include "psion_recreate_all.h"
#include "hw_config.h"

#include "ff.h"
#include "sdcard.h"
#include "diskio.h"
#include "f_util.h"
#include "sd_card.h"

/* Declarations of disk functions */
// Hardware Configuration of SPI "objects"
// Note: multiple SD cards can be driven by one SPI if they use different slave
// selects.

static spi_t spis[] = {  // One for each SPI.
    {
        .hw_inst = spi0,  // SPI component
        .sck_gpio = 18,  // GPIO number (not Pico pin number)
        .mosi_gpio = 19,
        .miso_gpio = 16,
        .set_drive_strength = true,
        .mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_4MA,
        .sck_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,

        // .baud_rate = 25 * 1000 * 1000,  // Actual frequency: 20833333.
        .baud_rate = 125E6 / 4,  

        // .DMA_IRQ_num = DMA_IRQ_0
    }
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_cards[] = {  // One for each SD card
    {
        .pcName = "0:",  // Name used to mount device
        .spi = &spis[0],  // Pointer to the SPI driving this card
        .ss_gpio = 22,     // The SPI slave select GPIO for this SD card
        
        // .set_drive_strength = true,
        // .ss_gpio_drive_strength = GPIO_DRIVE_STRENGTH_4MA,

        // SD Card detect:
        .use_card_detect = false,
        .card_detect_gpio = 9,  
        .card_detected_true = 0, // What the GPIO read returns when a card is
                                 // present.
    }
};

size_t sd_get_num() { return count_of(sd_cards); }
sd_card_t *sd_get_by_num(size_t num) {
    assert(num <= sd_get_num());
    if (num <= sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}
size_t spi_get_num() { return count_of(spis); }
spi_t *spi_get_by_num(size_t num) {
    assert(num <= spi_get_num());
    if (num <= spi_get_num()) {
        return &spis[num];
    } else {
        return NULL;
    }
}

 sd_card_t *sd_get_by_name(const char *const name) {
    for (size_t i = 0; i < sd_get_num(); ++i)
        if (0 == strcmp(sd_get_by_num(i)->pcName, name)) return sd_get_by_num(i);
    printf("%s: unknown name %s\n", __func__, name);
    return NULL;
}

FATFS *sd_get_fs_by_name(const char *name) {
    for (size_t i = 0; i < sd_get_num(); ++i)
        if (0 == strcmp(sd_get_by_num(i)->pcName, name)) return &sd_get_by_num(i)->fatfs;
    printf("%s: unknown name %s\n", __func__, name);
    return NULL;
}

// If the card is physically removed, unmount the filesystem:
static void card_detect_callback(uint gpio, uint32_t events) {
    static bool busy;
    if (busy) return; // Avoid switch bounce
    busy = true;
    for (size_t i = 0; i < sd_get_num(); ++i) {
        sd_card_t *pSD = sd_get_by_num(i);
        if (pSD->card_detect_gpio == gpio) {
            if (pSD->mounted) {
                printf("(Card Detect Interrupt: unmounting %s)\n", pSD->pcName);
                FRESULT fr = f_unmount(pSD->pcName);
                if (FR_OK == fr) {
                    pSD->mounted = false;
                } else {
                    printf("f_unmount error: %s (%d)\n", FRESULT_str(fr), fr);
                }
            }
            pSD->m_Status |= STA_NOINIT; // in case medium is removed
            sd_card_detect(pSD);
        }
    }
    busy = false;
}

void run_mount() {
  const char *arg1 = strtok(NULL, " ");
  if (!arg1)
    {
      arg1 = sd_get_by_num(0)->pcName;
    }

  FATFS *p_fs = sd_get_fs_by_name(arg1);
  if (!p_fs)
    {
      printf("Unknown logical drive number: \"%s\"\n", arg1);
      return;
    }
  else
    {
      printf("\nmounting %s\n", arg1);
    }

  FRESULT fr = f_mount(p_fs, arg1, 1);

  if (FR_OK != fr)
    {
      printf("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
      return;
    }

  sd_card_t *pSD = sd_get_by_name(arg1);
  
  pSD->mounted = true;
}

void run_unmount() {
    const char *arg1 = strtok(NULL, " ");
    if (!arg1) arg1 = sd_get_by_num(0)->pcName;
    FATFS *p_fs = sd_get_fs_by_name(arg1);
    if (!p_fs) {
        printf("Unknown logical drive number: \"%s\"\n", arg1);
        return;
    }
    FRESULT fr = f_unmount(arg1);
    if (FR_OK != fr) {
        printf("f_unmount error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    sd_card_t *pSD = sd_get_by_name(arg1);

    pSD->mounted = false;
    pSD->m_Status |= STA_NOINIT; // in case medium is removed
}

void ls(const char *dir)
{
  char cwdbuf[FF_LFN_BUF] = {0};
  
  FRESULT fr; /* Return value */
  char const *p_dir;
  
  if (strlen(dir) > 0)
    {
      p_dir = dir;
    }
  else
    {
      fr = f_getcwd(cwdbuf, sizeof cwdbuf);

      if (FR_OK != fr)
        {
          printf("f_getcwd error: %s (%d)\n", FRESULT_str(fr), fr);
          return;
        }
      p_dir = cwdbuf;
    }
  
  printf("Directory Listing: %s\n", p_dir);
  
  DIR dj;      /* Directory object */
  FILINFO fno; /* File information */

  memset(&dj, 0, sizeof dj);
  memset(&fno, 0, sizeof fno);

  fr = f_findfirst(&dj, &fno, p_dir, "*");

  if (FR_OK != fr) {
    printf("f_findfirst error: %s (%d)\n", FRESULT_str(fr), fr);
    return;
    }
  
  while (fr == FR_OK && fno.fname[0])
    { /* Repeat while an item is found */
      /* Create a string that includes the file name, the file size and the
         attributes string. */
      const char *pcWritableFile = "writable file",
        *pcReadOnlyFile = "read only file",
        *pcDirectory = "directory";
      const char *pcAttrib;

      /* Point pcAttrib to a string that describes the file. */
      if (fno.fattrib & AM_DIR)
        {
          pcAttrib = pcDirectory;
        }
      else if (fno.fattrib & AM_RDO)
        {
          pcAttrib = pcReadOnlyFile;
        }
      else
        {
          pcAttrib = pcWritableFile;
        }
      
      /* Create a string that includes the file name, the file size and the
         attributes string. */
      printf("%s [%s] [size=%llu]\n", fno.fname, pcAttrib, fno.fsize);
      
      fr = f_findnext(&dj, &fno); /* Search for next item */
    }
  f_closedir(&dj);
}


////////////////////////////////////////////////////////////////////////////////
//
// Initialise SD Card
//
////////////////////////////////////////////////////////////////////////////////


void sdcard_init(void)
{
  
  for (size_t i = 0; i < sd_get_num(); ++i)
    {
      sd_card_t *pSD = sd_get_by_num(i);
      if (pSD->use_card_detect)
        {
          // Set up an interrupt on Card Detect to detect removal of the card
          // when it happens:
          gpio_set_irq_enabled_with_callback(pSD->card_detect_gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &card_detect_callback);
        }
    }
  
#if 0
    for (;;) {  // Super Loop
        if (logger_enabled &&
            absolute_time_diff_us(get_absolute_time(), next_log_time) < 0) {
            if (!process_logger()) logger_enabled = false;
            next_log_time = delayed_by_ms(next_log_time, period);
        }
        int cRxedChar = getchar_timeout_us(0);
        /* Get the character from terminal */
        if (PICO_ERROR_TIMEOUT != cRxedChar) process_stdio(cRxedChar);
    }
#endif
}
