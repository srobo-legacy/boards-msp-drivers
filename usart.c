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
#include "usart.h"
#include <io.h>
#include <signal.h>
#include <sys/cdefs.h>

/* Provided by the user of this library */
extern usart_t usart_config[USART_NDEV];

/* Registers that control the peripheral */
typedef struct {
	typeof(U0CTL) *CTL;
	typeof(U0TCTL) *TCTL;

	typeof(U0BR0) *BR0;
	typeof(U0BR1) *BR1;
	typeof(U0MCTL) *MCTL;
	typeof(U0RCTL) *RCTL;
	typeof(IE1) *IE;
	uint8_t ie_mask_tx, ie_mask_rx;
	typeof(IFG1) *IFG;
	uint8_t ifg_mask_tx, ifg_mask_rx;

	typeof(ME1) *ME;
	uint8_t me_mask_tx, me_mask_rx;

	typeof(U0TXBUF) *TXBUF;
	typeof(U0RXBUF) *RXBUF;
} usart_regs_t;

static const usart_regs_t regs[USART_NDEV] =
{
#ifdef __MSP430_HAS_UART0__
	{
		.CTL = &U0CTL,
		.TCTL = &U0TCTL,
		.BR0 = &U0BR0,
		.BR1 = &U0BR1,
		.MCTL = &U0MCTL,
		.RCTL = &U0RCTL,
		.IE = &IE1,
		.ie_mask_tx = UTXIE0,
		.ie_mask_rx = URXIE0,
		.IFG = &IFG1,
		.ifg_mask_tx = UTXIFG0,
		.ifg_mask_rx = URXIFG0,
		.ME = &ME1,
		.me_mask_tx = UTXE0,
		.me_mask_rx = URXE0,
		.TXBUF = &U0TXBUF,
		.RXBUF = &U0RXBUF,
	},
#endif

#ifdef __MSP430_HAS_UART1__
	{
		.CTL = &U1CTL,
		.TCTL = &U1TCTL,
		.BR0 = &U1BR0,
		.BR1 = &U1BR1,
		.MCTL = &U1MCTL,
		.RCTL = &U1RCTL,
		.IE = &IE2,
		.ie_mask_tx = UTXIE1,
		.ie_mask_rx = URXIE1,
		.IFG = &IFG2,
		.ifg_mask_tx = UTXIFG1,
		.ifg_mask_rx = URXIFG1,
		.ME = &ME2,
		.me_mask_tx = UTXE1,
		.me_mask_rx = URXE1,
		.TXBUF = &U1TXBUF,
		.RXBUF = &U1RXBUF,
	},
#endif
};

void usart_init()
{
	uint8_t i;

	for( i=0; i<USART_NDEV; i++ ) {
		const usart_regs_t *r = regs + i;
		const usart_t *conf = usart_config + i;

		/* Hold peripheral in reset during configuration  */
		*(r->CTL) = SWRST;

		/* 8-bit data */
		*(r->CTL) |= CHAR;
		/* U1CTL.PENA = 0 : Parity disabled */
		/* U1CTL.SPB = 0 : One stop bit */
		/* U1CTL.LISTEN = 0 : Loopback disabled */
		/* U1CTL.SYNC = 0: UART mode */
		/* U1CTL.MM = 0 : Idle-line multiprocessor mode */

		/* Use SMCLK */
		*(r->TCTL) = SSEL_SMCLK;

		/* URXWIE = 0 : All received bytes trigger interrupt */
		*(r->RCTL) = 0;

		/* Configure baud rate */
		*(r->BR0) = conf->br0;
		*(r->BR1) = conf->br1;
		*(r->MCTL) = conf->mctl;

		/* Enable transmit and receive */
		*(r->ME) |= r->me_mask_tx | r->me_mask_rx;

		/* These pins please */
		*(conf->sel_rx) |= (1 << conf->sel_rx_num);
		*(conf->sel_tx) |= (1 << conf->sel_tx_num);

		/* Take out of reset */
		*(r->CTL) &= ~SWRST;

		/* Enable interrupts */
		*(r->IE) |= r->me_mask_tx | r->me_mask_rx;
	}
}

void usart_tx_start( uint8_t n )
{
	const usart_regs_t *r = regs + n;

	/* Enable the transmitter */
	*(r->ME) |= r->me_mask_tx;

	/* Trigger an interrupt :-P */
	*(r->IFG) |= r->ifg_mask_tx;
}

/* Generalised interrupt handler  */
static void usart_isr_tx( uint8_t n )
{
	uint8_t b;
	const usart_regs_t *r = regs + n;
	const usart_t *conf = usart_config + n;

	/* Ignore if the transmitter isn't enabled */
	if( !( *(r->ME) & r->me_mask_tx ) )
		return;

	if( conf->tx_gen_byte == NULL 
	    || !conf->tx_gen_byte(&b) )
		/* Nothing to transmit -- disable the transmitter */
		return;

	*(r->TXBUF) = b;
}

static void usart_isr_rx( uint8_t n )
{
	const usart_regs_t *r = regs + n;
	const usart_t *conf = usart_config + n;

	if( conf->rx_byte != NULL )
		conf->rx_byte( *(r->RXBUF) );
}


#define USART_TX_ISR(n, VECTOR) interrupt (VECTOR) usart ## n ## _isr_tx( void ) \
	{ \
		usart_isr_tx(n);		\
	}

#define USART_RX_ISR(n, VECTOR) interrupt (VECTOR) usart ## n ## _isr_rx( void ) \
	{								\
		usart_isr_rx(n);					\
	}

#ifdef __MSP430_HAS_UART0__
USART_TX_ISR(0, USART0TX_VECTOR);
USART_RX_ISR(0, USART0RX_VECTOR);
#endif

#ifdef __MSP430_HAS_UART1__
USART_TX_ISR(1, USART1TX_VECTOR);
USART_RX_ISR(1, USART1RX_VECTOR);
#endif

void usart_rx_gate( uint8_t n, bool en )
{
	const usart_regs_t *r = regs + n;

	if(en)
		*(r->ME) |= r->me_mask_rx;
	else
		*(r->ME) &= ~(r->me_mask_rx);
}
