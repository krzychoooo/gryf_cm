/*
 * main.c
 *
 *  Created on: 18 gru 2013
 *      Author: krzysztof
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
//#include <avr/interrupt.h>
#include "pcb.h"
#include "USART/usartc0.h"
#include "USART/usartd0.h"
#include "USART/usarte0.h"
#include "XMEGA_CLK/xmega_clk.h"
#include "timer0x.h"
#include "rs485Frame.h"

FILE mystdout = FDEV_SETUP_STREAM(putchard0Stream, NULL, _FDEV_SETUP_WRITE);
FILE mystdin = FDEV_SETUP_STREAM(NULL, getchard0, _FDEV_SETUP_READ);

volatile uint8_t timer10ms;
uint8_t addressMd;

void enterSetup(){
	printf("%cNaciœnij spacjê\n",12);

	if((char) (getchard0Time((uint8_t) 255)) == ' '){

		userSetRs485();
	}
}

int main()
{
	unsigned char n;


	stdin = &mystdin;
	stdout = &mystdout;

	// Interrupt system initialization
	// Optimize for speed

	// Make sure the interrupts are disabled
	asm("cli");
	// Low level interrupt: On
	// Round-robin scheduling for low level interrupt: Off
	// Medium level interrupt: Off
	// High level interrupt: Off
	// The interrupt vectors will be placed at the start of the Application FLASH section

	n=(PMIC.CTRL & (~(PMIC_RREN_bm | PMIC_IVSEL_bm | PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm))) |
		PMIC_LOLVLEN_bm;
	CCP=CCP_IOREG_gc;
	PMIC.CTRL=n;
	// Set the default priority for round-robin scheduling
		PMIC.INTPRI=0x00;
		// Restore optimization for size if needed

		system_clocks_init();
		usartc0_init();
		usartd0_init();
		usarte0_init();

		PORTC.DIRSET = 0X38 + 0x02 + 0x01;
		PORTA.DIRSET = 0X20;
		PORTD.DIRSET = 0X38;
//		CONFIG_RADIO_DIR_OUT;

		asm("sei");

		registerTimerd0(&timer10ms);
		registerTimerc0(&timer10ms);
		registerTimere0(&timer10ms);

		LED1_OFF;

//Setting RS485
		copyModuleConfigEEpromToRam();

//timer0 init
		tcc0_init();
		registerTimerInTimer0( &timer10ms);

//enter to user setup
		enterSetup();

		printf("DIRE=%X PORTE=%x\n",PORTE.DIR,PORTE.OUT);
		addressMd = 0x01;
		while(1){
			sendAskFrame(addressMd);
			_delay_ms(2);
			n = getFrameFromMd();
//			printf("kod%d\n", n);
			if(++addressMd == (moduleConfig.maxMdAddress+1)) addressMd = 1;
			if(n == 0){			// 0 -> no error frame OK
				sendAlarmFrame();
			}
			_delay_ms(2);
		}
}
