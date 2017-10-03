/*
	19.12.2013 - Kononenko -  Revised for vmd2_3. Add Port_INH2, DDR_INH2
	01.09.2016 - Potapov - change Timer8_xxxxx - > Timer8_ISR_xxxxx
*/
#define WaitSPI() while(!(SPSR &(1<<SPIF)))

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Analog inputs: ADC AD7706 & Mux
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
enum {RST, INIT1, CALI1,
#if ADC_Reg<=3 || ADC_Reg>8
	INIT2, CALI2, INIT3, CALI3,
#endif
	READ, ADC_OFF};

#ifndef ADC_Reg_Ext
	#define ADC_Reg_Ext 0
#endif
uint16_t ADC_Input[ADC_Reg+ADC_Reg_Ext];

// ~~~~~~~~~~~~~
#ifndef ADC_Miss
// ~~~~~~~~~~~~~

#ifndef ADC_Ref
	#define ADC_Ref 18000		// For TSM100 0 oC <-> 18073
#endif
#ifndef ADC_Ref_d
	#define ADC_Ref_d 400		// For TSM100 5 oC <-> 387
#endif

uint8_t ADC_Phase;

uint16_t	ADC_Fail, ADC_ReadFail, ADC_Break;
#define ADC_OK !(ADC_Fail||ADC_ReadFail||ADC_Break)

typedef struct {
	uint8_t Zero[3], Full[3];
} Scale;
Scale ADC_Scale1
#if ADC_Reg<=3 || ADC_Reg>8
	, ADC_Scale2, ADC_Scale3
#endif
;

#define ADC10ms 20
uint8_t ADC_Time;

uint8_t ADC_Chan;

// Communication Reg
#define ADC_CH0		(1<<0)
#define ADC_CH1		(1<<1)
#define ADC_STBY	(1<<2)
#define ADC_RW		(1<<3)
#define ADC_RS0		(1<<4)
#define ADC_RS1		(1<<5)
#define ADC_RS2		(1<<6)
#define ADC_DRDY	(1<<7)

// Setup Reg
#define ADC_FSYNC	(1<<0)
#define ADC_BUF		(1<<1)
#define ADC_BU		(1<<2)
#define ADC_G0		(1<<3)
#define ADC_G1		(1<<4)
#define ADC_G2		(1<<5)
#define ADC_MD0		(1<<6)
#define ADC_MD1		(1<<7)

// Clock Reg
#define ADC_FS0			(1<<0)
#define ADC_FS1			(1<<1)
#define ADC_CLK			(1<<2)
#define ADC_CLKDIV	(1<<3)
#define ADC_CLKDIS	(1<<4)
#define ADC_ZERO0		(1<<5)
#define ADC_ZERO1		(1<<6)
#define ADC_ZERO2		(1<<7)

#define CommReg (uint8_t)( ~ADC_DRDY &~ADC_RS2 &~ADC_RS1  &~ADC_RS0)  |ADC_RW //read
#define SetpReg (uint8_t)((~ADC_DRDY &~ADC_RS2 &~ADC_RS1)  |ADC_RS0) &~ADC_RW //write
#define ClckReg (uint8_t)((~ADC_DRDY &~ADC_RS2) |ADC_RS1) &~ADC_RS0  &~ADC_RW //write
#define DataReg (uint8_t)( ~ADC_DRDY &~ADC_RS2) |ADC_RS1   |ADC_RS0   |ADC_RW //read
#define ZeroReg (uint8_t)((~ADC_DRDY  |ADC_RS2  |ADC_RS1) &~ADC_RS0)  |ADC_RW //read
#define FullReg (uint8_t)  ~ADC_DRDY  |ADC_RS2  |ADC_RS1   |ADC_RS0   |ADC_RW //read

#define ADC_IN1(Reg)  (Reg) &~ADC_STBY &~ADC_CH1 &~ADC_CH0
#define ADC_IN2(Reg) ((Reg) &~ADC_STBY &~ADC_CH1) |ADC_CH0
#define ADC_IN3(Reg) ((Reg) &~ADC_STBY) |ADC_CH1  |ADC_CH0

#define NormMode	~ADC_MD1 &~ADC_MD0
#define CaliMode	~ADC_MD1  |ADC_MD0

