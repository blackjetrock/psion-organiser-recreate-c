////////////////////////////////////////////////////////////////////////////////
//
// Pack services
//

// Read and write byte driver function pointers.
// Each different type of hardware has one of these sets of drivers.
typedef uint8_t (*PK_RBYT_FNPTR)(PAK_ADDR pak_addr);
typedef void    (*PK_SAVE_FNPTR)(PAK_ADDR pak_addr, int len, uint8_t *src);
typedef void    (*PK_FMAT_FNPTR)(void); 

void pk_build_id_string(uint8_t *id,
			int size_in_bytes,
			int year,
			int month,
			int day,
			int hour,
			int32_t unique);

void      pk_setp(PAK pak);
void      pk_read(int len, uint8_t *dest);
uint8_t   pk_rbyt(void);
uint16_t  pk_rwrd(void);
void      pk_skip(int len);
int       pk_qadd(void);
void      pk_sadd(int addr);
void      pk_pkof(void);
void      pk_fmat(void);
void      pk_save(int len, uint8_t *src);

// The table of device ID to driver functions

typedef struct _PK_DRIVER_SET
{
  PK_RBYT_FNPTR rbyt;
  PK_SAVE_FNPTR save;
  PK_FMAT_FNPTR format;
} PK_DRIVER_SET;

extern PAK      pkb_curp;
extern PAK_ADDR pkw_cpad;

