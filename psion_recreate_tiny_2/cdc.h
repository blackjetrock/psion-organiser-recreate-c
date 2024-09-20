#define CDC_MAX_LINE 200

#define ITF_CLI  1
#define ITF_INFO 2
#define ITF_3    3
#define ITF_4    4
#define ITF_5    5
#define ITF_6    6

void cdc_printf(int itf, const char *format, ...);
void cdc_printf_xy(int itf, int x, int y, const char *format, ...);
void cdc_cls(int itf);