#define SetpVal(Mode) (uint8_t)(((Mode) &~ADC_G2 &~ADC_G1 &~ADC_G0) |ADC_BU |ADC_BUF)		// Gain: 1; Unipolar; High source impedance

// Clock: 2,45 MHz. Output update: 50 Hz
#define ClckVal (uint8_t)((~ADC_ZERO2 &~ADC_ZERO1 &~ADC_ZERO0 &~ADC_CLKDIS &~ADC_CLKDIV) |ADC_CLK) &~ADC_FS1 &~ADC_FS0

#ifdef ADC_Mux
	static prog_uint8_t Mux[] = ADC_Mux;
#endif

//static uint8_t TD_ADC;
uint8_t TD_ADC;

// ~~~~~~~~~~~~~~~~~~~~~~~~
static void
ADC_IN(uint8_t Reg)
{
	#if ADC_Reg<=3 || ADC_Reg>8
		switch(ADC_Chan) {
		case ADC_Reg-1:
			SPDR = ADC_IN3(Reg);
			break;
		case ADC_Reg-2:
			SPDR = ADC_IN2(Reg);
			break;
		default:
			SPDR = ADC_IN1(Reg);
			break;
		}
	#else
		SPDR = ADC_IN1(Reg);
	#endif
}

// ~~~~~~~~~~~
static void
WaitADC1(void)
{
	SPDR = 0;
	WaitSPI();
}

// ~~~~~~~~~~~
static void
WaitADC2(void)
{
	WaitSPI();
	SPDR = 0;
	WaitSPI();
}

// ~~~~~~~~~~~~~~~
inline static void
ADC_Select(void)
{
	SPCR |=(1<<CPOL);
	#ifdef CS_ADC
		cli();
		PortCS_ADC &=~CS_ADC;
		sei();
	#endif
}

// ~~~~~~~~~~~~~~~
inline static void
ADC_Desel(void)
{
	#ifdef CS_ADC
		cli();
		PortCS_ADC |=CS_ADC;
		sei();
	#endif
}

