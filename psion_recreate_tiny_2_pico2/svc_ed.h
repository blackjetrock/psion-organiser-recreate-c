#define ED_NUM_LINES  8
#define ED_NUM_CHARS  64
#define ED_VIEW_N     4

extern char ed_edit_buffer[ED_NUM_LINES][ED_NUM_CHARS];

KEYCODE ed_view(char *str, int ln);

#define ED_SINGLE_LINE     1
#define ED_MULTI_LINE      0
KEYCODE ed_epos(char *str, int len, int single_nmulti_line, int exit_on_mode, int cursor_line);
void display_epos(char *str_in, char *epos_prompt, int insert_point, int cursor_line, int display_start_index, int single_nmulti_line);
