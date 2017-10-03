// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Ver 1: Vega-Classic-2
// Ver 2: All PLC models
// Ver 3: Modbus-master (nets of PLC)
// Ver 4: GPRS-control
// Ver 5: Modbus extention blocks. All ATmega models
// Ver 6: Multimodbus
// Ver 7: ATxmega. All Atmel AVR models

// 02.04.2012 * Kononenko *  Revised for vmd2_1 - LCDBackLight
// 18.12.2012 * Kononenko *  Add menu for SIM900R
// 07.10.2013 * Kononenko *  Revised for vmd2_2
// 21.11.2013 * Kononenko * Add TCP_CONNECT_timeout, APN, DebugStr in menu for SIM900R
// 19.12.2013 * Kononenko *  Revised for vmd2_3
// 16.01.2014 * Kononenko *  Add EEMEM Memory retore for vmd2_3
// 28.05.2015 * Kononenko *  Add MsgErr41 "Timer allocation err"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define SysVer "8.0"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_char Fail_Str[] = "Сбой ";
prog_char Norm_Str[] = "Норма";

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define MsgErrPage(N, Txt) MsgPage MsgErr##N = {{	\
	{" __________________ "},												\
	{" Internal error "#N": "},											\
	{Txt},																					\
	{"                    "}												\
}};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_char DI_NO_Str[] = "нет ";
prog_char DI_OK_Str[] = "норм";
prog_str DI_Sta_Tbl[] = {DI_NO_Str, DI_OK_Str};

prog_char DI_Op_Str[] = "разм";
prog_char DI_Cl_Str[] = "замк";
prog_str DI_Tbl[] = {DI_Op_Str, DI_Cl_Str};

prog_char DI_Inv_Cl_Str[] = "н.з.";
prog_char DI_Inv_Op_Str[] = "н.р.";
prog_str DI_Inv_Tbl[] = {DI_Inv_Cl_Str, DI_Inv_Op_Str};

#define F_DI(N) InField F_DI##N[] = {{Bit, DigInput.State, DI_Tbl, N-1}, {Bit, DigInput.Status, DI_Sta_Tbl, N-1}};

#define F_DII(N) OutField F_DII##N[] = {{EE_Bit, DI_Invers, .EnumList=DI_Inv_Tbl, N-1}};

#define F_HSC(N) InField F_HSC##N[] = {{Word, &HSCounter[N], NULL, 0}};

#define L_1DI(N) {" N"#N":  {{{{ }}}} {{{{", F_DI##N,	F_DII##N}
#define L_2DI(N) {" N"#N": {{{{ }}}} {{{{", F_DI##N,	F_DII##N}
#define L_HSC(N) {" Счётчик "#N":    {{{{{", F_HSC##N}

extern MenuPage MenuDI;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_char DO_Op_Str[] = "выкл";
prog_char DO_Cl_Str[] = "вкл ";
prog_str DO_Tbl[] = {DO_Op_Str, DO_Cl_Str};

#define F_DO(N) OutField F_DO##N[] = {{Bit, DigOutput.Buf, .EnumList=DO_Tbl, N-1}};

#define L_1DO(N) {" Выход N"#N":     }}}} ", NULL, F_DO##N}
#define L_2DO(N) {" Выход N"#N":    }}}} ",  NULL, F_DO##N}

extern MenuPage MenuDO;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define F_AI(N1, N2) InField F_AI##N1##N2[] = {{z_Word, AnalogInput+N1-1}, {z_Word, AnalogInput+N2-1}};

#define L_AI(N1, N2) {" N"#N1": {{{{{ N"#N2": {{{{{", F_AI##N1##N2}

extern MenuPage MenuAI;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define F_AO(N) OutField F_AO##N[] = {{zc_Word, DAC_Output+N-1, 0, MaxAO}};

#define L_AO(N) {" Выход N"#N":     }}}}}", NULL, F_AO##N}

extern MenuPage MenuAO;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef ADC_Miss
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define F_ADC(N1, N2) InField F_ADC##N1##N2[] = {{Word, ADC_Input+N1-1}, {Word, ADC_Input+N2-1}};

#define L_ADC(N1, N2) {" N"#N1": {{{{{ N"#N2": {{{{{", F_ADC##N1##N2}

extern MenuPage Menu_ADC_Ch;

extern MenuPage Menu_ADC_Cali;

InField F_ADC_3[] = {{Word, &ADC_ReadFail}};
InField F_ADC_4[] = {{Word, &ADC_Fail}};
InField F_ADC_5[] = {{Word, &ADC_Break}};
MenuLine List_ADC_Err[] = {
	{" --- ADC errors --- "},
	{" Read fail:    {{{{{", F_ADC_3},
	{" Calibr fail:  {{{{{", F_ADC_4},
	{" Calibr break: {{{{{", F_ADC_5}
};
MenuPage Menu_ADC_Err = {SetMenu(List_ADC_Err), 3};

extern prog_str ADC_Tbl[];

