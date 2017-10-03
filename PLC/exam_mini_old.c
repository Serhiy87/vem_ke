#define sys_plc "sys_mini_old.h"

#define TD_InnADC	0
#define TD_EndInnADC	11

#define MB_C_Quant 0x20
#define MB_I_Quant 0x20
#define MB_HR_Quant 4
#define MB_IR_Quant 0x50

#define InitAI_10ms {[TerIdx]=Ter10ms, Ter10ms}

#define InitMenu Menu0

#include "system.h"

MenuLine Menu0List[] = {
	{" ---- Меню ---- ", .InnPage=&Menu0},
	{" Служебное      ", .InnPage=&MenuSyst}
};
MenuPage Menu0 = {SetMenu(Menu0List), 0, &Menu0};

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
