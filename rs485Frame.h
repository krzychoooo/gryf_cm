/*
 * rs485Frame.h
 *
 *  Created on: 7 kwi 2014
 *      Author: krzysztofklimas@interia.pl
 */

#ifndef RS485FRAME_H_
#define RS485FRAME_H_

#include <avr/io.h>
#include "USART/usarte0.h"

typedef struct moduleConfig{
	uint8_t myAddress;
	uint8_t	maxMdAddress;
	uint8_t baudRate;		// uwaga nie oprogramowane ustawienie prêdkoci siedzi w uart
} TModuleConfig;

typedef struct alarmFrameStruct{
	uint8_t mdAddress;
	uint8_t size;
	uint8_t mdState;
	uint8_t mmAddress[32];
	uint8_t mmAlarmCode[32];
} TAlarmFrame;

extern TModuleConfig moduleConfig;
extern uint8_t state;	//stan modu³u  ewentualnie przerobiæ na struct

void copyModuleConfigEEpromToRam();
void copyModuleConfigRamToEEprom();

void sendAskFrame(uint8_t didAddress);
uint8_t getFrameFromMd();

void userSetRs485();

void sendAlarmFrame();
void userSetRs485();

#endif /* RS485FRAME_H_ */
