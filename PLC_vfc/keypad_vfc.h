/*
	22.09.2011 Kononenko 
	для defined (__AVR_ATxmega128A1__) KeyQuantity 20
	KeypadIn доработал  если KeyQuantity>8

	27.12.2011 Kononenko добавлена фильтрация KeyPressed

	27.09.2013 Kononenko изменил KeypadInit, KeypadScan под vfc
	
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
static uint8_t KeyRow[] PROGMEM = ROW;
static uint8_t KeyStr[] PROGMEM = STR;

uint8_t KeyPressed;
uint8_t KeyPressedNonFilter;	// For test
uint32_t KeyPressedCounter;

static uint8_t  KeyStroke;

static uint8_t TD_Keypad;

#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega2561__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
	#define RowOn(KR)		cli(); PortKey &=~KR; sei()
	#define RowOff(KR)	cli(); PortKey  |=KR; sei()

	#define StrIn(str) PIN_Key
#elif defined (__AVR_ATxmega128A1__)
	#define RowOn(KR)		PORTK.OUTCLR = KR
	#define RowOff(KR)	PORTK.OUTSET = KR 

	#define StrIn(str) (str==0?PORTJ:PORTK).IN
#endif

// ~~~~~~~~~~~~~
void
KeypadInit(void)
{
	DDRC &= ~0b11110000;
	PORTC |= (1<<PC4)|(1<<PC5)|(1<<PC6)|(1<<PC7);	//Pull up
	
	TD_Keypad = Timer8SysAlloc(1);
}
// ~~~~~~~~~~~~~~~~~~
inline static uint8_t
KeypadScan(void)
{
	uint8_t Key = 0;

	if(!(PINC & (1<<PC4))) Key = 4;
	if(!(PINC & (1<<PC5))) Key = 6;
	if(!(PINC & (1<<PC6))) Key = 5;
	if(!(PINC & (1<<PC7))) Key = 3;

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
