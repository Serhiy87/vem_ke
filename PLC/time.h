/*
 * 06.10.2011 - Kononenko add volatile for Timerx[] and TimerxFlg[]
 * 07.2011 Сулима добавил функции для выделения автоматических таймеров блоками.
 * "uint8_t Timer8AppAlloc(uint8_t n)", "uint8_t Timer16AppAlloc(uint8_t n)", "uint8_t Timer32AppAlloc(uint8_t n)"
 * 14.11.2013 - Кононенко - Увеличил на +1байт размер TimerххFlg - Чтобы не было проблем с размером TimerххTotNumber не кратным 8
 * 24.12.2013 - Кононенко - Добавил HIGH_SPEED_COUNTER
 * 27.03.2015 - Кононенко - Добавил LiveTime
 * 15.08.2016 - Кононенко - Увеличил кол-во системных 8-и и 16-битных таймеров и проверка TimerAllocError(void), измеряем ScanPerSec
 * 01.09.2016 - Потапов, Кононенко - Уменьшение времени ISR. Добавил TimersInc(), дефайны Timer8_ISR_xxxx
 */
// ~~~~~~~~~~~~~~~~~
// WDT
// ~~~~~~~~~~~~~~~~~
#include <avr\wdt.h>

// ~~~~~~~~
void
WdtOn(void)
{
	wdt_reset();
	wdt_enable(WDTO_2S);
}

volatile uint32_t LiveTime;	
volatile uint16_t ScanCount, ScanPerSec;

// ~~~~~~~~~~~~~~~~~~~~~~
// Program timers (10 ms)
// ~~~~~~~~~~~~~~~~~~~~~~
#ifndef Timer8_ISR_ApplNumber
#define Timer8_ISR_ApplNumber 0
#endif

#ifndef Timer8_ISR_ApplManuNumber
#define Timer8_ISR_ApplManuNumber 0
#endif



// Divisible by 8!
#define Timer8_ISR_TotNumber		(Timer8_ISR_ApplNumber  + 010)
#define Timer8TotNumber		(Timer8ApplNumber  + 030)
#define Timer16TotNumber	(Timer16ApplNumber + 030)
#define Timer32TotNumber	(Timer32ApplNumber + 010)

volatile static uint8_t		Timer8_ISR[Timer8_ISR_TotNumber];
volatile static uint8_t		Timer8[Timer8TotNumber];
volatile static uint16_t	Timer16[Timer16TotNumber];
volatile static uint32_t	Timer32[Timer32TotNumber];

volatile static uint8_t Timer8_ISR_Flg[Timer8_ISR_TotNumber/8+1];
volatile static uint8_t Timer8Flg[Timer8TotNumber/8+1];
volatile static uint8_t Timer16Flg[Timer16TotNumber/8+1];
volatile static uint8_t Timer32Flg[Timer32TotNumber/8+1];

uint8_t VacantTimer8_ISR=Timer8_ISR_ApplManuNumber;
uint8_t VacantTimer8Sys_ISR=Timer8_ISR_ApplNumber;

uint8_t VacantTimer8=Timer8ApplManuNumber, VacantTimer16=Timer16ApplManuNumber, VacantTimer32=Timer32ApplManuNumber;
uint8_t VacantTimer8Sys=Timer8ApplNumber, VacantTimer16Sys=Timer16ApplNumber, VacantTimer32Sys=Timer32ApplNumber;

#define Timer8AllocErr()	(VacantTimer8>=Timer8ApplNumber)
#define Timer16AllocErr()	(VacantTimer16>=Timer16ApplNumber)
#define Timer32AllocErr()	(VacantTimer32>=Timer32ApplNumber)

// ~~~~~~~~~~~~~~
uint8_t
Timer8_ISR_Alloc(void)
{
	uint8_t N = VacantTimer8_ISR;
	VacantTimer8_ISR++;
	return N;
}

uint8_t
Timer8Alloc(void)
{
	uint8_t N = VacantTimer8;
	VacantTimer8++;
	return N;
}

// ~~~~~~~~~~~~~~~
uint8_t
Timer16Alloc(void)
{
	uint8_t N = VacantTimer16;
	VacantTimer16++;
	return N;
}

// ~~~~~~~~~~~~~~~
uint8_t
Timer32Alloc(void)
{
	uint8_t N = VacantTimer32;
	VacantTimer32++;
	return N;
}
// ~~~~~~~~~~~~~~

uint8_t
Timer8_ISR_AppAlloc(uint8_t n)
{
	if(n==0)return 0;
	uint8_t N = VacantTimer8_ISR;
	VacantTimer8_ISR+=n;
	return N;
}

