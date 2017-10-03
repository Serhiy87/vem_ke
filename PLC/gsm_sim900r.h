/* ~~~~~~~~~~~~~
// 	Modem: SIM900R, 9600 bps,N,1

	Режим авто - устанавливает скорость
	Режим ручн - можно руками набирать AT команды с консоли
	Режим кофиг - устанавливает скорость 9600
	
	Режим 
	Данные отсылаются UDP-пакетом на 
	 если GPRS_FlgSz_Out!=0
	Данные не отсылаются на IP=0.0.0.0 или если маскирована отслыка IP_SendMask!

	Затем отправка SMS. SMS не отправляется на номер 00000000000000!


	При приёме ожидается строка: xxxxxxxxxxxxxxxx<CR><LF>
	'\r' CR - carridge return 0x0D
	'\n' LF - line feed 0x0A
	'\0' LF - конец строки 0x00
	Пример: ATE0<CR><LF>

	эхо отключаем - т.к.модем повторяет на ногу ТХ всё, что ему пришло на UART RX, причём моментально

	A.Модем Встановити швидкість модему: 9600 baud.
	AT+IPR=9600
	AT&W (зберігти налаштування у енергонезалежній пам’яті)
	По замовчуванню стоїть 115200bps та режим автоматичного визначення швидкості (autobaud).
	
	B.SIM-карта
	Відімкнути запит PIN-коду

	Brief AT-command description. For detail see SIM300_ATC_V2.03.pdf
	ATE0 - No echo
	AT+CMGF=1 - Set SMS system into text mode, as opposed to PDU mode
	AT+CSMP - Set SMS text mode parameters
	AT+CMGS - Send SMS
	AT+CIPMUX - to enable multi connection mode
	AT+CGATT=1 - Attach to GPRS network
	AT+CIPCSGP - Set CSD or GPRS connection mode. +CIPCSGP:1-GPRS, APN, USER NAME, PASSWORD
	AT+CSTT - START task and Set APN..USER ID..PASSWORD
	AT+CIICR - Bring up wireless connection with GPRS or CSD
			AT+CIICR only activate moving scene at the status of IP START..after
			operate this command, the state changed to IP CONFIG. If module
			accept the activate operation, the state changed to IP IND; after module
			accept the activate operation, if activate successfully, the state changed
			to IP GPRSACT, response OK, otherwise response ERROR
	AT+CIFSR - Get local IP address
	AT+CLPORT - Set local port
	AT+CIPSERVER - Configure as a server
	AT+CIPCCON - choose connection. Note that there may exist two connections at one time: one connection is as
				client connecting with remote server, the other connection is as server connecting with remote client.
				Using this command to choose through which connection data is sent.
	AT+CIPSTART - Start up TCP or UDP connection
	AT+CIPSEND - Send data through TCP or UDP connection
	AT+CIPCLOSE - Close TCP or UDP Connection
	AT+CIPSHUT - Disconnect wireless connection

	Возможные проблемы:
	Приём по RX без буфера(очереди). Поэтому, пока анализируется текущая строка приёма GSM_RxStr, всё что пришло в этот момент игнорируется
	О пропущенных символах говорит	RxBufOverFlow. Т.е. при большой скорости прихода данных, при загрузе scan cycle, возможно 
	проглатывание символов.

	Критерий успешного ответа большинства AT-команд подсчёт кол-ва'\n' == 2
	Критерий не успешного ответа на AT+CIICR,AT+CIPSERVER, анализ последнего печатаемого символа ответа
		ERROR,OK Т.е. если 'R' - значит плохо - уходим на AT+CIPSHUT
	
	20 раз пытаемся подключиться к сети AT+CGATT=1. Помагает т.к.с первого раза после неудачи не всегда сеть разрешает.

	24.12.2012	- введена пауза 3 сек между отправками по UDP(т.к. пропуски на IP2)

	25.01.2013	- универсальность GPRS_TerminalProcess на экран 4x20 и 8x40

	15.10.2013	- таймаут для TCP-соединения при отсутсвии правильных TCP-запросов,
				- изменяемый APN(2-е строки), USER_NAME, PASSWORD
				- ручной ввод AT-команд при define GSM_DEBUG

	04.11.2013	- таймаут для GSM_WAIT_CALL_READY увеличен до 20 сек

	04.02.2014	- таймаут для GSM_SEND_CIICR увеличен до 120 сек (при APN test.umc.ua время выполнения до 43 сек)

	13.02.2014	- ОДЗ допустимых IP адресов: если 255.255.255.255 - приконектится может кто угодно

	04.03.2014	- Постоянно (1раз в 655,35сек) перезапускается Server (бывали случаи когда Server не отвечал)
					Также перезапуск происходит после разъединения TCP втечении таймаута (TCP_CONNECT_timeout)
	
	19.08.2014 - Для чтения uint8_t sscanf(str,"%hhu") 1-байт. Нельзя sscanf(str,"%hu"): т.к. 2 байта пишет в память

	02.04.2015 - Может зависать на этапе получения IP GSM_WAIT_CIFSR_OK. Добавлено if(Timer16Stopp(TD_GSM)) GSM_State = GSM_ReStart1
	
	01.06.2016 - Добавил uint8_t GSM_Modem для совместимости с gsm_sim800_http.h

~~~~~~~~~~~~~*/
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "ip.h"	// работа с IP-адресами

uint8_t GSM_Modem = 1;
	#define NOT_RECOGNIZED	0
	#define SIMCOM_SIM900R	1
	#define SIMCOM_SIM800	2

prog_char GSM_SIMCOM_SIM900R[]	= "SIMCOM_SIM900R";
prog_char GSM_SIMCOM_SIM800[]	= "SIMCOM_SIM800";

//#define GSM_DEBUG	//Подключает консоль, 
//#define GSM_DEBUG_DELAY 1	//замедляет  в  раз таймера 

#if LCDXSz==16
	#ifndef GPRS_APN1
		#define GPRS_APN1		"vpni.kyivstar.n"
	#endif
	#ifndef GPRS_APN2
		#define GPRS_APN2 		"et             "
	#endif
	#ifndef GPRS_USER_NAME
		#define GPRS_USER_NAME 	"               "
	#endif
	#ifndef GPRS_PSW
		#define GPRS_PSW 		"               "
	#endif
#elif LCDXSz==20
	#ifndef GPRS_APN1
		#define GPRS_APN1		"vpni.kyivstar.net  "
	#endif
	#ifndef GPRS_APN2
		#define GPRS_APN2 		"                   "
	#endif
	#ifndef GPRS_USER_NAME
		#define GPRS_USER_NAME 	"                   "
	#endif
	#ifndef GPRS_PSW
		#define GPRS_PSW 		"                   "
	#endif
#elif LCDXSz==40
	#ifndef GPRS_APN
		#define GPRS_APN		"vpni.kyivstar.net            "	//29символов
	#endif
	#ifndef GPRS_USER_NAME
		#define GPRS_USER_NAME 	"                   "	//19символов
	#endif
	#ifndef GPRS_PSW
		#define GPRS_PSW 		"                   "	//19символов
	#endif
#endif


#if LCDXSz==40
	uint8_t GPRS_apn[30] EEMEM 				= GPRS_APN;
	uint8_t GPRS_apn_user_name[20] EEMEM	= GPRS_USER_NAME;
	uint8_t GPRS_apn_psw[20] EEMEM 			= GPRS_PSW;
