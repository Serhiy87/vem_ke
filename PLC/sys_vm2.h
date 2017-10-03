/*~~~~~~~~~~~~~
	01.04.2011 - Kononenko - Add hardware turn on, restart using PWRKEY and STATUS pins for vmd HWrev2
	19.08.2011 - Sulima - Add enumerator "KEY_ENUM"
	02.04.2012 - Kononenko -  Revised for vmd2_1 (DI_INVERSION, ADC_Ref_Chan 8)
	07.10.2013 - Kononenko -  Revised for vmd2_2 (PORT_BACKLIGHT, DDR_BACKLIGHT, BACKLIGHT)
	19.12.2013 - Kononenko -  Revised for vmd2_3 (PortRW, DDR_RW, RW)
	13.10.2014 - Kononenko -  for vmd2_3 inverse logic for GSM_PWRCNTRL
	15.09.2015 - Kononenko -  GSM_CTS, GSM_RTS
	10.03.2017 - Kononenko -  Add Modbus 3 port
// ~~~~~~~~~~~~~~~~~~~~*/

#ifdef vmd2_3
	#define PinLED (1<<PF3)
	#define DDR_LED DDRF
	#define PortLED PORTF

	#define Port_LCD_RS PORTC
	#define DDR_LCD_RS DDRC
	#define LCD_RS (1<<PC4)

	#define Port_LCD_RW PORTC
	#define DDR_LCD_RW DDRC
	#define LCD_RW (1<<PC3)

	#define Port_LCD_E PORTC
	#define DDR_LCD_E DDRC
	#define LCD_E (1<<PC2)

	#define Port_LCD_DB0 PORTC
	#define Port_LCD_DB1 PORTC
	#define Port_LCD_DB2 PORTG
	#define Port_LCD_DB3 PORTG
	#define Port_LCD_DB4 PORTD
	#define Port_LCD_DB5 PORTD
	#define Port_LCD_DB6 PORTD
	#define Port_LCD_DB7 PORTL

	#define DDR_LCD_DB0 DDRC
	#define DDR_LCD_DB1 DDRC
	#define DDR_LCD_DB2 DDRG
	#define DDR_LCD_DB3 DDRG
	#define DDR_LCD_DB4 DDRD
	#define DDR_LCD_DB5 DDRD
	#define DDR_LCD_DB6 DDRD
	#define DDR_LCD_DB7 DDRL

	#define DB0 (1<<PC1)
	#define DB1 (1<<PC0)
	#define DB2 (1<<PG1)
	#define DB3 (1<<PG0)
	#define DB4 (1<<PD7)
	#define DB5 (1<<PD6)
	#define DB6 (1<<PD5)
	#define DB7 (1<<PL7)

	#define PORT_BACKLIGHT	PORTL
	#define DDR_BACKLIGHT	DDRL
	#define BACKLIGHT		(1<<PL6)

#elif defined vmd2_2
	#define PinLED (1<<PC2)
	#define DDR_LED DDRC
	#define PortLED PORTC

	#define Port_LCD_RS PORTF
	#define DDR_LCD_RS DDRF
	#define LCD_RS (1<<PF5)

	#define Port_LCD_E PORTF
	#define DDR_LCD_E DDRF
	#define LCD_E (1<<PF6)

	#define Port_LCD_DB4 PORTA
	#define Port_LCD_DB5 PORTF
	#define Port_LCD_DB6 PORTA
	#define Port_LCD_DB7 PORTA
	#define DDR_LCD_DB4 DDRA
	#define DDR_LCD_DB5 DDRF
	#define DDR_LCD_DB6 DDRA
	#define DDR_LCD_DB7 DDRA
	#define DB4 (1<<PA0)
	#define DB5 (1<<PF7)
	#define DB6 (1<<PA2)
	#define DB7 (1<<PA1)

	#define BACKLIGHT_INVERSION
	#define PORT_BACKLIGHT	PORTB
	#define DDR_BACKLIGHT	DDRB
	#define BACKLIGHT		(1<<PB6)

#else
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

	#ifdef vmd2_1
		#define BACKLIGHT_INVERSION
		#define PORT_BACKLIGHT	PORTG
		#define DDR_BACKLIGHT	DDRG
		#define BACKLIGHT		(1<<PG0)
	#endif
