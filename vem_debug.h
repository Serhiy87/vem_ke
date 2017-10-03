/*
	В режиме GSM_DebugMode=1 обмен транслируется на порт RS232/485 19200,8, E,1
*/

void StartDebug(void){
	GSM_DebugMode = 1;
	ModbusMode = 0;
	EM_InitFIFO();
	cli();
	PORTD |= (1<<PD4);	//RS485 Tx_on	
	UCSR_EMETER_A = (0<<U2X_EMETER) | (0<<MPCM_EMETER);
	UCSR_EMETER_B = (0<<RXCIE_EMETER) | (0<<TXCIE_EMETER) | (0<<UDRIE_EMETER) | (0<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);
	EM_SetUARTParam(5 + (1<<8) + (0<<10) + (3<<11)); //Параметры связи со счётчиком 19200,8,E,1
	sei();	
}

void StopDebug(void){
	GSM_DebugMode = 0;
	ModbusMode = 1;
	cli();
	MB_Init();
	sei();
/*	cli();
	EMeter_ReInit();
	sei();*/
}

//используется в ISR
void SendDebug(uint8_t Char){
	//---FIFO
	EM_RX_FIFO_Begin++;
	if(EM_RX_FIFO_Begin >= EM_RX_FIFO_SIZE) EM_RX_FIFO_Begin = 0;
	EM_RX_FIFO[EM_RX_FIFO_Begin] = Char;
	if(EM_RX_FIFO_Begin == EM_RX_FIFO_End){
		EM_RX_FIFOOverFlow = 1;
		EM_RX_FIFOMax = EM_RX_FIFO_SIZE;
	}
}
inline void SetDebug_DRE_ISR(void){
	// enable DRE interrupt
	UCSR_EMETER_B = (0<<RXCIE_EMETER) | (0<<TXCIE_EMETER) | (1<<UDRIE_EMETER) | (0<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);		
}

//используется в ISR
inline void Debug_DRE(void){

	if(EM_RX_FIFO_Begin != EM_RX_FIFO_End){
		EM_RX_FIFO_End++;
		if(EM_RX_FIFO_End >= EM_RX_FIFO_SIZE) EM_RX_FIFO_End = 0;
		UDR_EMETER = EM_RX_FIFO[EM_RX_FIFO_End];
	}
	else{
		// disable DRE interrupt
		UCSR_EMETER_B = (0<<RXCIE_EMETER) | (0<<TXCIE_EMETER) | (0<<UDRIE_EMETER) | (0<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);	

	}
}

