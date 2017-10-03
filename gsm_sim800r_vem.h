/* ~~~~~~~~~~~~~
// 	Modem: SIM900R, 9600 bps,N,1. 	��� EMReader.
	AT+CGMR = Revision:1137B01SIM900R64_ST_ENHANCE_EAT - �� ������! ��������� ������!

	����� ���� - ������������� ��������
	����� ���� - ����� ������ �������� AT ������� � �������
	����� ����� - ������������� �������� 9600
	
	����� 
	������ ���������� UDP-������� �� 
	 ���� GPRS_FlgSz_Out!=0
	������ �� ���������� �� IP=0.0.0.0 ��� ���� ����������� ������� IP_SendMask!

	����� �������� SMS. SMS �� ������������ �� ����� 00000000000000!


	��� ����� ��������� ������: xxxxxxxxxxxxxxxx<CR><LF>
	'\r' CR - carridge return 0x0D
	'\n' LF - line feed 0x0A
	'\0' LF - ����� ������ 0x00
	������: ATE0<CR><LF>

	��� ��������� - �.�.����� ��������� �� ���� �� ��, ��� ��� ������ �� UART RX, ������ �����������

	A.����� ���������� �������� ������: 9600 baud.
	AT+IPR=9600
	AT&W (������� ������������ � ��������������� �����)
	�� ������������ ����� 115200bps �� ����� ������������� ���������� �������� (autobaud).
	
	B.SIM-�����
	³������� ����� PIN-����

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

	��������� ��������:
	���� �� RX ��� ������(�������). �������, ���� ������������� ������� ������ ����� GSM_RxStr, �� ��� ������ � ���� ������ ������������
	� ����������� �������� �������	RxBufOverFlow. �.�. ��� ������� �������� ������� ������, ��� ������� scan cycle, �������� 
	������������� ��������.

	�������� ��������� ������ ����������� AT-������ ������� ���-��'\n' == 2
	�������� �� ��������� ������ �� AT+CIICR,AT+CIPSERVER, ������ ���������� ����������� ������� ������
		ERROR,OK �.�. ���� 'R' - ������ ����� - ������ �� AT+CIPSHUT
	
	20 ��� �������� ������������ � ���� AT+CGATT=1. �������� �.�.� ������� ���� ����� ������� �� ������ ���� ���������.

	24.12.2012	- ������� ����� 3 ��� ����� ���������� �� UDP(�.�. �������� �� IP2)

	25.01.2013	- ��������������� GPRS_TerminalProcess �� ����� 4x20 � 8x40

	15.10.2013	- ������� ��� TCP-���������� ��� ��������� ���������� TCP-��������,
				- ���������� APN(2-� ������), USER_NAME, PASSWORD
				- ������ ���� AT-������ ��� define GSM_DEBUG

	04.11.2013	- ������� ��� GSM_WAIT_CALL_READY �������� �� 20 ���

	04.02.2014	- ������� ��� GSM_SEND_CIICR �������� �� 120 ��� (��� APN test.umc.ua ����� ���������� �� 43 ���)

	13.02.2014	- ��� ���������� IP �������: ���� 255.255.255.255 - ������������� ����� ��� ������

	04.03.2014	- ��������� (1��� � 655,35���) ��������������� Server (������ ������ ����� Server �� �������)
					����� ���������� ���������� ����� ������������ TCP �������� �������� (TCP_CONNECT_timeout)
	
	15.07.2014 - ��������� (1��� � 15���) ������������ ������� GSM-�������
					���� �������� � ������� GSM_WAIT_Inv,GSM_WAIT_CIPSEND_SEND_OK
					�� ����������� (GSM_ReStart1), � ��������� � GSM_ServerIdle
	
	19.08.2014 - ��� ������ uint8_t sscanf(str,"%hhu") 1-����. ������ sscanf(str,"%hu"): �.�. 2 ����� ����� � ������

	07.04.2015 - ����� �������� �� ����� ��������� IP GSM_WAIT_CIFSR_OK. ��������� if(Timer16Stopp(TD_GSM)) GSM_State = GSM_ReStart1;
				- ������ ����� ������ 61 ��� TD_GSM_Reset. (������ ������ ����� ����� ����������, ��� ������� ��������� 
				�����������, �� ������ ���� ������������ � TCP/IP �������! )
***********	http

	06.05.2015 - ��������� TCP/IP-���������� � ���-��������
				��� ������� �������� �������� GSM_RXSTR_SIZE = 100 (����� 50)

	14.09.2015 - ���������� ���������� �������: CTS/RTS #define GSM_HW_FLOW_CONTROL - �� ��������! (��.webserver.h)

	18.09.2015 - � ��������� GSM_WAIT_CIPCLOSE_OK ��� ��� CLOSE OK ��� CLOSED

	22,09,2015 - ������������� ��������� ������ GSM_RxStr ��������� <CR><LF> uint8_t ForceEndStringFromFIFO(void)
	
	25,09,2015 - ����������� ��������� �� ��������� (������ ��������� ����) uint8_t AppProtocol � ������ GSM_ProtocolMode
				GSM_MultiCon
				������ ����� ICCID ��� �������������. ���������� sscanf_P(Buf*, PSTR(...))

	29,09,2015 - ������ �� ��������� � GSM_DataMode ��� if(AppProtocol != _HTTP)
	
	30.09.2015 - ����������� �� vem

		��� ����������� � GPRS, ����� ��������� � ��������� ����� ( =1).
		������������ ����� ������������������� ��� �������� ������ � ������� ������ � ���������� �����.
		���������� �� ��������������� ���� UART_Soft==1
		� ���������� ����� �� ������� FIFO ��������� � UART E_METER(�� �����������)  � ������� � GSM_RxStr ��� ������� ���� �� �������� �� ������

	01.10.2015 - ������� ������ ICCID2 = 0. �������� ������� ������ GSM_WAIT_CCID_READ
				
	02.10.2015 - GSM_StateBeforeReset

	05.11.2015 - �� ����� WebSession - �� ����������� ������� �������� � ������ GSM_ServerIdle
				���������� ������ ������ 24� ��� (����� 61 ���)

	23.02.2016 - �������� ����� CSD � ������ ����������� ������� CSD_AllowedNumbers!

	04.04.2016 - � GSM_CloseTransparent ������������� ������ TD_TCP_Connect �� 655,35 ���

	12.07.2016 - ��������� �������������� GPRS ����� �������� ������ ��� ����

~~~~~~~~~~~~~*/
void EM_InitFIFO(void);
void SendDebug(uint8_t Char);
inline void SetDebug_DRE_ISR(void);
void EMeter_ReInit(void);
void WebClose(void);
uint8_t IsWebSession(void);
uint8_t strcmp_E(char* str_RAM, char* str_EE);
uint8_t sendsWithoutReconnect=0;

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "ip.h"	// ������ � IP-��������

