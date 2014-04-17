/*
 * usarte0.c
 *
 *  Created on: 21 gru 2013
 *      Author: krzysztof
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usarte0.h"
#include "xmega_baud.h"
#include "../pcb.h"
#include <avr/eeprom.h>
volatile uint8_t *timere0;

uint8_t baudRateIndex;
uint8_t eeBaudRateIndex EEMEM;

const char string_1[7] PROGMEM = "1200";
const char string_2[7] PROGMEM = "2400";
const char string_3[7] PROGMEM = "4800";
const char string_4[7] PROGMEM = "9600";
const char string_5[7] PROGMEM = "19200";
const char string_6[7] PROGMEM = "28800";
const char string_7[7] PROGMEM = "38400";
const char string_8[7] PROGMEM = "57600";
const char string_9[7] PROGMEM = "115200";
const PGM_P stringBaudRateTable[] PROGMEM =
{
	string_1,
	string_2,
	string_3,
	string_4,
	string_5,
	string_6,
	string_7,
	string_8,
	string_9
};

const uint16_t baudRateTable[] = {0x0d04, 0xFd03, 0xed01, 0xdcfd,  0xccf5, 0xc89e, 0xbce5,  0xb88e, 0xa86e};

char rx_buffer_usarte0[RX_BUFFER_SIZE_USARTE0];

// This flag is set on USARTe0 Receiver buffer overflow
char rx_buffer_overflow_usarte0=0;

char tx_buffer_usarte0[TX_BUFFER_SIZE_USARTE0];

volatile unsigned char rx_wr_index_usarte0=0,rx_rd_index_usarte0=0,rx_counter_usarte0=0;
volatile unsigned char tx_wr_index_usarte0=0,tx_rd_index_usarte0=0,tx_counter_usarte0=0;

// USARTe0 initialization
void usarte0_init()
{
	baudRateIndex = eeprom_read_byte(&eeBaudRateIndex);
	if(baudRateIndex == 0xff) {
		baudRateIndex=0x04;
		eeprom_write_byte(&eeBaudRateIndex, 0x04);
	}

// Note: the correct PORTC direction for the RxD, TxD and XCK signals
// is configured in the ports_init function

// Transmitter is enabled
// Set TxD=1
PORTE.OUTSET = 0x08 + 0x04;
PORTE.PIN2CTRL = PORT_OPC_PULLUP_gc;
PORTE.DIRCLR = 0X04;
PORTE.DIRSET = 0X08;

#ifdef USARTE0_DE_PORT
		// inicjalizujemy liniê steruj¹c¹ nadajnikiem
		USARTE0_DE_ODBIERANIE;
		USARTE0_RE_ODBIERANIE;
	#endif

// Communication mode: Asynchronous USART
// Data bits: 8
// Stop bits: 1
// Parity: Disabled
USARTE0.CTRLC=USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;

// Receive complete interrupt: Low Level
// Transmit complete interrupt: Low Level
// Data register empty interrupt: Disabled
USARTE0.CTRLA=(USARTE0.CTRLA & (~(USART_RXCINTLVL_gm | USART_TXCINTLVL_gm | USART_DREINTLVL_gm))) |
	USART_RXCINTLVL_LO_gc | USART_TXCINTLVL_LO_gc | USART_DREINTLVL_OFF_gc;


USARTE0.BAUDCTRLA = (uint8_t) baudRateTable[4];
USARTE0.BAUDCTRLB = (uint8_t) (baudRateTable[4]>>8);
//USARTE0.BAUDCTRLA = BSEL(F_CPU,BAUDRATE) & 0xff;
//USARTE0.BAUDCTRLB = (BSCALE(F_CPU,BAUDRATE) << USART_BSCALE0_bp) | (BSEL(F_CPU,BAUDRATE) >> 8);
//// Required Baud rate: 115200
//// Real Baud Rate: 115211,5 (x1 Mode), Error: 0,0 %
//USARTE0.BAUDCTRLA=0x2E;
//USARTE0.BAUDCTRLB=((0x09 << USART_BSCALE_gp) & USART_BSCALE_gm) | 0x08;

// Receiver: On
// Transmitter: On
// Double transmission speed mode: Off
// Multi-processor communication mode: Off
USARTE0.CTRLB=(USARTE0.CTRLB & (~(USART_RXEN_bm | USART_TXEN_bm | USART_CLK2X_bm | USART_MPCM_bm | USART_TXB8_bm))) |
	USART_RXEN_bm | USART_TXEN_bm | USART_CLK2X_bm;
}




// USARTE0 Receiver interrupt service routine
//interrupt [USARTE0_RXC_vect] void usarte0_rx_isr(void)
ISR(USARTE0_RXC_vect)
{
unsigned char status;
char data;

status=USARTE0.STATUS;
data=USARTE0.DATA;
if ((status & (USART_FERR_bm | USART_PERR_bm | USART_BUFOVF_bm)) == 0)
   {
   rx_buffer_usarte0[rx_wr_index_usarte0++]=data;
   if (rx_wr_index_usarte0 == RX_BUFFER_SIZE_USARTE0) rx_wr_index_usarte0=0;
   if (++rx_counter_usarte0 == RX_BUFFER_SIZE_USARTE0)
      {
      rx_counter_usarte0=0;
      rx_buffer_overflow_usarte0=1;
      }
   }
}



char getchare0(void)
{
char data;

while (rx_counter_usarte0==0);
data=rx_buffer_usarte0[rx_rd_index_usarte0++];
if (rx_rd_index_usarte0 == RX_BUFFER_SIZE_USARTE0) rx_rd_index_usarte0=0;
asm("cli");
--rx_counter_usarte0;
asm("sei");
return data;
}


void registerTimere0(volatile uint8_t * t){
	timere0 = t;
}


char getchare0time(uint8_t time)
{
char data;

*timere0 = time;
while ((rx_counter_usarte0 == 0) && (*timere0));
if(*timere0 == 0){
	return 0x0000;		// brak znaków
}
while (rx_counter_usarte0==0);
data=rx_buffer_usarte0[rx_rd_index_usarte0++];
if (rx_rd_index_usarte0 == RX_BUFFER_SIZE_USARTE0) rx_rd_index_usarte0=0;
asm("cli");
--rx_counter_usarte0;
asm("sei");
return data;
}

// USARTE0 Transmitter interrupt service routine
ISR (USARTE0_TXC_vect)
{
	if (tx_counter_usarte0)
	   {
	   --tx_counter_usarte0;
	   USARTE0.DATA=tx_buffer_usarte0[tx_rd_index_usarte0++];
	   if (tx_rd_index_usarte0 == TX_BUFFER_SIZE_USARTE0) tx_rd_index_usarte0=0;
	   }
	else{
#ifdef USARTE0_DE_PORT
		rx_wr_index_usarte0=0;		// czyszczenie odebranych danych (odebrane to co nadane)
		rx_rd_index_usarte0=0;
		rx_counter_usarte0=0;
		USARTE0_DE_ODBIERANIE;	// zablokuj nadajnik RS485
		USARTE0_RE_ODBIERANIE;

#endif
	}
}


void putchare0(uint8_t c)
{
while (tx_counter_usarte0 == TX_BUFFER_SIZE_USARTE0);
asm("cli");
if (tx_counter_usarte0 || ((USARTE0.STATUS & USART_DREIF_bm)==0))
   {
   tx_buffer_usarte0[tx_wr_index_usarte0++]=c;
   if (tx_wr_index_usarte0 == TX_BUFFER_SIZE_USARTE0) tx_wr_index_usarte0=0;
   ++tx_counter_usarte0;
   }
else{
#ifdef USARTE0_DE_PORT
		USARTE0_DE_NADAWANIE;	// odblokuj nadajnik RS485
		USARTE0_RE_NADAWANIE;
#endif
	USARTE0.DATA=c;
}
asm("sei");
}


