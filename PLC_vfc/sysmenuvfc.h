/*~~~~~~~~~~~~~
	22.12.2011 - Kononenko - Add меню GSM 
	29.12.2013 - Kononenko - Revised for vfc2
	04.01.2014 - Kononenko - #ifdef Modbus_Map_List
	07.02.2014 - Kononenko - Запуск Bootloader
// ~~~~~~~~~~~~~~~~~~~~*/
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_char Fail_Str[] = "Сбой ";
prog_char Norm_Str[] = "Норма";

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define MsgErrPage(N, Txt) MsgPage MsgErr##N = {{	\
	{"Intern. error "#N},														\
	{Txt},																					\
}};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_char DI_NO_Str[] = "нет";
prog_char DI_OK_Str[] = "нрм";
prog_str DI_Sta_Tbl[] = {DI_NO_Str, DI_OK_Str};

prog_char DI_Op_Str[] = "раз";
prog_char DI_Cl_Str[] = "зам";
prog_str DI_Tbl[] = {DI_Op_Str, DI_Cl_Str};

prog_char DI_Inv_Cl_Str[] = "нз";
prog_char DI_Inv_Op_Str[] = "нр";
prog_str DI_Inv_Tbl[] = {DI_Inv_Cl_Str, DI_Inv_Op_Str};

#define F_DI(N) InField F_DI##N[] = {{Bit, DigInput.State, DI_Tbl, N-1}, {Bit, DigInput.Status, DI_Sta_Tbl, N-1}};
F_DI(1)		F_DI(2)		F_DI(3)		F_DI(4)
F_DI(5)		
#ifndef vfc2
F_DI(6)
#endif

#define F_DII(N) OutField F_DII##N[] = {{EE_Bit, DI_Invers, .EnumList=DI_Inv_Tbl, N-1}};
F_DII(1)	F_DII(2)	F_DII(3)	F_DII(4)
F_DII(5)	
#ifndef vfc2
	F_DII(6)
#endif

#define L_1DI(N) {" N"#N":  {{{ }} {{{", F_DI##N,	F_DII##N}
#define L_2DI(N) {" N"#N": {{{ }} {{{", F_DI##N,	F_DII##N}
MenuLine ListDI[] = {
	L_1DI(1),		L_1DI(2),		L_1DI(3),		L_1DI(4),
	L_1DI(5),
#ifndef vfc2
	L_1DI(6)
#endif
};
MenuPage MenuDI = {SetMenu(ListDI)};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_char DO_Op_Str[] = "выкл";
prog_char DO_Cl_Str[] = "вкл ";
prog_str DO_Tbl[] = {DO_Op_Str, DO_Cl_Str};

#define F_DO(N) OutField F_DO##N[] = {{Bit, DigOutput.Buf, .EnumList=DO_Tbl, N-1}};
F_DO(1)		F_DO(2)		F_DO(3)		F_DO(4)
F_DO(5)		F_DO(6)

#define L_1DO(N) {" Выход N"#N":  }}}}", NULL, F_DO##N}
#define L_2DO(N) {" Выход N"#N": }}}}", NULL, F_DO##N}
MenuLine ListDO[] = {
	L_1DO(1),		L_1DO(2),		L_1DO(3),
	L_1DO(4),		L_1DO(5),		
#ifndef vfc2
	L_1DO(6),
#endif
};
MenuPage MenuDO = {SetMenu(ListDO)};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define F_AO(N) OutField F_AO##N[] = {{zc_Word, DAC_Output+N-1, 0, MaxAO}};
#define L_AO(N) {" Выход N"#N": }}}}}", NULL, F_AO##N}


F_AO(1)
F_AO(2)
#ifndef vfc2
F_AO(3)
#endif

MenuLine ListAO[] = {
	{" Аналог.выходы  "},
	L_AO(1),
	L_AO(2),
#ifndef vfc2
	L_AO(3)
#endif
};
MenuPage MenuAO = {SetMenu(ListAO), 1};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef vfc2
#define F_AI(N1, N2) InField F_AI##N1##N2[] = {{z_Word, AnalogInput+N1-1}, {z_Word, AnalogInput+N2-1}};
F_AI(1, 2)

