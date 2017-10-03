/*~~~~~~~~~~~~~
	02.04.2012 - Kononenko -  Add define DI_INVERSION
	14.09.2012 - Kononenko -  Add define DigitIn_Hard_SPECIFIC
	27.09.2013 - Kononenko -  Apply define DI_INVERSION for DI_Map_Own
	19.12.2013 - Kononenko -  Apply define PortStrobe, PortData, PortEnable, PortClock
	23.12.2013 - Kononenko -  For DI_Map_Own and DO_Map_Own use static PROGMEM config array DI_M DO_M in PROGMEM instead RAM
								No define DI_INVERSION -> in struct T_DI_M
// ~~~~~~~~~~~~~~~~~~~~*/
#include <string.h>

#define DigitInit()	\
	DI_Init();				\
	DO_Init()

enum {NoFront, RiseFront, FallFront};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Discrete Inputs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef DI_Reg
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef InitInvers
	#define InitInvers {}
#endif
uint8_t DI_Invers[DI_Reg] EEMEM = InitInvers;
#ifndef DI_Reg_Ext
	#define DI_Reg_Ext 0
#endif
typedef uint8_t DI_Array[DI_Reg+DI_Reg_Ext];
struct {
	DI_Array State, StateOld, Status;
} DigInput;

// ~~~~~~~~~~~~~~~~~~
uint8_t
DI_State(uint8_t D_N)
{
	return (DigInput.State[D_N/8] &(1<<D_N%8)) != 0;
}

// ~~~~~~~~~~~~~~~~~~
uint8_t
DI_Front(uint8_t D_N)
{
	uint8_t Byte=D_N/8, Bit=1<<D_N%8;
	uint8_t State = DigInput.State[Byte] &Bit;
	return DigInput.StateOld[Byte] &Bit ? State?NoFront:FallFront : State?RiseFront:NoFront;
}

