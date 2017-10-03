// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Relative time
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
enum {RT_Stop, RT_Run, RT_Pause};
uint8_t RelatTimeState;

uint8_t RelatHour, RelatMin, RelatSec;
uint8_t EEMEM EE_RelatHour, EE_RelatMin, EE_RelatSec;

static uint8_t Relat1s;

// ~~~~~~~~~~~~~~~~~~~
uint8_t
RelatTimeStarted(void)
{
	return RelatSec>0||RelatMin>0||RelatHour>0;
}

// ~~~~~~~~~~~~~~~~
void
RelatTimeInit(void)
{
	RelatHour = erb(&EE_RelatHour);
	RelatMin = erb(&EE_RelatMin);
	RelatSec = erb(&EE_RelatSec);
	RelatTimeState = RelatTimeStarted() ? RT_Pause : RT_Stop;
}

// ~~~~~~~~~~~~~~~~~
uint8_t
RelatTimeCycle(void)
{
	uint8_t OK = RelatTimeState==RT_Run && Relat1s!=Timer1s;
	if(OK) {
		Relat1s = Timer1s;
		if(++RelatSec==60) {
			RelatSec = 0;
			if(++RelatMin==60) {
				RelatMin = 0;
				++RelatHour;
			}
		}
	}
	return OK;
}

// ~~~~~~~~~~~~~~~~~
static void
ResetRelatTime(void)
{
	ewb(&EE_RelatHour, 0);
	ewb(&EE_RelatMin, 0);
	ewb(&EE_RelatSec, 0);
	RelatHour = RelatMin = RelatSec = 0;
}

// ~~~~~~~~~~~~~~~
void
RunRelatTime(void)
{
	Relat1s = Timer1s;
	RelatTimeState = RT_Run;
}

// ~~~~~~~~~~~~~~~~~~~
void
RestartRelatTime(void)
{
	ResetRelatTime();
	RunRelatTime();
}

// ~~~~~~~~~~~~~~~~~
uint8_t
StartRelatTime(void)
{
	uint8_t OK = 1;
	switch(RelatTimeState) {
	case RT_Pause:
		OK = 0;
		break;
	case RT_Stop:
		RunRelatTime();
		break;
	}
	return OK;
}

// ~~~~~~~~~~~~~~~~~
void
PauseRelatTime(void)
{
	if(RelatTimeState!=RT_Run)
		return;
	if(RelatTimeStarted()) {
		RelatTimeState = RT_Pause;
		ewb(&EE_RelatHour, RelatHour);
		ewb(&EE_RelatMin, RelatMin);
		ewb(&EE_RelatSec, RelatSec);
	}
	else
		RelatTimeState = RT_Stop;
}

