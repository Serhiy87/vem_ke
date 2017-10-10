/*
	Для EMReader.
	Реализован прозрачный режим работы с GSM-мдемом.

	31.03.2016 - Potapov - добавлено чтение напряжений, токов, активной мощности для Меркурий 230
				Kononenko - изменяем UART-параметры при сменен типа счётчика ChangeUARTParam
				Убрал повторный опрос чеез 10сек после прозрачного режима
	
	13.03.2016 - Potapov - устранена проблема чтения времени на LANDIS_GYR_ZMR110, реализован двойной буфер с конвеерной обработкой для LANDIS_GYR_ZMR110

	23.02.2015 - Kononenko - добавил тип ELVIN_MODEM 
				Устранена проблема подсчёта BCC для ELVIN (EM_BCC &= 0b01111111)
	
	24.04.2015 - Kononenko - устранена проблема многократного переполнения буфера EMeter_RxStr (LANDIS_GYR_ZMR110 не работает)
				Два запроса(/?! + АСК U Z Y) на LANDIS_GYR_ZMR110 (на /?!+таймаут не все отвечают)
	
	27.04.2015 - Kononenko - используем define CTS, RTS

	29.04.2015 - Kononenko - для демонстрации читаем с Landis Ua,Ub,Uc, Ia,Ib,Ic
				Размер буфера более 256.

	09.10.2015 - Kononenko	- Опрос запускается извне. Период опроса зависит от периода отправки данных. Опрос запускается за 30сек до отправки.
		Т.е. для корректной работы нельзя ставить периода отправки данных менее 1 мин.
		Если часто опрашивать LANDIS_GYR_ZMR110, перестаёт отвечать. Нужно дать паузу в опросе.

	08.11.2015 - 3.02 - Переделан алгоритм чтения IEC Readout List LANDIS_GYR_ZMR110. Теперь можно читать все байты ответа.
		Читаем Ua,Ub,Uc,Ia,Ib,Ic,дату,время,код ошибок. 
		Выполняется анализ махинаций:магнит,клеммная крышка,вскрытие - по изменению счётчиков и общего времени воздействия.
		Если изменяется - значит есть воздействие.
		Счётчик снятия клеммной крышки C.61.0 инкременируется даже при кратковременном открытии ~ 1-2 сек при этом C.61.2 может не меняться
		Даётся только один Retry. Ранее 10.

*/
void Debug_DRE(void);
void Modbus_ReadHoldReg(void);
extern uint8_t AutoSend EEMEM;

typedef struct {
    uint32_t Ap;
    uint32_t Am;
  	uint32_t Rp;
    uint32_t Rm;
} E_METER_RAM;

uint16_t Ua,Ub,Uc;
uint16_t Ia,Ib,Ic;
uint8_t EM_Year, EM_Month, EM_Day, EM_Hour, EM_Min, EM_Sec;
uint16_t EM_PowerActive;
uint32_t EM_FailCode;
char EM_ID_Str[9];
uint32_t EM_Magnit_Min_new;
uint32_t EM_Magnit_Min_prev;
uint32_t EM_Klemm_Min_new;
uint32_t EM_Klemm_Min_prev;
uint32_t EM_Korpus_Min_new;
uint32_t EM_Korpus_Min_prev;
uint16_t EM_Magnit_Cnt_new;
uint16_t EM_Magnit_Cnt_prev;
uint16_t EM_Klemm_Cnt_new;
uint16_t EM_Klemm_Cnt_prev;
uint16_t EM_Korpus_Cnt_new;
uint16_t EM_Korpus_Cnt_prev;

volatile uint16_t EMeter_RxCharN;
volatile uint8_t EMeter_TxCharN;
volatile uint8_t EMeter_TxSz;
volatile uint8_t EMeter_TxFlag;	//transmitting is now
char EMeter_TxStr[15];
//#define  EM_RX_SIZE 350	TODO Для чтения параметров Ua,Ub,Uc, Ia,Ib,Ic из МЭК-листа нужен буфер 350 байт

///____Уменьшил размер буфера до 254, чтоб сделать число парным_____________________


#define  EM_RX_SIZE 254


char EMeter_RxStr[EM_RX_SIZE];
char* EMeter_RxStr_Offset;


#define EM_HalfBuffer_SIZE EM_RX_SIZE/2; //Размер полубуфера
volatile char* EM_HalfBufferStart;		//Указатель на начало полубуфера 
volatile uint8_t EM_BufferReadEnable; //разрешение чтеня буфера
// ~~~~~~~~~~~



// ~~~~~~~~~~~
//FIFO
#define EM_RX_FIFO_SIZE 255		// при условиях до 5мс ScanCycle и GPRS_In_MaxSz =< 50
uint8_t EM_RX_FIFO[EM_RX_FIFO_SIZE];
volatile uint8_t EM_RX_FIFO_Begin;	//Индекс последнего принятого байта
volatile uint8_t EM_RX_FIFO_End;	//Индекс последнего прочитаного байта
volatile uint8_t EM_RX_FIFOOverFlow;
volatile uint8_t EM_RX_FIFOMax;

uint8_t TD_EMeter;

uint8_t EM_Mode;
uint8_t EM_Retry=10;

uint8_t EM_BCC;

uint8_t EMeterTypeRAM; // для анализа EMeterType в ISR (в ISR нельзя работать с EEPROM - т.к. основной цикл также работает с EEPROM!)

enum EMETERS{
	EM_NONE,
	LANDIS_GYR_ZMR110,
	ELVIN,
	MERCURY_230,
	ELVIN_MODEM
};
//uint8_t EMeterType EEMEM = EM_NONE;

uint8_t EMeterType EEMEM = LANDIS_GYR_ZMR110;

//uint8_t EMeterType EEMEM = MERCURY_230;

#define LG_ZMR110_MAX_STR 30	// Самая длинная строка в ответе (список МЭК) 27 символов: C.90.1(0000000014674859)	

uint8_t LG_ZMR110_READ_FLAG[4];
	#define flg_OBIS_0_9_2		0
	#define flg_OBIS_0_9_1		1
	#define flg_OBIS_1_8_0		2
	#define flg_OBIS_3_8_0		3
	#define flg_OBIS_4_8_0		4
	#define flg_OBIS_2_8_0		5
	#define flg_OBIS_32_7_0		6
	#define flg_OBIS_52_7_0		7
	#define flg_OBIS_72_7_0		8
	#define flg_OBIS_31_7_0		9
	#define flg_OBIS_51_7_0		10
	#define flg_OBIS_71_7_0		11
	#define flg_OBIS_C_90_1		12
	#define flg_OBIS_16_7_0		13
	#define flg_OBIS_C_60_0		14
	#define flg_OBIS_C_60_2		15
	#define flg_OBIS_C_61_0		16
	#define flg_OBIS_C_61_2		17
	#define flg_OBIS_C_62_0		18
	#define flg_OBIS_C_62_2		19
	#define flg_OBIS_F_F_0		20
	#define flg_OBIS_0_0_0		21
