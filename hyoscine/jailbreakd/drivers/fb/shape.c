#include <drivers/drivers.h>

void display_stroke_rect(int start_x, int start_y, int end_x, int end_y, uint32_t colour) {


	/* Draw all four edges of the rectangle */
	for(int x = start_x; x <= end_x; x += 1) {
		display_write_pixel(x, start_y, colour);
		display_write_pixel(x, end_y, colour);
	}

	for(int y = start_y; y <= end_y; y += 1) {
		display_write_pixel(start_x, y, colour);
		display_write_pixel(end_x, y, colour);
	}
}

void display_fill_rect(int start_x, int start_y, int end_x, int end_y, uint32_t colour) {

	/* Simply loop through all pixels that make up the rectangle and write the colour to them */
	for(int x = start_x; x <= end_x; x += 1) {
		for(int y = start_y; y <= end_y; y += 1) {
			display_write_pixel(x, y, colour);
		}
	}
}