#else
	uint8_t GPRS_apn1[LCDXSz] EEMEM 			= GPRS_APN1;
	uint8_t GPRS_apn2[LCDXSz] EEMEM 			= GPRS_APN2;
	uint8_t GPRS_apn_user_name[LCDXSz] EEMEM	= GPRS_USER_NAME;
	uint8_t GPRS_apn_psw[LCDXSz] EEMEM 			= GPRS_PSW;
#endif

#ifndef VEGA_SN
	#define VEGA_SN 0
#endif
uint16_t Vega_SN EEMEM = VEGA_SN;


#ifndef GSM_PASSWORD
	#define GSM_PASSWORD 22780
#endif
uint16_t GSM_Password EEMEM = GSM_PASSWORD;

//URC
prog_char URC_RDY[]	= "RDY";
prog_char URC_CFUN[] = "+CFUN: 1";
prog_char URC_CPIN_READY[] = "+CPIN: READY";
prog_char URC_CALL_READY[] = "Call Ready";
prog_char URC_REMOTE_IP[] = ", REMOTE IP";
prog_char URC_RECEIVE[] = "+RECEIVE,";
prog_char URC_CLOSED[] = ", CLOSED";


// AT-commands RESPONSE
prog_char RESP_OK[] = "OK";
prog_char RESP_CONNECT_OK[] = "x, CONNECT OK";
prog_char RESP_SERVER_OK[]	= "SERVER OK";
prog_char RESP_SERVER_CLOSE[] = "SERVER CLOSE";
prog_char RESP_Invitation[] = ">";
prog_char RESP_SEND_OK[] = "x, SEND OK";
prog_char RESP_CLOSE_OK[] = "CLOSE OK";
prog_char RESP_CLOSE_OK_FAST[] = "x, CLOSE OK";

// ~~~~~~~~~~~
// AT-commands
// ~~~~~~~~~~~

// Config
prog_char AT_AT[]			= "AT";
prog_char AT_IPR9600[]		= "AT+IPR=9600";

// Total
prog_char AT_E0[]			= "ATE0";

// SMS
prog_char AT_CMGF[]			= "AT+CMGF=1";
prog_char AT_CSMP[]			= "AT+CSMP=17,167,0,0";

prog_char AT_CMGS[]			= "AT+CMGS=\"+";

// GPRS
prog_char AT_CIPMUX[]		= "AT+CIPMUX=1";

prog_char AT_CGATT[]		= "AT+CGATT=1";
//prog_char AT_CIPCSGP[]		= "AT+CIPCSGP=1,\""GPRS_APN"\"\r";
prog_char AT_CIPCSGP[]		= "AT+CIPCSGP=1,\"";
prog_char AT_CSTT[]			= "AT+CSTT";
prog_char AT_CIICR[]		= "AT+CIICR";
prog_char AT_CIFSR[]		= "AT+CIFSR";

prog_char AT_CIPSTART[]		= "AT+CIPSTART=0,\"UDP\",";
prog_char AT_CIPSERVER[]	= "AT+CIPSERVER=1,";
prog_char AT_SERVERCLOSE[]	= "AT+CIPSERVER=0";

prog_char AT_CIPSEND[]		= "AT+CIPSEND=";

prog_char AT_CIPCLOSE_1[]	= "AT+CIPCLOSE=x,1";

prog_char AT_CIPCLOSE[]		= "AT+CIPCLOSE=0";
prog_char AT_CIPSHUT[]		= "AT+CIPSHUT";


// Auto
enum {
	GSM_PowerOn,
	GSM_WAIT_RDY,
	GSM_WAIT_CFUN,
	GSM_WAIT_CPIN_READY,
	GSM_WAIT_CALL_READY,
	
	GSM_SEND_E0, 		GSM_WAIT_E0_OK,
	GSM_SEND_CMGF,		GSM_WAIT_CMGF_OK,
	GSM_SEND_CSMP,		GSM_WAIT_CSMP_OK,
	GSM_SEND_CIPMUX, 	GSM_WAIT_CIPMUX_OK,

	GSM_WAIT_1,

	GSM_SEND_CGATT,		GSM_WAIT_CGATT_OK,
	GSM_SEND_CIPCSGP,	GSM_WAIT_CIPCSGP_OK,
	GSM_SEND_CSTT,		GSM_WAIT_CSTT_OK,
	GSM_SEND_CIICR,		GSM_WAIT_CIICR_OK,
	GSM_SEND_CIFSR,		GSM_WAIT_CIFSR_OK,

	GSM_SEND_CIPSTART,		GSM_WAIT_CIPSTART_OK,	GSM_WAIT_CIPSTART_CONNECT_OK,
	GSM_SEND_CIPSERVER,		GSM_WAIT_CIPSERVER_OK,	GSM_WAIT_CIPSERVER_SERVER_OK,

	GSM_ServerIdle,			GSM_AnalyzeURC,

	GSM_SEND_CIPSEND,		GSM_WAIT_Inv,				GSM_SEND_DATA, 				GSM_WAIT_CIPSEND_SEND_OK,	GSM_WAIT_CIPSEND_PAUSE,
	GSM_SEND_CMGS,			GSM_WAIT_SMS_Inv,			GSM_SEND_SMS_DATA, 			GSM_WAIT_CMGS_OK,
	GSM_TCP_CLIENT_CONNECT, GSM_TCP_CLIENT_DISCONNECT,	GSM_WAIT_CLIENT_CIPCLOSED,
	GSM_RECIEVE, 			GSM_RECIEVE_DATA,

	GSM_SEND_CIPCLOSE,		GSM_WAIT_CIPCLOSE_OK,

	GSM_ServerRdy,
	GSM_Fail,
	GSM_ServerReceptSz, GSM_ServerRecept,
	GSM_ServerConnIdle,
	GSM_CIPSEND_Server, GSM_Transmit_Server,
	GSM_CLPORT_UDP, GSM_CIPSTART, GSM_CIPCCON_1,
	GSM_CIPSEND_Client, GSM_Transmit_Client,
	GSM_CIPCLOSE,
	GSM_CIPSHUT, GSM_ShutWait,

	GSM_CMGS, GSM_SMS,

	GSM_ReStart1, GSM_ReStart2,
	
	GSM_SEND_SERVERCLOSE,	GSM_WAIT_SERVERCLOSE_OK,	GSM_WAIT_SERVER_CLOSE,
};

// Config
enum {
	GSM_CFG_START,
	GSM_CFG_SEND_AT, 		GSM_CFG_WAIT_AT_OK,
	GSM_CFG_SEND_IPR,		GSM_CFG_WAIT_IPR_OK,
	GSM_SAVE_IPR,			GSM_FINISH 
};

volatile uint8_t GSM_State;

volatile uint8_t GSM_Control;	// Режим управления модемом
	#define GSM_CNTL_AUTO	0	// автомат (по умолчанию)
	#define GSM_CNTL_MANUAL	1	// вручную (через консоль)
	#define GSM_CNTL_CONFIG	2	// начальная конфигурация (установка скороти UART модема)
		
volatile uint8_t GSM_Flag = 0b00000001;
	#define flg_TxCStr	0	//передача строки завершена


