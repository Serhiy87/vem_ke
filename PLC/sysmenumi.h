/*~~~~~~~~~~~~~
	22.12.2011 - Kononenko - Add меню GSM 
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
F_DI(5)		F_DI(6)		F_DI(7)		F_DI(8)
F_DI(9)		F_DI(10)	F_DI(11)	F_DI(12)
F_DI(13)	F_DI(14)	F_DI(15)	F_DI(16)

#define F_DII(N) OutField F_DII##N[] = {{EE_Bit, DI_Invers, .EnumList=DI_Inv_Tbl, N-1}};
F_DII(1)	F_DII(2)	F_DII(3)	F_DII(4)
F_DII(5)	F_DII(6)	F_DII(7)	F_DII(8)
F_DII(9)	F_DII(10)	F_DII(11)	F_DII(12)
F_DII(13)	F_DII(14)	F_DII(15)	F_DII(16)

#define L_1DI(N) {" N"#N":  {{{ }} {{{", F_DI##N,	F_DII##N}
#define L_2DI(N) {" N"#N": {{{ }} {{{", F_DI##N,	F_DII##N}
MenuLine ListDI[] = {
	L_1DI(1),		L_1DI(2),		L_1DI(3),		L_1DI(4),
	L_1DI(5),		L_1DI(6),		L_1DI(7),		L_1DI(8),
	L_1DI(9),		L_2DI(10),	L_2DI(11),	L_2DI(12),
	L_2DI(13),	L_2DI(14),	L_2DI(15),	L_2DI(16)
};
MenuPage MenuDI = {SetMenu(ListDI)};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_char DO_Op_Str[] = "выкл";
prog_char DO_Cl_Str[] = "вкл ";
prog_str DO_Tbl[] = {DO_Op_Str, DO_Cl_Str};

#define F_DO(N) OutField F_DO##N[] = {{Bit, DigOutput.Buf, .EnumList=DO_Tbl, N-1}};
F_DO(1)		F_DO(2)		F_DO(3)		F_DO(4)
F_DO(5)		F_DO(6)		F_DO(7)		F_DO(8)
F_DO(9)		F_DO(10)	F_DO(11)	F_DO(12)

#define L_1DO(N) {" Выход N"#N":  }}}}", NULL, F_DO##N}
#define L_2DO(N) {" Выход N"#N": }}}}", NULL, F_DO##N}
MenuLine ListDO[] = {
	L_1DO(1),		L_1DO(2),		L_1DO(3),
	L_1DO(4),		L_1DO(5),		L_1DO(6),
	L_1DO(7),		L_1DO(8),		L_1DO(9),
	L_2DO(10),	L_2DO(11),	L_2DO(12)
};
MenuPage MenuDO = {SetMenu(ListDO)};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define F_AI(N1, N2) InField F_AI##N1##N2[] = {{z_Word, AnalogInput+N1-1}, {z_Word, AnalogInput+N2-1}};
F_AI(1, 2)
F_AI(3, 4)
F_AI(5, 6)
F_AI(7, 8)

#define L_AI(N1, N2) {" "#N1":{{{{{ "#N2":{{{{{", F_AI##N1##N2}
MenuLine ListAI[] = {
	L_AI(1, 2),
	L_AI(3, 4),
	L_AI(5, 6),
	L_AI(7, 8)
};
MenuPage MenuAI = {SetMenu(ListAI)};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
prog_char Therm_Off_Str[] =	"   x   ";
prog_char ADC_Off_Str[] =		"-------";
prog_str ThermTbl[] = {Therm_Off_Str, ADC_Off_Str};
#define F_Therm(N) InField F_Therm##N[] = {{s_Word, Temperature+N-1, ThermTbl, 1}};
F_Therm(1)
F_Therm(2)
F_Therm(3)

prog_char ThermOff_Str[]	= "Откл.";
prog_char TSM100_Str[]		= "ТСМ100";
prog_char TSM50_Str[]			= "ТСМ50";
prog_char TSP100_Str[]		= "ТСП100";
prog_char TSP50_Str[]			= "ТСП50";
prog_char Pt100_Str[]			= "Pt100";
PGM_P RTD_Name_List[] PROGMEM = {ThermOff_Str, TSM100_Str, TSM50_Str, TSP100_Str, TSP50_Str, Pt100_Str};

#define F_Thermis(N) OutField F_Thermis##N[] = {{EE_Enum, ThermType+N-1, 0, MaxThermis, NULL, RTD_Name_List}};
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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef UDR_GSM
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

uint8_t
Key_GSM(void)
{
	if(CurrLine==5)
		GSM_Reset();
	return 1;
}

MenuLine ListGSM[] = {
	{" --- Модем ---- "},
	{" }}}.}}}.}}}.}}}", NULL, F_GPRS_ServerAddr},
	{" Статус: {{{{{  ", F_GSM_OK},
	{" Состояние: {{{ ", F_GSM_State},
	{" <LF>: {{       ", F_GSM_LF},
	{" Reset->        "}
};
MenuPage MenuGSM = {SetMenu(ListGSM), 1, .Enter=Key_GSM};

// ~~~~~~~~~~~~~~~~~~~
#endif	// def UDR_GSM
// ~~~~~~~~~~~~~~~~~~~

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
InField F_WT[] = {{z_Word, &Timer1hour}, {z_Byte, &Timer1min}};

uint16_t IonThresh EEMEM = 512;
OutField F_IonT[] = {{EE_Word, &IonThresh, 0, 0x3FF}};

MenuLine ListSyst[] = {
	{" - Служебное -  "},
	{" Дискрет. входы ", .InnPage=&MenuDI},
	{" Дискрет. выходы", .InnPage=&MenuDO},
	{" Аналог. входы  ", .InnPage=&MenuAI},
	{" Термодатчики   ", .InnPage=&MenuTherm},
	{" Modbus         ", .InnPage=&MenuModbus},
#ifdef UDR_GSM
	{" GSM модем      ", .InnPage=&MenuGSM},
#endif
	{" Вкл.:{{{{{ч {{м", F_WT},
	{" Сост. памяти   ", .InnPage=&MenuMem},
	{" Cycle:   {{{0ms", F_ScanCycle, NULL, &MenuScanCycle},
	{" Порог ИД:  }}}}", NULL, F_IonT}
};
MenuPage MenuSyst = {SetMenu(ListSyst), 1};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef DataFlash_PageSize
	MsgErrPage(31, "   Dataflash fail   ")
#endif
