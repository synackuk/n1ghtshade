#include <display.h>

void draw_image(uint32_t* img, int x, int y, int width, int height) {

	/* Iterate the width and height of the bitmap */
	for(int img_y = 0; img_y < height; img_y += 1) {
		for(int img_x = 0; img_x < width; img_x += 1) {

			/* Find a given coordinate in the image on the framebuffer and set the framebuffer pixel to the correct value */
			uint32_t img_value = img[((img_y) * width) + (img_x)];
			display_write_pixel(x + img_x, y + img_y, img_value);
		}
	}
}