#endif

#define TypeLCD 420

#ifdef vmd2_3
	#define Timer_LCD 4
#else
	#define Timer_LCD 3
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifdef vmd2_3
	#define PortKeySTR1 PORTF
	#define PortKeySTR2 PORTF
	#define PortKeySTR3 PORTF
	#define PortKeyROW1 PORTF
	#define PortKeyROW2 PORTF
	#define DDR_KeySTR1 DDRF
	#define DDR_KeySTR2 DDRF
	#define DDR_KeySTR3 DDRF
	#define DDR_KeyROW1 DDRF
	#define DDR_KeyROW2 DDRF
	#define PIN_KeySTR1 PINF
	#define PIN_KeySTR2 PINF
	#define PIN_KeySTR3 PINF
	#define KeySTR1 (1<<PF5)
	#define KeySTR2 (1<<PF4)
	#define KeySTR3 (1<<PF2)
	#define KeyROW1 (1<<PF1)
	#define KeyROW2 (1<<PF0)
#elif defined vmd2_2
	#define PortKeySTR1 PORTC
	#define PortKeySTR2 PORTC
	#define PortKeySTR3 PORTC
	#define PortKeyROW1 PORTC
	#define PortKeyROW2 PORTG
	#define DDR_KeySTR1 DDRC
	#define DDR_KeySTR2 DDRC
	#define DDR_KeySTR3 DDRC
	#define DDR_KeyROW1 DDRC
	#define DDR_KeyROW2 DDRG
	#define PIN_KeySTR1 PINC
	#define PIN_KeySTR2 PINC
	#define PIN_KeySTR3 PINC
	#define KeySTR1 (1<<PC4)
	#define KeySTR2 (1<<PC3)
	#define KeySTR3 (1<<PC1)
	#define KeyROW1 (1<<PC0)
	#define KeyROW2 (1<<PG1)
#else
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
#endif

#define KEYS {NULL,					\
	StartKey,		StopKey,		\
	MenuEnterRight,	MenuEscLeft,	\
	MenuUp, 		MenuDown		\
}
enum KEY_ENUM{KEY_NONE, KEY_START, KEY_STOP, KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined vmd2_3
	void MB_PLC_Init(void)
	{
		// RS485
		DDRE |=(1<<PE2); PORTE &=~(1<<PE2);
		DDRD |=(1<<PD4); PORTD &=~(1<<PD4);
	#ifndef GSM
		DDRL |=(1<<PL5); PORTL &=~(1<<PL5);
	#endif
	}
	void RS485_0(void) {PORTE ^=(1<<PE2);}
	void RS485_1(void) {PORTD ^=(1<<PD4);}
	#ifndef GSM
		void RS485_2(void) {PORTL ^=(1<<PL5);}	
	#endif

#elif defined vmd2_2
	void MB_PLC_Init(void)
	{
		// RS485
		DDRE |=(1<<PE2); PORTE &=~(1<<PE2);
		DDRA |=(1<<PA4); PORTA &=~(1<<PA4);
	}
	void RS485_0(void) {PORTE ^=(1<<PE2);}
	void RS485_1(void) {PORTA ^=(1<<PA4);}
#else
	void MB_PLC_Init(void)
	{
		// RS485
		DDRE |=(1<<PE2); PORTE &=~(1<<PE2);
		DDRC |=(1<<PC0); PORTC &=~(1<<PC0);
	}
	void RS485_0(void) {PORTE ^=(1<<PE2);}
	void RS485_1(void) {PORTC ^=(1<<PC0);}
#endif

void MB_LED0(void) {}
void MB_LED1(void) {}
void MB_LED2(void) {}

void MB_LED_Err_On_0(void)	{}
void MB_LED_Err_Off_0(void)	{}
void MB_LED_Err_On_1(void)	{}
void MB_LED_Err_Off_1(void)	{}	
void MB_LED_Err_On_2(void)	{}
void MB_LED_Err_Off_2(void)	{}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef vmd2_3
	#define PortCS_ADC		PORTH
	#define DDR_CS_ADC		DDRH
	#define CS_ADC		(1<<PH3)

