/*
	19.09.2012 - Kononenko - Add define PWM_PERIOD
	14.12.2013 - Kononenko - Set IntOff();IntOn(); -> OCR3AH = Buf>>8; OCR3AL = Buf;
	19.12.2013 - Kononenko - Revised for vmd2_3
	27.12.2013 - Kononenko - Revised for vfc2 (PWM_Reg == 2)
*/

#ifdef PWM_Reg

	#ifndef DAC_Reg_Ext
		#define DAC_Reg_Ext 0
	#endif
	uint16_t DAC_Output[PWM_Reg+DAC_Reg_Ext];

	#define MaxAO 1023

#endif


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if PWM_Reg < 4
	#ifndef PWM_PERIOD
		#define PWM_PERIOD 0xFFFF
	#endif

	//#define Coef_AO_PWM (float)0xFFFF/MaxAO
	#define Coef_AO_PWM (float)PWM_PERIOD/MaxAO
#endif


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if PWM_Reg == 2
// ~~~~~~~~~~~
void
PWM_Init(void)
{
	TCCR3A = (0<<COM3A1) | (0<<COM3A0) | (1<<COM3B1) | (0<<COM3B0) | (1<<COM3C1) | (0<<COM3C0) | (1<<WGM31) | (0<<WGM30);
	TCCR3B = (0<<CS32) | (0<<CS31) | (1<<CS30) | (1<<WGM33) | (1<<WGM32);	// Fpwm = 244Hz
	ICR3 = PWM_PERIOD;
	DDRE |= (1<<PE4) | (1<<PE5);
}

// ~~~~~~~~~~
void
PWM_Out(void)
{
	uint16_t Buf;

	Buf = Coef_AO_PWM*DAC_Output[1];
	IntOff();
	OCR3BH = Buf>>8;
	OCR3BL = Buf;
	IntOn();

	Buf = Coef_AO_PWM*DAC_Output[0];
	IntOff();
	OCR3CH = Buf>>8;
	OCR3CL = Buf;
	IntOn();

	#ifdef DAC_Output_Ext
		for (uint8_t i=0; i<DAC_Reg_Ext; i++){
			IntOff();
			(DAC_Output_Ext)[i] = DAC_Output[DAC_Reg+i];
			IntOn();
		}
	#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif PWM_Reg == 3
// ~~~~~~~~~~~
void
PWM_Init(void)
{
	TCCR3A = (((((((1<<COM3A1) &~(1<<COM3A0)) |(1<<COM3B1)) &~(1<<COM3B0)) |(1<<COM3C1)) &~(1<<COM3C0)) |(1<<WGM31)) &~(1<<WGM30);
	TCCR3B = (~(1<<CS32) &~(1<<CS31)) |(1<<CS30) |(1<<WGM33) |(1<<WGM32);	// Fpwm = 244Hz
	ICR3 = PWM_PERIOD;
	DDRE |=(1<<PE3) |(1<<PE4) |(1<<PE5);
}

// ~~~~~~~~~~
void
PWM_Out(void)
{
	uint16_t Buf;

	Buf = Coef_AO_PWM*DAC_Output[0];
	IntOff();
	OCR3AH = Buf>>8;
	OCR3AL = Buf;
	IntOn();

	Buf = Coef_AO_PWM*DAC_Output[1];
	IntOff();
	OCR3BH = Buf>>8;
	OCR3BL = Buf;
	IntOn();

	Buf = Coef_AO_PWM*DAC_Output[2];
	IntOff();
	OCR3CH = Buf>>8;
	OCR3CL = Buf;
	IntOn();

	#ifdef DAC_Output_Ext
		for (uint8_t i=0; i<DAC_Reg_Ext; i++){
			IntOff();
			(DAC_Output_Ext)[i] = DAC_Output[DAC_Reg+i];
			IntOn();
		}
	#endif
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif PWM_Reg==4

#ifndef PWM_PERIOD_TC3
	#define PWM_PERIOD_TC3 0xFFFF
#endif
#ifndef PWM_PERIOD_TC5
	#define PWM_PERIOD_TC5 0xFFFF
#endif

#define Coef_AO_PWM_TC3 (float)PWM_PERIOD_TC3/MaxAO
#define Coef_AO_PWM_TC5 (float)PWM_PERIOD_TC5/MaxAO

// ~~~~~~~~~~~
void
PWM_Init(void)
{
	TCCR3A = (1<<COM3A1) | (0<<COM3A0) | (1<<COM3B1) | (0<<COM3B0) | (0<<COM3C1) | (0<<COM3C0) | (1<<WGM31) | (0<<WGM30);
	TCCR3B = (0<<CS32) | (0<<CS31) | (1<<CS30) | (1<<WGM33) | (1<<WGM32);	// Fpwm = 244Hz
	ICR3 = PWM_PERIOD_TC3;
	DDRE |= (1<<PE3) | (1<<PE4);

	TCCR5A = (1<<COM5A1) | (0<<COM5A0) | (1<<COM5B1) | (0<<COM5B0) | (0<<COM5C1) | (0<<COM5C0) | (1<<WGM51) | (0<<WGM50);
	TCCR5B = (0<<CS52) | (0<<CS51) | (1<<CS50) | (1<<WGM53) | (1<<WGM52);	// Fpwm = 244Hz
	ICR5 = PWM_PERIOD_TC5;
	DDRL |= (1<<PL3) | (1<<PL4);
}

// ~~~~~~~~~~
void
PWM_Out(void)
{
	uint16_t Buf;

	Buf = Coef_AO_PWM_TC3*DAC_Output[0];
	IntOff();
	OCR3AH = Buf>>8;
	OCR3AL = Buf;
	IntOn();

	Buf = Coef_AO_PWM_TC3*DAC_Output[1];
	IntOff();
	OCR3BH = Buf>>8;
	OCR3BL = Buf;
	IntOn();


	Buf = Coef_AO_PWM_TC5*DAC_Output[2];
	IntOff();
	OCR5AH = Buf>>8;
	OCR5AL = Buf;
	IntOn();

	Buf = Coef_AO_PWM_TC5*DAC_Output[3];
	IntOff();
	OCR5BH = Buf>>8;
	OCR5BL = Buf;
	IntOn();

	#ifdef DAC_Output_Ext
		for (uint8_t i=0; i<DAC_Reg_Ext; i++){
			IntOff();
			(DAC_Output_Ext)[i] = DAC_Output[PWM_Reg+i];
			IntOn();
		}
	#endif
}
		
#endif	// def PWM_Reg
// ~~~~~~~~~~~~~~~~~~~
