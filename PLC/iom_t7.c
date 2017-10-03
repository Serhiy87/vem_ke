/*
	10.04.2015 - HW rev2.0 - Кононенко - новые платы - инвертируются сигналы
				мультиплексора INH, TER_A, TER_B (не устанавливаются транзисторы)
				инверсия светодиода связи Modbus
*/
#include <avr/io.h>

//#define IOM_T7_WH2

// ~~~~~~~
// sys_*.h
// ~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MB_PLC_Init(void)
{
	// RS485
	DDRD |=(1<<PD2); PORTD &=~(1<<PD2);

	// LED
	DDRD |=(1<<PD3);

#ifdef IOM_T7_WH2
	PORTD &=~(1<<PD3);
#else
	PORTD |=(1<<PD3);
#endif

}

void RS485(void) {PORTD ^=(1<<PD2);}

void MB_LED(void) {PORTD ^=(1<<PD3);}

// ~~~~~~~~~~~~~~~~~~~~~~~~
#define PortCS_ADC	PORTA
#define DDR_CS_ADC	DDRA
#define CS_ADC	(1<<PA4)

#define ADC_Reg 8

#define PortRESET_PER PORTA
#define DDR_RESET_PER DDRA
#define RESET_PER	(1<<PA5)

#define Port_INH	PORTA
#define Port_TER_A	PORTA
#define Port_TER_B	PORTA
#define DDR_INH 	DDRA
#define DDR_TER_A 	DDRA
#define DDR_TER_B 	DDRA
#define INH		(1<<PA3)
#define TER_A	(1<<PA2)
#define TER_B	(1<<PA1)

#ifdef IOM_T7_WH2
	#define ADC_Mux {0b000, 0b011, 0b001, 0b010, 0b110, 0b101, 0b111, 0b100}
#else
	#define ADC_Mux {0b111, 0b100, 0b110, 0b101, 0b001, 0b010, 0b000, 0b011}
#endif

// ~~~~~~~~~~~~~~~~~~~~
	#define DO_Reg 1

	#define PortStrobe 	PORTC
	#define PortData 	PORTC
	#define PortEnable 	PORTC
	#define PortClock 	PORTC

	#define DDR_Strobe 	DDRC
	#define DDR_Data 	DDRC
	#define DDR_Enable 	DDRC
	#define DDR_Clock	DDRC

	#define Strobe		(1<<PC5)
	#define Data		(1<<PC7)
	#define Enable		(1<<PC4)
	#define Clock		(1<<PC6)

	#define DO_Map {3, 2, 1, 0, 4, 5, 6, 7}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define InitThermFami {Thermis, Thermis, Thermis, Thermis, Thermis, Thermis, Thermis}

// ~~~~~~~~~~~~~~~~~
// system.h
// ~~~~~~~~~~~~~~~~~
#include "iom_t7.h"
#include "digitio.h"

// ~~~~~~~
int
main(void)
{
	IOM_T7_Init();
	for (;;)
		IOM_T7_Cycle();
}
