/*
	23.02.2015 - Kononenko - добавил тип ELVIN_MODEM 
				Устранена проблема подсчёта BCC для ELVIN (EM_BCC &= 0b01111111)
	
	24.04.2015 - Kononenko - устранена проблема многократного переполнения буфера EMeter_RxStr (LANDIS_GYR_ZMR110 не работает)
				Два запроса(/?! + АСК U Z Y) на LANDIS_GYR_ZMR110 (на /?!+таймаут не все отвечают)
	
	27.04.2015 - Kononenko - используем define CTS, RTS
*/
typedef struct {
    uint32_t Ap;
    uint32_t Am;
  	uint32_t Rp;
    uint32_t Rm;
} E_METER_RAM;

volatile uint8_t EMeter_RxCharN, EMeter_TxCharN;
volatile uint8_t EMeter_TxSz;
volatile uint8_t EMeter_TxFlag;	//transmitting is now
char EMeter_TxStr[15];
#define  EM_RX_SIZE 255
char EMeter_RxStr[EM_RX_SIZE];

uint8_t TD_EMeter;

uint8_t EM_Mode;
uint8_t EM_Retry;

uint8_t EM_BCC;

uint8_t EMeterTypeRAM; // для анализа EMeterType в ISR (в ISR нельзя работать с EEPROM - т.к. основной цикл также работает с EEPROM!)

enum EMETERS{
	EM_NONE,
	LANDIS_GYR_ZMR110,
	ELVIN,
	MERCURY_230,
	ELVIN_MODEM
};
uint8_t EMeterType EEMEM = EM_NONE;

#ifdef E_METER

extern E_METER_RAM E_Meter_RAM;

//--- Landis&Gyr ZMR110
prog_char EM_RESP_1_8_0[] = "1.8.0";	//Суммарная активная энергия потребление (+ А), kВч Всего
prog_char EM_RESP_2_8_0[] = "2.8.0";	//Суммарная активная энергия отдача/генерация (- А), kВч –Всего
prog_char EM_RESP_3_8_0[] = "3.8.0";	//Суммарная реактивная энергия потребление (+R), кварч Всего 
prog_char EM_RESP_4_8_0[] = "4.8.0";	//Суммарная реактивная энергия генерация (-R), кварч - Всего
//---ЭЛВИН
prog_char EM_REQ[]	= "/?!";
prog_char EM_READ_DATA_REQ	[]	= {0x06,0x30,0x35,0x30, 0x00};	//АСК	U	Z	Y	CR	LF - Сообщение подтверждения/выбора опций
//---Меркурий 230
prog_uint8_t EM_OPEN_CH_REQ[]= {0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x77,0x81};	//Запрос на открытие канала связи
prog_uint8_t EM_ENERGY_REQ[]= {0x00,0x05,0x00,0x00,0x10,0x25};	//Запросы на чтение массивов регистров накопленной энергии
//---ЭЛВИН эмулятор модема
prog_char EM_RESP_AT[]		= "AT\r";
prog_char EM_RESP_ATZ[]		= "ATZ\r";
prog_char EM_RESP_ATE0[]	= "ATE0\r";
prog_char EM_RESP_ATH[]		= "+++ATH\r";
prog_char EM_RESP_ATA[]		= "ATA\r";

prog_uint8_t EM_RESP_OK[]		= "\r\nOK\r\n";
prog_uint8_t EM_RESP_RING[]		= "\r\nRING\r\n";
prog_uint8_t EM_RESP_CONNECT[]	= "\r\nCONNECT\r\n";
prog_uint8_t EM_RESP_NOCARRIER[]= "\r\nNO CARRIER\r\n";
prog_uint8_t EM_END_REQ[]	= {0x42,0x30,0x03,0x75};	//завершение обмена

void EMeter_ReInit(void);
void EMeter_Init(void);
void EMeter_SendFirstChar(void);
void EMeter_SendCR(prog_char *Str_P);
void EMeter_SendData_P(prog_uint8_t *Data_Out_P, uint8_t Size);
void EMeter_NoLink(void);
uint32_t EMeter_M230_Data_Convert(uint8_t Start);
uint32_t EMeter_ZMR110_Data_Convert(prog_char *Str_P);


