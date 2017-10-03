/*~~~~~~~~~~~~~~~~~~~
Подпрограмма TWI_In запускает цикл чтения по I2C шине значений
реальных даты/времени из регистров микросхемы PCF8583.
Цикл чтения длится xxx мс(PCF8583) каждые 500 мс.

// 04.2011 - Kononenko - Revised for Xmega128A1
	27.03.2014 - Kononenko - Add #define RTC_GetTrueTime(), RTC_Ready() for compability with m14t56

	04.09.2014 - Kononenko - RTC_Summer_Time - автоматический переход на летнее время и обратно.
		Переход выполняется только при включённом блоке в час перехода. Если блок был выключен в данный час - переход не произойдёт

//TODO: It is recommended to set the stop counting flag of the
control/status register before loading the actual time into
the counters. Loading of illegal states may lead to a
temporary clock malfunction.

//TODO: Write current time to PCF8583 RAM - for alarm "Сбой питания"

~~~~~~~~~~~~~*/

#include "digiproc.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Real-time clock/calendar PCF8583 & Temperature sensor LM75
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t Tm_Sec, Tm_Min, Tm_Hour, Tm_MDay, Tm_Mon, Tm_WDay;
uint8_t EEMEM Tm_Year = 12;	// Last leap year

uint8_t EEMEM SummerTime = 1;
uint8_t NotSummerTimeFlag;

#define PCF_SLA	0b1010000
#define LM_SLA	0b1001001

uint8_t TWI_Phase;

#define AddrSec 2