// ~~~~~~~~~~~~~~~
inline static void
ADC_Rst(void)
{
	cli();
	PortRESET_PER &=~RESET_PER;
	sei();
	_delay_us(1);							// 100 ns needs
	cli();
	PortRESET_PER |=RESET_PER;
	sei();
	StartTimer8_ISR(TD_ADC, 4);		// 16 ms needs

	ADC_Phase++;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~
static void
Init(uint8_t CR, uint8_t SR)
{
	SPDR = CR;
	WaitSPI();
	SPDR=ClckVal;
	WaitSPI();
	SPDR = SR;
	WaitSPI();
	SPDR = SetpVal(CaliMode) &~ADC_FSYNC;
	WaitSPI();
	StartTimer8_ISR(TD_ADC, 36);
	ADC_Phase++;
}

// ~~~~~~~~~~~~~~~
inline static void
ADC_Init1(void)
{
	if(!Timer8Stopp_ISR(TD_ADC))
		return;
	uint8_t i = 0;
	while(i++<8) {	// 4 x 0xFF needs
		SPDR = 0xFF;
		WaitSPI();
	}
	Init(ADC_IN1(ClckReg), ADC_IN1(SetpReg));
}

// ~~~~~~~~~~~~~~~
static void
Calibr(uint8_t CR)
{
	static uint8_t Try;
	SPDR = CR;
	WaitADC2();
	if(!(SPDR &ADC_DRDY)) {
		Try = 0;
		ADC_Phase++;
	}
	else if(Timer8Stopp_ISR(TD_ADC)) {
		ADC_Fail++;
		ADC_Phase = ++Try<27 ? RST : ADC_OFF;
	}
}

// ~~~~~~~~~~~~~~~~~~
inline static uint8_t
ADC_Check(void)
{
	Scale *Scl;
	int8_t i;

	ADC_IN(SetpReg);
	WaitSPI();
	SPDR = SetpVal(NormMode) |ADC_FSYNC;
	WaitSPI();
	ADC_Time = ADC10ms-Timer8_ISR[TD_ADC];
	#if ADC_Reg<=3 || ADC_Reg>8
		switch(ADC_Chan) {
		case ADC_Reg-1:
			Scl=&ADC_Scale3;
			break;
		case ADC_Reg-2:
			Scl=&ADC_Scale2;
			break;
		default:
			Scl=&ADC_Scale1;
			break;
		}
	#else
		Scl=&ADC_Scale1;
	#endif
	ADC_IN(ZeroReg);
	WaitADC2();
	for(i=2; i>=0; i--) {
		Scl->Zero[i] = SPDR;
		WaitADC1();
	}
	WaitADC1();
	ADC_IN(FullReg);
	WaitADC2();
	for(i=2; i>=0; i--) {
		Scl->Full[i] = SPDR;
		WaitADC1();
	}
	WaitADC1();
	return Scl->Zero[2]<25 || Scl->Zero[2]>27 || Scl->Full[2]<200 || Scl->Full[2]>203;
}

// ~~~~~~~~~~~~~~~
inline static void
SetMux(void)
{
	if(++ADC_Chan == ADC_Reg)
		ADC_Chan = 0;
	#ifdef ADC_Mux
		#if ADC_Reg>8
			if(ADC_Chan>=sizeof(Mux))
				return;
		#endif
		uint8_t Addr = prb(Mux+ADC_Chan);
		cli();
		if (Addr/4){
			Port_INH|=INH;
			#ifdef Port_INH2
				Port_INH2 &=~INH2;
			#endif
		}
		else{
			Port_INH &=~INH;
			#ifdef Port_INH2
				Port_INH2|=INH2;
			#endif
		}
		if (Addr%4/2)	Port_TER_B	|=TER_B;	else Port_TER_B	&=~TER_B;
		if (Addr%2)		Port_TER_A	|=TER_A;	else Port_TER_A	&=~TER_A;
		sei();
		_delay_us(0x10);		// Just in case
	#endif
}

// ~~~~~~~~~
uint8_t
ADC_In(void)
{
	uint8_t Chan = ADC_Reg;
	ADC_Select();

	switch(ADC_Phase) {
	case RST:
		ADC_Rst();
		break;
	case INIT1:
		ADC_Init1();
		break;
	case CALI1:
		Calibr(ADC_IN1(CommReg));
#if ADC_Reg<=3 || ADC_Reg>8
		break;
	case INIT2:
		Init(ADC_IN2(ClckReg), ADC_IN2(SetpReg));
		break;
	case CALI2:
		Calibr(ADC_IN2(CommReg));
		break;
	case INIT3:
		Init(ADC_IN3(ClckReg), ADC_IN3(SetpReg));
		break;
	case CALI3:
		Calibr(ADC_IN3(CommReg));
#endif
		if (ADC_Phase==READ)
			StartTimer8_ISR(TD_ADC, ADC10ms);
		break;
	case READ:
		ADC_IN(CommReg);
		WaitADC2();
		if(!(SPDR &ADC_DRDY)) {
			uint16_t Buf;
			ADC_IN(DataReg);
			WaitADC2();
			Buf = SPDR<<8;
			WaitADC1();
			Buf += SPDR;
			#ifdef ADC_Ref_Chan
				if (ADC_Chan==sizeof(Mux)-1 && ADC_Input[sizeof(Mux)-1]!=0 && (Buf<ADC_Ref-ADC_Ref_d || Buf>ADC_Ref+ADC_Ref_d)) {
					ADC_Break++;
					ADC_Phase = RST;
					ADC_Chan--;
				}
				else
			#endif
			if(ADC_Check()) {
				ADC_Break++;
				ADC_Phase = RST;
			}
			else {
				ADC_Input[ADC_Chan] = Buf;
				Chan = ADC_Chan;
				SetMux();
				ADC_IN(SetpReg);
				WaitSPI();
				SPDR = SetpVal(NormMode) &~ADC_FSYNC;
				WaitSPI();
				StartTimer8_ISR(TD_ADC, ADC10ms);
			}
		}
		else if(Timer8Stopp_ISR(TD_ADC)) {
			ADC_ReadFail++;
			ADC_Phase = RST;
		}
		break;
	}

	ADC_Desel();
	
	#ifdef ADC_Input_Ext
		for(uint8_t i=0; i<ADC_Reg_Ext; i++){
			IntOff();
			ADC_Input[ADC_Reg+i] = (ADC_Reg_Ext)[i];
			IntOn();
		}
	#endif

	return Chan;
}

// ~~~~~~~~~~~~~~~~~~~~~
#endif	// ndef ADC_Miss
// ~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// Analog outputs: DAC AD5322
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef DAC_Reg
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef DAC_Reg_Ext
	#define DAC_Reg_Ext 0
#endif
uint16_t DAC_Output[DAC_Reg+DAC_Reg_Ext];
#define MaxAO 1023

#define Delay_Sync 1	// High _Sync 100 ns min

// ~~~~~~~~~~
void
DAC_Out(void)
{
	uint16_t Buf;

	SPCR &=~(1<<CPOL);
	cli();
	PortCS_DAC_1 &=~CS_DAC_1;
	sei();
	Buf = DAC_Output[0]<<2;
	SPDR = (Buf>>8)&0x0F;					// Chan A
	WaitSPI();
	SPDR = Buf;
	WaitSPI();
	cli();
	PortCS_DAC_1 |=CS_DAC_1;
	sei();
	_delay_us(Delay_Sync);
	cli();
	PortCS_DAC_1 &=~CS_DAC_1;
	sei();
	Buf = DAC_Output[1]<<2;
	SPDR = ((Buf>>8)|0x80)&0x8F;	// Chan B
	WaitSPI();
	SPDR = Buf;
	WaitSPI();
	cli();
	PortCS_DAC_1 |=CS_DAC_1;
#ifdef CS_DAC_2
	PortCS_DAC_2 &=~CS_DAC_2;
	sei();
	Buf = DAC_Output[2]<<2;
	SPDR = (Buf>>8)&0x0F;					// Chan A
	WaitSPI();
	SPDR = Buf;
	WaitSPI();
	cli();
	PortCS_DAC_2 |=CS_DAC_2;
	sei();
	_delay_us(Delay_Sync);
	cli();
	PortCS_DAC_2 &=~CS_DAC_2;
	sei();
	Buf = DAC_Output[3]<<2;
	SPDR = ((Buf>>8)|0x80)&0x8F;	// Chan B
	WaitSPI();
	SPDR = Buf;
	WaitSPI();
	cli();
	PortCS_DAC_2 |=CS_DAC_2;
	sei();
#else
	sei();
	_delay_us(Delay_Sync);
#endif

	#ifdef DAC_Output_Ext
		for(uint8_t i=0; i<DAC_Reg_Ext; i++){
			IntOff();
			(DAC_Output_Ext)[i] = DAC_Output[DAC_Reg+i];
			IntOn();
		}
	#endif
}

// ~~~~~~~~~~~~~~~~~~~
#endif	// def DAC_Reg
// ~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~
// Total
// ~~~~~~~~~~~
void
SPI_Init(void)
{
	cli();
	#if defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		DDRB = (DDRB &~(1<<DDB3)) |(1<<DDB2) |(1<<DDB1) |(1<<DDB0);
		PORTB |=1<<PB3;
	#elif defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__)
		DDRB = (DDRB &~(1<<DDB6)) |(1<<DDB5) |(1<<DDB7) |(1<<DDB4);
		PORTB |=1<<PB6;
	#endif
	SPCR = (uint8_t)((~(1<<SPIE) |(1<<SPE)) &~(1<<DORD)) |(1<<MSTR) |(1<<CPHA) |(1<<SPR1) |(1<<SPR0);
	sei();

	#ifdef CS_DAC_1
		cli();
		DDR_CS_DAC_1 |=CS_DAC_1;
		PortCS_DAC_1 |=CS_DAC_1;
		sei();
	#endif
	#ifdef CS_DAC_2
		cli();
		DDR_CS_DAC_2 |=CS_DAC_2;
		PortCS_DAC_2 |=CS_DAC_2;
		sei();
	#endif

	#ifndef ADC_Miss
		cli();
		#ifdef CS_ADC
			DDR_CS_ADC |=CS_ADC;
			PortCS_ADC |=CS_ADC;
		#endif
		DDR_RESET_PER |=RESET_PER;
		#ifdef ADC_Mux
			DDR_INH |=INH;
			#ifdef DDR_INH2			
				DDR_INH2 |=INH2;
			#endif
			DDR_TER_A |=TER_A;
			DDR_TER_B |=TER_B;
		#endif
		sei();

		TD_ADC = Timer8_ISR_SysAlloc(1);


		ADC_Chan = ADC_Reg-1;		// Ignore 1-st measurement after calibration

		while(ADC_In()==ADC_Reg && ADC_OK);
			
	#endif
}
