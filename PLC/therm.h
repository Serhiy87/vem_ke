
// 14.10.2013 * Kononenko * Fix bug in Check valid range for Гр.23 (last Thermis or Thermoc)
// 05.02.2015 /Тирин /Добавлены участки пресчёта для ТСМ50, ТСМ100, Гр.23
// 15.04.2015 /Тирин /Добавлен участок пресчёта для ТСМ100 (-50)
// 23.06.2015 * Kononenko * Add point {18146,0} for TSP100 - по результатам метрологических аттестаций
// 30.03.2017 Kucherenko - Go AVRStudio4 on AtmelStudio7:
// typedef struct {...} PROGMEM TerLim; -> typedef const struct {...} PROGMEM TerLim;
// ... All variables PROGMEM -> ...const ... PROGMEM

#include "digiproc.h"

#define Therm_Off_Mark	-0x8000
#define ADC_Off_Mark		-0x7FFF

#define ThermOff 0

enum {Thermis, Thermoc};	// Thermistor & Thermocouple
prog_uint8_t ThermFami[TerReg] = InitThermFami;

enum {TSM100=1, TSM50, TSP100, TSP50, Pt100, Gr21, Gr23};		// Thermistors
#if	TerInput==ADC_Input
	#define MaxThermis Gr23
#else
	#define MaxThermis Pt100
#endif

enum {TC_K=1, TC_B, TC_J, TC_L, TC_N, TC_S, TC_R};					// Thermocouples
#if LM_Exist
	#define MaxThermoc TC_L
#else
	#define MaxThermoc TC_B
#endif

#ifndef InitThermType
	#define InitThermType {}
#endif

uint8_t ThermType[TerReg] EEMEM = InitThermType;

static prog_uint8_t MaxThermType[] = {[Thermis]=MaxThermis, [Thermoc]=MaxThermoc};

#define ThermCorrA_Min  900
#define ThermCorrA_Max 1100
struct {
	uint8_t D;
	uint16_t A;
} ThermCorr[TerReg] EEMEM = InitThermCorr;

#ifndef Ter_Reg_Ext
	#define Ter_Reg_Ext 0
#endif
int16_t Temperature[TerReg+Ter_Reg_Ext];

enum {ThermNorm=0, ThermUnder, ThermOver};

#define TermisLow			-500
#define TermisUpp_Cu	1800
#define TermisUpp_Pt	6000
typedef const struct {
	uint16_t Low, Upp;
} PROGMEM TerLim;

TerLim TermisLim[] = {
	[TSM100]	= {TermisLow, TermisUpp_Cu},
	[TSM50]		= {TermisLow, TermisUpp_Cu},
	[TSP100]	= {TermisLow, TermisUpp_Pt},
	[TSP50]		= {TermisLow, TermisUpp_Pt},
	[Pt100]		= {TermisLow, TermisUpp_Pt},
	[Gr21]		= {TermisLow, TermisUpp_Pt},
	[Gr23]		= {TermisLow, TermisUpp_Cu}
};
TerLim TermocLim[] = {
	[TC_K] = {-2000, 12000},
	[TC_B] = { 6000, 18000},
	[TC_J] = {-2000,  7500},
	[TC_L] = {-2000,  6000}
};
static TerLim *const TerLimType[] PROGMEM = {[Thermis]=TermisLim, [Thermoc]=TermocLim};

#define TerFiltLev 4
static uint16_t TerBuf[TerReg][TerFiltLev*4];

#define ThermTbl(Name) static PGM_uDataPoint Name##Tbl[]
#define ThermCalc(Name) PGM_uBrokLine(Name##Tbl, Code)

