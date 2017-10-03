/*
	02.04.2012 * Kononenko *  Revised for vmd2_1 - LCDBackLight
	20.05.2013 * Kononenko *  Atmega2560 compartible
	14.10.2013 * Kononenko *  include "lcd.h" before "usart.h"
	30.12.2013 * Kononenko *  For vfc1 and vfc2
	16.04.2014 * Kononenko * if(RTC_Ready()) RTC_GetTrueTime();
	14.04.2015 * Kononenko * For vem (no lcd, no RTC, no Key)
*/

#ifdef BOOTLOADER_SUPPORT
	// Create image of bootloader section to check overlaps errors
	void __attribute__ ((section (".bootimage"))) BootLoaderImage(void){;}
	uint8_t BootLoaderFlag __attribute__ ((section (".bootflag")));
#endif

//updater start address	(занимает полседние 512 байт ATmega128)
#define ADDR_UPDATER		0xFF00

#include "memory.h"
#include "interru.h" 
#include "time.h"
#include sys_plc
#include "led.h"
//#include "lcd_vfc.h"
#include "usart_vem.h"
//#include "keypad_vfc.h"
//#include "menu.h"
#include "digitio.h"
#include "digiproc.h"
//#include "analogi.h"
//#include "therm.h"
#include "therm_ds18b20.h"
//#include sysmenu
//#include "memoryX.h"

void ApplInit(void);
void ApplCycle(void);

// ~~~~~~~~~~~~~~
void Reboot(void){
	cli();
	while(1){}
}
void ResetGSMState(void){
	GSM_State = GSM_ReStart1;
}
void Start_Bootloader(void){
	cli();
	ewb((uint8_t*)E2END, 0xFF);	//яЁшчэръ эрышўш  Application program
	while(1){}		//reset
}
void Start_Updater(void){
	cli();
	(*((void(*)(void))ADDR_UPDATER))();			//jump
}
// ~~~~~~~~~~~~~~
/*
void
MemCheckMsg(void)
{
	if(MemCheck())
		ShowMsg(prp(MsgErrMemo+MemFail));
}
*/
// ~~~~~~~
int
main(void)
{
	TimersInc();
	TimeInit();
	MemCheckInit();
	LED_Init();
	DigitInit();
	OneWire_Init();
	USART_Init();
	InterruInit();	// <-sei();

	//Подготовка температур для ApplInit()
	do {TimersInc();
		wdt_reset();
		DigitIn();
		DS18B20();
	} while(DS18B20_Mode != DS18B20_END_CYCLE);

//	MemCheckMsg();

	ApplInit();
	
	ScanCycleInit();

	for(;;) {TimersInc();
		sei();

		TimeCycle();
		DigitIn();
		DS18B20();
		USART_Cycle();

		ApplCycle();

		DigitOut();
	}
}