	#define PortRESET_PER PORTH
	#define DDR_RESET_PER DDRH
	#define RESET_PER	(1<<PH4)

	#define Port_INH		PORTE
	#define Port_INH2		PORTH
	#define Port_TER_A		PORTE
	#define Port_TER_B		PORTE
	#define DDR_INH 		DDRE
	#define DDR_INH2 		DDRH
	#define DDR_TER_A 		DDRE
	#define DDR_TER_B 		DDRE
	#define INH				(1<<PE7)
	#define INH2			(1<<PH2)
	#define TER_A			(1<<PE5)
	#define TER_B			(1<<PE6)

#elif defined vmd2_2
	#define PortCS_DAC_1	PORTC
	#define PortCS_DAC_2	PORTC
	#define PortCS_ADC		PORTA
	#define DDR_CS_DAC_1	DDRC
	#define DDR_CS_DAC_2	DDRC
	#define DDR_CS_ADC		DDRA
	#define CS_DAC_1	(1<<PC5)
	#define CS_DAC_2	(1<<PC6)
	#define CS_ADC		(1<<PA6)

	#define PortRESET_PER PORTA
	#define DDR_RESET_PER DDRA
	#define RESET_PER	(1<<PA3)

	#define Port_INH		PORTC
	#define Port_TER_A		PORTG
	#define Port_TER_B		PORTA
	#define DDR_INH 		DDRC
	#define DDR_TER_A 		DDRG
	#define DDR_TER_B 		DDRA
	#define INH				(1<<PC7)
	#define TER_A			(1<<PG2)
	#define TER_B			(1<<PA7)

#else
	#define PortCS_DAC_1	PORTB
	#define PortCS_DAC_2	PORTE
	#define PortCS_ADC		PORTB
	#define DDR_CS_DAC_1	DDRB
	#define DDR_CS_DAC_2	DDRE
	#define DDR_CS_ADC		DDRB
	#define CS_DAC_1	(1<<PB0)
	#define CS_DAC_2	(1<<PE7)
	#define CS_ADC		(1<<PB4)

	#define PortRESET_PER PORTE
	#define DDR_RESET_PER DDRE
	#define RESET_PER	(1<<PE6)

	#define Port_INH		PORTD
	#define Port_TER_A		PORTE
	#define Port_TER_B		PORTB
	#define DDR_INH 		DDRD
	#define DDR_TER_A 		DDRE
	#define DDR_TER_B 		DDRB
	#define INH			(1<<PD7)
	#define TER_A		(1<<PE5)
	#define TER_B		(1<<PB5)	
#endif

#ifdef ADC_Ref_Chan
	#define ADC_Reg 7
	#if defined vmd2_1
		#define ADC_Mux {0b111, 0b100, 0b110, 0b101, 0b001, 0b010, 0b011}
	#elif defined vmd2_2
		#define ADC_Mux {0b111, 0b110, 0b101, 0b100, 0b011, 0b010, 0b001}
	#elif defined vmd2_3
		#define ADC_Mux {0b000, 0b001, 0b010, 0b011, 0b100, 0b101, 0b110}
	#else
		#define ADC_Mux {0b111, 0b100, 0b110, 0b101, 0b001, 0b010, 0b000}
	#endif
#else
	#define ADC_Reg 6
	#if defined vmd2_1
		#define ADC_Mux {0b111, 0b100, 0b110, 0b101, 0b001, 0b010}
	#elif defined vmd2_2
		#define ADC_Mux {0b111, 0b110, 0b101, 0b100, 0b011, 0b010}
	#elif defined vmd2_3
		#define ADC_Mux {0b000, 0b001, 0b010, 0b011, 0b100, 0b101}
	#else
		#define ADC_Mux {0b111, 0b100, 0b110, 0b101, 0b001, 0b010}
	#endif
#endif


// By trial (Naumenko)
#if defined (vmd2_1) || defined (vmd2_2) || defined (vmd2_3)
	#define ADC_Ref 18000
#else
	#define ADC_Ref 20000
#endif
#define ADC_Ref_d 2000


// ~~~~~~~~~~~~~~
#ifndef vmd2_3
	#define DAC_Reg 4
	#define PWM_Init()
	#define PWM_Out()
#else
	#define DAC_Out()
	#define PWM_Reg 4
	//#define PWM_PERIOD_TC3 3198	//5к√ц
	#define PWM_PERIOD_TC5 3198	//5к√ц
	#include "pwm.h"
#endif



#include "spi.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifdef vmd2_3
	#define DI_Reg 3
	
