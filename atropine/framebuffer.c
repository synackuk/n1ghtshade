#include <common.h>
#include <fonts/9x15.h>

int fb_x = 0;
int fb_y = 0;

int init_framebuffer() {
	for(int i = 0; i < (display_width / font_width); i++) {
		fb_print("=");
	}
	fb_print("\n");
	fb_print("n1ghtshade.\n");
	fb_print("By synackuk ;)\n");
	fb_print("\n");
	for(int i = 0; i < (display_width / font_width); i++) {
		fb_print("=");
	}
	fb_print("\n");

	debug("initialised framebuffer.\n");
	return 0;
}

void fb_set_loc(int x, int y) {
	fb_x = x;
	fb_y = y;
}

int font_get_pixel(int ch, int x, int y) {
	register int bitIndex = ((font_width * font_height) * ch) + (font_width * y) + x;
	return (font_data[bitIndex / 8] >> (bitIndex % 8)) & 0x1;
}

volatile uint32_t* fb_get_pixel(register uint32_t x, register uint32_t y) {
	return (((uint32_t*)framebuffer_address) + (y * display_width) + x);
}

static void fb_scrollup() {
	register volatile uint32_t* newFirstLine = fb_get_pixel(0, font_height);
	register volatile uint32_t* oldFirstLine = fb_get_pixel(0, 0);
	register volatile uint32_t* end = oldFirstLine + (display_width * display_height);
	while(newFirstLine < end) {
		*(oldFirstLine++) = *(newFirstLine++);
	}
	while(oldFirstLine < end) {
		*(oldFirstLine++) = COLOUR_BLACK;
	}
	fb_y--;
}

void fb_putc(int c) {
	if(!framebuffer_address){
		return;
	}
	if(c == '\r') {
		fb_x = 0;

	} else if(c == '\n') {
		fb_x = 0;
		fb_y++;

	} else {
		register uint32_t sx;
		register uint32_t sy;

		for(sy = 0; sy < font_height; sy++) {
			for(sx = 0; sx < font_width; sx++) {
				if(font_get_pixel(c, sx, sy)) {
					*(fb_get_pixel(sx + (font_width * fb_x), sy + (font_height * fb_y))) = COLOUR_WHITE;
				} else {
					*(fb_get_pixel(sx + (font_width * fb_x), sy + (font_height * fb_y))) = COLOUR_BLACK;
				}
			}
		}

		fb_x++;
	}

	if(fb_x == (display_width / font_width)) {
		fb_x = 0;
		fb_y++;
	}

	if(fb_y == (display_height / font_height)) {
		fb_scrollup();
	}
}

void fb_print(const char* str) {
	uint32_t len = strlen(str);
	int i;
	for(i = 0; i < len; i++) {
		fb_putc(str[i]);
	}
}

void fb_draw_image(uint32_t* image, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	register uint32_t sx;
	register uint32_t sy;
	for(sy = 0; sy < height; sy++) {
		for(sx = 0; sx < width; sx++) {
			*(fb_get_pixel(sx + x, sy + y)) = RGBA2RGB(image[(sy * width) + sx]);
		}
	}
}


