/*~~~~~~~~~~~~~
	22.12.2011 - Kononenko - Add GSM hardware turn on, restart using PWRKEY
	19.12.2012 - Kononenko - Add GSM hardware turn on GSM_PWRCNTRL_Config
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
enum KEY_ENUM{KEY_NONE, KEY_START, KEY_STOP, KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MB_PLC_Init(void)
{
	// RS485
#ifndef GSM
	DDRE |=(1<<PE7); PORTE &=~(1<<PE7);
#endif
	DDRC |=(1<<PC0); PORTC &=~(1<<PC0);
}

void RS485_0(void) {PORTE ^=(1<<PE7);}
void RS485_1(void) {PORTC ^=(1<<PC0);}

void MB_LED0(void) {}
void MB_LED1(void) {}

void MB_LED_Err_On_0(void)	{}
void MB_LED_Err_Off_0(void)	{}
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

#define PortRESET_PER PORTE
#define DDR_RESET_PER DDRE
#define RESET_PER	(1<<PE6)

#define Port_INH PORTD
#define Port_TER_A PORTE
#define Port_TER_B PORTB
#define DDR_INH DDRD
#define DDR_TER_A DDRE
#define DDR_TER_B DDRB
#define INH		(1<<PD7)
#define TER_A	(1<<PE5)
#define TER_B	(1<<PB5)

#define ADC_Mux {0b111, 0b100, 0b110, 0b101, 0b001, 0b010, 0b000, 0b011}

#include "spi.h"

// ~~~~~~~~~~~~~~~
#define PWM_Init()
#define PWM_Out()

// ~~~~~~~~~~~~~~~~~~~~~~~
#define DI_Reg 3

#define Port_OUT_DEA PORTB
#define Port_OUT_DEB PORTG
#define Port_OUT_DEC PORTG
#define DDR_OUT_DEA DDRB
#define DDR_OUT_DEB DDRG
#define DDR_OUT_DEC DDRG
#define OUT_DEA (1<<PB7)
#define OUT_DEB (1<<PG3)
#define OUT_DEC (1<<PG4)

#define Port_OUT_DE1 PORTD
#define Port_OUT_DE2 PORTD
#define Port_OUT_DE3 PORTB
#define Pin_OUT_DE1 PIND
#define Pin_OUT_DE2 PIND
#define Pin_OUT_DE3 PINB
#define DDR_OUT_DE1 DDRD
#define DDR_OUT_DE2 DDRD
#define DDR_OUT_DE3 DDRB
#define OUT_DE1 (1<<PD5)
#define OUT_DE2 (1<<PD4)
#define OUT_DE3 (1<<PB6)

#if LevelSensors
	#define DI_Map {{22, 23, 1, 0, 2, 3, 4, 5}, {6, 7, 9, 8, 10, 11, 12, 13}, {14, 15, 17, 16, 19, 18, 20, 21}}
#else
	#define DI_Map {{22, 23, 1, 0, 2, 3, 4, 5}, {6, 7, 9, 8, 10, 11, 12, 13}, {14, 15, 17, 16, 18, 19, 20, 21}}
#endif

#define HSC_Init()
#define HSC_In()

// ~~~~~~~~~~~~~~~~~~~~~
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

#define DO_Map {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 0, 1, 2, 3}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define AI_Reg 12

#define PortAN_A PORTD
#define PortAN_B PORTG
#define PortAN_C PORTE
#define DDR_AN_A DDRD
#define DDR_AN_B DDRG
#define DDR_AN_C DDRE
#define AN_A (1<<PD6)
#define AN_B (1<<PG1)
#define AN_C (1<<PE4)

#define AI_Mux {ADC2, ADC2, ADC2, ADC2, ADC2, ADC2, ADC2, ADC2, ADC0, ADC1, ADC3, ADC3}
#define AI_Mux2 {0b101, 0b110, 0b111, 0b100, 0b010, 0b000, 0b001, 0b011}

// ~~~~~~~~~~~~~~~
#include "i2c.h"

// ~~~~~~~~~~~~~~~~~~~~
#define PWRKEY (1<<PE7)
#define DDR_PWRKEY DDRE

// ~~~~~~~~~~~~~~~~~
#define R_PLC_Type 5

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define TerReg ADC_Reg
#define TerInput ADC_Input
#define InitThermFami {Thermis, Thermis, Thermis, Thermis, Thermis, Thermis, Thermis, Thermis, Thermoc, Thermoc}
#define InitThermCorr {{0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Add hardware turn on, restart using PWRKEY and STATUS pins
void GSM_PWRKEY_Config(void){
	cli();
	DDRE|=(1<<PE7);
	sei();
}
void GSM_PWRKEY_NotPressed(void){
	cli();
	PORTE &= ~(1<<PE7);
	sei();
}
void GSM_PWRKEY_Pressed(void){
	cli();
	PORTE|=(1<<PE7);
	sei();
}
//STATUS pin is not used
void GSM_STATUS_Config(void){
}
uint8_t GSM_Status(void){
	return 1;
}
void GSM_PWRCNTRL_Config(void){
	cli();
	DDRE|=(1<<PE7);
	sei();
}
void GSM_PWRCNTRL_OFF(void){
	cli();
	PORTE &= ~(1<<PE7);
	sei();
}
void GSM_PWRCNTRL_ON(void){
	cli();
	PORTE|=(1<<PE7);
	sei();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define sysmenu "sysmenucl3.h"
