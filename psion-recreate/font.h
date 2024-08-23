int invert_byte(int b);
void display_clear(void);
void printxy(int x, int y, int ch);
void printxy_str(int x, int y, char *str);
void printxy_hex(int x, int y, int value);

void i_printxy(int x, int y, int ch);
void i_printxy_str(int x, int y, char *str);
void i_printxy_hex(int x, int y, int value);

extern int printpos_x;
extern int printpos_y;
void flowprint(char *s);

void write_display_extra(int i, int ch);

void print_str(char *s);
void print_home(void);
void print_nl(void);
void print_nl_if_necessary(char *str);
