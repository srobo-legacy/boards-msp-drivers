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
#include "usci.h"
#include <io.h>
#include <signal.h>
#include <sys/cdefs.h>

/* Only build USCI driver functions if a USCI device is present */
#if USCI_NDEV > 0

/* Provided by the user of this library */
extern usci_t usci_config[USCI_NDEV];

/* Registers that control the peripheral */
typedef struct {
	typeof(UCA0CTL0) *CTL0;
	typeof(UCA0CTL1) *CTL1;
	typeof(UCA0STAT) *STAT;

	typeof(UCA0BR0) *BR0;
	typeof(UCA0BR1) *BR1;
	typeof(UCA0MCTL) *MCTL;
	typeof(IE1) *IE;
	uint8_t ie_mask_tx, ie_mask_rx;
	typeof(IFG1) *IFG;
	uint8_t ifg_mask_tx, ifg_mask_rx;

	typeof(UCA0TXBUF) *TXBUF;
	typeof(UCA0RXBUF) *RXBUF;
} usci_regs_t;

static const usci_regs_t regs[USCI_NDEV] =
{
#ifdef __MSP430_HAS_USCI0__
	{
		.CTL0 = &UCA0CTL0,
		.CTL1 = &UCA0CTL1,
		.STAT = &UCA0STAT,
		.BR0 = &UCA0BR0,
		.BR1 = &UCA0BR1,
		.MCTL = &UCA0MCTL,
		.IE = &IE2,
		.ie_mask_tx = UCA0TXIE,
		.ie_mask_rx = UCA0RXIE,
		.IFG = &IFG2,
		.ifg_mask_tx = UCA0TXIFG,
		.ifg_mask_rx = UCA0RXIFG,
		.TXBUF = &UCA0TXBUF,
		.RXBUF = &UCA0RXBUF,
	},
#endif

#ifdef __MSP430_HAS_USCI1__
	{
		.CTL0 = &UCA1CTL0,
		.CTL1 = &UCA1CTL1,
		.STAT = &UCA1STAT
		.BR0 = &UCA1BR0,
		.BR1 = &UCA1BR1,
		.MCTL = &UCA1MCTL,
		.IE = &UC1IE,
		.ie_mask_tx = UCA1TXIE,
		.ie_mask_rx = UCARXIE,
		.IFG = &UC1IFG,
		.ifg_mask_tx = UCA1TXIFG,
		.ifg_mask_rx = UCA1RXIFG,
		.TXBUF = &UCA1TXBUF,
		.RXBUF = &UCA1RXBUF,
	},
#endif
};

void usci_init()
{
	uint8_t i;

	for( i=0; i<USCI_NDEV; i++ ) {
		const usci_regs_t *r = regs + i;
		const usci_t *conf = usci_config + i;

		/* Hold peripheral in reset during configuration  */
		*(r->CTL1) = UCSWRST;

		/* 8-bit data */
		*(r->CTL0) = 0;
		/* UCAxCTL0.UC7BIT = 0 : 8-bit data */
		/* UCAxCTL0.UCPEN = 0 : Parity disabled */
		/* UCAxCTL0.UCSPB = 0 : One stop bit */
		/* UCAxCTL0.MSB = 0 : LSB first */
		/* UCAxCTL0.UCSYNC = 0: UART mode */
		/* UCAxCTL0.UCMODEx = 0 : No multi-processor mode */

		/* Use SMCLK */
		*(r->CTL1) = UCSSEL_SMCLK;

		/* Configure baud rate */
		*(r->BR0) = conf->br0;
		*(r->BR1) = conf->br1;
		*(r->MCTL) = conf->mctl;

		/* These pins please */
		*(conf->sel_rx) |= (1 << conf->sel_rx_num);
		*(conf->sel_tx) |= (1 << conf->sel_tx_num);

		/* Take out of reset */
		*(r->CTL1) &= ~UCSWRST;

		/* Enable interrupts */
		*(r->IE) |= r->ie_mask_tx | r->ie_mask_rx;
	}
}

void usci_tx_start( uint8_t n )
{
	const usci_regs_t *r = regs + n;

	/* Trigger an interrupt :-P */
	*(r->IFG) |= r->ifg_mask_tx;
}

/* Generalised interrupt handler  */
static void usci_isr_tx( uint8_t n )
{
	uint8_t b;
	const usci_regs_t *r = regs + n;
	const usci_t *conf = usci_config + n;

	if( conf->tx_gen_byte == NULL 
	    || !conf->tx_gen_byte(&b) ) {
		/* Nothing to transmit, clear flag */
		*(r->IFG) &= ~(r->ifg_mask_tx);
		return;
	}

	*(r->TXBUF) = b;
}

static void usci_isr_rx( uint8_t n )
{
	const usci_regs_t *r = regs + n;
	const usci_t *conf = usci_config + n;

	if( conf->rx_byte != NULL )
		conf->rx_byte( *(r->RXBUF) );
}


#define USCI_TX_ISR(n, VECTOR) interrupt (VECTOR) usci ## n ## _isr_tx( void ) \
	{ \
		usci_isr_tx(n);		\
	}

#define USCI_RX_ISR(n, VECTOR) interrupt (VECTOR) usci ## n ## _isr_rx( void ) \
	{								\
		usci_isr_rx(n);					\
	}

#ifdef __MSP430_HAS_USCI0__
USCI_TX_ISR(0, USCIAB0TX_VECTOR);
USCI_RX_ISR(0, USCIAB0RX_VECTOR);
#endif

#ifdef __MSP430_HAS_USCI1__
USCI_TX_ISR(1, USCIAB1TX_VECTOR);
USCI_RX_ISR(1, USCIAB1RX_VECTOR);
#endif

void usci_rx_gate( uint8_t n, bool en )
{
	const usci_t *conf = usci_config + n;

	if(en)
		*(conf->sel_rx) |= (1 << conf->sel_rx_num);
	else
		*(conf->sel_rx) &= ~(1 << conf->sel_rx_num);
}

#endif /* USCI_NDEV > 0 */
