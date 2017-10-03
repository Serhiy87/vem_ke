// ~~~~~~~~~~~~~
// Modem: SIM300
//
// 06.12.2011 - Kononenko - Add SMS send support
//
// ~~~~~~~~~~~~~

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define SIM300DZ

#ifndef VegaN
	#define VegaN 0
#endif

#ifndef GPRS_APN
	#define GPRS_APN "vpni.kyivstar.net"
#endif

#ifndef MaxTelephN
	#define MaxTelephN 15
#endif
#ifndef MaxTelephDirSz
	#define MaxTelephDirSz 3
#endif
typedef struct {
	struct {
		char N[MaxTelephN];
		uint8_t Sz;
	} N[MaxTelephDirSz];
	uint8_t Sz;
} TelephDir;

uint8_t SMS_On EEMEM = 1;

volatile uint8_t GSM_OK = 1;

#if defined (__AVR_ATmega16__) ||  defined (__AVR_ATmega32__)
	#define GPRS_Out_MaxSz	6
	#define GPRS_In_MaxSz		6
#elif defined (__AVR_ATmega128__) || defined (__AVR_ATmega2561__) || defined (__AVR_ATmega1280__)
	#define GPRS_Out_MaxSz	0x100
	#define GPRS_In_MaxSz	  100
#endif

#define GPRS_Buf_MaxSz GPRS_Out_MaxSz

uint8_t GPRS_Data_Out[GPRS_Out_MaxSz], GPRS_Data_In[GPRS_In_MaxSz];
char SMS_Data_Out[100];
uint8_t GPRS_FlgSz_Out, GPRS_FlgSz_In, SMS_FlgSz_Out;

// Non-printing ASCII
#define ENQ 5		// Enquiry
#define SOH 1		// Start of header

#define GPRS_HeadSz 4

static uint8_t GPRS_Buf[GPRS_Buf_MaxSz+GPRS_HeadSz];
static uint8_t GPRS_Buf_Sz;

#define GSM_Timeout 3000

#ifndef GPRS_ServerAddr_Init
	#define GPRS_ServerAddr_Init {194,176,97,118}
#endif
uint8_t GPRS_ServerAddr[4];
static uint8_t EEMEM GPRS_ServerAddr_EE[4] = GPRS_ServerAddr_Init;

// ~~~~~~~~~~~
// AT-commands
// ~~~~~~~~~~~

// Total
prog_char AT_E0[]	= "ATE0\r";

prog_char AT_CPOWD[]	= "AT+CPOWD=1\r";

// SMS
prog_char AT_CMGF[]	= "AT+CMGF=1\r";
prog_char AT_CSMP[]	= "AT+CSMP=17,167,0,0\r";

prog_char AT_CMGS[]	= "AT+CMGS=\"+";

// GPRS
prog_char AT_CIPHEAD[]		= "AT+CIPHEAD=1\r";

prog_char AT_CGATT_1[]		= "AT+CGATT=1\r";
prog_char AT_CIPCSGP[]		= "AT+CIPCSGP=1,\""GPRS_APN"\"\r";
prog_char AT_CSTT[]				= "AT+CSTT\r";
prog_char AT_CIICR[]			= "AT+CIICR\r";
prog_char AT_CIFSR[]			= "AT+CIFSR\r";

prog_char AT_CLPORT_TCP[]	= "AT+CLPORT=\"TCP\",\"2020\"\r";
prog_char AT_CIPSERVER[]	= "AT+CIPSERVER\r";
prog_char AT_CIPCCON_2[]	= "AT+CIPCCON=2\r";

prog_char AT_CLPORT_UDP[]	= "AT+CLPORT=\"UDP\",\"2021\"\r";
prog_char AT_CIPSTART[]		= "AT+CIPSTART=\"UDP\",\"###.###.###.###\",\"2021\"\r";
prog_char AT_CIPCCON_1[]	= "AT+CIPCCON=1\r";
prog_char AT_CIPSEND[]		= "AT+CIPSEND=";

prog_char AT_CIPCLOSE[]		= "AT+CIPCLOSE\r";
prog_char AT_CIPSHUT[]		= "AT+CIPSHUT\r";

