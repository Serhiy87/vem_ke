/*
	15.08.2015 - Kononenko - For vem - share UART1 for Modbus and E_METER
							Only ATmega128
	
	03.12.2015 - Kononenko - Add to USART1_UDRE_vect Modbus_TX ISR
*/

volatile uint8_t UART_Soft;	// определяет: что обрабатывает UART1 Modbus(1) или E_METER(0)
volatile uint8_t ModbusMode;
volatile uint8_t GSM_DebugMode;

uint8_t TD_UART_Soft;

volatile uint8_t COMM_LED;
volatile uint8_t GSM_LED;

extern void EMeter_ReInit(void);

#include "modbus.h"

//---- GSM
#define UDR_GSM UDR0

#define UCSR_GSM_A UCSR0A
#define U2X_GSM		U2X0
#define MPCM_GSM	MPCM0

#define UCSR_GSM_B UCSR0B
#define RXCIE_GSM		RXCIE0
#define TXCIE_GSM		TXCIE0
#define UDRIE_GSM		UDRIE0
#define RXEN_GSM		RXEN0
#define TXEN_GSM		TXEN0
#define UCSZ_GSM_2	UCSZ02

#define UCSR_GSM_C UCSR0C
#define UMSEL_GSM		UMSEL0
#define UPM_GSM_1		UPM01
#define UPM_GSM_0		UPM00
#define USBS_GSM		USBS0
#define UCSZ_GSM_1	UCSZ01
#define UCSZ_GSM_0	UCSZ00
#define UCPOL_GSM		UCPOL0

#define UBRR_GSM_H UBRR0H
#define UBRR_GSM_L UBRR0L

//---- E_METER
#define UDR_EMETER 		UDR1

#define UCSR_EMETER_A 	UCSR1A
#define TXC_EMETER		TXC1
#define U2X_EMETER		U2X1
#define MPCM_EMETER		MPCM1

#define UCSR_EMETER_B 	UCSR1B
#define RXCIE_EMETER	RXCIE1
#define TXCIE_EMETER	TXCIE1
#define UDRIE_EMETER	UDRIE1
#define RXEN_EMETER		RXEN1
#define TXEN_EMETER		TXEN1
#define UCSZ_EMETER_2	UCSZ12

#define UCSR_EMETER_C 	UCSR1C
#define UMSEL_EMETER	UMSEL1
#define UPM_EMETER_1	UPM11
#define UPM_EMETER_0	UPM10
#define USBS_EMETER		USBS1
#define UCSZ_EMETER_1	UCSZ11
#define UCSZ_EMETER_0	UCSZ10
#define UCPOL_EMETER	UCPOL1

#define UBRR_EMETER_H 	UBRR1H
#define UBRR_EMETER_L 	UBRR1L	

void EMeter_Init(void);
void EMeter_Cycle(void);
void EMeter_DRE(void);
void EMeter_RX(void);
void EMeter_TX(void);
inline void GetByteFromEM_FIFO_Transp(void);

// ~~~~~~~~~~~~~

#include GSM

// ~~~~~~~~~~~~~
void
USART_Init(void)
{	
	TD_UART_Soft = Timer8SysAlloc(1);

	MB_Init();
	EMeter_Init();

	//set PORTA.0 input with pull'up
	DDRA &= ~(1<<PA0);
	PORTA |= (1<<PA0);
	_delay_us(10);
	
	if(!(PINA & (1<<PA0))){
		MB_Init();
		UART_Soft = 1;
		ModbusMode = 1;
	}
	else{
		EMeter_ReInit();
		UART_Soft = 0;
		ModbusMode = 0;
	}
	
	GSM_Init();
}

// ~~~~~~~~~~~~~~
void
USART_Cycle(void)
{		
	//переключение UART_Soft задемпфировано на 100мс
	if(UART_Soft){

		if(ModbusMode) MB_Cycle();

		if(!(PINA & (1<<PA0)))	StartTimer8(TD_UART_Soft,10);
		else{
			if(Timer8Stopp(TD_UART_Soft)){
				cli();
				EMeter_ReInit();
				UART_Soft = 0;
				ModbusMode = 0;				
				GSM_DebugMode = 0;	
				sei();
			}
		}
	}
	else{
	//	EMeter_Cycle();
		if(!(PINA & (1<<PA0))){
			if(Timer8Stopp(TD_UART_Soft)){
				if(Transparent) GSM_Transparent2Modbus();	// если Modbus - то выходим из Transparent
				cli();
				MB_Init();
				UART_Soft = 1;
				ModbusMode = 1;
				sei();
			}
		}			
		else StartTimer8(TD_UART_Soft,10);
	}

	#ifdef GSM
		GSM_Cycle();
	#endif
}

// ~~~~~~~~~~
// USART0 - PORT0

ISR(USART0_UDRE_vect){
	GSM_DRE();
	GSM_LED = 1;
}
ISR(USART0_TX_vect){
	GSM_TX();
	GSM_LED = 1;
}
ISR(USART0_RX_vect){
	GSM_RX();
	GSM_LED = 1;
}

// ~~~~~~~~~~
// USART1 - PORT1

ISR(USART1_UDRE_vect){
	if(ModbusMode) Modbus_TX(&UDR1);
	else EMeter_DRE();
	COMM_LED = 1;
}
ISR(USART1_TX_vect){
	if(ModbusMode) Modbus_TX(&UDR1);
	else EMeter_TX();
	COMM_LED = 1;
}
ISR(USART1_RX_vect){
	if(ModbusMode) Modbus_RX(&UDR1);
	else EMeter_RX();
	COMM_LED = 1;
}	
//Modbus_ISR(1)



