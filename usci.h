#ifndef __USCI_H
#define __USCI_H
#include <io.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef __MSP430_HAS_USCI0__
#warning "This target has no USCI devices -- not including USCI driver."
#define USCI_NDEV 0
#else
#ifdef __MSP430_HAS_UART1__
#define USCI_NDEV 2
#else
#define USCI_NDEV 1
#endif
#endif

/* Only include USCI driver functions if a USCI device is present */
#if USCI_NDEV > 0

/* The user of this library must link in some code that provides an
   array of the following struct with USCI_NDEV entries.
   That array must be called usci_config. */
typedef struct {
	/* Callback to be called to retrieve the next byte for transmission.
	   Must return false if there is no next byte (in which case transmission
	   should be resumed using usci_tx_start). */
	bool (*tx_gen_byte) ( uint8_t *b );

	/* Callback to be presented with bytes as they're received */
	void (*rx_byte) ( uint8_t b );

	/* Baud rate config */
	uint8_t br0, br1, mctl;

	/* The PxSEL registers for the rx and tx pins */
	typeof(P1SEL) *sel_rx, *sel_tx;
	/* The bit _numbers_ of those pins in the PxSEL register */
	uint8_t sel_rx_num, sel_tx_num;
} usci_t;

void usci_init();

/* Trigger transmission.
   Bytes to be transmitted will be picked up through the next-byte callback. */
void usci_tx_start( uint8_t n );

/* Enable/disable the receiver -- pass true for enabled */
void usci_rx_gate( uint8_t n, bool en );

#endif /* USCI_NDEV > 0 */

#endif	/* __USCI_H */