InField F_ADC_1[] = {{Enum, &ADC_Phase, ADC_Tbl}};
InField F_ADC_2[] = {{Byte, &ADC_Time}, {Byte, &ADC_Chan}};

MenuLine List_ADC[] = {
	{" Режим: {{{{{{{{{{{{", F_ADC_1},
	{" Время:{{0мс Канал:{", F_ADC_2},
	{" - Каналы -         ", .InnPage=&Menu_ADC_Ch},
	{" - Калибровка -     ", .InnPage=&Menu_ADC_Cali},
	{" - Ошибки -         ", .InnPage=&Menu_ADC_Err}
};
MenuPage MenuADC = {SetMenu(List_ADC), 2};

MsgErrPage(21, "      ADC off       ")

// ~~~~~~~~~~~~~~~~~~~~~
#endif	// ndef ADC_Miss
// ~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_char Therm_Off_Str[] =	"   x   ";
prog_char ADC_Off_Str[] =		"-------";
prog_str ThermTbl[] = {Therm_Off_Str, ADC_Off_Str};

prog_char ThermOff_Str[]	= "Откл.";

prog_char TSM100_Str[]		= "ТСМ100";
prog_char TSM50_Str[]			= "ТСМ50";
prog_char TSP100_Str[]		= "ТСП100";
prog_char TSP50_Str[]			= "ТСП50";
prog_char Pt100_Str[]			= "Pt100";
prog_char Gr21_Str[]			= "Гр21";
prog_char Gr23_Str[]			= "Гр23";
prog_str RTD_Name_List[] = {ThermOff_Str, TSM100_Str, TSM50_Str, TSP100_Str, TSP50_Str, Pt100_Str, Gr21_Str, Gr23_Str};

prog_char TC_K_Str[]			= "TXA";
prog_char TC_B_Str[]			= "TПР";
prog_char TC_J_Str[]			= "TЖК";
prog_char TC_L_Str[]			= "TXК";
prog_str TC_Name_List[] = {ThermOff_Str, TC_K_Str, TC_B_Str, TC_J_Str, TC_L_Str};

// ~~~~~~~~~~~~~
#ifndef ADC_Miss
// ~~~~~~~~~~~~~

#define F_Therm(N) InField F_Therm##N[] = {{s_Word, Temperature+N-1, ThermTbl, 1}};

#define F_Thermis(N) OutField F_Thermis##N[] = {{EE_Enum, ThermType+N-1, 0, MaxThermis, NULL, RTD_Name_List}};

#define F_ThermisD(N) OutField F_ThermisD##N[] = {{s_EE_Byte, &ThermCorr[N-1].D, -128, 127, .Prec=1}};
#define F_ThermisA(N) OutField F_ThermisA##N[] = {{EE_Word, &ThermCorr[N-1].A, ThermCorrA_Min, ThermCorrA_Max, .Prec=3}};

