/*
 * rs485Frame.c
 *
 *  Created on: 7 kwi 2014
 *      Author: krzysztofklimas@interia.pl
 */

#include <util/crc16.h>
#include "rs485Frame.h"
#include "USART/usartd0.h"

TModuleConfig moduleConfig;
TModuleConfig eeModuleConfig EEMEM;
uint8_t state;
TAlarmFrame alarmFrame;

void copyModuleConfigEEpromToRam() {
	eeprom_read_block(&moduleConfig, &eeModuleConfig, sizeof(moduleConfig));
}

void copyModuleConfigRamToEEprom() {
	eeprom_write_block(&moduleConfig, &eeModuleConfig, sizeof(moduleConfig));
}

void sendAskFrame(uint8_t didAddress){
	uint16_t crc16;

	crc16 = 0xffff;
	putchare0(didAddress);
	crc16 = _crc16_update(crc16, didAddress);
	putchare0('?');
	crc16 = _crc16_update(crc16, '?');
	putchare0((uint8_t) crc16);
	putchare0((uint8_t) (crc16 >> 8));
	while(USARTE0_DE_ISHI);
}

void sendAlarmFrame() {
	uint8_t i;
	uint8_t address;

	printf("%03d;",alarmFrame.mdAddress);
	printf("%02d;",alarmFrame.size);
	printf("%02x;",alarmFrame.mdState);
	for(i=0; i!=alarmFrame.size/2; i++){
		address = alarmFrame.mdAddress + alarmFrame.mmAddress[i];
		printf("%03d;%03d;",address, alarmFrame.mmAlarmCode[i]);
	}
	printf("\n");
}

uint8_t getFrameFromMd() {
	uint16_t crc16;
	uint16_t iData;
	uint8_t debugFrame;
	uint8_t myAddress;
	uint8_t sizeData;
	uint8_t mdAddress;
	uint8_t mdState;
	uint8_t i;

	debugFrame = 0;
	crc16 = 0xffff;

	if (rx_counter_usarte0) {
		iData = getchare0time(10);			// my address MCA
		if (debugFrame)
			printf("%c", (uint8_t) iData);
		crc16 = _crc16_update(crc16, (uint8_t) iData);
		myAddress = (uint8_t) iData;

		iData = getchare0time(10);			// size
		if (debugFrame)
			printf("%c", (uint8_t) iData);
		crc16 = _crc16_update(crc16, (uint8_t) iData);
		sizeData = (uint8_t) iData;
		alarmFrame.size = sizeData;

		iData = getchare0time(10);			// MDA
		if (debugFrame)
			printf("%c", (uint8_t) iData);
		crc16 = _crc16_update(crc16, (uint8_t) iData);
		mdAddress = (uint8_t) iData;
		alarmFrame.mdAddress = mdAddress;

		iData = getchare0time(10);			// MD State
		if (debugFrame)
			printf("%c", (uint8_t) iData);
		crc16 = _crc16_update(crc16, (uint8_t) iData);
		mdState = (uint8_t) iData;
		alarmFrame.mdState = mdState;

		for (i = 0; i != sizeData; i++) {
			iData = getchare0time(10);			// address MM
			crc16 = _crc16_update(crc16, (uint8_t) iData);
			alarmFrame.mmAddress[i] = iData;
			if (debugFrame)
				printf("%c", (uint8_t) iData);

			iData = getchare0time(10);			// alarm code MM
			crc16 = _crc16_update(crc16, (uint8_t) iData);
			alarmFrame.mmAlarmCode[i] = iData;
			if (debugFrame)
				printf("%c", (uint8_t) iData);
		}

		// read crc
		iData = getchare0time(10);
		if (debugFrame)
			printf("%c", (uint8_t) iData);
		crc16 = _crc16_update(crc16, (uint8_t) iData);

		iData = getchare0time(10);
		if (debugFrame)
			printf("%c", (uint8_t) iData);
		crc16 = _crc16_update(crc16, (uint8_t) iData);

		if (debugFrame)
			printf("%c%c\n", (uint8_t) crc16, (uint8_t) (crc16 >> 8));			// print calculated frame

		if (crc16 != 0)
			return 5;

		if (myAddress != moduleConfig.myAddress)
			return 4;

		LED1_OFF;
		return 0;
	}else
		return 1;
}

void userSetRs485() {
	uint16_t getData;
	uint8_t i;
	char buffer[10];

	printf("\n\n%cUSTAWIENIA RS485\n",12);

	while (1) {
		printf("\n\n0 Wyjœcie z edycji\n");
		strcpy_P(buffer, (PGM_P) pgm_read_word(&(stringBaudRateTable[baudRateIndex])));
		printf("1 Szybkoœæ transmisji %s\n", buffer);
		printf("2 Adres modu³u %d\n", moduleConfig.myAddress);
		printf("3 Zakres adresó modu³ów zbiorczych w domach %d", moduleConfig.maxMdAddress);
		printf("\nWybierz numer parametru do edycji\n");

		scanf("%d", &getData);
		switch (getData) {
		case 0: {
			printf("Koniec konfiguracji radia\n");
			return;
			break;
		}
		case 1: {
			for (i = 0; i != (uint8_t) 9; i++) {
				strcpy_P(buffer, (PGM_P) pgm_read_word(&(stringBaudRateTable[i])));
				printf("%d %s\n", i, buffer);
			}
			printf("wybierz numer prêdkoœci\n");
			scanf("%d", &getData);
			baudRateIndex = (uint8_t) getData;
			eeprom_write_byte(&eeBaudRateIndex, baudRateIndex);
			break;
		}
		case 2: {
			printf("podaj adres\n");
			scanf("%d", &getData);
			moduleConfig.myAddress = (uint8_t) getData;
			copyModuleConfigRamToEEprom();
			break;
		}
		case 3: {
			printf("podaj max adres\n");
			scanf("%d", &getData);
			moduleConfig.maxMdAddress = (uint8_t) getData;
			copyModuleConfigRamToEEprom();
			break;
		}
		default: {
			printf("Z£Y WYBÓR\n\n\n");
		}
		}
	}
}

