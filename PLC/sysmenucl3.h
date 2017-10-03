#include "sysmenu.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F_DI(1)		F_DI(2)		F_DI(3)		F_DI(4)
F_DI(5)		F_DI(6)		F_DI(7)		F_DI(8)
F_DI(9)		F_DI(10)	F_DI(11)	F_DI(12)
F_DI(13)	F_DI(14)	F_DI(15)	F_DI(16)
F_DI(17)	F_DI(18)	F_DI(19)	F_DI(20)
F_DI(21)	F_DI(22)	F_DI(23)	F_DI(24)

F_DII(1)	F_DII(2)	F_DII(3)	F_DII(4)
F_DII(5)	F_DII(6)	F_DII(7)	F_DII(8)
F_DII(9)	F_DII(10)	F_DII(11)	F_DII(12)
F_DII(13)	F_DII(14)	F_DII(15)	F_DII(16)
F_DII(17)	F_DII(18)	F_DII(19)	F_DII(20)
F_DII(21)	F_DII(22)	F_DII(23)	F_DII(24)

MenuLine ListDI[] = {
	L_1DI(1),		L_1DI(2),		L_1DI(3),		L_1DI(4),
	L_1DI(5),		L_1DI(6),		L_1DI(7),		L_1DI(8),
	L_1DI(9),		L_2DI(10),	L_2DI(11),	L_2DI(12),
	L_2DI(13),	L_2DI(14),	L_2DI(15),	L_2DI(16),
	L_2DI(17),	L_2DI(18),	L_2DI(19),	L_2DI(20),
	L_2DI(21),	L_2DI(22),	L_2DI(23),	L_2DI(24)
};
MenuPage MenuDI = {SetMenu(ListDI)};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F_DO(1)		F_DO(2)		F_DO(3)		F_DO(4)
F_DO(5)		F_DO(6)		F_DO(7)		F_DO(8)
F_DO(9)		F_DO(10)	F_DO(11)	F_DO(12)
F_DO(13)	F_DO(14)	F_DO(15)	F_DO(16)

MenuLine ListDO[] = {
	L_1DO(1),		L_1DO(2),		L_1DO(3),		L_1DO(4),
	L_1DO(5),		L_1DO(6),		L_1DO(7),		L_1DO(8),
	L_1DO(9),		L_2DO(10),	L_2DO(11),	L_2DO(12),
	L_2DO(13),	L_2DO(14),	L_2DO(15),	L_2DO(16)
};
MenuPage MenuDO = {SetMenu(ListDO)};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F_AI(1, 2)
F_AI(3, 4)
F_AI(5, 6)
F_AI(7, 8)
F_AI(9, 10)
F_AI(11, 12)

MenuLine ListAI[] = {
	L_AI(1, 2),
	L_AI(3, 4),
	L_AI(5, 6),
	L_AI(7, 8),
	{" N9: {{{{{ N10:{{{{{", F_AI910},
	{" N11:{{{{{ N12:{{{{{", F_AI1112}
};
MenuPage MenuAI = {SetMenu(ListAI)};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F_AO(1)
F_AO(2)

MenuLine ListAO[] = {
	{" - Аналог. выходы - "},
	L_AO(1),
	L_AO(2),
	{"                    "}
};
MenuPage MenuAO = {SetMenu(ListAO), 1};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef ADC_Miss
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F_ADC(1, 2)
F_ADC(3, 4)
F_ADC(5, 6)
#ifdef ADC_Ref_Chan
	InField F_ADC_Ref[]	= {{Word, ADC_Input+sizeof(Mux)-1}};
	InField F_ADC7[]		= {{Word, ADC_Input+6}};
#else
	F_ADC(7, 8)
#endif
F_ADC(9, 10)

MenuLine List_ADC_Chan[] = {
	L_ADC(1, 2),
	L_ADC(3, 4),
	L_ADC(5, 6),
#ifdef ADC_Ref_Chan
	{" N7: {{{{{          ", F_ADC7},
	{" Опорный (N8): {{{{{", F_ADC_Ref},
#else
	L_ADC(7, 8),
#endif
	{" N9: {{{{{ N10:{{{{{", F_ADC910}
};
MenuPage Menu_ADC_Ch = {SetMenu(List_ADC_Chan)};

#define F_ADC_Zero(N) InField F_ADC_Zero##N[] = {{Byte, ADC_Scale##N.Zero+2}, {z_Byte, ADC_Scale##N.Zero+1}, {z_Byte, ADC_Scale##N.Zero}};
F_ADC_Zero(1)
F_ADC_Zero(2)
F_ADC_Zero(3)