#define M_Therm(N) MenuLine ListTherm##N[] = {			\
	{" Канал N"#N": {{{{{{{|C", F_Therm##N},					\
	{" Термодатчик: }}}}}}", NULL, F_Thermis##N},			\
	{" Сдвиг:      }}}}}|C", NULL, F_ThermisD##N},		\
	{" Коэффициент:  }}}}}", NULL, F_ThermisA##N}			\
};																									\
MenuPage MenuTherm##N = {SetMenu(ListTherm##N), 1};

#define L_Thermis(N) {" Канал N"#N": {{{{{{{|C", F_Therm##N, NULL, &MenuTherm##N}

extern MenuPage MenuTherm;

// ~~~~~~~~~~~~~~~~~~~~~
#endif	// ndef ADC_Miss
// ~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_char NoPariStr[]		= "нет";
prog_char EvenPariStr[]	= "четный";
prog_char OddPariStr[]	= "нечетный";
prog_str MBPariTbl[] = {NoPariStr, EvenPariStr, OddPariStr};

#define BR_Str(BR) prog_char BR_Str##BR[] = #BR;
BR_Str(1200)	BR_Str(2400)	BR_Str(4800)
BR_Str(9600)	BR_Str(14400)	BR_Str(19200)
BR_Str(28800)	BR_Str(38400)	BR_Str(57600)
prog_str BaudTbl[] = {
	BR_Str1200,		BR_Str2400,		BR_Str4800,
	BR_Str9600,		BR_Str14400,	BR_Str19200,
	BR_Str28800,	BR_Str38400, 	BR_Str57600
};

#define MB_CPT(N, N1, N2) InField MB##N##_CPT_##N1##N2[] = {{z_Word, MB_CPT[N]+N1}, {z_Word, MB_CPT[N]+N2}};

#define M_Modbus(N)																																													\
void Key_MB##N##_Parity(void)		{SetParity(N);}																															\
void Key_MB##N##_BitRate(void)	{SetBAUD(N);}																																\
																																																						\
OutField F_MB##N##_Pari[] = {{EE_Enum, &MB_Param[N].Parity, 0, OddParity, Key_MB##N##_Parity, MBPariTbl}};	\
OutField F_MB##N##_Stopbits[] = {{EE_Byte, &MB_Param[N].StopBits, 1, 2, Key_MB##N##_Parity}};								\
OutField F_MB##N##_Baud[] = {{EE_Enum, &MB_Param[N].BitRate, 0, MBBR57600, Key_MB##N##_BitRate, BaudTbl}};	\
																																																						\
MB_CPT(N, 0, 4)																																															\
MB_CPT(N, 1, 5)																																															\
MB_CPT(N, 2, 6)																																															\
MB_CPT(N, 3, 7)																																															\
																																																						\
uint8_t Key_MB##N##_CPT_Clear(void) {MB_CPT_Clear(N); return 1;}																						\
																																																						\
MenuLine List_MB##N##_CPT[] = {																																							\
	{"CP1:{{{{{  CP5:{{{{{", MB##N##_CPT_04},																																	\
	{"CP2:{{{{{  CP6:{{{{{", MB##N##_CPT_15},																																	\
	{"CP3:{{{{{  CP7:{{{{{", MB##N##_CPT_26},																																	\
	{"CP4:{{{{{  CP8:{{{{{", MB##N##_CPT_37}																																	\
};																																																					\
MenuPage M_MB##N##_CPT = {SetMenu(List_MB##N##_CPT), 3, .Enter=Key_MB##N##_CPT_Clear};											\
																																																						\
MenuLine L_MB##N[] = {																																											\
	{" ---- Modbus "#N" ---- "},																																							\
	{" Счетчики (сброс:->)", .InnPage=&M_MB##N##_CPT},																												\
	{" Паритет: }}}}}}}}  ", NULL, F_MB##N##_Pari},																														\
	{" Стоп-биты: }       ", NULL, F_MB##N##_Stopbits},																												\
	{" BAUD: }}}}}bps     ", NULL, F_MB##N##_Baud}																														\
};																																																					\
MenuPage MenuModbus##N = {SetMenu(L_MB##N), 1};

M_Modbus(0)
#if Modbus_Qt>1
	M_Modbus(1)
#endif

MenuLine List_Modbus[] = {
	{" Modbus 0           ", .InnPage=&MenuModbus0},
#if Modbus_Qt>1
	{" Modbus 1           ", .InnPage=&MenuModbus1},
#else
	{"    X               "},
#endif
	{"    X               "},
	{"    X               "},
};
MenuPage MenuModbus = {SetMenu(List_Modbus)};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_char N_Str[] = "Нет";
prog_char Y_Str[] = "Да";
prog_str YN_Tbl[] = {N_Str, Y_Str};
#ifdef M41T56
	OutField F_SummerTime[] = {{EE_Enum, &SummerTime, 0, 1, NULL, YN_Tbl}};
#endif

InField F_WDay[] = {{Byte, &Tm_WDay}};
OutField F_DateTime[] = {
#ifdef M41T56
	{z_Byte,		&Tm_MDay,	1, 31, SetMDay},
	{z_Byte,		&Tm_Mon,	1, 12, SetMon},
	{z_Byte,		&Tm_Year,	0, 99, SetYear},
	{z_Byte,		&Tm_Hour,	0, 23, SetHour},
	{z_Byte,		&Tm_Min,	0, 59, SetMin},
	{z_Byte,		&Tm_Sec,	0, 59, SetSec}
#else
	{z_Byte,		&Tm_MDay,	1, 31, SetYearDay},
	{z_Byte,		&Tm_Mon,	1, 12, SetMon},
	{z_EE_Byte,	&Tm_Year,	0, 99, SetYearDay},
	{z_Byte,		&Tm_Hour,	0, 23, SetHour},
	{z_Byte,		&Tm_Min,	0, 59, SetMin},
	{z_Byte,		&Tm_Sec,	0, 59, SetSec}
#endif
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef GSM
// ~~~~~~~~~~~~~~
#ifdef SIM300DZ

	prog_str GSM_OK_Tbl[] = {Fail_Str, Norm_Str};
	InField F_GSM_OK[] = {{Enum, &GSM_OK, GSM_OK_Tbl}};

	InField F_GSM_State[] = {{Byte, &GSM_State}};

	InField F_GSM_LF[] = {{Byte, &GSM_LF}};

	OutField F_GPRS_ServerAddr[] = {
		{z_Byte, GPRS_ServerAddr+0, 0, 0xFF, GPRS_ServerAddr_Save},
		{z_Byte, GPRS_ServerAddr+1, 0, 0xFF, GPRS_ServerAddr_Save},
		{z_Byte, GPRS_ServerAddr+2, 0, 0xFF, GPRS_ServerAddr_Save},
		{z_Byte, GPRS_ServerAddr+3, 0, 0xFF, GPRS_ServerAddr_Save}
	};

	uint8_t Key_GSM(void) {if(CurrLine==5) GSM_Reset(); return StandartKey;}

	MenuLine ListGSM[] = {
		{" ----- Модем ------ "},
		{" IP: }}}.}}}.}}}.}}}", NULL, F_GPRS_ServerAddr},
		{" Статус: {{{{{      ", F_GSM_OK},
		{" Состояние: {{{     ", F_GSM_State},
		{" <LF>: {{           ", F_GSM_LF},
		{" Reset->            "}
	};
	MenuPage MenuGSM = {SetMenu(ListGSM), 0, .Enter=Key_GSM};

#endif	//SIM300DZ
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef SIM900R
	#ifdef GSM_DEBUG
		MenuPage MenuGSMConsole;
	#endif

	void ResetGSMState(void){
		GSM_State = GSM_ReStart1;
	}

	prog_char GSM_CNTL_AUTO_Str[] 	= "auto  ";
	prog_char GSM_CNTL_MANUAL_Str[] = "manual";
	prog_char GSM_CNTL_CONFIG_Str[] = "config";
	prog_str  GSM_ControlList[] = {GSM_CNTL_AUTO_Str, GSM_CNTL_MANUAL_Str, GSM_CNTL_CONFIG_Str};


	InField MenuGSM_State[]			= { {Byte, &GSM_State, NULL, 0} };
	OutField MenuGSM_GSM_Cont[] 	= { {Enum, (uint8_t *)&GSM_Control, 0, 2, ResetGSMState, GSM_ControlList, 0} };
	InField MenuGSM_GSM_MyIP[] 		= { {Byte, &GSM_MyIP.IP1, NULL, 0}, {Byte, &GSM_MyIP.IP2, NULL, 0},
										{Byte, &GSM_MyIP.IP3, NULL, 0}, {Byte, &GSM_MyIP.IP4, NULL, 0} };
	InField MenuGSM_ClientIP[] 		= { {Byte, &GSM_ClientIP.IP1, NULL, 0}, {Byte, &GSM_ClientIP.IP2, NULL, 0},
										{Byte, &GSM_ClientIP.IP3, NULL, 0}, {Byte, &GSM_ClientIP.IP4, NULL, 0} };

	InField MenuGSM_ActiveCon[] 	= { {Byte, &GSM_ActiveConnection, NULL, 0} };
	InField MenuGSM_FIFOMax[] 		= { {Byte, &GSM_RX_FIFOMax, NULL, 0} };

	OutField MenuGSM_APN1[] 		= { {EE_Text, &GPRS_apn1, DIGIT, UKR_CAPS, NULL, NULL, 0} };
	OutField MenuGSM_APN2[] 		= { {EE_Text, &GPRS_apn2, DIGIT, UKR_CAPS, NULL, NULL, 0} };
	OutField MenuGSM_APN_USER_NAME[]= { {EE_Text, &GPRS_apn_user_name, DIGIT, UKR_CAPS, NULL, NULL, 0} };
	OutField MenuGSM_APN_PASSWORD[]	= { {EE_Text, &GPRS_apn_psw, DIGIT, UKR_CAPS, NULL, NULL, 0} };

	OutField MenuGSM_IPUDP1[] = { {EE_Byte, &UDP_ServerIP[0].IP1, 0, 255, NULL, NULL, 0}, {EE_Byte, &UDP_ServerIP[0].IP2, 0, 255, NULL, NULL, 0},{EE_Byte, &UDP_ServerIP[0].IP3, 0, 255, NULL, NULL, 0},{EE_Byte, &UDP_ServerIP[0].IP4, 0, 255, NULL, NULL, 0}};
	OutField MenuGSM_IPUDP2[] = { {EE_Byte, &UDP_ServerIP[1].IP1, 0, 255, NULL, NULL, 0}, {EE_Byte, &UDP_ServerIP[1].IP2, 0, 255, NULL, NULL, 0},{EE_Byte, &UDP_ServerIP[1].IP3, 0, 255, NULL, NULL, 0},{EE_Byte, &UDP_ServerIP[1].IP4, 0, 255, NULL, NULL, 0}};
	OutField MenuGSM_IPUDP3[] = { {EE_Byte, &UDP_ServerIP[2].IP1, 0, 255, NULL, NULL, 0}, {EE_Byte, &UDP_ServerIP[2].IP2, 0, 255, NULL, NULL, 0},{EE_Byte, &UDP_ServerIP[2].IP3, 0, 255, NULL, NULL, 0},{EE_Byte, &UDP_ServerIP[2].IP4, 0, 255, NULL, NULL, 0}};
	OutField MenuGSM_IPUDP4[] = { {EE_Byte, &UDP_ServerIP[3].IP1, 0, 255, NULL, NULL, 0}, {EE_Byte, &UDP_ServerIP[3].IP2, 0, 255, NULL, NULL, 0},{EE_Byte, &UDP_ServerIP[3].IP3, 0, 255, NULL, NULL, 0},{EE_Byte, &UDP_ServerIP[3].IP4, 0, 255, NULL, NULL, 0}};

	OutField MenuGSM_PortUDP1[] = { {EE_Word, &UDP_ServerPort[0], 0, 65535, NULL, NULL, 0} };
	OutField MenuGSM_PortUDP2[] = { {EE_Word, &UDP_ServerPort[1], 0, 65535, NULL, NULL, 0} };
	OutField MenuGSM_PortUDP3[] = { {EE_Word, &UDP_ServerPort[2], 0, 65535, NULL, NULL, 0} };
	OutField MenuGSM_PortUDP4[] = { {EE_Word, &UDP_ServerPort[3], 0, 65535, NULL, NULL, 0} };

	OutField MenuGSM_IPTCP1[] = { {EE_Byte, &TCP_ClientIP[0].IP1, 0, 255, NULL, NULL, 0}, {EE_Byte, &TCP_ClientIP[0].IP2, 0, 255, NULL, NULL, 0},{EE_Byte, &TCP_ClientIP[0].IP3, 0, 255, NULL, NULL, 0},{EE_Byte, &TCP_ClientIP[0].IP4, 0, 255, NULL, NULL, 0}};
	OutField MenuGSM_IPTCP2[] = { {EE_Byte, &TCP_ClientIP[1].IP1, 0, 255, NULL, NULL, 0}, {EE_Byte, &TCP_ClientIP[1].IP2, 0, 255, NULL, NULL, 0},{EE_Byte, &TCP_ClientIP[1].IP3, 0, 255, NULL, NULL, 0},{EE_Byte, &TCP_ClientIP[1].IP4, 0, 255, NULL, NULL, 0}};
	OutField MenuGSM_IPTCP3[] = { {EE_Byte, &TCP_ClientIP[2].IP1, 0, 255, NULL, NULL, 0}, {EE_Byte, &TCP_ClientIP[2].IP2, 0, 255, NULL, NULL, 0},{EE_Byte, &TCP_ClientIP[2].IP3, 0, 255, NULL, NULL, 0},{EE_Byte, &TCP_ClientIP[2].IP4, 0, 255, NULL, NULL, 0}};
	OutField MenuGSM_IPTCP4[] = { {EE_Byte, &TCP_ClientIP[3].IP1, 0, 255, NULL, NULL, 0}, {EE_Byte, &TCP_ClientIP[3].IP2, 0, 255, NULL, NULL, 0},{EE_Byte, &TCP_ClientIP[3].IP3, 0, 255, NULL, NULL, 0},{EE_Byte, &TCP_ClientIP[3].IP4, 0, 255, NULL, NULL, 0}};

	OutField MenuGSM_TCPPort[]		= { {EE_Word, &TCP_ListenPort, 0, 65535, NULL, NULL, 0} };
	OutField MenuGSM_TCPTimeout[]	= { {EE_Word, &TCP_CONNECT_timeout, 100, 65535, NULL, NULL, 2} };
	OutField MenuGSM_SMS[] 			= { {EE_Byte, &SMS_On, 0, 1, NULL, NULL, 0} };
#ifdef VEGA_SN
	OutField MenuGSM_VegaSN[]	= { {EE_Word, &Vega_SN, 0, 65535, NULL, NULL, 0} };
#endif
#ifdef GSM_PASSWORD
	OutField MenuGSM_Password[]	= { {EE_Word, &GSM_Password, 0, 65535, NULL, NULL, 0} };
#endif

	MenuLine ListGSM[] = {
		{" Модем SIM900R.  {{{", MenuGSM_State, 		NULL,				NULL},
		{" Control mode:}}}}}}", NULL, 					MenuGSM_GSM_Cont,	NULL},
		{" MyIP{{{.{{{.{{{.{{{", MenuGSM_GSM_MyIP, 		NULL, 				NULL},
		{" ClIP{{{.{{{.{{{.{{{", MenuGSM_ClientIP, 		NULL, 				NULL},
		{" Active conection{{{", MenuGSM_ActiveCon,		NULL, 				NULL},
		{" FIFO:max {{/70byte ", MenuGSM_FIFOMax, 		NULL, 				NULL},
		{" -------APN---------", NULL,	NULL, NULL},
		{" }}}}}}}}}}}}}}}}}}}", NULL,	MenuGSM_APN1, NULL},
		{" }}}}}}}}}}}}}}}}}}}", NULL,	MenuGSM_APN2, NULL},
		{" ---APN-USER NAME---", NULL,	NULL, 	NULL},
		{" }}}}}}}}}}}}}}}}}}}", NULL,	MenuGSM_APN_USER_NAME, NULL},
		{" ---APN-PASSWORD----", NULL,	NULL,	NULL},
		{" }}}}}}}}}}}}}}}}}}}", NULL,	MenuGSM_APN_PASSWORD, NULL},
		{" --Send UDP to IP---", NULL,	NULL, NULL},
		{" IP1:}}}.}}}.}}}.}}}", NULL, MenuGSM_IPUDP1, NULL},
		{" IP2:}}}.}}}.}}}.}}}", NULL, MenuGSM_IPUDP2, NULL},
		{" IP3:}}}.}}}.}}}.}}}", NULL, MenuGSM_IPUDP3, NULL},
		{" IP4:}}}.}}}.}}}.}}}", NULL, MenuGSM_IPUDP4, NULL},
		{" ---- UDP Port -----", NULL,	NULL, NULL},
		{" IP1 Port:}}}}}     ", NULL, MenuGSM_PortUDP1, NULL},
		{" IP2 Port:}}}}}     ", NULL, MenuGSM_PortUDP2, NULL},
		{" IP3 Port:}}}}}     ", NULL, MenuGSM_PortUDP3, NULL},
		{" IP4 Port:}}}}}     ", NULL, MenuGSM_PortUDP4, NULL},
		{" - TCP IP Firewall--", NULL,	NULL, NULL},
		{" IP1:}}}.}}}.}}}.}}}", NULL, MenuGSM_IPTCP1, NULL},
		{" IP2:}}}.}}}.}}}.}}}", NULL, MenuGSM_IPTCP2, NULL},
		{" IP3:}}}.}}}.}}}.}}}", NULL, MenuGSM_IPTCP3, NULL},
		{" IP4:}}}.}}}.}}}.}}}", NULL, MenuGSM_IPTCP4, NULL},
		{" TCPServerPort:}}}}}", NULL, MenuGSM_TCPPort, NULL},
		{" Timeout TCP:}}}}}}c", NULL, MenuGSM_TCPTimeout, NULL},
		{" ------- SMS -------", NULL,	NULL, NULL},
		{" Рассылка SMS:   }}}", NULL, MenuGSM_SMS, NULL},
		{" --Identification---", NULL,	NULL, NULL},
#ifdef VEGA_SN
		{" Vega S/N:     }}}}}", NULL,	MenuGSM_VegaSN, NULL},
#endif
#ifdef GSM_PASSWORD
		{" GSM_Password: }}}}}", NULL,	MenuGSM_Password, NULL},
#endif
	#ifdef GSM_DEBUG
		{" Консоль            ", NULL,	NULL, &MenuGSMConsole},
	#endif
	};
	MenuPage MenuGSM = {SetMenu(ListGSM), 0, NULL};

	#ifdef GSM_DEBUG
		
		OutField MenuGSMConsole_DebugStr[] = { {Text, GPRS_DebugStr, DIGIT, UKR_CAPS, GPRS_RunDebugStr, NULL, 0} }; 
		InField MenuGSMConsole0[] = { {Text, &GPRS_RxTerminalStr[0][0], NULL, 0} };
		InField MenuGSMConsole1[] = { {Text, &GPRS_RxTerminalStr[1][0], NULL, 0} };
		InField MenuGSMConsole2[] = { {Text, &GPRS_RxTerminalStr[2][0], NULL, 0} };
		InField MenuGSMConsole3[] = { {Text, &GPRS_RxTerminalStr[3][0], NULL, 0} };
		InField MenuGSMConsole4[] = { {Text, &GPRS_RxTerminalStr[4][0], NULL, 0} };
		InField MenuGSMConsole5[] = { {Text, &GPRS_RxTerminalStr[5][0], NULL, 0} };
		InField MenuGSMConsole6[] = { {Text, &GPRS_RxTerminalStr[6][0], NULL, 0} };
		InField MenuGSMConsole7[] = { {Text, &GPRS_RxTerminalStr[7][0], NULL, 0} };
		InField MenuGSMConsole8[] = { {Text, &GPRS_RxTerminalStr[8][0], NULL, 0} };
		InField MenuGSMConsole9[] = { {Text, &GPRS_RxTerminalStr[9][0], NULL, 0} };
		InField MenuGSMConsole10[] = { {Text, &GPRS_RxTerminalStr[10][0], NULL, 0} };
		InField MenuGSMConsole11[] = { {Text, &GPRS_RxTerminalStr[11][0], NULL, 0} };
		InField MenuGSMConsole12[] = { {Text, &GPRS_RxTerminalStr[12][0], NULL, 0} };
		InField MenuGSMConsole13[] = { {Text, &GPRS_RxTerminalStr[13][0], NULL, 0} };
		InField MenuGSMConsole14[] = { {Text, &GPRS_RxTerminalStr[14][0], NULL, 0} };
		InField MenuGSMConsole15[] = { {Text, &GPRS_RxTerminalStr[15][0], NULL, 0} };
		InField MenuGSMConsole16[] = { {Text, &GPRS_RxTerminalStr[16][0], NULL, 0} };
		InField MenuGSMConsole17[] = { {Text, &GPRS_RxTerminalStr[17][0], NULL, 0} };
		InField MenuGSMConsole18[] = { {Text, &GPRS_RxTerminalStr[18][0], NULL, 0} };
		InField MenuGSMConsole19[] = { {Text, &GPRS_RxTerminalStr[19][0], NULL, 0} };

		MenuLine ListGSMConsole[] = {
			{" Состояние:      {{{", MenuGSM_State, 		NULL,				NULL},
			{" }}}}}}}}}}}}}}}}}}}", NULL,			MenuGSMConsole_DebugStr,	NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole0,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole1,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole2,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole3,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole4,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole5,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole6,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole7,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole8,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole9,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole10,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole11,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole12,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole13,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole14,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole15,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole16,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole17,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole18,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{{{{{", MenuGSMConsole19,	NULL, NULL}
		};
		MenuPage MenuGSMConsole = {SetMenu(ListGSMConsole), 0, NULL};
	
	#endif	//GSM_DEBUG
// ~~~~~~~~~~~~~~~
#endif	//SIM900R
// ~~~~~~~~~~~~~~~
#endif	// def GSM
// ~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_str Mem_OK_Tbl[] = {Norm_Str, Fail_Str};

InField F_Ok_Flash[]	= {{Bit, &MemFail, Mem_OK_Tbl, MemFail_Flash}};
InField F_Ok_EEPROM[]	= {{Bit, &MemFail, Mem_OK_Tbl, MemFail_EEPROM}};

#if defined (vmd2_3)
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//		Нет сохраненных настроек
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		MsgPage Msg_EEWarning = {
		{
			{"                    "},
			{" Перед продолжением "},
			{"работы проверьте все"},
			{"      настройки     "},
		},	NULL, NULL, NULL};
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//		Нет сохраненных настроек настроек
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		MsgPage Msg_NoBackUp = {
		{
			{"                    "},
			{"Сохранённых настроек"},
			{"    не обнаружено   "},
			{"                    "}
		},	NULL, NULL, NULL};
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//		Подтверждение сохранения всех настроек
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	uint8_t Menu_ConfirmationSave_Enter(){

		//сохранение даты backup-а
		eeprom_write_byte((uint8_t*)E2END, 0xAA);
		eeprom_write_byte(&DateBackUp.Day, Tm_MDay);
		eeprom_write_byte(&DateBackUp.Month, Tm_Mon);
		#ifdef M41T56
			eeprom_write_byte(&DateBackUp.Year, Tm_Year);
		#else
			eeprom_write_byte(&DateBackUp.Year, erb(&Tm_Year));
		#endif
		eeprom_write_byte(&MemCheck_stor.ReprogFlg, 0);

		//перезагрузка
		for(;;){}
		return StandartKey;
	}

		MenuLine Menu_ConfirmationSave_List[] = {
			{"Сохранить настройки?"},
			{"   Да - кнопка ВХОД "},
			{"  Нет - кнопка ВЫХОД"},
			{"                    "},
		};
		MenuPage Menu_ConfirmationSave = { SetMenu(Menu_ConfirmationSave_List), 3, NULL,
				NULL, NULL, NULL, Menu_ConfirmationSave_Enter, NULL, NULL, NULL};
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//		Подтверждение востановления всех настроек
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	uint8_t Menu_ConfirmationRest_Enter(){

		eeprom_write_byte((uint8_t*)E2END, 0xBB);
		eeprom_write_byte(&MemCheck_stor.ReprogFlg, 0);
		//перезагрузка
		for(;;){}
		return StandartKey;
	}

		MenuLine Menu_ConfirmationRest_List[] = {
			{"Востанов. настройки?"},
			{"   Да - кнопка ВХОД "},
			{"  Нет - кнопка ВЫХОД"},
			{"                    "},
		};
		MenuPage Menu_ConfirmationRestore = { SetMenu(Menu_ConfirmationRest_List), 3, NULL,
			NULL, NULL, NULL, Menu_ConfirmationRest_Enter, NULL, NULL, NULL};
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//		Меню сохранения настроек
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	uint8_t Menu_BackUp_Enter(){
		switch(CurrLine){
		case 2:
			if(!EEMEMBackUpOk()){
				//если есть ошибка EEPROM, но нет BackUp-а - сброс аварии
				if(MemFail & 1<<MemFail_EEPROM){
					MemFailReset();
					ShowMsg(&Msg_EEWarning);
				}
				else ShowMsg(&Msg_NoBackUp);
				break;
			}
			NextPage = &Menu_ConfirmationRestore;
			break;
		}
		return StandartKey;
	}
		prog_char NoBackUp_Str[] =		"--";
		prog_str BackUp_Tbl[] = {NoBackUp_Str};		

		InField Menu_BackUp_IField[] = { {z_EE_Byte, &DateBackUp.Day, BackUp_Tbl, 0}, {z_EE_Byte, &DateBackUp.Month, BackUp_Tbl, 0}, {z_EE_Byte, &DateBackUp.Year, BackUp_Tbl, 0} };

		MenuLine Menu_BackUp_List[] = {
	/*0*/	{"Устн/Резервная копия"},
	/*1*/	{" Сохранить          ", NULL, NULL, &Menu_ConfirmationSave},
	/*2*/	{" Востанов:{{.{{.20{{", Menu_BackUp_IField},
	/*3*/	{" Сост.EEPROM: {{{{{ ", F_Ok_EEPROM},
		};
		MenuPage Menu_BackUp = { SetMenu(Menu_BackUp_List), 1, NULL,
				NULL, NULL, NULL, Menu_BackUp_Enter, NULL, NULL, NULL};
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t Key_Mem(void) {if(CurrLine==3) MemFailReset(); return StandartKey;}

MenuLine ListMem[] = {
	{" ----- Memory ----- "},
	{" Flash:  {{{{{      ", F_Ok_Flash},
	{" EEPROM: {{{{{      ", F_Ok_EEPROM},
	{" Reset EEPROM fail  "},
#if defined (vmd2_3)
	{" BackUp EEPROM      ", NULL, NULL, &Menu_BackUp},
#endif
};
MenuPage MenuMem = {SetMenu(ListMem), 1, .Enter=Key_Mem};


	


MsgErrPage(11, "     Flash fail     ")
MsgErrPage(12, "    EEPROM fail     ")
MsgErrPage(13, " Total memory fail  ")
MsgPage *MsgErrMemo[] PROGMEM = {
	[1<<MemFail_Flash]												= &MsgErr11,
	[1<<MemFail_EEPROM]												= &MsgErr12,
	[(1<<MemFail_Flash)+(1<<MemFail_EEPROM)]	= &MsgErr13
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
InField F_ScanCycle[] = {{Byte, &ScanCycle}};
InField F_MaxScanCycle[] = {{Byte, &MaxScanCycle}};

uint8_t Key_MaxScanCycle_Clear(void) {if(CurrLine==3) MaxScanCycle_Clear(); return StandartKey;}

MenuLine ListScanCycle[] = {
	{" --- Scan Cycle --- "},
	{" ScanCycle:   {{{0ms", F_ScanCycle},
	{" MaxScanCycle:{{{0ms", F_MaxScanCycle},
	{" Clear MaxScanCycle "}
};
MenuPage MenuScanCycle = {SetMenu(ListScanCycle), 1, .Enter=Key_MaxScanCycle_Clear};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

InField F_WT[] = {{z_Word, &Timer1hour}, {z_Byte, &Timer1min}};

#if LM_Exist
	InField F_InnTer[] = {{s_Word, &InnerTer, NULL, 1}};
#endif

#ifdef BACKLIGHT
	prog_char sec10_Str[] = 	"10 сек";
	prog_char sec30_Str[] = 	"30 сек";
	prog_char min1_Str[] =		"1 мин ";
	prog_char min3_Str[] = 		"3 мин ";
	prog_char min10_Str[] = 	"10 мин";
	prog_str BCKLT_Tbl[] = {sec10_Str, sec30_Str, min1_Str, min3_Str, min10_Str};
	OutField F_Backlight[] = { {EE_Enum, &BackLightTimeEE, BCKLT_10sec, BCKLT_10min, NULL, BCKLT_Tbl, 0} };
#endif

uint16_t IonThresh EEMEM = 512;
OutField F_IonT[] = {{EE_Word, &IonThresh, 0, 0x3FF}};
InField F_Menu_EMICounter[] = { {Longint, &KeyPressedCounter, NULL, 0} };

MenuLine ListSyst[] = {
	{" - Служебное меню - "},
	{" Дискретные входы   ", .InnPage=&MenuDI},
	{" Дискретные выходы  ", .InnPage=&MenuDO},
	{" Аналоговые входы   ", .InnPage=&MenuAI},
	{" Аналоговые выходы  ", .InnPage=&MenuAO},
#ifndef ADC_Miss
	{" АЦП                ", .InnPage=&MenuADC},
	{" Термодатчики       ", .InnPage=&MenuTherm},
#endif
#if LM_Exist
	{" Внутр.т-ра:{{{{{{|C", F_InnTer},
#endif
	{" Modbus             ", .InnPage=&MenuModbus},
#ifdef GSM
	{" Модем              ", .InnPage=&MenuGSM},
#endif
	{" { }}.}}.}} }}:}}:}}", F_WDay, F_DateTime},
#ifdef M41T56
	{" АвтоПерехЛетнВр:}}}", NULL, F_SummerTime},
#endif
	{" Наработка:{{{{{ч{{м", F_WT},
	{" Состояние памяти   ", .InnPage=&MenuMem},
	{" Scan Cycle:  {{{0ms", F_ScanCycle, NULL, &MenuScanCycle},
	{" Порог ионод.:  }}}}", NULL, F_IonT},
#ifdef BACKLIGHT
	{" ПодсветкаЖКИ:}}}}}}", NULL, F_Backlight, NULL},
#endif
	{" Счёт.ЭМН:{{{{{{{{{{", F_Menu_EMICounter, NULL, NULL}
};
MenuPage MenuSyst = {SetMenu(ListSyst), 1};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef DataFlash_PageSize
	MsgErrPage(31, "   Dataflash fail   ")
#endif
	MsgErrPage(41, "Timer allocation err")