	#define SetDI(Port, Pin, Inversion) {&DDR##Port, &PORT##Port, &PIN##Port, 1<<P##Port##Pin, Inversion}

	#define DI_Map_Own {																												\
		SetDI(K, 6, 0), SetDI(K, 7, 0), SetDI(J, 7, 0), SetDI(A, 0, 0), SetDI(A, 1, 0), SetDI(A, 2, 0), SetDI(A, 3, 0), SetDI(A, 4, 0),	\
		SetDI(A, 5, 0), SetDI(A, 6, 0), SetDI(A, 7, 0), SetDI(G, 2, 0), SetDI(J, 6, 0), SetDI(J, 5, 0), SetDI(J, 4, 0), SetDI(J, 3, 0),	\
		SetDI(J, 1, 1), SetDI(J, 2, 1)	\
	}	

#else
	#define DI_Reg 2

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
	#define Pin_OUT_DE1 PIND
	#define Pin_OUT_DE2 PIND
	#define DDR_OUT_DE1 DDRD
	#define DDR_OUT_DE2 DDRD
	#define OUT_DE1 (1<<PD5)
	#define OUT_DE2 (1<<PD4)

	#define DI_Map {{0, 1, 3, 2, 4, 5, 6, 7}, {8, 9, 11, 10, 12, 13, 14, 15}}
#endif

#if defined (vmd2_1) || defined (vmd2_2)
	#define DI_INVERSION
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef HIGH_SPEED_COUNTER
		#define HSC_Reg 2
		#include "hscounter.h"
#else
	#define HSC_Init()
	#define HSC_In()
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define DO_Reg 2

#ifdef vmd2_3
	#define PortStrobe 	PORTJ
	#define PortData 	PORTC
	#define PortEnable 	PORTC
	#define PortClock 	PORTC

	#define DDR_Strobe 	DDRJ
	#define DDR_Data 	DDRC
	#define DDR_Enable 	DDRC
	#define DDR_Clock 	DDRC

	#define Strobe		(1<<PJ0)
	#define Data		(1<<PC7)
	#define Enable		(1<<PC6)
	#define Clock		(1<<PC5)

	#define DO_Map {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}
#elif defined vmd2_2
	#define PortStrobe 	PORTF
	#define PortData 	PORTF
	#define PortEnable 	PORTF
	#define PortClock 	PORTF

	#define DDR_Strobe 	DDRF
	#define DDR_Data 	DDRF
	#define DDR_Enable 	DDRF
	#define DDR_Clock	DDRF

	#define Strobe		(1<<PF1)
	#define Data		(1<<PF2)
	#define Enable		(1<<PF3)
	#define Clock		(1<<PF4)

	#define DO_Map {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}
#else
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

	#define DO_Map {0, 1, 2, 3, 4, 5, 15, 15, 6, 7, 8, 9, 10, 11, 15, 15}
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define AI_Reg 8

#ifdef vmd2_3
	#define AI_Mux {ADC6, ADC7, ADC8, ADC9, ADC10, ADC11, ADC12, ADC13}
#elif defined vmd2_2
	#define PortAN_A PORTD
	#define PortAN_B PORTD
	#define PortAN_C PORTG
	#define DDR_AN_A DDRD
	#define DDR_AN_B DDRD
	#define DDR_AN_C DDRG
	#define AN_A (1<<PD6)
	#define AN_B (1<<PD7)
	#define AN_C (1<<PG0)

	#define AI_Mux {ADC0, ADC0, ADC0, ADC0, ADC0, ADC0, ADC0, ADC0}
	#define AI_Mux2 {0b111, 0b110, 0b101, 0b100, 0b011, 0b010, 0b001, 0b000}
