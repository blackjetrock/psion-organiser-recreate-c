////////////////////////////////////////////////////////////////////////////////
//
// EEPROM
//
////////////////////////////////////////////////////////////////////////////////


// EEPROM 0 is used to hold up to 64K of RAM data when the machine is off.
// it is restored at power on if valid

// Some areas of RAM are not used as they don't exist on the real organiser
// so these are used for meta data
#define PAGE_SIZE 128

// EEPROM meta data

// Checksum of RAM data in EEPROM 0 (summed with 0 in these locations)
#define EEPROM_CSUM_LEN  2
#define EEPROM_CSUM_H    0x0182
#define EEPROM_CSUM_L    0x0183

// Offset of memories in RAM copy
#define EEPROM_OFF_COPY_MEMS_START      0x20ff
#define EEPROM_OFF_COPY_MEMS_END        0x214e
#define EEPROM_LEN_COPY_MEMS            (EEPROM_OFF_COPY_MEMS_END - EEPROM_OFF_COPY_MEMS_START+1)

// Offsets of saved memories in second EEPROM chip
#define EEPROM_OFF_SAVED_MEMS_0_START   0x1000


extern volatile int eeprom_do_invalidate;
extern volatile int eeprom_do_dump;
extern volatile int eeprom_do_restore;
extern volatile int eeprom_done_dump;
extern volatile int eeprom_done_restore;
extern volatile int eeprom_done_invalidate;

void eeprom_ram_check(void);
void eeprom_tasks(void);
void eeprom_perform_restore(void);
void eeprom_perform_dump(void);
void eeprom_test2(void);
void eeprom_test3(void);



