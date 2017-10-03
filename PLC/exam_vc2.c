#include <stdint.h>

#define sys_plc "sys_vc2.h"

#define LM_Exist 0

// ~~~~~~~~~~~~~~~
// Divisible by 8!
#define Timer8ApplManuNumber	040
#define Timer16ApplManuNumber	060
#define Timer32ApplManuNumber	020
#define Timer8ApplNumber	(Timer8ApplManuNumber  + 040)
#define Timer16ApplNumber	(Timer16ApplManuNumber + 030)
#define Timer32ApplNumber	(Timer32ApplManuNumber + 020)

uint8_t		MB_Coil[4];
uint8_t		MB_Input[4];
uint16_t	MB_HoldReg[4];
uint16_t	MB_InReg[80];

#define Modbus_Map_List {	\
	{												\
		MB_Coil,		040,			\
		MB_Input,		040,			\
		MB_HoldReg,	4,				\
		MB_InReg,		80				\
	}												\
}

#define RTYP (MB_InReg+38)
#define RMON (MB_InReg+39)
#define RKEY (MB_Coil+3)

#define InitMenu Menu0

#include "system.h"

MenuLine Menu0List[] = {
	{" -- Vega-Classic -- ", .InnPage=&Menu0},
	{" System version: "SysVer},
	{"                    "},
	{" Служебное          ", .InnPage=&MenuSyst}
};
MenuPage Menu0 = {SetMenu(Menu0List), 0, &Menu0};

MB_Master MB_Master_List[] = {};

// ~~~~~~~~~~~
void
ApplInit(void)
{
}

// ~~~~~~~~~~~~
void
ApplCycle(void)
{
}

// ~~~~~~~~~~~
void
StartKey(void)
{
}

// ~~~~~~~~~~
void
StopKey(void)
{
}
