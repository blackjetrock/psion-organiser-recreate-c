////////////////////////////////////////////////////////////////////////////////
//
// Pack services
//

typedef enum _PAK
  {
   PAKNONE = -1,
   PAKA = 1,
   PAKB = 2,
  } PAK;



void pk_setp(PAK pak);
void pk_save(void);
void pk_read(void);
void pk_rbyt(void);
void pk_rwrd(void);
void pk_skip(void);
void pk_qadd(void);
void pk_sadd(void);
void pk_pkof(void);
