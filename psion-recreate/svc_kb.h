typedef uint64_t MATRIX_MAP;

void matrix_scan(void);

#define NOS_KEY_BUFFER_LEN 16

extern volatile int nos_key_in;
extern volatile int nos_key_out;

void nos_put_key(char key);
char nos_get_key(void);
void check_keys(void);


KEYCODE kb_getk(void);
KEYCODE kb_test(void);

extern KEYCODE kb_external_key;
void queue_hid_key(int k);
int unqueue_hid_key(void);
int translate_to_hid(char ch);