#else
	#define PortAN_A PORTD
	#define PortAN_B PORTG
	#define PortAN_C PORTE
	#define DDR_AN_A DDRD
	#define DDR_AN_B DDRG
	#define DDR_AN_C DDRE
	#define AN_A (1<<PD6)
	#define AN_B (1<<PG1)
	#define AN_C (1<<PE4)

	#define AI_Mux {ADC2, ADC2, ADC2, ADC2, ADC2, ADC2, ADC2, ADC2}
	#define AI_Mux2 {0b101, 0b110, 0b111, 0b100, 0b010, 0b000, 0b001, 0b011}
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef vmd2_3
	#include "rtc_m14t56.h"	
#else
	#include "i2c.h"
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
#define PWRKEY (1<<PE3)	//Ќа схеме нет PWRKEY
#define DDR_PWRKEY DDRE
*/
// ~~~~~~~~~~~~~~~~~
#define R_PLC_Type 7

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define TerInput ADC_Input
#ifdef ADC_Ref_Chan
	#define TerReg (ADC_Reg-1)
#else
	#define TerReg ADC_Reg
#endif
#define InitThermFami {Thermis, Thermis, Thermis, Thermis, Thermis, Thermis}
#ifndef InitThermCorr
	#define InitThermCorr {{0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}}
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef vmd2_2
#ifndef vmd2_3
	// Add hardware turn on, restart using PWRKEY and STATUS pins
	void GSM_PWRKEY_Config(void){
		cli();
		DDRE|=(1<<PE3);
		sei();
	}
	void GSM_PWRKEY_NotPressed(void){
		cli();
		PORTE &= ~(1<<PE3);
		sei();
	}
	void GSM_PWRKEY_Pressed(void){
		cli();
		PORTE|=(1<<PE3);
		sei();
	}	
	void GSM_STATUS_Config(void){
		cli();
		DDRB&=~(1<<PB6);
		PORTB|=(1<<PB6);
		sei();
	}
	uint8_t GSM_Status(void){
		return !(PINB & (1<<PB6));
	}
#endif
#endif

// ƒл€ gsm_sim900r.h используетс€ только PWRCNTRL
#ifdef vmd2_3
	void GSM_PWRCNTRL_Config(void){
		cli();
		DDRL|=(1<<PL5);
		sei();
	}
	void GSM_PWRCNTRL_OFF(void){
		cli();
		PORTL|=(1<<PL5);
		sei();
	}
	void GSM_PWRCNTRL_ON(void){
		cli();
		PORTL &= ~(1<<PL5);
		sei();
	}

	void GSM_CTS_RTS_Config(void){
		cli();
		//—TS
		DDRB &=~(1<<PB6);
		PORTB |=(1<<PB6);	//pull'up
		//RTS
		DDRG |= (1<<PG5);
		PORTG &= ~(1<<PG5);
		sei();
	}
	uint8_t GSM_CTS_State(void){
		return PINB & (1<<PB6);
	}
	void GSM_RTS_OFF(void){
		cli();
		PORTG &= ~(1<<PG5);
		sei();
	}
	void GSM_RTS_ON(void){
		cli();
		PORTG |= (1<<PG5);
		sei();
	}

#else
	void GSM_PWRCNTRL_Config(void){
		cli();
		DDRE|=(1<<PE2);
		sei();
	}
	void GSM_PWRCNTRL_OFF(void){
		cli();
		PORTE &= ~(1<<PE2);
		sei();
	}
	void GSM_PWRCNTRL_ON(void){
		cli();
		PORTE|=(1<<PE2);
		sei();
	}
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef vmd2_3
	#define PORT_CS_DATAFLASH	PORTB
	#define DDR_CS_DATAFLASH	DDRB
	#define CS_DATAFLASH		(1<<PB0)
#elif defined vmd2_2
	#define PORT_CS_DATAFLASH	PORTE
	#define DDR_CS_DATAFLASH	DDRE
	#define CS_DATAFLASH		(1<<PE5)
#else
	#define PORT_CS_DATAFLASH	PORTF
	#define DDR_CS_DATAFLASH	DDRF
	#define CS_DATAFLASH		(1<<PF3)
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define sysmenu "sysmenumo2.h"

