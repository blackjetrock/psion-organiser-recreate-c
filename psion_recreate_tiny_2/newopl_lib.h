
// FP offsets
#define FP_OFF_GLOB_START -2
#define FP_OFF_NEXT_FP     0
#define FP_OFF_BASE_SP     2
#define FP_OFF_ONERR       4
#define FP_OFF_RETURN_PC   6
#define FP_OFF_DEVICE      8

void read_proc_file(FILE *fp, NOBJ_PROC *p);
void read_ob3_header(FILE *fp);
int read_item(FILE *fp, void *ptr, int n, size_t size);
int read_item_16(FILE *fp, uint16_t *ptr);

char *decode_vartype(NOBJ_VARTYPE t);
void init_machine(NOBJ_MACHINE *m);
void push_proc_on_stack(NOBJ_PROC *p, NOBJ_MACHINE *machine);
void error(char *fmt, ...);

int datatype_length(int type, int next_byte);

void debug(char *fmt, ...);
uint16_t stack_entry_16(NOBJ_MACHINE *m, uint16_t ptr);
uint8_t stack_entry_8(NOBJ_MACHINE *m, uint16_t ptr);
void init_sp(NOBJ_MACHINE *m, NOBJ_SP sp );