#if	TerInput==ADC_Input

	ThermTbl(TSM100)={
		{0,			-2335},
		{2193,		-2000},	//Тирин
		{10254,		-1000},	//Тирин
		{14242,		-500},	//Тирин
		{18154,		0},		//Тирин
		{25929,		1000},	//Тирин
		{33702,		2000},	//Тирин
		{65535,		6132}
	};

	ThermTbl(TSM50)={
			{0,			-2337},
			{1094,		-2000},	//Тирин
			{3091,		-1500},	//Тирин
			{5118,		-1000},	//Тирин
			{7110,		-500},	//Тирин
			{9067,		0},		//Тирин
			{11011,		500},	//Тирин
			{12954,		1000},	//Тирин
			{14896,		1500},	//Тирин
			{16841,		2000},	//Тирин
			{65535,		14525}
		};

	ThermTbl(TSP100) = {
		{0,		-2570},
		{14530,	-500},
		{18146,	0},		//Kononenko add 23.06.2015
		{25270,	1000},
		{32150,	2000},
		{38830,	3000},
		{45290,	4000},
		{51540,	5000},
		{57570,	6000},
		{65535,	7831}
	};

	ThermTbl(TSP50) = {
		{0,		-2529},
		{7263,	-500},
		{12630,	1000},
		{16080,	2000},
		{19410,	3000},
		{22640,	4000},
		{25770,	5000},
		{28780,	6000},
		{65535,	18190}
	};

	ThermTbl(Pt100) = {
		{0,		-2528},
		{14590,	-500},
		{25160,	1000},
		{31940,	2000},
		{38510,	3000},
		{44880,	4000},
		{51030,	5000},
		{56980,	6000},
		{62710,	7000},
		{65535,	8075}
	};

	ThermTbl(Gr21) = {
		{0,		-2530},
		{6684,	-500},
		{11620,	1000},
		{14790,	2000},
		{17860,	3000},
		{20840,	4000},
		{23710,	5000},
		{26490,	6000},
		{65535,	20430}
	};

	ThermTbl(Gr23)={
		{0,			-2347},
		{7549,		-500},	//Тирин
		{9611,		0},		//Тирин
		{11663,		500},	//Тирин
		{13714,		1000},	//Тирин
		{15765,		1500},	//Тирин
		{16995,		1800},	//Тирин
		{65535,		13630}
	};

	ThermTbl(TC_B_) = {
		{0,			400},
		{409,		1500},
		{1916,	3000},
		{5518,	5000},
		{7963,	6000},
		{10800,	7000},
		{14020,	8000},
		{17590,	9000},
		{21490,	10000},
		{25690,	11000},
		{30160,	12000},
		{34880,	13000},
		{39800,	14000},
		{44880,	15000},
		{50050,	16000},
		{55250,	17000},
		{65535,	19000}
	};

#if LM_Exist
	ThermTbl(TC_K_) = {
		{0,			-1335},
		{3490,	-400},
		{4984,	0},
		{5962,	250},
		{6961,	500},
		{8178,	800},
		{8989,	1000},
		{12940,	2000},
		{16920,	3000},
		{21020,	4000},
		{25170,	5000},
		{29340,	6000},
		{33470,	7000},
		{37530,	8000},
		{41490,	9000},
		{45350,	10000},
		{49100,	11000},
		{65535,	15560}
	};

	ThermTbl(TC_J_) = {
		{0,			-1040},
		{3067,	-400},
		{4984,	0},
		{6233,	250},
		{7512,	500},
		{9078,	800},
		{10140,	1000},
		{13360,	1600},
		{16610,	2200},
		{19870,	2800},
		{23110,	3400},
		{26350,	4000},
		{29590,	4600},
		{32870,	5200},
		{36220,	5800},
		{39670,	6400},
		{43250,	7000},
		{65535,	10610}
	};

	ThermTbl(TC_L_) = {
		{0,			-839},
		{2549,	-410},
		{4984,	0},
		{6957,	300},
		{10280,	800},
		{15370,	1500},
		{19200,	2000},
		{23210,	2500},
		{27320,	3000},
		{32370,	3600},
		{35780,	4000},
		{40940,	4600},
		{46120,	5200},
		{51280,	5800},
		{56440,	6400},
		{59860,	6800},
		{64970,	7400},
		{65535,	7467}
	};
#endif
#else		// TerInput!=ADC_Input
	ThermTbl(TSM100) = {
		{0,			-2347},
		{1023,	4545}
	};
	ThermTbl(TSM50) = {
		{0,			-2347},
		{1023,	11440}
	};
	ThermTbl(TSP100) = {
		{0,			-2030},
		{279,		-500},
		{485,		1000},
		{617,		2000},
		{745,		3000},
		{869,		4000},
		{989,		5000},
		{1023,	5278}
	};
	ThermTbl(TSP50) = {
		{0,			-2025},
		{139,		-500},
		{242,		1000},
		{309,		2000},
		{372,		3000},
		{434,		4000},
		{494,		5000},
		{1023,	14680}
	};
	ThermTbl(Pt100) = {
		{0,			-2070},
		{280,		-500},
		{483,		1000},
		{613,		2000},
		{739,		3000},
		{861,		4000},
		{980,		5000},
		{1023,	5366}
	};
