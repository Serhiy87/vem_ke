// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Divisible by 8!
#define Timer8ApplManuNumber	0
#define Timer16ApplManuNumber	0
#define Timer32ApplManuNumber	0

#define Timer8ApplNumber	010
#define Timer16ApplNumber	040
#define Timer32ApplNumber	010

// ~~~~~~~~~~~~~~~~~~~~~~~~~
void MB_LED_Err_On(void) {}
void MB_LED_Err_Off(void) {}

// ~~~~~~~~~~~~~~~~~
// system.h
// ~~~~~~~~~~~~~~~~~
#include "memory.h"
#include "interru.h"
#include "time.h"
#include "usart2.h"

MB_Master MB_Master_List[] = {};

void SetDigOut(uint8_t);
void ResDigOut(uint8_t);
void SetAllDigOut(void);
void DO_Init(void);
void DigitOut(void);

// ~~~~~~~~~~~
void
IOM_Init(void)
{
	TimeInit();
	MemCheckInit();
	sei();
	DO_Init();

	SetAllDigOut();
	DigitOut();
}

// ~~~~~~~~~~~~~~~~~~
void
IOM_Modbus_Init(void)
{
	void MB_Addr_Read(void);
	cli();
	USART_Init();
	MB_Addr_Read();
	*(uint8_t*)MB_HoldReg = MB_BitRate_RAM[0];
	*((uint8_t*)MB_HoldReg+1) = erb(&MB_Param[0].Parity) + ((erb(&MB_Param[0].StopBits)-1)<<2);
	sei();
}

// ~~~~~~~~~~~~
void
IOM_Cycle(void)
{
	TimeCycle();
	MemCheck();
	USART_Cycle();

	MB_InReg[0] = MemFail;

	IntOff();
	for (uint8_t i=0; i<8; i++)
		MB_InReg[1+i] = MB_CPT[0][i];
	IntOn();

	if (*(uint8_t*)MB_HoldReg<=MBBR57600 && *(uint8_t*)MB_HoldReg!=MB_BitRate_RAM[0]) {
		ewb(&MB_Param[0].BitRate, *(uint8_t*)MB_HoldReg);
		SetBAUD(0);
	}
	uint8_t MB_Set_Flg = 0;
	uint8_t MB_Set;
	uint8_t MB_Set_Curr = erb(&MB_Param[0].Parity);
	if ((MB_Set=*((uint8_t*)MB_HoldReg+1)&0b11)!=0b11 && MB_Set!=MB_Set_Curr) {
		ewb(&MB_Param[0].Parity, MB_Set_Curr=MB_Set);
		MB_Set_Flg = 1;
	}
	if (((MB_Set=*((uint8_t*)MB_HoldReg+1)&0b100?2:1)==1 || MB_Set_Curr==NoParity) && MB_Set!=erb(&MB_Param[0].StopBits)) {
		ewb(&MB_Param[0].StopBits, MB_Set);
		MB_Set_Flg = 1;
	}
	if (MB_Set_Flg)
		SetParity(0);

		DigitOut();
}