uint8_t
Timer8AppAlloc(uint8_t n)
{
	if(n==0)return 0;
	uint8_t N = VacantTimer8;
	VacantTimer8+=n;
	return N;
}

// ~~~~~~~~~~~~~~~
uint8_t
Timer16AppAlloc(uint8_t n)
{
	if(n==0)return 0;
	uint8_t N = VacantTimer16;
	VacantTimer16+=n;
	return N;
}

// ~~~~~~~~~~~~~~~
uint8_t
Timer32AppAlloc(uint8_t n)
{
	if(n==0)return 0;
	uint8_t N = VacantTimer32;
	VacantTimer32+=n;
	return N;
}

// ~~~~~~~~~~~~~~~~~~~~~~
uint8_t
Timer8_ISR_SysAlloc(uint8_t n)
{
	uint8_t N = VacantTimer8Sys_ISR;
	VacantTimer8Sys_ISR += n;
	return N;
}

uint8_t
Timer8SysAlloc(uint8_t n)
{
	uint8_t N = VacantTimer8Sys;
	VacantTimer8Sys += n;
	return N;
}

// ~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
Timer16SysAlloc(uint8_t n)
{
	uint8_t N = VacantTimer16Sys;
	VacantTimer16Sys += n;
	return N;
}

// ~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
Timer32SysAlloc(uint8_t n)
{
	uint8_t N = VacantTimer32Sys;
	VacantTimer32Sys += n;
	return N;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
StartTimer8_ISR(uint8_t TimerN, uint8_t Timeout)
{
	IntOff();
	if ((Timer8_ISR[TimerN]=Timeout)>0)
		Timer8_ISR_Flg[TimerN/8] &=~(1<<TimerN%8);
	else
		Timer8_ISR_Flg[TimerN/8] |=(1<<TimerN%8);
	IntOn();
}


void
StartTimer8(uint8_t TimerN, uint8_t Timeout)
{
	IntOff();
	if ((Timer8[TimerN]=Timeout)>0)
		Timer8Flg[TimerN/8] &=~(1<<TimerN%8);
	else
		Timer8Flg[TimerN/8] |=(1<<TimerN%8);
	IntOn();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
StartTimer16(uint8_t TimerN, uint16_t Timeout)
{
	IntOff();
	if ((Timer16[TimerN]=Timeout)>0)
		Timer16Flg[TimerN/8] &=~(1<<TimerN%8);
	else
		Timer16Flg[TimerN/8] |=(1<<TimerN%8);
	IntOn();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
StartTimer32(uint8_t TimerN, uint32_t Timeout)
{
	IntOff();
	if ((Timer32[TimerN]=Timeout)>0)
		Timer32Flg[TimerN/8] &=~(1<<TimerN%8);
	else
		Timer32Flg[TimerN/8] |=(1<<TimerN%8);
	IntOn();
}

// ~~~~~~~~~~~~~~~~~~~~~~~
void
StopTimer8_ISR(uint8_t TimerN)
{
	IntOff();
	Timer8_ISR_Flg[TimerN/8] |=(1<<TimerN%8);
	IntOn();
}

void
StopTimer8(uint8_t TimerN)
{
	IntOff();
	Timer8Flg[TimerN/8] |=(1<<TimerN%8);
	IntOn();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~
void
StopTimer16(uint8_t TimerN)
{
	IntOff();
	Timer16Flg[TimerN/8] |=1<<TimerN%8;
	IntOn();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~
void
StopTimer32(uint8_t TimerN)
{
	IntOff();
	Timer32Flg[TimerN/8] |=1<<TimerN%8;
	IntOn();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
Timer8Stopp_ISR(uint8_t TimerN)
{
	return (Timer8_ISR_Flg[TimerN/8] &(1<<TimerN%8)) && Timer8_ISR[TimerN]==0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
Timer8Stopp(uint8_t TimerN)
{
	return (Timer8Flg[TimerN/8] &(1<<TimerN%8)) && Timer8[TimerN]==0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
Timer16Stopp(uint8_t TimerN)
{
	IntOff();
	uint8_t T_S = (Timer16Flg[TimerN/8] &(1<<TimerN%8)) && Timer16[TimerN]==0;
	IntOn();
	return T_S;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
Timer32Stopp(uint8_t TimerN)
{
	IntOff();
	uint8_t T_S = (Timer32Flg[TimerN/8] &(1<<TimerN%8)) && Timer32[TimerN]==0;
	IntOn();
	return T_S;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~
void
ResetTimer8_ISR(uint8_t TimerN)
{
	IntOff();
	Timer8_ISR_Flg[TimerN/8] |=(1<<TimerN%8);
	IntOn();
	Timer8_ISR[TimerN] = 0;
}

void
ResetTimer8(uint8_t TimerN)
{
	IntOff();
	Timer8Flg[TimerN/8] |=(1<<TimerN%8);
	IntOn();
	Timer8[TimerN] = 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~
void
ResetTimer16(uint8_t TimerN)
{
	IntOff();
	Timer16Flg[TimerN/8] |=1<<TimerN%8;
	Timer16[TimerN] = 0;
	IntOn();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~
void
ResetTimer32(uint8_t TimerN)
{
	IntOff();
	Timer32Flg[TimerN/8] |=1<<TimerN%8;
	Timer32[TimerN] = 0;
	IntOn();
}

// ~~~~~~~~~~~~~~~~~~~~~~
uint8_t
GetTimer8_ISR(uint8_t TimerN)
{
	return Timer8_ISR[TimerN];
}
uint8_t
GetTimer8(uint8_t TimerN)
{
	return Timer8[TimerN];
}

// ~~~~~~~~~~~~~~~~~~~~~~~
uint16_t
GetTimer16(uint8_t TimerN)
{
	IntOff();
	uint16_t Time = Timer16[TimerN];
	IntOn();
	return Time;
}

// ~~~~~~~~~~~~~~~~~~~~~~~
uint32_t
GetTimer32(uint8_t TimerN)
{
	IntOff();
	uint32_t Time = Timer32[TimerN];
	IntOn();
	return Time;
}
// ~~~~~~~~~~~~~~~~~~~~~~~
uint8_t TimerAllocError(void){
	return ( (VacantTimer8Sys >=  Timer8TotNumber) || (VacantTimer16Sys >=  Timer16TotNumber) || (VacantTimer32Sys >=  Timer32TotNumber) ||
		 Timer8AllocErr() || Timer16AllocErr() || Timer32AllocErr() );
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 1 s
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t Timer1s, Timer1min;
uint16_t Timer1hour;
static uint8_t Timer1min_EE EEMEM;
static uint16_t Timer1hour_EE EEMEM;

// ~~~~~~~~~~~~~
void
OnTimeSave(void)
{
	ewb(&Timer1min_EE, Timer1min);
	eww(&Timer1hour_EE, Timer1hour);
}

// ~~~~~~~~~~~~~~~
inline static void
RT(void)
{
	if (++Timer1s==60) {
		Timer1s = 0;
		if (++Timer1min==60) {
			Timer1min = 0;
			++Timer1hour;
			OnTimeSave();
		}
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined (__AVR_ATxmega128A1__)
	ISR(TCD0_OVF_vect)
	{
		RT();
	}
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Total
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static uint8_t TD_Scan;
uint8_t ScanCycle, MaxScanCycle;

#ifdef HIGH_SPEED_COUNTER
	extern inline void HSC_Process(void);
#endif

// ~~~~~~~~~~~
void
TimeInit(void)
{
	#if defined (__AVR_ATxmega128A1__)
		OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc |OSC_XOSCSEL_XTAL_16KCLK_gc;
		OSC.CTRL = OSC_XOSCEN_bm &~OSC_RC32KEN_bm &~OSC_RC32MEN_bm &~OSC_RC2MEN_bm;
		OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc |2;
		while (!(OSC.STATUS &(OSC_XOSCRDY_bm)));
		OSC.CTRL |=OSC_PLLEN_bm;
		while (!(OSC.STATUS &(OSC_PLLRDY_bm)));
		CCP = CCP_IOREG_gc; CLK.CTRL = CLK_SCLKSEL_PLL_gc;
		CCP = CCP_IOREG_gc; CLK.LOCK = CLK_LOCK_bm;
	#endif

	// WDT
	WdtOn();

	#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		TCCR1A = 0;
		TCCR1B = (((~(1<<WGM13) |(1<<WGM12)) &~(1<<CS12)) |(1<<CS11)) &~(1<<CS10);
		OCR1A = 20000;
	#elif defined (__AVR_ATxmega128A1__)
		TCC0.PER = 40000;
		TCC0.CTRLA = TC_CLKSEL_DIV8_gc;
		TCC0.CTRLB = TC_WGMODE_NORMAL_gc &~TC0_CCBEN_bm &~TC0_CCAEN_bm;
		TCC0.CTRLD = TC_EVACT_OFF_gc;
		TCC0.CTRLE = ~TC0_BYTEM_bm;
	#endif
	#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__)
		TIMSK |=(1<<OCIE1A);
	#elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		TIMSK1 |=(1<<OCIE1A);
	#elif defined (__AVR_ATxmega128A1__)
		TCC0.INTCTRLA = TC_ERRINTLVL_OFF_gc |TC_OVFINTLVL_MED_gc;
		TCC0.INTCTRLB = 0;
	#endif

	// Program timers (10 ms)
	uint8_t i;
	for (i=0; i<sizeof(Timer8Flg); i++)
		Timer8Flg[i] = 0xFF;
	for (i=0; i<sizeof(Timer16Flg); i++)
		Timer16Flg[i] = 0xFF;
	for (i=0; i<sizeof(Timer32Flg); i++)
		Timer32Flg[i] = 0xFF;

	// 1 s
	Timer1min = erb(&Timer1min_EE);
	Timer1hour = erw(&Timer1hour_EE);
	#if defined (__AVR_ATxmega128A1__)
		TCD0.PER = 32000;
		TCD0.CTRLA = TC_CLKSEL_DIV1024_gc;
		TCD0.CTRLB = TC_WGMODE_NORMAL_gc &~TC0_CCBEN_bm &~TC0_CCAEN_bm;
		TCD0.CTRLD = TC_EVACT_OFF_gc;
		TCD0.CTRLE = ~TC0_BYTEM_bm;
		TCD0.INTCTRLA = TC_ERRINTLVL_OFF_gc |TC_OVFINTLVL_MED_gc;
		TCD0.INTCTRLB = 0;
	#endif

	// Application
	#ifdef InitTimer
		StartTimer16(InitTimer, InitTime);
	#endif
}

// ~~~~~~~~~~~~~~~~
void
ScanCycleInit(void)
{
	TD_Scan = Timer8SysAlloc(1);
	StartTimer8(TD_Scan, 0xFF);
}

// ~~~~~~~~~~~~~~~~~~~~~
static volatile uint8_t TickCounter;

#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
	ISR(TIMER1_COMPA_vect)
#elif defined (__AVR_ATxmega128A1__)
	ISR(TCC0_OVF_vect)
#endif
{
	TickCounter++;
	uint8_t i;
	for (i=0; i<Timer8_ISR_TotNumber; i++)
			if (!(Timer8_ISR_Flg[i/8] &(1<<i%8)) && --Timer8_ISR[i]==0)
				Timer8_ISR_Flg[i/8] |=(1<<i%8);

	#ifdef HIGH_SPEED_COUNTER
		HSC_Process();
	#endif
}
void 
TimersInc(void)
{
	uint8_t i, TickCounterVar;	
	cli();
	TickCounterVar = TickCounter;
	TickCounter = 0;
	sei();
	for(uint8_t j=0;j<TickCounterVar;j++){
		LiveTime++;
	 	for (i=0; i<Timer8TotNumber; i++){
			cli();
			if (!(Timer8Flg[i/8] &(1<<i%8)) && --Timer8[i]==0)
				Timer8Flg[i/8] |=(1<<i%8);
			sei();
		}
		for (i=0; i<Timer16TotNumber; i++){
			cli();
			if (!(Timer16Flg[i/8] &(1<<i%8)) && --Timer16[i]==0)
				Timer16Flg[i/8] |=(1<<i%8);
			sei();
		}
		for (i=0; i<Timer32TotNumber; i++){
			cli();
			if (!(Timer32Flg[i/8] &(1<<i%8)) && --Timer32[i]==0)
				Timer32Flg[i/8] |=(1<<i%8);	
			sei();
		}

		#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		static uint8_t Timer10ms;
		if (++Timer10ms==100) {
			Timer10ms = 0;
			ScanPerSec = ScanCount;
 			ScanCount = 0;
			RT();
		}
		#endif
	}
}

// ~~~~~~~~~~~~
void
TimeCycle(void)
{
	// WDT
	wdt_reset();

	// Scan cycle
	ScanCount++;
	if ((ScanCycle=0xFF-Timer8[TD_Scan])>MaxScanCycle)
		MaxScanCycle = ScanCycle;
	StartTimer8(TD_Scan, 0xFF);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~
void
MaxScanCycle_Clear(void)
{
	MaxScanCycle = 0;
}

// ~~~~~~~~~~~~~~~~~~~~~
// Delay functions
// ~~~~~~~~~~~~~~~~~~~~~
#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
	#define	F_CPU 16000000UL
#elif defined (__AVR_ATxmega128A1__)
	#define	F_CPU 32000000UL
#endif

#include <util\delay.h>

// ~~~~~~~~~~~~~~~~~~~~
void
Delay_ms(uint8_t Delay)
{
	while (Delay--)
		_delay_ms(1);
}

// ~~~~~~~~~~~~~~~~~~~~~
void
Delay_us(uint16_t Delay)
{
	/*uint8_t i;
	for(i=Delay/Delay_us_Max; i; i--)
		_delay_us(Delay_us_Max);*/
	//_delay_us(Delay%Delay_us_Max);
	Delay *= .8;	// By trial
	while (Delay--)
		_delay_us(1);
}

