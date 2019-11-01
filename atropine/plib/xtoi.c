/*
 *  ibex - pseudo-library
 *
 *  Copyright (c) 2015 xerub
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <includes/plib.h>


int
xtoi(const char *hptr)
{
    unsigned int val = 0;
    if (hptr[0] == '0' && hptr[1] == 'x') {
        hptr += 2;
    }
    while (1) {
        int nibble = *hptr++;
        if (nibble >= '0' && nibble <= '9') {
            nibble -= '0';
        } else {
            nibble |= 0x20;
            if (nibble >= 'a' && nibble <= 'f') {
                nibble -= 'a' - 10;
            } else {
                break;
            }
        }
        val = (val << 4) | nibble;
    }
    return val;
}
