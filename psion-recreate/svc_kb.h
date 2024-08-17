typedef uint64_t MATRIX_MAP;

void matrix_scan(void);

#define NOS_KEY_BUFFER_LEN 16
#define NOS_KEY_NONE -1

extern volatile int nos_key_in;
extern volatile int nos_key_out;

void nos_put_key(char key);
char nos_get_key(void);
void check_keys(void);

#define KEY_ON  1

