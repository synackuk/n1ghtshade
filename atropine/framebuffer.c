#include <common.h>
#include <fonts/9x15.h>

int fb_x = 0;
int fb_y = 0;

int init_framebuffer() {
	fb_print("Pwned by SynAck ;)\n");
	debug("initialised framebuffer.\n");
	return 0;
}

int font_get_pixel(int ch, int x, int y) {
	register int bitIndex = ((font_width * font_height) * ch) + (font_width * y) + x;
	return (font_data[bitIndex / 8] >> (bitIndex % 8)) & 0x1;
}

volatile unsigned int* fb_get_pixel(register unsigned int x, register unsigned int y) {
	return (((unsigned int*)framebuffer_address) + (y * display_width) + x);
}

static void fb_scrollup() {
	register volatile unsigned int* newFirstLine = fb_get_pixel(0, font_height);
	register volatile unsigned int* oldFirstLine = fb_get_pixel(0, 0);
	register volatile unsigned int* end = oldFirstLine + (display_width * display_height);
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
		register unsigned int sx;
		register unsigned int sy;

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
	unsigned int len = strlen(str);
	int i;
	for(i = 0; i < len; i++) {
		fb_putc(str[i]);
	}
}