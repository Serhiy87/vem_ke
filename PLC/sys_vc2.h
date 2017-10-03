/*~~~~~~~~~~~~~
	23.10.2013 - Kononenko -  Revised for new keypad.h
 	26.12.2013 - Kononenko -  Revised
 ~~~~~~~~~~~~~~~~~~~~*/

#define PinLED (1<<PC4)
#define DDR_LED DDRC
#define PortLED PORTC

#define Port_LCD_RS PORTC
#define DDR_LCD_RS DDRC
#define LCD_RS (1<<PC7)

#define Port_LCD_E PORTG
#define DDR_LCD_E DDRG
#define LCD_E (1<<PG2)

#define Port_LCD_DB4 PORTA
#define Port_LCD_DB5 PORTA
#define Port_LCD_DB6 PORTA
#define Port_LCD_DB7 PORTA
#define DDR_LCD_DB4 DDRA
#define DDR_LCD_DB5 DDRA
#define DDR_LCD_DB6 DDRA
#define DDR_LCD_DB7 DDRA
#define DB4 (1<<PA6)
#define DB5 (1<<PA7)
#define DB6 (1<<PA4)
#define DB7 (1<<PA5)

#define TypeLCD 420

#define Timer_LCD 3

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define PortKeySTR1 PORTC
#define PortKeySTR2 PORTC
#define PortKeySTR3 PORTC
#define PortKeyROW1 PORTC
#define PortKeyROW2 PORTC
#define DDR_KeySTR1 DDRC
#define DDR_KeySTR2 DDRC
#define DDR_KeySTR3 DDRC
#define DDR_KeyROW1 DDRC
#define DDR_KeyROW2 DDRC
#define PIN_KeySTR1 PINC
#define PIN_KeySTR2 PINC
#define PIN_KeySTR3 PINC
#define KeySTR1 (1<<PC6)
#define KeySTR2 (1<<PC5)
#define KeySTR3 (1<<PC3)
#define KeyROW1 (1<<PC2)
#define KeyROW2 (1<<PC1)

#define KEYS {NULL,							\
	StartKey,				StopKey,			\
	MenuEnterRight,	MenuEscLeft,	\
	MenuUp, 				MenuDown			\
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MB_PLC_Init(void)
{
	// RS485
	DDRC |=(1<<PC0); PORTC |=(1<<PC0);
}

void RS485_1(void) {PORTC ^=(1<<PC0);}

void MB_LED1(void) {}

void MB_LED_Err_On_1(void)	{}
void MB_LED_Err_Off_1(void)	{}

// ~~~~~~~~~~~~~~~~~~~~~~~~
#define ADC_Reg 10
#define DAC_Reg 2

#define PortCS_DAC_1	PORTB
#define PortCS_ADC		PORTB
#define DDR_CS_DAC_1	DDRB
#define DDR_CS_ADC		DDRB
#define CS_DAC_1	(1<<PB0)
#define CS_ADC		(1<<PB4)

#define PortRESET_PER PORTD
#define DDR_RESET_PER DDRD
#define RESET_PER	(1<<PD7)

#define Port_INH PORTE
#define Port_TER_A PORTE
#define Port_TER_B PORTE
#define DDR_INH DDRE
#define DDR_TER_A DDRE
#define DDR_TER_B DDRE
#define INH		(1<<PE4)
#define TER_A	(1<<PE5)
#define TER_B	(1<<PE6)

#define ADC_Mux {0b111, 0b100, 0b110, 0b101, 0b001, 0b010, 0b000, 0b011}

#include "spi.h"

// ~~~~~~~~~~~~~~~
#define PWM_Init()
#define PWM_Out()

// ~~~~~~~~~~~~~~~~~~~~~~~
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
#define Port_OUT_DE3 PORTB
#define Pin_OUT_DE1 PING
#define Pin_OUT_DE2 PIND
#define Pin_OUT_DE3 PINB
#define DDR_OUT_DE1 DDRG
#define DDR_OUT_DE2 DDRD
#define DDR_OUT_DE3 DDRB
#define OUT_DE1 (1<<PG3)
#define OUT_DE2 (1<<PD4)
#define OUT_DE3 (1<<PB6)

#define DI_Map {{23, 22, 21, 20, 19, 18, 17, 16}, {7, 6, 5, 4, 2, 3, 1, 0}, {15, 14, 13, 12, 10, 11, 9, 8}}

#define ION_Init() DDRG &=~(1<<PG0) &~(1<<PG1); PORTG |=(1<<PG0) |(1<<PG1)
#define ION_In() DI(PING &(1<<PG0), 0, 4); DI(PING &(1<<PG1), 0, 5)

#define HSC_Init()
#define HSC_In()

// ~~~~~~~~~~~~~~~~~~~~
#define DO_Reg 2

#define PortStrobe 	PORTA
#define PortData 	PORTA
#define PortEnable 	PORTA
#define PortClock 	PORTA

#define DDR_Strobe 	DDRA
#define DDR_Data 	DDRA
#define DDR_Enable 	DDRA
#define DDR_Clock	DDRA

#define Strobe		(1<<PA0)
#define Data		(1<<PA1)
#define Enable		(1<<PA2)
#define Clock		(1<<PA3)

#define DO_Map {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}

// ~~~~~~~~~~~~~~~~~~~
#define AI_Reg 12

#define PortAN_A PORTE
#define PortAN_B PORTE
#define DDR_AN_A DDRE
#define DDR_AN_B DDRE
#define AN_A (1<<PE2)
#define AN_B (1<<PE3)

#define AI_Mux {ADC4, ADC5, ADC2, ADC3, ADC6, ADC6, ADC6, ADC6, ADC7, ADC7, ADC7, ADC7}
#define AI_Mux2 {[4]=0b11, 0b10, 0b00, 0b01, 0b00, 0b01, 0b10, 0b11}

// ~~~~~~~~~~~~~
#include "i2c.h"

// ~~~~~~~~~~~~~~~~~
#define R_PLC_Type 5

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define TerReg ADC_Reg
#define TerInput ADC_Input
#define InitThermFami {Thermis, Thermis, Thermis, Thermis, Thermis, Thermis, Thermis, Thermis, Thermoc, Thermoc}
#define InitThermCorr {{0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define sysmenu "sysmenucl3.h"
