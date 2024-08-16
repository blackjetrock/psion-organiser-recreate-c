#define RECORD_LENGTH 64
#define NUM_RECORDS 100
#define NO_RECORD -1

typedef struct _RECORD
{
  uint8_t flag;
  char key[16];
  char data[RECORD_LENGTH-1-16];
} RECORD;
