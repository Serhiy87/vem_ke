/*
	07.10.2013 * Kononenko *  Revised for vmd2_2 ()
	19.12.2013 * Kononenko *  Revised for vmd2_3 ()
*/
#include "sysmenu.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F_DI(1)		F_DI(2)		F_DI(3)		F_DI(4)
F_DI(5)		F_DI(6)		F_DI(7)		F_DI(8)
F_DI(9)		F_DI(10)	F_DI(11)	F_DI(12)
F_DI(13)	F_DI(14)	F_DI(15)	F_DI(16)
#if defined (vmd2_3) 
	F_DI(17)	F_DI(18)
#endif

F_DII(1)	F_DII(2)	F_DII(3)	F_DII(4)
F_DII(5)	F_DII(6)	F_DII(7)	F_DII(8)
F_DII(9)	F_DII(10)	F_DII(11)	F_DII(12)
F_DII(13)	F_DII(14)	F_DII(15)	F_DII(16)
#if defined (vmd2_3)
	F_DII(17)	F_DII(18)
	#ifdef HIGH_SPEED_COUNTER	
		F_HSC(0)	F_HSC(1)
	#endif
#endif

MenuLine ListDI[] = {
	L_1DI(1),		L_1DI(2),	L_1DI(3),		L_1DI(4),
	L_1DI(5),		L_1DI(6),	L_1DI(7),		L_1DI(8),
	L_1DI(9),		L_2DI(10),	L_2DI(11),		L_2DI(12),
	L_2DI(13),		L_2DI(14),	L_2DI(15),		L_2DI(16),
#if defined (vmd2_3)
	L_2DI(17),		L_2DI(18),
	#ifdef HIGH_SPEED_COUNTER	
		L_HSC(0),		L_HSC(1),
	#endif
#endif
};
MenuPage MenuDI = {SetMenu(ListDI)};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F_DO(1)		F_DO(2)		F_DO(3)		F_DO(4)
F_DO(5)		F_DO(6)		F_DO(7)		F_DO(8)
F_DO(9)		F_DO(10)	F_DO(11)	F_DO(12)
#if defined (vmd2_2) || defined (vmd2_3) 
F_DO(13)	F_DO(14)	F_DO(15)	F_DO(16)
#endif
MenuLine ListDO[] = {
	L_1DO(1),		L_1DO(2),		L_1DO(3),		L_1DO(4),
	L_1DO(5),		L_1DO(6),		L_1DO(7),		L_1DO(8),
	L_1DO(9),		L_2DO(10),		L_2DO(11),		L_2DO(12),
#if defined (vmd2_2) || defined (vmd2_3) 
	L_2DO(13),		L_2DO(14),		L_2DO(15),		L_2DO(16)
#endif
};
MenuPage MenuDO = {SetMenu(ListDO)};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F_AI(1, 2)
F_AI(3, 4)
F_AI(5, 6)
F_AI(7, 8)

MenuLine ListAI[] = {
	L_AI(1, 2),
	L_AI(3, 4),
	L_AI(5, 6),
	L_AI(7, 8)
};
MenuPage MenuAI = {SetMenu(ListAI), 3};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F_AO(1)
F_AO(2)
F_AO(3)
F_AO(4)

MenuLine ListAO[] = {
	L_AO(1),
	L_AO(2),
	L_AO(3),
	L_AO(4)
};
MenuPage MenuAO = {SetMenu(ListAO)};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef ADC_Miss
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F_ADC(1, 2)
F_ADC(3, 4)
F_ADC(5, 6)
#ifdef ADC_Ref_Chan
	InField F_ADC_Ref[] = {{Word, ADC_Input+sizeof(Mux)-1}};
#endif

MenuLine List_ADC_Chan[] = {
	{" --- Каналы АЦП --- "},
	L_ADC(1, 2),
	L_ADC(3, 4),
	L_ADC(5, 6),
#ifdef ADC_Ref_Chan
	{" Опорн. канал: {{{{{", F_ADC_Ref}
#endif
};
MenuPage Menu_ADC_Ch = {SetMenu(List_ADC_Chan), 1};

InField F_ADC_Zero[] = {{Byte, ADC_Scale1.Zero+2}, {z_Byte, ADC_Scale1.Zero+1}, {z_Byte, ADC_Scale1.Zero}};
InField F_ADC_Full[] = {{Byte, ADC_Scale1.Full+2}, {z_Byte, ADC_Scale1.Full+1}, {z_Byte, ADC_Scale1.Full}};

MenuLine List_ADC_Cali[] = {
	{" --- Калибровка --- "},
	{" Zero:  {{{ {{{ {{{ ", F_ADC_Zero},
	{" Full:  {{{ {{{ {{{ ", F_ADC_Full},
	{"                    "}
};
MenuPage Menu_ADC_Cali = {SetMenu(List_ADC_Cali), 3};

prog_char ADC_Rt_Str[] = "Reset";
prog_char ADC_I1_Str[] = "Initialisat";
prog_char ADC_C1_Str[] = "Calibration";
prog_char ADC_Re_Str[] = "Read";
prog_char ADC_Of_Str[] = "Off";
prog_str ADC_Tbl[] = {ADC_Rt_Str, ADC_I1_Str, ADC_C1_Str, ADC_Re_Str, ADC_Of_Str};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F_Therm(1) F_Therm(2)
F_Therm(3) F_Therm(4)
F_Therm(5) F_Therm(6)

F_Thermis(1) F_Thermis(2)
F_Thermis(3) F_Thermis(4)
F_Thermis(5) F_Thermis(6)

F_ThermisD(1) F_ThermisD(2)
F_ThermisD(3) F_ThermisD(4)
F_ThermisD(5) F_ThermisD(6)

F_ThermisA(1) F_ThermisA(2)
F_ThermisA(3) F_ThermisA(4)
F_ThermisA(5) F_ThermisA(6)

M_Therm(1) M_Therm(2)
M_Therm(3) M_Therm(4)
M_Therm(5) M_Therm(6)

MenuLine List_Therm[] = {
	L_Thermis(1), L_Thermis(2),
	L_Thermis(3), L_Thermis(4),
	L_Thermis(5), L_Thermis(6)
};
MenuPage MenuTherm = {SetMenu(List_Therm)};

// ~~~~~~~~~~~~~~~~~~~~~
#endif	// ndef ADC_Miss
// ~~~~~~~~~~~~~~~~~~~~~