/*
Младший байт:
Скорость:
0=MBBR1200, 1=MBBR2400, 2=MBBR4800, 3=MBBR9600, 4=MBBR14400, 5=MBBR19200, 6=MBBR28800, 7=MBBR38400, 8=MBBR57600
Старший байт:
Биты:0-1 Чётность;
00=NoParity(нет), 01=EvenParity(четный), 10=OddParity(нечетный)
Бит 2 - Стоп-бит; 0=1, 1=2
Биты:3-4:Кол-во бит:
00-5бит, 01-6бит, 10-7бит, 11- 8бит
*/

uint16_t EM_UART_Param EEMEM = 3 + (1<<8) + (0<<10) + (2<<11);	//Параметры связи со счётчиком 9600, 7, E,1

//uint16_t EM_UART_Param EEMEM = 3 + (0<<8) + (0<<10) + (3<<11);	//Параметры связи со счётчиком 9600, 8, N,1

uint8_t EM_UART_Rate ;
uint8_t EM_UART_BitQt;
uint8_t EM_UART_Parity;
uint8_t EM_UART_StopBits;

#ifdef E_METER

extern E_METER_RAM E_Meter_RAM;

//--- Landis&Gyr ZMR110
prog_char EM_RESP_0_9_2[] = "0.9.2";	//Дата (ГГ-ММ-ДД)
prog_char EM_RESP_0_9_1[] = "0.9.1";	//Время (чч:мм:сс)

prog_char EM_RESP_1_8_0[] = "1.8.0";	//Суммарная активная энергия потребление (+ А), kВч Всего
prog_char EM_RESP_2_8_0[] = "2.8.0";	//Суммарная активная энергия отдача/генерация (- А), kВч –Всего
prog_char EM_RESP_3_8_0[] = "3.8.0";	//Суммарная реактивная энергия потребление (+R), кварч Всего 
prog_char EM_RESP_4_8_0[] = "4.8.0";	//Суммарная реактивная энергия генерация (-R), кварч - Всего

prog_char EM_RESP_32_7_0[] = "32.7.0";	//Напряжение фазы А (RMS), В
prog_char EM_RESP_52_7_0[] = "52.7.0";	//Напряжение фазы B (RMS), В
prog_char EM_RESP_72_7_0[] = "72.7.0";	//Напряжение фазы C (RMS), В

prog_char EM_RESP_31_7_0[] = "31.7.0";	//Ток фазы А (RMS), А
prog_char EM_RESP_51_7_0[] = "51.7.0";	//Ток фазы B (RMS), А
prog_char EM_RESP_71_7_0[] = "71.7.0";	//Ток фазы C (RMS), А

prog_char EM_RESP_16_7_0[] = "16.7.0";	//Мгновенная активная мощность суммы 3-х фаз, кВт

prog_char EM_RESP_C_60_0[] = "C.60.0";	//Количество влияний на счетчик сильным магнитным полем нарастающим итогом
prog_char EM_RESP_C_60_2[] = "C.60.2";	//Общая длительность влияния на счетчик сильным магнитным полем нарастающим итогом (мин)
prog_char EM_RESP_C_61_0[] = "C.61.0";	//Количество снятий клеммной кришки нарастающим итогом
prog_char EM_RESP_C_61_2[] = "C.61.2";	//Общая длительность снятий клеммной крышки нарастающим итогом (мин)
prog_char EM_RESP_C_62_0[] = "C.62.0";	//Количество вскрытий корпуса счетчика нарастающим итогом
prog_char EM_RESP_C_62_2[] = "C.62.2";	//Общая длительность вскрытий корпуса нарастающим итогом (мин)

prog_char EM_RESP_0_0_0[] = "0.0.0";	//Идентификатор счетчика ID1.1 (8 знаков)
prog_char EM_RESP_F_F_0[] = "F.F.0";	//Код ошибки

//---ЭЛВИН
prog_char EM_REQ[]	= "/?!";
prog_char EM_READ_DATA_REQ	[]	= {0x06,0x30,0x35,0x30, 0x00};	//АСК	U	Z	Y	CR	LF - Сообщение подтверждения/выбора опций
//---Меркурий 230
prog_uint8_t EM_OPEN_CH_REQ[]= {0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x77,0x81};	//Запрос на открытие канала связи
prog_uint8_t EM_ENERGY_REQ[]= {0x00,0x05,0x00,0x00,0x10,0x25};	//Запросы на чтение массивов регистров накопленной энергии
prog_uint8_t EM_VOLTAGE_REQ[]={0x00,0x08,0x16,0x11,0x4F,0x8A};  //Запрос на чтение напряжений
prog_uint8_t EM_CURRENT_REQ[]={0x00,0x08,0x16,0x21,0x4F,0x9E};  //Запрос на чтение токов
prog_uint8_t EM_ACTIVEPOWER_REQ[]={0x00, 0x08, 0x16, 0x00, 0x8F, 0x86};  //Запрос на чтение активных мощностей
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

void ChangeUARTParam(void);
void EM_SetUARTParam(uint16_t Param);
void EMeter_ReInit(void);
void EMeter_Init(void);
void EMeter_SendFirstChar(void);
void EMeter_SendCR(prog_char *Str_P);
void EMeter_SendData_P(prog_uint8_t *Data_Out_P, uint8_t Size);
void EMeter_NoLink(void);
uint32_t EMeter_M230_Data_Convert(uint16_t Start);
uint32_t EMeter_ZMR110_Data_Convert(prog_char *Str_P);