volatile uint8_t RxBufOverFlow;

#define GPRS_HeadSz 4

#if defined (__AVR_ATmega16__) ||  defined (__AVR_ATmega32__)
	#define GPRS_Out_MaxSz	(10 - GPRS_HeadSz)
	#define GPRS_In_MaxSz	(10 - GPRS_HeadSz)
#elif defined (__AVR_ATmega128__) || defined (__AVR_ATmega2561__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)  || (__AVR_ATxmega128A1__)
	#define GPRS_Out_MaxSz	(0xFF - GPRS_HeadSz)
	#define GPRS_In_MaxSz	(50 - GPRS_HeadSz)
#endif


static uint8_t TD_GSM, TD_TCP_Connect;
#define GSM_RXSTR_SIZE 50	// максимальная длина строки (не данных!) от модема
char GSM_RxStr[GSM_RXSTR_SIZE];
volatile uint8_t GSM_RxCharN, GSM_TxCharN;

char GSM_TxStr[0xFF];

uint8_t GPRS_Data_Out[GPRS_Out_MaxSz], GPRS_Data_In[GPRS_In_MaxSz];
char SMS_Data_Out[100];
uint8_t GPRS_FlgSz_Out, GPRS_FlgSz_In, SMS_FlgSz_Out;
volatile uint8_t GSMTxSz;
volatile uint8_t GSM_Tx;
uint8_t GPRS_Protocol_Out;
	#define UDP 0
	#define TCP 1
uint8_t IP_SendMask;	// Блокирует отправкуб если бит установлен : 0bit-UDP IP1, 1bit-UDP IP2, 2bit-UDP IP3, 3bit-UDP IP4, 
uint8_t TempNum;
uint8_t GSM_Temp;

// ~~~~~~~~~~~
//FIFO
#define GSM_RX_FIFO_SIZE 70		// при условиях до 5мс ScanCycle и GPRS_In_MaxSz =< 50
uint8_t GSM_RX_FIFO[GSM_RX_FIFO_SIZE];
volatile uint8_t GSM_RX_FIFO_Begin;	//Индекс последнего принятого байта
uint8_t GSM_RX_FIFO_End;	//Индекс последнего прочитаного байта
volatile uint8_t GSM_RX_FIFOOverFlow;
volatile uint8_t GSM_RX_FIFOMax;

// ~~~~~~~~~~~
IP_Addr GSM_MyIP;
IP_Addr GSM_ClientIP;
#define NO_CONNECTION 255
uint8_t GSM_ActiveConnection = NO_CONNECTION;


#ifndef UDP_ServerIP_Init
	#define UDP_ServerIP_Init { \
	 {194,176,97,118},	\
	{0,0,0,0},	\
	{0,0,0,0},	\
	{0,0,0,0}}
#endif
IP_Addr UDP_ServerIP[4] EEMEM= UDP_ServerIP_Init;

#ifndef UDP_ServerPort_Init
	#define UDP_ServerPort_Init	{2021,0,0,0}
#endif
uint16_t UDP_ServerPort[4] EEMEM = UDP_ServerPort_Init;

#ifndef TCP_ListenPort_Init
	#define TCP_ListenPort_Init	2020
#endif
uint16_t TCP_ListenPort EEMEM = TCP_ListenPort_Init;

#ifndef TCP_ClientIP_Init
	#define TCP_ClientIP_Init { \
	{194,176,97,118},	\
	{176,36,14,205},	\
	{0,0,0,0},	\
	{0,0,0,0}}
#endif
IP_Addr TCP_ClientIP[4] EEMEM = TCP_ClientIP_Init;

uint16_t TCP_CONNECT_timeout EEMEM = 3000;

//---SMS
#ifndef MaxTelephN
	#define MaxTelephN 13	//+1 for \0 end of string
#endif
#ifndef MaxTelephDirSz
	#define MaxTelephDirSz 3
#endif

#ifndef SMS_NUMBER_INIT
	#define SMS_NUMBER_INIT {"000000000000", "000000000000", "000000000000"}
#endif
char SMS_Number[MaxTelephDirSz][MaxTelephN] EEMEM = SMS_NUMBER_INIT;

uint8_t SMS_On EEMEM = 0;

void GSM_SendFirstChar(void);

// ~~~~~~~~~~~
#ifdef GSM_DEBUG
	uint8_t GPRS_DebugStr[LCDXSz];
	uint8_t GPRS_RxTerminalPtr;
	char GPRS_RxTerminalStr[20][LCDXSz];

	void ShiftDown(void){
		for (uint8_t j=19; j>0; j--)
			for (uint8_t i=0; i<LCDXSz; i++)
				GPRS_RxTerminalStr[j][i] = GPRS_RxTerminalStr[j-1][i];
		for (uint8_t i=0; i<LCDXSz; i++)
			GPRS_RxTerminalStr[0][i] = ' ';	
	}

	void GPRS_TerminalProcess(char Char){
		switch(Char){
			case 0x0D://CR - carridge return 0x0D
				GPRS_RxTerminalPtr = 0;
				break;
			case 0x0A://LF - line feed 0x0A
				ShiftDown();
				break;
			default:
				if(GPRS_RxTerminalPtr>=LCDXSz){
					ShiftDown();
					GPRS_RxTerminalPtr = 0;	
				}
				if(GPRS_RxTerminalPtr<LCDXSz)
					if ( isprint(Char) ||  ( (0xC0<=Char) && (Char<=0xFF) ) ||
						Char==0xB2 || Char==0xB3 || Char==0xA8 || Char==0xB8 || Char==0xAA || Char==0xBA || Char==0xAF || Char==0xBF)
							GPRS_RxTerminalStr[0][GPRS_RxTerminalPtr++] = Char;
		}
	}
	
	void GPRS_RunDebugStr(void){
		if(GSM_Flag & (1<<flg_TxCStr)){		
			uint8_t i=0;
			while ( (i<LCDXSz-1) && (GPRS_DebugStr[i]!=' ')){
				GSM_TxStr[i] = GPRS_DebugStr[i];
				i++;
			}
			GSM_TxStr[i] = '\r';
			GSMTxSz = i+1;
			GSM_SendFirstChar();
			memset(GPRS_DebugStr,' ',sizeof(GPRS_DebugStr)-1);
		}
	}
