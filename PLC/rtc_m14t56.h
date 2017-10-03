
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

30.09.2013 * Kononenko *  M41T56 Serial real-time clock (RTC) with 56 bytes NVRAM

Подпрограмма TWI_In запускает цикл чтения по TWI(I2C) шине значений
реальных даты/времени из регистров микросхемы M41T56.
Цикл чтения длится ~2,7мс(зависит от scan cycle) каждые 490 мс.

Проверено: не зависает при записи WriteRTC()

При сильных помехах сбивается дата/время в M41T56 на случайное, часы останавливаются

26.12.2013 - Kononenko - Увеличил TD_TWI_Timeout с 100 мс до 400 мс
						При больших t scan cycle>20мс не считывает год,месяц -т.е не успевает закончить весь цикл опроса за TD_TWI_Timeout

06.02.2014 - Kononenko - Если часы нет тикают(установлен бит stop bit (ST)) - записываем секунды - часы начинают идти

07.02.2014 - Kononenko - При клацаньях реле на стенде фрикулинга дата и время сбиваются
			Причём есть закономерность наименее помехоустойчив год- затем месяц - затем день
			Дата сбивается на 05 05 2037
			Часы теперь никогда не останавливаются.
			Как с эти бороться не ясно.

16.02.2014 - Kononenko - RTC_Check -контроль показаний RTC

20.03.2014 - Kononenko - Контроль диапазона - Tm_Mon
		По непонятным причинам без батарейки происходит сбой показаний даты и времени.

04.09.2014 - Kononenko - RTC_Summer_Time - автоматический переход на летнее время и обратно.
		Переход выполняется только при включённом блоке в час перехода. Если блок был выключен в данный час - переход не произойдёт

27.11.2015 - Kononenko - Проблемы с точностью:
		На микросхемах с надписью DS1307Z - спешат на (9-11сек за 5мин!!!) т.е. на 1440мин в месяц 
		При чём в выключенном состоянии (от батарейки) идут точно. ?
		На микросхемах с надписью DS1307  - спешат на ~1,5мин в месяц 
		Определяющим является точность кварца (от 20ppm до 30ppm = от 1мин до 1,5мин в месяц) 
		Вывод: Т.е.с DS1307 все ок! Не использовать DS1307Z! 
		См.Application Note 58 Crystal Considerations with Dallas Real-Time Clocks
		Для DS1307 нужен кварц 32768Hz C=12.5pF, ESR =45kOhm, accuracy +-20ppm
18.01.2016 - Kononenko - Проблемы с точностью:
		В новой партии DS1307Z - точноcть Ок! Вывод: Каждую партию DS1307Z надо тестировать!


TODO: 1) Анализировать состояния (NACK и пр.)
		2) Почему то зависатет TWI при помехах.
		4) Корректировка хода
		5) Портировать для Xmega

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "digiproc.h"

#define M41T56

uint8_t Tm_Sec, Tm_Min, Tm_Hour, Tm_MDay, Tm_Mon, Tm_WDay, Tm_Year;
uint8_t TrueTime_Sec, TrueTime_Min, TrueTime_Hour, TrueTime_MDay, TrueTime_WDay, TrueTime_Mon, TrueTime_WDay, TrueTime_Year;

/*
#define TimeZone_Hour_Min -12
#define TimeZone_Hour_Max +13
uint8_t EEMEM TimeZone_Hour, TimeZone_Min;
*/
uint8_t EEMEM SummerTime = 1;
uint8_t NotSummerTimeFlag;

#define PCF_SLA	0b1101000

uint8_t TWI_Phase;

#define AddrSec 0

