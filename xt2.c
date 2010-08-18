/*   Copyright (C) 2010 Robert Spanton

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */
#include "xt2.h"
#include <io.h>

#define field_set( x, val, mask ) do { x = (x & ~mask) | val; } while(0)

/* Kick the XT2 crystal until it starts oscillating */
void xt2_start( void )
{
	do {
		uint8_t i;

		/* Switch the XT2 osc on */
		BCSCTL1 &= ~XT2OFF;
		field_set( BCSCTL2, SELM_XT2CLK, SELM_3 );

		/* Clear flag to allow oscillator test */
		IFG1 &= ~OFIFG;

		/* Wait at least 50µs (currently on ~800kHz DCO) */
		for( i=0; i<40; i++ )
			nop();
	} while( IFG1 & OFIFG );
}
