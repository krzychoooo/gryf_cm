/*
 * usarte0.h
 *
 *  Created on: 21 gru 2013
 *      Author: krzysztof
 */

#ifndef USARTE0_H_
#define USARTE0_H_

#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "../pcb.h"

#define BAUDRATE 19200

extern const char string_1[];
extern const char string_2[];
extern const char string_3[];
extern const char string_4[];
extern const char string_5[];
extern const char string_6[];
extern const char string_7[];
extern const char string_8[];
extern const char string_9[];
extern const PGM_P stringBaudRateTable[];
extern volatile uint8_t *timere0;
extern volatile unsigned char rx_counter_usarte0;
extern uint8_t baudRateIndex;
extern uint8_t eeBaudRateIndex EEMEM;
extern const uint16_t baudRateTable[] ;


// USARE0 initialization
void usarte0_init();

// USARTE0 Receiver buffer
#define RX_BUFFER_SIZE_USARTE0 8


char getchare0();
void registerTimere0(volatile uint8_t * t);
char getchare0time(uint8_t time);

// USARTE0 Transmitter buffer
#define TX_BUFFER_SIZE_USARTE0 8

// Write a character to the USARTE0 Transmitter buffer
// USARTE0 is used as the default output device by the 'putchar' function

void putchare0(uint8_t c);

#endif /* USARTE0_H_ */