prog_uint8_t DaysMonth[] = {[1]=31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define StartTWI()		TWCR = (1<<TWINT) |(1<<TWSTA) |(1<<TWEN)
#define StopTWI()		TWCR = (1<<TWINT) |(1<<TWSTO) |(1<<TWEN)
#define RunTWI()		TWCR = (1<<TWINT) |(1<<TWEN)
#define AcknTWI()		TWCR = (1<<TWINT) |(1<<TWEA) |(1<<TWEN)
#define PowerOffTWI()	TWCR = (0<<TWEN)

// TWI STATUS
#define TWI_STATUS_START 		0x08
#define TWI_STATUS_RP_START 	0x10
#define TWI_STATUS_SLAW_ACK 	0x18
#define TWI_STATUS_SLAW_NACK 	0x20
#define TWI_STATUS_DATATX_ACK 	0x28
#define TWI_STATUS_DATATX_NACK 	0x30
#define TWI_STATUS_ARB_LOST 	0x38
#define TWI_STATUS_SLAR_ACK 	0x40
#define TWI_STATUS_SLAR_NACK 	0x48
#define TWI_STATUS_DATARX_ACK 	0x50
#define TWI_STATUS_DATARX_NACK 	0x58

#define WaitTWI() while(!(TWCR &(1<<TWINT)))

#define SLA_R(Addr) TWDR=(Addr<<1)+1
#define SLA_W(Addr) TWDR=(Addr<<1)+0

enum {
	PCF_START, PCF_CHECK_START, 
	PCF_SLA_W, PCF_CHECK_SLA_W,
	PCF_Addr, PCF_CHECK_Addr,
	PCF_REAP_START1, PCF_CHECK_REAP_START1,
	PCF_SLA_R, PCF_CHECK_SLA_R,
	PFC_ACKN, 
	PCF_Se, 
	PCF_Mi, 
	PCF_Ho, 
	PCF_WDay, 
	PCF_MDay, 
	PCF_Mo, 
	PCF_Year};


static uint8_t TD_TWI_Poll, TD_TWI_Timeout, TD_RTC_Check;

static uint8_t PFC_Stop_bit;

uint16_t TWI_Error_Counter;

void TWI_Init(void);
void TWI_Error(void);
void TWI_In(void);
static void WriteRTC(uint8_t Addr, uint8_t Dat);
uint8_t SetWeekday(uint8_t Year, uint8_t Month, uint8_t MDay);
void SetSec(void);
void SetMin(void);
void SetHour(void);
void SetMDay(void);
void SetMon(void);
void SetYear(void);
uint8_t RTC_Check(void);
void SetDateTime(uint8_t Year, uint8_t Month, uint8_t Day, uint8_t WDay, uint8_t Hour, uint8_t Min, uint8_t Sec);
void RTC_GetTrueTime(void);
void RTC_SetTrueTime(uint8_t Year, uint8_t Month, uint8_t Day, uint8_t WDay, uint8_t Hour, uint8_t Min, uint8_t Sec);
uint8_t RTC_Ready(void);
void RTC_Summer_Time(void);

// ~~~~~~~~~~~
void
TWI_Init(void)
{
	TD_TWI_Poll = Timer8SysAlloc(1);
	TD_TWI_Timeout = Timer8SysAlloc(1);
	TD_RTC_Check = Timer16SysAlloc(1);
	StartTimer16(TD_RTC_Check, 300);
	TWBR = 144;	//50кГц
}
// ~~~~~~~~~~~
void
TWI_Error(void)
{
	TWI_Error_Counter++;
	//StopTWI();
	PowerOffTWI();
	TWI_Phase = PCF_START;
	StartTimer8(TD_TWI_Poll, 49);
}

// ~~~~~~~~~
void
TWI_In(void)
{
	if(Timer8Stopp(TD_TWI_Poll)){

		if (TWI_Phase == PCF_START){
			StartTWI();		// Send Start
			TWI_Phase++;
			StartTimer8(TD_TWI_Timeout,40);
			return;
		}
		if(Timer8Stopp(TD_TWI_Timeout)){	//Если 
			TWI_Error();
			return;
		}

		if (!(TWCR &(1<<TWINT))) return;

		switch(TWI_Phase) {
		
		case PCF_CHECK_START:
			if( (TWSR & 0b11111000) != TWI_STATUS_START){ TWI_Error(); return;}
			else TWI_Phase++;
			
		case PCF_SLA_W:
			SLA_W(PCF_SLA);
			RunTWI();
			break;
		case PCF_CHECK_SLA_W:
			if( (TWSR & 0b11111000) != TWI_STATUS_SLAW_ACK){ TWI_Error(); return;}
			else TWI_Phase++;

		case PCF_Addr:
			TWDR = AddrSec;
			RunTWI();
			break;
		case PCF_CHECK_Addr:
			if( (TWSR & 0b11111000) != TWI_STATUS_DATATX_ACK){ TWI_Error(); return;}
			else TWI_Phase++;

		case PCF_REAP_START1:
			StartTWI();
			break;
		case PCF_CHECK_REAP_START1:
			if( (TWSR & 0b11111000) != TWI_STATUS_RP_START){ TWI_Error(); return;}
			else TWI_Phase++;

		case PCF_SLA_R:
			SLA_R(PCF_SLA);
			RunTWI();
			break;
		case PCF_CHECK_SLA_R:
			if( (TWSR & 0b11111000) != TWI_STATUS_SLAR_ACK){ TWI_Error(); return;}
			else TWI_Phase++;

		case PFC_ACKN:
			AcknTWI();
			break;

		case PCF_Se:
			if( (TWSR & 0b11111000) != TWI_STATUS_DATARX_ACK){ TWI_Error(); return;}
			Tm_Sec = DecodBCD(TWDR & 0b01111111);	// exclude ST bit
			PFC_Stop_bit = TWDR & 0b10000000;
/*			if(PFC_Stop_bit) NoTickCnt++;
			else NoTickCnt=0;*/
			AcknTWI();
			break;
		case PCF_Mi:
			if( (TWSR & 0b11111000) != TWI_STATUS_DATARX_ACK){ TWI_Error(); return;}
			Tm_Min = DecodBCD(TWDR & 0b01111111);
			AcknTWI();
			break;
		case PCF_Ho:
			if( (TWSR & 0b11111000) != TWI_STATUS_DATARX_ACK){ TWI_Error(); return;}
			Tm_Hour = DecodBCD(TWDR & 0b00111111);
			AcknTWI();
			break;
		case PCF_WDay:
			if( (TWSR & 0b11111000) != TWI_STATUS_DATARX_ACK){ TWI_Error(); return;}
			TWDR;
			//Tm_WDay = DecodBCD(TWDR & 0b00000111); Ignore 
			AcknTWI();
			break;
		case PCF_MDay:
			if( (TWSR & 0b11111000) != TWI_STATUS_DATARX_ACK){ TWI_Error(); return;}
			Tm_MDay = DecodBCD(TWDR & 0b00111111);
			AcknTWI();
			break;
		case PCF_Mo:
			if( (TWSR & 0b11111000) != TWI_STATUS_DATARX_ACK){ TWI_Error(); return;}
			Tm_Mon = DecodBCD(TWDR & 0b00011111);
			if(Tm_Mon > 12) Tm_Mon = 0;	//
			RunTWI();		//NACK
			break;
		case PCF_Year:
			if( (TWSR & 0b11111000) != TWI_STATUS_DATARX_NACK){ TWI_Error(); return;}
			Tm_Year = DecodBCD(TWDR);
			StopTWI();
			Tm_WDay = SetWeekday(Tm_Year, Tm_Mon, Tm_MDay);
			StartTimer8(TD_TWI_Poll, 49);
			TWI_Phase = 0xFF;
			break;
		}
		TWI_Phase++;
	}//Timer8Stopp(TD_TWI)

	if( Timer16Stopp(TD_RTC_Check) && (TWI_Phase == PCF_START) ){
		StartTimer16(TD_RTC_Check, 300);
		if(RTC_Check() || PFC_Stop_bit){
			//завести RTC
			SetDateTime(TrueTime_Year, TrueTime_Mon, TrueTime_MDay, TrueTime_WDay, TrueTime_Hour, TrueTime_Min, TrueTime_Sec);
		}
		else RTC_GetTrueTime();

		RTC_Summer_Time();
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
WriteRTC(uint8_t Addr, uint8_t Dat)
{
	
/*	if (!(TWCR &(1<<TWINT))){ //Wait for the end of current TWI operation and isuue STOP
		_delay_us(100);
		StopTWI();
		_delay_us(50);
	}*/
	PowerOffTWI();
	_delay_us(100);
	//_delay_us(150);_delay_us(150);
	StartTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	TWDR = SLA_W(PCF_SLA);
	RunTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	TWDR = AddrSec+Addr;
	RunTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	TWDR = CodBCD(Dat);
	RunTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	StopTWI();
	TWI_Phase = PCF_START;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif defined (__AVR_ATxmega128A1__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
				StartTimer8(TD_TWI,50);
				break;
			}//switch
			TWI_Phase++;
		}

	}//if(Timer8Stopp(TD_TWI))
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
WriteRTC(uint8_t Addr, uint8_t Dat)
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

/*
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
*/

// ~~~~~~~~~~~~~~~
// 31.12.00 was Su
uint8_t SetWeekday(uint8_t Year, uint8_t Month, uint8_t MDay)
{
	uint8_t DayCount;
	uint8_t LastYear = Year-1;

	DayCount = (LastYear+LastYear/4);
	for (uint8_t i=1; i<Month; DayCount+=prb(DaysMonth+i++)%7);
	if (LastYear%4==3 && Month>2)
		DayCount++;
	DayCount += MDay%7;
	return DayCount%7;
}
// ~~~~~~~~~
void
SetSec(void)
{
	WriteRTC(0, Tm_Sec);
	TrueTime_Sec = Tm_Sec;
}

// ~~~~~~~~~
void
SetMin(void)
{
	WriteRTC(1, Tm_Min);
	TrueTime_Min = Tm_Min;
}

// ~~~~~~~~~~
void
SetHour(void)
{
	WriteRTC(2,  Tm_Hour);
	TrueTime_Hour = Tm_Hour;
}
// ~~~~~~~~~~
void
SetMDay(void)
{
	WriteRTC(4,  Tm_MDay);
	TrueTime_MDay = Tm_MDay;
}
// ~~~~~~~~~
void
SetMon(void)
{
	WriteRTC(5, Tm_Mon);
	TrueTime_Mon = Tm_Mon;
}
// ~~~~~~~~~~~~~
void
SetYear(void)
{
	WriteRTC(6, Tm_Year);
	TrueTime_Year = Tm_Year;
}
// ~~~~~~~~~~~~~
void
SetDateTime(uint8_t Year, uint8_t Month, uint8_t Day, uint8_t WDay, uint8_t Hour, uint8_t Min, uint8_t Sec){
	PowerOffTWI();
	_delay_us(100);
	//_delay_us(150);_delay_us(150);
	StartTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	TWDR = SLA_W(PCF_SLA);
	RunTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	TWDR = AddrSec;
	RunTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	TWDR = CodBCD(Sec);
	RunTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	TWDR = CodBCD(Min);
	RunTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	TWDR = CodBCD(Hour);
	RunTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	TWDR = WDay;
	RunTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	TWDR = CodBCD(Day);
	RunTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	TWDR = CodBCD(Month);
	RunTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	TWDR = CodBCD(Year);
	RunTWI();
	//WaitTWI();
	_delay_us(150);_delay_us(150);
	StopTWI();
	TWI_Phase = PCF_START;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t RTC_Ready(void){
	if(TWI_Phase == PCF_START) return 1;
	else return 0;
}
void RTC_GetTrueTime(void){
	TrueTime_Sec = Tm_Sec;
	TrueTime_Min = Tm_Min;
	TrueTime_Hour = Tm_Hour;
	TrueTime_MDay = Tm_MDay;
	TrueTime_Mon = Tm_Mon;
	TrueTime_Year = Tm_Year;
	TrueTime_WDay = SetWeekday(TrueTime_Year, TrueTime_Mon, TrueTime_MDay);
}
void RTC_SetTrueTime(uint8_t Year, uint8_t Month, uint8_t Day, uint8_t WDay, uint8_t Hour, uint8_t Min, uint8_t Sec){
	SetDateTime(Year, Month, Day, WDay, Hour, Min, Sec);
	TrueTime_Sec = Sec;
	TrueTime_Min = Min;
	TrueTime_Hour = Hour;
	TrueTime_MDay = Day;
	TrueTime_Mon = Month;
	TrueTime_Year = Year;
	TrueTime_WDay = WDay;
}

//запускать через 3 сек

uint8_t RTC_Check(void){

	if( (Tm_Year == TrueTime_Year)  && (Tm_Mon == TrueTime_Mon) && (Tm_MDay == TrueTime_MDay) &&
			(Tm_Hour == TrueTime_Hour) && (Tm_Min == TrueTime_Min) && (Tm_Sec == TrueTime_Sec) ) return 1;

	if(TrueTime_Sec>55){
		if(TrueTime_Min == 59){
			if(TrueTime_Hour == 23){
				if(TrueTime_MDay >= 28){
					if(TrueTime_Mon == 12){
						if( ((TrueTime_Year+1) == Tm_Year) || (TrueTime_Year==Tm_Year) || ((TrueTime_Year==99) && (Tm_Year==0)) ) return 0;
						else return 1;
					}
					else{
						if(Tm_Year!= TrueTime_Year) return 1;
					}
				}
				else{
					if( (Tm_Year!= TrueTime_Year)  || (Tm_Mon != TrueTime_Mon) ) return 1;
				}
			}
			else{
				if( (Tm_Year!= TrueTime_Year)  || (Tm_Mon != TrueTime_Mon) || (Tm_MDay != TrueTime_MDay) ) return 1;
			}
		}
		else{
			if( (Tm_Year!= TrueTime_Year)  || (Tm_Mon != TrueTime_Mon) || (Tm_MDay != TrueTime_MDay) ||
				(Tm_Hour != TrueTime_Hour) ) return 1;		
		}
	}
	else{
		if( (Tm_Year!= TrueTime_Year)  || (Tm_Mon != TrueTime_Mon) || (Tm_MDay != TrueTime_MDay) ||
			(Tm_Hour != TrueTime_Hour) || (Tm_Min != TrueTime_Min) ) return 1;
	}
	
	return 0;
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