prog_uint8_t DaysMonth[] = {[1]=31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


#define RTC_GetTrueTime() 
#define RTC_Ready() 1

void RTC_Summer_Time(void);

// ~~~~~~~~~~~~~~~
// 31.12.00 was Su
static void
SetWeekday(void)
{
	uint8_t DayCount;
	uint8_t LastYear = erb(&Tm_Year)-1;

	DayCount = (LastYear+LastYear/4);
	for (uint8_t i=1; i<Tm_Mon; DayCount+=prb(DaysMonth+i++)%7);
	if (LastYear%4==3 && Tm_Mon>2)
		DayCount++;
	DayCount += Tm_MDay%7;
	Tm_WDay = DayCount%7;
}

// ~~~~~~~~~~~~~~~~~~
inline static void
SetYear(uint8_t Year)
{
	uint8_t EE_Year = erb(&Tm_Year);

	if (EE_Year%4!=Year)
		ewb(&Tm_Year, Year==0?EE_Year+1:EE_Year/4*4+Year);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define StartTWI()	TWCR = (1<<TWINT) |(1<<TWSTA) |(1<<TWEN)
#define StopTWI()		TWCR = (1<<TWINT) |(1<<TWSTO) |(1<<TWEN)
#define RunTWI()		TWCR = (1<<TWINT) |(1<<TWEN)
#define AcknTWI()		TWCR = (1<<TWINT) |(1<<TWEA) |(1<<TWEN)

#define WaitTWI() while(!(TWCR &(1<<TWINT)))

#define SLA_R(Addr) TWDR=(Addr<<1)+1
#define SLA_W(Addr) TWDR=(Addr<<1)+0

enum {
	PCF_SLA_W, PCF_Addr, StartTime, PCF_SLA_R, AcknPFC, PCF_Se, PCF_Mi, PCF_Ho, PCF_DY, PCF_Mo,
	LM_SLA_R, AcknLM, LM_Ter1, LM_Ter2
};

#if LM_Exist
	int16_t InnerTer;
	static uint8_t TD_TWI;
#endif

// ~~~~~~~~~~~
void
TWI_Init(void)
{
	#if LM_Exist
		TD_TWI = Timer8SysAlloc(1);
	#endif
	TWBR = 144;
	StartTWI();
}

// ~~~~~~~~~
void
TWI_In(void)
{
	#if LM_Exist
		static int8_t InnerTerBuf;
	#endif
	if (!(TWCR &(1<<TWINT)))
		return;
	switch(TWI_Phase) {
	case PCF_SLA_W:
		SLA_W(PCF_SLA);
		RunTWI();
		break;
	case PCF_Addr:
		TWDR = AddrSec;
		RunTWI();
		break;
	case StartTime:
		StartTWI();
		break;
	case PCF_SLA_R:
		SLA_R(PCF_SLA);
		RunTWI();
		break;
	case AcknPFC:
#if LM_Exist
	case AcknLM:
#endif
		AcknTWI();
		break;
	case PCF_Se:
		Tm_Sec = DecodBCD(TWDR);
		AcknTWI();
		break;
	case PCF_Mi:
		Tm_Min = DecodBCD(TWDR);
		AcknTWI();
		break;
	case PCF_Ho:
		Tm_Hour = DecodBCD(TWDR);
		AcknTWI();
		break;
	case PCF_DY:
		Tm_MDay = DecodBCD(TWDR&0x3F);
		SetYear(TWDR>>6);
		RunTWI();
		break;
	case PCF_Mo:
		if ((Tm_Mon=DecodBCD(TWDR&0x1F))>12)
			Tm_Mon = 0;
		SetWeekday();
	
		RTC_Summer_Time();	

		StartTWI();
#if LM_Exist
		if (!Timer8Stopp(TD_TWI))
			TWI_Phase = 0xFF;
		break;
	case LM_SLA_R:
		SLA_R(LM_SLA);
		RunTWI();
		break;
	case LM_Ter1:
		InnerTerBuf = TWDR;
		RunTWI();
		break;
	case LM_Ter2:
		InnerTer = InnerTerBuf*10;
		if (TWDR)
			InnerTer += 5;
		StartTimer8(TD_TWI, 40);	// LM75 300 ms needs
		StartTWI();
#endif
		TWI_Phase = 0xFF;
		break;
	}
	TWI_Phase++;
}

// ~~~~~~~~
static void
Tick(void)
{
	RunTWI();
	WaitTWI();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
WritePCF(uint8_t Addr, uint8_t Dat)
{
	WaitTWI();
	if(TWI_Phase!=PCF_SLA_W && TWI_Phase!=PCF_SLA_R
	#if LM_Exist
		&& TWI_Phase!=LM_SLA_R
	#endif
	 ) {
		StartTWI();
		WaitTWI();
	}
	TWDR = SLA_W(PCF_SLA);
	Tick();
	TWDR = AddrSec+Addr;
	Tick();
	TWDR = Addr==3 ? Dat : CodBCD(Dat);
	Tick();
	TWI_Phase = 0;
	StartTWI();
}




























// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif defined (__AVR_ATxmega128A1__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
enum {PCF_SLA_W, PCF_Addr, PCF_SLA_R, PCF_Se, PCF_Mi, PCF_Ho, PCF_DY, PCF_Mo};

#define WaitTWI() while (!( TWID.MASTER.STATUS & ((1<<TWI_MASTER_WIF_bp)|(1<<TWI_MASTER_RIF_bp)) ) )

#define AcknTWI()		TWID.MASTER.CTRLC = (0<<TWI_MASTER_ACKACT_bp) | (1<<TWI_MASTER_CMD1_bp) | (0<<TWI_MASTER_CMD0_bp);	
#define NAcknTWISTOP()	TWID.MASTER.CTRLC = (1<<TWI_MASTER_ACKACT_bp) | (1<<TWI_MASTER_CMD1_bp) | (1<<TWI_MASTER_CMD0_bp);	

#define SLA_R(Addr) TWID.MASTER.ADDR=(Addr<<1)+1
#define SLA_W(Addr) TWID.MASTER.ADDR=(Addr<<1)+0

static uint8_t TD_TWI;

// ~~~~~~~~~~~
void
TWI_Init(void)
{
	TWID.MASTER.BAUD = 255;	// ~61 кГц SCL
	TWID.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
	TWID.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;

	TD_TWI = Timer8SysAlloc(1);
}

// ~~~~~~~~~
void
TWI_In(void)
{
	if(Timer8Stopp(TD_TWI)){
		
		if (TWI_Phase == PCF_SLA_W){
			SLA_W(PCF_SLA);		// Send Start &  Slave Address
			TWI_Phase++;
			return;
		}
		if( TWID.MASTER.STATUS & ((1<<TWI_MASTER_WIF_bp)|(1<<TWI_MASTER_RIF_bp)) ){
			uint8_t temp;
			switch (TWI_Phase) {
			case PCF_Addr:
				TWID.MASTER.DATA = AddrSec;		// Write data to slave 
				break;
			case PCF_SLA_R:
				SLA_R(PCF_SLA);		// Send Repeated Start &  Slave Address
				break;
			case PCF_Se:
				TWID.MASTER.CTRLC = 0;	// Acknowledge Action: Send ACK
				Tm_Sec = DecodBCD(TWID.MASTER.DATA);
				AcknTWI();
				break;
			case PCF_Mi:
				Tm_Min = DecodBCD(TWID.MASTER.DATA);
				AcknTWI();
				break;
			case PCF_Ho:
				Tm_Hour = DecodBCD(TWID.MASTER.DATA);
				AcknTWI();
				break;
			case PCF_DY:
				temp = TWID.MASTER.DATA;
				AcknTWI();
				Tm_MDay = DecodBCD(temp&0x3F);
				SetYear(temp>>6);
				break;
			case PCF_Mo:
				TWI_Phase = 0xFF;
				TWID.MASTER.CTRLC = (1<<TWI_MASTER_ACKACT_bp);	// Acknowledge Action: Send NACK
				temp = TWID.MASTER.DATA;
				NAcknTWISTOP();
				if ((Tm_Mon=DecodBCD(temp&0x1F))>12)
					Tm_Mon = 0;
				SetWeekday();

				RTC_Summer_Time();

				StartTimer8(TD_TWI,50);
				break;
			}//switch
			TWI_Phase++;
		}

	}//if(Timer8Stopp(TD_TWI))
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
WritePCF(uint8_t Addr, uint8_t Dat)
{
	if( (TWID.MASTER.STATUS & TWI_MASTER_BUSSTATE_gm) != TWI_MASTER_BUSSTATE_IDLE_gc){
		NAcknTWISTOP();
		_delay_us(100);
	}
	SLA_W(PCF_SLA);		// Send Start &  Slave Address
	WaitTWI();
	TWID.MASTER.DATA = AddrSec+Addr;
	WaitTWI();
	TWID.MASTER.DATA = Addr==3 ? Dat : CodBCD(Dat);
	NAcknTWISTOP();	
	TWI_Phase = 0;
}

// ~~~
#endif
// ~~~

#define PCF_YearDay_Convert ((erb(&Tm_Year)%4<<6)|CodBCD(Tm_MDay))

// ~~~~~~~~~~~~~~~~~~~~~
static uint8_t
GetLastMDay(uint8_t Mon)
{
	uint8_t LastDay = prb(DaysMonth+Mon);
	if(Tm_Mon==2 && !(erb(&Tm_Year)%4))
		LastDay++;
	return LastDay;
}

// ~~~~~~~~~
void
SetSec(void)
{
	WritePCF(0, Tm_Sec);
}

// ~~~~~~~~~
void
SetMin(void)
{
	WritePCF(1, Tm_Min);
}

// ~~~~~~~~~~
void
SetHour(void)
{
	WritePCF(2,  Tm_Hour);
}

// ~~~~~~~~~~~~~
void
SetYearDay(void)
{
	uint8_t LD = GetLastMDay(Tm_Mon);
	if(Tm_MDay>LD)
		Tm_MDay = LD;
	WritePCF(3, PCF_YearDay_Convert);
	SetWeekday();
}

// ~~~~~~~~~
void
SetMon(void)
{
	SetYearDay();
	WritePCF(4, Tm_Mon);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void RTC_Summer_Time(void){
	if(erb(&SummerTime)){
		// Весной вперёд (ВВ) в 02:00:00 переход на 03:00:00 в последнее Вс марта
		if( (Tm_Mon == 3) && (Tm_WDay == 0) && (Tm_MDay>=25) && (Tm_Hour == 2) ){
				Tm_Hour=3;
				SetHour();
		}

		// Осенью обратно (ОО) в 04:00:00 переход на 03:00:00 в последнее Вс октября
		if( (Tm_Mon == 10) && (Tm_WDay == 0) && (Tm_MDay>=25) ){
				if(Tm_Hour == 4){
					if(!NotSummerTimeFlag){
						Tm_Hour=3;
						SetHour();
						NotSummerTimeFlag = 1;
					}
				}
				if(Tm_Hour < 2) NotSummerTimeFlag = 0;
				if(Tm_Hour >= 5) NotSummerTimeFlag = 0;
		}
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
void
SetTime(uint8_t Year, uint8_t Mon, uint8_t Day, uint8_t Hour, uint8_t Min, uint8_t Sec)
{
	int8_t CorrectHour = (int8_t)erb(&TimeZone_Hour)+erb(&SummerTime);
	uint8_t CorrectMin = erb(&TimeZone_Min);
	if(CorrectHour>=0) {
		if((Min+=CorrectMin)>=60) {
			Min -= 60;
			CorrectHour++;
		}
		if((Hour+=CorrectHour)>=24) {
			Hour -= 24;
			uint8_t LD = GetLastMDay(Mon);
			if(++Day>LD) {
				Day -= LD;
				if(++Mon>12) {
					Mon = 1;
					Year++;
				}
			}
		}
	}
	else if(CorrectHour<0) {
		if(CorrectMin<Min)
			Min -= CorrectMin;
		else {
			Min = 60+Min-CorrectMin;
			CorrectHour--;
		}
		if((CorrectHour*=-1)<=Hour)
			Hour -= CorrectHour;
		else {
			Hour = 24+Hour-CorrectHour;
			if(Day>1)
				Day--;
			else {
				if(Mon>1)
					Day = GetLastMDay(--Mon);
				else
					Mon = 12;
					Day = 31;
					Year--;
			}
		}
	}
	if(Tm_Sec!=Sec) {
		Tm_Sec = Sec;
		WritePCF(0, Tm_Sec);
	}
	if(Tm_Min!=Min) {
		Tm_Min = Min;
		WritePCF(1, Tm_Min);
	}
	if(Tm_Hour!=Hour) {
		Tm_Hour = Hour;
		WritePCF(2, Tm_Hour);
	}
	if(Tm_MDay!=Day || Tm_Year!=Year) {
		Tm_MDay = Day;
		ewb(&Tm_Year, Year);
		WritePCF(3, PCF_YearDay_Convert);
		SetWeekday();
	}
	if(Tm_Mon!=Mon) {
		Tm_Mon = Mon;
		WritePCF(4, Tm_Mon);
		SetWeekday();
	}
}
*/