#endif
// ~~~~~~~~~~~
void InitFIFO(void){
	GSM_RX_FIFO_Begin = 255;
	GSM_RX_FIFO_End = 255;
	GSM_RxCharN = 0;
	GSM_RX_FIFOOverFlow = 0;
	GSM_RX_FIFOMax = 0;
}
// ~~~~~~~~~~~
void GetByteFromFIFO(uint8_t *ReadBuf, uint8_t *Index, uint8_t ReadBuf_Sz){

	//Reset *Index if read buffer overflow
	if(*Index >= ReadBuf_Sz){
		*Index = 0;
		//TODO установить флаг
	}	

	if(GSM_RX_FIFO_Begin > GSM_RX_FIFO_End){
		if((GSM_RX_FIFO_End != 255)&&(GSM_RX_FIFO_End != 255))			
			if(GSM_RX_FIFOMax < (GSM_RX_FIFO_Begin - GSM_RX_FIFO_End))
				GSM_RX_FIFOMax =  GSM_RX_FIFO_Begin - GSM_RX_FIFO_End;
		GSM_RX_FIFO_End++;
		ReadBuf[*Index] = GSM_RX_FIFO[GSM_RX_FIFO_End];
		*Index=*Index+1;
	}
	if(GSM_RX_FIFO_End > GSM_RX_FIFO_Begin){
		if((GSM_RX_FIFO_End != 255)&&(GSM_RX_FIFO_End != 255))
			if(GSM_RX_FIFOMax < (GSM_RX_FIFO_SIZE - GSM_RX_FIFO_End  + GSM_RX_FIFO_Begin))
				GSM_RX_FIFOMax = GSM_RX_FIFO_SIZE - GSM_RX_FIFO_End  + GSM_RX_FIFO_Begin;
		GSM_RX_FIFO_End++;
		if(GSM_RX_FIFO_End >= GSM_RX_FIFO_SIZE) GSM_RX_FIFO_End = 0;
		ReadBuf[*Index] = GSM_RX_FIFO[GSM_RX_FIFO_End];
		*Index=*Index+1;
	}
}
// ~~~~~~~~~~~
// Возвращает длину строки с учётом символа завершения строки
uint8_t GetStringFromFIFO(void){

	uint8_t StrLength = 0;

	while(GSM_RX_FIFO_Begin != GSM_RX_FIFO_End){
		
		GetByteFromFIFO((uint8_t *)GSM_RxStr, (uint8_t *)&GSM_RxCharN, GSM_RXSTR_SIZE);

		//При приёме ожидается строка: xxxxxxxxxxxxxxxx<CR><LF>
		//	'\r' CR - carridge return 0x0D
		//	'\n' LF - line feed 0x0A
		//Преобразуем в строку xxxxxxxxxxxxxxxx<\0>
		if( (GSM_RxCharN >= 2) && (GSM_RxStr[GSM_RxCharN-2]== '\r') && (GSM_RxStr[GSM_RxCharN-1]== '\n') ){
			GSM_RxStr[GSM_RxCharN-2] = '\0';
			StrLength = GSM_RxCharN-1;
			GSM_RxCharN = 0;
			//GSM_Flag |= (1<<flg_RxCStr);	// Ignore GSM_RxStr like (0x0D 0x0A)
			return StrLength;
		}

	}//while

	return StrLength;
}
// ~~~~~~~~~~~
uint8_t GetDataFromFIFO(const uint8_t Amount){

	while(GSM_RX_FIFO_Begin != GSM_RX_FIFO_End){
		GetByteFromFIFO((uint8_t *)GSM_RxStr, (uint8_t *)&GSM_RxCharN, GSM_RXSTR_SIZE);
		if(GSM_RxCharN >= Amount){
			GSM_RxCharN = 0;
			return Amount;
		}
	}//while
	return GSM_RxCharN;
}
// ~~~~~~~~~~~
// Копирует со строки EEPROM в строку RAM до первого пробела или '\0' или размер строки EEPROM
uint8_t strcpy_EE(char* str_RAM, uint8_t* str_EE){
	uint8_t i=0;
	while( (erb(&str_EE[i])!='\0') && (erb(&str_EE[i])!=' ') ){
		str_RAM[i] = (char)erb(&str_EE[i]);
		i++;
	}
	return i;	
}
// ~~~~~~~~~~~
void
GSM_Init(void)
{	
	InitFIFO();
	#ifdef GSM_DEBUG
		memset(GPRS_RxTerminalStr,' ',sizeof(GPRS_RxTerminalStr)-1);
		memset(GPRS_DebugStr,' ',sizeof(GPRS_DebugStr)-1);
	#endif

	#if defined (__AVR_ATxmega128A1__)
		// USARTE0
		PORTE.OUT |=(1<<3);
		PORTE.DIR |=(1<<3);
		USART_GSM.BAUDCTRLA = 0x67;	// 9600
		USART_GSM.BAUDCTRLB = 0x00 + USART_BSCALE0_bm;
		USART_GSM.CTRLA = USART_RXCINTLVL_HI_gc;
		USART_GSM.CTRLB = (1<<USART_RXEN_bp) | (1<<USART_TXEN_bp) | (0<<USART_CLK2X_bp) | (0 << USART_MPCM_bp) | (0 << USART_TXB8_bp);
		USART_GSM.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | (0<< USART_SBMODE_bp) | USART_CHSIZE_8BIT_gc;
	#else
		UCSR_GSM_A = ~(1<<U2X_GSM) &~(1<<MPCM_GSM);
		UCSR_GSM_B =  (1<<RXCIE_GSM) | (0<<TXCIE_GSM) | (0<<UDRIE_GSM) | (1<<RXEN_GSM) | (0<<UCSZ_GSM_2) | (1<<TXEN_GSM);
		#if defined (__AVR_ATmega2561__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)			
			UCSR_GSM_C = (0<<UMSEL_GSM_1) | (0<<UMSEL_GSM_0) | (0<<UPM_GSM_1) | (0<<UPM_GSM_0) | (0<<USBS_GSM) | (1<<UCSZ_GSM_1) | (1<<UCSZ_GSM_0) | (0<<UCPOL_GSM);
		#else
			UCSR_GSM_C = ((~(1<<UMSEL_GSM) &~(1<<UPM_GSM_1) &~(1<<UPM_GSM_0) &~(1<<USBS_GSM)) |(1<<UCSZ_GSM_1) |(1<<UCSZ_GSM_0)) &~(1<<UCPOL_GSM);
		#endif
		UBRR_GSM_H = 0x00;
		UBRR_GSM_L = 0x67;	// 9600
	#endif

	GSM_PWRCNTRL_Config();
	GSM_PWRCNTRL_ON();

	TD_GSM = Timer16SysAlloc(1);
	TD_TCP_Connect = Timer16SysAlloc(1);

/*	for (uint8_t i=0; i<4; i++)
		GPRS_ServerAddr[i] = erb(GPRS_ServerAddr_EE+i);
*/
	GSM_Control = GSM_CNTL_AUTO;
	GSM_State = GSM_PowerOn;



}
// ~~~~~~~~~
void
GSM_RX(void)
{
	char Char = UDR_GSM;

	//---FIFO
	GSM_RX_FIFO_Begin++;
	if(GSM_RX_FIFO_Begin >= GSM_RX_FIFO_SIZE) GSM_RX_FIFO_Begin = 0;
	GSM_RX_FIFO[GSM_RX_FIFO_Begin] = Char;
	if(GSM_RX_FIFO_Begin == GSM_RX_FIFO_End){
		GSM_RX_FIFOOverFlow = 1;
		GSM_RX_FIFOMax = GSM_RX_FIFO_SIZE;
	}
	#ifdef GSM_DEBUG
		GPRS_TerminalProcess(Char);
	#endif

}
// ~~~~~~~~~
//Вызывать только после GetStringFromFIFO()!
uint8_t GSM_Wait_Response(char *Str, uint8_t RestoreCMD){
//	if(Timer16Stopp(TD_GSM) && RestoreCMD){
//		GSM_State = RestoreCMD;
//		return 0;
//	}
	if(strcmp(GSM_RxStr, Str))return 0;
	else return 1;
}
// ~~~~~~~~~
uint8_t GSM_Wait_Response_P(prog_char *Str_P, uint8_t RestoreCMD){
	
	if(Timer16Stopp(TD_GSM) && RestoreCMD){
		GSM_State = RestoreCMD;
		return 0;
	}
	//Анализ принятой строки
	if(GetStringFromFIFO()){
		if(strcmp_P(GSM_RxStr, Str_P)) return 0;
		else return 1;
	}
	else return 0;
}
// ~~~~~~~~~
uint8_t GSM_Wait_Char(char Char, uint8_t RestoreCMD){
	if(Timer16Stopp(TD_GSM)){
		GSM_State = RestoreCMD;
		return 0;
	}
	//Анализ первого принятого байта
	if(GetDataFromFIFO(1)){
		if(GSM_RxStr[0] == Char)	return 1;
		else return 0;
	}
	else return 0;
}
// ~~~~~~~~~
void GSM_SendFirstChar(void){

	UDR_GSM = GSM_TxStr[0];
	#ifdef GSM_DEBUG
		GPRS_TerminalProcess(GSM_TxStr[0]);
	#endif

	GSM_TxCharN = 1;
	cli();
	GSM_Flag &=~(1<<flg_TxCStr);	
	sei();
	cli();
	#if defined (__AVR_ATxmega128A1__)
		USART_GSM.CTRLA = USART_RXCINTLVL_HI_gc | USART_DREINTLVL_HI_gc;
	#else
		UCSR_GSM_B =  (1<<RXCIE_GSM) | (0<<TXCIE_GSM) | (1<<UDRIE_GSM) | (1<<RXEN_GSM) | (0<<UCSZ_GSM_2) | (1<<TXEN_GSM);
	#endif
	
	sei();	
}
// ~~~~~~~~~
//Инициирует передачу строки + CR
void GSM_SendCR(prog_char *Str_P){
	GSMTxSz = strlen_P(Str_P);
	sprintf_P(GSM_TxStr, Str_P);
	sprintf(GSM_TxStr + GSMTxSz, "\r");
	GSMTxSz++;
	GSM_SendFirstChar();
}
// ~~~~~~~~~
//Инициирует передачу данных + CntrZ
void GSM_AddHeaderSendData(uint8_t *Data_Out, uint8_t Size){
	GSMTxSz = Size;
	GSM_TxStr[0] = 5;
	GSM_TxStr[1] = 1;
	GSM_TxStr[2] = erw(&Vega_SN)>>8;
	GSM_TxStr[3] = (uint8_t)erw(&Vega_SN);
/*	GSM_TxStr[0] = 'H';
	GSM_TxStr[1] = 'E';
	GSM_TxStr[2] = 'A';
	GSM_TxStr[3] = 'D';*/
	memcpy(GSM_TxStr+GPRS_HeadSz, Data_Out, GSMTxSz);
	GSMTxSz = GSMTxSz + GPRS_HeadSz;
	GSM_SendFirstChar();
}
// ~~~~~~~~~
void GSM_SendData(uint8_t *Data_Out, uint8_t Size){
	GSMTxSz = Size;
	memcpy(GSM_TxStr, Data_Out, GSMTxSz);
	GSM_SendFirstChar();
}
// ~~~~~~~~~
void GSM_Execute_Command(prog_char *Str_P, uint16_t Timeout){
	if(GSM_Flag & (1<<flg_TxCStr)){
		GSM_SendCR(Str_P);
		StartTimer16(TD_GSM, Timeout);
	}
}
// ~~~~~~~~~
void GSM_DRE(void){
	if(GSM_TxCharN < GSMTxSz){
		UDR_GSM = GSM_TxStr[GSM_TxCharN];
		#ifdef GSM_DEBUG
			GPRS_TerminalProcess(GSM_TxStr[GSM_TxCharN]);
		#endif
		GSM_TxCharN++;
	}
	else{
		#if defined (__AVR_ATxmega128A1__)
			USART_GSM.CTRLA = USART_RXCINTLVL_HI_gc | USART_TXCINTLVL_HI_gc;
		#else
			UCSR_GSM_B =  (1<<RXCIE_GSM) | (1<<TXCIE_GSM) | (0<<UDRIE_GSM) | (1<<RXEN_GSM) | (0<<UCSZ_GSM_2) | (1<<TXEN_GSM);
		#endif
	}
}
// ~~~~~~~~~
void
GSM_TX(void)
{
	GSM_Flag |=(1<<flg_TxCStr);
	#if defined (__AVR_ATxmega128A1__)
		USART_GSM.CTRLA = USART_RXCINTLVL_HI_gc;
	#else
		UCSR_GSM_B =  (1<<RXCIE_GSM) | (0<<TXCIE_GSM) | (0<<UDRIE_GSM) | (1<<RXEN_GSM) | (0<<UCSZ_GSM_2) | (1<<TXEN_GSM);
	#endif
}

