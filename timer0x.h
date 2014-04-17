/*
 * timer0x.h
 *
 *  Created on: 8 kwi 2014
 *      Author: krzysztofklimas@interia.pl
 */

#ifndef TIMER0X_H_
#define TIMER0X_H_

#include <avr/io.h>
#include <avr/interrupt.h>

extern volatile uint8_t *timer0T;

void tcc0_init(void);

void registerTimerInTimer0(volatile uint8_t * time);

#endif /* TIMER0X_H_ */
