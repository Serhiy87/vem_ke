/* ~~~~~~~~~~~~~
 	30.09.2013 * Kononenko *  Потокол 1-Wire. OW = OneWire 

	The DS18B20 uses Maxim’s exclusive 1-Wire bus protocol that implements bus communication using one control signal.
	
	В ф-ии DS18B20() выполняется поочерёдный запуск измрения OneWireStartADC по всем(3) каналам.
	Через 1.0 сек считывается результат OneWireReadTemp()
	Контролируется наличие сигнала presence pulse от DS18B20, если нет прочерки "-------"
	Значение 85,0С ингорируется - отсеиваем начальные показания DS18B20 при первом считывании без запроса на запуск измерения

 	02.06.2014 * Kononenko *  Add DHT22 support (RH_CHANNNEL)

	11.07.2014 * Kononenko * Read DS18B20 10 times (10sec) before ADC_Off_Mark.

	14.04.2015 * Kononenko * Support TerReg == 1. Конфиг железа перенёс в sys_xxx.h


	TODO:
	1) Считывать весь sratchpad для проверки CRC8 - более достоверные данные
	2) Работать 1 бит (можно 1байт) за цикл - чтобы не загружать svan-cycle
	3) Разработать двее версии SINGLE_DROP и MULTI_DROP

~~~~~~~~~~~~~*/

void Read_DHT22(uint8_t CH);

#define Therm_Off_Mark	-0x8000
#define ADC_Off_Mark	-0x7FFF

#define ThermOff 0

enum {Thermis, Thermoc, ThermDS18B20};	// Thermistor & Thermocouple & DS18B20
prog_uint8_t ThermFami[TerReg] = InitThermFami;

enum {TSM100=1, TSM50, TSP100, TSP50, Pt100, Gr21, Gr23};		// Thermistors
#if	TerInput==ADC_Input
	#define MaxThermis Gr23
#else
	#define MaxThermis Pt100
#endif

enum {TC_K=1, TC_B, TC_J, TC_L, TC_N, TC_S, TC_R};				// Thermocouples
#if LM_Exist
	#define MaxThermoc TC_L
#else
	#define MaxThermoc TC_B
#endif

enum {ThermOn=1};												// ThermDS18B20
#define MaxThermDS18B20 ThermOn


#ifndef InitThermType
	#define InitThermType {}
#endif


uint8_t ThermType[TerReg] EEMEM = InitThermType;
static prog_uint8_t MaxThermType[] = {[Thermis]=MaxThermis, [Thermoc]=MaxThermoc, [ThermDS18B20]=MaxThermDS18B20};


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
#define TermisLow_DS18B20	-550
#define TermisUpp_Cu		1800
#define TermisUpp_Pt		6000
#define TermisUpp_DS18B20	1250
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
TerLim TerDS18B20Lim[] = {
	[ThermOn] = {TermisLow_DS18B20, TermisUpp_DS18B20},
};
static TerLim *const TerLimType[] PROGMEM = {[Thermis]=TermisLim, [Thermoc]=TermocLim, [ThermDS18B20]=TerDS18B20Lim};

//#define TerFiltLev 4
//static uint16_t TerBuf[TerReg][TerFiltLev*4];

#define ThermTbl(Name) static PGM_uDataPoint Name##Tbl[]
#define ThermCalc(Name) PGM_uBrokLine(Name##Tbl, Code)

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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~

enum {DS18B20_INITIAL, DS18B20_START_ADC1, DS18B20_START_ADC2,  DS18B20_START_ADC3, DS18B20_READ_T1, DS18B20_READ_T2, DS18B20_READ_T3, DS18B20_END_CYCLE}; 
uint8_t DS18B20_Mode = DS18B20_INITIAL;

enum {DS18B20_INIT, DS18B20_OK, DS18B20_ERROR}; 
uint8_t DS18B20_Status[TerReg];

uint8_t DS18B20_Error[TerReg];

uint8_t TD_DS18B20Read;

enum{OW_NO_CODE, OW_OK, OW_NO_PRESENCE_PULSE};

//~~~~~~~~~~~~~
void OW_MasterTx_Command(const uint8_t CH, const uint8_t Command){
	uint8_t i ;
	for(i=0; i<8; i++){
		cli();
		OW_MasterPullLowBus(CH);
		_delay_us(5);
		if(Command & (1<<i)){
			OW_MasterReleaseBus(CH);
			_delay_us(85);
		}
		else{
			Delay_us(85);
			OW_MasterReleaseBus(CH);
		}
		sei();
		_delay_us(5);	//recovery time minimum 1us
	}
}
//~~~~~~~~~~~~~
uint8_t OW_MasterRx_Data(const uint8_t CH){
	uint8_t i;
	uint8_t Data = 0;
	for(i=0; i<8; i++){
		cli();
		OW_MasterPullLowBus(CH);
		_delay_us(5);
		OW_MasterReleaseBus(CH);
		_delay_us(5);
		if(OW_MasterReadBus(CH)) Data |= (1<<i);
		sei();
		_delay_us(60);
	}
	return Data;
}