// ~~~~~~~~~~~
void EM_InitFIFO(void){cli();
	EM_RX_FIFO_Begin = 255;
	EM_RX_FIFO_End = 255;
	EMeter_RxCharN = 0;
	sei();
//	EM_RX_FIFOOverFlow = 0;
//	EM_RX_FIFOMax = 0;
}
// ~~~~~~~~~~~
void EM_GetByteFromFIFO(uint8_t *ReadBuf, uint8_t *Index, uint8_t ReadBuf_Sz){

	//Reset *Index if read buffer overflow
	if(*Index >= ReadBuf_Sz){
		*Index = 0;
		//TODO установить флаг
	}	

	if(EM_RX_FIFO_Begin > EM_RX_FIFO_End){
		if((EM_RX_FIFO_End != 255)&&(EM_RX_FIFO_End != 255))			
			if(EM_RX_FIFOMax < (EM_RX_FIFO_Begin - EM_RX_FIFO_End))
				EM_RX_FIFOMax =  EM_RX_FIFO_Begin - EM_RX_FIFO_End;
		EM_RX_FIFO_End++;
		ReadBuf[*Index] = EM_RX_FIFO[EM_RX_FIFO_End];
		*Index=*Index+1;
	}
	if(EM_RX_FIFO_End > EM_RX_FIFO_Begin){
		if((EM_RX_FIFO_End != 255)&&(EM_RX_FIFO_End != 255))
			if(EM_RX_FIFOMax < (EM_RX_FIFO_SIZE - EM_RX_FIFO_End  + EM_RX_FIFO_Begin))
				EM_RX_FIFOMax = EM_RX_FIFO_SIZE - EM_RX_FIFO_End  + EM_RX_FIFO_Begin;
		EM_RX_FIFO_End++;
		if(EM_RX_FIFO_End >= EM_RX_FIFO_SIZE) EM_RX_FIFO_End = 0;
		ReadBuf[*Index] = EM_RX_FIFO[EM_RX_FIFO_End];
		*Index=*Index+1;
	}
}
// ~~~~~~~~~~~
uint8_t EM_GetDataFromFIFO(const uint8_t Amount){

	while(EM_RX_FIFO_Begin != EM_RX_FIFO_End){
		EM_GetByteFromFIFO((uint8_t *)EMeter_RxStr, (uint8_t *)&EMeter_RxCharN, EM_RX_SIZE);
		if(EMeter_RxCharN >= Amount){
			EMeter_RxCharN = 0;
			return Amount;
		}
	}//while
	return EMeter_RxCharN;
}
// ~~~~~~~~~~~
inline void GetByteFromEM_FIFO_Transp(void){

	if(EM_RX_FIFO_Begin != EM_RX_FIFO_End){
		EM_RX_FIFO_End++;
		if(EM_RX_FIFO_End >= EM_RX_FIFO_SIZE) EM_RX_FIFO_End = 0;
		UDR_GSM = EM_RX_FIFO[EM_RX_FIFO_End];
		UCSR_GSM_A |= (1<<TXC0);	//Clear TxC pending interrupt
	}
	else{
		// disable DRE interrupt & enable TxC interrupt
		UCSR_GSM_B = (1<<RXCIE_GSM) | (1<<TXCIE_GSM) | (0<<UDRIE_GSM) | (1<<RXEN_GSM) | (0<<UCSZ_GSM_2) | (1<<TXEN_GSM);

	}

	//--- Calculate EM_RX_FIFOMax
	if(EM_RX_FIFO_Begin > EM_RX_FIFO_End){
			if(EM_RX_FIFOMax < (EM_RX_FIFO_Begin - EM_RX_FIFO_End))
				EM_RX_FIFOMax =  EM_RX_FIFO_Begin - EM_RX_FIFO_End;
	}
	if(EM_RX_FIFO_End > EM_RX_FIFO_Begin){
			if(EM_RX_FIFOMax < (EM_RX_FIFO_SIZE - EM_RX_FIFO_End + EM_RX_FIFO_Begin))
				EM_RX_FIFOMax = EM_RX_FIFO_SIZE - EM_RX_FIFO_End + EM_RX_FIFO_Begin;
	}
}
// ~~~~~~~~~~~
void ApplyUARTParam(void){
	
	uint16_t Temp16 = EM_UART_Rate + (EM_UART_Parity<<8) + (EM_UART_StopBits<<10) + (EM_UART_BitQt<<11);
	eww(&EM_UART_Param, Temp16);
	if(!UART_Soft){	// не применять настройки СОМ-порта в режимах GSM_DebugMode или Modbus
		EM_SetUARTParam(erw(&EM_UART_Param));
	}
	Modbus_ReadHoldReg();
}
// ~~~~~~~~~~~
void ChangeUARTParam(void){
	if( erb(&EMeterType) == EM_NONE){
		
	}
	if(erb(&EMeterType) == LANDIS_GYR_ZMR110){
		//Параметры связи со счётчиком 9600, 7, E,1
		EM_UART_Rate = 3;
		EM_UART_Parity = 1;
		EM_UART_StopBits = 0;
		EM_UART_BitQt = 2;
	}
	if( (erb(&EMeterType) == ELVIN) || (erb(&EMeterType) == MERCURY_230) || (erb(&EMeterType) == ELVIN_MODEM) ){
		//Параметры связи со счётчиком 9600, 8, N,1
		EM_UART_Rate = 3;
		EM_UART_Parity = 0;
		EM_UART_StopBits = 0;
		EM_UART_BitQt = 3;		
	}
	ApplyUARTParam();
}
// ~~~~~~~~~~~
void EM_SetUARTParam(uint16_t Params){
	
	UCSR_EMETER_A = (0<<U2X_EMETER) | (0<<MPCM_EMETER);	
	
	EM_UART_Rate = Params & 0x00FF;

	switch(EM_UART_Rate){
		case 0:// 1200
			UBRR_EMETER_H = 0x03;
			UBRR_EMETER_L = 0x42;
			break;
		case 1:// 2400
			UBRR_EMETER_H = 0x01;
			UBRR_EMETER_L = 0xA0;
			break;
		case 2:// 4800
			UBRR_EMETER_H = 0x00;
			UBRR_EMETER_L = 0xCF;
			break;
		case 3:// 9600
			UBRR_EMETER_H = 0x00;
			UBRR_EMETER_L = 0x67;
			break;
		case 4:// 14400
			UBRR_EMETER_H = 0x00;
			UBRR_EMETER_L = 0x44;
			break;
		case 5:// 19200
			UBRR_EMETER_H = 0x00;
			UBRR_EMETER_L = 0x33;
			break;
		case 6:// 28800
			UBRR_EMETER_H = 0x00;
			UBRR_EMETER_L = 0x22;
			break;
		case 7:// 38400
			UBRR_EMETER_H = 0x00;
			UBRR_EMETER_L = 0x19;
			break;
		case 8:// 57600	
			UBRR_EMETER_H = 0x00;
			UBRR_EMETER_L = 0x10;
			break;
		default:
			UBRR_EMETER_H = 0x00;
			UBRR_EMETER_L = 0x67;
			break;
	}

	UCSR_EMETER_C = (0<<UMSEL_EMETER) | (0<<UPM_EMETER_1) | (0<<UPM_EMETER_0) | (0<<USBS_EMETER) | (0<<UCSZ_EMETER_1) | (0<<UCSZ_EMETER_0) | (0<<UCPOL_EMETER);


	EM_UART_Parity = (Params >> 8) & 0b00000011;

	//00=NoParity(нет), 01=EvenParity(четный), 10=OddParity(нечетный)
	switch(EM_UART_Parity){
		case 0://Disabled
			UCSR_EMETER_C |= (0<<UPM_EMETER_1) | (0<<UPM_EMETER_0);
			break;
		case 1://Enabled, Even Parity
			UCSR_EMETER_C |= (1<<UPM_EMETER_1) | (0<<UPM_EMETER_0);
			break;
		case 2://Enabled, Odd Parity
			UCSR_EMETER_C |= (1<<UPM_EMETER_1) | (1<<UPM_EMETER_0);
			break;
		case 3://Disabled
			UCSR_EMETER_C |= (0<<UPM_EMETER_1) | (0<<UPM_EMETER_0);
			break;
	}


	EM_UART_StopBits = (Params >> 10) & 0b00000001;
	//Бит 2 - Стоп-бит; 0=1, 1=2
	switch(EM_UART_StopBits){
		case 0://1-bit
			UCSR_EMETER_C |= (0<<USBS_EMETER);
			break;
		case 1://2-bits
			UCSR_EMETER_C |= (1<<USBS_EMETER);
			break;
	}
	
	EM_UART_BitQt = (Params >> 11) & 0b00000011;
	//Биты:3-4:Кол-во бит: 00-5бит, 01-6бит, 10-7бит, 11- 8бит
	switch(EM_UART_BitQt){
		case 0://5-bit
			UCSR_EMETER_C |= (0<<UCSZ_EMETER_1) | (0<<UCSZ_EMETER_0);
			break;
		case 1://6-bit
			UCSR_EMETER_C |= (0<<UCSZ_EMETER_1) | (1<<UCSZ_EMETER_0);
			break;
		case 2://7-bit
			UCSR_EMETER_C |= (1<<UCSZ_EMETER_1) | (0<<UCSZ_EMETER_0);
			break;
		case 3://8-bit
			UCSR_EMETER_C |= (1<<UCSZ_EMETER_1) | (1<<UCSZ_EMETER_0);
			break;
	}

}
// ~~~~~~~~~~~

