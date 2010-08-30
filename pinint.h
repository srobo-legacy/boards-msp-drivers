/*   Copyright (C) 2010 Richard Barlow

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __PININT_H
#define __PININT_H
#include <io.h>
#include <stdint.h>

#define PININT_NCONF 4

typedef struct {
	uint16_t mask;
	void (*int_cb) (uint16_t flags);
} pinint_conf_t;

/* Each module that requires access to pin interrupts should set-up
 * the mask and callback for its specific inputs in the following array */
extern pinint_conf_t pinint_conf[PININT_NCONF];

#endif /* __PININT_H */