//~~~~~~~~~~~~~
void OneWire_Init(void){
	TD_DS18B20Read = Timer8SysAlloc(1);
	uint8_t i;
	for(i=0; i<TerReg; i++) DS18B20_Error[i] = 10;
//To initiate a temperature measurement and A-to-D conversion, the master must issue a Convert T [44h] command.
	#ifdef RH_CHANNNEL
		StartTimer8(TD_DS18B20Read, 200);
	#endif
}
// ~~~~~~~~~~~~~
void OneWireStartADC(const uint8_t CH){
	#ifdef RH_CHANNNEL
		if(CH == RH_CHANNNEL) return;
	#endif

	if(erb(&ThermType[CH]) == ThermOff){
		Temperature[CH] = Therm_Off_Mark;
		DS18B20_Status[CH] = DS18B20_ERROR;
		return;
	}

	//MASTER TX RESET PULSE 480мs minimum
	cli(); OW_MasterPullLowBus(CH); sei();
	Delay_us(580);	
	cli(); OW_MasterReleaseBus(CH); 
	
	//MASTER RX presence pulse480мs minimum
	_delay_us(70);	//SLAVE waits
	if(OW_MasterReadBus(CH)==0)	DS18B20_Status[CH] = DS18B20_OK;
	else{ 
		DS18B20_Status[CH] = DS18B20_ERROR;
		DS18B20_Error[CH]++;
	}
	sei();
	
	Delay_us(510);

	if(DS18B20_Status[CH] != DS18B20_ERROR){
		OW_MasterTx_Command(CH,0xCC);	// SKIP ROM Command
		OW_MasterTx_Command(CH,0x44);	// CONVERT T Command
	}
}
// ~~~~~~~~~~~~~
void OneWireReadTemp(const uint8_t CH){

	#ifdef RH_CHANNNEL
		if(CH == RH_CHANNNEL){
			Read_DHT22(CH);
			return;
		}
	#endif
	
	if(erb(&ThermType[CH]) == ThermOff){
		Temperature[CH] = Therm_Off_Mark;
		DS18B20_Status[CH] = DS18B20_INIT;
		DS18B20_Error[CH]=0;
		return;
	}
		
	if(DS18B20_Status[CH] == DS18B20_ERROR){
		if(DS18B20_Error[CH] > 9) Temperature[CH] = ADC_Off_Mark;
		else DS18B20_Error[CH]++;
		return;
	}
	else{
		DS18B20_Error[CH]=0;
	}

	//MASTER TX RESET PULSE 480мs minimum
	cli(); OW_MasterPullLowBus(CH); sei();
	Delay_us(580);
	cli(); OW_MasterReleaseBus(CH); 

	//MASTER RX presence pulse 480мs minimum
	Delay_us(70);	//SLAVE waits
	if(OW_MasterReadBus(CH)!=0) DS18B20_Status[CH] = DS18B20_ERROR;
	sei();
	Delay_us(510);

	if(DS18B20_Status[CH] != DS18B20_ERROR){
		OW_MasterTx_Command(CH,0xCC);	// SKIP ROM Command
		OW_MasterTx_Command(CH,0xBE);	// READ SCRATCHPAD
		int16_t Tres;
		Tres = (int16_t)OW_MasterRx_Data(CH);
		Tres |= ((int16_t)OW_MasterRx_Data(CH)<<8);
		Tres = (Tres*10)/16;
		if(Tres != 850)	//DS18B20 power up value = 85.0C
			Temperature[CH] = (Tres+(int8_t)erb(&ThermCorr[CH].D))*(erw(&ThermCorr[CH].A)/1000.0);

		//MASTER TX RESET PULSE 480мs minimum
		cli(); OW_MasterPullLowBus(CH); sei();
		Delay_us(580);	
		cli(); OW_MasterReleaseBus(CH); sei();

		//if( OW_Temperature[CH]>300 || OW_Temperature[CH]<150 ) DS18B20_Error[CH]++;
	}
}
// ~~~~~~~~~~~~~
#if TerReg == 1
	void DS18B20(void){
		if(Timer8Stopp(TD_DS18B20Read)){
			if(DS18B20_Mode >= DS18B20_END_CYCLE) DS18B20_Mode = DS18B20_START_ADC1;
		
			switch(DS18B20_Mode){
			case DS18B20_INITIAL:
				break;

			case DS18B20_START_ADC1:
				OneWireStartADC(0);	// 2.9мсек
				StartTimer8(TD_DS18B20Read, 100);
				DS18B20_Mode = DS18B20_START_ADC3;
				break;

			case DS18B20_READ_T1:
				OneWireReadTemp(0);	// 4.6мсек
				DS18B20_Mode = DS18B20_READ_T3;
				break;
			}
			DS18B20_Mode++;
		}
	}
#endif 

#if TerReg == 3
	void DS18B20(void){
		if(Timer8Stopp(TD_DS18B20Read)){
			if(DS18B20_Mode >= DS18B20_END_CYCLE) DS18B20_Mode = DS18B20_START_ADC1;
		
			switch(DS18B20_Mode){
			case DS18B20_INITIAL:
				break;

			case DS18B20_START_ADC1:
				OneWireStartADC(0);	// 2.9мсек
				break;
			case DS18B20_START_ADC2:
				OneWireStartADC(1);	// 2.9мсек
				break;
			case DS18B20_START_ADC3:
				OneWireStartADC(2);	// 2.9мсек
				StartTimer8(TD_DS18B20Read, 100);
				break;

			case DS18B20_READ_T1:
				OneWireReadTemp(0);	// 4.6мсек
				break;
			case DS18B20_READ_T2:
				OneWireReadTemp(1);	// 4.6мсек
				break;
			case DS18B20_READ_T3:
				OneWireReadTemp(2);	// 4.6мсек
				break;
			}
			DS18B20_Mode++;
		}
	}
#endif 
// ~~~~~~~~~~~~~




