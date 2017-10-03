/*
	20.08.2012 - Kononenko add UDP_EASY
	Комбинации по 2 usart
	1) UDP_EASY + Modbus
	2) GSM + Modbus
	3) UDP_EASY + GSM
	4) Modbus + Modbus

	19.10.2012 - Kononenko For __AVR_ATmega1280__ (USART1)Port1 - only GSM
	18.12.2012 - Kononenko add USART0_UDRE_vect for GSM
	04.11.2013 - Kononenko For __AVR_ATmega1281__, __AVR_ATmega2561__
	20.12.2013 - Kononenko Revised for vmd2_3
	10.03.2017 - Kononenko -  Add Modbus 3 port
*/
#if Modbus_Qt==3 && defined GSM
	#error Modbus3 and GSM shared the same USART!
#endif


#include "modbus.h"

#ifdef GSM
	#if defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
		#ifdef vmd2_3
			#define UDR_GSM UDR2

			#define UCSR_GSM_A UCSR2A
			#define U2X_GSM		U2X2
			#define MPCM_GSM	MPCM2

			#define UCSR_GSM_B UCSR2B
			#define RXCIE_GSM		RXCIE2
			#define TXCIE_GSM		TXCIE2
			#define UDRIE_GSM		UDRIE2
			#define RXEN_GSM		RXEN2
			#define TXEN_GSM		TXEN2
			#define UCSZ_GSM_2	UCSZ22

			#define UCSR_GSM_C UCSR2C
			#define UMSEL_GSM_1		UMSEL21
			#define UMSEL_GSM_0		UMSEL20
			#define UPM_GSM_1		UPM21
			#define UPM_GSM_0		UPM20
			#define USBS_GSM		USBS2
			#define UCSZ_GSM_1	UCSZ21
			#define UCSZ_GSM_0	UCSZ20
			#define UCPOL_GSM		UCPOL2

			#define UBRR_GSM_H UBRR2H
			#define UBRR_GSM_L UBRR2L
		#else
			#define UDR_GSM UDR1

			#define UCSR_GSM_A UCSR1A
			#define U2X_GSM		U2X1
			#define MPCM_GSM	MPCM1

			#define UCSR_GSM_B UCSR1B
			#define RXCIE_GSM		RXCIE1
			#define TXCIE_GSM		TXCIE1
			#define UDRIE_GSM		UDRIE1
			#define RXEN_GSM		RXEN1
			#define TXEN_GSM		TXEN1
			#define UCSZ_GSM_2	UCSZ12

			#define UCSR_GSM_C UCSR1C
			#define UMSEL_GSM_1		UMSEL11
			#define UMSEL_GSM_0		UMSEL10
			#define UPM_GSM_1		UPM11
			#define UPM_GSM_0		UPM10
			#define USBS_GSM		USBS1
			#define UCSZ_GSM_1	UCSZ11
			#define UCSZ_GSM_0	UCSZ10
			#define UCPOL_GSM		UCPOL1

			#define UBRR_GSM_H UBRR1H
			#define UBRR_GSM_L UBRR1L
		#endif
	#elif defined (__AVR_ATmega1281__) || defined (__AVR_ATmega2561__)
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
		#define UMSEL_GSM_1		UMSEL01
		#define UMSEL_GSM_0		UMSEL00
		#define UPM_GSM_1		UPM01
		#define UPM_GSM_0		UPM00
		#define USBS_GSM		USBS0
		#define UCSZ_GSM_1	UCSZ01
		#define UCSZ_GSM_0	UCSZ00
		#define UCPOL_GSM		UCPOL0

		#define UBRR_GSM_H UBRR0H
		#define UBRR_GSM_L UBRR0L	
	#else 
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
	#endif
	#include GSM
#endif

#ifdef UDP_EASY
	#define UDR_UDP UDR1

	#define UCSR_UDP_A UCSR1A
	#define U2X_UDP		U2X1
	#define MPCM_UDP	MPCM1

	#define UCSR_UDP_B UCSR1B
	#define RXCIE_UDP		RXCIE1
	#define TXCIE_UDP		TXCIE1
	#define UDRIE_UDP		UDRIE1
	#define RXEN_UDP		RXEN1
	#define TXEN_UDP		TXEN1
	#define UCSZ_UDP_2	UCSZ12

	#define UCSR_UDP_C UCSR0C
	#define UMSEL_UDP		UMSEL1
	#define UPM_UDP_1		UPM11
	#define UPM_UDP_0		UPM10
	#define USBS_UDP		USBS1
	#define UCSZ_UDP_1	UCSZ11
	#define UCSZ_UDP_0	UCSZ10
	#define UCPOL_UDP		UCPOL1

	#define UBRR_UDP_H UBRR1H
	#define UBRR_UDP_L UBRR1L

	#include "udp_easy.h"
#endif

// ~~~~~~~~~~~~~
void
USART_Init(void)
{
	MB_Init();
	#ifdef GSM
		GSM_Init();
	#endif
	#ifdef UDP_EASY
		UDP_Init();
	#endif
}

// ~~~~~~~~~~~~~~
void
USART_Cycle(void)
{
	MB_Cycle();
	#ifdef GSM
		GSM_Cycle();
	#endif
	#ifdef UDP_EASY
		UDP_RxProcess();
	#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega2561__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// ~~~~~~~~~~
	// USART0 - PORT0
	#ifdef GSM
		#ifndef SIM300DZ
			ISR(USART0_UDRE_vect) {GSM_DRE();}
		#endif
		ISR(USART0_TX_vect) {GSM_TX();}
		ISR(USART0_RX_vect) {GSM_RX();}
	#else
		Modbus_ISR(0)
	#endif

	// ~~~~~~~~~~
	// USART1 - PORT1
	#ifdef UDP_EASY
		ISR(USART1_UDRE_vect) {UDP_UDRE();}
		ISR(USART1_TX_vect) {UDP_TX();}
		ISR(USART1_RX_vect) {UDP_RX();}
	#else
		Modbus_ISR(1)
	#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	#ifdef vmd2_3
		// ~~~~~~~~~~
		// USART0 - PORT0
		Modbus_ISR(0)

		// ~~~~~~~~~~
		// USART1 - PORT1
		Modbus_ISR(1)

		// USART2 - PORT2 (only GSM)
		#ifdef GSM
			#ifndef SIM300DZ
				ISR(USART2_UDRE_vect) {GSM_DRE();}
			#endif
			ISR(USART2_TX_vect) {GSM_TX();}
			ISR(USART2_RX_vect) {GSM_RX();}
		#else
			// USART1 - PORT2
			Modbus_ISR(2)			
		#endif
	#else
		// ~~~~~~~~~~
		// USART0 - PORT0
		Modbus_ISR(0)

		// ~~~~~~~~~~
		// USART2 - PORT2
		Modbus_ISR(2)

		// USART1 - PORT1 (only GSM)
		#ifdef GSM
			#ifndef SIM300DZ
				ISR(USART1_UDRE_vect) {GSM_DRE();}
			#endif
			ISR(USART1_TX_vect) {GSM_TX();}
			ISR(USART1_RX_vect) {GSM_RX();}
		#endif
	#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif defined (__AVR_ATxmega128A1__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// ~~~~~~~~~~~~~
	// USART0
	Modbus_ISR(D, 0)

	// ~~~~~~~~~~~~~
	// USART1
	Modbus_ISR(D, 1)

	// ~~~
#endif
// ~~~

