/*~~~~~~~~~~~~~
	22.12.2011 - Kononenko - Add GSM hardware turn on, restart using PWRKEY
// ~~~~~~~~~~~~~~~~~~~~*/

#define LED_Miss

#define LCDXSz 40

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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define DI_Reg 1

#define SetDI(Port, Pin, Inversion) {&DDR##Port, &PORT##Port, &PIN##Port, 1<<P##Port##Pin, Inversion}

//#define DI_Map_Own {SetDI(G, 3, 1),	SetDI(G, 4, 1), SetDI(B, 4, 1),	SetDI(B, 5, 1),	SetDI(B, 6, 0)}
#define DI_Map_Own {SetDI(C, 1, 1),	SetDI(C, 0, 1),	SetDI(A, 0, 0),	SetDI(A, 1, 0) }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define DO_Reg 1

#define SetDO(Port, Pin) {&DDR##Port, &PORT##Port, 1<<P##Port##Pin}

#define DO_Map_Own {SetDO(F, 1), SetDO(F, 0), SetDO(F, 2) }

// ~~~~~~~~~~~~~~~~~
#define R_PLC_Type 10

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define TerReg 1

//~~~~~~~~~~~~~
void OW_MasterPullLowBus(uint8_t CH){
	switch(CH){
//		case 0: DDRF |= (1<<PF2);
//				PORTF &= ~(1<<PF2);
		case 0: DDRG |= (1<<PG0);
				PORTG &= ~(1<<PG0);
				break;
	}
}
//~~~~~~~~~~~~~
void OW_MasterReleaseBus(uint8_t CH){
	switch(CH){
//		case 0: DDRF &= ~(1<<PF2);
//				PORTF &= ~(1<<PF2);
		case 0: DDRG &= ~(1<<PG0);
				PORTG &= ~(1<<PG0);
				break;
	}
}
//~~~~~~~~~~~~~
uint8_t OW_MasterReadBus(uint8_t CH){
	switch(CH){
		//default: return (PINF & (1<<PF2));
		default: return (PING & (1<<PG0));
	}
}
//~~~~~~~~~~~~~

//#define TerIdx 5
#define TerInput OW_Temperature
#define InitThermFami {ThermDS18B20}
#define InitThermCorr {{0, 1000}}
//#define Ter10ms 60
// ~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Add hardware turn on, 
void GSM_PWRCNTRL_Config(void){
	cli();
	//DDRD|=(1<<PD6);
	DDRE|=(1<<PE7);
	sei();
}
void GSM_PWRCNTRL_OFF(void){
	cli();
	//PORTD &= ~(1<<PD6);
	PORTE &= ~(1<<PE7);
	sei();
}
void GSM_PWRCNTRL_ON(void){
	cli();
	//PORTD|=(1<<PD6);
	PORTE|=(1<<PE7);
	sei();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// For electrometer.h

#define CTS

void CTS_Config(void){
	DDRD|=(1<<PD5);
}
void CTS_OFF(void){
	PORTD &= ~(1<<PD5);
}
void CTS_ON(void){
	PORTD|=(1<<PD5);
}

//----
#define RTS

void RTS_Config(void){
	DDRD &= ~(1<<PD6);
	PORTD |= (1<<PD6);	//pull'up
}
uint8_t RTS_Read(void){
	return (PIND & (1<<PD6));	
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~
