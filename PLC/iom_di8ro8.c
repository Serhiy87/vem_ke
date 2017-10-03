#include <stdint.h>

#include <avr/io.h>

#define CommunicatFail

uint8_t		MB_Coil[4];
uint8_t		MB_Input[4];
uint16_t	MB_HoldReg[4];
uint16_t	MB_InReg[10];

#define Modbus_Map_List {	\
	{												\
		MB_Coil,		040,			\
		MB_Input,		040,			\
		MB_HoldReg,	4,				\
		MB_InReg,		10				\
	}												\
}

// ~~~~~~~
// sys_*.h
// ~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MB_PLC_Init(void)
{
	// RS485
	DDRD |=(1<<PD2); PORTD &=~(1<<PD2);

	// LED
	DDRC |=(1<<PC3); PORTC &=~(1<<PC3);
}

void RS485(void) {PORTD ^=(1<<PD2);}

void MB_LED(void) {PORTC ^=(1<<PC3);}

// ~~~~~~~~~~~~~~~~~~~~
#define DO_Reg 2

#define PortShift PORTC
#define PortClock PORTC
#define DDR_Shift DDRC
#define DDR_Clock DDRC
#define Strobe	(1<<PC5)
#define Data		(1<<PC7)
#define Enable	(1<<PC4)
#define Clock		(1<<PC6)

#define DO_Map {4, 5, 6, 7, 3, 2, 1, 0, 11, 10, 9, 8, 12, 13, 14, 15}

// ~~~~~~~~~~~~~~~~~~
// system.h
// ~~~~~~~~~~~~~~~~~~
#include "iom.h"
#include "digitio.h"

uint8_t DigInput_Invers EEMEM;

// ~~~~~~~~~~~~~~~
void
MB_Addr_Read(void)
{
	static prog_uint8_t MB_Addr[] = {1<<PA7, 1<<PA6, 1<<PA5, 1<<PA4};
	for (uint8_t i=0; i<sizeof(MB_Addr); i++) {
		uint8_t MB_Addr_i = prb(MB_Addr+i);
		DDRA &=~MB_Addr_i;
		PORTA |=MB_Addr_i;
		_delay_us(1);
		if (!(PINA &MB_Addr_i))
			MB_NativeAddr[0] += 1<<i;
	}
}

// ~~~~~~~~
// digitio2
// ~~~~~~~~
#define PinIn(Bit, Port, Pin) if (PIN##Port &(1<<P##Port##Pin)) ResDigOut(8+Bit); else SetDigOut(8+Bit)

#define PinOut(Bit, Port, Pin)																															\
	cli();																																										\
	if (DO_State(Bit)) PORT##Port |=(1<<P##Port##Pin); else PORT##Port &=~(1<<P##Port##Pin);	\
	sei()

// ~~~~~~~
int
main(void)
{
	IOM_Init();

	uint8_t TD_Common = Timer16Alloc();

	cli();

	DDRA &=~((1<<PA3)|(1<<PA2)|(1<<PA1)|(1<<PA0));
	DDRB &=~((1<<PB4)|(1<<PB3)|(1<<PB2)|(1<<PB1));
	PORTA |= (1<<PA3)|(1<<PA2)|(1<<PA1)|(1<<PA0);
	PORTB |= (1<<PB4)|(1<<PB3)|(1<<PB2)|(1<<PB1);

	DDRC |= (1<<PC2)|(1<<PC1)|(1<<PC0);
	DDRD |= (1<<PD7)|(1<<PD6)|(1<<PD5)|(1<<PD4)|(1<<PD3);

	sei();

	MB_Coil[2] = erb(&DigInput_Invers);

	// LEDs test
	StartTimer16(TD_Common, 50);
	while(!Timer16Stopp(TD_Common));

	IOM_Modbus_Init();

	#ifdef CommunicatFail
		StartTimer16(TD_Common, 500);
	#endif

	for(;;) {
		static uint16_t MB_CPT_4;

		IOM_Cycle();

		PinIn(0, B, 4);
		PinIn(1, B, 3);
		PinIn(2, B, 2);
		PinIn(3, B, 1);
		PinIn(4, A, 0);
		PinIn(5, A, 1);
		PinIn(6, A, 2);
		PinIn(7, A, 3);

		PinOut(0, C, 2);
		PinOut(1, C, 1);
		PinOut(2, C, 0);
		PinOut(3, D, 7);
		PinOut(4, D, 6);
		PinOut(5, D, 5);
		PinOut(6, D, 4);
		PinOut(7, D, 3);

		#ifdef CommunicatFail
			if (MB_CPT_4!=MB_CPT[0][3]) {
				MB_CPT_4 = MB_CPT[0][3];
				StartTimer16(TD_Common, 500);
			}
			else if (Timer16Stopp(TD_Common))
				MB_Coil[1] = 0;
		#endif

		MB_Input[1] = (MB_Input[0] = DigOutput.Buf[1]) ^ MB_Coil[2];
		if (MB_Coil[2]!=erb(&DigInput_Invers))
			ewb(&DigInput_Invers, MB_Coil[2]);
		DigOutput.Buf[0] = MB_Coil[1];
		MB_Coil[0] = DigOutput.State[0];
	}
}
