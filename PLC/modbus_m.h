// 24.11.2011 Kononenko add extern define MB_M_Pause_Min: some devices do not like small pause!
// 03.02.2012 Kononenko fix bug uint8_t MB_Slave_Off() must be uint32_t MB_Slave_Off()
// 25.07.2012 Kononenko fix bugs frame fusion together (no timeout delay between frames)
// 29.07.2013 Kononenko в FailSlave() добавил MB_CPT[MB_N][4]++ кол-во сообщений без правильного ответа
// 11.07.2016 Kononenko Master зависал на прерывании DRE при малом Timeout. Scancycle ~ 800мс! Добавил режим MB_NoReply.
// 23.09.2016 Kononenko Устранено неправильное формирование данных FormData(MB_N) в многократных запросах 
//				при вызове MB_SingleRun подряд более 1раза
// 30.03.2017 Kucherenko - Go AVRStudio4 on AtmelStudio7:
// 1) typedef struct {...} PROGMEM MB_Query; -> typedef const struct {...} PROGMEM MB_Query;
// ... All variables PROGMEM -> ...const ... PROGMEM
// 2) MB_Slave *Slave; -> MB_Slave *Slave = NULL;
//    MB_Query *Query; -> MB_Query *Query = NULL;
//    uint8_t Err; -> uint8_t Err = 0;

	
#ifndef MB_M_Pause_Min
	#define MB_M_Pause_Min 5
#endif

#define Std_Modbus_M_Param {100, 20, 5, 60}
#ifndef Init_Modbus_M_Param
	#define Init_Modbus_M_Param {Std_Modbus_M_Param}
#endif
static struct {
	uint16_t Period;
	uint8_t Timeout, Try, TimeOff;
} MB_M_Param_EE[Modbus_Qt] EEMEM = Init_Modbus_M_Param;

uint16_t MB_M_Period[Modbus_Qt];
uint8_t MB_M_Timeout[Modbus_Qt], MB_M_Try[Modbus_Qt], MB_M_TimeOff[Modbus_Qt];

typedef const struct {
	uint8_t Func;
	uint16_t StartAddr, Qt;
	uint16_t AllocAddr;
} PROGMEM MB_Query;
typedef const struct {
	uint8_t Addr;
	MB_Query *QueryList;
	uint8_t QueryQt;
} PROGMEM MB_Slave;
typedef const struct {
	MB_Slave *SlaveList;
	uint8_t SlaveQt;
} PROGMEM MB_Master;
extern MB_Master MB_Master_List[Modbus_Qt];

uint8_t CurrSlave[Modbus_Qt], CurrQuery[Modbus_Qt];

static struct {
	uint8_t Addr;
	uint8_t Func;
	uint16_t StartAddr, Qt;
	void *Buf;
} MB_SingleQuery[Modbus_Qt];

enum {MB_SingleIdle, MB_SingleStart, MB_SingleExec};
volatile uint8_t MB_SingleState[Modbus_Qt];
static volatile uint8_t LastProcSingle;

#define MB_SingleBusy(MB_N) (MB_SingleState[MB_N]!=MB_SingleIdle)

static uint8_t Try[Modbus_Qt];

uint32_t SlaveOff[Modbus_Qt];
static uint32_t SlaveSkip[Modbus_Qt];

static uint8_t TD_MB;

#define TD_MB_Idle			 TD_MB
#define TD_MB_SlaveOff	(TD_MB+Modbus_Qt)
#define TD_MB_Cycle			(TD_MB+Modbus_Qt*2)

uint16_t MB_Master_Cycle[Modbus_Qt], MB_Master_MaxCycle[Modbus_Qt];