void EMeter_Init(void){
	TD_EMeter = Timer16SysAlloc(1);
	//RS485
	MB_PLC_Init();
	EMeter_ReInit();
}
void EMeter_ReInit(void){
	
	#ifdef CTS
		CTS_Config();
		CTS_OFF();
	#endif
	#ifdef RTS
		RTS_Config();
	#endif

	if( erb(&EMeterType) == EM_NONE){
		
	}
	
	if(erb(&EMeterType) == LANDIS_GYR_ZMR110){
		//Параметры связи со счётчиком 9600, 7, E,1
		UCSR_EMETER_A = (0<<U2X_EMETER) | (0<<MPCM_EMETER);
		UCSR_EMETER_B = (1<<RXCIE_EMETER) | (0<<TXCIE_EMETER) | (0<<UDRIE_EMETER) | (1<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);
		UCSR_EMETER_C = (0<<UMSEL_EMETER) | (1<<UPM_EMETER_1) | (0<<UPM_EMETER_0) | (0<<USBS_EMETER) | (1<<UCSZ_EMETER_1) | (0<<UCSZ_EMETER_0) | (0<<UCPOL_EMETER);
		UBRR_EMETER_H = 0x00;
		UBRR_EMETER_L = 0x67;	// 9600
	}
	if( (erb(&EMeterType) == ELVIN) || (erb(&EMeterType) == MERCURY_230) || (erb(&EMeterType) == ELVIN_MODEM) ){
		//Параметры связи со счётчиком 9600, 8, N,1
		UCSR_EMETER_A = (0<<U2X_EMETER) | (0<<MPCM_EMETER);
		UCSR_EMETER_B = (1<<RXCIE_EMETER) | (0<<TXCIE_EMETER) | (0<<UDRIE_EMETER) | (1<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);
		UCSR_EMETER_C = (0<<UMSEL_EMETER) | (0<<UPM_EMETER_1) | (0<<UPM_EMETER_0) | (0<<USBS_EMETER) | (1<<UCSZ_EMETER_1) | (1<<UCSZ_EMETER_0) | (0<<UCPOL_EMETER);
		UBRR_EMETER_H = 0x00;
		UBRR_EMETER_L = 0x67;	// 9600
	}
	StartTimer16(TD_EMeter,1000);
	EM_Mode = 0;
}

// ~~~~~~~~~
void EMeter_RX(void){
	EMeter_RxStr[EMeter_RxCharN] = UDR_EMETER;
	if(EMeterTypeRAM == ELVIN_MODEM){
		EMeter_RxCharN++;
		if(EMeter_RxCharN >= EM_RX_SIZE) EMeter_RxCharN = 0; // Защита от переполнения для ELVIN_MODEM - на второй круг
	}
	else{
		if(EMeter_RxCharN < (EM_RX_SIZE-1) )EMeter_RxCharN++;// Защита от переполнения для остальных - стоп
	}
}
// ~~~~~~~~~
void EMeter_DRE(void){
	if(EMeter_TxCharN < EMeter_TxSz){
		UDR_EMETER = EMeter_TxStr[EMeter_TxCharN];
		EMeter_TxCharN++;
	}
	else{
		UCSR_EMETER_B =  (1<<RXCIE_EMETER) | (1<<TXCIE_EMETER) | (0<<UDRIE_EMETER) | (1<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);
	}
}
// ~~~~~~~~~
void EMeter_TX(void){
	UCSR_EMETER_B =  (1<<RXCIE_EMETER) | (0<<TXCIE_EMETER) | (0<<UDRIE_EMETER) | (1<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);
	EMeter_TxFlag = 0;
	RS485_1();	// Tx off
}