void EMeter_Init(void){
	TD_EMeter = Timer16SysAlloc(1);
	EMeter_ReInit();
	if(erb(&AutoSend)){
		StartTimer16(TD_EMeter,1000);
		EM_Mode = 0;
	}
	else{
		EM_Mode = 255;
	}
}
void EMeter_ReInit(void){

	//RS485
	MB_PLC_Init();
	
	#ifdef CTS
		CTS_Config();
		CTS_OFF();
	#endif
	#ifdef RTS
		RTS_Config();
	#endif

	EM_InitFIFO();

	UCSR_EMETER_B = (1<<RXCIE_EMETER) | (0<<TXCIE_EMETER) | (0<<UDRIE_EMETER) | (1<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);
	UCSR_EMETER_A = (1<<TXC_EMETER) | (0<<U2X_EMETER) | (0<<MPCM_EMETER); //Clear TXC_EMETER -  pending interrupt

	EM_SetUARTParam(erw(&EM_UART_Param));

	EM_HalfBufferStart= EMeter_RxStr;//----Присваиваем указатель на начало полубуфера
	EMeter_RxStr_Offset = EMeter_RxStr;
	EM_BufferReadEnable=0;//-----запрещаем чтение
	EMeter_RxCharN=0;
}

// ~~~~~~~~~
void EMeter_RX(void){
	//----- Transparent
	if(Transparent){
					StartTimer16(TCP_CONNECT_check_timer, Connection_check_period);
					StartTimer16(GPRS_RECONNECT_timer, GPRS_RECONNECT_period);
		char Char = UDR_EMETER;

		//---FIFO
		EM_RX_FIFO_Begin++;
		if(EM_RX_FIFO_Begin >= EM_RX_FIFO_SIZE) EM_RX_FIFO_Begin = 0;
		EM_RX_FIFO[EM_RX_FIFO_Begin] = Char;
		if(EM_RX_FIFO_Begin == EM_RX_FIFO_End){
			EM_RX_FIFOOverFlow = 1;
			EM_RX_FIFOMax = EM_RX_FIFO_SIZE;
			
		}

		// enable DRE interrupt for UDR0
		UCSR_GSM_B =  (1<<RXCIE_GSM) | (0<<TXCIE_GSM) | (1<<UDRIE_GSM) | (1<<RXEN_GSM) | (0<<UCSZ_GSM_2) | (1<<TXEN_GSM);
		UCSR_GSM_A |= (1<<TXC0);	//Clear TxC pending interrupt
	}
	//----- Non transparent
/*	else{

		if(EMeterTypeRAM == LANDIS_GYR_ZMR110){
			EM_HalfBufferStart[EMeter_RxCharN] = UDR_EMETER;
		}
		else
		{
			EMeter_RxStr[EMeter_RxCharN] = UDR_EMETER;
		}
		
		
		if(EMeterTypeRAM == ELVIN_MODEM){
			EMeter_RxCharN++;
			if(EMeter_RxCharN >= EM_RX_SIZE) EMeter_RxCharN = 0; // Защита от переполнения для ELVIN_MODEM - на второй круг
		}
		else{
			if(EMeterTypeRAM == LANDIS_GYR_ZMR110)
			{
				if( ((EM_HalfBufferStart[EMeter_RxCharN] == 0x0A) && (EMeter_RxCharN >= (EM_RX_SIZE/2 - LG_ZMR110_MAX_STR)))||(EMeter_RxCharN >= EM_RX_SIZE/2-1) ) 
				{
					EMeter_RxCharN=0;
					if(EM_HalfBufferStart==EMeter_RxStr)
					{
						EMeter_RxStr_Offset=EMeter_RxStr;
						EM_HalfBufferStart=&EMeter_RxStr[EM_RX_SIZE/2];
										
					}else if(EM_HalfBufferStart==&EMeter_RxStr[EM_RX_SIZE/2])
					{	
													
						EMeter_RxStr_Offset=&EMeter_RxStr[EM_RX_SIZE/2];
						EM_HalfBufferStart=EMeter_RxStr;						
					}
					EM_BufferReadEnable=1;
				}
				else{
					EMeter_RxCharN++;	

					}
			}					
			else{
				if(EMeter_RxCharN < (EM_RX_SIZE-1) )EMeter_RxCharN++;// Защита от переполнения для остальных - стоп
			}
		}
	}*/
	//-----
}
// ~~~~~~~~~
void EMeter_DRE(void){
	if(GSM_DebugMode) Debug_DRE();
	else{
		if(Transparent){
			GetByteFromGSM_FIFO_Transp();
		}
		else{
			if(EMeter_TxCharN < EMeter_TxSz){
				UDR_EMETER = EMeter_TxStr[EMeter_TxCharN];
				EMeter_TxCharN++;
			}
			else{
				UCSR_EMETER_B =  (1<<RXCIE_EMETER) | (1<<TXCIE_EMETER) | (0<<UDRIE_EMETER) | (1<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);
			}
		}
	}
}
// ~~~~~~~~~
void EMeter_TX(void){
	if(Transparent){
		UCSR_EMETER_B =  (1<<RXCIE_EMETER) | (0<<TXCIE_EMETER) | (0<<UDRIE_EMETER) | (1<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);
		PORTD &= ~(1<<PD4);	//RS485 Tx_off
	}
	else{
		UCSR_EMETER_B =  (1<<RXCIE_EMETER) | (0<<TXCIE_EMETER) | (0<<UDRIE_EMETER) | (1<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);
		EMeter_TxFlag = 0;
		PORTD &= ~(1<<PD4);	//RS485 Tx_off
	}

}

