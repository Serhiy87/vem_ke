/*
	22.09.2011 Kononenko 
	для defined (__AVR_ATxmega128A1__) KeyQuantity 20
	KeypadIn доработал  если KeyQuantity>8

	27.12.2011 Kononenko добавлена фильтрация KeyPressed
		
	07.10.2013 - Kononenko -  Revised for vmd2_2 
		Поддержка возможности расположения пинов на разных портах
		Переделана KeypadInit, KeypadScan
		STR1, STR2, STR3, ROW1, ROW2
	
	29.07.2015 - Kucherenko - если объявлен дефайн QslRD, то RKEY обрабатывается если RNUM = 0 (в RemoteDisplay_F выбран Master)
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <stdint.h>

#include "memory.h"
#include "interru.h"

#define KeyPauseOff() KeyStroke=0xFF


#if defined (__AVR_ATxmega128A1__)
	#define KeyQuantity 20
	#define ROW {1<<7, 1<<6, 1<<5, 1<<4}
	#define STR {1<<7, 1<<0, 1<<1, 1<<2, 1<<3}
#else
	#define KeyQuantity 6
#endif
//static uint8_t KeyRow[] PROGMEM = ROW;
//static uint8_t KeyStr[] PROGMEM = STR;

uint8_t KeyPressed;
uint8_t KeyPressedNonFilter;	// For test
uint32_t KeyPressedCounter;

static uint8_t  KeyStroke;

static uint8_t TD_Keypad;

#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega2561__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
//	#define RowOn(KR)		cli(); PortKey &=~KR; sei()
//	#define RowOff(KR)	cli(); PortKey  |=KR; sei()
//	#define StrIn(str) PIN_Key
#elif defined (__AVR_ATxmega128A1__)
	#define RowOn(KR)		PORTK.OUTCLR = KR
	#define RowOff(KR)	PORTK.OUTSET = KR 
	#define StrIn(str) (str==0?PORTJ:PORTK).IN
#endif

// ~~~~~~~~~~~~~
void
KeypadInit(void)
{
	#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
/*		for (uint8_t row=0; row<sizeof(KeyRow); row++) {
			uint8_t KR = prb(KeyRow+row);
			DDR_Key |=KR;
			PortKey |=KR;
		}
		for (uint8_t str=0; str<sizeof(KeyStr); str++) {
			uint8_t KS = prb(KeyStr+str);
			DDR_Key &=~KS;
			PortKey |=KS;
		}*/
		DDR_KeyROW1 |= KeyROW1;
		DDR_KeyROW2 |= KeyROW2;
		PortKeyROW1 |= KeyROW1;
		PortKeyROW2 |= KeyROW2;
		
		DDR_KeySTR1 &= ~KeySTR1;
		DDR_KeySTR2 &= ~KeySTR2;
		DDR_KeySTR3 &= ~KeySTR3;
		PortKeySTR1 |= KeySTR1;
		PortKeySTR2 |= KeySTR2;
		PortKeySTR3 |= KeySTR3;

	#elif defined (__AVR_ATxmega128A1__)
		for (uint8_t row=0; row<sizeof(KeyRow); row++)
			PORTK.DIRSET = PORTK.OUTSET = prb(KeyRow+row);

		PORTJ.DIRCLR = (1<<7);
		for (uint8_t str=1; str<sizeof(KeyStr); str++)
			PORTK.DIRCLR = prb(KeyStr+str);
		PORTJ.PIN7CTRL = PORTK.PIN0CTRL = PORTK.PIN1CTRL = PORTK.PIN2CTRL = PORTK.PIN3CTRL = PORT_OPC_PULLUP_gc;
	#endif
	TD_Keypad = Timer8SysAlloc(1);
}
// ~~~~~~~~~~~~~~~~~~
inline static uint8_t
KeypadScan(void)
{
	uint8_t Key = 0;
	uint8_t PresKeysQuant = 0;

/*	for (uint8_t row=0; row<sizeof(KeyRow); row++) {
		uint8_t KR = prb(KeyRow+row);
		RowOn(KR);
		_delay_us(10);		// By trial
		for (uint8_t str=0; str<sizeof(KeyStr); str++) {
			if (StrIn(str) &prb(KeyStr+str))
				continue;
			if (Key==0)
				Key = str*sizeof(KeyRow)+row+1;
			else {
				Key = 0;
				str = sizeof(KeyStr);
				row = sizeof(KeyRow);
			}
		}
		RowOff(KR);
	}*/

	cli(); PortKeyROW1 &= ~KeyROW1; sei();
	_delay_us(10);		// By trial
	if(!(PIN_KeySTR1 & (KeySTR1))){ Key=2; PresKeysQuant++;} 
	if(!(PIN_KeySTR2 & (KeySTR2))){ Key=6; PresKeysQuant++;} 
	if(!(PIN_KeySTR3 & (KeySTR3))){ Key=4; PresKeysQuant++;} 
	cli(); PortKeyROW1 |= KeyROW1; sei();	

	cli(); PortKeyROW2 &= ~KeyROW2; sei();
	_delay_us(10);		// By trial
	if(!(PIN_KeySTR1 & (KeySTR1))){ Key=1; PresKeysQuant++;}
	if(!(PIN_KeySTR2 & (KeySTR2))){ Key=5; PresKeysQuant++;}
	if(!(PIN_KeySTR3 & (KeySTR3))){ Key=3; PresKeysQuant++;}
	cli(); PortKeyROW2 |= KeyROW2; sei();
	if(PresKeysQuant!=1) Key = 0;

	return KeyPressedNonFilter = Key;
}
// ~~~~~~~~~~~
uint8_t
KeypadIn(void)
{
	static uint8_t PrevKey;
	uint8_t KeyResult = 0;
	uint8_t Key = KeypadScan();	// постоянно считываем код
	if(Key) KeyPressedCounter++;
	if(Key != PrevKey){			// если код не совпадает - отмена действия 
		PrevKey = 0;
	}
	
	if (Timer8Stopp(TD_Keypad)) {
		StartTimer8(TD_Keypad, 6);	// By trial keystroke time == 70-180 ms
		if (Key != PrevKey) {
				PrevKey = Key;
				Key = 0;
				KeyStroke = 0;
		}
		else{
			KeyPressed = KeyResult = Key;
			if(KeyStroke<0xFF && ++KeyStroke>1 && KeyStroke<=0xF) KeyResult = 0;
		}

		#ifdef RKEY
			#ifdef QslRD
				if(!*(uint8_t*)RNUM)
			#endif
				if (KeyResult==0) {
					uint8_t I = 0;
					while (1<<(I++ & 0b00000111) != *((uint8_t*)RKEY+(I>>3)) && I<=KeyQuantity);
					if (I<=KeyQuantity)
						KeyResult = I;
					for(I=0;I<=(KeyQuantity/8);I++) *((uint8_t*)RKEY+I) = 0;
				}
		#endif
	}
	return KeyResult;
}
// ~~~~~~~~~~~
