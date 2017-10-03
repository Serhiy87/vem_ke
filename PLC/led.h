// ~~~~~~~~~~~~~~~~~~~
#ifndef _LED_H_
#define _LED_H_
// ~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~
#ifdef LED_Miss
// ~~~~~~~~~~~~~~~~~~~
#define LED_Init()

#define LED_Set()
#define LED_Rst()
#define LED_Inv()

#define LED_On()
#define LED_Off()

// ~~~~~~~~~~~~~~~~~~~
#else
// ~~~~~~~~~~~~~~~~~~~
#include <stdint.h>

#include "interru.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define LED_Set() PortLED  |=PinLED
#define LED_Rst() PortLED &=~PinLED
#define LED_Inv() PortLED  ^=PinLED

//~~~~~~~~~~~~
void
LED_Init(void)
{
	IntOff();
	DDR_LED |=PinLED;
	LED_Rst();
	IntOn();
}

// ~~~~~~~~~
void
LED_On(void)
{
	IntOff();
	LED_Set();
	IntOn();
}

// ~~~~~~~~~~
void
LED_Off(void)
{
	IntOff();
	LED_Rst();
	IntOn();
}

// ~~~~~~~~~~~~~
uint8_t
LED_Toggle(void)
{
	IntOff();
	LED_Inv();
	IntOn();
	return PortLED &PinLED;
}

// ~~~~~~~~~~~~~~~~~~~
#endif	//def LED_Miss
// ~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~
#endif	// _LED_H_
// ~~~~~~~~~~~~~~~~~~~