#endif	// TerInput==ADC_Input

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int16_t
Thermode(uint8_t T_Fami, uint8_t T_Type, uint16_t Code)
{
	if (T_Type==ThermOff)
		return Therm_Off_Mark;
	if (T_Fami==Thermis)
		switch(T_Type) {
		case TSM100:
			return ThermCalc(TSM100);
		case TSM50:
			return ThermCalc(TSM50);
		case TSP100:
			return ThermCalc(TSP100);
		case TSP50:
			return ThermCalc(TSP50);
		case Pt100:
			return ThermCalc(Pt100);
#if	TerInput==ADC_Input
		case Gr21:
			return ThermCalc(Gr21);
		case Gr23:
			return ThermCalc(Gr23);
#endif
		default:
			return Code;
		}
	else	// T_Fami==Thermoc
		switch(T_Type) {
#if LM_Exist
		case TC_K:
			return ThermCalc(TC_K_) + InnerTer;
		case TC_J:
			return ThermCalc(TC_J_) + InnerTer;
		case TC_L:
			return ThermCalc(TC_L_) + InnerTer;
#else
		case TC_K:
			return .1851*Code;
#endif
#if	TerInput==ADC_Input
		case TC_B:
			return ThermCalc(TC_B_);
#endif
		default:
			return Code;
		}
}

// ~~~~~~~~~~~~~~~~~~~~~
uint8_t
ThermInit(uint8_t ChanN)
{
	if (ChanN<TerReg) {
		for (uint8_t i=0; i<TerFiltLev*4; TerBuf[ChanN][i++]=TerInput[ChanN]);
		Temperature[ChanN] = Thermode(prb(ThermFami+ChanN), erb(ThermType+ChanN), TerInput[ChanN]);
	}
	return ChanN!=TerReg-1;
}

// ~~~~~~~~~~~~~~~~~~~~~
void
ThermChan(uint8_t ChanN)
{
	if (ChanN>=TerReg)
		return;
	for (uint8_t i=TerFiltLev*4-1; i; i--)
		TerBuf[ChanN][i] = TerBuf[ChanN][i-1];
	TerBuf[ChanN][0] = TerInput[ChanN];
	int16_t Ter = Thermode(prb(ThermFami+ChanN), erb(ThermType+ChanN), DigiFilt(TerBuf[ChanN],TerFiltLev));
	if (Ter!=Therm_Off_Mark)
		Ter = (Ter+(int8_t)erb(&ThermCorr[ChanN].D))*(erw(&ThermCorr[ChanN].A)/1000.0);
	Temperature[ChanN] = Ter;
}

// ~~~~~~~~~~~
void
ThermExt(void)
{
	#ifdef Temperature_Ext
		for (uint8_t i=0; i<Ter_Reg_Ext; i++){
			IntOff();
			Temperature[TerReg+i] = (Temperature_Ext)[i];
			IntOn();
		}
	#endif
}

// ~~~~~~~~~~~~~~~~
void
Therm_ADC_Off(void)
{
	for (uint8_t i=0; i<TerReg; Temperature[i++]=ADC_Off_Mark);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
ThermWrong(uint8_t T_Fami, uint8_t T_Type, int16_t Ter)
{
	uint8_t TW = ThermNorm;
	if (T_Type!=ThermOff && T_Type<=prb(MaxThermType+T_Fami)) {
		TerLim *T_L = prp(TerLimType+T_Fami);
		if (Ter<(int16_t)prw(&T_L[T_Type].Low))
			TW = ThermUnder;
		else if(Ter>(int16_t)prw(&T_L[T_Type].Upp))
			TW = ThermOver;
	}
	return TW;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
ThermChanWrong(uint8_t ChanN)
{
	return ThermWrong(prb(ThermFami+ChanN), erb(ThermType+ChanN), Temperature[ChanN]);
}
