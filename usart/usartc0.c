/*
 * usartc0.c
 *
 *  Created on: 18 gru 2013
 *      Author: krzysztof
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usartc0.h"
#include "xmega_baud.h"
#include "../pcb.h"
volatile uint8_t *timerc0;

char rx_buffer_usartc0[RX_BUFFER_SIZE_USARTC0];

// This flag is set on USARTC0 Receiver buffer overflow
char rx_buffer_overflow_usartc0 = 0;

char tx_buffer_usartc0[TX_BUFFER_SIZE_USARTC0];

volatile unsigned char rx_wr_index_usartc0 = 0, rx_rd_index_usartc0 = 0,	rx_counter_usartc0 = 0;
volatile unsigned char tx_wr_index_usartc0 = 0, tx_rd_index_usartc0 = 0,	tx_counter_usartc0 = 0;

// USARTC0 initialization
void usartc0_init() {

// Note: the correct PORTC direction for the RxD, TxD and XCK signals
// is configured in the ports_init function

// Transmitter is enabled
// Set TxD=1
	PORTC.OUTSET = 0x08;
	PORTC.DIRSET = 0X08;

#ifdef UARTC0_DE_PORT
	// inicjalizujemy liniê steruj¹c¹ nadajnikiem
	UARTC0_DE_DIR |= UART_DE_BIT;
	UARTC0_DE_ODBIERANIE;
#endif

// Communication mode: Asynchronous USART
// Data bits: 8
// Stop bits: 1
// Parity: Disabled
	USARTC0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc
			| USART_CHSIZE_8BIT_gc;

// Receive complete interrupt: Low Level
// Transmit complete interrupt: Low Level
// Data register empty interrupt: Disabled
	USARTC0.CTRLA = (USARTC0.CTRLA
			& (~(USART_RXCINTLVL_gm | USART_TXCINTLVL_gm | USART_DREINTLVL_gm)))
			| USART_RXCINTLVL_LO_gc | USART_TXCINTLVL_LO_gc
			| USART_DREINTLVL_OFF_gc;

//#define F_CPU 32000000
#define BAUDRATE 19200

	USARTC0.BAUDCTRLA = BSEL(F_CPU,BAUDRATE) & 0xff;
	USARTC0.BAUDCTRLB = (BSCALE(F_CPU,BAUDRATE) << USART_BSCALE0_bp)
			| (BSEL(F_CPU,BAUDRATE) >> 8);

// Receiver: On
// Transmitter: On
// Double transmission speed mode: Off
// Multi-processor communication mode: Off
	USARTC0.CTRLB = (USARTC0.CTRLB
			& (~(USART_RXEN_bm | USART_TXEN_bm | USART_CLK2X_bm | USART_MPCM_bm
					| USART_TXB8_bm))) |
	USART_RXEN_bm | USART_TXEN_bm | USART_CLK2X_bm;
}

// USARTC0 Receiver interrupt service routine
//interrupt [USARTC0_RXC_vect] void usartc0_rx_isr(void)
ISR(USARTC0_RXC_vect) {
	unsigned char status;
	char data;
	status = USARTC0.STATUS;
	data = USARTC0.DATA;
if ((status & (USART_FERR_bm | USART_PERR_bm | USART_BUFOVF_bm)) == 0)
   {
   rx_buffer_usartc0[rx_wr_index_usartc0++]=data;
   if (rx_wr_index_usartc0 == RX_BUFFER_SIZE_USARTC0) rx_wr_index_usartc0=0;
   if (++rx_counter_usartc0 == RX_BUFFER_SIZE_USARTC0)
      {
      rx_counter_usartc0=0;
      rx_buffer_overflow_usartc0=1;
      }
   }
}

char getcharc0() {
	char data;


	while (rx_counter_usartc0 == 0);
	data = rx_buffer_usartc0[rx_rd_index_usartc0++];
#if RX_BUFFER_SIZE_USARTC0 != 256
	if (rx_rd_index_usartc0 == RX_BUFFER_SIZE_USARTC0)
		rx_rd_index_usartc0 = 0;
#endif
	asm("cli");
	--rx_counter_usartc0;
	asm("sei");
	return data;
}

void registerTimerc0(volatile uint8_t * t){
	timerc0 = t;
}

int getcharc0Time(uint8_t time) {
	int data;

	*timerc0 = time;
	while ((rx_counter_usartc0 == 0) && (*timerc0));
	if(*timerc0 == 0){
		return 0x0000;		// brak znaków
	}
	data = rx_buffer_usartc0[rx_rd_index_usartc0++];
#if RX_BUFFER_SIZE_USARTC0 != 256
	if (rx_rd_index_usartc0 == RX_BUFFER_SIZE_USARTC0)
		rx_rd_index_usartc0 = 0;
#endif
	asm("cli");
	--rx_counter_usartc0;
	asm("sei");
	data |= 0x0100;			// faktycznie zosta³ odebrany znak
	return data;
}

// USARTC0 Transmitter interrupt service routine
ISR (USARTC0_TXC_vect) {
	if (tx_counter_usartc0) {
		--tx_counter_usartc0;
		USARTC0.DATA = tx_buffer_usartc0[tx_rd_index_usartc0++];
		if (tx_rd_index_usartc0 == TX_BUFFER_SIZE_USARTC0)
			tx_rd_index_usartc0 = 0;
	} else {
#ifdef UARTC0_DE_PORT
		UARTC0_DE_PORT &= ~UARTC0_DE_BIT;	// zablokuj nadajnik RS485
#endif
	}
}

void putcharc0(char c) {
	while (tx_counter_usartc0 == TX_BUFFER_SIZE_USARTC0)
		;
	asm("cli");
	if (tx_counter_usartc0 || ((USARTC0.STATUS & USART_DREIF_bm) == 0)) {
		tx_buffer_usartc0[tx_wr_index_usartc0++] = c;
		if (tx_wr_index_usartc0 == TX_BUFFER_SIZE_USARTC0)
			tx_wr_index_usartc0 = 0;
		++tx_counter_usartc0;
	} else {
#ifdef UARTC0_DE_PORT
		UARTC0_DE_PORT |= UARTC0_DE_BIT;	// zablokuj nadajnik RS485
#endif
		USARTC0.DATA = c;
	}
	asm("sei");
}

