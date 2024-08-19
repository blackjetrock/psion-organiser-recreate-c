////////////////////////////////////////////////////////////////////////////////
//
// Pack services
//

// Read and write byte driver function pointers.
// Each different type of hardware has one of these sets of drivers.
typedef uint8_t (*PK_RBYT_FNPTR)(PAK_ADDR pak_addr);
typedef void    (*PK_SAVE_FNPTR)(PAK_ADDR pak_addr, int len, uint8_t *src);
typedef void    (*PK_FMAT_FNPTR)(void); 

void      pk_setp(PAK pak);
void      pk_read(PAK_ADDR pak_addr, int len, uint8_t *dest);
uint8_t   pk_rbyt(PAK_ADDR pak_addr);
uint16_t  pk_rwrd(PAK_ADDR pak_addr);
void      pk_skip(int len);
int       pk_qadd(void);
void      pk_sadd(int addr);
void      pk_pkof(void);


// The table of device ID to driver functions

typedef struct _PK_DRIVER_SET
{
  PK_RBYT_FNPTR rbyt;
  PK_SAVE_FNPTR save;
  PK_FMAT_FNPTR format;
} PK_DRIVER_SET;