enum {
	GSM_On,

	GSM_E0,

	GSM_CIPHEAD,

	GSM_CMGF, GSM_CSMP,

	GSM_CGATT_1, GSM_CIPCSGP, GSM_CSTT, GSM_CIICR, GSM_CIFSR,
	GSM_CLPORT_TCP, GSM_CIPSERVER, GSM_CIPCCON_2,
	GSM_ServerIdle,
	GSM_ServerRdy,
	GSM_Fail,
	GSM_ServerReceptSz, GSM_ServerRecept,
	GSM_ServerConnIdle,
	GSM_CIPSEND_Server, GSM_Transmit_Server,
	GSM_CLPORT_UDP, GSM_CIPSTART, GSM_CIPCCON_1,
	GSM_CIPSEND_Client, GSM_Transmit_Client,
	GSM_CIPSHUT, GSM_ShutWait,

	GSM_CMGS, GSM_SMS,

	GSM_CPOWD
};
uint8_t GSM_State;

#if defined GPRS_UDP_Server || defined GPRS_UDP_Client
	#define GSM_CIFSR_Next GSM_CLPORT_UDP
	#define GSM_CIPSTART_Next GSM_ServerIdle
	#define GSM_StartClientTransmit GSM_CIPSEND_Client
	#define GSM_EndClientTransmit GSM_ServerIdle
#else
	#define GSM_CIFSR_Next GSM_CLPORT_TCP
	#define GSM_CIPSTART_Next GSM_CIPCCON_1
	#define GSM_StartClientTransmit GSM_CLPORT_UDP
	#define GSM_EndClientTransmit GSM_CIPSHUT
#endif

prog_str AT_Comm[] = {
	[GSM_E0]=AT_E0,
	AT_CIPHEAD,
	AT_CMGF, AT_CSMP,
	AT_CGATT_1, AT_CIPCSGP, AT_CSTT, AT_CIICR, AT_CIFSR,
	AT_CLPORT_TCP, AT_CIPSERVER, AT_CIPCCON_2,
	[GSM_CIPSEND_Server]=AT_CIPSEND,
	[GSM_CLPORT_UDP]=AT_CLPORT_UDP, AT_CIPSTART, AT_CIPCCON_1, AT_CIPSEND,
	[GSM_CIPSHUT]=AT_CIPSHUT,
	[GSM_CMGS]=AT_CMGS,		//Add Kononenko
	[GSM_CPOWD]=AT_CPOWD
};

static uint8_t AT_CharN;

volatile static uint8_t GSM_Wait;

volatile uint8_t GSM_LF;

static uint8_t TD_GSM;

static const TelephDir *SMS_Addr;
static uint8_t SMS_Addr_Idx;

// ~~~~~~~~~~~
void
GSM_Init(void)
{
	UCSR_GSM_A = ~(1<<U2X_GSM) &~(1<<MPCM_GSM);
	UCSR_GSM_B = ((((1<<RXCIE_GSM) &~(1<<TXCIE_GSM) &~(1<<UDRIE_GSM)) |(1<<RXEN_GSM)) |(1<<TXEN_GSM)) &~(1<<UCSZ_GSM_2);
	UCSR_GSM_C = ((~(1<<UMSEL_GSM) &~(1<<UPM_GSM_1) &~(1<<UPM_GSM_0) &~(1<<USBS_GSM)) |(1<<UCSZ_GSM_1) |(1<<UCSZ_GSM_0)) &~(1<<UCPOL_GSM);
	UBRR_GSM_H = 0x00;
	UBRR_GSM_L = 0x67;	// 9600

	TD_GSM = Timer16SysAlloc(1);

	for (uint8_t i=0; i<4; i++)
		GPRS_ServerAddr[i] = erb(GPRS_ServerAddr_EE+i);

	GSM_State = GSM_On;
	GSM_Wait = 1;
	StartTimer16(TD_GSM, GSM_Timeout);
}

