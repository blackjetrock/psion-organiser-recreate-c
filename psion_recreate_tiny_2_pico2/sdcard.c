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
static volatile bool card_det_int_pend;
static volatile uint card_det_int_gpio;

 spi_t spis[] = {  // One for each SPI.
    {
        // spis[0]
        .hw_inst = spi0,  // RP2040 SPI component
        .sck_gpio = 18,    // GPIO number (not Pico pin number)
        .mosi_gpio = 19,
        .miso_gpio = 16,

        /* GPIO reset drive strength is 4 mA.
         * When set_drive_strength is true,
         * the drive strength is set to 2 mA unless otherwise specified. */
        .set_drive_strength = true,
        .mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,
        .sck_gpio_drive_strength = GPIO_DRIVE_STRENGTH_12MA,
        .no_miso_gpio_pull_up = true,
        // .baud_rate = 125 * 1000 * 1000 / 10 // 12500000 Hz
        // .baud_rate = 125 * 1000 * 1000 / 8  // 15625000 Hz
        .baud_rate = 125 * 1000 * 1000 / 6  // 20833333 Hz
        // .baud_rate = 125 * 1000 * 1000 / 4  // 31250000 Hz

#if 0        
        .hw_inst = spi0,  // SPI component
        .sck_gpio = 18,  // GPIO number (not Pico pin number)
        .mosi_gpio = 19,
        .miso_gpio = 16,
        .set_drive_strength = true,
        .mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_4MA,
        .sck_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,

        // .baud_rate = 25 * 1000 * 1000,  // Actual frequency: 20833333.
        .baud_rate = 125E6 / 4,  
#endif
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
        .ss_gpio = 7,     // The SPI slave select GPIO for this SD card
#endif
        .set_drive_strength = true,
        .ss_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA
    },
    {   // spi_ifs[1]
        .spi = &spis[1],   // Pointer to the SPI driving this card
        .ss_gpio = 12,     // The SPI slave select GPIO for this SD card
        .set_drive_strength = true,
        .ss_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA
    },
    {   // spi_ifs[2]
        .spi = &spis[1],   // Pointer to the SPI driving this card
        .ss_gpio = 13,     // The SPI slave select GPIO for this SD card
        .set_drive_strength = true,
        .ss_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA
    }
};

