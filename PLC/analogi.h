/*
	19.12.2013 - Kononenko -  Revised for vmd2_3. Change AI_Mux from prog_uint8_t Mux[] -> prog_uint16_t Mux[]
*/
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef AI_Reg
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef AI_Reg_Ext
	#define AI_Reg_Ext 0
#endif
uint16_t AnalogInput[AI_Reg+AI_Reg_Ext];

static uint8_t AI_N;

#define REFS (uint8_t)(~(1<<REFS1) &~(1<<REFS0) &~(1<<ADLAR))

#if defined ADFR
	#define AD_Mode ADFR
#elif defined ADATE
	#define AD_Mode ADATE
#endif

#define ADC0    REFS &~(1<<MUX4) &~(1<<MUX3) &~(1<<MUX2)  &~(1<<MUX1)  &~(1<<MUX0)
#define ADC1   (REFS &~(1<<MUX4) &~(1<<MUX3) &~(1<<MUX2)  &~(1<<MUX1))  |(1<<MUX0)
#define ADC2  ((REFS &~(1<<MUX4) &~(1<<MUX3) &~(1<<MUX2))  |(1<<MUX1)) &~(1<<MUX0)
#define ADC3   (REFS &~(1<<MUX4) &~(1<<MUX3) &~(1<<MUX2))  |(1<<MUX1)   |(1<<MUX0)
#define ADC4  ((REFS &~(1<<MUX4) &~(1<<MUX3)) |(1<<MUX2)) &~(1<<MUX1)  &~(1<<MUX0)
#define ADC5 (((REFS &~(1<<MUX4) &~(1<<MUX3)) |(1<<MUX2)) &~(1<<MUX1))  |(1<<MUX0)
#define ADC6  ((REFS &~(1<<MUX4) &~(1<<MUX3)) |(1<<MUX2)   |(1<<MUX1)) &~(1<<MUX0)
#define ADC7   (REFS &~(1<<MUX4) &~(1<<MUX3)) |(1<<MUX2)   |(1<<MUX1)   |(1<<MUX0)
#define ADC8	( ((1<<MUX5)<<8) + (ADC0) )
#define ADC9	( ((1<<MUX5)<<8) + (ADC1) )
#define ADC10	( ((1<<MUX5)<<8) + (ADC2) )
#define ADC11	( ((1<<MUX5)<<8) + (ADC3) )
#define ADC12	( ((1<<MUX5)<<8) + (ADC4) )
#define ADC13	( ((1<<MUX5)<<8) + (ADC5) )
#define ADC14	( ((1<<MUX5)<<8) + (ADC6) )
#define ADC15	( ((1<<MUX5)<<8) + (ADC7) )

#ifdef InitAI_Filt
	prog_uint8_t AI_Filt[AI_Reg] = InitAI_Filt;
	static uint16_t *AI_Buf[AI_Reg];
#endif
#ifdef InitAI_10ms
	static uint8_t TD_InnADC;
	uint8_t AI_10ms[AI_Reg] EEMEM = InitAI_10ms;
#endif
#if defined InitAI_Filt || defined InitAI_10ms
	uint16_t AnalogInputF[AI_Reg];
#endif

// ~~~~~~~~~~~
static void
SetMuxAI(void)
{
	static prog_uint16_t Mux[] = AI_Mux;
	ADMUX = (uint8_t) prw(Mux+AI_N);
	#ifdef MUX5
		ADCSRB = (uint8_t) (prw(Mux+AI_N)>>8);
	#endif
	#ifdef AI_Mux2
		static prog_uint8_t Mux2[] = AI_Mux2;
		if (AI_N>sizeof(Mux2)-1)
			return;
		uint8_t Addr = prb(Mux2+AI_N);
		cli();
		if (Addr%2)		PortAN_A |=AN_A; else PortAN_A &=~AN_A;
		if (Addr%4/2)	PortAN_B |=AN_B; else PortAN_B &=~AN_B;
	#ifdef AN_C
		if (Addr/4)		PortAN_C |=AN_C; else PortAN_C &=~AN_C;
	#endif
		sei();
		Delay_us(60);
	#endif
}

// ~~~~~~~~~~
void
AI_Init(void)
{
	#ifdef AI_Mux2
		DDR_AN_A |=AN_A;
		DDR_AN_B |=AN_B;
		#ifdef AN_C
			DDR_AN_C |=AN_C;
		#endif
	#endif
	SetMuxAI();
	ADCSRA = (((((1<<ADEN) |(1<<ADSC)) &~(1<<AD_Mode)) |(1<<ADIF)) &~(1<<ADIE)) |(1<<ADPS2) |(1<<ADPS1) |(1<<ADPS0);

	#ifdef InitAI_Filt
		for (uint8_t i=0; i<AI_Reg; i++) {
			uint8_t Filt=prb(AI_Filt+i);
			if (Filt)
				AI_Buf[i] = (uint16_t*)calloc(4*Filt, 2);
		}
	#endif
	#ifdef InitAI_10ms
		TD_InnADC = Timer8SysAlloc(AI_Reg);
	#endif
}

// ~~~~~~~~~~~
uint8_t
AnalogIn(void)
{
	if (ADCSRA &(1<<ADSC))
		return AI_Reg;
	AnalogInput[AI_N] = ADCL;
	AnalogInput[AI_N] |= ADCH<<8;
	uint8_t Chan = AI_N;

	#ifdef InitAI_10ms
		if (Timer8Stopp(TD_InnADC+AI_N)) {
			StartTimer8(TD_InnADC+AI_N, erb(AI_10ms+AI_N));
	#endif
	#if defined InitAI_Filt
		if (AI_Buf[AI_N]) {
			uint8_t Filt = prb(AI_Filt+AI_N);
			for (uint8_t i=Filt*4-1; i; i--)
				AI_Buf[AI_N][i] = AI_Buf[AI_N][i-1];
			AI_Buf[AI_N][0] = AnalogInput[AI_N];
			AnalogInputF[AI_N] = DigiFilt(AI_Buf[AI_N], Filt);
		}
		else
			AnalogInputF[AI_N] = AnalogInput[AI_N];
	#elif defined InitAI_10ms
		AnalogInputF[AI_N] = AnalogInput[AI_N];
	#endif
	#ifdef InitAI_10ms
		}
		else
			Chan = AI_Reg;
	#endif

	if (++AI_N == AI_Reg)
		AI_N = 0;
	SetMuxAI();
	ADCSRA |=(1<<ADSC);

	#ifdef AnalogInput_Ext
		for (uint8_t i=0; i<AI_Reg_Ext; i++){
			IntOff();
			AnalogInput[AI_Reg+i] = (AnalogInput_Ext)[i];
			IntOn();
		}
	#endif

	return Chan;
}

#endif //AI_Reg
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