// ~~~~~~~~~~~~~~~~
void
StopRelatTime(void)
{
	if(RelatTimeState!=RT_Stop) {
		ResetRelatTime();
		RelatTimeState = RT_Stop;
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint32_t
Time2Sec(uint8_t Hour, uint8_t Min, uint8_t Sec)
{
	return (uint32_t)3600*Hour+Min*60+Sec;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Reverce time
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t RevHour, RevMin, RevSec;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
RevTimeCycle(uint8_t F_R_H, uint8_t F_R_M)
{
	RevHour = F_R_H-RelatHour;
	if(RelatMin<=F_R_M)
		RevMin = F_R_M-RelatMin;
	else {
		RevHour--;
		RevMin = 60+F_R_M-RelatMin;
	}
	if(RelatSec>0) {
		if(RevMin>0)
			RevMin--;
		else {
			RevHour--;
			RevMin = 59;
		}
		RevSec = 60-RelatSec;
	}
	else
		RevSec = 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Scheduler
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef SchedMaxSz
	#define SchedMaxSz 20
#endif

#define SchedMinVal -30000

typedef struct {
	uint8_t Hour, Min;
	uint16_t Val;
} SchedPoint;
typedef struct {
	SchedPoint Point[SchedMaxSz];
	uint8_t Size;
} SchedDay;

typedef SchedDay SchedWeek[7];

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
GetSchedSuffiN(SchedDay *SD, uint8_t Hour, uint8_t Min)
{
	uint8_t Sz = erb(&SD->Size);
	uint8_t N = 0;
	while(N<Sz && Hour>erb(&(SD->Point+N)->Hour))
		N++;
	while(N<Sz && Hour==erb(&(SD->Point+N)->Hour) && Min>=erb(&(SD->Point+N)->Min))
		N++;
	return N;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
GetSchedEquiN(SchedDay *SD, uint8_t Hour, uint8_t Min)
{
	uint8_t N = GetSchedSuffiN(SD, Hour, Min);
	if(N) {
		SchedPoint *ptrSP = SD->Point+N-1;
		if(Hour!=erb(&ptrSP->Hour) || Min!=erb(&ptrSP->Min))
			N = 0;
	}
	return N;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
GetSchedDaySuffiN(SchedDay *SD)
{
	return GetSchedSuffiN(SD, Tm_Hour, Tm_Min);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
GetSchedWeekSuffiN(SchedDay *SW)
{
	return GetSchedDaySuffiN(SW+Tm_WDay);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
GetSchedDayEquiN(SchedDay *SD)
{
	return GetSchedEquiN(SD, Tm_Hour, Tm_Min);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
GetSchedWeekEquiN(SchedDay *SW)
{
	return GetSchedDayEquiN(SW+Tm_WDay);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
GetSchedHour(SchedDay *SD, uint8_t N)
{
	return erb(&(SD->Point+N)->Hour);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
GetSchedMin(SchedDay *SD, uint8_t N)
{
	return erb(&(SD->Point+N)->Min);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int16_t
GetSchedVal(SchedDay *SD, uint8_t N)
{
	return (int16_t)erw(&(SD->Point+N)->Val);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
PutValSchedDay(int16_t *Var, SchedDay *SD)
{
	uint8_t N = GetSchedEquiN(SD, Tm_Hour, Tm_Min);
	if(N)
		*Var = GetSchedVal(SD, N-1);
	return N;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
PutValSchedWeek(int16_t *Var, SchedDay *SW)
{
	return PutValSchedDay(Var, SW+Tm_WDay);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
MovSchedPoint(SchedDay *SD, uint8_t D_P, uint8_t S_P)
{
	SchedPoint *SP_D=SD->Point+D_P, *SP_S=SD->Point+S_P;
	for(uint8_t i=0; i<sizeof(SchedPoint); i++)
		ewb((uint8_t*)SP_D+i, erb((uint8_t*)SP_S+i));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
AddSchedPoint(SchedDay *SD, uint8_t Hour, uint8_t Min, int16_t Val)
{
	uint8_t OK = 1;
	uint8_t Sz = erb(&SD->Size);
	uint8_t N = GetSchedSuffiN(SD, Hour, Min);
	SchedPoint *ptrSP = SD->Point+N-1;
	if(N && Hour==erb(&ptrSP->Hour) && Min==erb(&ptrSP->Min))
		eww(&ptrSP->Val, Val);
	else if(Sz==SchedMaxSz)
		OK = 0;
	else {
		for(uint8_t i=Sz; i>N; i--)
			MovSchedPoint(SD, i, i-1);
		ptrSP++;
		ewb(&ptrSP->Hour, Hour);
		ewb(&ptrSP->Min, Min);
		eww(&ptrSP->Val, Val);
		ewb(&SD->Size, Sz+1);
	}
	return OK;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
DelSchedPoint(SchedDay *SD, uint8_t N)
{
	uint8_t OK = 1;
	uint8_t Sz = erb(&SD->Size);
	if(N<Sz) {
		Sz -= 1;
		for(uint8_t i=N; i<Sz; i++)
			MovSchedPoint(SD, i, i+1);
		ewb(&SD->Size, Sz);
	}
	else
		OK = 0;
	return OK;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
RepSchedDay(SchedDay *SW, uint8_t D_D, uint8_t S_D)
{
	SchedDay *SD_D=SW+D_D, *SD_S=SW+S_D;
	uint8_t Buf[sizeof(SchedDay)];
	erbl(Buf, (uint8_t*)SD_S, sizeof(SchedDay));
	ewbl(Buf, (uint8_t*)SD_D, sizeof(SchedDay));
	wdt_reset();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
CpySchedDay(SchedDay *SW, uint8_t D_D, uint8_t S_D)
{
	if(D_D>6) {
		uint8_t N;
		for(N=0; N<S_D; N++)
			RepSchedDay(SW, N, S_D);
		for(N=S_D+1; N<7; N++)
			RepSchedDay(SW, N, S_D);
	}
	else if(D_D!=S_D)
		RepSchedDay(SW, D_D, S_D);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint8_t
PutTechproc(int16_t *Var, SchedDay *TP)
{
	uint8_t OK = 0;
	if(RelatTimeState==RT_Run) {
		uint8_t Sz = erb(&TP->Size);
		if(Sz==0)
			RelatTimeState = RT_Stop;
		else {
			static int16_t VarBuf;
			uint8_t Tick = RelatTimeCycle();
			uint8_t Start = !RelatTimeStarted();
			if(Start)
				VarBuf = *Var;
			if(Tick||Start) {
				uint8_t N = GetSchedSuffiN(TP, RelatHour, RelatMin);
				uLineGraph LG;
				if(N<Sz) {
					LG.x2 = Time2Sec(GetSchedHour(TP, N), GetSchedMin(TP, N), 0);
					LG.y2 = GetSchedVal(TP, N);
				}
				else
					StopRelatTime();
				if(N>0) {
					uint8_t N0 = N-1;
					LG.x1 = Time2Sec(GetSchedHour(TP, N0), GetSchedMin(TP, N0), 0);
					LG.y1 = GetSchedVal(TP, N0);
				}
				else {
					LG.x1 = 0;
					LG.y1 = VarBuf;
				}
				if(LG.y1<SchedMinVal) {
					void TechprocOff(void);
					StopRelatTime();
					TechprocOff();
				}
				else if(N==Sz || LG.y2<SchedMinVal)
					*Var = LG.y1;
				else
					*Var = uLineFunc(&LG, Time2Sec(RelatHour, RelatMin, RelatSec));
				N = Sz-1;
				RevTimeCycle(GetSchedHour(TP, N), GetSchedMin(TP, N));
				OK = 1;
			}
		}
	}
	return OK;
}
