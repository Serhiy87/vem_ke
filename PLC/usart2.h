//	11.04.2016 - Kononenko - Use Modbus DRE ISR. Пока нет поддержки Xmega!
// ~~~~~~~~~~~~~~~~
// GSM
// ~~~~~~~~~~~~~~~~
#define UDR_GSM UDR

#define UCSR_GSM_A UCSRA
#define RXC_GSM		RXC
#define TXC_GSM		TXC
#define UDRE_GSM	UDRE
#define FE_GSM		FE
#define DOR_GSM		DOR
#define PE_GSM		PE
#define U2X_GSM		U2X
#define MPCM_GSM	MPCM

#define UCSR_GSM_B UCSRB
#define RXCIE_GSM		RXCIE
#define TXCIE_GSM		TXCIE
#define UDRIE_GSM		UDRIE
#define RXEN_GSM		RXEN
#define TXEN_GSM		TXEN
#define UCSZ_GSM_2	UCSZ2

#define UCSR_GSM_C UCSRC
#define UMSEL_GSM		UMSEL
#define UPM_GSM_1		UPM1
#define UPM_GSM_0		UPM0
#define USBS_GSM		USBS
#define UCSZ_GSM_1	UCSZ1
#define UCSZ_GSM_0	UCSZ0
#define UCPOL_GSM		UCPOL

#define UBRR_GSM_H UBRRH
#define UBRR_GSM_L UBRRL

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// GSM or Modbus
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef SMS_Send
	#include "gsm2.h"
#elif defined GPRS_UDP_Server || defined GPRS_UDP_Client
	#include "gsm.h"
#else
	#include "modbus.h"
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined SMS_Send || defined GPRS_UDP_Server || defined GPRS_UDP_Client
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// GSM
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~
//ISR(USART_UDRE_vect)
ISR(USART_TXC_vect)
{
	GSM_TX();
}

// ~~~~~~~~~~~~~~~~
ISR(USART_RXC_vect)
{
	GSM_RX();
}

// ~~~~~~~~~~~~~
void
USART_Init(void)
{
	IntOff();
	GSM_Init();
	IntOn();
}

#define USART_Cycle() GSM_Cycle()

// ~~~~~~~~~~~~~~~~~~
#else
// ~~~~~~~~~~~~~~~~~~
// Modbus
// ~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~
ISR(USART_UDRE_vect){
	Modbus_TX(&UDR);
}
ISR(USART_TXC_vect){
	Modbus_TX(&UDR);
}
ISR(USART_RXC_vect){
	Modbus_RX(&UDR);
}

// ~~~~~~~~~~~~~
void
USART_Init(void)
{
	IntOff();
	MB_Init();
	IntOn();
}

#define USART_Cycle() MB_Cycle()

// ~~~~~~~~~~~~~~~~~~
#endif
// ~~~~~~~~~~~~~~~~~~
