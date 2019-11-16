#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#define RGB(r, g, b) ((r & 0xFF) << 16) + ((g & 0xFF) << 8) + (b & 0xFF)

#define COLOUR_WHITE RGB(0xFF, 0xFF, 0xFF)
#define COLOUR_BLACK RGB(0, 0, 0)

int init_framebuffer();
void fb_set_loc(int x, int y);
#ifdef DISPLAY_OUTPUT
void fb_print(const char* str);
#else
#define fb_print(str) 
#endif
#endif