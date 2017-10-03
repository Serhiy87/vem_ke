#include <stdint.h>

#include <avr/io.h>

#define ADC_Ref 0xFFFF/2		// Naumenko

// ~~~~~~~
// sys_*.h
// ~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MB_PLC_Init(void)
{
	// RS485
	DDRD |=(1<<PD2); PORTD &=~(1<<PD2);

	// LED
	DDRD |=(1<<PD6); PORTD &=~(1<<PD6);
}

void RS485(void) {PORTD ^=(1<<PD2);}

void MB_LED(void) {PORTD ^=(1<<PD6);}

// ~~~~~~~~~~~~~~~~~~~~~~~~
#define ADC_Reg 8

#define PortRESET_PER PORTA
#define DDR_RESET_PER DDRA
#define RESET_PER	(1<<PA5)

#define Port_TER_A	PORTD
#define Port_TER_B	PORTD
#define Port_INH		PORTD
#define DDR_TER_A DDRD
#define DDR_TER_B DDRD
#define DDR_INH 	DDRD
#define TER_A	(1<<PD3)
#define TER_B	(1<<PD4)
#define INH		(1<<PD5)

#define ADC_Mux {0b111, 0b110, 0b101, 0b100, 0b011, 0b010, 0b001, 0b000}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define InitThermFami {Thermoc, Thermoc, Thermoc, Thermoc, Thermoc, Thermoc, Thermoc}

// ~~~~~~~~~~~~~~~
#define LM_Exist 1

// ~~~~~~~~~~~~~~~~~~~
// i2c.h
// ~~~~~~~~~~~~~~~~~~~
int16_t InnerTer;

// ~~~~~~~~~~~~~~~~
// system.h
// ~~~~~~~~~~~~~~~~
#include "iom_t7.h"

// ~~~~~~~~
// digitio2
// ~~~~~~~~
prog_uint8_t DO_Pin[] = {1<<PA0, 1<<PA1, 1<<PA2, 1<<PA3, 1<<PA4, 1<<PA6, 1<<PA7};

// ~~~~~~~~~~~~~~~~~~~
void
SetDigOut(uint8_t D_N)
{
	cli();
	PORTA |=prb(DO_Pin+D_N);
	sei();
}

// ~~~~~~~~~~~~~~~~~~~
void
ResDigOut(uint8_t D_N)
{
	cli();
	PORTA &=~prb(DO_Pin+D_N);
	sei();
}

// ~~~~~~~~~~~~~~~
void
SetAllDigOut(void)
{
	cli();
	PORTA |=(1<<PA0) |(1<<PA1) |(1<<PA2) |(1<<PA3) |(1<<PA4) |(1<<PA6) |(1<<PA7);
	sei();
}

// ~~~~~~~~~~
void
DO_Init(void)
{
	cli();
	DDRA |=(1<<PA0) |(1<<PA1) |(1<<PA2) |(1<<PA3) |(1<<PA4) |(1<<PA6) |(1<<PA7);
	sei();
}

// ~~~~~~~~~~~
void
DigitOut(void)
{
}

// ~~~~~~~~~~~~
// i2c.h
// ~~~~~~~~~~~~
uint8_t TD_TWI;

// ~~~~~~~~~~~
inline void
TWI_Init(void)
{
	TWBR = 144;
	TD_TWI = Timer8SysAlloc(1);
}

// ~~~~~~~~~
void
TWI_In(void)
{
	enum {LM_Start, LM_SLA_R, AcknLM, LM_Ter1, LM_Ter2};
	static uint8_t TWI_Phase;

	static int8_t InnerTerBuf;

	if(TWI_Phase!=LM_Start && !(TWCR &(1<<TWINT)))
		return;
	switch(TWI_Phase) {
	case LM_Start:
		if(Timer8Stopp(TD_TWI)) {
			TWCR = (1<<TWINT) |(1<<TWSTA) |(1<<TWEN);
			TWI_Phase++;
		}
		break;
	case LM_SLA_R:
		TWDR = (0b1001001<<1)+1;
		TWCR = (1<<TWINT) |(1<<TWEN);
		TWI_Phase++;
		break;
	case AcknLM:
		TWCR = (1<<TWINT) |(1<<TWEA) |(1<<TWEN);
		TWI_Phase++;
		break;
	case LM_Ter1:
		InnerTerBuf = TWDR;
		TWCR = (1<<TWINT) |(1<<TWEN);
		TWI_Phase++;
		break;
	case LM_Ter2:
		InnerTer = InnerTerBuf*10;
		if(TWDR)
			InnerTer += 5;
		TWCR = (1<<TWINT) |(1<<TWSTO) |(1<<TWEN);
		TWI_Phase = LM_Start;
		StartTimer8(TD_TWI, 40);	// LM75 300 ms needs
		break;
	}
}

// ~~~~~~~
int
main(void)
{
	IOM_T7_Init();
	TWI_Init();
	for(;;) {
		IOM_T7_Cycle();
		TWI_In();
		MB_InReg[20] = InnerTer;
	}
}