// ~~~~~~~~~~~~~~~~~~~
uint8_t
DI_Status(uint8_t D_N)
{
	return (DigInput.Status[D_N/8] &(1<<D_N%8)) != 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined DI_Map
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~
static void
DI_Init_Hard(void)
{
	cli();
	DDR_OUT_DEA |=OUT_DEA;
	DDR_OUT_DEB |=OUT_DEB;
	DDR_OUT_DEC |=OUT_DEC;
	DDR_OUT_DE1 &=~OUT_DE1;
	Port_OUT_DE1 |=OUT_DE1;
	DDR_OUT_DE2 &=~OUT_DE2;
	Port_OUT_DE2 |=OUT_DE2;
#ifdef OUT_DE3
	Port_OUT_DE3 |=OUT_DE3;
	DDR_OUT_DE3 &=~OUT_DE3;
#endif
	#ifdef ION_Init
		ION_Init();
	#endif
	sei();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
DI(uint8_t Mask, uint8_t Byte, uint8_t Bit)
{
	static prog_uint8_t M[DI_Reg][8] = DI_Map;
	uint8_t D_N = prb((uint8_t*)(M+Byte)+Bit);
	Byte = D_N/8;
	Bit = 1<<D_N%8;
	#ifdef DI_INVERSION
		if (Mask) DigInput.State[Byte] &=~Bit;
		else DigInput.State[Byte] |=Bit;
	#else
		if (Mask) DigInput.State[Byte] |=Bit;
		else DigInput.State[Byte] &=~Bit;
	#endif
}

// ~~~~~~~~~~~~~~~
static void
DigitIn_Hard(void)
{
	for (uint8_t i=0; i<8; i++) {
		cli();
		if (i/4)		Port_OUT_DEC |=OUT_DEC; else Port_OUT_DEC &=~OUT_DEC;
		if (i%4/2)		Port_OUT_DEB |=OUT_DEB; else Port_OUT_DEB &=~OUT_DEB;
		if (i%2)		Port_OUT_DEA |=OUT_DEA; else Port_OUT_DEA &=~OUT_DEA;
		sei();
		Delay_us(300);
		DI(Pin_OUT_DE1 &OUT_DE1, 0, i);
		DI(Pin_OUT_DE2 &OUT_DE2, 1, i);
	#ifdef OUT_DE3
		DI(Pin_OUT_DE3 &OUT_DE3, 2, i);
	#endif
		#ifdef ION_In
			ION_In();
		#endif
	}
}

// ~~~~~~~~~~~~~~~~~~~~~
#elif defined DI_Map_Own
// ~~~~~~~~~~~~~~~~~~~~~

typedef const struct {
	volatile uint8_t *DDR, *PORT, *PIN;
	uint8_t P, Inversion;
} PROGMEM T_DI_M;
static T_DI_M DI_M[] PROGMEM = DI_Map_Own;

// ~~~~~~~~~~~~~~~
static void
DI_Init_Hard(void)
{
	for (uint8_t D_N=0; D_N<sizeof(DI_M)/sizeof(T_DI_M); D_N++) {
		cli();
		*(uint8_t*)prp(&DI_M[D_N].DDR) &=~prb(&DI_M[D_N].P);
		*(uint8_t*)prp(&DI_M[D_N].PORT) |=prb(&DI_M[D_N].P);
		sei();
	}
}

// ~~~~~~~~~~~~~~~
#ifdef DigitIn_Hard_SPECIFIC
	void DigitIn_Hard(void);
#else
	static void
	DigitIn_Hard(void)
	{
		for (uint8_t D_N=0; D_N<sizeof(DI_M)/sizeof(T_DI_M); D_N++) {
			uint8_t Byte=D_N/8, Bit=1<<D_N%8;
			if(prb(&DI_M[D_N].Inversion)){
				if (*(uint8_t*)prp(&DI_M[D_N].PIN) & prb(&DI_M[D_N].P))
					DigInput.State[Byte] |=Bit;
				else
					DigInput.State[Byte] &=~Bit;
			}
			else{
				if (*(uint8_t*)prp(&DI_M[D_N].PIN) & prb(&DI_M[D_N].P))
					DigInput.State[Byte] &=~Bit;
				else
					DigInput.State[Byte]  |=Bit;
			}
		}
	}
#endif //DigitIn_Hard_SPECIFIC

// ~~~~~~~~~~~~~~~~~~~
#else
// ~~~~~~~~~~~~~~~~~~~

#define DI_Init_Hard()
#define DigitIn_Hard()

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#endif	// defined DI_Map
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~
void
DI_Init(void)
{
	DI_Init_Hard();
	#ifdef HSC_Reg
		HSC_Init();
	#endif
}

// ~~~~~~~~~~
void
DigitIn(void)
{
	memcpy(DigInput.StateOld, DigInput.State, DI_Reg+DI_Reg_Ext);
	for (uint8_t i=0; i<DI_Reg; i++)
		DigInput.Status[i] = DigInput.State[i]^erb(DI_Invers+i);
	DigitIn_Hard();
	#ifdef DigInput_Ext
		for (uint8_t i=0; i<DI_Reg_Ext; i++) {
			uint8_t j = DI_Reg+i;
			IntOff();
			DigInput.State[j]		= (DigInput_Ext)[i];
			DigInput.Status[j]	= (DigInput_Ext)[i+DI_Reg_Ext];
			IntOn();
		}
	#endif
}

// ~~~~~~~~~~~~~~~~~~
#endif	// def DI_Reg
// ~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~
// Discrete Outputs
// ~~~~~~~~~~~~~~~~~~
#ifndef DO_Reg_Ext
	#define DO_Reg_Ext 0
#endif
typedef uint8_t DO_Array[DO_Reg+DO_Reg_Ext];
struct {
	DO_Array State, StateOld, Buf;
} DigOutput;

// ~~~~~~~~~~~~~~~~~~
uint8_t
DO_State(uint8_t D_N)
{
	return (DigOutput.State[D_N/8] &(1<<D_N%8)) > 0;
}

// ~~~~~~~~~~~~~~~~~~
uint8_t
DO_Front(uint8_t D_N)
{
	uint8_t Byte=D_N/8, Bit=1<<D_N%8;
	uint8_t State = DigOutput.State[Byte] &Bit;
	return DigOutput.StateOld[Byte] &Bit ? State?NoFront:FallFront : State?RiseFront:NoFront;
}

// ~~~~~~~~~~~~~~~~~~~
void
SetDigOut(uint8_t D_N)
{
	DigOutput.Buf[D_N/8] |=(1<<D_N%8);
}

// ~~~~~~~~~~~~~~~~~~~
void
ResDigOut(uint8_t D_N)
{
	DigOutput.Buf[D_N/8] &=~(1<<D_N%8);
}

// ~~~~~~~~~~~~~~~~~~~
void
TogDigOut(uint8_t D_N)
{
	DigOutput.Buf[D_N/8] ^=(1<<D_N%8);
}

// ~~~~~~~~~~~~~~~
void
SetAllDigOut(void)
{
	for (uint8_t i=0; i<DO_Reg; DigOutput.Buf[i++]=0xFF);
}

// ~~~~~~~~~~~~~~~~
#if defined DO_Map
// ~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~
static void
DigitOut_Hard(void)
{
	static prog_uint8_t M[DO_Reg*8] = DO_Map;

	uint8_t N = DO_Reg*8;
	while (N) {
		uint8_t D_N = prb(M+--N);
		cli();
		if (DigOutput.State[D_N/8] &(1<<D_N%8))
			PortData  |=Data;
		else
			PortData &=~Data;
		sei();
		_delay_us(1);				// 125 ns needs at 5V
		cli();
		PortClock |=Clock;
		sei();
		_delay_us(1);				// 200 ns needs at 5V
		cli();
		PortClock &=~Clock;
		sei();
	}
	cli();
	PortStrobe |=Strobe;
	sei();
	_delay_us(1);					// 200 ns needs at 5V
	cli();
	PortStrobe &=~Strobe;
	sei();
}

// ~~~~~~~~~~~~~~~
static void
DO_Init_Hard(void)
{
	cli();
	DDR_Strobe |= Strobe;
	DDR_Data |= Data;
	DDR_Enable |= Enable;
	DDR_Clock |=Clock;
	PortStrobe &=~Strobe;
	PortClock &=~Clock;
	sei();
}

// ~~~~~~~~~~~~~~~~~~~~~
#elif defined DO_Map_Own
// ~~~~~~~~~~~~~~~~~~~~~

typedef const struct {
	volatile uint8_t *DDR, *PORT;
	uint8_t P;
} PROGMEM T_DO_M;
static T_DO_M DO_M[] PROGMEM = DO_Map_Own;

// ~~~~~~~~~~~~~~~
static void
DO_Init_Hard(void)
{
	for (uint8_t D_N=0; D_N<sizeof(DO_M)/sizeof(T_DO_M); D_N++) {
		cli();
		*(uint8_t*)prp(&DO_M[D_N].DDR) |= prb(&DO_M[D_N].P);
		sei();
	}
}

// ~~~~~~~~~~~~~~~~
static void
DigitOut_Hard(void)
{
	for (uint8_t D_N=0; D_N<sizeof(DO_M)/sizeof(T_DO_M); D_N++) {
		cli();
		if (DigOutput.State[D_N/8] &(1<<D_N%8))
			*(uint8_t*)prp(&DO_M[D_N].PORT)  |= prb(&DO_M[D_N].P);
		else
			*(uint8_t*)prp(&DO_M[D_N].PORT) &= ~prb(&DO_M[D_N].P);
		sei();
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~
#endif	// defined DO_Map
// ~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~
// Relay on
void
DO_Enable(void)
{
	#ifdef Enable
		IntOff();
		PortEnable |=Enable;
		IntOn();
	#endif
}

// ~~~~~~~~~
// Relay off
void
DO_Disable()
{
	#ifdef Enable
		IntOff();
		PortEnable &=~Enable;
		IntOn();
	#endif
}

// ~~~~~~~~~~~
void
DigitOut(void)
{
	memcpy(DigOutput.StateOld, DigOutput.State, DO_Reg+DO_Reg_Ext);
	memcpy(DigOutput.State, DigOutput.Buf, DO_Reg);
	DigitOut_Hard();
	#ifdef DigOutput_Ext
		for (uint8_t i=0; i<DO_Reg_Ext; i++) {
			uint8_t j = DO_Reg+i;
			IntOff();
				DigOutput.State[j] = (DigOutput_Ext)[i];
			(DigOutput_Ext)[i+DO_Reg_Ext] = DigOutput.Buf[j];
			IntOn();
		}
	#endif
}

// ~~~~~~~~~~
void
DO_Init(void)
{
	DO_Init_Hard();
	DigitOut();		// Clear all
	DO_Enable();
}
