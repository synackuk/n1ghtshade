#include <libc.h>

putchar_t _putchar;

void set_putchar(putchar_t new_putchar) {
	_putchar = new_putchar;
}