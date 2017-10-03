// ~~~~~~~~~~~~~~~~
#define TypeLCD 216

// ~~~~~~~~~~~~~~
#define RS485Tx 0

// ~~~~~~~~~~~~~~~
#define SPI_Init()
#define DAC_Out()

// ~~~~~~~~~~~~~~~
#define DI_Reg 3

#define Port_OUT_DEA PORTB
#define Port_OUT_DEB PORTD
#define Port_OUT_DEC PORTG
#define DDR_OUT_DEA DDRB
#define DDR_OUT_DEB DDRD
#define DDR_OUT_DEC DDRG
#define OUT_DEA (1<<PB7)
#define OUT_DEB (1<<PD5)
#define OUT_DEC (1<<PG4)

#define Port_OUT_DE1 PORTG
#define Port_OUT_DE2 PORTD
#define Pin_OUT_DE1 PING
#define Pin_OUT_DE2 PIND
#define DDR_OUT_DE1 DDRG
#define DDR_OUT_DE2 DDRD
#define OUT_DE1 (1<<PG3)
#define OUT_DE2 (1<<PD4)

#define DI_Map {{5, 4, 3, 2, 0, 1, 15, 14}, {13, 12, 11, 10, 8, 9, 7, 6}, {17, 16}}

#define ION_Init() DDRG &=~(1<<PG0) &~(1<<PG1); PORTG |=(1<<PG0) |(1<<PG1)
#define ION_In() DI(PING &(1<<PG0), 2, 0); DI(PING &(1<<PG1), 2, 1)

// ~~~~~~~~~~~~~~~~~~~~
#define DO_Reg 2

#define PortShift PORTA
#define DDR_Shift DDRA
#define Strobe	(1<<PA0)
#define Data		(1<<PA1)
#define Enable	(1<<PA2)
#define Clock		(1<<PA3)

#define DO_Map {0, 1, 2, 3, 4, 5, 15, 15, 6, 7, 8, 9, 10, 11, 15, 15}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define AI_Reg 8
#define AI_Mux {ADC4, ADC5, ADC6, ADC7, ADC3, ADC0, ADC1, ADC2}

// ~~~~~~~~~~~~~~~
#define TWI_Init()
#define TWI_In()

// ~~~~~~~~~~~~~~~~~~~~
#define PWRKEY (1<<PB6)
#define DDR_PWRKEY DDRB

// ~~~~~~~~~~~~~~~~~~
#define MB_PLC_Type 2

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define TerReg 2
#define TerIdx 6
#define TerInput (AnalogInputF+TerIdx)
#define InitThermFami {Thermis, Thermis}
#define InitThermCorr {{0, 1000}, {0, 1000}}
#define Ter10ms 60

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define sysmenu "sysmenumiold.h"
