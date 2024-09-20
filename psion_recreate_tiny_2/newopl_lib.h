void read_proc_file(FILE *fp, NOBJ_PROC *p);
void read_ob3_header(FILE *fp);

char *decode_vartype(NOBJ_VARTYPE t);
void init_machine(NOBJ_MACHINE *m);
void push_proc_on_stack(NOBJ_PROC *p, NOBJ_MACHINE *machine);
void push_machine_8(NOBJ_MACHINE *m, uint8_t v);

uint16_t pop_discard_sp_int(NOBJ_MACHINE *m, uint16_t sp);
uint16_t pop_discard_sp_float(NOBJ_MACHINE *m, uint16_t sp);
uint16_t pop_discard_sp_str(NOBJ_MACHINE *m, uint16_t sp);

