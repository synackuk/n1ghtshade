#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#define RGB(r, g, b) ((r & 0xFF) << 16) + ((g & 0xFF) << 8) + (b & 0xFF)

#define COLOUR_WHITE RGB(0xFF, 0xFF, 0xFF)
#define COLOUR_BLACK RGB(0, 0, 0)

#define RGBA2RGB(x) ((((x)) & 0xFF) | ((((x) >> 8) & 0xFF) << 8) | ((((x) >> 16) & 0xFF) << 16))


int init_framebuffer();
void fb_set_loc(int x, int y);
#ifdef DISPLAY_OUTPUT
void fb_print(const char* str);
void fb_draw_image(uint32_t* image, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
#else
#define fb_print(str) 
#define fb_draw_image(image, x, y, width, height)
#endif
#endif