#define PIXELS_MAX_X   128
#define PIXELS_MAX_Y    32

#define PIXEL_BUFFER_SIZE_BYTES (PIXELS_MAX_X/8*PIXELS_MAX_Y)

void plot_point(int x, int y, int mode);
void pixels_clear(void);