#define L_AI(N1, N2) {" "#N1":{{{{{ "#N2":{{{{{", F_AI##N1##N2}
MenuLine ListAI[] = {
	L_AI(1, 2),
};
MenuPage MenuAI = {SetMenu(ListAI)};
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_char Therm_Off_Str[] =	"   x   ";
prog_char ADC_Off_Str[] =		"-------";
prog_str ThermTbl[] = {Therm_Off_Str, ADC_Off_Str};
#define F_Therm(N) InField F_Therm##N[] = {{s_Word, Temperature+N-1, ThermTbl, 1}};
F_Therm(1)
F_Therm(2)
F_Therm(3)

prog_char ThermOff_Str[]	= "Откл.";
prog_char ThermOn_Str[]		= "Вкл. ";

PGM_P RTD_Name_List[] PROGMEM = {ThermOff_Str, ThermOn_Str};

#define F_Thermis(N) OutField F_Thermis##N[] = {{EE_Enum, ThermType+N-1, 0, MaxThermDS18B20, NULL, RTD_Name_List}};
F_Thermis(1)
F_Thermis(2)
F_Thermis(3)

#define F_ThermisD(N) OutField F_ThermisD##N[] = {{s_EE_Byte, &ThermCorr[N-1].D, -128, 127, .Prec=1}};
F_ThermisD(1)
F_ThermisD(2)
F_ThermisD(3)

#define F_ThermisA(N) OutField F_ThermisA##N[] = {{EE_Word, &ThermCorr[N-1].A, ThermCorrA_Min, ThermCorrA_Max, .Prec=3}};
F_ThermisA(1)
F_ThermisA(2)
F_ThermisA(3)

