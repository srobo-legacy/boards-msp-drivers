#ifndef __USART_H
#define __USART_H
#include <io.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef __MSP430_HAS_UART0__
#warning "This target has no USART devices -- this almost certainly isn't what you wanted."
#define USART_NDEV 0
#else
#ifdef __MSP430_HAS_UART1__
#define USART_NDEV 2
#else
#define USART_NDEV 1
#endif
#endif

/* The user of this library must link in some code that provides an
   array of the following struct with USART_NDEV entries.
   That array must be called usart_config. */
typedef struct {
	/* Callback to be called to retrieve the next byte for transmission.
	   Must return false if there is no next byte (in which case transmission
	   should be resumed using usart_tx_start). */
	bool (*tx_gen_byte) ( uint8_t *b );

	/* Callback to be presented with bytes as they're received */
	void (*rx_byte) ( uint8_t b );

	/* Baud rate config */
	uint8_t br0, br1, mctl;

	/* The PxSEL registers for the rx and tx pins */
	typeof(P1SEL) *sel_rx, *sel_tx;
	/* The bit _numbers_ of those pins in the PxSEL register */
	uint8_t sel_rx_num, sel_tx_num;
} usart_t;

void usart_init();

/* Trigger transmission.
   Bytes to be transmitted will be picked up through the next-byte callback. */
void usart_tx_start( uint8_t n );

/* Enable/disable the receiver -- pass true for enabled */
void usart_rx_gate( uint8_t n, bool en );

#endif	/* __USART_H */