// ~~~~~~~~~~~~~~~~~~~~~~~~~~
void
MB_M_Period_Set(uint8_t MB_N)
{
	eww(&MB_M_Param_EE[MB_N].Period, MB_M_Period[MB_N]);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
MB_M_Timeout_Set(uint8_t MB_N)
{
	ewb(&MB_M_Param_EE[MB_N].Timeout, MB_M_Timeout[MB_N]);
}

// ~~~~~~~~~~~~~~~~~~~~~~~
void
MB_M_Try_Set(uint8_t MB_N)
{
	ewb(&MB_M_Param_EE[MB_N].Try, MB_M_Try[MB_N]);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
MB_M_TimeOff_Set(uint8_t MB_N)
{
	ewb(&MB_M_Param_EE[MB_N].TimeOff, MB_M_TimeOff[MB_N]);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint32_t
MB_Slave_Off(uint8_t MB_N, uint8_t S_N)
{
	return SlaveOff[MB_N] &((uint32_t)1<<S_N);
}

// ~~~~~~~~~~~~~~~~~~~~
void
MB_M_Init(uint8_t MB_N)
{
	MB_M_Period[MB_N]		= erw(&MB_M_Param_EE[MB_N].Period);
	MB_M_Timeout[MB_N]	= erb(&MB_M_Param_EE[MB_N].Timeout);
	MB_M_Try[MB_N]			= erb(&MB_M_Param_EE[MB_N].Try);
	MB_M_TimeOff[MB_N]	= erb(&MB_M_Param_EE[MB_N].TimeOff);

	TD_MB = Timer16SysAlloc(Modbus_Qt*3);
}

// ~~~~~~~~~~~~~~~~~~~
static MB_Slave*
GetSlave(uint8_t MB_N)
{
	return (MB_Slave*)prp(&(MB_Master_List+MB_N)->SlaveList)+CurrSlave[MB_N];
}

// ~~~~~~~~~~~~~~~~~~~
static MB_Query*
GetQuery(uint8_t MB_N)
{
	return (MB_Query*)prp(&GetSlave(MB_N)->QueryList)+CurrQuery[MB_N];
}

// ~~~~~~~~~~~~~~~~~~~
static void
SlavesOn(uint8_t MB_N)
{
	SlaveSkip[MB_N] = 0;
	StartTimer16(TD_MB_SlaveOff+MB_N, 100*MB_M_TimeOff[MB_N]);
}

// ~~~~~~~~~~~~~~~~~
static uint16_t*
GetReg(uint8_t MB_N)
{
	return MB_SingleState[MB_N]!=MB_SingleExec?(uint16_t*)prp(&Modbus_Map[MB_N].HoldReg)+prw(&GetQuery(MB_N)->AllocAddr):MB_SingleQuery[MB_N].Buf;
}

// ~~~~~~~~~~~~~~~~~~~
static void
FormData(uint8_t MB_N)
{
	uint16_t Qt;
	uint16_t *Reg;
	switch (MB_Frame[MB_N][1]) {
	case 1: case 2: case 3: case 4:
		MB_Frame_Sz[MB_N] = 8;
		break;
	/*
	case 5:
		MB_Frame_Sz[MB_N] = 8;
		MB_Frame[MB_N][5] = 0;
		break;
	*/
	case 6:
		MB_Frame_Sz[MB_N] = 8;
		Reg = GetReg(MB_N);
		MB_Frame[MB_N][4] = *Reg>>8;
		MB_Frame[MB_N][5] = *Reg;
		break;
	case 0x0F:
		Qt = MB_FrameReg(MB_N, 4);
		MB_Frame_Sz[MB_N] = (MB_Frame[MB_N][6] = Qt/8 + (Qt%8>0)) + 9;
		MB_Frame[MB_N][7+Qt/8] = 0;
		uint8_t *Discr;
		uint8_t Pin;
		if (MB_SingleState[MB_N]!=MB_SingleExec) {
			Discr = prp(&Modbus_Map[MB_N].Coil);
			Pin = prw(&GetQuery(MB_N)->AllocAddr);
		}
		else {
			Discr = MB_SingleQuery[MB_N].Buf;
			Pin = 0;
		}
		for (uint8_t i=0; i<Qt; i++) {
			uint8_t j = Pin+i;
			uint8_t Byte=7+i/8, Bit=1<<i%8;
			if (Discr[j/8] &(1<<j%8))
				MB_Frame[MB_N][Byte] |=Bit;
			else
				MB_Frame[MB_N][Byte] &=~Bit;
		}
		break;
	case 0x10:
		MB_Frame_Sz[MB_N] = (MB_Frame[MB_N][6] = MB_Frame[MB_N][5]*2) + 9;
		Reg = GetReg(MB_N);
		for (uint8_t i=0; i<MB_Frame[MB_N][5]; i++) {
			uint8_t j = i*2;
			MB_Frame[MB_N][7+j] = Reg[i]>>8;
			MB_Frame[MB_N][8+j] = Reg[i];
		}
		break;
	}
	SetCRC_MB(MB_N);
}

// ~~~~~~~~~~~~~~~~~~~~
static void
FormQuery(uint8_t MB_N)
{
	MB_Slave *Slave = GetSlave(MB_N);
	MB_Query *Query = (MB_Query*)prp(&Slave->QueryList)+CurrQuery[MB_N];

	MB_Frame[MB_N][0] = prb(&Slave->Addr);
	MB_Frame[MB_N][1] = prb(&Query->Func);
	MB_Frame[MB_N][2] = prb((uint8_t*)&Query->StartAddr+1);
	MB_Frame[MB_N][3] = prb((uint8_t*)&Query->StartAddr);
	MB_Frame[MB_N][4] = prb((uint8_t*)&Query->Qt+1);
	MB_Frame[MB_N][5] = prb((uint8_t*)&Query->Qt);

	FormData(MB_N);
}

// ~~~~~~~~~~~~~~~~~~~~~
static void
FormSingle(uint8_t MB_N)
{
	MB_Frame[MB_N][0] = MB_SingleQuery[MB_N].Addr;
	MB_Frame[MB_N][1] = MB_SingleQuery[MB_N].Func;
	MB_Frame[MB_N][2] = MB_SingleQuery[MB_N].StartAddr>>8;
	MB_Frame[MB_N][3] = MB_SingleQuery[MB_N].StartAddr;
	MB_Frame[MB_N][4] = MB_SingleQuery[MB_N].Qt>>8;
	MB_Frame[MB_N][5] = MB_SingleQuery[MB_N].Qt;
	
	MB_SingleState[MB_N] = MB_SingleExec;
	FormData(MB_N);
}

// ~~~~~~~~~~~~~~~~~~~~
static uint8_t
NextSlave(uint8_t MB_N)
{
	uint8_t Over = 0;
	uint8_t i = 0;
	uint8_t SlaveQt = prb(&(MB_Master_List+MB_N)->SlaveQt);
	do
		if (++CurrSlave[MB_N]==SlaveQt) {
			CurrSlave[MB_N] = 0;
			Over = 1;

			if ((MB_Master_Cycle[MB_N]=0xFFFF-GetTimer16(TD_MB_Cycle+MB_N))>MB_Master_MaxCycle[MB_N])
				MB_Master_MaxCycle[MB_N] = MB_Master_Cycle[MB_N];
			StartTimer16(TD_MB_Cycle+MB_N, 0xFFFF);
		}
	while (SlaveSkip[MB_N] &((uint32_t)1<<CurrSlave[MB_N]) && ++i<SlaveQt);
	if (i==SlaveQt) {
		SlavesOn(MB_N);
		if (++CurrSlave[MB_N]==SlaveQt)
			CurrSlave[MB_N] = 0;
	}
	CurrQuery[MB_N] = 0;
	return Over;
}

// ~~~~~~~~~~~~~~~~~~~~
static uint8_t
FailSlave(uint8_t MB_N)
{
	uint8_t Over = 0;
	if (MB_SingleState[MB_N]==MB_SingleExec)
		MB_SingleState[MB_N] = MB_SingleIdle;
	else{
		MB_CPT[MB_N][4]++;	// кол-во сообщений без правильного ответа
		if (++Try[MB_N]==MB_M_Try[MB_N]) {
			uint32_t S_N = (uint32_t)1<<CurrSlave[MB_N];
			SlaveOff[MB_N] |=S_N;
			SlaveSkip[MB_N] |=S_N;
			Try[MB_N] = 0;
			Over = NextSlave(MB_N);
		}
	}
	return Over;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline static uint8_t
MB_M_ReadDiscr(uint8_t MB_N, MB_Query *Query)
{
	uint8_t Qt = LastProcSingle ? MB_SingleQuery[MB_N].Qt : prb((uint8_t*)&Query->Qt);
	if (MB_Frame_Sz[MB_N]!=MB_Frame[MB_N][2]+5 || Qt/8+((Qt%8)>0)!=MB_Frame[MB_N][2])
		return 1;
	uint8_t *Discr;
	uint8_t Pin;
	if (LastProcSingle) {
		Discr = MB_SingleQuery[MB_N].Buf;
		Pin = 0;
	}
	else {
		Discr = MB_Frame[MB_N][1]==1 ? prp(&Modbus_Map[MB_N].Coil) : prp(&Modbus_Map[MB_N].Input);
		Pin = prw(&Query->AllocAddr);
	}
	for (uint16_t i=0; i<Qt; i++) {
		uint8_t j = Pin+i;
		uint8_t Byte=j/8, Bit=1<<j%8;
		if (MB_Frame[MB_N][3+i/8] &(1<<i%8))
			Discr[Byte] |=Bit;
		else
			Discr[Byte] &=~Bit;
	}
	return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline static uint8_t
MB_M_ReadReg(uint8_t MB_N, MB_Query *Query)
{
	if (MB_Frame_Sz[MB_N]!=MB_Frame[MB_N][2]+5 || (LastProcSingle?MB_SingleQuery[MB_N].Qt:prw(&Query->Qt))*2!=MB_Frame[MB_N][2])
		return 1;
	uint16_t *Reg = LastProcSingle ?
		MB_SingleQuery[MB_N].Buf
	:
		(uint16_t*)(MB_Frame[MB_N][1]==3?prp(&Modbus_Map[MB_N].HoldReg):prp(&Modbus_Map[MB_N].InReg))+prw(&Query->AllocAddr);
	for (int8_t j=MB_Frame[MB_N][2]/2-1; j>=0; j--) {
		int8_t i = j*2;
		Reg[j] = MB_FrameReg(MB_N, 3+i);
	}
	return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline static uint8_t
MB_M_PresetSingle(uint8_t MB_N, MB_Query *Query)
{
	uint16_t StartAddr;
	uint16_t *Reg;
	if (LastProcSingle) {
		StartAddr = MB_SingleQuery[MB_N].StartAddr;
		Reg = MB_SingleQuery[MB_N].Buf;
	}
	else {
		StartAddr = prw(&Query->StartAddr);
		Reg = (uint16_t*)prp(&Modbus_Map[MB_N].HoldReg)+prw(&Query->AllocAddr);
	}
	return MB_Frame_Sz[MB_N]!=8 || MB_FrameReg(MB_N, 2)!=StartAddr || MB_FrameReg(MB_N, 4)!=*Reg;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline static uint8_t
MB_M_Force(uint8_t MB_N, MB_Query *Query)
{
	uint16_t StartAddr, Qt;
	if (LastProcSingle) {
		StartAddr	= MB_SingleQuery[MB_N].StartAddr;
		Qt				= MB_SingleQuery[MB_N].Qt;
	}
	else {
		StartAddr	= prw(&Query->StartAddr);
		Qt				= prw(&Query->Qt);
	}
	return MB_Frame_Sz[MB_N]!=8 || MB_FrameReg(MB_N, 2)!=StartAddr || MB_FrameReg(MB_N, 4)!=Qt;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~
inline static void
MB_M_CtrlProc(uint8_t MB_N)
{
	uint8_t Over = 0;
	LastProcSingle = MB_SingleState[MB_N]==MB_SingleExec;
	if (CommErr(MB_N))
		Over = FailSlave(MB_N);
	else {
		USART_Func(MB_N, LED);
		MB_Slave *Slave = NULL;
		MB_Query *Query = NULL;
		uint8_t Addr, Func;
		if (LastProcSingle) {
			Addr = MB_SingleQuery[MB_N].Addr;
			Func = MB_SingleQuery[MB_N].Func;
		}
		else {
			Slave = GetSlave(MB_N);
			Query = (MB_Query*)prp(&Slave->QueryList)+CurrQuery[MB_N];
			Addr = prb(&Slave->Addr);
			Func = prb(&Query->Func);
		}
		if (MB_Frame[MB_N][0]!=Addr || MB_Frame[MB_N][1]!=Func)
			Over = FailSlave(MB_N);
		else {
			uint8_t Err = 0;
			switch (Func) {
			case 1: case 2:
				Err = MB_M_ReadDiscr(MB_N, Query);
				break;
			case 3: case 4:
				Err = MB_M_ReadReg(MB_N, Query);
				break;
			case 6:
				Err = MB_M_PresetSingle(MB_N, Query);
				break;
			case 0x0F: case 0x10:
				Err = MB_M_Force(MB_N, Query);
				break;
			}
			if (Err) {
				MB_CPT[MB_N][2]++;
				Over = FailSlave(MB_N);
			}
			else if (LastProcSingle)
				MB_SingleState[MB_N] = MB_SingleIdle;
			else {
				SlaveOff[MB_N] &=~((uint32_t)1<<CurrSlave[MB_N]);
				if (++CurrQuery[MB_N]==prb(&Slave->QueryQt))
					Over = NextSlave(MB_N);
				Try[MB_N] = 0;
			}
		}
		/*if (Over)
			USART_Func(MB_N, LED);*/
	}
	StartTimer16(TD_MB_Idle+MB_N, Over?MB_M_Period[MB_N]:MB_M_Pause_Min);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~
void
MB_M_Timer_ISR(uint8_t MB_N)
{
	switch (MB_State[MB_N]) {
	case MB_Initi:
		StartTimer16(TD_MB_Cycle+MB_N, 0xFFFF);
		FormQuery(MB_N);
		MB_Transm(MB_N);
		break;
	case MB_Recept:
		MB_State[MB_N] = MB_Proc;
		MB_M_CtrlProc(MB_N);
		break;
	}
}

// ~~~~~~~~~~~~~~~~~~
void
MB_M_Tx(uint8_t MB_N)
{
	StartTimer16(TD_MB_Idle+MB_N, MB_M_Timeout[MB_N]);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
MB_SingleRun(uint8_t MB_N, uint8_t Addr, uint8_t Func, uint16_t StartAddr, uint16_t Qt, void *Buf)
{
	if (MB_SingleState[MB_N]!=MB_SingleIdle)
		return;

	MB_SingleQuery[MB_N].Addr				= Addr;
	MB_SingleQuery[MB_N].Func				= Func;
	MB_SingleQuery[MB_N].StartAddr	= StartAddr;
	MB_SingleQuery[MB_N].Qt					= Qt;
	MB_SingleQuery[MB_N].Buf				= Buf;

	MB_SingleState[MB_N] = MB_SingleStart;
	if (MB_State[MB_N]==MB_Proc && !LastProcSingle) {
		uint16_t T = (CurrSlave[MB_N]==0 ? MB_M_Period[MB_N] : MB_M_Timeout[MB_N]) - GetTimer16(TD_MB_Idle+MB_N);
		if (T<MB_M_Pause_Min)
			StartTimer16(TD_MB_Idle+MB_N, MB_M_Pause_Min-T);
		else {
			IntOff();
			USART_Func(MB_N, LED);
			IntOn();
			ResetTimer16(TD_MB_Idle+MB_N);
			FormSingle(MB_N);
			IntOff();
			MB_Transm(MB_N);
			IntOn();
		}
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
MB_Master_MaxCycle_Clear(uint8_t MB_N)
{
	MB_Master_MaxCycle[MB_N] = 0;
}

// ~~~~~~~~~~~~~~~~~~~~~
void
MB_M_Cycle(uint8_t MB_N)
{
	if (Timer16Stopp(TD_MB_SlaveOff+MB_N))
		SlavesOn(MB_N);

	if (Timer16Stopp(TD_MB_Idle+MB_N)){
		cli();
		if(MB_State[MB_N] == MB_Idle) MB_State[MB_N] = MB_NoReply;
		sei();
	}
	else return;
	
	switch (MB_State[MB_N]) {
	case MB_NoReply:
		FailSlave(MB_N);
		switch (MB_SingleState[MB_N]) {
		case MB_SingleIdle:
			FormQuery(MB_N);
			break;
		case MB_SingleStart:
			if (Try[MB_N]==0)
				FormSingle(MB_N);
			break;
		}
		StartTimer16(TD_MB_Idle+MB_N, 0xFFFF);	// для защиты от склеивания пакетов
		cli();
		MB_Transm(MB_N);
		sei();
		break;
	case MB_Proc:
		cli();
		USART_Func(MB_N, LED);
		sei();
		switch (MB_SingleState[MB_N]) {
		case MB_SingleIdle:
			FormQuery(MB_N);
			break;
		case MB_SingleStart:
			if (LastProcSingle)
				FormQuery(MB_N);
			else
				FormSingle(MB_N);
			break;
		}
		StartTimer16(TD_MB_Idle+MB_N, 0xFFFF);	// для защиты от склеивания пакетов
		cli();
		MB_Transm(MB_N);
		sei();
		break;
	}
}