// ~~~~~~~~~
static void
AT_Run(void)
{
	UCSR_GSM_B = (UCSR_GSM_B &~(1<<RXCIE_GSM) &~(1<<RXEN_GSM)) |(1<<TXCIE_GSM);
	GSM_Wait = 0;
	AT_CharN = 0;
	ResetTimer16(TD_GSM);
	switch(GSM_State) {
	case GSM_Transmit_Client: case GSM_Transmit_Server:
		UDR_GSM = GPRS_Buf[0];
		break;
	case GSM_SMS:
		UDR_GSM = SMS_Data_Out[0];
		break;
	default:
		UDR_GSM = prc(prp(AT_Comm+GSM_State));
		break;
	}
}

// ~~~~~~~~~~
static void
AT_Next(void)
{
	GSM_State++;
	AT_Run();
}

// ~~~~~~~~~~
static void
GSM_Err(void)
{
	if (GPRS_Buf[0]=='R') {
		GSM_State = GSM_CIPSHUT;
		AT_Run();
	}
	else
		AT_Next();
}

// ~~~~~~~~~~~~~~~~~~~
static void
GSM_IsServerIdle(void)
{
	if (GSM_State==GSM_ServerIdle)
		GSM_Wait = 0;
	else
		AT_Run();
}
// ~~~~~~~~~~~
static void
GSM2Idle(void)
{
	UCSR_GSM_B &=~(1<<RXCIE_GSM) &~(1<<RXEN_GSM);
	ResetTimer16(TD_GSM);
	GSM_Wait = 0;
//	GSM_State = GSM_Idle;
	GSM_State = GSM_ServerIdle;
}

// ~~~~~~~~~~~~~~~
inline static void
SMS_Next(void)
{
	if(++SMS_Addr_Idx<SMS_Addr->Sz) {
		GSM_State = GSM_CMGS;
		AT_Run();
	}
	else {
		GSM2Idle();
		SMS_FlgSz_Out = 0;
	}
}
// ~~~~~~~~~~~~~~~~~~~
inline static void
GSM_CIPSERVER_LF(void)
{
	switch(++GSM_LF) {
	case 2:
		if(GPRS_Buf[0]=='R') {
			GSM_State = GSM_CIPSHUT;
			AT_Run();
		}
		break;
	case 4:
		GSM_Err();
		break;
	}
}

