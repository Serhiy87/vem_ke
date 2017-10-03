/*~~~~~~~~~~~~~
	23.10.2013 - Kononenko -  Revised for new keypad.h
	26.12.2013 - Kononenko -  Revised
 ~~~~~~~~~~~~~~~~~~~~*/
// ~~~~~~~~~~~~~~~~~~~~
#define PinLED (1<<PA3)
#define DDR_LED DDRA
#define PortLED PORTA

// ~~~~~~~~~~~~~~~~~~
#define Port_LCD_RS PORTK
#define DDR_LCD_RS DDRK
#define LCD_RS (1<<PK4)

#define Port_LCD_E PORTK
#define DDR_LCD_E DDRK
#define LCD_E (1<<PK5)

#define Port_LCD_DB4 PORTK
#define Port_LCD_DB5 PORTK
#define Port_LCD_DB6 PORTJ
#define Port_LCD_DB7 PORTA
#define DDR_LCD_DB4 DDRK
#define DDR_LCD_DB5 DDRK
#define DDR_LCD_DB6 DDRJ
#define DDR_LCD_DB7 DDRA
#define DB4 (1<<PK6)
#define DB5 (1<<PK7)
#define DB6 (1<<PJ7)
#define DB7 (1<<PA0)

#define TypeLCD 420

#define Timer_LCD 4

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define DDR_Key DDRA
#define PortKey PORTA
#define PIN_Key PINA

#define ROW {1<<PA6, 1<<PA5}
#define STR {1<<PA1, 1<<PA4, 1<<PA2}

#define PortKeySTR1 PORTA
#define PortKeySTR2 PORTA
#define PortKeySTR3 PORTA
#define PortKeyROW1 PORTA
#define PortKeyROW2 PORTA
#define DDR_KeySTR1 DDRA
#define DDR_KeySTR2 DDRA
#define DDR_KeySTR3 DDRA
#define DDR_KeyROW1 DDRA
#define DDR_KeyROW2 DDRA
#define PIN_KeySTR1 PINA
#define PIN_KeySTR2 PINA
#define PIN_KeySTR3 PINA
#define KeySTR1 (1<<PA1)
#define KeySTR2 (1<<PA2)
#define KeySTR3 (1<<PA4)
#define KeyROW1 (1<<PA5)
#define KeyROW2 (1<<PA6)

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define KEYS {NULL,							\
	StartKey,				StopKey,			\
	MenuEnterRight,	MenuEscLeft,	\
	MenuUp, 				MenuDown			\
}
enum KEY_ENUM{KEY_NONE, KEY_START, KEY_STOP, KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MB_PLC_Init(void)
{
	// RS485
	DDRE |=(1<<PE2); PORTE &=~(1<<PE2);
	DDRH |=(1<<PH2); PORTH &=~(1<<PH2);
}

void RS485_0(void) {PORTE ^=(1<<PE2);}
void RS485_2(void) {PORTH ^=(1<<PH2);}

void MB_LED0(void) {}
void MB_LED2(void) {}

void MB_LED_Err_On_0(void)	{}
void MB_LED_Err_Off_0(void)	{}
void MB_LED_Err_On_1(void)	{}
void MB_LED_Err_Off_1(void)	{}
void MB_LED_Err_On_2(void)	{}
void MB_LED_Err_Off_2(void)	{}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define ADC_Reg 8

#define PortRESET_PER PORTH
#define DDR_RESET_PER DDRH
#define RESET_PER	(1<<PH6)

#define Port_INH PORTG
#define Port_TER_A PORTE
#define Port_TER_B PORTE
#define DDR_INH DDRG
#define DDR_TER_A DDRE
#define DDR_TER_B DDRE
#define INH		(1<<PG5)
#define TER_A	(1<<PE6)
#define TER_B	(1<<PE7)

#define ADC_Mux {0b111, 0b100, 0b110, 0b101, 0b001, 0b010, 0b000, 0b011}

#define ADC_Ref_Chan

#define DAC_Out()

#include "spi.h"

// ~~~~~~~~~~~~~~
#define PWM_Reg 3

#include "pwm.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define DI_Reg 3

#define SetDI(Port, Pin, Inversion) {&DDR##Port, &PORT##Port, &PIN##Port, 1<<P##Port##Pin, Inversion}

#define DI_Map_Own {																																											\
	SetDI(D, 5, 0), SetDI(D, 6, 0), SetDI(D, 7, 0), SetDI(G, 0, 0), SetDI(G, 1, 0), SetDI(C, 0, 0), SetDI(C, 1, 0), SetDI(C, 2, 0),	\
	SetDI(C, 3, 0), SetDI(C, 4, 0), SetDI(C, 5, 0), SetDI(C, 6, 0), SetDI(C, 7, 0), SetDI(J, 0, 0), SetDI(J, 1, 0), SetDI(J, 2, 0),	\
	SetDI(J, 3, 0), SetDI(J, 4, 0), SetDI(J, 5, 0), SetDI(J, 6, 0), SetDI(G, 2, 0), SetDI(A, 7, 0), SetDI(L, 7, 0), SetDI(D, 4, 0)	\
}

#define HSC_Init()
#define HSC_In()

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define DO_Reg 2

#define PortEnable PORTB
#define DDR_Enable DDRB
#define Enable	(1<<PB0)

#define SetDO(Port, Pin) {&DDR##Port, &PORT##Port, 1<<P##Port##Pin}

#define DO_Map_Own {																																											\
	SetDO(L, 5), SetDO(L, 4), SetDO(L, 3), SetDO(L, 2), SetDO(L, 1), SetDO(L, 0), SetDO(G, 4), SetDO(G, 3),	\
	SetDO(H, 7), SetDO(B, 7), SetDO(B, 6), SetDO(B, 5), SetDO(B, 4), SetDO(H, 5), SetDO(H, 4), SetDO(H, 3)	\
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define AI_Reg 12
#define AI_Mux {ADC0, ADC1, ADC2, ADC3, ADC4, ADC5, ADC6, ADC7, ADC8, ADC9, ADC10, ADC11}

// ~~~~~~~~~~~~~~~
#define LM_Exist 0
#include "i2c.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Add hardware turn on, restart using PWRKEY and STATUS pins
void GSM_PWRCNTRL_Config(void){
	cli();
	DDRL|=(1<<PL6);
	sei();
}
void GSM_PWRCNTRL_OFF(void){
	cli();
	PORTL &= ~(1<<PL6);
	sei();
}
void GSM_PWRCNTRL_ON(void){
	cli();
	PORTL|=(1<<PL6);
	sei();
}

// ~~~~~~~~~~~~~~~~~
#define R_PLC_Type 5

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define TerReg (ADC_Reg-1)
#define TerInput ADC_Input
#define InitThermFami {Thermis, Thermis, Thermis, Thermis, Thermis, Thermis, Thermis}
#define InitThermCorr {{0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define sysmenu "sysmenucl4.h"
