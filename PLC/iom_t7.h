#include <stdint.h>

uint8_t		MB_Coil[4];
uint8_t		MB_Input[4];
uint16_t	MB_HoldReg[0x18];
uint16_t	MB_InReg[0x18];

#define Modbus_Map_List {				\
	{									\
		MB_Coil,		040,			\
		MB_Input,		040,			\
		MB_HoldReg,		0x18,			\
		MB_InReg,		0x18			\
	}									\
}

#define ADC_Ref_Chan

#define MB_ThermSets 1

// ~~~~~~~
// sys_*.h
// ~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define TerInput ADC_Input
#define TerReg (ADC_Reg-1)
#define InitThermCorr {{0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}, {0, 1000}}

// ~~~~~~~~~~~~~~~~~~
// system.h
// ~~~~~~~~~~~~~~~~~~
#include "iom.h"
#include "digiproc.h"
#include "spi.h"
#include "therm.h"

// ~~~~~~~~~~~~~~~
void
MB_Addr_Read(void)
{
	DDRC &=~(1<<PC3);
	DDRB &=~(1<<PB3) &~(1<<PB2) &~(1<<PB1);
	PORTC |=(1<<PC3);
	PORTB |=(1<<PB3) |(1<<PB2) |(1<<PB1);
	_delay_us(1);
	if (!(PINC &(1<<PC3)))
		MB_NativeAddr[0] += 1<<0;
	if (!(PINB &(1<<PB3)))
		MB_NativeAddr[0] += 1<<1;
	if (!(PINB &(1<<PB2)))
		MB_NativeAddr[0] += 1<<2;
	if (!(PINB &(1<<PB1)))
		MB_NativeAddr[0] += 1<<3;
}

// ~~~~~~~~~~~~~~
void
IOM_T7_Init(void)
{
	IOM_Init();

	SPI_Init();

	while(ADC_OK && ThermInit(ADC_In()));

	for(uint8_t i=0; i<TerReg; i++) {
		MB_InReg[9+i] = Temperature[i];

		uint8_t j = MB_ThermSets+i*3;
		MB_HoldReg[j]		= erb(ThermType+i);
		MB_HoldReg[j+1]	= (int8_t)erb(&ThermCorr[i].D);
		MB_HoldReg[j+2]	= erw(&ThermCorr[i].A);
	}

	IOM_Modbus_Init();
}

// ~~~~~~~~~~~~~~~
void
IOM_T7_Cycle(void)
{
	IOM_Cycle();

	static uint8_t ADC_Work = 1;
	if(ADC_Work && ADC_Phase==ADC_OFF) {
		Therm_ADC_Off();
		ADC_Work = 0;
		SetDigOut(7);
	}
	if(ADC_Work) {
		ThermChan(ADC_In());
		if (ADC_Phase==READ)
			ResDigOut(7);
		else
			SetDigOut(7);
	}

	cli();
	MB_InReg[17] = ADC_Fail;
	MB_InReg[18] = ADC_ReadFail;
	MB_InReg[19] = ADC_Break;
	sei();

	for(uint8_t i=0; i<TerReg; i++) {
		if (ThermChanWrong(i))
			SetDigOut(i);
		else
			ResDigOut(i);

		uint8_t j = MB_ThermSets+i*3;
		MB_InReg[9+i] = Temperature[i];
		if (erb(ThermType+i)!=MB_HoldReg[j])
			ewb(ThermType+i, MB_HoldReg[j]);
		if (erb(&ThermCorr[i].D)!=MB_HoldReg[j+1])
			ewb(&ThermCorr[i].D, MB_HoldReg[j+1]);
		cli();
		uint16_t MB_TC = MB_HoldReg[j+2];
		sei();
		if (erw(&ThermCorr[i].A)!=MB_TC)
			eww(&ThermCorr[i].A, MB_TC);
	}

	MB_InReg[16] = ADC_Input[7];
}
