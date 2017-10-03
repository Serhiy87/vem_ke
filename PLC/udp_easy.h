#ifndef __UDP_EASY_H
#define __UDP_EASY_H

/* 
 File			: udp_easy.h

 Compiler		: gcc 4.3.3

 Author			: Svyatoslav Kononenko

 Date			: July 2012

 Hardware		: Vega-Module HW rev3.1, Xmega128A1, 32MHz, Tibbo EM203A

 Description	: Параметры связи с Tibbo: 38400,N,1
 				  

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Реализация очень ограниченная

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Полезные замечания при работе по системе EM203A

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
#define UDPOutMax	100
#define UDPInMax	50

uint8_t UDP_Data_Out[UDPOutMax];
uint8_t UDP_Data_In[UDPInMax]; 
uint8_t UDP_Data_Rx[UDPInMax];
uint16_t UDP_Data_Rx_Cnt;

volatile uint8_t UDP_Cnt_Out, UDP_Flg_Out, UDP_Cnt_In, UDP_Flg_In;		// флаги 

uint8_t TD_UDP_RxC;
typedef struct{
	uint8_t IP1;
	uint8_t IP2;
	uint8_t IP3;
	uint8_t IP4;
} IP_Addr;

IP_Addr UDP_Rx_IP;
uint16_t UDP_Rx_Port;

void UDP_Init();
void UDP_Send(IP_Addr *IP, uint16_t Port, uint8_t Quantity, uint8_t *Buf);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined (__AVR_ATxmega128A1__)


#define USART_UDP				USARTF0
#define USART_UDP_ISR_DRE_vect	USARTF0_DRE_vect 
#define USART_UDP_ISR_TXC_vect	USARTF0_TXC_vect 
#define USART_UDP_ISR_RXC_vect	USARTF0_RXC_vect

#define	TCP_BSCALE  0	// Деление частоты 1 для бод-генератора
//#define	TCP_BSEL  207	// Деление частоты 2 для бод-генератора // Скорость 9600 bps
#define	TCP_BSEL  51	// Деление частоты 2 для бод-генератора 38400 

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void UDP_Init(void){
	cli();

// Конфигурируем Tx для UART
	PORTF.OUT |= (1<<3);
	PORTF.DIR |= (1<<3);

	//38400 bps
	USART_UDP.BAUDCTRLA = TCP_BSEL % 256;
	USART_UDP.BAUDCTRLB = (TCP_BSCALE<<USART_BSCALE_gp) | TCP_BSEL/256;

	// Конфигурируем 8bit, 1 stop bit, none, UART
	USART_UDP.CTRLC =  USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | (0<<USART_SBMODE_bp) | USART_CHSIZE_8BIT_gc;

	// Включаем передатчик и приёмник
	USART_UDP.CTRLB =  (1<<USART_RXEN_bp) | (1<<USART_TXEN_bp) | (0<<USART_CLK2X_bp) | (0<<USART_MPCM_bp) | (0<<USART_TXB8_bp);
	
	// Разрешить прерывание по RxC
	USART_UDP.CTRLA = USART_DREINTLVL_OFF_gc | USART_TXCINTLVL_OFF_gc | USART_RXCINTLVL_HI_gc;

	sei();

	TD_UDP_RxC = Timer8SysAlloc(1);
}
//~~~~~~~~~~~~~~~~
void UDP_Send(IP_Addr *IP, uint16_t Port, uint8_t Quantity, uint8_t *Buf){
	
	if(!UDP_Flg_Out){
		// Send nothing if buffer more than permitted
		if( Quantity + 7 > UDPOutMax) return; 

		//Start Tx
		UDP_Data_Out[0] = IP->IP1; 	//
		UDP_Data_Out[1] = IP->IP2;	//
		UDP_Data_Out[2] = IP->IP3;	//
		UDP_Data_Out[3] = IP->IP4;	//
		UDP_Data_Out[4] = (uint8_t)(Port>>8);
		UDP_Data_Out[5] = (uint8_t)(Port & 0x00FF);
		UDP_Data_Out[6] = Quantity;
		for(uint8_t i = 7; i<Quantity+7; i++){
			UDP_Data_Out[i] = Buf[i-7];
		}
		UDP_Flg_Out = Quantity+7;
		//uint16_t Temp16 = CRC(rrb, MDBTCPIP_Data_Out, 5);
		//MDBTCPIP_Data_Out[5] = (uint8_t)(Temp16 & 0x00FF);
		//MDBTCPIP_Data_Out[6] = (uint8_t)(Temp16 >> 8);

		UDP_Cnt_Out = 1;
	
		cli();
		USART_UDP.CTRLA = USART_DREINTLVL_HI_gc | USART_TXCINTLVL_OFF_gc | USART_RXCINTLVL_HI_gc;	//	Разрешение прерывания только по пустому Tx буферу
		USART_UDP.DATA = UDP_Data_Out[0];
		sei();
	}
}
//~~~~~~~~~~~~~~~~
void UDP_RxProcess(void){
	uint8_t Temp8 = 0;
	cli();
	if(Timer8Stopp(TD_UDP_RxC) && UDP_Cnt_In){
		UDP_Flg_In = UDP_Cnt_In;
		UDP_Cnt_In = 0;
		Temp8 = UDP_Data_Rx[6];
		// Check quantity for valid range
		if(Temp8==0 || Temp8 > UDPInMax-7 || Temp8 != UDP_Flg_In - 7)
			UDP_Flg_In = 0;
		else{
			UDP_Rx_IP.IP1 = UDP_Data_Rx[0];
			UDP_Rx_IP.IP2 = UDP_Data_Rx[1];
			UDP_Rx_IP.IP3 = UDP_Data_Rx[2];
			UDP_Rx_IP.IP4 = UDP_Data_Rx[3];
			UDP_Rx_Port = ((uint16_t)UDP_Data_Rx[4]<<8) + UDP_Data_Rx[5];
			for(uint8_t i = 7; i<UDP_Flg_In; i++){
				UDP_Data_In[i-7] = UDP_Data_Rx[i];
			}
			UDP_Flg_In =UDP_Flg_In- 7;
			UDP_Data_Rx_Cnt++;	
		}
	}
	sei();
}
//~~~~~~~~~~~~~~~~
ISR(USART_UDP_ISR_DRE_vect){
	USART_UDP.DATA = UDP_Data_Out[UDP_Cnt_Out++];
	if(UDP_Cnt_Out>=UDP_Flg_Out){
		//перед началом отправки последнего символа запретить прерывания по DRE, разрешить по TxC
		USART_UDP.CTRLA = USART_DREINTLVL_OFF_gc | USART_TXCINTLVL_HI_gc | USART_RXCINTLVL_HI_gc;		
	}
}
//~~~~~~~~~~~~~~~~
//после отправки последнего символа - запретить прерывания по Tx,  разрешить по RxC
ISR(USART_UDP_ISR_TXC_vect){
	USART_UDP.CTRLA = USART_DREINTLVL_OFF_gc | USART_TXCINTLVL_OFF_gc | USART_RXCINTLVL_HI_gc;
	UDP_Flg_Out = 0;
	UDP_Cnt_Out = 0;
}
//~~~~~~~~~~~~~~~~
ISR(USART_UDP_ISR_RXC_vect){
/*	
	if(UGT_BB66_Cnt_In>=UGTBB66InMax-1){
		USART_UGTBB66.CTRLA = USART_DREINTLVL_OFF_gc | USART_TXCINTLVL_OFF_gc | USART_RXCINTLVL_OFF_gc;
		UGT_BB66_Data_In[UGTBB66InMax-1] = USART_UGTBB66.DATA;
		UGT_BB66_Flg_In = 1;
		PORTB.OUTSET =(1<<5);
	}
	else{
		UGT_BB66_Data_In[UGT_BB66_Cnt_In++] = USART_UGTBB66.DATA;		
	}*/

	if(UDP_Cnt_In >= UDPInMax){
		UDP_Cnt_In = 0;
		USART_UDP.DATA;
	}
	else{
		UDP_Data_Rx[UDP_Cnt_In++] =	USART_UDP.DATA;
		StartTimer8(TD_UDP_RxC,2);
	}
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif defined (__AVR_ATmega128__)


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void UDP_Init(void){
	cli();

	// Конфигурируем pin управления Ethernet модулем
	// Reset active high
	DDRE|=(1<<PE2);
	PORTE |=(1<<PE2);

	UCSR_UDP_A = (0<<U2X_UDP) | (0<<MPCM_UDP);
	// Включаем передатчик и приёмник, Разрешить прерывание по RxC
	UCSR_UDP_B =  (1<<RXCIE_UDP) | (0<<TXCIE_UDP) | (0<<UDRIE_UDP) | (1<<RXEN_UDP) | (0<<UCSZ_UDP_2) | (1<<TXEN_UDP);
	// Конфигурируем 8bit, 1 stop bit, none, UART
	UCSR_UDP_C = (0<<UMSEL_UDP) | (0<<UPM_UDP_1) | (0<<UPM_UDP_0) | (0<<USBS_UDP) | (1<<UCSZ_UDP_1) | (1<<UCSZ_UDP_0) | (0<<UCPOL_UDP);
	UBRR_UDP_H = 0x00;
	UBRR_UDP_L = 0x19;	// 38400

	Delay_ms(100);

	// Конфигурируем pin управления Ethernet модулем
	// Reset active high
	PORTE &=~(1<<PE2);
	// Pins VEGA_READY and EM_READY are not used 
	
	sei();

	TD_UDP_RxC = Timer8SysAlloc(1);
}
//~~~~~~~~~~~~~~~~
void UDP_Send(IP_Addr *IP, uint16_t Port, uint8_t Quantity, uint8_t *Buf){
	
	if(!UDP_Flg_Out){
		// Send nothing if buffer more than permitted
		if( Quantity + 7 > UDPOutMax) return; 

		//Start Tx
		UDP_Data_Out[0] = IP->IP1; 	//
		UDP_Data_Out[1] = IP->IP2;	//
		UDP_Data_Out[2] = IP->IP3;	//
		UDP_Data_Out[3] = IP->IP4;	//
		UDP_Data_Out[4] = (uint8_t)(Port>>8);
		UDP_Data_Out[5] = (uint8_t)(Port & 0x00FF);
		UDP_Data_Out[6] = Quantity;
		for(uint8_t i = 7; i<Quantity+7; i++){
			UDP_Data_Out[i] = Buf[i-7];
		}
		UDP_Flg_Out = Quantity+7;
		UDP_Cnt_Out = 1;
	
		cli();
		UCSR_UDP_B |= (1<<UDRIE_UDP);	//	Разрешение прерывания только по пустому Tx буферу
		UDR_UDP = UDP_Data_Out[0];
		sei();
	}
}
//~~~~~~~~~~~~~~~~
void UDP_RxProcess(void){
	uint8_t Temp8 = 0;
	cli();
	if(Timer8Stopp(TD_UDP_RxC) && UDP_Cnt_In){
		UDP_Flg_In = UDP_Cnt_In;
		UDP_Cnt_In = 0;
		Temp8 = UDP_Data_Rx[6];
		// Check quantity for valid range
		if(Temp8==0 || Temp8 > UDPInMax-7 || Temp8 != UDP_Flg_In - 7)
			UDP_Flg_In = 0;
		else{
			UDP_Rx_IP.IP1 = UDP_Data_Rx[0];
			UDP_Rx_IP.IP2 = UDP_Data_Rx[1];
			UDP_Rx_IP.IP3 = UDP_Data_Rx[2];
			UDP_Rx_IP.IP4 = UDP_Data_Rx[3];
			UDP_Rx_Port = ((uint16_t)UDP_Data_Rx[4]<<8) + UDP_Data_Rx[5];
			for(uint8_t i = 7; i<UDP_Flg_In; i++){
				UDP_Data_In[i-7] = UDP_Data_Rx[i];
			}
			UDP_Flg_In =UDP_Flg_In- 7;
			UDP_Data_Rx_Cnt++;	
		}
	}
	sei();
}
//~~~~~~~~~~~~~~~~
void UDP_UDRE(void){
	UDR_UDP = UDP_Data_Out[UDP_Cnt_Out++];
	if(UDP_Cnt_Out>=UDP_Flg_Out){
		//перед началом отправки последнего символа запретить прерывания по DRE, разрешить по TxC
		UCSR_UDP_B &= ~(1<<UDRIE_UDP);
		UCSR_UDP_B |= (1<<TXCIE_UDP);
	}
}
//~~~~~~~~~~~~~~~~
//после отправки последнего символа - запретить прерывания по Tx,  разрешить по RxC
void UDP_TX(void){
	UCSR_UDP_B &= ~(1<<TXCIE_UDP);
	UDP_Flg_Out = 0;
	UDP_Cnt_Out = 0;
}
//~~~~~~~~~~~~~~~~
void UDP_RX(void){
	if(UDP_Cnt_In >= UDPInMax){
		UDP_Cnt_In = 0;
		UDR_UDP;
	}
	else{
		UDP_Data_Rx[UDP_Cnt_In++] =	UDR_UDP;
		StartTimer8(TD_UDP_RxC,2);
	}
}


#endif
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~
#endif /* !__UDP_EASY_H */
