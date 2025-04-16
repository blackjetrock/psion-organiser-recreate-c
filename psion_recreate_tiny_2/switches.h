// Switches and debug

#define KEY_SCAN_IN_MAIN 1

#define DB_KB_GETK       0
#define DB_KB_DEBOUNCE   0
#define DB_KB_MATRIX     0
#define DB_KB_TEST       0
#define DB_DIGIT         0
#define DB_PK_SAVE       0
#define DB_PK_SETP       0
#define DB_CORE1         0

#define DB_FL_POS_AT_END 0
#define DB_FL_SCAN_PAK   0
#define DB_FL_CATL       0
#define DB_FL_WRIT       0
#define DB_FL_FREC       0
#define DB_FL_READ       0
#define DB_FL_SIZE       0
#define DB_FL_FIND       0
#define DB_FL_ERAS       0

#define DB_ED_EPOS       0
#define DB_NEXT_PRINTPOS 0

// Key scanning in core1 main() or core0 loops?
#define CORE0_SCAN       0

// Different builds
#define PSION_MINI       1   // Small hardware version
#define PSION_RECREATE   0   // Hardware replacement for original case

#define SPI              1
#define DD               1   // Display driver scheme
