// ~~~~~~~~~~~~~~~~~~~
#ifndef _INTERRU_H_
#define _INTERRU_H_
// ~~~~~~~~~~~~~~~~~~~

#include <stdint.h>

#include <avr\interrupt.h>

static uint8_t interru_enub_flg;

// ~~~~~~~~~~~~~~
void
InterruInit(void)
{
	#ifdef PMIC
		PMIC.CTRL = (uint8_t)(~PMIC_RREN_bm &~PMIC_IVSEL_bm) |PMIC_HILVLEN_bm |PMIC_MEDLVLEN_bm |PMIC_LOLVLEN_bm;
	#endif
	sei();
}

// ~~~~~~~~~
uint8_t
IntOff(void)
{
	interru_enub_flg = SREG &(1<<7);
	cli();
	return interru_enub_flg;
}

// ~~~~~~~~
uint8_t
IntOn(void)
{
	if (interru_enub_flg)
		sei();
	return interru_enub_flg;
}

// ~~~~~~~~~~~~~~~~~~~
#endif	// _INTERRU_H_
// ~~~~~~~~~~~~~~~~~~~