// Hardware Configuration of the SD Card "objects"
 sd_card_t sd_cards[] = {  // One for each SD card
    {
        .type = SD_IF_SPI,
        .spi_if_p = &spi_ifs[0],  // Pointer to the SPI interface driving this card
        // SD Card detect:
        .use_card_detect = true,
#if PICOCALC
        .card_detect_gpio = 22,
#else
        .card_detect_gpio = 9,
#endif        
        .card_detected_true = 0, // What the GPIO read returns when a card is
                                 // present.
        .card_detect_use_pull = true,
        .card_detect_pull_hi = true                                 
#if 0
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
#endif
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

#if 0
 sd_card_t *sd_get_by_name(const char *const name) {
    for (size_t i = 0; i < sd_get_num(); ++i)
        if (0 == strcmp(sd_get_by_num(i)->pcName, name)) return sd_get_by_num(i);
    printf("%s: unknown name %s\n", __func__, name);
    return NULL;
}
#endif
#if 0
FATFS *sd_get_fs_by_name(const char *name) {
    for (size_t i = 0; i < sd_get_num(); ++i)
        if (0 == strcmp(sd_get_by_num(i)->pcName, name)) return &sd_get_by_num(i)->fatfs;
    printf("%s: unknown name %s\n", __func__, name);
    return NULL;
}
#endif

// If the card is physically removed, unmount the filesystem:
static void card_detect_callback(uint gpio, uint32_t events) {
    (void)events;
    // This is actually an interrupt service routine!
    card_det_int_gpio = gpio;
    card_det_int_pend = true;
}

#if 0
// If the card is physically removed, unmount the filesystem:
 void card_detect_callback(uint gpio, uint32_t events) {
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
#endif

void run_mount(void) {
  //    const char *arg = chk_dflt_log_drv(argc, argv);
  const char *arg = "0:";
  if (!arg)
        return;
    sd_card_t *sd_card_p = sd_get_by_drive_prefix(arg);
    if (!sd_card_p) {
        printf("Unknown logical drive id: \"%s\"\n", arg);
        return;
    }
    FATFS *fs_p = &sd_card_p->state.fatfs;
    FRESULT fr = f_mount(fs_p, arg, 1);
    if (FR_OK != fr) {
        printf("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    sd_card_p->state.mounted = true;
}

void run_unmount(void) {
  //    const char *arg = chk_dflt_log_drv(argc, argv);
  const char *arg = "0:";
  
  if (!arg)
        return;

    sd_card_t *sd_card_p = sd_get_by_drive_prefix(arg);
    if (!sd_card_p) {
        printf("Unknown logical drive id: \"%s\"\n", arg);
        return;
    }    
    FRESULT fr = f_unmount(arg);
    if (FR_OK != fr) {
        printf("f_unmount error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    sd_card_p->state.mounted = false;
    sd_card_p->state.m_Status |= STA_NOINIT;  // in case medium is removed
}
#if 0
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
#endif

#if 0
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


#endif

void run_info(void)
{
  //    const char *arg = chk_dflt_log_drv(argc, argv);
  const char *arg = "0:";
  
    if (!arg)
        return;
    sd_card_t *sd_card_p = sd_get_by_drive_prefix(arg);
    if (!sd_card_p) {
        printf("Unknown logical drive id: \"%s\"\n", arg);
        return;
    }
    int ds = sd_card_p->init(sd_card_p);
    if (STA_NODISK & ds || STA_NOINIT & ds) {
        printf("SD card initialization failed\n");
        return;
    }
    // Card IDendtification register. 128 buts wide.
    cidDmp(sd_card_p, printf);
    // Card-Specific Data register. 128 bits wide.
    csdDmp(sd_card_p, printf);
    
    // SD Status
    size_t au_size_bytes;
    bool ok = sd_allocation_unit(sd_card_p, &au_size_bytes);
    if (ok)
        printf("\nSD card Allocation Unit (AU_SIZE) or \"segment\": %zu bytes (%zu sectors)\n", 
            au_size_bytes, au_size_bytes / sd_block_size);
    
    if (!sd_card_p->state.mounted) {
        printf("Drive \"%s\" is not mounted\n", arg);
        return;
    }

    /* Get volume information and free clusters of drive */
    FATFS *fs_p = &sd_card_p->state.fatfs;
    if (!fs_p) {
        printf("Unknown logical drive id: \"%s\"\n", arg);
        return;
    }
    DWORD fre_clust, fre_sect, tot_sect;
    FRESULT fr = f_getfree(arg, &fre_clust, &fs_p);
    if (FR_OK != fr) {
        printf("f_getfree error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    /* Get total sectors and free sectors */
    tot_sect = (fs_p->n_fatent - 2) * fs_p->csize;
    fre_sect = fre_clust * fs_p->csize;
    /* Print the free space (assuming 512 bytes/sector) */
    printf("\n%10lu KiB (%lu MiB) total drive space.\n%10lu KiB (%lu MiB) available.\n",
           tot_sect / 2, tot_sect / 2 / 1024,
           fre_sect / 2, fre_sect / 2 / 1024);

#if FF_USE_LABEL
    // Report label:
    TCHAR buf[34] = {};/* [OUT] Volume label */
    DWORD vsn;         /* [OUT] Volume serial number */
    fr = f_getlabel(arg, buf, &vsn);
    if (FR_OK != fr) {
        printf("f_getlabel error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    } else {
        printf("\nVolume label: %s\nVolume serial number: %lu\n", buf, vsn);
    }
#endif

    // Report format
    //printf("\nFilesystem type: %s\n", fs_type_string(fs_p->fs_type));

    // Report Partition Starting Offset
    // uint64_t offs = fs_p->volbase;
    // printf("Partition Starting Offset: %llu sectors (%llu bytes)\n",
    //         offs, offs * sd_block_size);
	printf("Volume base sector: %llu\n", fs_p->volbase);		
	printf("FAT base sector: %llu\n", fs_p->fatbase);		
	printf("Root directory base sector (FAT12/16) or cluster (FAT32/exFAT): %llu\n", fs_p->dirbase);		 
	printf("Data base sector: %llu\n", fs_p->database);		

    // Report cluster size ("allocation unit")
    printf("FAT Cluster size (\"allocation unit\"): %d sectors (%llu bytes)\n",
           fs_p->csize,
           (uint64_t)sd_card_p->state.fatfs.csize * FF_MAX_SS);
}

#if 0
 void run_getfree() {
   const char *arg1;

    //if (!arg1)
      arg1 = sd_get_by_num(0)->pcName;

    DWORD fre_clust, fre_sect, tot_sect;
    /* Get volume information and free clusters of drive */
    FATFS *p_fs = sd_get_fs_by_name(arg1);
    if (!p_fs) {
        printf("Unknown logical drive number: \"%s\"\n", arg1);
        return;
    }
    FRESULT fr = f_getfree(arg1, &fre_clust, &p_fs);
    if (FR_OK != fr) {
        printf("f_getfree error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    /* Get total sectors and free sectors */
    tot_sect = (p_fs->n_fatent - 2) * p_fs->csize;
    fre_sect = fre_clust * p_fs->csize;
    /* Print the free space (assuming 512 bytes/sector) */
    printf("%10lu KiB total drive space.\n%10lu KiB available.\n", tot_sect / 2,
           fre_sect / 2);
}
#endif

 void run_cd(char *arg1) {
#if 0
    char *arg1 = strtok(NULL, " ");
    if (!arg1) {
        printf("Missing argument\n");
        return;
    }
#endif
    FRESULT fr = f_chdir(arg1);
    if (FR_OK != fr) printf("f_mkfs error: %s (%d)\n", FRESULT_str(fr), fr);
}

 void run_mkdir(char *arg1) {
#if 0
   char *arg1 = strtok(NULL, " ");
    if (!arg1) {
        printf("Missing argument\n");
        return;
    }
#endif
    FRESULT fr = f_mkdir(arg1);
    if (FR_OK != fr) printf("f_mkfs error: %s (%d)\n", FRESULT_str(fr), fr);
}

//------------------------------------------------------------------------------

void run_cat(char *filename)
{
  if (strlen(filename) == 0)
    {
      printf("Missing argument\n");
      return;
    }
  
  FIL fil;
  FRESULT fr = f_open(&fil, filename, FA_READ);
  
  if (FR_OK != fr)
    {
      printf("f_open error: %s (%d)\n", FRESULT_str(fr), fr);
      return;
    }
  
  char buf[256];
  while (f_gets(buf, sizeof buf, &fil))
    {
      printf("%s", buf);
    }

  fr = f_close(&fil);

  if (FR_OK != fr)
    {
      printf("f_open error: %s (%d)\n", FRESULT_str(fr), fr);
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// Initialise SD Card
//
////////////////////////////////////////////////////////////////////////////////


void sdcard_init(void)
{
#if 0
  for (size_t i = 0; i < sd_get_num(); ++i)
    {
      sd_card_t *pSD = sd_get_by_num(i);
      if (pSD->use_card_detect)
        {
          // Set up an interrupt on Card Detect to detect removal of the card
          // when it happens:
          //gpio_set_irq_enabled_with_callback(pSD->card_detect_gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &card_detect_callback);
          gpio_set_irq_enabled_with_callback(sd_card_p->card_detect_gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &card_detect_callback);
        }
    }
#endif

    for (size_t i = 0; i < sd_get_num(); ++i) {
        sd_card_t *sd_card_p = sd_get_by_num(i);
        if (!sd_card_p)
            continue;
        if (sd_card_p->card_detect_gpio == card_det_int_gpio) {
            if (sd_card_p->state.mounted) {
                DBG_PRINTF("(Card Detect Interrupt: unmounting %s)\n", sd_get_drive_prefix(sd_card_p));
                FRESULT fr = f_unmount(sd_get_drive_prefix(sd_card_p));
                if (FR_OK == fr) {
                    sd_card_p->state.mounted = false;
                } else {
                    printf("f_unmount error: %s (%d)\n", FRESULT_str(fr), fr);
                }
            }
            sd_card_p->state.m_Status |= STA_NOINIT;  // in case medium is removed
            sd_card_detect(sd_card_p);
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

bool sd_init_driver() {
    auto_init_mutex(initialized_mutex);
    mutex_enter_blocking(&initialized_mutex);
    bool ok = true;
    if (!driver_initialized) {
        myASSERT(sd_get_num());
        for (size_t i = 0; i < sd_get_num(); ++i) {
            sd_card_t *sd_card_p = sd_get_by_num(i);
            if (!sd_card_p) continue;

            myASSERT(sd_card_p->type);

            if (!mutex_is_initialized(&sd_card_p->state.mutex))
                mutex_init(&sd_card_p->state.mutex);
            sd_lock(sd_card_p);

            sd_card_p->state.m_Status = STA_NOINIT;

            sd_set_drive_prefix(sd_card_p, i);

            // Set up Card Detect
            if (sd_card_p->use_card_detect) {
                if (sd_card_p->card_detect_use_pull) {
                    if (sd_card_p->card_detect_pull_hi) {
                        gpio_pull_up(sd_card_p->card_detect_gpio);
                    } else {
                        gpio_pull_down(sd_card_p->card_detect_gpio);
                    }
                }
                gpio_init(sd_card_p->card_detect_gpio);
            }

            switch (sd_card_p->type) {
                case SD_IF_NONE:
                    myASSERT(false);
                    break;
                case SD_IF_SPI:
                    myASSERT(sd_card_p->spi_if_p);  // Must have an interface object
                    myASSERT(sd_card_p->spi_if_p->spi);
                    sd_spi_ctor(sd_card_p);
                    if (!my_spi_init(sd_card_p->spi_if_p->spi)) {
                        ok = false;
                    }
                    /* At power up the SD card CD/DAT3 / CS  line has a 50KOhm pull up enabled
                     * in the card. This resistor serves two functions Card detection and Mode
                     * Selection. For Mode Selection, the host can drive the line high or let it
                     * be pulled high to select SD mode. If the host wants to select SPI mode it
                     * should drive the line low.
                     *
                     * There is an important thing needs to be considered that the MMC/SDC is
                     * initially NOT the SPI device. Some bus activity to access another SPI
                     * device can cause a bus conflict due to an accidental response of the
                     * MMC/SDC. Therefore the MMC/SDC should be initialized to put it into the
                     * SPI mode prior to access any other device attached to the same SPI bus.
                     */
                    sd_go_idle_state(sd_card_p);
                    break;
                case SD_IF_SDIO:
                    myASSERT(sd_card_p->sdio_if_p);
                    sd_sdio_ctor(sd_card_p);
                    break;
                default:
                    myASSERT(false);
            }  // switch (sd_card_p->type)

            sd_unlock(sd_card_p);
        }  // for
        driver_initialized = true;
    }
    mutex_exit(&initialized_mutex);
    return ok;
}

