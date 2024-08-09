////////////////////////////////////////////////////////////////////////////////
//
// menu definitions
//
//
////////////////////////////////////////////////////////////////////////////////


#define NUM_LAST 5

#define NUM_SCAN_STATES  2
#define SCAN_STATE_DRIVE 0
#define SCAN_STATE_READ  (NUM_SCAN_STATES/2)

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
extern char keychar;      // What the key code is
extern int gotkey;        // We have a key

extern MENU menu_top;
extern MENU menu_eeprom;

void check_menu_launch(void);
void scan_keys(void);
void menu_enter(void);
void menu_process(void);
void menu_leave(void);
void menu_loop(void);

extern volatile int menu_done;
