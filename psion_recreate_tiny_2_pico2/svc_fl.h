//

typedef uint8_t FL_REC_TYPE;

typedef struct _FL_SCAN_PAK_CONTEXT
{
  PAK_ADDR save_pak_addr;
} FL_SCAN_PAK_CONTEXT;

typedef enum
  {
   FL_OP_OPEN  = 1,
   FL_OP_CLOSE = 2,
   FL_OP_FIRST = 3,
   FL_OP_NEXT  = 4,
  }
    FL_OP;

int fl_scan_pak(FL_SCAN_PAK_CONTEXT *context, FL_OP op, int device, uint8_t *dest, PAK_ADDR *recstart);

void fl_print_len_string(char *str, int len);
void fl_pos_at_end(void);

void fl_back(void);
void fl_bcat(void);
void fl_bdel(void);
void fl_bopn(void);
void fl_bsav(void);
int fl_catl(FL_OP, int device, char *filename, uint8_t *rectype);
  
void fl_copy(void);
FL_REC_TYPE fl_cret(int logfile, FL_REC_TYPE type);
void fl_deln(void);
void fl_eras(void);
void fl_ffnd(void);
int fl_find(char *srch, char *dest, int *len);
int fl_frec(int n, PAK_ADDR *pak_addr, FL_REC_TYPE *rectype, int *reclen);
void fl_next(void);
void fl_open(int logfile);
int fl_rpos(void);
void fl_pars(void);
int fl_read(uint8_t *dest);
void fl_rect(FL_REC_TYPE rect);

void fl_renm(void);
void fl_rset(int recno);
void fl_setp(int device);
void fl_size(int *bytes_free, int *num_recs, PAK_ADDR *first_free);
void fl_writ(uint8_t *src, int len);

void tl_cpyx(void);
int fl_exist(char *filename);