// ~~~~~~~~~
void EMeter_SendFirstChar(void){
	cli();
	EMeter_TxFlag = 1;
	RS485_1();	// Tx on
	UDR_EMETER = EMeter_TxStr[0];
	sei();

	EMeter_TxCharN = 1;

	cli();
	UCSR_EMETER_B =  (1<<RXCIE_EMETER) | (0<<TXCIE_EMETER) | (1<<UDRIE_EMETER) | (1<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);

	EMeter_RxCharN = 0;	//Очистить буфер приёма
	sei();	
}
// ~~~~~~~~~
//Инициирует передачу строки + CR
void EMeter_SendCR(prog_char *Str_P){
	EMeter_TxSz = strlen_P(Str_P);
	sprintf_P(EMeter_TxStr, Str_P);
	sprintf(EMeter_TxStr + EMeter_TxSz, "\r\n");
	EMeter_TxSz +=2;
	EMeter_SendFirstChar();
}
// ~~~~~~~~~
void EMeter_SendData_P(prog_uint8_t *Data_Out_P, uint8_t Size){
	EMeter_TxSz = Size;
	memcpy_P(EMeter_TxStr, Data_Out_P, Size);
	EMeter_SendFirstChar();
}
// ~~~~~~~~~
void EMeter_ClearRXStr(void){
	uint8_t i;
	for(i=0; i<EM_RX_SIZE; i++) EMeter_RxStr[i] = 0;
	EMeter_RxCharN = 0;

}
// ~~~~~~~~~
void EMeter_NoLink(void){
	if(EM_Retry>10){
		E_Meter_RAM.Ap = 0;
		E_Meter_RAM.Am = 0;
		E_Meter_RAM.Rp = 0;
		E_Meter_RAM.Rm = 0;
	}
	else EM_Retry++;

	StartTimer16(TD_EMeter,1000);
	EM_Mode = 0;
}
// ~~~~~~~~~
void EMeter_ELVIN_Modem_NoLink(void){
	E_Meter_RAM.Ap = 0;
	E_Meter_RAM.Am = 0;
	E_Meter_RAM.Rp = 0;
	E_Meter_RAM.Rm = 0;
}
// ~~~~~~~~~
uint32_t EMeter_M230_Data_Convert(uint8_t Start){
	uint8_t TempArray[4];
	TempArray[3] = EMeter_RxStr[Start+1];
	TempArray[2] = EMeter_RxStr[Start+0];
	TempArray[1] = EMeter_RxStr[Start+3];
	TempArray[0] = EMeter_RxStr[Start+2];
	if( *(uint32_t*)TempArray == 0xFFFFFFFF) return 0;
	else return *(uint32_t*)TempArray;
}
// ~~~~~~~~~
uint32_t EMeter_ZMR110_Data_Convert(prog_char *Str_P){
	uint32_t Result;
	char* temp_ptr = strstr_P(EMeter_RxStr, Str_P);
	if(temp_ptr != NULL){
		uint8_t Start = (uint8_t)(temp_ptr - EMeter_RxStr);
//		EM_BCC = Start;	//TODO for Debug
		if(Start<(EM_RX_SIZE-1)){
			EMeter_RxStr[Start+12]=	EMeter_RxStr[Start+13];	//Сдвиг десятичной точки
			EMeter_RxStr[Start+13]=	EMeter_RxStr[Start+14];
			EMeter_RxStr[Start+14]=	EMeter_RxStr[Start+15];
			EMeter_RxStr[Start+15]= ' ';
			sscanf(EMeter_RxStr+Start+6,"%ld",&Result);							
			if(EMeter_RxStr[Start+13]=='*') Result *=100;
			return Result;
		}
	}	
	return 0;
}
// ~~~~~~~~~
void EMeter_Cycle(void){

	static uint8_t RING_Retry;	// Для ELVIN_MODEM

	EMeterTypeRAM = erb(&EMeterType);

	if( erb(&EMeterType) == EM_NONE){
		EMeter_RxCharN = 0;
		EMeter_ELVIN_Modem_NoLink();
	}

	//--- Landis&Gyr ZMR110
	if( erb(&EMeterType) == LANDIS_GYR_ZMR110){
		switch(EM_Mode){

			case 1:	
				if(Timer16Stopp(TD_EMeter)){
					EMeter_SendCR(EM_READ_DATA_REQ);
					StartTimer16(TD_EMeter,1000);
					EM_Mode = 2;
				}
				break;
			case 2:
				 //read data
				if(Timer16Stopp(TD_EMeter)){
					
					// check frame
					if(EMeter_RxCharN < (EM_RX_SIZE-1)){
						EMeter_NoLink();
						break;
					}				
						
					EMeter_RxStr[EM_RX_SIZE-1] = '\0';
					
					E_Meter_RAM.Ap = EMeter_ZMR110_Data_Convert(EM_RESP_1_8_0);
					E_Meter_RAM.Am = EMeter_ZMR110_Data_Convert(EM_RESP_2_8_0);
					E_Meter_RAM.Rp = EMeter_ZMR110_Data_Convert(EM_RESP_3_8_0);
					E_Meter_RAM.Rm = EMeter_ZMR110_Data_Convert(EM_RESP_4_8_0);

					StartTimer16(TD_EMeter,1000);
					EM_Retry = 0;
					EM_Mode = 0;
				}
				break;
			
			default: 
				if(Timer16Stopp(TD_EMeter)){
					EMeter_SendCR(EM_REQ);
					StartTimer16(TD_EMeter,200);	//TODO уменшить ?
					EM_Mode = 1;
				}
				break;
		}		
	}

	//--- ЭЛВИН
	if(erb(&EMeterType) == ELVIN){
		switch(EM_Mode){
	
			case 1:	
				if(Timer16Stopp(TD_EMeter)){
					EMeter_SendCR(EM_READ_DATA_REQ);
					StartTimer16(TD_EMeter,100);
					EM_Mode = 2;
				}
				break;
			case 2:
				if(Timer16Stopp(TD_EMeter)){	
				
					 //read data

					// check frame
					if(EMeter_RxCharN != 40){
						EMeter_NoLink();
						break;
					}
					EM_BCC = 0;
					for(uint8_t i=1; i<39; i++) EM_BCC += EMeter_RxStr[i];
					EM_BCC &= 0b01111111;
					if(EM_BCC != EMeter_RxStr[39]){
						EMeter_NoLink();
						break;
					}
					if( EMeter_RxStr[0]!=0x02 || EMeter_RxStr[1]!='(' || EMeter_RxStr[34] != ')' || EMeter_RxStr[35] != '!' ||
						EMeter_RxStr[36] != '\r' || EMeter_RxStr[37] != '\n'|| EMeter_RxStr[38] != 0x03){
						EMeter_NoLink();
						break;
					}
					//process data
					char TempStr[11];
					TempStr[0] = '0';
					TempStr[1] = 'x';
					TempStr[10] = 0;

					uint8_t Start = 2;
					for(uint8_t i=0; i<8; i=i+2){
						TempStr[i+2] = EMeter_RxStr[Start+6-i];
						TempStr[i+3] = EMeter_RxStr[Start+7-i];
					}
					sscanf(TempStr,"%lXd",&E_Meter_RAM.Ap);		
					E_Meter_RAM.Ap *=10;
					Start = 10;
					for(uint8_t i=0; i<8; i=i+2){
						TempStr[i+2] = EMeter_RxStr[Start+6-i];
						TempStr[i+3] = EMeter_RxStr[Start+7-i];
					}
					sscanf(TempStr,"%lXd",&E_Meter_RAM.Am);
					E_Meter_RAM.Am *=10;
					Start = 18;
					for(uint8_t i=0; i<8; i=i+2){
						TempStr[i+2] = EMeter_RxStr[Start+6-i];
						TempStr[i+3] = EMeter_RxStr[Start+7-i];
					}
					sscanf(TempStr,"%lXd",&E_Meter_RAM.Rp);
					E_Meter_RAM.Rp *=10;
					Start = 26;
					for(uint8_t i=0; i<8; i=i+2){
						TempStr[i+2] = EMeter_RxStr[Start+6-i];
						TempStr[i+3] = EMeter_RxStr[Start+7-i];
					}
					sscanf(TempStr,"%lXd",&E_Meter_RAM.Rm);
					E_Meter_RAM.Rm *=10;
				
					StartTimer16(TD_EMeter,1000);
					EM_Retry = 0;
					EM_Mode = 0;
				}
				break;
			default: 
				if(Timer16Stopp(TD_EMeter)){
					EMeter_SendCR(EM_REQ);
					StartTimer16(TD_EMeter,100);
					EM_Mode = 1;
				}
				break;
		}
	}

	// --- Меркурий 230
	if(erb(&EMeterType) == MERCURY_230){
		switch(EM_Mode){

			case 1:	
				if(Timer16Stopp(TD_EMeter)){
					EMeter_SendData_P(EM_ENERGY_REQ, sizeof(EM_ENERGY_REQ));
					StartTimer16(TD_EMeter,100);
					EM_Mode = 2;
				}
				break;
			case 2:
				if(Timer16Stopp(TD_EMeter)){	
				
					 //read data

					// check frame
					if(EMeter_RxCharN != 19){
						EMeter_NoLink();
						break;
					}
					uint16_t MyCRC = CRC(rrb, (uint8_t *)EMeter_RxStr, 17);
					uint16_t EM_CRC = ((uint16_t)EMeter_RxStr[18]<<8) + EMeter_RxStr[17];
					if(MyCRC != EM_CRC){
						EMeter_NoLink();
						break;
					}
					if( EMeter_RxStr[0]!=0x00){
						EMeter_NoLink();
						break;
					}
					//process data
					E_Meter_RAM.Ap = EMeter_M230_Data_Convert(1);
					E_Meter_RAM.Am = EMeter_M230_Data_Convert(5);
					E_Meter_RAM.Rp = EMeter_M230_Data_Convert(9);
					E_Meter_RAM.Rm = EMeter_M230_Data_Convert(13);

					StartTimer16(TD_EMeter,1000);
					EM_Retry = 0;
					EM_Mode = 0;
				}
				break;
			default: 
				if(Timer16Stopp(TD_EMeter)){
					EMeter_SendData_P(EM_OPEN_CH_REQ, sizeof(EM_OPEN_CH_REQ));
					StartTimer16(TD_EMeter,100);
					EM_Mode = 1;
				}
				break;
		}
	}

	// --- ЭЛВИН эмуляция модема
	// EM_Mode = 0 - режим нач.конфигурации переводится командой ATZ
	// EM_Mode = 1 - режим ожидания вызова переводится командой ATE0 или AT, а также после завершения опроса
	// В режиме 1 делается RING (макс 5 раз пауза 5 сек) каждые 30сек 
	// EM_Mode = 2 - режим ождания запроса (/?!) переводится командой ATA
	// 
	if(erb(&EMeterType) == ELVIN_MODEM){
		
		#ifdef CTS
			CTS_ON();	// Clear RTS pin
		#endif

		if(EM_Retry>10){
			EMeter_ELVIN_Modem_NoLink();
			EM_Retry = 10;
		}
			
		if(!EMeter_TxFlag){		//если сейчас не передаём

			// Ответ на ATZ
			if( (strstr_P(EMeter_RxStr, EM_RESP_ATZ) != NULL) ){
				EMeter_ClearRXStr();
				EMeter_SendData_P(EM_RESP_OK, sizeof(EM_RESP_OK)-1);
				StartTimer16(TD_EMeter, 1500);	//ожидание 40сек
				EM_Retry++;	
				EM_Mode = 0;
				return;
			}

			// Ответ на ATE0
			if( (strstr_P(EMeter_RxStr, EM_RESP_ATE0) != NULL) ){
				EMeter_ClearRXStr();
				EMeter_SendData_P(EM_RESP_OK, sizeof(EM_RESP_OK)-1);
				EM_Mode = 1;
				return;
			}

			// Ответ на AT
			if( (strstr_P(EMeter_RxStr, EM_RESP_AT) != NULL) ){
				EMeter_ClearRXStr();
				EMeter_SendData_P(EM_RESP_OK, sizeof(EM_RESP_OK)-1);
				EM_Mode = 1;
				return;
			}

			// Реакция на +++ATH
			if( (strstr_P(EMeter_RxStr, EM_RESP_ATH) != NULL) ){
				EMeter_ClearRXStr();
				EM_Mode = 1;
				return;
			}

			// Ответ на ATA
			if( (strstr_P(EMeter_RxStr, EM_RESP_ATA) != NULL) ){
				EMeter_ClearRXStr();
				EMeter_SendData_P(EM_RESP_CONNECT, sizeof(EM_RESP_CONNECT)-1);
				RING_Retry = 0;
				EM_Mode = 2;
				return;
			}
			

			if(Timer16Stopp(TD_EMeter)){
				switch(EM_Mode){
					case 0:
						EMeter_ELVIN_Modem_NoLink();
						break;
					case 1:
						if(RING_Retry++>5){ 
							EM_Mode = 5;
							EM_Retry++;
						}
						else{
							EMeter_SendData_P(EM_RESP_RING, sizeof(EM_RESP_RING)-1);
							StartTimer16(TD_EMeter, 500);
						}
						break;
					case 2:	
						EMeter_SendCR(EM_REQ);
						StartTimer16(TD_EMeter, 300);
						EM_Mode = 3;
						break;
					case 3:
						EMeter_SendCR(EM_READ_DATA_REQ);
						StartTimer16(TD_EMeter,300);
						EM_Mode = 4;
						break;
					case 4:
						// Read data
						
						// check frame
						if(EMeter_RxCharN != 24){
							EM_Retry++;
							EM_Mode = 5;
							break;
						}
						EM_BCC = 0;
						for(uint8_t i=1; i<23; i++) EM_BCC += EMeter_RxStr[i];
						EM_BCC &= 0b01111111;
						if(EM_BCC != EMeter_RxStr[23]){
							EM_Retry++;
							EM_Mode = 5;
							break;
						}
						if( EMeter_RxStr[0]!=0x02 || EMeter_RxStr[1]!='(' || EMeter_RxStr[18] != ')' || EMeter_RxStr[19] != '!' ||
							EMeter_RxStr[20] != '\r' || EMeter_RxStr[21] != '\n'|| EMeter_RxStr[22] != 0x03){
							EM_Retry++;
							EM_Mode = 5;
							break;
						}
						
						{//process data
							char TempStr[9];
							TempStr[8] = 0;

							uint8_t Start = 2;
							for(uint8_t i=0; i<8; i=i+2){
								TempStr[i+0] = EMeter_RxStr[Start+6-i];
								TempStr[i+1] = EMeter_RxStr[Start+7-i];
							}
							sscanf(TempStr,"%ld",&E_Meter_RAM.Ap);		
							E_Meter_RAM.Ap *=10;
							Start = 10;
							for(uint8_t i=0; i<8; i=i+2){
								TempStr[i+0] = EMeter_RxStr[Start+6-i];
								TempStr[i+1] = EMeter_RxStr[Start+7-i];
							}
							sscanf(TempStr,"%ld",&E_Meter_RAM.Am);
							E_Meter_RAM.Am *=10;
						}
						EM_Retry = 0;
					case 5:												
						EMeter_SendData_P(EM_END_REQ, sizeof(EM_END_REQ));				
						StartTimer16(TD_EMeter,200);
						RING_Retry = 0;
						EM_Mode = 6;
						break;
					case 6:
						EMeter_SendData_P(EM_RESP_NOCARRIER, sizeof(EM_RESP_NOCARRIER)-1);
						StartTimer16(TD_EMeter,3000);
						EM_Mode = 1;
						break;
					default: 
						EM_Mode = 0;
				}//switch 
			}//Timer16Stopp

		}//EMeter_TxFlag

	}
	// ---
}
// ~~~~~~~~~

#endif
