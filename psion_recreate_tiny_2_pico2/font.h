int invert_byte(int b);
void printxy(int x, int y, int ch);
void printxy_str(int x, int y, char *str);
void printxy_hex(int x, int y, int value);

void i_printxy(int x, int y, int ch);
void i_printxy_str(int x, int y, char *str);
void i_printxy_hex(int x, int y, int value);
void next_printpos_line(void);

extern int printpos_x;
extern int printpos_y;
extern int printpos_at_end;

void flowprint(char *s);

void write_display_extra(int i, int ch);

void print_str(char *s);
void print_home(void);
void print_nl(void);
void print_nl_if_necessary(char *str);
void i2c_ssd(int cx, int cy, int ch);
void i2c_ssd_plot_point(int x, int y, int mode);
