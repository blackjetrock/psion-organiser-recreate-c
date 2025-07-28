
#define DP_MAX_STR     64

#define DP_NUM_LINE     4
#define DP_NUM_CHARS   21
#define DP_MAX_LINE     3
#define DP_MAX_CHARS   20

#define DP_STAT_CURSOR_ON    1
#define DP_STAT_CURSOR_OFF   0
#define DP_STAT_BLOCK_CURSOR 1
#define DP_STAT_LINE_CURSOR  0

void dp_stat(int x, int y, int cursor_on, int cursor_block);
KEYCODE dp_view(char *str, int line);
void dp_clr_eol(void);
void dp_clr_eos(void);
void dp_cls(void);
void dp_prnt(char *s);
void dp_emit(int ch);
void dp_newline(void);