// ~~~~~~~~~
void
GSM_RX(void)
{
	static uint8_t CharN;
	char Char = UDR_GSM;
	StartTimer16(TD_GSM, GSM_Timeout);
	GSM_OK = 1;
	switch(GSM_State) {
	case GSM_On:
		if (Char=='\n' && ++GSM_LF==8) {
			UCSR_GSM_B &=~(1<<RXCIE_GSM) &~(1<<RXEN_GSM);
			StartTimer16(TD_GSM, 1000);		// between "Call Ready" and "+CMGS"; 8 sec min needs; for kyivstar vpni only
		}
		break;
	case GSM_CIICR:
		if(Char=='\n') {
			if(++GSM_LF==2)
				GSM_Err();
		}
		else if(Char!='\r')
			GPRS_Buf[0] = Char;
		break;
	case GSM_CIFSR:
		if(Char=='\n' && ++GSM_LF==2) {
			GSM_State = GSM_CIFSR_Next;
			AT_Run();
		}
		break;
	case GSM_CIPSERVER:
		if (Char=='\n')
			GSM_CIPSERVER_LF();
		else if(Char!='\r')
			GPRS_Buf[0] = Char;
		break;
	case GSM_CIPCCON_2:
		if (Char=='\n' && ++GSM_LF==2) {
			GSM_Wait = 0;
			GSM_State = GSM_ServerIdle;
		}
		break;
	case GSM_ServerIdle:
		if (Char=='+')
			GSM_State++;
		break;
	case GSM_ServerRdy:
		if (Char=='I') {		// +IPD..:
			CharN = GPRS_Buf_Sz = 0;
			GSM_State = GSM_ServerReceptSz;
		}
		else
			GSM_State = GSM_Fail;
		break;
	case GSM_Fail:
		if(Char=='\n') {
			GSM_State = GSM_CIPSHUT;
			AT_Run();
		}
		break;
	case GSM_ServerReceptSz:
		if(isdigit(Char))
			GPRS_Buf_Sz = GPRS_Buf_Sz*10-'0'+Char;
		else if(Char==':')
			GSM_State++;
		break;
	case GSM_ServerRecept:
		GPRS_Buf[CharN] = Char;
		if(++CharN==GPRS_Buf_Sz) {
			if(GPRS_Buf[0]==ENQ && GPRS_Buf[1]==SOH && !GPRS_FlgSz_In)
				memcpy(GPRS_Data_In, GPRS_Buf+GPRS_HeadSz, GPRS_FlgSz_In = GPRS_Buf_Sz-GPRS_HeadSz);
			GSM_State++;
		}
		break;
	case GSM_ServerConnIdle:
		switch(Char) {
		case '+':
			GSM_State = GSM_ServerRdy;
			break;
		case 'C':
			GSM_State = GSM_ServerIdle;
			break;
		}
		break;
	case GSM_CIPSTART:
		if(Char=='\n' && ++GSM_LF==4) {
			GSM_State = GSM_CIPSTART_Next;
			GSM_IsServerIdle();
		}
		break;
	case GSM_CIPSEND_Client: case GSM_CIPSEND_Server:
		switch(Char) {
		case '\n':
			if (++GSM_LF==2) {
				GSM_State = GSM_CIPSHUT;
				AT_Run();
			}
			break;
		case ' ':
			AT_Next();
			break;
		}
		break;
	case GSM_Transmit_Server:
		if(Char=='\n')
			GSM_State = GSM_ServerConnIdle;
		break;
	case GSM_Transmit_Client:
		if(Char=='\n' && ++GSM_LF==2) {
			GSM_State = GSM_EndClientTransmit;
			GSM_IsServerIdle();
		}
		break;
	case GSM_CIPSHUT:
		if(Char=='\n' && ++GSM_LF==2) {
			GSM_Wait = 0;
			GSM_State++;
			StartTimer16(TD_GSM, 300);
		}
		break;
//Add Kononenko
	case GSM_CMGS:
		if (Char==' ')
			AT_Next();
		break;
	case GSM_SMS:
		if (Char=='\n' && ++GSM_LF==4)
			SMS_Next();
		break;
//-------
	case GSM_CPOWD:
		if(Char=='\n' && ++GSM_LF==8) {
			GSM_State = GSM_On;
			GSM_LF = 0;
		}
		break;
	default:
		if(Char=='\n' && ++GSM_LF==2)
			AT_Next();
		break;
	}
}

// ~~~~~~~~~~~
static void
GSM2Wait(void)
{
	GSM_LF = 0;
	GSM_Wait = 1;
	StartTimer16(TD_GSM, GSM_Timeout);
	UCSR_GSM_B = (UCSR_GSM_B |(1<<RXCIE_GSM) |(1<<RXEN_GSM)) &~(1<<TXCIE_GSM);
}