uint8_t GSM_Modem;
	#define NOT_RECOGNIZED	0
	#define SIMCOM_SIM900R	1
	#define SIMCOM_SIM800	2

prog_char GSM_SIMCOM_SIM900R[]	= "SIMCOM_SIM900R";
prog_char GSM_SIMCOM_SIM800[]	= "SIMCOM_SIM800";

#define SIM900R
//#define GSM_HW_FLOW_CONTROL

//#define GSM_DEBUG	//���������� �������, 
//#define GSM_DEBUG_DELAY 1	//���������  �  ��� ������� 

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
		#define GPRS_APN		"vpni.kyivstar.net            "	//29��������
	#endif
	#ifndef GPRS_USER_NAME
		#define GPRS_USER_NAME 	"                   "	//19��������
	#endif
	#ifndef GPRS_PSW
		#define GPRS_PSW 		"                   "	//19��������
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

//URC
prog_char URC_RDY[]	= "RDY";
prog_char URC_CFUN[] = "+CFUN: 1";
prog_char URC_CPIN_READY[] = "+CPIN: READY";
prog_char URC_CALL_READY[] = "Call Ready";
prog_char URC_SMS_READY[] = "SMS Ready";
prog_char URC_REMOTE_IP[] = "REMOTE IP";
prog_char URC_CONNECT[] = "CONNECT";
prog_char URC_RECEIVE[] = "+RECEIVE,";
prog_char URC_CLOSED[] = ", CLOSED";
prog_char URC_PDPDEACT[] = "+PDP DEACT";
prog_char URC_RING[] = "RING";
prog_char URC_NO_CARRIER[] = "NO CARRIER";

// AT-commands RESPONSE
prog_char RESP_OK[] = "OK";
prog_char RESP_CONNECT_OK[] = "x, CONNECT OK";
prog_char RESP_SERVER_OK[]	= "SERVER OK";
prog_char RESP_SERVER_CLOSE[] = "SERVER CLOSE";
prog_char RESP_Invitation[] = ">";
prog_char RESP_SEND_OK[] = "x, SEND OK";
prog_char RESP_CLOSE_OK[] = "CLOSE OK";
prog_char RESP_CLOSE_OK_FAST[] = "x, CLOSE OK";
prog_char RESP_CSQ[] = "+CSQ:";
prog_char RESP_SHUT_OK[] = "SHUT OK";
prog_char RESP_CONNECT_9600[] = "CONNECT 9600";

prog_char ESC_SEQ[] = "+++";


// ~~~~~~~~~~~
// AT-commands
// ~~~~~~~~~~~

// Config
prog_char AT_AT[]			= "AT";
prog_char AT_IPR9600[]		= "AT+IPR=9600";
prog_char AT_W[]			= "AT&W";

// Total
prog_char AT_E0[]			= "ATE0";
prog_char AT_CGMM[]			= "AT+CGMM";
prog_char AT_CLIP[]			= "AT+CLIP=1";
prog_char AT_CCID[]			= "AT+CCID";
prog_char AT_IFC[]			= "AT+IFC=2,2";

// SMS
prog_char AT_CMGF[]			= "AT+CMGF=1";
prog_char AT_CSMP[]			= "AT+CSMP=17,167,0,0";

prog_char AT_CMGS[]			= "AT+CMGS=\"+";

// GPRS
prog_char AT_CIPMODE_0[]	= "AT+CIPMODE=0";
prog_char AT_CIPMODE_1[]	= "AT+CIPMODE=1";
prog_char AT_CIPMUX_0[]		= "AT+CIPMUX=0";
prog_char AT_CIPMUX_1[]		= "AT+CIPMUX=1";

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

prog_char AT_CIPCLOSE[]		= "AT+CIPCLOSE=1";	//������������ ������ ������� ����� �������� TCP/IP �����������
prog_char AT_CIPSHUT[]		= "AT+CIPSHUT";
prog_char AT_CGATT_0[]		= "AT+CGATT=0";

prog_char AT_CSQ[]			= "AT+CSQ";
prog_char AT_ATA[]			= "ATA";
prog_char AT_ATH[]			= "ATH";


// Auto
enum {
	GSM_PowerOn,
	GSM_WAIT_RDY,
	GSM_SEND_CGMM,
 	GSM_WAIT_CGMM_READ,
	GSM_WAIT_CFUN,
	GSM_WAIT_CPIN_READY,
	GSM_WAIT_CALL_READY,
	GSM_WAIT_SMS_READY,
	
	GSM_SEND_E0, 		GSM_WAIT_E0_OK,
	GSM_SEND_CLIP, 		GSM_WAIT_CLIP_OK,
	GSM_SEND_CCID, 		GSM_WAIT_CCID_READ,	GSM_WAIT_CCID_OK,
//	GSM_SEND_IFC, 		GSM_WAIT_IFC_OK,
	GSM_SEND_CMGF,		GSM_WAIT_CMGF_OK,
	GSM_SEND_CSMP,		GSM_WAIT_CSMP_OK,
	GSM_SEND_CIPMUX, 	GSM_WAIT_CIPMUX_OK,
	GSM_SEND_CIPMODE, 	GSM_WAIT_CIPMODE_OK,

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

	GSM_CIPSHUT_, GSM_ShutWait_, GSM_SEND_CGATT0, GSM_WAIT_CGATT0_OK,

	GSM_CMGS, GSM_SMS,

	GSM_ReStart1, GSM_ReStart2,
	
	GSM_SEND_SERVERCLOSE,	GSM_WAIT_SERVERCLOSE_OK,	GSM_WAIT_SERVER_CLOSE,	 

	GSM_SEND_CSQ, 	GSM_WAIT_CSQ,
	
