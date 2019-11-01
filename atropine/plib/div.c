/*
 *  ibex - pseudo-library
 *
 *  Division algorithm from Wikipedia
 */


#include <includes/plib.h>


unsigned
div(unsigned N, unsigned D)
{
    int i;
    unsigned Q = 0;
    unsigned R = 0;
    if (D == 0) {
        return -1; /* DivisionByZeroException */
    }
    for (i = sizeof(N) * 8 - 1; i >= 0; i--) {
        R <<= 1;
        R = (R & ~1) | ((N >> i) & 1);
        if (R >= D) {
            R -= D;
            Q |= (1 << i);
        }
    }
    return Q;
}