#define F_ADC_Full(N) InField F_ADC_Full##N[] = {{Byte, ADC_Scale##N.Full+2}, {z_Byte, ADC_Scale##N.Full+1}, {z_Byte, ADC_Scale##N.Full}};
F_ADC_Full(1)
F_ADC_Full(2)
F_ADC_Full(3)

#define L_ADC_Reg(N, Reg) {" "#Reg#N": {{{ {{{ {{{ ", F_ADC_##Reg##N}
MenuLine List_ADC_Cali[] = {
	L_ADC_Reg(1, Zero), L_ADC_Reg(1, Full),
	L_ADC_Reg(2, Zero), L_ADC_Reg(2, Full),
	L_ADC_Reg(3, Zero), L_ADC_Reg(3, Full)
};
MenuPage Menu_ADC_Cali = {SetMenu(List_ADC_Cali)};

prog_char ADC_Rt_Str[] = "Reset";
prog_char ADC_I1_Str[] = "Initialisat1";
prog_char ADC_C1_Str[] = "Calibration1";
prog_char ADC_I2_Str[] = "Initialisat2";
prog_char ADC_C2_Str[] = "Calibration2";
prog_char ADC_I3_Str[] = "Initialisat3";
prog_char ADC_C3_Str[] = "Calibration3";
prog_char ADC_Re_Str[] = "Read";
prog_char ADC_Of_Str[] = "Off";
prog_str ADC_Tbl[] = {ADC_Rt_Str, ADC_I1_Str, ADC_C1_Str, ADC_I2_Str, ADC_C2_Str, ADC_I3_Str, ADC_C3_Str, ADC_Re_Str, ADC_Of_Str};

// ~~~~~~~~~~~~~~~~~~~~~
#endif	// ndef ADC_Miss
// ~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~
#ifndef ADC_Miss
// ~~~~~~~~~~~~~~~~~~~~~

F_Therm(1) F_Therm(2)
F_Therm(3) F_Therm(4)
F_Therm(5) F_Therm(6)
F_Therm(7)
F_Therm(9) F_Therm(10)

F_Thermis(1) F_Thermis(2)
F_Thermis(3) F_Thermis(4)
F_Thermis(5) F_Thermis(6)
F_Thermis(7)
OutField F_Thermoc9[]  = {{EE_Enum, ThermType+8,  0, MaxThermoc,  NULL, TC_Name_List}};
OutField F_Thermoc10[] = {{EE_Enum, ThermType+9, 0, MaxThermoc,  NULL, TC_Name_List}};

F_ThermisD(1) F_ThermisD(2)
F_ThermisD(3) F_ThermisD(4)
F_ThermisD(5) F_ThermisD(6)
F_ThermisD(7)
F_ThermisD(9) F_ThermisD(10)

F_ThermisA(1) F_ThermisA(2)
F_ThermisA(3) F_ThermisA(4)
F_ThermisA(5) F_ThermisA(6)
F_ThermisA(7)
F_ThermisA(9) F_ThermisA(10)

M_Therm(1) M_Therm(2)
M_Therm(3) M_Therm(4)
M_Therm(5) M_Therm(6)
M_Therm(7)

MenuLine ListTherm9[] = {
	{" Канал N9: {{{{{{{|C", F_Therm9},
	{" Термодатчик: }}}}}}", NULL, F_Thermoc9},
	{" Сдвиг:      }}}}}|C", NULL, F_ThermisD9},
	{" Коэффициент:  }}}}}", NULL, F_ThermisA9}
};
MenuPage MenuTherm9 = {SetMenu(ListTherm9), 1};

MenuLine ListTherm10[] = {
	{" Канал N10:{{{{{{{|C", F_Therm10},
	{" Термодатчик: }}}}}}", NULL, F_Thermoc10},
	{" Сдвиг:      }}}}}|C", NULL, F_ThermisD10},
	{" Коэффициент:  }}}}}", NULL, F_ThermisA10}
};
MenuPage MenuTherm10 = {SetMenu(ListTherm10), 1};

#ifndef ADC_Ref_Chan
	F_Therm(8)
	F_Thermis(8)
	F_ThermisD(8)
	F_ThermisA(8)
	M_Therm(8)
#endif

MenuLine List_Therm[] = {
	L_Thermis(1), L_Thermis(2),
	L_Thermis(3), L_Thermis(4),
	L_Thermis(5), L_Thermis(6),
	L_Thermis(7),
#ifndef ADC_Ref_Chan
	L_Thermis(8),
#else
	{" Канал N8: x (опор.)"},
#endif
	L_Thermis(9),
	{" Канал N10:{{{{{{{|C", F_Therm10, NULL, &MenuTherm10}
};
MenuPage MenuTherm = {SetMenu(List_Therm)};

// ~~~~~~~~~~~~~~~~~~~~~
#endif	// ndef ADC_Miss
// ~~~~~~~~~~~~~~~~~~~~~
