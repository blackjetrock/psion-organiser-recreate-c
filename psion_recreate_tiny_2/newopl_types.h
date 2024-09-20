////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////

// The Psion data types
typedef int16_t NOPL_INT;
typedef struct _NOPL_FLOAT
{
  uint64_t m;
  int8_t   e;
  uint8_t  s;
} NOPL_FLOAT;

NOPL_INT   psion_int(uint8_t *p);
NOPL_FLOAT psion_float(uint8_t *p);
double nopl_float_to_double(NOPL_FLOAT *nf);
char *nopl_float_str(NOPL_FLOAT *nf);

#define DB_PSION_FLOAT 1