// ~~~~~~~~~~~~
void
GSM_Reset(void)
{
/*	IntOff();
	GSM_State = GSM_Restart;
	IntOn();*/
}
// ~~~~~~~~~~~~~~~~~~~~~~~~
inline static void GSM_Auto(){

	switch(GSM_State){

		case GSM_PowerOn:
			StartTimer16(TD_GSM,2000*GSM_DEBUG_DELAY);
			GSM_PWRCNTRL_ON();
			InitFIFO();
			GSM_State = GSM_WAIT_CALL_READY;
			break;
/*
		case GSM_WAIT_RDY:
			if(GSM_Wait_Response_P(URC_RDY, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_WAIT_CFUN:
			if(GSM_Wait_Response_P(URC_CFUN, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_WAIT_CPIN_READY:
			if(GSM_Wait_Response_P(URC_CPIN_READY, GSM_ReStart1)) GSM_State++;
			break;*/
		case GSM_WAIT_CALL_READY:
			if(GSM_Wait_Response_P(URC_CALL_READY, GSM_ReStart1)) GSM_State++;
			break;
		//------------------------
		case GSM_SEND_E0:
			GSM_Execute_Command(AT_E0,100*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_E0_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_SEND_CMGF:
			GSM_Execute_Command(AT_CMGF,100*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CMGF_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_SEND_CSMP:
			GSM_Execute_Command(AT_CSMP, 100*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CSMP_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_SEND_CIPMUX:
			GSM_Execute_Command(AT_CIPMUX, 100*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CIPMUX_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)){
				GSM_State++;
				// between "Call Ready" and "+CMGS"; 8 sec min needs; for kyivstar vpni only
				StartTimer16(TD_GSM, 1000);
			}
			break;
		//------------------------
		case GSM_WAIT_1:
			GetStringFromFIFO();		// чтобы не было переполнения FIFO
			if(Timer16Stopp(TD_GSM)){
				GSM_State++;
				GSM_Temp = 0;
			}
			break;
		case GSM_SEND_CGATT:
			GSM_Execute_Command(AT_CGATT, 500*GSM_DEBUG_DELAY); GSM_Temp++; GSM_State++;
			break;
		case GSM_WAIT_CGATT_OK:
			if(GSM_Temp>20) GSM_State = GSM_ReStart1;
			if(GSM_Wait_Response_P(RESP_OK, GSM_SEND_CGATT)) GSM_State++;
			break;

		case GSM_SEND_CIPCSGP:
			//GSM_Execute_Command(AT_CIPCSGP, 300*GSM_DEBUG_DELAY); GSM_State++;
			if(GSM_Flag & (1<<flg_TxCStr)){
				sprintf_P(GSM_TxStr, AT_CIPCSGP);	GSMTxSz = strlen_P(AT_CIPCSGP);
				#if LCDXSz==40
					GSMTxSz += strcpy_EE(&GSM_TxStr[GSMTxSz],GPRS_apn);
				#else
					GSMTxSz += strcpy_EE(&GSM_TxStr[GSMTxSz],GPRS_apn1);
					GSMTxSz += strcpy_EE(&GSM_TxStr[GSMTxSz],GPRS_apn2);
				#endif
				sprintf(GSM_TxStr + GSMTxSz, "\",\"");	GSMTxSz = GSMTxSz+3;				
				GSMTxSz += strcpy_EE(&GSM_TxStr[GSMTxSz],GPRS_apn_user_name);
				sprintf(GSM_TxStr + GSMTxSz, "\",\"");	GSMTxSz = GSMTxSz+3;
				GSMTxSz += strcpy_EE(&GSM_TxStr[GSMTxSz],GPRS_apn_psw);
				sprintf(GSM_TxStr + GSMTxSz, "\"\r");	GSMTxSz = GSMTxSz+2;
				GSM_SendFirstChar();
				StartTimer16(TD_GSM, 300*GSM_DEBUG_DELAY);
			}
			GSM_State++;
			break;
		case GSM_WAIT_CIPCSGP_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;

		case GSM_SEND_CSTT:
			GSM_Execute_Command(AT_CSTT, 300*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CSTT_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;

		case GSM_SEND_CIICR:
			GSM_Execute_Command(AT_CIICR, 12000*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CIICR_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;

		case GSM_SEND_CIFSR:
			GSM_Execute_Command(AT_CIFSR, 1000*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CIFSR_OK:
			if(Timer16Stopp(TD_GSM)) GSM_State = GSM_ReStart1;
			if(GetStringFromFIFO()){
				SetIP(&GSM_MyIP,0,0,0,0);
				for(uint8_t i=0; i<strlen(GSM_RxStr); i++)
					if(!isdigit(GSM_RxStr[i])) GSM_RxStr[i] = ' ';
				sscanf(GSM_RxStr,"%hhu %hhu %hhu %hhu", (unsigned char*)&GSM_MyIP.IP1, (unsigned char*)&GSM_MyIP.IP2, (unsigned char*)&GSM_MyIP.IP3, (unsigned char*)&GSM_MyIP.IP4);
				if(!IP_compare_Const(&GSM_MyIP,0,0,0,0)) GSM_State++;
				TempNum=0;	//
			}			
			break;
		//------------------------
		case GSM_SEND_CIPSTART:
			if(IP_E_compare_Const(&UDP_ServerIP[TempNum],0,0,0,0)){
				TempNum++;
				if(TempNum>3) GSM_State = GSM_SEND_CIPSERVER;
			}
			else{
				if(GSM_Flag & (1<<flg_TxCStr)){
					GSMTxSz = strlen_P(AT_CIPSTART);
					sprintf_P(GSM_TxStr, AT_CIPSTART);
					GSM_TxStr[12] = 0x30+TempNum;
					sprintf(GSM_TxStr + GSMTxSz, "\"%u.%u.%u.%u\",\"%u\"\r",(unsigned int)erb(&UDP_ServerIP[TempNum].IP1), (unsigned int)erb(&UDP_ServerIP[TempNum].IP2), 
						(unsigned int)erb(&UDP_ServerIP[TempNum].IP3), (unsigned int)erb(&UDP_ServerIP[TempNum].IP4), (unsigned int)erw(&UDP_ServerPort[TempNum]));
					GSMTxSz = strlen(GSM_TxStr);
					GSM_SendFirstChar();
					StartTimer16(TD_GSM, 500*GSM_DEBUG_DELAY);
					GSM_State++;
				}
			}
			break;
		case GSM_WAIT_CIPSTART_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_WAIT_CIPSTART_CONNECT_OK:
			if(Timer16Stopp(TD_GSM)) GSM_State = GSM_ReStart1;
			if(GetStringFromFIFO()){
				char RightAnswer[sizeof(RESP_CONNECT_OK)];
				strcpy_P(RightAnswer, RESP_CONNECT_OK);
				RightAnswer[0] = 0x30+TempNum;
				if(GSM_Wait_Response(RightAnswer, GSM_ReStart1)){
					if(++TempNum>3) GSM_State++;
					else GSM_State = GSM_SEND_CIPSTART;
				}
			}
			break;

		case GSM_SEND_CIPSERVER:
			if(GSM_Flag & (1<<flg_TxCStr)){
				GSMTxSz = strlen_P(AT_CIPSERVER);
				sprintf_P(GSM_TxStr, AT_CIPSERVER);
				sprintf(GSM_TxStr + GSMTxSz,"%u\r",(unsigned int)erw(&TCP_ListenPort));
				GSMTxSz = strlen(GSM_TxStr);
				GSM_SendFirstChar();
				StartTimer16(TD_GSM, 500*GSM_DEBUG_DELAY);
				GSM_State++;
				StartTimer16(TD_TCP_Connect, 65535);	// Перезапуск сервера каждые 655,35 сек
			}			
			break;
		case GSM_WAIT_CIPSERVER_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_WAIT_CIPSERVER_SERVER_OK:
			if(GSM_Wait_Response_P(RESP_SERVER_OK, GSM_ReStart1)) GSM_State = GSM_ServerIdle;
			break;

		//------------------------
		case GSM_ServerIdle:
			if( GPRS_FlgSz_Out && (GPRS_FlgSz_Out <= GPRS_Out_MaxSz) ){
				if(GPRS_Protocol_Out == UDP){
					TempNum = 0;
					GSM_State = GSM_SEND_CIPSEND;
					ResetTimer16(TD_GSM);
				}
				if(GPRS_Protocol_Out == TCP){
					if(GSM_ActiveConnection == NO_CONNECTION){
						GPRS_FlgSz_Out = 0;
					}
					else GSM_State = GSM_SEND_CIPSEND;
				}
				break;
			}
			if(SMS_FlgSz_Out && erb(&SMS_On)){
				GSM_State = GSM_SEND_CMGS;
				TempNum = 0;
				break;
			}
			
			if(GetStringFromFIFO()){
				GSM_State = GSM_AnalyzeURC;
				TempNum = 0;
				break;
			}

			if(Timer16Stopp(TD_TCP_Connect) ){
				if(GSM_ActiveConnection == NO_CONNECTION){
					GSM_State = GSM_SEND_SERVERCLOSE;
					break;
				} 
				else{
					GSM_Temp = GSM_ActiveConnection;
					GSM_State = GSM_TCP_CLIENT_DISCONNECT;
					break;				
				}
			}

			break;

		case GSM_AnalyzeURC:			//Анализ принятой строки

			// Приконектился клиент
			if( (strstr_P(GSM_RxStr, URC_REMOTE_IP) != NULL) ){
				GSM_State = GSM_TCP_CLIENT_CONNECT;
				break;
			}

			// Отконектился клиент
			if(strstr_P(GSM_RxStr, URC_CLOSED) != NULL){
				// читаем №конекта
				sscanf(GSM_RxStr,"%hhu, CLOSED",(unsigned char*)&TempNum);
				if(GSM_ActiveConnection == TempNum){
					GSM_ActiveConnection = NO_CONNECTION;
					SetIP(&GSM_ClientIP,0,0,0,0);
				}
				GSM_State = GSM_ServerIdle;
				break;
			}

			// Получили данные от клиента и предыдущие данные обработали
			if((strstr_P(GSM_RxStr, URC_RECEIVE) != NULL) && (!GPRS_FlgSz_In) ){
					//проверка на авторизацию соединения
					//чтение данных
				GSM_State = GSM_RECIEVE;
				break;
			}

			// Мусор
			GSM_State = GSM_ServerIdle;
			break;
		//------------------------

		case GSM_SEND_CIPSEND:
			if(GPRS_Protocol_Out == UDP){
				if(IP_E_compare_Const(&UDP_ServerIP[TempNum],0,0,0,0) || (IP_SendMask & (1<<TempNum)) ){
					TempNum++;
					if(TempNum>3){
						StartTimer16(TD_GSM,150);
						GSM_State = GSM_WAIT_CIPSEND_PAUSE;
						GPRS_FlgSz_Out = 0;
					}
				}
				else{
					if(GSM_Flag & (1<<flg_TxCStr)){
						if(Timer16Stopp(TD_GSM)){	// Пауза между посылками по UDP
							GSMTxSz = strlen_P(AT_CIPSEND);
							sprintf_P(GSM_TxStr, AT_CIPSEND);
							sprintf(GSM_TxStr + GSMTxSz, "%u,%u\r",(unsigned int)TempNum, (unsigned int)(GPRS_FlgSz_Out+GPRS_HeadSz));
							GSMTxSz = strlen(GSM_TxStr);
							GSM_SendFirstChar();
							StartTimer16(TD_GSM, 500*GSM_DEBUG_DELAY);
							GSM_State++;
						}
					}
				}
			}
			if(GPRS_Protocol_Out == TCP){
				if(GSM_Flag & (1<<flg_TxCStr)){
					GSMTxSz = strlen_P(AT_CIPSEND);
					sprintf_P(GSM_TxStr, AT_CIPSEND);
					sprintf(GSM_TxStr + GSMTxSz, "%u,%u\r",(unsigned int)GSM_ActiveConnection, (unsigned int)(GPRS_FlgSz_Out+GPRS_HeadSz));
					GSMTxSz = strlen(GSM_TxStr);
					GSM_SendFirstChar();
					StartTimer16(TD_GSM, 500*GSM_DEBUG_DELAY);
					GSM_State++;
				}				
			}
			break;
		case GSM_WAIT_Inv:
			if(GSM_Wait_Char('>', GSM_ReStart1)) GSM_State++;
			break;
		case GSM_SEND_DATA:
			//	memcpy (void  dest, const void  src, size_t len)
			GSM_AddHeaderSendData(GPRS_Data_Out, GPRS_FlgSz_Out);
			GSM_State++;
			break;
		case GSM_WAIT_CIPSEND_SEND_OK:
			if(Timer16Stopp(TD_GSM)) GSM_State = GSM_ReStart1;
			if(GetStringFromFIFO()){
				if(GPRS_Protocol_Out == UDP){
					char RightAnswer[sizeof(RESP_SEND_OK)];
					strcpy_P(RightAnswer, RESP_SEND_OK);
					RightAnswer[0] = 0x30+TempNum;
					if(GSM_Wait_Response(RightAnswer, GSM_ReStart1)){
						StartTimer16(TD_GSM,300);//min~ 2 sec
						if(++TempNum>3){
							GSM_State = GSM_WAIT_CIPSEND_PAUSE;
							GPRS_FlgSz_Out = 0;
						}
						else GSM_State = GSM_SEND_CIPSEND;
					}
				}
				if(GPRS_Protocol_Out == TCP){
					char RightAnswer[sizeof(RESP_SEND_OK)];
					strcpy_P(RightAnswer, RESP_SEND_OK);
					RightAnswer[0] = 0x30+GSM_ActiveConnection;
					if(GSM_Wait_Response(RightAnswer, GSM_ReStart1)){
						GPRS_FlgSz_Out = 0;
						GSM_State = GSM_ServerIdle;
					}
				}
			}	
			break;
		case GSM_WAIT_CIPSEND_PAUSE:// Пауза необходима если одновременно отправка данных и SMS
			if(Timer16Stopp(TD_GSM)){
				GSM_State = GSM_ServerIdle;
			}
			break;
		//------------------------
		case GSM_SEND_CMGS:
			if(GSM_Flag & (1<<flg_TxCStr)){			
				erbl(GSM_TxStr, SMS_Number+TempNum, MaxTelephN);
				GSMTxSz = strlen(GSM_TxStr);
				uint8_t j=0;
				for(uint8_t i=0; i<GSMTxSz; i++)
					if(GSM_TxStr[i] != '0') j++;
				if(j){
					GSMTxSz = strlen_P(AT_CMGS);
					sprintf_P(GSM_TxStr, AT_CMGS);
					erbl(GSM_TxStr+GSMTxSz, SMS_Number+TempNum, MaxTelephN);
					GSMTxSz = strlen(GSM_TxStr);
					sprintf(GSM_TxStr + GSMTxSz, "\"\r");
					GSMTxSz = strlen(GSM_TxStr);
					GSM_SendFirstChar();
					StartTimer16(TD_GSM, 500*GSM_DEBUG_DELAY);
					GSM_State++;
				}
				else{
					TempNum++;
					if(TempNum>=MaxTelephDirSz){
						//StartTimer16(TD_GSM,150);
						GSM_State = GSM_ServerIdle;
						SMS_FlgSz_Out = 0;
					}					
				}
			}
			break;
		case GSM_WAIT_SMS_Inv:
			if(GSM_Wait_Char('>', GSM_ReStart1)) GSM_State++;
			break;
		case GSM_SEND_SMS_DATA:
			SMS_Data_Out[SMS_FlgSz_Out] = 0x1A;	// SMS Data end - char Ctrl-Z
			GSM_SendData((uint8_t *)SMS_Data_Out, SMS_FlgSz_Out+1);
			GSM_State++;
			break;		
		case GSM_WAIT_CMGS_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)){
				if(++TempNum>=MaxTelephDirSz){
					GSM_State = GSM_ServerIdle;
					SMS_FlgSz_Out = 0;
					//StartTimer16(TD_GSM,150);
				}
				else GSM_State = GSM_SEND_CMGS;
			}			
			break;
		//------------------------		
		case GSM_TCP_CLIENT_CONNECT: 
			// читаем №конекта и IP			
			sscanf(GSM_RxStr,"%hhu, REMOTE IP:%hhu.%hhu.%hhu.%hhu",(unsigned char*)&GSM_Temp, 
				(unsigned char*)&GSM_ClientIP.IP1, (unsigned char*)&GSM_ClientIP.IP2, (unsigned char*)&GSM_ClientIP.IP3, (unsigned char*)&GSM_ClientIP.IP4);
			
			// только один клиент может приконектится
			if( GSM_ActiveConnection != NO_CONNECTION ){
				GSM_State = GSM_TCP_CLIENT_DISCONNECT;
				break;
			}
			// сравниваем IP с ОДЗ клиентов
			TempNum = 0;
			for(uint8_t i=0; i<4; i++){
				if(IP_E_compare_Const(&TCP_ClientIP[i],255,255,255,255)){	// Anybody may conect
					TempNum = 1;
					break;
				}
				if(!IP_E_compare_Const(&TCP_ClientIP[i],0,0,0,0)){
					if(IP_E_compare_Const(&TCP_ClientIP[i], GSM_ClientIP.IP1, GSM_ClientIP.IP2, GSM_ClientIP.IP3, GSM_ClientIP.IP4)){
						TempNum = 1;
						break;
					}
				}
			}
			if(TempNum){
				GSM_State = GSM_ServerIdle;
				GSM_ActiveConnection = GSM_Temp;
				StartTimer16(TD_TCP_Connect, erw(&TCP_CONNECT_timeout));	// запускаем таймер таймаута
			}
			else GSM_State = GSM_TCP_CLIENT_DISCONNECT;
			break;
		case GSM_TCP_CLIENT_DISCONNECT:
			// Not allowaeble Client IP			
			if(GSM_Flag & (1<<flg_TxCStr)){
				GSMTxSz = strlen_P(AT_CIPCLOSE_1);
				sprintf_P(GSM_TxStr, AT_CIPCLOSE_1);
				sprintf(GSM_TxStr + GSMTxSz, "\r");
				GSMTxSz++;
				GSM_TxStr[12] = 0x30 + GSM_Temp;
				GSM_SendFirstChar();
				StartTimer16(TD_GSM, 500*GSM_DEBUG_DELAY);
				GSM_State++;
			}			 
			break;			
		case GSM_WAIT_CLIENT_CIPCLOSED:
			if(Timer16Stopp(TD_GSM)) GSM_State = GSM_ReStart1;
			if(GetStringFromFIFO()){
				char RightAnswer[sizeof(RESP_CLOSE_OK_FAST)];
				strcpy_P(RightAnswer, RESP_CLOSE_OK_FAST);
				RightAnswer[0] = 0x30+GSM_Temp;
				if(GSM_Wait_Response(RightAnswer, GSM_ReStart1)){
					GSM_State = GSM_ServerIdle;
					if( (GSM_ActiveConnection != NO_CONNECTION) && Timer16Stopp(TD_TCP_Connect) ){	//закрыть ТСР-соединение по таймауту
						GSM_ActiveConnection = NO_CONNECTION;
						SetIP(&GSM_ClientIP,0,0,0,0);
					}
				}
			}
			break;
		//------------------------
		case GSM_RECIEVE:
			{ 
				uint8_t TempConnect;
				sscanf(GSM_RxStr,"+RECEIVE,%hhu,%hhu",(unsigned char*)&TempConnect, (unsigned char*)&GSM_Temp);
				// принимаем данные только от GSM_ActiveConnection // поверка данных по длине
				if( (TempConnect == GSM_ActiveConnection) && (GSM_Temp <= (GPRS_In_MaxSz + GPRS_HeadSz)) && (GSM_Temp >= 7) ){
					StartTimer16(TD_GSM, 500*GSM_DEBUG_DELAY);
					GSM_State++;
					break;
				}
				GSM_State = GSM_ServerIdle;
			}
			break;
		case GSM_RECIEVE_DATA:
			if(Timer16Stopp(TD_GSM)) GSM_State = GSM_ReStart1;			
			if(GetDataFromFIFO(GSM_Temp) == GSM_Temp){

				if( (GSM_RxStr[0]==5) &&  (GSM_RxStr[1]==1) ){
					if( (((uint16_t)GSM_RxStr[2]<<8) + GSM_RxStr[3]) == GSM_PASSWORD){
						GPRS_FlgSz_In = GSM_Temp - GPRS_HeadSz;
						memcpy(GPRS_Data_In, GSM_RxStr + GPRS_HeadSz, GPRS_FlgSz_In);
						StartTimer16(TD_TCP_Connect, erw(&TCP_CONNECT_timeout));	// перезапускаем таймер таймаута
					}
				}
				GSM_State = GSM_ServerIdle;
			}
			break;
		//------------------------
		case GSM_SEND_CIPCLOSE:
			GSM_Execute_Command(AT_CIPCLOSE, 500*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CIPCLOSE_OK:
			if(GSM_Wait_Response_P(RESP_CLOSE_OK, GSM_ReStart1)){
				
				GSM_State = GSM_ServerIdle;
				GPRS_FlgSz_Out = 0;
			}
			break;
		//------------------------

		
		//------------------------
		case GSM_ReStart1:
			GSM_PWRCNTRL_OFF();
			StartTimer16(TD_GSM,1000);
			RxBufOverFlow = 0;
//			InitFIFO();
			SMS_FlgSz_Out = 0;	// т.к. если неправильный хотя бы один номер будет отсылать бесконечно - трата денег
			GSM_ActiveConnection = NO_CONNECTION;
			SetIP(&GSM_ClientIP,0,0,0,0);
			GSM_Flag |=(1<<flg_TxCStr);
			GSM_State++;
			break;
		case GSM_ReStart2:
			if(Timer16Stopp(TD_GSM)){
				GSM_State = GSM_PowerOn;
			}
			break;
		//------------------------


		//------------------------ Перезапуск сервера
		case GSM_SEND_SERVERCLOSE:
			GSM_Execute_Command(AT_SERVERCLOSE, 500*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_SERVERCLOSE_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_WAIT_SERVER_CLOSE:
			if(GSM_Wait_Response_P(RESP_SERVER_CLOSE, GSM_ReStart1)) GSM_State = GSM_SEND_CIPSERVER;
			break;

		//------------------------

	}//switch

}
// ~~~~~~~~~~~~~~~~~~~~~~~~
inline static void GSM_Manual(){}
// ~~~~~~~~~~~~~~~~~~~~~~~~
inline static void GSM_Config(){
	switch(GSM_State){
		case GSM_CFG_START:
			StartTimer16(TD_GSM,200);
			GSM_PWRCNTRL_ON();
			GSM_State++;
			break;
		case GSM_CFG_SEND_AT:
			if(Timer16Stopp(TD_GSM)) GSM_Execute_Command(AT_AT, 100); GSM_State++;
			break;
		case GSM_CFG_WAIT_AT_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_CFG_SEND_AT)) GSM_State++;
			break;
		case GSM_CFG_SEND_IPR:
			GSM_Execute_Command(AT_IPR9600, 100); GSM_State++;
			break;
		case GSM_CFG_WAIT_IPR_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_CFG_SEND_AT)){
				StartTimer16(TD_GSM,200);
				GSM_State++;
			}
			break;
		case GSM_SAVE_IPR:
			if(Timer16Stopp(TD_GSM)) GSM_State++;
			break;
		case GSM_FINISH:			
			break;
		default:
			GSM_State = GSM_CFG_START;
	}//switch
}
// ~~~~~~~~~~~~~~~~~~~~~~~~
void
GSM_Cycle(void)
{
	switch(GSM_Control){
		case GSM_CNTL_AUTO:
			GSM_Auto();
			break;
		case GSM_CNTL_MANUAL:
			GSM_Manual();
			break;	
		case GSM_CNTL_CONFIG:
			GSM_Config();
			break;								
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~
void
GPRS_ServerAddr_Save(void)
{

}
// ~~~~~~~~~~~~~~~~~~~~~~~~

