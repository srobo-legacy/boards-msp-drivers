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

#include <io.h>
#include <signal.h>
#include <stddef.h>
#include "pinint.h"

static pinint_conf_t *pinint[PININT_NCONF];

#define foreach_int(p) for( p=pinint; (p-pinint) < PININT_NCONF; p++ )

static void pinint_isr(void) {
	pinint_conf_t **p;
	uint16_t flags = ((uint16_t)P2IFG) << 8 | P1IFG;
	/* Clear flags */
	P1IFG = P2IFG = 0;

	foreach_int(p) {
		if( *p == NULL )
			continue;

		/* Check flags against mask */
		if ( (*p)->mask & flags )
			(*p)->int_cb(flags);
	}
}

interrupt (PORT1_VECTOR) p1_isr(void) {
	pinint_isr();
}

interrupt (PORT2_VECTOR) p2_isr(void) {
	pinint_isr();
}

void pinint_init( void )
{
	uint8_t i;
	for( i=0; i<PININT_NCONF; i++ )
		pinint[i] = NULL;
}

void pinint_add( pinint_conf_t* conf )
{
	pinint_conf_t **p;

	foreach_int(p) {
		if( (*p) == NULL ) {
			*p = conf;
			return;
		}
	}

	/* Too many pin interrupts registered -- increase PININT_NCONF */
	while(1);
}