// ~~~~~~~~~
void EMeter_SendFirstChar(void){
	cli();
	EMeter_TxFlag = 1;
	UCSR_EMETER_A |= (1<<TXC_EMETER);	//Clear TXC_EMETER -  pending interrupt	
	PORTD |= (1<<PD4);	// Tx on
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
	sprintf_P(EMeter_TxStr + EMeter_TxSz, PSTR("\r\n"));
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
	uint16_t i;
	for(i=0; i<EM_RX_SIZE; i++) EMeter_RxStr[i] = 0;
	EMeter_RxCharN = 0;

}
// ~~~~~~~~~
uint8_t EMeter_Link(void){
//	if(EM_Retry>9) return 0;
	if(EM_Retry>=1) return 0;
	else return 1;
}

void EMeter_NoLink(void){
//	if(EM_Retry>10){
	if(EM_Retry>=1){
		E_Meter_RAM.Ap = 0;
		E_Meter_RAM.Am = 0;
		E_Meter_RAM.Rp = 0;
		E_Meter_RAM.Rm = 0;
		Ua = 0;
		Ub = 0;
		Uc = 0;
		Ia = 0;
		Ib = 0;
		Ic = 0;
		EM_PowerActive = 0;
	}
	else EM_Retry++;

	//StartTimer16(TD_EMeter,1000);
	EM_Mode = 255;
}
// ~~~~~~~~~
void EMeter_ELVIN_Modem_NoLink(void){
	E_Meter_RAM.Ap = 0;
	E_Meter_RAM.Am = 0;
	E_Meter_RAM.Rp = 0;
	E_Meter_RAM.Rm = 0;
}
// ~~~~~~~~~
uint32_t EMeter_M230_Data_Convert(uint16_t Start){
	uint8_t TempArray[4];
	TempArray[3] = EMeter_RxStr[Start+1];
	TempArray[2] = EMeter_RxStr[Start+0];
	TempArray[1] = EMeter_RxStr[Start+3];
	TempArray[0] = EMeter_RxStr[Start+2];
	if(*(uint32_t*)TempArray == 0xFFFFFFFF) return 0;
	else return *(uint32_t*)TempArray;
}
// ~~~~~~~~~
// ~~~~~~~~~
uint16_t EMeter_M230_Data_ConvertWord(uint16_t Start){
	uint8_t TempArray[4];
	TempArray[0] = EMeter_RxStr[Start];
	TempArray[1] = EMeter_RxStr[Start+1];
	if(*(uint16_t*)TempArray == 0xFFFF) return 0;
	else return *(uint16_t*)TempArray;
}
// ~~~~~~~~~
uint16_t EMeter_M230_Data_ConvertP(uint16_t Start){
	uint8_t TempArray[4];
	TempArray[3] = 0;
	TempArray[2] = EMeter_RxStr[Start];
	TempArray[1] = EMeter_RxStr[Start+2];
	TempArray[0] = EMeter_RxStr[Start+1];
	return (uint16_t)(((*(uint32_t*)TempArray)&0x3FFFFF)/1000);
}

//~~~~~~


uint8_t Check_LG_ZMR110_READ_FLAG(uint8_t FlagNum){
	return LG_ZMR110_READ_FLAG[FlagNum/8] & (1<<FlagNum%8);
}

