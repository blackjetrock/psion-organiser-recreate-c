typedef uint64_t MATRIX_MAP;

//typedef int KEYCODE;
//#define KEY_NONE -1

typedef enum _KEYCODE
  {
   KEY_ON        = 1,
   KEY_MODE      = 2,
   KEY_UP        = 3,
   KEY_DOWN      = 4,
   KEY_LEFT      = 5,
   KEY_RIGHT     = 6,
   KEY_SHIFT_DEL = 7,
   KEY_DEL       = 8,
   KEY_TAB       = 9,
   KEY_EXE       = 13,
   NOS_KEY_NONE  = -1,
   KEY_NONE      = -1,
  } KEYCODE;


void matrix_scan(void);

#define NOS_KEY_BUFFER_LEN 16

extern volatile int nos_key_in;
extern volatile int nos_key_out;

void nos_put_key(char key);
char nos_get_key(void);
void check_keys(void);
void kb_set_caps(int on_noff);
void kb_set_num(int on_noff);

KEYCODE kb_getk(void);
KEYCODE kb_test(void);

extern KEYCODE kb_external_key;
void queue_hid_key(int k);
int unqueue_hid_key(void);
int translate_to_hid(char ch);

