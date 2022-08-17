#ifndef DIMENSIONS_H
#define DIMENSIONS_H

#define PADDING 10


#define CENTRE(x, xx) ((((x) - (xx))/2))

#define CENTRE_SHIFT(s, x, xx) ((s) + CENTRE(x, xx))

#define PAD_W(x) (WINDOW_WIDTH - ((x) + PADDING))
#define PAD_H(x) (WINDOW_HEIGHT - ((x) + PADDING))

#define WINDOW_WIDTH 450
#define WINDOW_HEIGHT 580

#define LOGO_WIDTH 64
#define LOGO_HEIGHT 64

#define TEXT_WIDTH WINDOW_WIDTH
#define TEXT_HEIGHT 18
#define BOLD_TEXT_HEIGHT TEXT_HEIGHT

#define BUTTON_WIDTH 160
#define BUTTON_HEIGHT 28


#define BACK_BUTTON_X PAD_W(BUTTON_WIDTH)
#define BACK_BUTTON_Y PADDING

#define PROGRESS_BAR_HEIGHT 20

#endif