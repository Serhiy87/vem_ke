/*
	02.04.2012 * Kononenko *  Revised for vmd2_1 - LCDBackLight
	20.05.2013 * Kononenko *  Atmega2560 compartible
	07.10.2013 * Kononenko *  Revised for vmd2_2 ()
	14.10.2013 * Kononenko *  include "lcd.h" before "usart.h"
	19.12.2013 * Kononenko *  Revised for vmd2_3 ()
	25.02.2014 * Kononenko * if(RTC_Ready()) RTC_GetTrueTime();
	30.07.2015 * Kucherenko* include "slaverd.h" after "usart.h" and after sysmenu
	15.08.2016 * Kononenko * Add check TimerAllocError
	01.09.2016 * Potapov * Add TimersInc()
	30.03.2017 * Kucherenko *  Go AVRStudio4 on AtmelStudio7:
	static T_KeyAct KeyAct[] PROGMEM = KEYS; ->  static const T_KeyAct KeyAct[] PROGMEM = KEYS;
*/

#ifdef BOOTLOADER_SUPPORT
	// Create image of bootloader section to check overlaps errors
	void __attribute__ ((section (".bootimage"))) BootLoaderImage(void){;}
	uint8_t BootLoaderFlag __attribute__ ((section (".bootflag")));
#endif

#include "memory.h"
#include "interru.h"
#include "time.h"
#include sys_plc
#include "led.h"
#include "lcd.h"
#include "usart.h"
#include "keypad.h"
#include "menu.h"
#include "digitio.h"
#include "analogi.h"
#include "therm.h"
#include sysmenu
#include "slaverd.h"
#include "memoryX.h"

void ApplInit(void);
void ApplCycle(void);

void StartKey(void);
void StopKey(void);

// ~~~~~~~~~~~~~~
static void
KeypadSweep(void)
{
	static const T_KeyAct KeyAct[] PROGMEM = KEYS;
	T_KeyAct KA = prp(KeyAct+KeypadIn());
	if(KA){
		KA();
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
		MenuOut();
		LCD_Out();
	}
}

// ~~~~~~~~~~~~~~
void
MemCheckMsg(void)
{
	if(MemCheck())
		ShowMsg(prp(MsgErrMemo+MemFail));
}

// ~~~~~~~
int
main(void)
{
	TimeInit();
	TimersInc();
	MemCheckInit();
	LED_Init();

	LCD_Init();

	KeypadInit();
	MenuInit();
	InterruInit();
	DigitInit();
	HSC_Init();

	AI_Init();

	SPI_Init();

	PWM_Init();
	TWI_Init();
	DF_Init();

#ifdef QslRD
	SlaveRD_Init();
#endif

	do {
		TimersInc();
		wdt_reset();
		DigitIn();
		TWI_In();
		if(RTC_Ready()) RTC_GetTrueTime();
		LCD_Sweep();
	#ifdef TerIdx
	} while(ThermInit(AnalogIn()-TerIdx) || !RTC_Ready());
	#elif !defined ADC_Miss
		AnalogIn();
	} while( (ADC_OK&&ThermInit(ADC_In())) || !RTC_Ready() );
	#else
	} while((AnalogIn()!=AI_Reg-1) || !RTC_Ready());
	#endif

	MemCheckMsg();

	ApplInit();

	USART_Init();
	ScanCycleInit();
	if(TimerAllocError()) ShowMsg(&MsgErr41);

	for(;;) {	
		TimersInc();
		TimeCycle();

		KeypadSweep();
		DigitIn();
		HSC_In();
		#ifdef TerIdx
			ThermChan(AnalogIn()-TerIdx);
		#else
			AnalogIn();
		#endif
		#if !defined ADC_Miss && !defined TerIdx
			static uint8_t ADC_Work = 1;
			if(ADC_Work&&ADC_Phase==ADC_OFF) {
				Therm_ADC_Off();
				ShowMsg(&MsgErr21);
				ADC_Work = 0;
			}
			ThermChan(ADC_In());
		#endif
		ThermExt();
		TWI_In();

		USART_Cycle();
#ifdef QslRD
		SlaveRD();
#endif
		ApplCycle();
		
		LCD_Sweep();
		DigitOut();
		DAC_Out();
		PWM_Out();
		#ifdef BACKLIGHT
			LCDBackLightControl();
		#endif
	}
}
