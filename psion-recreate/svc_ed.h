#define ED_NUM_LINES  8
#define ED_NUM_CHARS  64
#define ED_VIEW_N     4

char ed_edit_buffer[ED_NUM_LINES][ED_NUM_CHARS];

KEYCODE ed_view(char *str);
KEYCODE ed_epos(char *str, int len, int single_nmulti_line, int exit_on_mode);