#define M_Therm(N) MenuLine ListTherm##N[] = {				\
		{" N"#N":   {{{{{{{|C", F_Therm##N},							\
		{" Датчик:  }}}}}}", NULL, F_Thermis##N},					\
		{" Сдвиг:  }}}}}|C", NULL, F_ThermisD##N},				\
		{" Коэфф.:   }}}}}", NULL, F_ThermisA##N}					\
	};																									\
	MenuPage MenuTherm##N = {SetMenu(ListTherm##N), 1};
M_Therm(1)
M_Therm(2)
M_Therm(3)

#define L_Thermis(N) {" N"#N":   {{{{{{{|C", F_Therm##N, NULL, &MenuTherm##N}
MenuLine List_Therm[] = {
	L_Thermis(1),
	L_Thermis(2),
	L_Thermis(3)
};
MenuPage MenuTherm = {SetMenu(List_Therm)};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef Modbus_Map_List
	prog_char NoPariStr[]		= "нет";
	prog_char EvenPariStr[]	= "чет.";
	prog_char OddPariStr[]	= "нечет.";
	prog_str MBPariTbl[] = {NoPariStr, EvenPariStr, OddPariStr};

	#define BR_Str(BR) prog_char BR_Str##BR[]	= #BR;
	BR_Str(1200)	BR_Str(2400)	BR_Str(4800)
	BR_Str(9600)	BR_Str(14400)	BR_Str(19200)
	BR_Str(28800)	BR_Str(38400)	BR_Str(57600)
	prog_str BaudTbl[] = {
		BR_Str1200, BR_Str2400, BR_Str4800,
		BR_Str9600, BR_Str14400, BR_Str19200,
		BR_Str28800, BR_Str38400, BR_Str57600
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
		{" 1:{{{{{ 5:{{{{{", MB##N##_CPT_04},																																			\
		{" 2:{{{{{ 6:{{{{{", MB##N##_CPT_15},																																			\
		{" 3:{{{{{ 7:{{{{{", MB##N##_CPT_26},																																			\
		{" 4:{{{{{ 8:{{{{{", MB##N##_CPT_37}																																			\
	};																																																					\
	MenuPage M_MB##N##_CPT = {SetMenu(List_MB##N##_CPT), .Enter=Key_MB##N##_CPT_Clear};													\
																																																							\
	MenuLine L_MB##N[] = {																																											\
		{" -- Modbus "#N" -- "},																																									\
		{" Диагн. счетчики", .InnPage=&M_MB##N##_CPT},																														\
		{" Паритет: }}}}}}", NULL, F_MB##N##_Pari},																																\
		{" Стоп-биты: }   ", NULL, F_MB##N##_Stopbits},																														\
		{" BAUD: }}}}}bps ", NULL, F_MB##N##_Baud}																																\
	};																																																					\
	MenuPage MenuModbus##N = {SetMenu(L_MB##N), 1};

	M_Modbus(0)
	#if Modbus_Qt>1
		M_Modbus(1)
	#endif

	MenuLine List_Modbus[] = {
		{" Modbus 0       ", .InnPage=&MenuModbus0},
	#if Modbus_Qt>1
		{" Modbus 1       ", .InnPage=&MenuModbus1},
	#else
		{"    X           "},
	#endif
		{"    X           "},
		{"    X           "},
	};
	MenuPage MenuModbus = {SetMenu(List_Modbus)};
#endif
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

	MenuLine ListGSM[] = {
		{" МодемSIM900R{{{", MenuGSM_State, 		NULL,				NULL},
		{" Mode:    }}}}}}", NULL, 					MenuGSM_GSM_Cont,	NULL},
		{" -My IP-address-", NULL,	NULL, NULL},
		{" {{{.{{{.{{{.{{{", MenuGSM_GSM_MyIP, 		NULL, 				NULL},
		{" -Client IP-----", NULL,	NULL, NULL},
		{" {{{.{{{.{{{.{{{", MenuGSM_ClientIP, 		NULL, 				NULL},
		{" Active con. {{{", MenuGSM_ActiveCon,		NULL, 				NULL},
		{" FIFO:max {{/70b", MenuGSM_FIFOMax, 		NULL, 				NULL},
		{" -----APN-------", NULL,	NULL, NULL},
		{" }}}}}}}}}}}}}}}", NULL,	MenuGSM_APN1, NULL},
		{" }}}}}}}}}}}}}}}", NULL,	MenuGSM_APN2, NULL},
		{" -APN-USER NAME-", NULL,	NULL, 	NULL},
		{" }}}}}}}}}}}}}}}", NULL,	MenuGSM_APN_USER_NAME, NULL},
		{" -APN-PASSWORD--", NULL,	NULL,	NULL},
		{" }}}}}}}}}}}}}}}", NULL,	MenuGSM_APN_PASSWORD, NULL},
		{" Send UDP to IP1", NULL,	NULL, NULL},
		{" }}}.}}}.}}}.}}}", NULL, MenuGSM_IPUDP1, NULL},
		{" Send UDP to IP2", NULL,	NULL, NULL},
		{" }}}.}}}.}}}.}}}", NULL, MenuGSM_IPUDP2, NULL},
		{" Send UDP to IP3", NULL,	NULL, NULL},
		{" }}}.}}}.}}}.}}}", NULL, MenuGSM_IPUDP3, NULL},
		{" Send UDP to IP4", NULL,	NULL, NULL},
		{" }}}.}}}.}}}.}}}", NULL, MenuGSM_IPUDP4, NULL},
		{" -- UDP Port ---", NULL,	NULL, NULL},
		{" IP1 Port:}}}}} ", NULL, MenuGSM_PortUDP1, NULL},
		{" IP2 Port:}}}}} ", NULL, MenuGSM_PortUDP2, NULL},
		{" IP3 Port:}}}}} ", NULL, MenuGSM_PortUDP3, NULL},
		{" IP4 Port:}}}}} ", NULL, MenuGSM_PortUDP4, NULL},
		{" -IP1 Firewall--", NULL,	NULL, NULL},
		{" }}}.}}}.}}}.}}}", NULL, MenuGSM_IPTCP1, NULL},
		{" -IP2 Firewall--", NULL,	NULL, NULL},
		{" }}}.}}}.}}}.}}}", NULL, MenuGSM_IPTCP2, NULL},
		{" -IP3 Firewall--", NULL,	NULL, NULL},
		{" }}}.}}}.}}}.}}}", NULL, MenuGSM_IPTCP3, NULL},
		{" -IP4 Firewall--", NULL,	NULL, NULL},
		{" }}}.}}}.}}}.}}}", NULL, MenuGSM_IPTCP4, NULL},
		{" TCPSrvPrt:}}}}}", NULL, MenuGSM_TCPPort, NULL},
		{" Timeout:}}}}}}c", NULL, MenuGSM_TCPTimeout, NULL},
		{" ----- SMS -----", NULL,	NULL, NULL},
		{" РассылкаSMS:}}}", NULL, MenuGSM_SMS, NULL},
	#ifdef GSM_DEBUG
		{" Консоль        ", NULL,	NULL, &MenuGSMConsole},
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
			{" Состояние:  {{{", MenuGSM_State, 		NULL,				NULL},
			{" }}}}}}}}}}}}}}}", NULL,			MenuGSMConsole_DebugStr,	NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole0,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole1,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole2,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole3,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole4,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole5,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole6,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole7,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole8,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole9,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole10,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole11,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole12,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole13,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole14,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole15,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole16,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole17,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole18,	NULL, NULL},
			{"{{{{{{{{{{{{{{{{", MenuGSMConsole19,	NULL, NULL}
		};
		MenuPage MenuGSMConsole = {SetMenu(ListGSMConsole), 0, NULL};
	
	#endif	//GSM_DEBUG
// ~~~~~~~~~~~~~~~
#endif	//SIM900R
// ~~~~~~~~~~~~~~~
#endif	// def GSM
// ~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_str Mem_OK_Tbl[] = {Norm_Str, Fail_Str};

InField F_Ok_Flash[]	= {{Bit, &MemFail, Mem_OK_Tbl, MemFail_Flash}};
InField F_Ok_EEPROM[]	= {{Bit, &MemFail, Mem_OK_Tbl, MemFail_EEPROM}};

uint8_t Key_Mem(void) {if(CurrLine==2) MemFailReset(); return StandartKey;}

MenuLine ListMem[] = {
	{" Flash:  {{{{{  ", F_Ok_Flash},
	{" EEPROM: {{{{{  ", F_Ok_EEPROM},
	{" Сброс сбоя EEPR"}
};
MenuPage MenuMem = {SetMenu(ListMem), 0, .Enter=Key_Mem};

MsgErrPage(11, "   Flash fail   ")
MsgErrPage(12, "  EEPROM fail   ")
MsgErrPage(13, " Total mem fail ")
MsgPage *MsgErrMemo[] PROGMEM = {
	[1<<MemFail_Flash]												= &MsgErr11,
	[1<<MemFail_EEPROM]												= &MsgErr12,
	[(1<<MemFail_Flash)+(1<<MemFail_EEPROM)]	= &MsgErr13
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
InField F_ScanCycle[] = {{Byte, &ScanCycle}};
InField F_MaxScanCycle[] = {{Byte, &MaxScanCycle}};

uint8_t Key_MaxScanCycle_Clear(void) {if(CurrLine==2) MaxScanCycle_Clear(); return StandartKey;}

MenuLine ListScanCycle[] = {
	{" Cycle:   {{{0ms", F_ScanCycle},
	{" MaxCycle:{{{0ms", F_MaxScanCycle},
	{" Clear MaxCycle "}
};
MenuPage MenuScanCycle = {SetMenu(ListScanCycle), 1, .Enter=Key_MaxScanCycle_Clear};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
InField LCD_BUF_Error1_InField[] = { {Byte, &LCD_Buf[2][0],NULL, 0}, {Byte, &LCD_Buf[2][1],NULL, 0}, {Byte, &LCD_Buf[2][2],NULL, 0}, {Byte, &LCD_Buf[2][3],NULL, 0} };
InField LCD_BUF_Error2_InField[] = { {Byte, &LCD_Buf[2][4],NULL, 0}, {Byte, &LCD_Buf[2][5],NULL, 0}, {Byte, &LCD_Buf[2][6],NULL, 0}, {Byte, &LCD_Buf[2][7],NULL, 0} };
InField LCD_BUF_Error3_InField[] = { {Byte, &LCD_Buf[2][8],NULL, 0}, {Byte, &LCD_Buf[2][9],NULL, 0}, {Byte, &LCD_Buf[2][10],NULL, 0}, {Byte, &LCD_Buf[2][11],NULL, 0} };
InField LCD_BUF_Error4_InField[] = { {Byte, &LCD_Buf[2][12],NULL, 0}, {Byte, &LCD_Buf[2][13],NULL, 0}, {Byte, &LCD_Buf[2][14],NULL, 0}, {Byte, &LCD_Buf[2][15],NULL, 0} };

MenuLine Menu_LCD_Buf_List[] ={
	{" {{{ {{{ {{{ {{{", LCD_BUF_Error1_InField, NULL, NULL},
	{" {{{ {{{ {{{ {{{", LCD_BUF_Error2_InField, NULL, NULL},
	{" {{{ {{{ {{{ {{{", LCD_BUF_Error3_InField, NULL, NULL},
	{" {{{ {{{ {{{ {{{", LCD_BUF_Error4_InField, NULL, NULL},
};
MenuPage Menu_LCD_Buf = { SetMenu(Menu_LCD_Buf_List), 0, NULL};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t Menu_Bootloader_Enter(void){
	cli();
	ewb((uint8_t*)E2END, 0xFF);	//признак наличия Application program
	while(1){}		//reset
	return CustomKey;
}

MenuLine Menu_Bootloader_List[] ={
	{" Для запуска BL ", NULL, NULL, NULL},
	{" нажать Вход.   ", NULL, NULL, NULL},
};
MenuPage Menu_Bootloader = { SetMenu(Menu_Bootloader_List), 0, NULL,
	.Enter = Menu_Bootloader_Enter};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t Menu_Updater_Enter(void){
	cli();
	(*((void(*)(void))ADDR_UPDATER))();			//jump
	return 0;
}

MenuLine Menu_Updater_List[] ={
	{" Для обновления ", NULL, NULL, NULL},
	{" BL нажать Вход ", NULL, NULL, NULL},
};
MenuPage Menu_Updater = { SetMenu(Menu_Updater_List), 0, NULL,
	.Enter = Menu_Updater_Enter};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
InField F_WDay[] = {{Byte, &Tm_WDay}};
OutField F_Date[] = {
	{z_Byte,		&Tm_MDay,	1, 31, SetMDay},
	{z_Byte,		&Tm_Mon,	1, 12, SetMon},
	{z_Byte,		&Tm_Year,	0, 99, SetYear},
};
OutField F_Time[] = {
	{z_Byte,		&Tm_Hour,	0, 23, SetHour},
	{z_Byte,		&Tm_Min,	0, 59, SetMin},
	{z_Byte,		&Tm_Sec,	0, 59, SetSec}
};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
InField F_WT[] = {{z_Word, &Timer1hour}, {z_Byte, &Timer1min}};

MenuLine ListSyst[] = {
	{" - Служебное -  "},
	{" Дискрет. входы ", .InnPage=&MenuDI},
	{" Дискрет. выходы", .InnPage=&MenuDO},
#ifdef vfc2
	{" Аналог. входы  ", .InnPage=&MenuAI},
#endif
	{" Аналог. выходы ", .InnPage=&MenuAO},
	{" Термодатчики   ", .InnPage=&MenuTherm},
#ifdef Modbus_Map_List
	{" Modbus         ", .InnPage=&MenuModbus},
#endif
#ifdef UDR_GSM
	{" GSM модем      ", .InnPage=&MenuGSM},
#endif
	{" Дата:{ }}.}}.}}", F_WDay, F_Date},
	{" Время: }}:}}:}}", NULL, F_Time},
	{" Вкл.:{{{{{ч {{м", F_WT},
	{" Сост. памяти   ", .InnPage=&MenuMem},
	{" Cycle:   {{{0ms", F_ScanCycle, NULL, &MenuScanCycle},
	{" Буфер LCD_Buf  ", NULL, NULL, &Menu_LCD_Buf},
	{" Bootloader     ", NULL, NULL, &Menu_Bootloader},
	{" Update BL      ", NULL, NULL, &Menu_Updater},
};
MenuPage MenuSyst = {SetMenu(ListSyst), 1};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef DataFlash_PageSize
	MsgErrPage(31, "   Dataflash fail   ")
#endif