uint32_t EMeter_ZMR110_Data_ConvertQ(prog_char *Str_P, uint8_t FlagNum){
	uint32_t Result;
	char* temp_ptr = strstr_P(EMeter_RxStr_Offset, Str_P);
	if(temp_ptr != NULL){
		uint16_t Start = temp_ptr - EMeter_RxStr_Offset;
//		EM_BCC = Start;	//TODO for Debug
		if(Start<(EM_RX_SIZE-1)){
			EMeter_RxStr_Offset[Start+12]=	EMeter_RxStr_Offset[Start+13];	//Сдвиг десятичной точки
			EMeter_RxStr_Offset[Start+13]=	EMeter_RxStr_Offset[Start+14];
			EMeter_RxStr_Offset[Start+14]=	EMeter_RxStr_Offset[Start+15];
			EMeter_RxStr_Offset[Start+15]= ' ';
			sscanf(EMeter_RxStr_Offset+Start+6,"%ld",&Result);
			LG_ZMR110_READ_FLAG[FlagNum/8] |= (1<<FlagNum%8);
			if(EMeter_RxStr_Offset[Start+13]=='*') Result *=100;
			if(EMeter_RxStr_Offset[Start+14]=='*') Result *=10;
			return Result;
		}
	}	
	return 0;
}
// ~~~~~~~~~
uint16_t EMeter_ZMR110_Read_Word(prog_char *Str_P, uint8_t FlagNum){
	uint16_t Result;
	char* temp_ptr = strstr_P(EMeter_RxStr_Offset, Str_P);
	if(temp_ptr != NULL){
		uint16_t Start = temp_ptr - EMeter_RxStr_Offset;
//		EM_BCC = Start;	//TODO for Debug
		if(Start<(EM_RX_SIZE-1)){
			LG_ZMR110_READ_FLAG[FlagNum/8] |= (1<<FlagNum%8);
			//sscanf(EMeter_RxStr_Offset+Start+7,"%u",&Result);
			sscanf(EMeter_RxStr_Offset+Start+strlen_P(Str_P)+1,"%u",&Result);							
			return Result;
		}
	}	
	return 0;
}
// ~~~~~~~~~
uint16_t EMeter_ZMR110_Data_ConvertI(prog_char *Str_P, uint8_t FlagNum){
	uint16_t Result;
	char* temp_ptr = strstr_P(EMeter_RxStr_Offset, Str_P);
	if(temp_ptr != NULL){
		uint16_t Start = temp_ptr - EMeter_RxStr_Offset;
//		EM_BCC = Start;	//TODO for Debug
		if(Start<(EM_RX_SIZE-1)){
			EMeter_RxStr_Offset[Start+10]=	EMeter_RxStr_Offset[Start+11];	//Сдвиг десятичной точки
			EMeter_RxStr_Offset[Start+11]=	EMeter_RxStr_Offset[Start+12];
			EMeter_RxStr_Offset[Start+12]= ' ';
			sscanf(EMeter_RxStr_Offset+Start+7,"%u",&Result);
			LG_ZMR110_READ_FLAG[FlagNum/8] |= (1<<FlagNum%8);
			return Result;
		}
	}	
	return 0;
}
// ~~~~~~~~~
void EMeter_ZMR110_Read_Date(prog_char *Str_P, uint8_t FlagNum){
	char* temp_ptr = strstr_P(EMeter_RxStr_Offset, Str_P);
	if(temp_ptr != NULL){
		uint16_t Start = temp_ptr - EMeter_RxStr_Offset;
//		EM_BCC = Start;	//TODO for Debug
		if(Start<(EM_RX_SIZE-1)){
			LG_ZMR110_READ_FLAG[FlagNum/8] |= (1<<FlagNum%8);
			sscanf_P(EMeter_RxStr_Offset+Start+6,PSTR("%hhu-%hhu-%hhu"),&EM_Year, &EM_Month, &EM_Day);
		}
	}	
}

// ~~~~~~~~~
void EMeter_ZMR110_Read_Time(prog_char *Str_P, uint8_t FlagNum){
	char* temp_ptr = strstr_P(EMeter_RxStr_Offset, Str_P);
	if(temp_ptr != NULL){
		uint16_t Start = temp_ptr - EMeter_RxStr_Offset;
//		EM_BCC = Start;	//TODO for Debug
		if(Start<(EM_RX_SIZE-1)){
			LG_ZMR110_READ_FLAG[FlagNum/8] |= (1<<FlagNum%8);
			sscanf_P(EMeter_RxStr_Offset+Start+6,PSTR("%hhu:%hhu:%hhu"),&EM_Hour, &EM_Min, &EM_Sec);
		}
	}	
}
// ~~~~~~~~~
uint32_t EMeter_ZMR110_Read_DWord(prog_char *Str_P, uint8_t FlagNum){
	uint32_t Result;
	char* temp_ptr = strstr_P(EMeter_RxStr_Offset, Str_P);
	if(temp_ptr != NULL){
		uint16_t Start = temp_ptr - EMeter_RxStr_Offset;
//		EM_BCC = Start;	//TODO for Debug
		if(Start<(EM_RX_SIZE-1)){
			sscanf_P(EMeter_RxStr_Offset+Start+strlen_P(Str_P)+1,PSTR("%lud"),&Result);
			LG_ZMR110_READ_FLAG[FlagNum/8] |= (1<<FlagNum%8);
			return Result;
		}
	}	
	return 0;
}
// ~~~~~~~~~
void EMeter_ZMR110_Read_ID1_1(prog_char *Str_P, uint8_t FlagNum){
	char* temp_ptr = strstr_P(EMeter_RxStr_Offset, Str_P);
	if(temp_ptr != NULL){
		uint16_t Start = temp_ptr - EMeter_RxStr_Offset;
//		EM_BCC = Start;	//TODO for Debug
		if(Start<(EM_RX_SIZE-1)){
			LG_ZMR110_READ_FLAG[FlagNum/8] |= (1<<FlagNum%8);
			sscanf_P(EMeter_RxStr_Offset+Start+6,PSTR("%8s"), EM_ID_Str);
		}
	}	
}
// ~~~~~~~~~
uint8_t IsEMReady(void){
	if(EM_Mode == 255) return 1;
	else return 0;
}

// ~~~~~~~~~
void EMRead(void){
	if(erb(&EMeterType) != ELVIN_MODEM)	EM_Mode = 0;
}

