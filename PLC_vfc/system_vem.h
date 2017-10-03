/*
	02.04.2012 * Kononenko *  Revised for vmd2_1 - LCDBackLight
	20.05.2013 * Kononenko *  Atmega2560 compartible
	14.10.2013 * Kononenko *  include "lcd.h" before "usart.h"
	30.12.2013 * Kononenko *  For vfc1 and vfc2
	16.04.2014 * Kononenko * if(RTC_Ready()) RTC_GetTrueTime();
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
#include "usart_vfc.h"
//#include "keypad_vfc.h"
//#include "menu.h"
#include "digitio.h"
//#include "analogi.h"
//#include "therm.h"
#include "therm_ds18b20.h"
//#include sysmenu
//#include "memoryX.h"

void ApplInit(void);
void ApplCycle(void);

/*
// ~~~~~~~~~~~~~~
static void
KeypadSweep(void)
{
	static T_KeyAct KeyAct[] PROGMEM = KEYS;
	T_KeyAct KA = prp(KeyAct+KeypadIn());
	if(KA){
		KA();
		AnyKey();
		#ifdef BACKLIGHT
			if(BackLightMode == 1){
				LCDBackLightOn();
				LCDBackLightStartTimer();
			}
		#endif
	}
}

// ~~~~~~~~~~~~
static void
LCD_Sweep(void)
{
	if(LCD_Rdy()) {
//		MenuOut();
	}
}

// ~~~~~~~~~~~~~~
*/
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
	TimeInit();
	MemCheckInit();
	LED_Init();
//	LCD_Init();
//	KeypadInit();
//	MenuInit();
	DigitInit();
//	AI_Init();
//	PWM_Init();
	OneWire_Init();
//	TWI_Init();
	USART_Init();
	InterruInit();	// <-sei();

	//Подготовка температур для ApplInit()
	do {
		wdt_reset();
		DigitIn();
//		TWI_In();
//		if(RTC_Ready()) RTC_GetTrueTime();
//		LCD_Sweep();
		DS18B20();
	} while( (DS18B20_Mode != DS18B20_END_CYCLE) );

//	MemCheckMsg();

	ApplInit();
	
	ScanCycleInit();

	for(;;) {
		sei();

		TimeCycle();
//		KeypadSweep();
		DigitIn();
		DS18B20();
//		AnalogIn();
//		TWI_In();
		USART_Cycle();

		ApplCycle();

		//LCD_Sweep();
		//LCD_Out();
		DigitOut();
//		PWM_Out();
	}
}
