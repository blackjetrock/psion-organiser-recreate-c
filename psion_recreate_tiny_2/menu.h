////////////////////////////////////////////////////////////////////////////////
//
// menu definitions
//
//
////////////////////////////////////////////////////////////////////////////////


extern int scan_keys_off;

#define NUM_LAST 5


#define SCAN_STATE_DRIVE  0
#define SCAN_STATE_DRIVE1 1
#define SCAN_STATE_DRIVE2 2
#define SCAN_STATE_DRIVE3 3
#define SCAN_STATE_DRIVE4 4
#define SCAN_STATE_DRIVE5 5
#define SCAN_STATE_DRIVE6 6
#define SCAN_STATE_DRIVE7 7
#define SCAN_STATE_DRIVE8 8

#define SCAN_STATE_READ  10
#define SCAN_STATE_RELEASE_DRIVE  11
#define SCAN_STATE_RELEASE_READ   12

typedef void (*MENU_FN)(void);

typedef struct _MENU_ITEM
{
  char        key;
  char        *item_text;
  MENU_FN     do_fn;
} MENU_ITEM;

typedef struct _MENU
{
  struct _MENU   *last;         // Menu that started this one
  char           *name;
  MENU_FN        init_fn;
  MENU_ITEM      item[];
} MENU;

extern int shift_edge_counter;
extern int last_shift[NUM_LAST];
extern int shift;
extern volatile char keychar;      // What the key code is
extern volatile int gotkey;        // We have a key

extern MENU menu_top;
extern MENU menu_eeprom;
extern MENU menu_rtc;
extern MENU menu_test_os;
extern MENU menu_buzzer;
extern MENU menu_format;

void check_menu_launch(void);
void scan_keys(void);
void menu_enter(void);
void menu_process(void);
void menu_leave(void);
void menu_loop(void);
void menu_loop_tasks(void);

extern volatile int menu_done;
