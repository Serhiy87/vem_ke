/*~~~~~~~~~~~~~
	22.12.2011 - Kononenko - Add GSM hardware turn on, restart using PWRKEY
	23.10.2013 - Kononenko -  Revised for new keypad.h
	26.12.2013 - Kononenko -  Revised
// ~~~~~~~~~~~~~~~~~~~~*/

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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MB_PLC_Init(void)
{
	// RS485
	DDRB |=(1<<PB1); PORTB &=~(1<<PB1);
	DDRC |=(1<<PC0); PORTC &=~(1<<PC0);
}

void RS485_0(void) {PORTB ^=(1<<PB1);}
void RS485_1(void) {PORTC ^=(1<<PC0);}

void MB_LED0(void) {}
void MB_LED1(void) {}

void MB_LED_Err_On_0(void)	{}
void MB_LED_Err_Off_0(void)	{}
void MB_LED_Err_On_1(void)	{}
void MB_LED_Err_Off_1(void)	{}

// ~~~~~~~~~~~~~~~
#define SPI_Init()
#define DAC_Out()
// ~~~~~~~~~~~~~~~
#define RTC_Ready()	1
#define RTC_GetTrueTime()
// ~~~~~~~~~~~~~~~
#define PWM_Init()
#define PWM_Out()

// ~~~~~~~~~~~~~~~~~~~~~~~
#define DI_Reg 2

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

#define DI_Map {{5, 4, 3, 2, 0, 1, 15, 14}, {13, 12, 11, 10, 8, 9, 7, 6}}

// ~~~~~~~~~~~~~~~~~~~~~
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

#define DO_Map {0, 1, 2, 3, 4, 5, 15, 15, 6, 7, 8, 9, 10, 11, 15, 15}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define AI_Reg 8
#define AI_Mux {ADC4, ADC5, ADC6, ADC7, ADC3, ADC1, ADC2, ADC0}

// ~~~~~~~~~~~~~~~
#define TWI_Init()
#define TWI_In()

// ~~~~~~~~~~~~~~~~~~~~
#define PWRKEY (1<<PB6)
#define DDR_PWRKEY DDRB

// ~~~~~~~~~~~~~~~~~
#define R_PLC_Type 2

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define TerReg 3
#define TerIdx 5
#define TerInput (AnalogInputF+TerIdx)
#define InitThermFami {Thermis, Thermis, Thermis}
#define InitThermCorr {{0, 1000}, {0, 1000}, {0, 1000}}
#define Ter10ms 60
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Add hardware turn on, restart using PWRKEY and STATUS pins
void GSM_PWRKEY_Config(void){
	cli();
	DDRB|=(1<<PB6);
	sei();
}
void GSM_PWRKEY_NotPressed(void){
	cli();
	PORTB &= ~(1<<PB6);
	sei();
}
void GSM_PWRKEY_Pressed(void){
	cli();
	PORTB|=(1<<PB6);
	sei();
}
//STATUS pin is not used
void GSM_STATUS_Config(void){
}
uint8_t GSM_Status(void){
	return 1;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
#define sysmenu "sysmenumi.h"
