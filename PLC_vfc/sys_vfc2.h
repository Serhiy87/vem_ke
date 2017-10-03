/*~~~~~~~~~~~~~
	22.12.2011 - Kononenko - Add GSM hardware turn on, restart using PWRKEY
// ~~~~~~~~~~~~~~~~~~~~*/

#define LED_Miss

#define Port_LCD_RS PORTB
#define DDR_LCD_RS DDRB
#define LCD_RS (1<<PB3)

#define Port_LCD_RW PORTB
#define DDR_LCD_RW DDRB
#define LCD_RW (1<<PB2)

#define Port_LCD_E PORTB
#define DDR_LCD_E DDRB
#define LCD_E (1<<PB0)

#define Port_LCD_DB4 PORTE
#define Port_LCD_DB5 PORTE
#define Port_LCD_DB6 PORTE
#define Port_LCD_DB7 PORTE
#define DDR_LCD_DB4 DDRE
#define DDR_LCD_DB5 DDRE
#define DDR_LCD_DB6 DDRE
#define DDR_LCD_DB7 DDRE
#define DB4 (1<<PE7)
#define DB5 (1<<PE6)
#define DB6 (1<<PE3)
#define DB7 (1<<PE2)

#define PORT_BACKLIGHT	PORTD
#define DDR_BACKLIGHT	DDRD
#define BACKLIGHT		(1<<PD7)

#define TypeLCD 216

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define DDR_Key DDRA
#define PortKey PORTA
#define PIN_Key PINA

#define ROW {1<<PA1, 1<<PA2}
#define STR {1<<PA6, 1<<PA3, 1<<PA5}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define KEYS {NULL,					\
	StartKey,				StopKey,\
	MenuEnterRight,	MenuEscLeft,	\
	MenuUp, 				MenuDown\
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MB_PLC_Init(void)
{
	// RS485
	DDRD |=(1<<PD4); PORTD &=~(1<<PD4);
}

void RS485_0(void) {}
void RS485_1(void) {PORTD ^=(1<<PD4);}

void MB_LED0(void) {}
void MB_LED1(void) {}

void MB_LED_Err_On_0(void)	{}
void MB_LED_Err_Off_0(void)	{}
void MB_LED_Err_On_1(void)	{}
void MB_LED_Err_Off_1(void)	{}

// ~~~~~~~~~~~~~~~
#define SPI_Init()
#define DAC_Out()

// ~~~~~~~~~~~~~~
#define PWM_Reg 2

#ifndef PWM_PERIOD
	#define PWM_PERIOD 3198	//5êÃö
#endif

#include "pwm.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define DI_Reg 1

#define SetDI(Port, Pin, Inversion) {&DDR##Port, &PORT##Port, &PIN##Port, 1<<P##Port##Pin, Inversion}

#define DI_Map_Own {SetDI(G, 3, 1),	SetDI(G, 4, 1), SetDI(B, 4, 1),	SetDI(B, 5, 1),	SetDI(B, 6, 0)}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define DO_Reg 1

#define SetDO(Port, Pin) {&DDR##Port, &PORT##Port, 1<<P##Port##Pin}

#define DO_Map_Own {SetDO(C, 2), SetDO(G, 0), SetDO(G, 1), SetDO(C, 0), SetDO(C, 1) }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define AI_Reg 2
#define AI_Mux {ADC1, ADC5}

// ~~~~~~~~~~~~~~~~~~~~
#define PWRKEY (1<<PB6)
#define DDR_PWRKEY DDRB

// ~~~~~~~~~~~~~~~~~
#define R_PLC_Type 10

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define TerReg 3

//~~~~~~~~~~~~~
void OW_MasterPullLowBus(uint8_t CH){
	switch(CH){
		case 0: DDRF |= (1<<PF2);
				PORTF &= ~(1<<PF2);
				break;
		case 1: DDRF |= (1<<PF3);
				PORTF &= ~(1<<PF3);
				break;
		case 2: DDRF |= (1<<PF4);
				PORTF &= ~(1<<PF4);
				break;
	}
}
//~~~~~~~~~~~~~
void OW_MasterReleaseBus(uint8_t CH){
	switch(CH){
		case 0: DDRF &= ~(1<<PF2);
				PORTF &= ~(1<<PF2);
				break;
		case 1: DDRF &= ~(1<<PF3);
				PORTF &= ~(1<<PF3);
				break;
		case 2: DDRF &= ~(1<<PF4);
				PORTF &= ~(1<<PF4);
				break;
	}
}
//~~~~~~~~~~~~~
uint8_t OW_MasterReadBus(uint8_t CH){
	switch(CH){
		case 0: return (PINF & (1<<PF2));
		case 1: return (PINF & (1<<PF3));
		case 2: 
		default:
			return (PINF & (1<<PF4));
	}
}

//#define TerIdx 5
#define TerInput OW_Temperature
#define InitThermFami {ThermDS18B20, ThermDS18B20, ThermDS18B20}
#define InitThermCorr {{0, 1000}, {0, 1000}, {0, 1000}}
//#define Ter10ms 60
// ~~~~~~~~~~~~~~~

#define LM_Exist 0

#include "rtc_m14t56.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Add hardware turn on, 
void GSM_PWRCNTRL_Config(void){
	cli();
	DDRD|=(1<<PD6);
	sei();
}
void GSM_PWRCNTRL_OFF(void){
	cli();
	PORTD &= ~(1<<PD6);
	sei();
}
void GSM_PWRCNTRL_ON(void){
	cli();
	PORTD|=(1<<PD6);
	sei();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
#define sysmenu "sysmenuvfc.h"