// ~~~~~~~~~
void
GSM_TX(void)
{
	PGM_P AT_C;
	uint8_t AT_Ln;
	char Str[4];
	uint8_t Str_Ln;

	AT_CharN++;
	switch (GSM_State) {
	case GSM_SMS:
		if(AT_CharN<SMS_FlgSz_Out)
			UDR_GSM = SMS_Data_Out[AT_CharN];
		else if (AT_CharN==SMS_FlgSz_Out)
			UDR_GSM = 26;
		else
			GSM2Wait();
		break;
	case GSM_Transmit_Client: case GSM_Transmit_Server:
		if(AT_CharN<GPRS_Buf_Sz)
			UDR_GSM = GPRS_Buf[AT_CharN];
		else
			GSM2Wait();
		break;
	case GSM_CIPSTART:
		switch (AT_CharN) {
		case 19:		case 19+4:		case 19+4*2:		case 19+4*3:
			UDR_GSM = '0'+GPRS_ServerAddr[(AT_CharN-19)/4]/100;
			break;
		case 19+1:	case 19+4+1:	case 19+4*2+1:	case 19+4*3+1:
			UDR_GSM = '0'+GPRS_ServerAddr[(AT_CharN-19)/4]%100/10;
			break;
		case 19+2:	case 19+4+2:	case 19+4*2+2:	case 19+4*3+2:
			UDR_GSM = '0'+GPRS_ServerAddr[(AT_CharN-19)/4]%10;
			break;
		case 43:
			GSM2Wait();
			break;
		default:
			UDR_GSM = prc((PGM_P)prp(AT_Comm+GSM_State)+AT_CharN);
			break;
		}
		break;
	default:
		if (AT_CharN<(AT_Ln=strlen_P(AT_C=prp(AT_Comm+GSM_State)))) {
			UDR_GSM = prc(AT_C+AT_CharN);
			break;
		}
		switch (GSM_State) {
		case GSM_CIPSEND_Client: case GSM_CIPSEND_Server:
			Str_Ln = sprintf(Str, "%u", GPRS_Buf_Sz);
			AT_Ln = AT_CharN-AT_Ln;
			if(AT_Ln<Str_Ln)
				UDR_GSM = Str[AT_Ln];
			else if(AT_Ln==Str_Ln)
				UDR_GSM = '\r';
			else
				GSM2Wait();
			break;
		case GSM_CMGS:
			AT_Ln += SMS_Addr->N[SMS_Addr_Idx].Sz;
			for (char*ptr=(char*)(SMS_Addr->N+SMS_Addr_Idx)->N-sizeof(AT_CMGS)+1; AT_CharN<AT_Ln&&!isdigit(Str[0]=*(ptr+AT_CharN)); AT_CharN++);
			if(AT_CharN<AT_Ln)
				UDR_GSM = Str[0];
			else if(AT_CharN==AT_Ln)
				UDR_GSM = '\"';
			else if(AT_CharN==AT_Ln+1)
				UDR_GSM = '\r';
			else
				GSM2Wait();
			break;
		default:
			GSM2Wait();
			break;
		}
		break;
	}
}

// ~~~~~~~~~~~~
void
GSM_Reset(void)
{
	IntOff();
	if (!GSM_OK || GSM_State==GSM_ServerIdle) {
		GSM_State = GSM_CPOWD;
		AT_Run();
	}
	IntOn();
}

// ~~~~~~~~~~~
static void
GPRS_Out_Run()
{
	GPRS_Buf[0] = ENQ;
	GPRS_Buf[1] = SOH;
	GPRS_Buf[2] = VegaN>>8;
	GPRS_Buf[3] = (uint8_t)VegaN;
	memcpy(GPRS_Buf+GPRS_HeadSz, GPRS_Data_Out, GPRS_FlgSz_Out);
	GPRS_Buf_Sz = GPRS_FlgSz_Out+GPRS_HeadSz;
	GPRS_FlgSz_Out = 0;
	AT_Run();
}

// ~~~~~~~~~~~~
void
GSM_Cycle(void)
{
	switch(GSM_State) {
	case GSM_On:
		if(Timer16Stopp(TD_GSM))
			AT_Next();
		break;
	#ifndef GPRS_UDP_Server
		case GSM_ServerConnIdle:
			if(GPRS_FlgSz_Out) {
				GSM_State = GSM_CIPSEND_Server;
				GPRS_Out_Run();
			}
			break;
		case GSM_ServerIdle:
			if(GPRS_FlgSz_Out) {
				GSM_State = GSM_StartClientTransmit;
				GPRS_Out_Run();
				break;
			}
			if (SMS_FlgSz_Out && erb(&SMS_On) && SMS_Addr) {
				SMS_Addr_Idx = 0;
				GSM_State = GSM_CMGS;
				AT_Run();
			}
			break;
	#endif
	case GSM_ShutWait:
		if(Timer16Stopp(TD_GSM)) {
			GSM_State = GSM_CGATT_1;
			AT_Run();
		}
		break;
	default:
		if(GSM_Wait && Timer16Stopp(TD_GSM))
			GSM_OK = 0;
		break;
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~
void
GPRS_ServerAddr_Save(void)
{
	for (uint8_t i=0; i<4; i++)
		ewb(GPRS_ServerAddr_EE+i, GPRS_ServerAddr[i]);
}