// ~~~~~~~~~
void EMeter_Cycle(void){

	static uint8_t RING_Retry;	// Для ELVIN_MODEM

	EMeterTypeRAM = erb(&EMeterType);


	if(UART_Soft || Transparent){
	
	
	}
	else{

		if( erb(&EMeterType) == EM_NONE){
			EMeter_RxCharN = 0;
			EMeter_ELVIN_Modem_NoLink();
		}

		//--- Landis&Gyr ZMR110
		if( erb(&EMeterType) == LANDIS_GYR_ZMR110){
			switch(EM_Mode){

				case 1:	
						cli();
						EM_HalfBufferStart= EMeter_RxStr;//----Присваиваем указатель на начало полубуфера
						EMeter_RxStr_Offset = EMeter_RxStr;
						EM_BufferReadEnable=0;//-----запрещаем чтение
						EMeter_RxCharN=0;
						sei();	
					if(Timer16Stopp(TD_EMeter)){
						EMeter_SendCR(EM_READ_DATA_REQ);
						StartTimer16(TD_EMeter,1000);
						for(uint8_t i=0;i<sizeof(LG_ZMR110_READ_FLAG); i++) LG_ZMR110_READ_FLAG[i] = 0;
						//EMeter_ClearRXStr();
						
						
						
							cli();
						EM_HalfBufferStart= EMeter_RxStr;//----Присваиваем указатель на начало полубуфера
						EMeter_RxStr_Offset = EMeter_RxStr;
						EM_BufferReadEnable=0;//-----запрещаем чтение
						EMeter_RxCharN=0;
						sei();


						EM_Mode = 2;
					}
					break;
				case 2:
					if(EM_BufferReadEnable==1)
					{
												cli();
												EMeter_RxStr[EM_RX_SIZE-1] = '\0';
												EMeter_RxStr[EM_RX_SIZE/2-1]='\0';//конец первого полубуфера
												 //read & analyze data
					


												char *p; 
												p=strchr(EMeter_RxStr_Offset,'\n');
												sei();
												if(p != NULL){
													*p = '\0';
													// search for OBIS code
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_0_9_2)) 
														EMeter_ZMR110_Read_Date(EM_RESP_0_9_2, flg_OBIS_0_9_2);
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_0_9_1)) 
														EMeter_ZMR110_Read_Time(EM_RESP_0_9_1, flg_OBIS_0_9_1);

													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_1_8_0)) 
														E_Meter_RAM.Ap = EMeter_ZMR110_Data_ConvertQ(EM_RESP_1_8_0, flg_OBIS_1_8_0);
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_2_8_0)) 
														E_Meter_RAM.Am = EMeter_ZMR110_Data_ConvertQ(EM_RESP_2_8_0, flg_OBIS_2_8_0);
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_3_8_0)) 
														E_Meter_RAM.Rp = EMeter_ZMR110_Data_ConvertQ(EM_RESP_3_8_0, flg_OBIS_3_8_0);
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_4_8_0)) 
														E_Meter_RAM.Rm = EMeter_ZMR110_Data_ConvertQ(EM_RESP_4_8_0, flg_OBIS_4_8_0);

													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_32_7_0)) 
														Ua = EMeter_ZMR110_Read_Word(EM_RESP_32_7_0, flg_OBIS_32_7_0);
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_52_7_0)) 							
														Ub = EMeter_ZMR110_Read_Word(EM_RESP_52_7_0, flg_OBIS_52_7_0);
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_72_7_0)) 
														Uc = EMeter_ZMR110_Read_Word(EM_RESP_72_7_0, flg_OBIS_72_7_0);

													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_31_7_0)) 
														Ia = EMeter_ZMR110_Data_ConvertI(EM_RESP_31_7_0, flg_OBIS_31_7_0);
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_51_7_0)) 
														Ib = EMeter_ZMR110_Data_ConvertI(EM_RESP_51_7_0, flg_OBIS_51_7_0);
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_71_7_0)) 
														Ic = EMeter_ZMR110_Data_ConvertI(EM_RESP_71_7_0, flg_OBIS_71_7_0);

													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_16_7_0)) 
														EM_PowerActive = EMeter_ZMR110_Data_ConvertI(EM_RESP_16_7_0, flg_OBIS_16_7_0);
						
													//-----
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_C_60_0)){
														uint16_t Temp = EMeter_ZMR110_Read_Word(EM_RESP_C_60_0, flg_OBIS_C_60_0);
														if(Check_LG_ZMR110_READ_FLAG(flg_OBIS_C_60_0)){
															if(EM_Magnit_Cnt_new == 0){
																EM_Magnit_Cnt_prev = EM_Magnit_Cnt_new = Temp;
															}
															else{
																EM_Magnit_Cnt_prev = EM_Magnit_Cnt_new;
																EM_Magnit_Cnt_new = Temp;								
															}
														}
													} 
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_C_60_2)){
														uint32_t Temp = EMeter_ZMR110_Read_DWord(EM_RESP_C_60_2, flg_OBIS_C_60_2);
														if(Check_LG_ZMR110_READ_FLAG(flg_OBIS_C_60_2)){
															if(EM_Magnit_Min_new	 == 0){
																EM_Magnit_Min_prev = EM_Magnit_Min_new = Temp;
															}
															else{
																EM_Magnit_Min_prev = EM_Magnit_Min_new;
																EM_Magnit_Min_new = Temp;								
															}
														}
													} 

													//-----
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_C_61_0)){
														uint16_t Temp = EMeter_ZMR110_Read_Word(EM_RESP_C_61_0, flg_OBIS_C_61_0);
														if(Check_LG_ZMR110_READ_FLAG(flg_OBIS_C_61_0)){
															if(EM_Klemm_Cnt_new	 == 0){
																EM_Klemm_Cnt_prev = EM_Klemm_Cnt_new = Temp;
															}
															else{
																EM_Klemm_Cnt_prev = EM_Klemm_Cnt_new;
																EM_Klemm_Cnt_new = Temp;								
															}
														}
													} 
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_C_61_2)){
														uint32_t Temp = EMeter_ZMR110_Read_DWord(EM_RESP_C_61_2, flg_OBIS_C_61_2);
														if(Check_LG_ZMR110_READ_FLAG(flg_OBIS_C_61_2)){
															if(EM_Klemm_Min_new	 == 0){
																EM_Klemm_Min_prev = EM_Klemm_Min_new = Temp;
															}
															else{
																EM_Klemm_Min_prev = EM_Klemm_Min_new;
																EM_Klemm_Min_new = Temp;								
															}
														}
													} 
													//-----
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_C_62_0)){
														uint16_t Temp = EMeter_ZMR110_Read_Word(EM_RESP_C_62_0, flg_OBIS_C_62_0);
														if(Check_LG_ZMR110_READ_FLAG(flg_OBIS_C_62_0)){
															if(EM_Korpus_Cnt_new	 == 0){
																EM_Korpus_Cnt_prev = EM_Korpus_Cnt_new = Temp;
															}
															else{
																EM_Korpus_Cnt_prev = EM_Korpus_Cnt_new;
																EM_Korpus_Cnt_new = Temp;								
															}
														}
													} 
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_C_62_2)){
														uint32_t Temp = EMeter_ZMR110_Read_DWord(EM_RESP_C_62_2, flg_OBIS_C_62_2);
														if(Check_LG_ZMR110_READ_FLAG(flg_OBIS_C_62_2)){
															if(EM_Korpus_Min_new	 == 0){
																EM_Korpus_Min_prev = EM_Korpus_Min_new = Temp;
															}
															else{
																EM_Korpus_Min_prev = EM_Korpus_Min_new;
																EM_Korpus_Min_new = Temp;								
															}
														}
													} 
													//-----

													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_F_F_0)) 
														EM_FailCode = EMeter_ZMR110_Read_DWord(EM_RESP_F_F_0, flg_OBIS_F_F_0);
													if(!Check_LG_ZMR110_READ_FLAG(flg_OBIS_0_0_0)) 
														EMeter_ZMR110_Read_ID1_1(EM_RESP_0_0_0, flg_OBIS_0_0_0);

							

													/*
													if(p >= EMeter_RxStr + EM_RX_SIZE - LG_ZMR110_MAX_STR) EMeter_RxStr_Offset = EMeter_RxStr;
													else EMeter_RxStr_Offset = p+1;
													*/
													cli();
													//----Старая проверка
													/*
													if(((p >= (EMeter_RxStr + EM_RX_SIZE - LG_ZMR110_MAX_STR))&&(EM_HalfBufferStart!=EMeter_RxStr))||((p >= (EMeter_RxStr + EM_RX_SIZE/2 - LG_ZMR110_MAX_STR))&&(EM_HalfBufferStart==EMeter_RxStr)))
													{
														EM_BufferReadEnable=0;
													}
						
						
													else 
													{
														EMeter_RxStr_Offset = p+1;
													}
													*/

												if(EM_HalfBufferStart==EMeter_RxStr)
													{
														if((p >= (EMeter_RxStr + EM_RX_SIZE/2 - LG_ZMR110_MAX_STR))&&(p<&EMeter_RxStr[EM_RX_SIZE/2]))
															{
																EM_BufferReadEnable=0;
															}
														else
															{
																EMeter_RxStr_Offset = p+1;
			
															}
				
													}
												else
													{								
														if((p >= (EMeter_RxStr + EM_RX_SIZE - LG_ZMR110_MAX_STR))&&(p<&EMeter_RxStr[EM_RX_SIZE]))
															{
																EM_BufferReadEnable=0;				
															}
														else
															{
																EMeter_RxStr_Offset = p+1;
															}	
													}
													sei();

												}
					}

					 //check EM link
					if(Timer16Stopp(TD_EMeter)){
						
						uint8_t read_flag = 0;
						for(uint8_t i=0;i<sizeof(LG_ZMR110_READ_FLAG); i++) 
							if(LG_ZMR110_READ_FLAG[i]) read_flag = 1;
						if(read_flag){
							EM_Retry = 0;
						}
						else{
							EMeter_NoLink();
							EM_Retry = 1;
						}
						EM_Mode = 255;
					
					}
					break;
				case 255:
					// состояние ожидания команды на опрос 
					break;
			
				default: 
					if(Timer16Stopp(TD_EMeter)){
						EMeter_ClearRXStr();
						EMeter_SendCR(EM_REQ);
						//StartTimer16(TD_EMeter,200);	//TODO уменшить ?
						//StartTimer16(TD_EMeter,230);	//TODO уменшить ?
						StartTimer16(TD_EMeter,170);	//TODO уменшить ?
					//	StartTimer16(TD_EMeter,100);	//TODO уменшить ?
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
				
						//StartTimer16(TD_EMeter,1000);
						EM_Retry = 0;
						EM_Mode = 255;
					}
					break;
				case 255:
					// состояние ожидания команды на опрос 
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

						//StartTimer16(TD_EMeter,1000);
						
						//EM_Mode = 255;

						StartTimer16(TD_EMeter,100);
						EM_Mode = 3;
					}
					break;
				case 3:
					if(Timer16Stopp(TD_EMeter)){
						EMeter_SendData_P(EM_VOLTAGE_REQ, sizeof(EM_VOLTAGE_REQ));
						StartTimer16(TD_EMeter,100);
						EM_Mode = 4;							
					}
					break;

				case 4:
					if(Timer16Stopp(TD_EMeter)){
						if(EMeter_RxCharN != 12){
							EMeter_NoLink();
							break;
						}
						uint16_t MyCRC = CRC(rrb, (uint8_t *)EMeter_RxStr, 10);
						uint16_t EM_CRC = ((uint16_t)EMeter_RxStr[11]<<8) + EMeter_RxStr[10];
						if(MyCRC != EM_CRC){
							EMeter_NoLink();
							break;
						}
						if( EMeter_RxStr[0]!=0x00){
							EMeter_NoLink();
							break;
						}
						Ua=EMeter_M230_Data_ConvertWord(2)/100;
						Ub=EMeter_M230_Data_ConvertWord(5)/100;
						Uc=EMeter_M230_Data_ConvertWord(8)/100;
						StartTimer16(TD_EMeter,100);
						EM_Mode = 5;						
					}
					break;

				case 5:
					if(Timer16Stopp(TD_EMeter)){
						EMeter_SendData_P(EM_CURRENT_REQ, sizeof(EM_CURRENT_REQ));
						StartTimer16(TD_EMeter,100);
						EM_Mode = 6;							
					}
					break;

				case 6:
					if(Timer16Stopp(TD_EMeter)){
						if(EMeter_RxCharN != 12){
							EMeter_NoLink();
							break;
						}
						uint16_t MyCRC = CRC(rrb, (uint8_t *)EMeter_RxStr, 10);
						uint16_t EM_CRC = ((uint16_t)EMeter_RxStr[11]<<8) + EMeter_RxStr[10];
						if(MyCRC != EM_CRC){
							EMeter_NoLink();
							break;
						}
						if( EMeter_RxStr[0]!=0x00){
							EMeter_NoLink();
							break;
						}
						Ia=EMeter_M230_Data_ConvertWord(2)/10;
						Ib=EMeter_M230_Data_ConvertWord(5)/10;
						Ic=EMeter_M230_Data_ConvertWord(8)/10;
						StartTimer16(TD_EMeter,100);
						EM_Mode = 7;						
					}
					break;

				case 7:
					if(Timer16Stopp(TD_EMeter)){
						EMeter_SendData_P(EM_ACTIVEPOWER_REQ, sizeof(EM_ACTIVEPOWER_REQ));
						StartTimer16(TD_EMeter,100);
						EM_Mode = 8;							
					}
					break;

				case 8:
					if(Timer16Stopp(TD_EMeter)){
						if(EMeter_RxCharN != 15){
							EMeter_NoLink();
							break;
						}
						uint16_t MyCRC = CRC(rrb, (uint8_t *)EMeter_RxStr, 13);
						uint16_t EM_CRC = ((uint16_t)EMeter_RxStr[14]<<8) + EMeter_RxStr[13];
						if(MyCRC != EM_CRC){
							EMeter_NoLink();
							break;
						}
						if( EMeter_RxStr[0]!=0x00){
							EMeter_NoLink();
							break;
						}
						EM_PowerActive = EMeter_M230_Data_ConvertP(1);

						EM_Retry = 0;
						EM_Mode = 255;						
					}
					break;
				case 255:
					// состояние ожидания команды на опрос 
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
				cli();
				CTS_ON();	// Clear RTS pin
				sei();
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
	}//NonTransparent
}
// ~~~~~~~~~

#endif
