#include <ctype.h>

#define SIM300DZ

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

const TelephDir *SMS_Addr;

char SMS_Data_Out[100];
uint8_t SMS_FlgSz_Out;

#define GSM_Timeout 3000

// ~~~~~~~~~~~
// AT-commands
// ~~~~~~~~~~~

// Total
prog_char AT_CPOWD[] = "AT+CPOWD=1\r";

// SMS
prog_char AT_CMGF[]	= "AT+CMGF=1\r";
prog_char AT_CSMP[]	= "AT+CSMP=17,167,0,0\r";

prog_char AT_CMGS[]	= "AT+CMGS=\"";

enum {
	GSM_On,
	GSM_CMGF, GSM_CSMP,
	GSM_Idle,
	GSM_CMGS, GSM_SMS_Send,
	GSM_CPOWD
};
volatile uint8_t GSM_State;

prog_str AT_Comm[] = {
	[GSM_CMGF]=AT_CMGF, AT_CSMP,
	[GSM_CMGS]=AT_CMGS,
	[GSM_CPOWD]=AT_CPOWD
};

static uint8_t AT_CharN;

static uint8_t GSM_Wait;

volatile uint8_t GSM_LF;

static uint8_t TD_GSM;

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

	GSM_State = GSM_On;
	GSM_Wait = 1;
	StartTimer16(TD_GSM, GSM_Timeout);
}

// ~~~~~~~~~
static void
AT_Run(void)
{
	GSM_Wait = 0;
	AT_CharN = 0;
	ResetTimer16(TD_GSM);
	UCSR_GSM_B = (UCSR_GSM_B &~(1<<RXCIE_GSM) &~(1<<RXEN_GSM)) |(1<<TXCIE_GSM);
	UDR_GSM = GSM_State==GSM_SMS_Send ? SMS_Data_Out[0] : prc(prp(AT_Comm+GSM_State));
}

// ~~~~~~~~~~
static void
AT_Next(void)
{
	GSM_State++;
	AT_Run();
}

// ~~~~~~~~~~~
static void
GSM2Idle(void)
{
	UCSR_GSM_B &=~(1<<RXCIE_GSM) &~(1<<RXEN_GSM);
	ResetTimer16(TD_GSM);
	GSM_Wait = 0;
	GSM_State = GSM_Idle;
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

// ~~~~~~~~~
void
GSM_RX(void)
{
	char Char = UDR_GSM;
	StartTimer16(TD_GSM, GSM_Timeout);
	GSM_OK = 1;
	switch (GSM_State) {
	case GSM_On:
		if (Char=='\n' && ++GSM_LF==8) {
			UCSR_GSM_B &=~(1<<RXCIE_GSM) &~(1<<RXEN_GSM);
			StartTimer16(TD_GSM, 1000);		// between "Call Ready" and "+CMGS"; 8 sec min needs; for kyivstar vpni only
		}
		break;
	case GSM_CMGF:
		if(Char=='\n' && ++GSM_LF==2)
			AT_Next();
		break;
	case GSM_CSMP:
		if(Char=='\n' && ++GSM_LF==2)
			GSM2Idle();
		break;
	case GSM_CMGS:
		if (Char==' ')
			AT_Next();
		break;
	case GSM_SMS_Send:
		if (Char=='\n' && ++GSM_LF==4)
			SMS_Next();
		break;
	case GSM_CPOWD:
		if (Char=='\n' && ++GSM_LF==2) {
			GSM_State = GSM_On;
			GSM_LF = 0;
		}
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
	uint8_t AT_Ln;
	PGM_P AT_C;
	char Char;

	AT_CharN++;
	if (GSM_State==GSM_SMS_Send) {
		if(AT_CharN<SMS_FlgSz_Out)
			UDR_GSM = SMS_Data_Out[AT_CharN];
		else if(AT_CharN==SMS_FlgSz_Out)
			UDR_GSM = 26;
		else
			GSM2Wait();
	}
	else if (AT_CharN<(AT_Ln=strlen_P(AT_C=prp(AT_Comm+GSM_State))))
		UDR_GSM = prc(AT_C+AT_CharN);
	else if (GSM_State==GSM_CMGS) {
		AT_Ln += SMS_Addr->N[SMS_Addr_Idx].Sz;
		for (char*ptr=(char*)(SMS_Addr->N+SMS_Addr_Idx)->N-sizeof(AT_CMGS)+1; AT_CharN<AT_Ln&&!isdigit(Char=*(ptr+AT_CharN)); AT_CharN++);
		if(AT_CharN<AT_Ln)
			UDR_GSM = Char;
		else if(AT_CharN==AT_Ln)
			UDR_GSM = '\"';
		else if(AT_CharN==AT_Ln+1)
			UDR_GSM = '\r';
		else
			GSM2Wait();
	}
	else
		GSM2Wait();
}

// ~~~~~~~~~~~~
void
GSM_Reset(void)
{
	if (!GSM_OK || GSM_State==GSM_Idle) {
		SMS_FlgSz_Out = 0;
		GSM_State = GSM_CPOWD;
		AT_Run();
	}
}

// ~~~~~~~~~~~~
void
GSM_Cycle(void)
{
	switch (GSM_State) {
	case GSM_On:
		if (Timer16Stopp(TD_GSM))
			AT_Next();
		break;
	case GSM_Idle:
		if (SMS_FlgSz_Out && erb(&SMS_On) && SMS_Addr) {
			SMS_Addr_Idx = 0;
			GSM_State = GSM_CMGS;
			AT_Run();
		}
		break;
	default:
		if (GSM_Wait && Timer16Stopp(TD_GSM)) {
			GSM_OK = 0;
			if (GSM_LF==2) {
				GSM2Idle();
				SMS_FlgSz_Out = 0;
			}
		}
		break;
	}
}
