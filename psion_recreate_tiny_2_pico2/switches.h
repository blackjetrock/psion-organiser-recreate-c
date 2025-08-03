// Switches and debug

// Different builds
#define PSION_MINI       0   // Small hardware version
#define PSION_RECREATE   0   // Hardware replacement for original case
#define PICOCALC         1   // Use Picocalc hardware

//------------------------------------------------------------------------------
// Pico 2 with external PSRAM

#define SWITCH_EXTERNAL_PSRAM   1

#if PICOCALC
#define KEY_SCAN_IN_MAIN 1
#else
#define KEY_SCAN_IN_MAIN 0
#endif

#define DB_KB_GETK              0 
#define DB_KB_DEBOUNCE          0
#define DB_KB_MATRIX            0
#define DB_KB_TEST              0
#define DB_KB_KEYCODE_IN_BUFFER 0

#define DB_DP_VIEW       1

#define DB_MN_MENU       1

#define DB_DIGIT         0
#define DB_PK_SAVE       1
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

#define DB_FILE_LOAD     1
#define DB_FILE_EDITOR   1

#define DB_ED_EPOS       1
#define DB_NEXT_PRINTPOS 0
#define DB_PRINTPOS      0

// Key scanning in core1 main() or core0 loops?
#define CORE0_SCAN       0

// Core 1 might be unused
#if PICOCALC
#define CORE1_UNUSED     1
#else
#define CORE1_UNUSED     0
#endif

#define SPI              1
#define DD               1   // Display driver scheme
