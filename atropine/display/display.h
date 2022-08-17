#ifndef DISPLAY_H
#define DISPLAY_H

#include <libc/libc.h>

#define RGB(r, g, b) (uint32_t)((((r) & 0xFF) << 16) + (((g) & 0xFF) << 8) + ((b) & 0xFF))

typedef struct {
	uint32_t* framebuffer;
	uint32_t width;
	uint32_t height;
	int sf;
	uint32_t background_colour;
	uint32_t foreground_colour;
} display_info_t;


/* display.c */
extern display_info_t display_info;
void display_init(uint32_t* framebuffer, uint32_t width, uint32_t height);
uint32_t* display_coords_to_address(int x, int y);
void display_write_pixel(int x, int y, uint32_t colour);
void display_clear();
void display_invert();
void display_set_sf(int sf);
void display_progress_print(char* str);
void display_debug_print_char(char c);
void display_debug_print(char* str);
void display_logo();
void display_progress_bar(int progress);


/* text.c */

#define FONT_WIDTH (8 * display_info.sf)
#define FONT_HEIGHT (8 * display_info.sf)

void display_write_char(int chr, int x, int y);
void display_write(char* s, int x, int y);
void display_write_debug_char(char chr);
void display_write_debug(char* s);

/* image.c */
void draw_image(uint32_t* img, int x, int y, int width, int height);

/* shape.c */
void display_stroke_rect(int start_x, int start_y, int end_x, int end_y, uint32_t colour);
void display_fill_rect(int start_x, int start_y, int end_x, int end_y, uint32_t colour);

#endif