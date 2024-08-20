//

typedef uint8_t FL_REC_TYPE;

void fl_pos_at_end(void);

void fl_back(void);
void fl_bcat(void);
void fl_bdel(void);
void fl_bopn(void);
void fl_bsav(void);
int fl_catl(int first, int device, char *filename, uint8_t *rectype);
  
void fl_copy(void);
FL_REC_TYPE fl_cret(char *filename);
void fl_deln(void);
void fl_eras(void);
void fl_ffnd(void);
void fl_find(void);
void fl_frec(void);
void fl_next(void);
void fl_open(void);
void fl_pars(void);
void fl_read(void);
void fl_rect(FL_REC_TYPE rect);

void fl_renm(void);
void fl_rset(void);
void fl_setp(int device);

void fl_size(void);
void fl_writ(uint8_t *src, int len);

void tl_cpyx(void);