	GSM_ProtocolMode, GSM_DataMode,	GSM_Swtch2CommandMode,	GSM_Swtch2CommandModePlus,	GSM_WAIT_D2C_SWITCH_OK,
	
	GSM_CheckNumber,	GSM_SEND_ATA, GSM_WAIT_CONNECT_9600,

	GSM_SEND_ATH, GSM_WAIT_ATH_OK
};

// Config
enum {
	GSM_CFG_START,
	GSM_CFG_SEND_AT, 		GSM_CFG_WAIT_AT_OK,
	GSM_CFG_SEND_IPR,		GSM_CFG_WAIT_IPR_OK,
	GSM_CFG_SEND_ATW,		GSM_CFG_WAIT_ATW_OK,
	GSM_SAVE_IPR,			GSM_FINISH 
};

volatile uint8_t GSM_State;
uint8_t GSM_StateBeforeReset;

volatile uint8_t GSM_Control;	// ����� ���������� �������
	#define GSM_CNTL_AUTO	0	// ������� (�� ���������)
	#define GSM_CNTL_MANUAL	1	// ������� (����� �������)
	#define GSM_CNTL_CONFIG	2	// ��������� ������������ (��������� ������� UART ������)
		
volatile uint8_t GSM_Flag = 0b00000001;
	#define flg_TxCStr	0	//�������� ������ ���������


volatile uint8_t RxBufOverFlow;

#if defined (__AVR_ATmega16__) ||  defined (__AVR_ATmega32__)
	#define GPRS_Out_MaxSz	(10)
	#define GPRS_In_MaxSz	(10)
#elif defined (__AVR_ATmega128__) || defined (__AVR_ATmega2561__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)  || (__AVR_ATxmega128A1__)
	#define GPRS_Out_MaxSz	(0xFF)
	#define GPRS_In_MaxSz	(50)
#endif

static uint8_t TD_GSM, TD_TCP_Connect, TD_RSSI, TD_GSM_Reset;

#define GSM_RXSTR_SIZE 100	// ������������ ����� ������ (�� ������!) �� ������
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
uint8_t IP_SendMask;	// ��������� ��������� ���� ��� ���������� : 0bit-UDP IP1, 1bit-UDP IP2, 2bit-UDP IP3, 3bit-UDP IP4, 
uint8_t TempNum;
uint8_t GSM_Temp;
uint8_t GSM_RSSI, GSM_BER;	//RSSI - recieved signal strengh indicator BER - bit error rate
uint8_t GSM_MultiCon;
uint64_t ICCID;
uint16_t ICCID1;
uint32_t ICCID2; 

// ~~~~~~~~~~~
//FIFO
#define GSM_RX_FIFO_SIZE 250		// ��� �������� �� 5�� ScanCycle � GPRS_In_MaxSz =< 50
uint8_t GSM_RX_FIFO[GSM_RX_FIFO_SIZE];
volatile uint8_t GSM_RX_FIFO_Begin;	//������ ���������� ��������� �����
uint8_t GSM_RX_FIFO_End;	//������ ���������� ����������� �����
volatile uint8_t GSM_RX_FIFOOverFlow;
volatile uint8_t GSM_RX_FIFOMax;

volatile uint8_t GSM_RX_FIFO_End_Transp;		//������ ���������� ����������� ����� � ���������� ������
volatile uint8_t GSM_RX_FIFOOverFlow_Transp;
volatile uint8_t GSM_RX_FIFOMax_Transp;

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

uint16_t TCP_CONNECT_timeout EEMEM = 7500;


#ifndef VEGA_SN
	#define VEGA_SN 0
#endif
uint16_t Vega_SN EEMEM = VEGA_SN;


#ifndef GSM_PASSWORD
	#define GSM_PASSWORD 22780
#endif
uint16_t GSM_Password EEMEM = GSM_PASSWORD;

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

#ifndef CSD_ALLOWED_NUMBERS_INIT
	#define CSD_ALLOWED_NUMBERS_INIT {"000000000000", "000000000000", "000000000000", "000000000000", \
	"000000000000", "000000000000",	"000000000000", "000000000000", "000000000000", "000000000000"}
#endif
char CSD_AllowedNumbers[10][MaxTelephN] EEMEM = CSD_ALLOWED_NUMBERS_INIT;
//char CSD_AllowedNumbers[MaxTelephN] EEMEM = "380501696928";

uint8_t SMS_On EEMEM = 0;
uint8_t CLIP_On EEMEM = 1;

uint8_t AppProtocol;
	#define _HTTP 1
	#define _VFCP 2

uint8_t Transparent;	// ������� ����������� ������

uint8_t GSM_CSD;


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
//	GSM_RX_FIFOOverFlow = 0;
//	GSM_RX_FIFOMax = 0;
}
// ~~~~~~~~~~~
void GetByteFromFIFO(uint8_t *ReadBuf, uint8_t *Index, uint8_t ReadBuf_Sz){

	//Reset *Index if read buffer overflow
	if(*Index >= ReadBuf_Sz){
		*Index = 0;
		//TODO ���������� ����
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
inline void GetByteFromGSM_FIFO_Transp(void){

	if(GSM_RX_FIFO_Begin != GSM_RX_FIFO_End_Transp){
		GSM_RX_FIFO_End_Transp++;
		if(GSM_RX_FIFO_End_Transp >= GSM_RX_FIFO_SIZE) GSM_RX_FIFO_End_Transp = 0;
		PORTD |= (1<<PD4);	//RS485 Tx_on
		UDR_EMETER = GSM_RX_FIFO[GSM_RX_FIFO_End_Transp];
		UCSR_EMETER_A |= (1<<TXC_EMETER);	//Clear TxC pending interrupt
	}
	else{
		// disable DRE interrupt & enable TxC interrupt
		UCSR_EMETER_B = (1<<RXCIE_EMETER) | (1<<TXCIE_EMETER) | (0<<UDRIE_EMETER) | (1<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);

	}

	//--- Calculate GSM_RX_FIFOMax_Transp
	if(GSM_RX_FIFO_Begin > GSM_RX_FIFO_End_Transp){
			if(GSM_RX_FIFOMax_Transp < (GSM_RX_FIFO_Begin - GSM_RX_FIFO_End_Transp))
				GSM_RX_FIFOMax_Transp =  GSM_RX_FIFO_Begin - GSM_RX_FIFO_End_Transp;
	}
	if(GSM_RX_FIFO_End_Transp > GSM_RX_FIFO_Begin){
			if(GSM_RX_FIFOMax_Transp < (GSM_RX_FIFO_SIZE - GSM_RX_FIFO_End_Transp  + GSM_RX_FIFO_Begin))
				GSM_RX_FIFOMax_Transp = GSM_RX_FIFO_SIZE - GSM_RX_FIFO_End_Transp  + GSM_RX_FIFO_Begin;
	}
}

// ~~~~~~~~~~~
// ���������� ����� ������ � ������ ������� ���������� ������
uint8_t GetStringFromFIFO(void){

	uint8_t StrLength = 0;

	while(GSM_RX_FIFO_Begin != GSM_RX_FIFO_End){
		
		GetByteFromFIFO((uint8_t *)GSM_RxStr, (uint8_t *)&GSM_RxCharN, GSM_RXSTR_SIZE);

		//��� ����� ��������� ������: xxxxxxxxxxxxxxxx<CR><LF>
		//	'\r' CR - carridge return 0x0D
		//	'\n' LF - line feed 0x0A
		//����������� � ������ xxxxxxxxxxxxxxxx<\0>
		if( (GSM_RxCharN >= 2) && (GSM_RxStr[GSM_RxCharN-2]== '\r') && (GSM_RxStr[GSM_RxCharN-1]== '\n') ){
			GSM_RxStr[GSM_RxCharN-2] = '\0';
			StrLength = GSM_RxCharN-1;
			GSM_RxCharN = 0;
			// Ignore GSM_RxStr like (0x0D 0x0A)
			return StrLength;
		}

	}//while

	return StrLength;
}
// ~~~~~~~~~~~
// ������������� ��������� ������ GSM_RxStr ��������� <CR><LF>
uint8_t ForceEndStringFromFIFO(void){
	uint8_t StrLength;
	if(GSM_RxCharN > GSM_RXSTR_SIZE-1) GSM_RxCharN = GSM_RXSTR_SIZE-1;
	StrLength = GSM_RxCharN;
	GSM_RxStr[GSM_RxCharN]='\0';
	GSM_RxCharN = 0;
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
// �������� �� ������ EEPROM � ������ RAM �� ������� ������� ��� '\0' ��� ������ ������ EEPROM
uint8_t strcpy_EE(char* str_RAM, uint8_t* str_EE){
	uint8_t i=0;
	while( (erb(&str_EE[i])!='\0') && (erb(&str_EE[i])!=' ') ){
		str_RAM[i] = (char)erb(&str_EE[i]);
		i++;
	}
	return i;	
}
// ~~~~~~~~~~~
void GSM_CloseTransparent(void)
{	
	if(Transparent){
		
		cli();		
		Transparent = 0;
		EMeter_ReInit();
		sei();
		
	}
	StartTimer16(TD_TCP_Connect, 65535);	// ���������� ������� ������ 655,35 ���
}
// ~~~~~~~~~~~
void GSM_Transparent2Modbus(void)
{
	GSM_CloseTransparent();	
	GSM_State = GSM_Swtch2CommandMode;
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
		USART_GSM.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABl_gc | (0<< USART_SBMODE_bp) | USART_CHSIZE_8BIT_gc;
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
	#ifdef GSM_HW_FLOW_CONTROL
		GSM_CTS_RTS_Config();
	#endif

	TD_GSM = Timer16SysAlloc(1);
	TD_TCP_Connect = Timer16SysAlloc(1);
	TD_RSSI = Timer16SysAlloc(1);
	TD_GSM_Reset = Timer32SysAlloc(1);

/*	for (uint8_t i=0; i<4; i++)
		GPRS_ServerAddr[i] = erb(GPRS_ServerAddr_EE+i);
*/
	GSM_Control = GSM_CNTL_AUTO;
	GSM_State = GSM_PowerOn;

/*	GSM_Control = GSM_CNTL_CONFIG;
	GSM_State =	GSM_CFG_START;*/



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

	//---In Transparent Mode
	if(Transparent){
		// enable DRE interrupt for UDR1
		UCSR_EMETER_B = (1<<RXCIE_EMETER) | (0<<TXCIE_EMETER) | (1<<UDRIE_EMETER) | (1<<RXEN_EMETER) | (0<<UCSZ_EMETER_2) | (1<<TXEN_EMETER);
		UCSR_EMETER_A |= (1<<TXC_EMETER);	//Clear TxC pending interrupt

		if(GSM_RX_FIFO_Begin == GSM_RX_FIFO_End_Transp){
			GSM_RX_FIFOOverFlow_Transp = 1;
			GSM_RX_FIFOMax_Transp = GSM_RX_FIFO_SIZE;
		}

	}

	#ifdef GSM_DEBUG
		GPRS_TerminalProcess(Char);
	#endif
	if(GSM_DebugMode){
		SendDebug(Char);
		SetDebug_DRE_ISR();
	}

}
// ~~~~~~~~~
//�������� ������ ����� GetStringFromFIFO()!
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
	//������ �������� ������
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
	//������ ������� ��������� �����
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
	if(GSM_DebugMode){
		SendDebug(GSM_TxStr[0]);
		cli();
		SetDebug_DRE_ISR();
		sei();
	}

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
//���������� �������� ������ + CR
void GSM_SendCR(prog_char *Str_P){
	GSMTxSz = strlen_P(Str_P);
	sprintf_P(GSM_TxStr, Str_P);
	sprintf(GSM_TxStr + GSMTxSz, "\r");
	GSMTxSz++;
	GSM_SendFirstChar();
}
// ~~~~~~~~~
//���������� �������� ������ + CntrZ
void GSM_AddHeaderSendData(uint8_t *Data_Out, uint8_t Size){
	GSMTxSz = Size;
	memcpy(GSM_TxStr, Data_Out, GSMTxSz);
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

	if(Transparent){
		GetByteFromEM_FIFO_Transp();
	}	
	else{
		if(GSM_TxCharN < GSMTxSz){
			UDR_GSM = GSM_TxStr[GSM_TxCharN];
			#ifdef GSM_DEBUG
				GPRS_TerminalProcess(GSM_TxStr[GSM_TxCharN]);
			#endif
			if(GSM_DebugMode){
				SendDebug(GSM_TxStr[GSM_TxCharN]);
				SetDebug_DRE_ISR();
			}

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
}
// ~~~~~~~~~
void
GSM_TX(void)
{
	if(Transparent){
		UCSR_GSM_B =  (1<<RXCIE_GSM) | (0<<TXCIE_GSM) | (0<<UDRIE_GSM) | (1<<RXEN_GSM) | (0<<UCSZ_GSM_2) | (1<<TXEN_GSM);
	}
	else{	
		GSM_Flag |=(1<<flg_TxCStr);
		#if defined (__AVR_ATxmega128A1__)
			USART_GSM.CTRLA = USART_RXCINTLVL_HI_gc;
		#else
			UCSR_GSM_B =  (1<<RXCIE_GSM) | (0<<TXCIE_GSM) | (0<<UDRIE_GSM) | (1<<RXEN_GSM) | (0<<UCSZ_GSM_2) | (1<<TXEN_GSM);
		#endif
	}
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
			StartTimer32(TD_GSM_Reset, 24UL*60UL*6000);	// ���������� ������ ������ 24� ���
			GSM_PWRCNTRL_ON();
			InitFIFO();
			GSM_Modem = NOT_RECOGNIZED;
			GSM_State = GSM_WAIT_RDY;
			break;

		case GSM_WAIT_RDY:
			if(GSM_Wait_Response_P(URC_RDY, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_SEND_CGMM:
			GSM_Execute_Command(AT_CGMM,5000*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CGMM_READ:
			if(Timer16Stopp(TD_GSM)) GSM_State = GSM_ReStart1;
			if(GetStringFromFIFO()){
				if(!strcmp_P(GSM_RxStr, GSM_SIMCOM_SIM900R)) GSM_Modem = SIMCOM_SIM900R;
				if(!strcmp_P(GSM_RxStr, GSM_SIMCOM_SIM800)) GSM_Modem = SIMCOM_SIM800;
				//Add here new model GSM-modem
				if(GSM_Modem != NOT_RECOGNIZED) GSM_State = GSM_WAIT_CALL_READY;
			}
/*		case GSM_WAIT_CFUN:
			if(GSM_Wait_Response_P(URC_CFUN, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_WAIT_CPIN_READY:
			if(GSM_Wait_Response_P(URC_CPIN_READY, GSM_ReStart1)) GSM_State++;
			break;*/

		case GSM_WAIT_CALL_READY:
			if(GSM_Wait_Response_P(URC_CALL_READY, GSM_ReStart1)){
				if(GSM_Modem == SIMCOM_SIM800) GSM_State = GSM_WAIT_SMS_READY;
				else GSM_State = GSM_SEND_E0;
			}
			break;
		case GSM_WAIT_SMS_READY:
			if(GSM_Wait_Response_P(URC_SMS_READY, GSM_ReStart1))GSM_State++;
			break;
		//------------------------

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

		//------------------------
		case GSM_SEND_E0:
			GSM_Execute_Command(AT_E0,100*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_E0_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_SEND_CLIP:
			GSM_Execute_Command(AT_CLIP,100*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CLIP_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_SEND_CCID:
			GSM_RxStr[19] = 0;
			GSM_Execute_Command(AT_CCID,300*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CCID_READ:
			//8938001300106446004F
			if(GetStringFromFIFO() && ((GSM_RxStr[19] == 'F')||(GSM_RxStr[19] == 'f')) ){



				GSM_RxStr[18] = ' ';
				sscanf_P(GSM_RxStr+9,PSTR("%lud"), (long unsigned int*)&ICCID2);
				GSM_RxStr[9] = ' ';
				sscanf_P(GSM_RxStr+5,PSTR("%u"), (unsigned int*)&ICCID1);
				if((ICCID1 == 0) || (ICCID2==0)) GSM_State = GSM_ReStart1;
				else{
					ICCID = (uint64_t)ICCID1 * 1000000000UL + (uint64_t)ICCID2;
					GSM_State++;
				}
			}
			break;
		case GSM_WAIT_CCID_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;			
/*		case GSM_SEND_IFC:
			GSM_Execute_Command(AT_IFC,100*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_IFC_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;*/
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
		//------------------------
		case GSM_SEND_CIPMUX:
			if(GSM_MultiCon) GSM_Execute_Command(AT_CIPMUX_1, 100*GSM_DEBUG_DELAY);
			else GSM_Execute_Command(AT_CIPMUX_0, 100*GSM_DEBUG_DELAY);
			GSM_State++; 
			break;
		case GSM_WAIT_CIPMUX_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)){
				if(GSM_MultiCon){
					GSM_State = GSM_WAIT_1;
					// between "Call Ready" and "+CMGS"; 8 sec min needs; for kyivstar vpni only
					StartTimer16(TD_GSM, 1000);
				}
				else GSM_State++;
			}
			break;
		case GSM_SEND_CIPMODE:
			if(GSM_MultiCon) GSM_Execute_Command(AT_CIPMODE_0, 100*GSM_DEBUG_DELAY);
			else GSM_Execute_Command(AT_CIPMODE_1, 100*GSM_DEBUG_DELAY);
			GSM_State++;
			break;
		case GSM_WAIT_CIPMODE_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)){
				GSM_State++;
				// between "Call Ready" and "+CMGS"; 8 sec min needs; for kyivstar vpni only
				StartTimer16(TD_GSM, 1000);
			}
			break;

		//------------------------
		case GSM_WAIT_1:
/*			GetStringFromFIFO();		// ����� �� ���� ������������ FIFO
			if(Timer16Stopp(TD_GSM)){
				GSM_State++;
				GSM_Temp = 0;
			}*/
			GSM_Temp = 0;
			GSM_State++;
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
				sscanf_P(GSM_RxStr,PSTR("%hhu %hhu %hhu %hhu"), (unsigned char*)&GSM_MyIP.IP1, (unsigned char*)&GSM_MyIP.IP2, (unsigned char*)&GSM_MyIP.IP3, (unsigned char*)&GSM_MyIP.IP4);
				if(!IP_compare_Const(&GSM_MyIP,0,0,0,0)){
					if(GSM_MultiCon) GSM_State++;
					else GSM_State = GSM_SEND_CIPSERVER;
				}
				TempNum=0;	//
			}			
			break;
		//------------------------
		case GSM_SEND_CIPSTART:
			if(IP_E_compare_Const(&UDP_ServerIP[TempNum],0,0,0,0)){
				TempNum++;
				if(TempNum>3) GSM_State = GSM_ServerIdle;	//GSM_SEND_CIPSERVER;
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
					if(++TempNum>3) GSM_State = GSM_ServerIdle;	//GSM_State++;
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
				StartTimer16(TD_TCP_Connect, 65535);	// ���������� ������� ������ 655,35 ���
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
			if(!IsWebSession()){
				if( GPRS_FlgSz_Out && (GPRS_FlgSz_Out <= GPRS_Out_MaxSz) ){
					if(GSM_MultiCon == 1){
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
					}
					else{
						GSM_MultiCon = 1;
						GSM_State = GSM_CIPSHUT;
					}
					break;
				}

				if(SMS_FlgSz_Out && erb(&SMS_On)){
					GSM_State = GSM_SEND_CMGS;
					TempNum = 0;
					break;
				}
			}

			if(GetStringFromFIFO()){
				GSM_State = GSM_AnalyzeURC;
				TempNum = 0;
				break;
			}

			if(!IsWebSession()){
				if(Timer16Stopp(TD_TCP_Connect) ){
					GSM_State = GSM_CIPSHUT;
					break;				
					/*if(GSM_ActiveConnection == NO_CONNECTION){
						GSM_State = GSM_SEND_SERVERCLOSE;
						break;
					} 
					else{
						GSM_Temp = GSM_ActiveConnection;
						GSM_State = GSM_TCP_CLIENT_DISCONNECT;
						break;				
					}*/
				}
			
				if(Timer16Stopp(TD_RSSI)){
					GSM_State = GSM_SEND_CSQ;
					break;
				}

				if(Timer32Stopp(TD_GSM_Reset)){
					GSM_State = GSM_ReStart1;
					break;
				}
			}
			break;

		case GSM_AnalyzeURC:			//������ �������� ������

			// ������������� ������
			if( (strstr_P(GSM_RxStr, URC_REMOTE_IP) != NULL) ){
				GSM_State = GSM_TCP_CLIENT_CONNECT;
				break;
			}

			// ������������� ������
			if( (strstr_P(GSM_RxStr, URC_CONNECT) != NULL) ){
				StartTimer16(TD_TCP_Connect, 1000);	// ��������� ������ ��������
				GSM_CSD = 0;
				GSM_State = GSM_ProtocolMode;
				break;
			}

			// ��� ������ 
			if( (strstr_P(GSM_RxStr, URC_RING) != NULL) ){
				StartTimer16(TD_GSM, 150);
				GSM_State = GSM_CheckNumber;	//��������� ����� � ��������� ������
				break;
			}

			// ������������ ������
			if(strstr_P(GSM_RxStr, URC_CLOSED) != NULL){
				// ������ ��������
				sscanf_P(GSM_RxStr,PSTR("%hhu, CLOSED"),(unsigned char*)&TempNum);
				if(GSM_ActiveConnection == TempNum){
					GSM_ActiveConnection = NO_CONNECTION;
					SetIP(&GSM_ClientIP,0,0,0,0);
				}
				GSM_State = GSM_ServerIdle;
				break;
			}

			// +PDP DEACT
			if( (strstr_P(GSM_RxStr, URC_PDPDEACT) != NULL) ){
				GSM_State = GSM_SEND_E0;
				break;
			}

			// �������� ������ �� ������� � ���������� ������ ����������
			if((strstr_P(GSM_RxStr, URC_RECEIVE) != NULL) && (!GPRS_FlgSz_In) ){
					//�������� �� ����������� ����������
					//������ ������
				GSM_State = GSM_RECIEVE;
				break;
			}

			// �������� ����� �� ������ �� 
			if(strstr_P(GSM_RxStr, RESP_CSQ) != NULL ){
				GSM_State = GSM_WAIT_CSQ;
				break;
			}

			// �����
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
						if(Timer16Stopp(TD_GSM)){	// ����� ����� ��������� �� UDP
							GSMTxSz = strlen_P(AT_CIPSEND);
							sprintf_P(GSM_TxStr, AT_CIPSEND);
							sprintf(GSM_TxStr + GSMTxSz, "%u,%u\r",(unsigned int)TempNum, (unsigned int)(GPRS_FlgSz_Out));
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
					sprintf(GSM_TxStr + GSMTxSz, "%u,%u\r",(unsigned int)GSM_ActiveConnection, (unsigned int)(GPRS_FlgSz_Out));
					GSMTxSz = strlen(GSM_TxStr);
					GSM_SendFirstChar();
					StartTimer16(TD_GSM, 500*GSM_DEBUG_DELAY);
					GSM_State++;
				}				
			}
			break;
		case GSM_WAIT_Inv:
			//if(GSM_Wait_Char('>', GSM_ReStart1)) GSM_State++;

			if(GSM_Wait_Char('>', GSM_ServerIdle)) GSM_State++;
			if(GSM_State == GSM_ServerIdle) GPRS_FlgSz_Out = 0;			
			break;
		case GSM_SEND_DATA:
			//	memcpy (void  dest, const void  src, size_t len)
			GSM_AddHeaderSendData(GPRS_Data_Out, GPRS_FlgSz_Out);
			GSM_State++;
			break;
		case GSM_WAIT_CIPSEND_SEND_OK:
//			if(Timer16Stopp(TD_GSM)) GSM_State = GSM_ReStart1;
			if(Timer16Stopp(TD_GSM)){
				GSM_State = GSM_ServerIdle;
				GPRS_FlgSz_Out = 0;
			}

			if(GetStringFromFIFO()){
				if(GPRS_Protocol_Out == UDP){
					char RightAnswer[sizeof(RESP_SEND_OK)];
					strcpy_P(RightAnswer, RESP_SEND_OK);
					RightAnswer[0] = 0x30+TempNum;
					if(GSM_Wait_Response(RightAnswer, GSM_ReStart1)){
						StartTimer16(TD_GSM,500);//min~ 2 sec
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
						
						//GSM_State = GSM_ServerIdle;

					}
				}
			}	
			break;
		case GSM_WAIT_CIPSEND_PAUSE:// ����� ���������� ���� ������������ �������� ������ � SMS
			if(Timer16Stopp(TD_GSM)){
				//GSM_State = GSM_ServerIdle;
				GSM_MultiCon = 0;
				if(sendsWithoutReconnect<3)
				{
					GSM_State = GSM_CIPSHUT;
				}
				else{
					sendsWithoutReconnect=0;
					GSM_State = GSM_CIPSHUT_;
				}
				sendsWithoutReconnect++;

			}
			break;
		//------------------------
		case GSM_CIPSHUT:
			GSM_Execute_Command(AT_CIPSHUT, 1000*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_ShutWait:
			if(GSM_Wait_Response_P(RESP_SHUT_OK, GSM_ReStart1)) GSM_State = GSM_SEND_CIPMUX;
			break;

		//------------------------


				//------------------------ ������������ GPRS
		case GSM_CIPSHUT_:
			GSM_Execute_Command(AT_CIPSHUT, 1000*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_ShutWait_:
			if(GSM_Wait_Response_P(RESP_SHUT_OK, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_SEND_CGATT0:
			GSM_Execute_Command(AT_CGATT_0, 1000*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CGATT0_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)){
				StartTimer16(TD_GSM, 1000);
			 	GSM_State = GSM_SEND_CIPMUX;
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
			// ������ �������� � IP			
			sscanf_P(GSM_RxStr,PSTR("REMOTE IP:%hhu.%hhu.%hhu.%hhu"),
				(unsigned char*)&GSM_ClientIP.IP1, (unsigned char*)&GSM_ClientIP.IP2, (unsigned char*)&GSM_ClientIP.IP3, (unsigned char*)&GSM_ClientIP.IP4);
			
			// ���������� IP � ��� ��������
			TempNum = 0;
			for(uint8_t i=0; i<4; i++){
				if(IP_E_compare_Const(&TCP_ClientIP[i],255,255,255,255)){	// Anybody may coonect
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
				StartTimer16(TD_TCP_Connect, erw(&TCP_CONNECT_timeout));	// ��������� ������ ��������
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
					if( (GSM_ActiveConnection != NO_CONNECTION) && Timer16Stopp(TD_TCP_Connect) ){	//������� ���-���������� �� ��������
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
				sscanf_P(GSM_RxStr,PSTR("+RECEIVE,%hhu,%hhu"),(unsigned char*)&TempConnect, (unsigned char*)&GSM_Temp);
				// ��������� ������ ������ �� GSM_ActiveConnection // ������� ������ �� �����
				if( (TempConnect == GSM_ActiveConnection) /*&& (GSM_Temp <= GPRS_In_MaxSz) && (GSM_Temp >= 7)*/ ){
					StartTimer16(TD_GSM, 500*GSM_DEBUG_DELAY);
					GSM_State++;
					break;
				}
				GSM_State = GSM_ServerIdle;
			}
			break;
		case GSM_RECIEVE_DATA:
			if(Timer16Stopp(TD_GSM)) GSM_State = GSM_ReStart1;			
			/*if(GetDataFromFIFO(GSM_Temp) == GSM_Temp){
				GPRS_FlgSz_In = GSM_Temp;
				memcpy(GPRS_Data_In, GSM_RxStr, GPRS_FlgSz_In);
				StartTimer16(TD_TCP_Connect, erw(&TCP_CONNECT_timeout));	// ������������� ������ ��������

				GSM_State = GSM_ServerIdle;
			}*/
			GPRS_FlgSz_In = GSM_Temp;

			GSM_State = GSM_ServerIdle;
			break;
		//------------------------
		case GSM_SEND_CIPCLOSE:
			GSM_Execute_Command(AT_CIPCLOSE, 6000*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CIPCLOSE_OK:
			if(Timer16Stopp(TD_GSM)) GSM_State = GSM_ReStart1;
			//������ �������� ������
			if(GetStringFromFIFO()){
				if(!strcmp_P(GSM_RxStr, RESP_CLOSE_OK) || !strcmp_P(GSM_RxStr, URC_CLOSED + 2)){
					GSM_State = GSM_ServerIdle;
					GPRS_FlgSz_Out = 0;						
				}
			}
			break;
		//------------------------

		/*NEW COMMIT*/
		//------------------------
		case GSM_ReStart1:
			GSM_PWRCNTRL_OFF();
			StartTimer16(TD_GSM,1000);
			RxBufOverFlow = 0;
//			InitFIFO();
			SMS_FlgSz_Out = 0;	// �.�. ���� ������������ ���� �� ���� ����� ����� �������� ���������� - ����� �����
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


		//------------------------ ���������� �������
		case GSM_SEND_SERVERCLOSE:
			GSM_Execute_Command(AT_SERVERCLOSE, 500*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_SERVERCLOSE_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State++;
			break;
		case GSM_WAIT_SERVER_CLOSE:
			if(GSM_Wait_Response_P(RESP_SERVER_CLOSE, GSM_ReStart1)) GSM_State = GSM_SEND_CIPSERVER;
			break;

		//------------------------ �������� ������ GSM-�������
		case GSM_SEND_CSQ:
			GSM_Execute_Command(AT_CSQ, 500*GSM_DEBUG_DELAY); 
			StartTimer16(TD_RSSI, 6000);
			GSM_State = GSM_ServerIdle;
			break;
		case GSM_WAIT_CSQ:
			GSM_RSSI = 255; GSM_BER = 255;
			sscanf_P(GSM_RxStr,PSTR("+CSQ: %hhu,%hhu"), (unsigned char*)&GSM_RSSI, (unsigned char*)&GSM_BER);
			GSM_State = GSM_ServerIdle;
			break;
		//------------------------

		case GSM_ProtocolMode:	//����������� ������ ������� 4 �����
			
			if(GSM_CSD == 1){	// � CSD ������ ���������� �����
				AppProtocol=0;	
				GSM_State = GSM_DataMode;
				if(UART_Soft){
					GSM_State = GSM_Swtch2CommandMode;
				}
				else{
					EM_InitFIFO();
					Transparent = 1;
					StartTimer16(TD_TCP_Connect, erw(&TCP_CONNECT_timeout));	// ��������� ������ ��������
				}
				break;						
			}

			while(GSM_RX_FIFO_Begin != GSM_RX_FIFO_End){
				
				static uint8_t GSM_RX_FIFO_End_old;	// ����� �� ���������� ������ 4 �����
				if(!GSM_RxCharN) GSM_RX_FIFO_End_old = GSM_RX_FIFO_End;
				
				GetByteFromFIFO((uint8_t *)GSM_RxStr, (uint8_t *)&GSM_RxCharN, GSM_RXSTR_SIZE);
				if(GSM_RxCharN >= 4){
					AppProtocol = 0;
					if(!strncmp_P(GSM_RxStr, PSTR("GET "), 4)) AppProtocol = _HTTP;
					if(!strncmp_P(GSM_RxStr, PSTR("POST"), 4)) AppProtocol = _HTTP;
					if( (GSM_RxStr[0]==5) && (GSM_RxStr[1]==1) ) AppProtocol = _VFCP;
					GSM_State = GSM_DataMode;
					
					if(AppProtocol == 0){
						if(UART_Soft){
							GSM_State = GSM_Swtch2CommandMode;
						}
						else{
							EM_InitFIFO();
							GSM_RX_FIFO_End_Transp = GSM_RX_FIFO_End_old;	// ����� �� ���������� ������ 4 �����
							Transparent = 1;
							StartTimer16(TD_TCP_Connect, erw(&TCP_CONNECT_timeout));	// ��������� ������ ��������
						}
						break;
					}
					break;
				}
			}//while
			// ----- 
			if(Timer16Stopp(TD_TCP_Connect)){	// ���� ��� ���������� �� ������ ��������� � CommandMode
				GSM_State = GSM_Swtch2CommandMode;
				GSM_CloseTransparent();
				break;
			}

			break;

		case GSM_DataMode:
		
	
			// ----- �� ������ � ��������
			if(GSM_RX_FIFO_Begin != GSM_RX_FIFO_End){	// if FIFO not empty
				StartTimer16(TD_TCP_Connect, erw(&TCP_CONNECT_timeout));	// ��������� ������ ��������
			}
			if(AppProtocol != _HTTP){	// � ��������� HTTP ���� ������ �� ���������
				if(GetStringFromFIFO()){
								
					// ������ URC ������
					// ������������ ������
					if( (strstr_P(GSM_RxStr, URC_CLOSED+2) != NULL) ){
						GSM_State = GSM_ServerIdle;
						GSM_CloseTransparent();
						break;
					}
					// ������������ ������ � CSD
					if( (strstr_P(GSM_RxStr, URC_NO_CARRIER) != NULL) ){
						GSM_State = GSM_ServerIdle;
						GSM_CloseTransparent();
						break;
					}
					// +PDP DEACT
					if( (strstr_P(GSM_RxStr, URC_PDPDEACT) != NULL) ){
						GSM_State = GSM_SEND_E0;
						GSM_CloseTransparent();
						break;
					}
				}
			}
			// ----- 
			if(Timer16Stopp(TD_TCP_Connect)){	// ���� ��� ���������� �� ������ ��������� � CommandMode
				GSM_State = GSM_Swtch2CommandMode;
				GSM_CloseTransparent();
				break;
			}
			break;

		case GSM_Swtch2CommandMode:
			StartTimer16(TD_GSM,110);	// min 1000ms before +++
			WebClose();
			GSM_State++;
			break;
		case GSM_Swtch2CommandModePlus:
			if(Timer16Stopp(TD_GSM)){
				sprintf_P(GSM_TxStr, ESC_SEQ);
				GSMTxSz = strlen(GSM_TxStr);
				GSM_SendFirstChar();
				StartTimer16(TD_GSM,1000);	// min 500ms after +++
				GSM_State++;
			}
			break;
		case GSM_WAIT_D2C_SWITCH_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)){
				StartTimer16(TD_TCP_Connect, 65535);	// ���������� ������� ������ 655,35 ���
				if(GSM_CSD==1) GSM_State = GSM_SEND_ATH;
				else GSM_State = GSM_SEND_CIPCLOSE;
			}
			break;
		
		//------------------------ CSD
//RING
//
//+CLIP: "+380976969029",145,"",,"",0
		case GSM_CheckNumber:
			if(Timer16Stopp(TD_GSM)) GSM_State = GSM_SEND_ATH;
			if(!erb(&CLIP_On)){
				GSM_State = GSM_SEND_ATA;
				break;
			}
			if(GetStringFromFIFO() > 25){
				char *qoute_open = strchr(GSM_RxStr,'"');
				char *qoute_clos = strchr(qoute_open+1,'"');
				if( (qoute_open == NULL) || (qoute_clos == NULL)){
					GSM_State = GSM_SEND_ATH;
					break;
				}
				*qoute_clos = '\0';	//for strcmp_E
				for(uint8_t i = 0; i<10; i++){
					if( strcmp_E(qoute_open + 2, CSD_AllowedNumbers[i]) ) GSM_State = GSM_SEND_ATH;
					else { GSM_State = GSM_SEND_ATA; break;}
				}
			}
			break;
		case GSM_SEND_ATA:
			GSM_Execute_Command(AT_ATA, 6000*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_CONNECT_9600:
			if(GSM_Wait_Response_P(RESP_CONNECT_9600, GSM_SEND_ATH)){
				StartTimer16(TD_TCP_Connect, 500);	// ��������� ������ ��������
				GSM_CSD = 1;
				GSM_State = GSM_ProtocolMode;
			}
			break;
		case GSM_SEND_ATH:
			GSM_Execute_Command(AT_ATH, 300*GSM_DEBUG_DELAY); GSM_State++;
			break;
		case GSM_WAIT_ATH_OK:
			if(GSM_Wait_Response_P(RESP_OK, GSM_ReStart1)) GSM_State = GSM_ServerIdle;
			break;

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
		case GSM_CFG_SEND_ATW:
			GSM_Execute_Command(AT_W, 100); GSM_State++;
			break;
		case GSM_CFG_WAIT_ATW_OK:
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
	if((GSM_State != GSM_ReStart1) && (GSM_State != GSM_ReStart2)) GSM_StateBeforeReset = GSM_State;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~
void
GPRS_ServerAddr_Save(void)
{

}
// ~~~~~~~~~~~~~~~~~~~~~~~~

