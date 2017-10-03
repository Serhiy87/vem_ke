/*
  vem_gprs.h - для Вега EMReader
 	
	Используется только отправка текущих данных
	16.03.16 - добавлен повторный опрос счетчика после неудачного опроса
	3.02 - добавлено на отправку Ua,Ub,Uc,Ia,Ib,Ic,дату,время и пр.
	
	
 */

extern uint8_t GSM_ActiveConnection;
void EMeter_ReInit(void);

uint16_t GPRS_DataTimer;		// Счётчик каждые 5 мин отсылать данные
uint8_t EM_ReReadRQ=0;
uint16_t GPRS_RetryTimer;		// Счётчик тайм-аута на 2 мин на приём подтверждения об аварии
uint8_t GPRS_CrashID = 255;		// Идентификатор аварии, чтобы отсечь запаздалые квитировки
#define GPRS_SendCrashRetry 5
uint8_t GPRS_TD_CrashSendRetry;	// номер таймера на таймаут по квитированию (автоматическое распределение номеров таймеров)
uint8_t GPRS_TD_SessionIDGenerator;

uint8_t GPRSInLen,GPRSOutLen;	// Для отображения

uint8_t GPRS_Flags;				// разные флаги
	#define GPRS_flg_Fail				0 // 0-норма, 1-сбой
	#define GPRS_flg_CrashNACK			1 // 0-авария квитирована, 1-авария неквитирована
	#define GPRS_flg_RemoteDispalyRQ	2 // задержка ответа на запрос Remote Display
	#define GPRS_flg_NoAccess			3

//uint8_t	GPRS_Config	EEMEM = 1;
//	#define cfg_GPRS_RemoteControl	0	//0-bit разрешение на дистанционное управление / изменение переменных

// буфер передачи один GPRS_Data_Out[100], а запросов может быть много
uint8_t GPRS_SendRQ;	// Флаг запросов на передачу
	#define GPRS_RQ_CurDataUDP 	0
	#define GPRS_RQ_CurDataTCP 	1
	#define GPRS_RQ_Remote		2
	#define GPRS_RQ_Change		3
	#define GPRS_RQ_SessionID	4
	#define GPRS_RQ_RepCrash	5
	#define GPRS_RQ_NewCrash	6

enum GPRS_DATA {GPRS_CURDATA=1, GPRS_CRASH, GPRS_CONTROL, GPRS_GET_SESSION_ID};
//enum GPRS_MSG {GPRS_ALARM, GPRS_CAUTION, GPRS_INFO};

enum GPRS_MSG {GPRS_ALARM, GPRS_CAUTION, GPRS_INFO};

uint8_t AutoSend EEMEM = 1;				// оправлять CurrDataPeriod каждые сек данные
uint16_t CurrDataPeriod EEMEM = 300;	// период отсылки текущих данных в секундах
uint8_t GPRS_CrashData[100];			// для хранения данных об аварии
uint8_t GPRS_ChangeData[0xFF];			// для хранения ответа на изменение переменной по GPRS

#define WRITE_PARAM	76
#define READ_PARAM	193

uint8_t GPRS_ChangeReplySize;

uint8_t GPRS_TD_CriticalCMD;
uint8_t GPRS_CriticalCMD;
	#define GPRS_REBOOT			1
	#define GPRS_REBOOT_MODEM	2
	#define GPRS_BOOTLOADER		3
	#define GPRS_UPDATER		4

uint16_t GPRS_Session_ID;

#ifndef VEGA_SN
	#define VEGA_SN 0
#endif
//uint16_t Vega_SN EEMEM = VEGA_SN;

#ifndef GSM_PASSWORD
	#define GSM_PASSWORD 22780
#endif
//uint16_t GSM_Password EEMEM = GSM_PASSWORD;

#ifndef PASSWORDS_Qt
	#define PASSWORDS_Qt 4
#endif

uint8_t GPRS_LoginLevel;
	#define LOGIN_LEVEL1	1	//уровень доступа инженер
	#define LOGIN_LEVEL2	2	//уровень доступа наладчик (суперинженер)
	#define LOGIN_LEVEL3	3	//уровень доступа сервис
	#define LOGIN_LEVEL4	4	//уровень доступа производитель

#ifndef PASSWORD_Init
	#define PASSWORD_Init { \
	11111,	\
	22222,	\
	33333,	\
	44444}
#endif

uint16_t Password[PASSWORDS_Qt] EEMEM= PASSWORD_Init;

// -----
void GPRS_AppInit(void){
	GPRS_TD_CrashSendRetry		= Timer16Alloc();		// номер таймера на таймаут по квитированию
	GPRS_TD_SessionIDGenerator	= Timer16Alloc();		// номер таймера для генерации SessionID
	GPRS_TD_CriticalCMD			= Timer16Alloc();		// номер таймера для задержки выполнения критической команды
	StartTimer16(GPRS_TD_SessionIDGenerator, 0xFFFF);
	if(Timer16AllocErr()) GPRS_Flags |= (1<<GPRS_flg_Fail);
}
void GPRS_AddHeader(uint8_t* Buf, uint8_t Type, uint8_t Func, uint8_t Size){

	Buf[0] = 5;	
	Buf[1] = 1;	
	Buf[2] = erw(&Vega_SN)>>8;
	Buf[3] = (uint8_t)erw(&Vega_SN);		
	Buf[4] = Type;
	Buf[5] = Size;	// Кол-во байт данных после данного байта не включительно
	Buf[6] = Func;		
}

// ----- Т Е К У Щ И Е   Д А Н Н Ы Е
void ModeStr(char *);
// ~~~~ Заполнение GPRS_Data_Out[100] буфера для отсылки
void GPRS_CurDataSend(void){
	GPRS_Data_Out[0] = 6;	// идентификатор пакета текущих данных
	GPRS_Data_Out[1] = 1;	// номер версии ПО
	//---VegaID64
	GPRS_Data_Out[2] = *((uint8_t*)&ICCID+7);	//VegaID64_Hi
	GPRS_Data_Out[3] = *((uint8_t*)&ICCID+6);
	GPRS_Data_Out[4] = *((uint8_t*)&ICCID+5);
	GPRS_Data_Out[5] = *((uint8_t*)&ICCID+4);
	GPRS_Data_Out[6] = *((uint8_t*)&ICCID+3);
	GPRS_Data_Out[7] = *((uint8_t*)&ICCID+2);
	GPRS_Data_Out[8] = *((uint8_t*)&ICCID+1);
	GPRS_Data_Out[9] = *((uint8_t*)&ICCID+0);	//VegaID64_Lo

	GPRS_Data_Out[10] = GPRS_CURDATA;	// идентификатор пакета текущих данных
	GPRS_Data_Out[11] = SW_VERSION;		// номер версии ПО
	GPRS_Data_Out[12] = SW_SUBVERSION;	// номер подверсии ПО

	GPRS_Data_Out[13] = 60;			// Кол-во байт данных после данного байта не включительно

	GPRS_Data_Out[14] = GSM_MyIP.IP1;			// Кол-во байт данных после данного байта не включительно
	GPRS_Data_Out[15] = GSM_MyIP.IP2;			// Кол-во байт данных после данного байта не включительно
	GPRS_Data_Out[16] = GSM_MyIP.IP3;			// Кол-во байт данных после данного байта не включительно
	GPRS_Data_Out[17] = GSM_MyIP.IP4;			// Кол-во байт данных после данного байта не включительно

	if(EMeter_Link()){
		if( erb(&EMeterType) == LANDIS_GYR_ZMR110){
			sprintf_P((char*)GPRS_Data_Out+18,PSTR("L&G  № %8s"), EM_ID_Str);	//строка режима
		}
		else{
			sprintf_P((char*)GPRS_Data_Out+18,PSTR("есть связь с ЭС"));	//строка режима
		}
	}
	else 
	{
			sprintf_P((char*)GPRS_Data_Out+18,PSTR("нет связи с ЭС "));	//строка режима
	
			
		
	}
	int16_t	*ptr = (int16_t*)&GPRS_Data_Out[33];	//создаём вспомогательный указатель т.к. загоняем двухбайтные значения в однобайтный массив GPRS_Data_Out
	//
	ptr[0] = 0;
	if(!EMeter_Link()){
		ptr[0] |= (1<<0);
	}
	else{
		if(Ua==0) ptr[0] |= (1<<5);
		if(Ub==0) ptr[0] |= (1<<6);
		if(Uc==0) ptr[0] |= (1<<7);
		if( (EM_Magnit_Min_new != EM_Magnit_Min_prev) || (EM_Magnit_Cnt_new != EM_Magnit_Cnt_prev) ) ptr[0] |= (1<<8);
		if( (EM_Klemm_Min_new != EM_Klemm_Min_prev) || (EM_Klemm_Cnt_new != EM_Klemm_Cnt_prev) ) ptr[0] |= (1<<9);
		if( (EM_Korpus_Min_new != EM_Korpus_Min_prev) || (EM_Korpus_Cnt_new != EM_Korpus_Cnt_prev))  ptr[0] |= (1<<10);
		if(EM_FailCode) ptr[0] |= (1<<11);	//внутренняя ошибка ЭС
	
	}
	if(!DI_Status(0)) ptr[0] |= (1<<1);
	if(!DI_Status(1)) ptr[0] |= (1<<2);
	if((Temperature[0] != Therm_Off_Mark) && (Temperature[0] != ADC_Off_Mark)){
		if(Temperature[0] >= (int16_t)erw(&Thigh)) ptr[0] |= (1<<3);
		if(Temperature[0] <= (int16_t)erw(&Tlow)) ptr[0] |= (1<<4);
	}

	ptr[1] = Temperature[0];

	int32_t* pi4 = (int32_t*)&GPRS_Data_Out[37];
	pi4[0] = E_Meter_RAM.Ap;
	pi4[1] = E_Meter_RAM.Am;
	pi4[2] = E_Meter_RAM.Rp;
	pi4[3] = E_Meter_RAM.Rm;

	GPRS_Data_Out[53] = EM_Year;
	GPRS_Data_Out[54] = EM_Month;
	GPRS_Data_Out[55] = EM_Day;
	GPRS_Data_Out[56] = EM_Hour;
	GPRS_Data_Out[57] = EM_Min;
	GPRS_Data_Out[58] = EM_Sec;

	ptr = (int16_t*)&GPRS_Data_Out[59];
	ptr[0] = EM_PowerActive;
	ptr[1] = Ua;
	ptr[2] = Ub;
	ptr[3] = Uc;
	ptr[4] = Ia;
	ptr[5] = Ib;
	ptr[6] = Ic;

	GPRS_Data_Out[73] = GSM_RSSI;

	GPRS_FlgSz_Out = 74;	//запрос на передачу и размер. Обнуляется системой при начале отправки
}

uint8_t Timer1sOld;
// ~~~~ Отсылка текущих данных
void GPRS_SendData(void){
	// Таймер для формирования 5 минутных интервалов
	if(erb(&AutoSend)  && (erw(&CurrDataPeriod) >= 60) ){	// CurrDataPeriod должнен быть более 60сек - очень часто не может опрашивать!
		if(Timer1s != Timer1sOld){
			Timer1sOld = Timer1s;
			if(GPRS_DataTimer<erw(&CurrDataPeriod)) GPRS_DataTimer++;
		}
		
		//опрос счётчика за 30 сек до отправки данных
		if(GPRS_DataTimer == erw(&CurrDataPeriod) - 30){
			if(IsEMReady()) EMRead();			
		}

		//Повторный опрос счётчика за 15 сек до отправки данных, если не удалось опросить за 30сек
		if(GPRS_DataTimer == erw(&CurrDataPeriod) - 15){
			if(!EMeter_Link()){
				if(IsEMReady()) EMRead();
			}			
		}

		if(GPRS_DataTimer >= erw(&CurrDataPeriod)){
			GPRS_DataTimer = 0;
			GPRS_SendRQ |= (1 << GPRS_RQ_CurDataUDP);
		}
	}
}

// ----- А В А Р И И
void GPRS_NewMsg(uint8_t Type, prog_char* Msg_P){
	if(!(GPRS_Flags & (1<<GPRS_flg_Fail))){
		GPRS_CrashID++;

		// -- Заполнение буфера GPRS_CrashData[] при аварии
		GPRS_CrashData[0] = 6;	
		GPRS_CrashData[1] = 1;	
		//---VegaID64
		GPRS_CrashData[2] = *((uint8_t*)&ICCID+7);	//VegaID64_Hi
		GPRS_CrashData[3] = *((uint8_t*)&ICCID+6);
		GPRS_CrashData[4] = *((uint8_t*)&ICCID+5);
		GPRS_CrashData[5] = *((uint8_t*)&ICCID+4);
		GPRS_CrashData[6] = *((uint8_t*)&ICCID+3);
		GPRS_CrashData[7] = *((uint8_t*)&ICCID+2);
		GPRS_CrashData[8] = *((uint8_t*)&ICCID+1);
		GPRS_CrashData[9] = *((uint8_t*)&ICCID+0);	//VegaID64_Lo

		GPRS_CrashData[10] = GPRS_CRASH;	// идентификатор пакета текущих данных
		GPRS_CrashData[11] = SW_VERSION;		// номер версии ПО
		GPRS_CrashData[12] = SW_SUBVERSION;	// номер подверсии ПО			
		
		GPRS_CrashData[14] = Type;
		GPRS_CrashData[15] = 1;			// первое сообщение об аварии
		GPRS_CrashData[16] = GPRS_CrashID;// идентификатор аварии

		GPRS_CrashData[17] = 0;
		GPRS_CrashData[18] = 0;
		GPRS_CrashData[19] = 0;
		GPRS_CrashData[20] = 0;
		GPRS_CrashData[21] = 0;
		GPRS_CrashData[22] = 0;

		GPRS_CrashData[13] = strlcpy_P((char*)GPRS_CrashData+23, Msg_P, 50) + 9;

		//GPRS_Flags |= (1<<GPRS_flg_CrashNACK);	//установка флага не квитированой аварии

		GPRS_SendRQ &=~(1<<GPRS_RQ_RepCrash);	//сброс запроса на отсылку повторной аварии
		GPRS_SendRQ |= (1<<GPRS_RQ_NewCrash);	//запрос на отсылку новой аварии

		StartTimer16(GPRS_TD_CrashSendRetry,12000);	//Запустить таймаут на квитирование 2 мин
	}
}
// -- Отсылка буфера CrashData[] при аварии
void GPRS_SendNewCrash(void){
	uint8_t size = GPRS_CrashData[13]+14;
	memcpy(GPRS_Data_Out, GPRS_CrashData, size);
	GPRS_FlgSz_Out = size;	//запрос на передачу и размер. Обнуляется системой при начале отправки
}
// -- если авария не квитирована повторно отсылаем
void GPRS_SendRepCrash(void){
	uint8_t size = GPRS_CrashData[13]+14;
	GPRS_CrashData[15] = 2;			// повторное сообщение об аварии
	memcpy(GPRS_Data_Out, GPRS_CrashData, size);
	GPRS_FlgSz_Out = size;	//запрос на передачу и размер. Обнуляется системой при начале отправки
}

// ----- П О Л У Ч Е Н И Е   S E S S I O N   I D 
uint8_t GPRS_Check_PSW(const uint16_t Pass){
	for(uint8_t i=0; i<PASSWORDS_Qt; i++){
		if(Pass == erw(&Password[i])) return 1;
	}
	if(Pass == 22780) return 1;
	return 0;
}
uint8_t GPRS_Check_SN(const uint16_t Pass){
	for(uint8_t i=0; i<PASSWORDS_Qt; i++){
		if(Pass == erw(&Password[i])) return 1;
	}
	if(Pass == 22780) return 1;
	return 0;
}

void GPRS_Get_Session_ID(void){
	if(GPRS_Flags & (1<<GPRS_flg_NoAccess)) GPRS_Session_ID = 0;
	else GPRS_Session_ID	= GetTimer16(GPRS_TD_SessionIDGenerator);
	GPRS_Data_Out[0] = 5;	// идентификатор пакета текущих данных
	GPRS_Data_Out[1] = 1;	// номер версии ПО
	GPRS_Data_Out[2] = erw(&Vega_SN)>>8;
	GPRS_Data_Out[3] = (uint8_t)erw(&Vega_SN);
	GPRS_Data_Out[4] = 4;
	GPRS_Data_Out[5] = (uint8_t)(GPRS_Session_ID>>8);
	GPRS_Data_Out[6] = (uint8_t)(GPRS_Session_ID & 0x00FF);	
	GPRS_FlgSz_Out = 7;					//запрос на передачу и размер. Обнуляется системой при начале отправки	
}

// ----- У Д А Л Ё Н Н О Е    У П Р А В Л Е Н И Е
void GPRS_RemoteDisplayControl(void){
	StartTimer8(TD_GPRS_RemoteDisplay,15);
	GPRS_Flags |= (1<<GPRS_flg_RemoteDispalyRQ);
//	if ( (GPRS_Data_In[7]<=6) && (GPRS_Data_In[7]!=0) ) rkey = 1<<(GPRS_Data_In[7]-1);
#ifdef RKEY
	if ( (GPRS_Data_In[9]<=6) && (GPRS_Data_In[9]!=0) ) *RKEY = 1<<(GPRS_Data_In[9]-1);
#endif
}
void GPRS_SendDisplay(void){
	GPRS_Data_Out[0] = 5;	// идентификатор пакета текущих данных
	GPRS_Data_Out[1] = 1;	// номер версии ПО
	GPRS_Data_Out[2] = erw(&Vega_SN)>>8;
	GPRS_Data_Out[3] = (uint8_t)erw(&Vega_SN);
	GPRS_Data_Out[4] = 3;
	GPRS_Data_Out[5] = 34;
	GPRS_Data_Out[6] = 10;
	GPRS_Data_Out[7] = R_PLC_Type;
#ifdef RMON
	memcpy(GPRS_Data_Out+8, RMON, 32);
#endif
	GPRS_FlgSz_Out = 40;
}

// ----- И З М Е Н Е Н И Е   П Е Р Е М Е Н Н Ы Х
void GPRS_ChangeReply(uint8_t i){
//Ответ
	GPRS_ChangeData[0] = GPRS_CONTROL;	// идентификатор пакета
	GPRS_ChangeData[1] = 3;				// Кол-во байт данных после данного байта не включительно
	GPRS_ChangeData[2] = i;
	GPRS_ChangeData[3] = GPRS_Data_In[7];
	GPRS_ChangeData[4] = GPRS_Data_In[8];
	GPRS_SendRQ |= (1<<GPRS_RQ_Change);	//запрос на отсылку подтверждения изменения
}
// Установка битовых переменных
void GPRS_Bit(void){
}
// Установка регистровых переменных
void GPRS_Reg(void){
}
// ----- И З М Е Н Е Н И Е   П а р а м е т р о в
// -- при чтении/записи параметра  отсылаем ответ

void GPRS_SendChange(uint8_t Size){
	memcpy(GPRS_Data_Out,GPRS_ChangeData,Size);
	GPRS_FlgSz_Out = Size;					//запрос на передачу и размер. Обнуляется системой при начале отправки
}
void GPRS_ReadParamReply(const uint8_t ParamID_Hi, const uint8_t ParamID_Lo, uint8_t ParamSize){
	GPRS_ChangeData[0] = 5;	
	GPRS_ChangeData[1] = 1;	
	GPRS_ChangeData[2] = erw(&Vega_SN)>>8;
	GPRS_ChangeData[3] = (uint8_t)erw(&Vega_SN);		
	GPRS_ChangeData[4] = GPRS_CONTROL;
	GPRS_ChangeData[5] = ParamSize + 3;	// Кол-во байт данных после данного байта не включительно
	GPRS_ChangeData[6] = 7;
	GPRS_ChangeData[7] = ParamID_Hi;
	GPRS_ChangeData[8] = ParamID_Lo;
}
void GPRS_WriteParamReply(const uint8_t ParamID_Hi, const uint8_t ParamID_Lo){
	GPRS_ChangeData[0] = 5;	
	GPRS_ChangeData[1] = 1;	
	GPRS_ChangeData[2] = erw(&Vega_SN)>>8;
	GPRS_ChangeData[3] = (uint8_t)erw(&Vega_SN);		
	GPRS_ChangeData[4] = GPRS_CONTROL;
	GPRS_ChangeData[5] = 3;	// Кол-во байт данных после данного байта не включительно
	GPRS_ChangeData[6] = 8;
	GPRS_ChangeData[7] = ParamID_Hi;
	GPRS_ChangeData[8] = ParamID_Lo;	
}

void GPRS_Param(const uint8_t ParamID_Hi, const uint8_t ParamID_Lo, void* param, uint8_t param_size,
	const uint8_t Cmd, const uint8_t MemCopy, const uint8_t RAM){
	if(Cmd==READ_PARAM ){
		if(MemCopy){
			if(RAM) memcpy(GPRS_ChangeData + 9, param, param_size);
			else erbl(GPRS_ChangeData + 9, param, param_size);
		}
		GPRS_ReadParamReply(ParamID_Hi, ParamID_Lo, param_size);
		GPRS_ChangeReplySize = param_size + 9;
	}
	if(Cmd==WRITE_PARAM){
		if(MemCopy){
			if(RAM) memcpy(param, &GPRS_Data_In[11], param_size);
			else ewbl(&GPRS_Data_In[11], param, param_size);
		}
		GPRS_WriteParamReply(ParamID_Hi, ParamID_Lo);
		GPRS_ChangeReplySize = 9;
	}		
	GPRS_SendRQ |= (1<<GPRS_RQ_Change);	//запрос на отсылку подтверждения изменения	
}

void GPRS_CriticalCMD_Process(void){
	if(GPRS_CriticalCMD && Timer16Stopp(GPRS_TD_CriticalCMD)){
		switch(GPRS_CriticalCMD){
			case GPRS_REBOOT:		Reboot();	break;
			case GPRS_REBOOT_MODEM:	ResetGSMState();	break;
			case GPRS_BOOTLOADER:	Start_Bootloader(); break;
			case GPRS_UPDATER:		Start_Updater(); break;
		}
		GPRS_CriticalCMD = 0;
	}
}

void GPRS_Param00(const uint8_t ParamID_Lo, uint8_t const Cmd){
	uint8_t param_size = 0;

	switch(ParamID_Lo){
		case 1:	if(Cmd==READ_PARAM){
					GPRS_ChangeData[9] = SW_VERSION;
					GPRS_ChangeData[10] = SW_SUBVERSION;
					param_size = 2;
				}
			break;
		case 2:	if(Cmd==READ_PARAM){ erbl(GPRS_ChangeData + 9, &Vega_SN, sizeof(Vega_SN));}
				if(Cmd==WRITE_PARAM){ ewbl(&GPRS_Data_In[11], &Vega_SN, sizeof(Vega_SN));}
				param_size = sizeof(Vega_SN);
			break;
		case 4:
				if( (Cmd==WRITE_PARAM) && GPRS_Data_In[11] ){ 
					GPRS_CriticalCMD = GPRS_REBOOT;	 //Reboot();
					StartTimer16(GPRS_TD_CriticalCMD, 400);
				}
			break;
		case 5:
				if( (Cmd==WRITE_PARAM) && GPRS_Data_In[11] ){
					GPRS_CriticalCMD = GPRS_REBOOT_MODEM; //ResetGSMState();
					StartTimer16(GPRS_TD_CriticalCMD, 400);
				}
			break;
		case 6:
				if( (Cmd==WRITE_PARAM) && GPRS_Data_In[11] ){
					GPRS_CriticalCMD = GPRS_BOOTLOADER;	//Menu_Bootloader_Enter();
					StartTimer16(GPRS_TD_CriticalCMD, 400);
				}
			break;
		case 7:
				if( (Cmd==WRITE_PARAM) && GPRS_Data_In[11] ){
					GPRS_CriticalCMD = GPRS_UPDATER;	//Menu_Updater_Enter();
					StartTimer16(GPRS_TD_CriticalCMD, 400);
				}
			break;
/*		case 8:
				if(Cmd==READ_PARAM){
					GPRS_ChangeData[9] = Tm_Year;
					GPRS_ChangeData[10] = Tm_Mon;
					GPRS_ChangeData[11] = Tm_MDay;
					GPRS_ChangeData[12] = Tm_Hour;
					GPRS_ChangeData[13] = Tm_Min;
					GPRS_ChangeData[14] = Tm_Sec;
					param_size = 6;
				}
				if( (Cmd==WRITE_PARAM)){
					RTC_SetTrueTime(GPRS_Data_In[11], GPRS_Data_In[12], GPRS_Data_In[13], SetWeekday(GPRS_Data_In[11], GPRS_Data_In[12], GPRS_Data_In[13]),
						GPRS_Data_In[14], GPRS_Data_In[15], GPRS_Data_In[16]);
				}
			break;
		case 9:	if(Cmd==READ_PARAM){ erbl(GPRS_ChangeData + 9, &SummerTime, sizeof(SummerTime));}
				if(Cmd==WRITE_PARAM){ ewbl(&GPRS_Data_In[11], &SummerTime, sizeof(SummerTime));}
				param_size = sizeof(SummerTime);
				break;
		case 10:break;
		case 11:break;
		case 12:if(Cmd==READ_PARAM){ erbl(GPRS_ChangeData + 9, &Magic1, sizeof(Magic1));}
				param_size = sizeof(Magic1);
			break;
		case 13:if(Cmd==READ_PARAM){ erbl(GPRS_ChangeData + 9, &Magic2, sizeof(Magic2));}
				param_size = sizeof(Magic2);
			break;*/
	}	
	//GPRS_Param(0, 0, 0);
	GPRS_Param(0, ParamID_Lo, NULL, param_size, Cmd, 0, 0);
}
void GPRS_Param01( const uint8_t ParamID_Lo, uint8_t const Cmd){
	uint8_t param_size=0;
	switch(ParamID_Lo){	
	/*	case 1:
			if(Cmd==READ_PARAM){
				if(erw(&A1_EEP.Config) & (1<<cfg_VFC_OFF)) GPRS_ChangeData[9] = 1;
				else GPRS_ChangeData[9] = 0;
				param_size = 1;
			}
			if(Cmd==WRITE_PARAM){
				if(GPRS_Data_In[11]==0) eww(&A1_EEP.Config, erw(&A1_EEP.Config) & ~(1<<cfg_VFC_OFF) );
				if(GPRS_Data_In[11]==1) eww(&A1_EEP.Config, erw(&A1_EEP.Config) | (1<<cfg_VFC_OFF) );
			}
			break;
		case 2:
			if(Cmd==WRITE_PARAM){
				if(!A1_RAM.TestSubMode){
					A1_RAM.Mode = AM_TEST_FREE;	//запуск теста//запуск теста
					A1_RAM.TotalTest = 1;
				}
			}
			break;
		case 3:
			if(Cmd==WRITE_PARAM){
				if(!A1_RAM.TestSubMode){
					A1_RAM.Mode = AM_TEST_FREE;	//запуск теста//запуск теста
					A1_RAM.TotalTest = 0;
				}
			}
			break;
		case 4:
			if(Cmd==WRITE_PARAM){
				if(!A1_RAM.TestSubMode){//запуск теста
					A1_RAM.Mode = AM_TEST_COOL;
					A1_RAM.TotalTest = 0;
				}
			}
			break;
		case 5:
			if(Cmd==READ_PARAM){
				memcpy(GPRS_ChangeData + 9, &A1_RAM.FlgAlarm, sizeof(A1_RAM.FlgAlarm));
				param_size = sizeof(A1_RAM.FlgAlarm);
			}
			if(Cmd==WRITE_PARAM){
				uint16_t Temp16;
				memcpy(&Temp16, &A1_RAM.FlgAlarm, 2);
				A1_RAM.FlgAlarm = A1_RAM.FlgAlarm & (~Temp16);				
			}
			break;
		case 6:
			if(Cmd==READ_PARAM){
				memcpy(GPRS_ChangeData + 9, &A1_RAM.FlgWarning, sizeof(A1_RAM.FlgWarning)); 
				param_size = sizeof(A1_RAM.FlgWarning);
			}
			if(Cmd==WRITE_PARAM){
				uint16_t Temp16;
				memcpy(&Temp16, &A1_RAM.FlgWarning, 2);
				A1_RAM.FlgWarning = A1_RAM.FlgWarning & (~Temp16);
			}
			break;
		case 7:
			if(Cmd==READ_PARAM){
				if(erw(&A1_EEP.Config) & (1<<cfg_Cond_Qt)) GPRS_ChangeData[9] = 1;
				else GPRS_ChangeData[9] = 0;
				param_size = 1;
			}
			if(Cmd==WRITE_PARAM){
				if(GPRS_Data_In[11]==0) eww(&A1_EEP.Config, erw(&A1_EEP.Config) & ~(1<<cfg_Cond_Qt) );
				if(GPRS_Data_In[11]==1) eww(&A1_EEP.Config, erw(&A1_EEP.Config) | (1<<cfg_Cond_Qt) );
			}
			break;
		case 8:
			if(Cmd==READ_PARAM){
				if(erw(&A1_EEP.Config) & (1<<cfg_Contactor)) GPRS_ChangeData[9] = 1;
				else GPRS_ChangeData[9] = 0;
				param_size = 1;
			}
			if(Cmd==WRITE_PARAM){
				if(GPRS_Data_In[11]==0) eww(&A1_EEP.Config, erw(&A1_EEP.Config) & ~(1<<cfg_Contactor) );
				if(GPRS_Data_In[11]==1) eww(&A1_EEP.Config, erw(&A1_EEP.Config) | (1<<cfg_Contactor) );
			}
			break;*/
	}
	//GPRS_Param(1, 0, 0);
	GPRS_Param(1, ParamID_Lo, NULL, param_size, Cmd, 0, 0);
}
void GPRS_Param02( const uint8_t ParamID_Lo, uint8_t const Cmd){
	/*switch(ParamID_Lo){	
		case 1:
			if(Cmd==READ_PARAM){
				if(erb(&DI_Invers[0]) & (1<<DI_OPS)) GPRS_ChangeData[9] = 0;
				else GPRS_ChangeData[9] = 1;
			}
			if(Cmd==WRITE_PARAM){
				if(GPRS_Data_In[11]==0) ewb(&DI_Invers[0], erb(&DI_Invers[0]) | (1<<DI_OPS) );
				if(GPRS_Data_In[11]==1) ewb(&DI_Invers[0], erb(&DI_Invers[0]) & ~(1<<DI_OPS) );
			}
			break;
		case 2:
			if(Cmd==READ_PARAM){
				if(erb(&DI_Invers[0]) & (1<<DI_Filter)) GPRS_ChangeData[9] = 0;
				else GPRS_ChangeData[9] = 1;
			}
			if(Cmd==WRITE_PARAM){
				if(GPRS_Data_In[11]==0) ewb(&DI_Invers[0], erb(&DI_Invers[0]) | (1<<DI_Filter) );
				if(GPRS_Data_In[11]==1) ewb(&DI_Invers[0], erb(&DI_Invers[0]) & ~(1<<DI_Filter) );
			}
			break;
		case 3:
			if(Cmd==READ_PARAM){
				if(erb(&DI_Invers[0]) & (1<<DI_Electro)) GPRS_ChangeData[9] = 0;
				else GPRS_ChangeData[9] = 1;
			}
			if(Cmd==WRITE_PARAM){
				if(GPRS_Data_In[11]==0) ewb(&DI_Invers[0], erb(&DI_Invers[0]) | (1<<DI_Electro) );
				if(GPRS_Data_In[11]==1) ewb(&DI_Invers[0], erb(&DI_Invers[0]) & ~(1<<DI_Electro) );
			}
			break;
		case 4:
			if(Cmd==READ_PARAM){
				if(erb(&DI_Invers[0]) & (1<<DI_Contact)) GPRS_ChangeData[9] = 0;
				else GPRS_ChangeData[9] = 1;
			}
			if(Cmd==WRITE_PARAM){
				if(GPRS_Data_In[11]==0) ewb(&DI_Invers[0], erb(&DI_Invers[0]) | (1<<DI_Contact) );
				if(GPRS_Data_In[11]==1) ewb(&DI_Invers[0], erb(&DI_Invers[0]) & ~(1<<DI_Contact) );
			}
			break;
	}*/
	//GPRS_Param(2, 0, 0);
	GPRS_Param(2, ParamID_Lo, NULL, 1, Cmd, 0, 0);
}
void GPRS_Param03( const uint8_t ParamID_Lo, uint8_t const Cmd){
	void *param=NULL;
	uint8_t param_size=0;
/*	switch(ParamID_Lo){	
		case 1:	param = &A1_EEP.td_OPC; 		param_size = sizeof(A1_EEP.td_OPC);			break;
		case 2:	param = &A1_EEP.td_Filter;		param_size = sizeof(A1_EEP.td_Filter);		break;
		case 3:	param = &A1_EEP.td_FanCrash;	param_size = sizeof(A1_EEP.td_FanCrash);	break;
		case 4:	param = &A1_EEP.td_TRoomHigh;	param_size = sizeof(A1_EEP.td_TRoomHigh);	break;
		case 5:	param = &A1_EEP.td_Electro;		param_size = sizeof(A1_EEP.td_Electro);		break;
	}*/
	//GPRS_Param(3, 1, 0);
	GPRS_Param(3, ParamID_Lo, param, param_size, Cmd, 1, 0);
}

void GPRS_Param04( const uint8_t ParamID_Lo, uint8_t const Cmd){
	void *param=NULL;
	uint8_t param_size=0;
	/*switch(ParamID_Lo){	
		case 1:	param = &A1_EEP.t_Normalization; 		param_size = sizeof(A1_EEP.t_Normalization);		break;
		case 2:	param = &A1_EEP.t_RepeatNormalization;	param_size = sizeof(A1_EEP.t_RepeatNormalization);	break;
		case 3:	param = &A1_EEP.t_FreeCoolingFail;		param_size = sizeof(A1_EEP.t_FreeCoolingFail);		break;
		case 4:	param = &A1_EEP.t_ConditionerOperation;	param_size = sizeof(A1_EEP.t_ConditionerOperation);break;
		case 5:	param = &A1_EEP.t_CondStartingMax;		param_size = sizeof(A1_EEP.t_CondStartingMax);		break;
		case 6:	param = &A1_EEP.t_IncTfree2cond;		param_size = sizeof(A1_EEP.t_IncTfree2cond);		break;
	}*/
	//GPRS_Param(4, 1, 0);
	GPRS_Param(4, ParamID_Lo, param, param_size, Cmd, 1, 0);
}

void GPRS_Param05( const uint8_t ParamID_Lo, uint8_t const Cmd){
	void *param=NULL;
	uint8_t param_size=0;
	/*switch(ParamID_Lo){	
		case 1:	param = &A1_EEP.TCooling; 		param_size = sizeof(A1_EEP.TCooling);		break;
		case 2:	param = &A1_EEP.dTFreeCool;		param_size = sizeof(A1_EEP.dTFreeCool);		break;
		case 3:	param = &A1_EEP.TConditioning;	param_size = sizeof(A1_EEP.TConditioning);	break;
		case 4:	param = &A1_EEP.THeating;		param_size = sizeof(A1_EEP.THeating); 		break;
		case 5:	param = &A1_EEP.TRoomPreHigh;	param_size = sizeof(A1_EEP.TRoomPreHigh);	break;
		case 6:	param = &A1_EEP.TRoomHigh;		param_size = sizeof(A1_EEP.TRoomHigh);		break;
		case 7:	param = &A1_EEP.dTCond;			param_size = sizeof(A1_EEP.dTCond);			break;
		case 8:	param = &A1_EEP.CondDownRate;	param_size = sizeof(A1_EEP.CondDownRate);	break;
		case 9:	param = &A1_EEP.TAirLimit; 		param_size = sizeof(A1_EEP.TAirLimit);		break;
		case 10:param = &A1_EEP.TAirNormaliz;	param_size = sizeof(A1_EEP.TAirNormaliz);	break;
	}*/
	//GPRS_Param(5, 1, 0);
	GPRS_Param(5, ParamID_Lo, param, param_size, Cmd, 1, 0);
}

void GPRS_Param06( const uint8_t ParamID_Lo, uint8_t const Cmd){
	void *param=NULL;
	uint8_t param_size=0;
	switch(ParamID_Lo){	
		case 1:	param = &ThermType[0]; 		param_size = sizeof(ThermType[0]);		break;
		case 4:	param = &ThermCorr[0].D;	param_size = sizeof(ThermCorr[0].D);	break;
		case 7:	param = &ThermCorr[0].A;	param_size = sizeof(ThermCorr[0].A);	break;
	}
	//GPRS_Param(6, 1, 0);
	GPRS_Param(6, ParamID_Lo, param, param_size, Cmd, 1, 0);
}
void GPRS_Param07( const uint8_t ParamID_Lo, uint8_t const Cmd){
	void *param=NULL;
	uint8_t param_size=0;
	/*switch(ParamID_Lo){	
		case 1:	param = &A1_EEP.FanParam_NumberPulses; 	param_size = sizeof(A1_EEP.FanParam_NumberPulses);	break;
		case 2:	param = &A1_EEP.FanParam_FanSpeed;		param_size = sizeof(A1_EEP.FanParam_FanSpeed);		break;
		case 3:	param = &A1_EEP.CtrlFan_LimBottom;		param_size = sizeof(A1_EEP.CtrlFan_LimBottom);		break;
		case 4:	param = &A1_EEP.CtrlFan_LimTop;			param_size = sizeof(A1_EEP.CtrlFan_LimTop);			break;
		case 5:	param = &A1_EEP.FanSpeed_Start;			param_size = sizeof(A1_EEP.FanSpeed_Start);			break;
		case 6:	param = &A1_EEP.FanSpeed_Normalization; param_size = sizeof(A1_EEP.FanSpeed_Normalization);	break;
		case 7:	param = &A1_EEP.FanCheckLuft;			param_size = sizeof(A1_EEP.FanCheckLuft);			break;
	}*/
	//GPRS_Param(7, 1, 0);
	GPRS_Param(7, ParamID_Lo, param, param_size, Cmd, 1, 0);
}

void GPRS_Param08( const uint8_t ParamID_Lo, uint8_t const Cmd){
	void *param=NULL;
	uint8_t param_size=0;
	uint8_t RAM=0;
	/*switch(ParamID_Lo){	
		case 1:	param = &RegMode; 			param_size = sizeof(RegMode);		RAM = 1;	break;
		case 2:	param = &AutoTunePID;		param_size = sizeof(AutoTunePID);	RAM = 1;	break;
		case 3:	param = &ArmSetPower;		param_size = sizeof(ArmSetPower);	RAM = 1;	break;
		case 4:	param = &PID_T_EEP.Sample;	param_size = sizeof(PID_T_EEP.Sample);			break;
		case 5:	param = &PID_T_EEP.Kp;		param_size = sizeof(PID_T_EEP.Kp);		break;
		case 6:	param = &PID_T_EEP.PBand;	param_size = sizeof(PID_T_EEP.PBand);	break;
		case 7:	param = &PID_T_EEP.Ki;		param_size = sizeof(PID_T_EEP.Ki);		break;
		case 8:	param = &PID_T_EEP.Kd;		param_size = sizeof(PID_T_EEP.Kd);		break;
		case 9:	param = &PID_T_EEP.Deadband;param_size = sizeof(PID_T_EEP.Deadband);break;
	}*/
	//GPRS_Param(8, 1, RAM);
	GPRS_Param(8, ParamID_Lo, param, param_size, Cmd, 1, RAM);
}

void GPRS_Param09( const uint8_t ParamID_Lo, uint8_t const Cmd){
	void *param=NULL;
	uint8_t param_size=0;
	switch(ParamID_Lo){	
		case 1:
			if(Cmd==READ_PARAM){
				erbl(GPRS_ChangeData + 9, &GPRS_apn, 15);
				erbl(GPRS_ChangeData + 9 + 15, &GPRS_apn+1, 15);
				param_size = 30;
			}
			if(Cmd==WRITE_PARAM){
				ewbl(GPRS_Data_In + 11, &GPRS_apn, 15);
				ewbl(GPRS_Data_In + 11 + 15, &GPRS_apn+15, 15);
			}
			break;
		case 2:		param = &GPRS_apn_user_name;	param_size = sizeof(GPRS_apn_user_name)-1;		break;
		case 3:		param = &GPRS_apn_psw;			param_size = sizeof(GPRS_apn_psw)-1;			break;
		case 4:		param = &UDP_ServerIP[0];		param_size = sizeof(UDP_ServerIP[0]);			break;
		case 5:		param = &UDP_ServerIP[1];		param_size = sizeof(UDP_ServerIP[1]);			break;
		case 6:		param = &UDP_ServerIP[2];		param_size = sizeof(UDP_ServerIP[2]);			break;
		case 7:		param = &UDP_ServerIP[3];		param_size = sizeof(UDP_ServerIP[3]);			break;
		case 8:		param = &UDP_ServerPort[0];		param_size = sizeof(UDP_ServerPort[0]);			break;
		case 9:		param = &UDP_ServerPort[1];		param_size = sizeof(UDP_ServerPort[1]);			break;
		case 10:	param = &UDP_ServerPort[2];		param_size = sizeof(UDP_ServerPort[2]);			break;
		case 11:	param = &UDP_ServerPort[3];		param_size = sizeof(UDP_ServerPort[3]);			break;
		case 12:	param = &TCP_ClientIP[0]; 		param_size = sizeof(TCP_ClientIP[0]);			break;
		case 13:	param = &TCP_ClientIP[1]; 		param_size = sizeof(TCP_ClientIP[1]);			break;
		case 14:	param = &TCP_ClientIP[2]; 		param_size = sizeof(TCP_ClientIP[2]);			break;
		case 15:	param = &TCP_ClientIP[3]; 		param_size = sizeof(TCP_ClientIP[3]);			break;
		case 16:	param = &TCP_ListenPort;		param_size = sizeof(TCP_ListenPort);			break;
		case 17:	param = &TCP_CONNECT_timeout;	param_size = sizeof(TCP_CONNECT_timeout);		break;
		case 18:	param = &SMS_On;				param_size = sizeof(SMS_On);					break;
		case 19:	param = &SMS_Number[0];			param_size = MaxTelephN-1;						break;
		case 20:	param = &SMS_Number[1];			param_size = MaxTelephN-1;						break;
		case 21:	param = &SMS_Number[2]; 		param_size = MaxTelephN-1;						break;
		case 22:	param = &CurrDataPeriod;		param_size = sizeof(CurrDataPeriod);			break;

	}
//	if(ParamID_Lo == 1) GPRS_Param(9, 0, 0);
//	else  GPRS_Param(9, 1, 0);
	if(ParamID_Lo == 1) GPRS_Param(9, ParamID_Lo, NULL, param_size, Cmd, 0, 0);
	else GPRS_Param(9, ParamID_Lo, param, param_size, Cmd, 1, 0);
}
void GPRS_Param10( const uint8_t ParamID_Lo, uint8_t const Cmd){
	void *param=NULL;
	uint8_t param_size=0;
	switch(ParamID_Lo){	
		case 1:	param = &User_PSW; 			param_size = sizeof(User_PSW);		break;
		case 2:	param = &Password[0];		param_size = sizeof(Password[0]);	break;
		case 3:	param = &Password[1];		param_size = sizeof(Password[1]);	break;
		case 4:	param = &Password[2];		param_size = sizeof(Password[2]);	break;
		case 5:	param = &Password[3];		param_size = sizeof(Password[3]);	break;
	}
	//GPRS_Param(10, 1, 0);
	GPRS_Param(10, ParamID_Lo, param, param_size, Cmd, 1, 0);
}
void GPRS_Param11( const uint8_t ParamID_Lo, uint8_t const Cmd){
	void *param=NULL;
	uint8_t param_size=0;
	switch(ParamID_Lo){	
		case 1:	param = &EMeterType; 		param_size = sizeof(EMeterType);			break;
	}
	GPRS_Param(11, ParamID_Lo, param, param_size, Cmd, 1, 0);
#ifdef E_METER	
	if(ParamID_Lo==1) EMeter_ReInit();	//Переинициализировать ПОРТ
#endif
}

void GPRS_Parameter(const uint8_t ParamID_Hi, const uint8_t ParamID_Lo, uint8_t const Cmd){
	switch(ParamID_Hi){
		case 0:GPRS_Param00(ParamID_Lo, Cmd);break;			
		case 1:GPRS_Param01(ParamID_Lo, Cmd);break;			
		case 2:GPRS_Param02(ParamID_Lo, Cmd);break;			
		case 3:GPRS_Param03(ParamID_Lo, Cmd);break;
		case 4:GPRS_Param04(ParamID_Lo, Cmd);break;
		case 5:GPRS_Param05(ParamID_Lo, Cmd);break;
		case 6:GPRS_Param06(ParamID_Lo, Cmd);break;
		case 7:GPRS_Param07(ParamID_Lo, Cmd);break;
		case 8:GPRS_Param08(ParamID_Lo, Cmd);break;
		case 9:GPRS_Param09(ParamID_Lo, Cmd);break;
		case 10:GPRS_Param10(ParamID_Lo, Cmd);break;
		case 11:GPRS_Param11(ParamID_Lo, Cmd);break;
	}
}

void GPRS_ParamWrite(void){

}
//--- Чтение блока параметров
uint8_t ReadParamBlock(const uint8_t Block){

	uint8_t blocksize=8;

/*	if(Block == 1){
		//---Системные
		GPRS_ChangeData[blocksize++] = SW_VERSION;
		GPRS_ChangeData[blocksize++] = SW_SUBVERSION;
		erbl(GPRS_ChangeData + blocksize, &Vega_SN, sizeof(Vega_SN)); blocksize += sizeof(Vega_SN);
		GPRS_ChangeData[blocksize++] = 0;
		GPRS_ChangeData[blocksize++] = 0;
		GPRS_ChangeData[blocksize++] = 0;
		GPRS_ChangeData[blocksize++] = 0;
		GPRS_ChangeData[blocksize++] = 0;
		GPRS_ChangeData[blocksize++] = Tm_Year;
		GPRS_ChangeData[blocksize++] = Tm_Mon;
		GPRS_ChangeData[blocksize++] = Tm_MDay;
		GPRS_ChangeData[blocksize++] = Tm_Hour;
		GPRS_ChangeData[blocksize++] = Tm_Min;
		GPRS_ChangeData[blocksize++] = Tm_Sec;
		GPRS_ChangeData[blocksize++] = erb(&SummerTime);
		GPRS_ChangeData[blocksize++] = 0;
		GPRS_ChangeData[blocksize++] = 0;
		GPRS_ChangeData[blocksize++] = erb(&Magic1);
		GPRS_ChangeData[blocksize++] = erb(&Magic2);
		//---Общие
		if(erw(&A1_EEP.Config) & (1<<cfg_VFC_OFF)) GPRS_ChangeData[blocksize++] = 1;
		else GPRS_ChangeData[blocksize++] = 0;
		GPRS_ChangeData[blocksize++] = 0;
		GPRS_ChangeData[blocksize++] = 0;
		GPRS_ChangeData[blocksize++] = 0;
		memcpy(GPRS_ChangeData + blocksize, &A1_RAM.FlgAlarm, sizeof(A1_RAM.FlgAlarm)); 	blocksize += sizeof(A1_RAM.FlgAlarm);
		memcpy(GPRS_ChangeData + blocksize, &A1_RAM.FlgWarning, sizeof(A1_RAM.FlgWarning)); blocksize += sizeof(A1_RAM.FlgWarning);
		if(erw(&A1_EEP.Config) & (1<<cfg_Cond_Qt)) GPRS_ChangeData[blocksize++] = 1;
		else GPRS_ChangeData[blocksize++] = 0;
		if(erw(&A1_EEP.Config) & (1<<cfg_Contactor)) GPRS_ChangeData[blocksize++] = 1;
		else GPRS_ChangeData[blocksize++] = 0;
		//---Инверсии
		if(erb(&DI_Invers[0]) & (1<<DI_OPS)) GPRS_ChangeData[blocksize++] = 0;
		else GPRS_ChangeData[blocksize++] = 1;
		if(erb(&DI_Invers[0]) & (1<<DI_Filter)) GPRS_ChangeData[blocksize++] = 0;
		else GPRS_ChangeData[blocksize++] = 1;
		if(erb(&DI_Invers[0]) & (1<<DI_Electro)) GPRS_ChangeData[blocksize++] = 0;
		else GPRS_ChangeData[blocksize++] = 1;	
		if(erb(&DI_Invers[0]) & (1<<DI_Contact)) GPRS_ChangeData[blocksize++] = 0;
		else GPRS_ChangeData[blocksize++] = 1;	
		//---Демпфирование
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.td_OPC, 		sizeof(A1_EEP.td_OPC));			blocksize += sizeof(A1_EEP.td_OPC);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.td_Filter, 	sizeof(A1_EEP.td_Filter));		blocksize += sizeof(A1_EEP.td_Filter);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.td_FanCrash, 	sizeof(A1_EEP.td_FanCrash));	blocksize += sizeof(A1_EEP.td_FanCrash);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.td_TRoomHigh,	sizeof(A1_EEP.td_TRoomHigh));	blocksize += sizeof(A1_EEP.td_TRoomHigh);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.td_Electro, 	sizeof(A1_EEP.td_Electro));		blocksize += sizeof(A1_EEP.td_Electro);
		//---Временные
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.t_Normalization, 			sizeof(A1_EEP.t_Normalization));		blocksize += sizeof(A1_EEP.t_Normalization);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.t_RepeatNormalization, 	sizeof(A1_EEP.t_RepeatNormalization));	blocksize += sizeof(A1_EEP.t_RepeatNormalization);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.t_FreeCoolingFail, 		sizeof(A1_EEP.t_FreeCoolingFail));		blocksize += sizeof(A1_EEP.t_FreeCoolingFail);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.t_ConditionerOperation,	sizeof(A1_EEP.t_ConditionerOperation));	blocksize += sizeof(A1_EEP.t_ConditionerOperation);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.t_CondStartingMax,		sizeof(A1_EEP.t_CondStartingMax));		blocksize += sizeof(A1_EEP.t_CondStartingMax);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.t_IncTfree2cond, 			sizeof(A1_EEP.t_IncTfree2cond));		blocksize += sizeof(A1_EEP.t_IncTfree2cond);
		//---Температурные
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.TCooling, 	sizeof(A1_EEP.TCooling));		blocksize += sizeof(A1_EEP.TCooling);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.dTFreeCool, 	sizeof(A1_EEP.dTFreeCool));		blocksize += sizeof(A1_EEP.dTFreeCool);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.TConditioning,sizeof(A1_EEP.TConditioning));	blocksize += sizeof(A1_EEP.TConditioning);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.THeating, 	sizeof(A1_EEP.THeating));		blocksize += sizeof(A1_EEP.THeating);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.TRoomPreHigh, sizeof(A1_EEP.TRoomPreHigh));	blocksize += sizeof(A1_EEP.TRoomPreHigh);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.TRoomHigh, 	sizeof(A1_EEP.TRoomHigh));		blocksize += sizeof(A1_EEP.TRoomHigh);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.dTCond, 		sizeof(A1_EEP.dTCond));			blocksize += sizeof(A1_EEP.dTCond);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.CondDownRate, sizeof(A1_EEP.CondDownRate));	blocksize += sizeof(A1_EEP.CondDownRate);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.TAirLimit, 	sizeof(A1_EEP.TAirLimit));		blocksize += sizeof(A1_EEP.TAirLimit);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.TAirNormaliz, sizeof(A1_EEP.TAirNormaliz));	blocksize += sizeof(A1_EEP.TAirNormaliz);
		//---Датчики Т
		GPRS_ChangeData[blocksize++] = erb(&ThermType[Troom]);
		GPRS_ChangeData[blocksize++] = erb(&ThermType[Tair]);
		GPRS_ChangeData[blocksize++] = erb(&ThermType[Tcond]);
		GPRS_ChangeData[blocksize++] = erb(&ThermCorr[Troom].D);
		GPRS_ChangeData[blocksize++] = erb(&ThermCorr[Tair].D);
		GPRS_ChangeData[blocksize++] = erb(&ThermCorr[Tcond].D);
		erbl(GPRS_ChangeData + blocksize, &ThermCorr[Troom].A, 	sizeof(ThermCorr[Troom].A));	blocksize += sizeof(ThermCorr[Troom].A);
		erbl(GPRS_ChangeData + blocksize, &ThermCorr[Tair].A, 	sizeof(ThermCorr[Tair].A));		blocksize += sizeof(ThermCorr[Tair].A);
		erbl(GPRS_ChangeData + blocksize, &ThermCorr[Tcond].A, 	sizeof(ThermCorr[Tcond].A));	blocksize += sizeof(ThermCorr[Tcond].A);
		//---Вентилятор
		GPRS_ChangeData[blocksize++] = erb(&A1_EEP.FanParam_NumberPulses);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.FanParam_FanSpeed, 		sizeof(A1_EEP.FanParam_FanSpeed));		blocksize += sizeof(A1_EEP.FanParam_FanSpeed);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.CtrlFan_LimBottom, 		sizeof(A1_EEP.CtrlFan_LimBottom));		blocksize += sizeof(A1_EEP.CtrlFan_LimBottom);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.CtrlFan_LimTop, 			sizeof(A1_EEP.CtrlFan_LimTop));			blocksize += sizeof(A1_EEP.CtrlFan_LimTop);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.FanSpeed_Start, 			sizeof(A1_EEP.FanSpeed_Start));			blocksize += sizeof(A1_EEP.FanSpeed_Start);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.FanSpeed_Normalization, 	sizeof(A1_EEP.FanSpeed_Normalization));	blocksize += sizeof(A1_EEP.FanSpeed_Normalization);
		erbl(GPRS_ChangeData + blocksize, &A1_EEP.FanCheckLuft, 			sizeof(A1_EEP.FanCheckLuft));			blocksize += sizeof(A1_EEP.FanCheckLuft);
		//---ПИД-регулятор
		GPRS_ChangeData[blocksize++] = RegMode;
		GPRS_ChangeData[blocksize++] = AutoTunePID;
		memcpy(GPRS_ChangeData + blocksize, &ArmSetPower, 		sizeof(ArmSetPower));		blocksize += sizeof(ArmSetPower);
		erbl(GPRS_ChangeData + blocksize, &PID_T_EEP.Sample, 	sizeof(PID_T_EEP.Sample));	blocksize += sizeof(PID_T_EEP.Sample);
		erbl(GPRS_ChangeData + blocksize, &PID_T_EEP.Kp, 		sizeof(PID_T_EEP.Kp));		blocksize += sizeof(PID_T_EEP.Kp);
		erbl(GPRS_ChangeData + blocksize, &PID_T_EEP.PBand, 	sizeof(PID_T_EEP.PBand));	blocksize += sizeof(PID_T_EEP.PBand);
		erbl(GPRS_ChangeData + blocksize, &PID_T_EEP.Ki, 		sizeof(PID_T_EEP.Ki));		blocksize += sizeof(PID_T_EEP.Ki);
		erbl(GPRS_ChangeData + blocksize, &PID_T_EEP.Kd, 		sizeof(PID_T_EEP.Kd));		blocksize += sizeof(PID_T_EEP.Kd);
		erbl(GPRS_ChangeData + blocksize, &PID_T_EEP.Deadband, 	sizeof(PID_T_EEP.Deadband));blocksize += sizeof(PID_T_EEP.Deadband);
		//---Пароли
		erbl(GPRS_ChangeData + blocksize, &User_PSW, 	sizeof(User_PSW));		blocksize += sizeof(User_PSW);
		erbl(GPRS_ChangeData + blocksize, &Password[0], sizeof(Password[0]));	blocksize += sizeof(Password[0]);
		erbl(GPRS_ChangeData + blocksize, &Password[1], sizeof(Password[1]));	blocksize += sizeof(Password[1]);
		erbl(GPRS_ChangeData + blocksize, &Password[2], sizeof(Password[2]));	blocksize += sizeof(Password[2]);
		erbl(GPRS_ChangeData + blocksize, &Password[3], sizeof(Password[3]));	blocksize += sizeof(Password[3]);

		GPRS_ChangeData[7] = Block;
		GPRS_AddHeader(GPRS_ChangeData, GPRS_CONTROL, 17, blocksize - 8  + 2);
		GPRS_SendRQ |= (1<<GPRS_RQ_Change);	//запрос на отсылку подтверждения изменения	
		GPRS_ChangeReplySize = blocksize;
	}*/

	if(Block == 2){
	//---Сетевые
		erbl(GPRS_ChangeData + blocksize, &GPRS_apn, 			sizeof(GPRS_apn)-15);			blocksize += sizeof(GPRS_apn)-15;
		erbl(GPRS_ChangeData + blocksize, &GPRS_apn+15, 		sizeof(GPRS_apn)-15);			blocksize += sizeof(GPRS_apn)-15;
		erbl(GPRS_ChangeData + blocksize, &GPRS_apn_user_name,	sizeof(GPRS_apn_user_name)-1);	blocksize += sizeof(GPRS_apn_user_name)-1;
		erbl(GPRS_ChangeData + blocksize, &GPRS_apn_psw, 		sizeof(GPRS_apn_psw));			blocksize += sizeof(GPRS_apn_psw)-1;
		erbl(GPRS_ChangeData + blocksize, &UDP_ServerIP, 		sizeof(UDP_ServerIP));			blocksize += sizeof(UDP_ServerIP);
		erbl(GPRS_ChangeData + blocksize, &UDP_ServerPort, 		sizeof(UDP_ServerPort));		blocksize += sizeof(UDP_ServerPort);
		erbl(GPRS_ChangeData + blocksize, &TCP_ClientIP, 		sizeof(TCP_ClientIP));			blocksize += sizeof(TCP_ClientIP);
		erbl(GPRS_ChangeData + blocksize, &TCP_ListenPort,		sizeof(TCP_ListenPort));		blocksize += sizeof(TCP_ListenPort);
		erbl(GPRS_ChangeData + blocksize, &TCP_CONNECT_timeout, sizeof(TCP_CONNECT_timeout));	blocksize += sizeof(TCP_CONNECT_timeout);
		erbl(GPRS_ChangeData + blocksize, &SMS_On, 				sizeof(SMS_On));				blocksize += sizeof(SMS_On);
		erbl(GPRS_ChangeData + blocksize, &SMS_Number[0][0], 	MaxTelephN-1);					blocksize += MaxTelephN-1;
		erbl(GPRS_ChangeData + blocksize, &SMS_Number[1][0], 	MaxTelephN-1);					blocksize += MaxTelephN-1;
		erbl(GPRS_ChangeData + blocksize, &SMS_Number[2][0], 	MaxTelephN-1);					blocksize += MaxTelephN-1;
		erbl(GPRS_ChangeData + blocksize, &CurrDataPeriod, 		sizeof(CurrDataPeriod));		blocksize += sizeof(CurrDataPeriod);
								
		GPRS_ChangeData[7] = Block;
		GPRS_AddHeader(GPRS_ChangeData, GPRS_CONTROL, 17, blocksize - 8  + 2);
		GPRS_SendRQ |= (1<<GPRS_RQ_Change);	//запрос на отсылку подтверждения изменения	
		GPRS_ChangeReplySize = blocksize;
	}

	if(Block == 3){
	//---Электросчётчик
		GPRS_ChangeData[blocksize++] = erb(&EMeterType);		

		GPRS_ChangeData[7] = Block;
		GPRS_AddHeader(GPRS_ChangeData, GPRS_CONTROL, 17, blocksize - 8  + 2);
		GPRS_SendRQ |= (1<<GPRS_RQ_Change);	//запрос на отсылку подтверждения изменения	
		GPRS_ChangeReplySize = blocksize;
	}

	return blocksize;
	
}

// ----- А Н А Л И З   П Р И Ё М А
void GPRS_ReciveData(void){

	if(GPRS_FlgSz_In /*&& (erb(&GPRS_Config)&(1<<cfg_GPRS_RemoteControl) )*/ ){

		if( (GPRS_Data_In[0]!=5) || (GPRS_Data_In[1]!=1) ){ GPRS_FlgSz_In = 0; return; }
		
		//if( (((uint16_t)GPRS_Data_In[2]<<8) + GPRS_Data_In[3]) != GSM_PASSWORD){ 
		uint16_t Psw = ((uint16_t)GPRS_Data_In[2]<<8) + GPRS_Data_In[3];
		if(GPRS_Check_PSW(Psw) == 0){
			GPRS_Flags |= (1<<GPRS_flg_NoAccess);
			GPRS_SendRQ |= (1 << GPRS_RQ_SessionID);
			GPRS_FlgSz_In = 0;
			return;
		}

		if(GPRS_Data_In[4] == GPRS_GET_SESSION_ID){
			if( erw(&Vega_SN) == (((uint16_t)GPRS_Data_In[5]<<8) + GPRS_Data_In[6]) ) GPRS_Flags &= ~(1<<GPRS_flg_NoAccess);
			else GPRS_Flags |= (1<<GPRS_flg_NoAccess);
			GPRS_SendRQ |= (1 << GPRS_RQ_SessionID);
			GPRS_FlgSz_In = 0; 
			return;
		}
	
		/*GPRSInLen = GPRS_FlgSz_In;
		if(GPRS_Data_In[4] == GPRS_CRASH){
			if(GPRS_Data_In[6] == 0){	// признак квитирования
				if((GPRS_Data_In[5] == 2)&& (GPRS_FlgSz_In == 8) ){// кол-во байт после
					if(GPRS_Data_In[7] == GPRS_CrashID){ // квитирование самой последней аварии
						if(GPRS_Flags & (1<<GPRS_flg_CrashNACK)){
							ResetTimer16(GPRS_TD_CrashSendRetry);
							GPRS_Flags &= ~(1<<GPRS_flg_CrashNACK);
						}
					}
				}
			}
		}*/

		if(GPRS_Data_In[4] == GPRS_CONTROL){

			if( (GPRS_Session_ID==0) || (GPRS_Session_ID != ( (uint16_t)(GPRS_Data_In[5]<<8) + GPRS_Data_In[6]) ) ){
				// если не тот GPRS_Session_ID
				GPRS_Flags |= (1<<GPRS_flg_NoAccess);
				GPRS_SendRQ |= (1 << GPRS_RQ_SessionID);
				GPRS_FlgSz_In = 0; 
				return;
			}

			uint8_t DataLength = GPRS_Data_In[7];
			
			switch(GPRS_Data_In[8]){
	   		case 1:		// текущие данные
				if ( DataLength == 1 && (GPRS_FlgSz_In == 9) ) // кол-во байт
					GPRS_SendRQ |= (1<<GPRS_RQ_CurDataTCP);	//запрос на отсылку текущих данных
	        	break;
	   		case 5:
				if ( (DataLength == 4) && (GPRS_FlgSz_In == 10) ) // кол-во байт
					GPRS_Bit();		// Изменение битовых переменных
				break;
	   		case 6:
				if ( (DataLength == 5) && (GPRS_FlgSz_In == 11) ) // кол-во байт
	   			GPRS_Reg();		// Изменение регистровых переменных
	        	break;
	   		case 7:	//read param
				if ( (DataLength == 3) && (GPRS_FlgSz_In == 11) ) // кол-во байт
	   				GPRS_Parameter(GPRS_Data_In[9], GPRS_Data_In[10], READ_PARAM);
	        	break;				
	   		case 8: //write param
				GPRS_Parameter(GPRS_Data_In[9], GPRS_Data_In[10], WRITE_PARAM);
				break;
	   		case 10:	// Remote Display
				if ( (DataLength==2) && (GPRS_FlgSz_In == 10) ) // кол-во байт
					GPRS_RemoteDisplayControl();
	        	break;
			case 17:	//read param block
				if ( (DataLength == 2) && (GPRS_FlgSz_In == 10) ) // кол-во байт
					ReadParamBlock	(GPRS_Data_In[9]);
				break;
			}
		}
		GPRS_FlgSz_In = 0;
	}// пришли данные по
	else GPRS_FlgSz_In = 0;
}
// ----- А Р Б И Р Т Р А Ж  П Е Р Е Д А Ч И
void GPRS_Arbitr(void){
	
	if(GPRS_SendRQ & (1<<GPRS_RQ_NewCrash)){
		GPRS_SendNewCrash();
		GPRS_SendRQ &= ~(1 << GPRS_RQ_NewCrash);
		GPRS_Protocol_Out = UDP;
		IP_SendMask = 0;	//отсылать на все IP
		GPRSOutLen = GPRS_FlgSz_Out;
		return;
	}
	if(GPRS_SendRQ & (1<<GPRS_RQ_RepCrash)){
		GPRS_SendRepCrash();
		GPRS_SendRQ &= ~(1 << GPRS_RQ_RepCrash);
		GPRS_Protocol_Out = UDP;
		IP_SendMask = 0b11111110;	//отсылать только на IP1
		GPRSOutLen = GPRS_FlgSz_Out;
		return;
	}
	if(GPRS_SendRQ & (1<<GPRS_RQ_SessionID)){
		GPRS_Get_Session_ID();
		GPRS_SendRQ &= ~(1 << GPRS_RQ_SessionID);
		GPRS_Protocol_Out = TCP;
		GPRSOutLen = GPRS_FlgSz_Out;
		return;
	}
	if(GPRS_SendRQ & (1<<GPRS_RQ_Change)){
		GPRS_SendChange(GPRS_ChangeReplySize);
		GPRS_SendRQ &= ~(1 << GPRS_RQ_Change);
		GPRS_Protocol_Out = TCP;
		GPRSOutLen = GPRS_FlgSz_Out;
		return;
	}
	if(GPRS_SendRQ & (1<<GPRS_RQ_Remote)){
		GPRS_SendDisplay();
		GPRS_SendRQ &= ~(1 << GPRS_RQ_Remote);
		GPRS_Protocol_Out = TCP;
		GPRSOutLen = GPRS_FlgSz_Out;
		return;
	}
	if(GPRS_SendRQ & (1<<GPRS_RQ_CurDataTCP)){
		GPRS_CurDataSend();
		GPRS_SendRQ &= ~(1 << GPRS_RQ_CurDataTCP);
		GPRS_Protocol_Out = TCP;
		GPRSOutLen = GPRS_FlgSz_Out;
		return;
	}
	if(GPRS_SendRQ & (1<<GPRS_RQ_CurDataUDP)){
		GPRS_CurDataSend();
		GPRS_SendRQ &= ~(1 << GPRS_RQ_CurDataUDP);
		GPRS_Protocol_Out = UDP;
		IP_SendMask = 0;	//отсылать на все IP
		GPRSOutLen = GPRS_FlgSz_Out;
		return;
	}
}
// ----- Вызывается из ApplCycle
void GPRS_App(void){

	if(GetTimer16(GPRS_TD_SessionIDGenerator)<200) StartTimer16(GPRS_TD_SessionIDGenerator, 0xFFFF);
	if(GSM_ActiveConnection == NO_CONNECTION){
		GPRS_Session_ID = 0;
		GPRS_LoginLevel = 0;
	}

	GPRS_RetryTimer = GetTimer16(GPRS_TD_CrashSendRetry);

	if(!(GPRS_Flags & (1<<GPRS_flg_Fail))){
		//если есть неквитированные аварии
		/*if(GPRS_Flags & (1<<GPRS_flg_CrashNACK)){
			if(Timer16Stopp(GPRS_TD_CrashSendRetry)){
				GPRS_SendRQ |= (1<<GPRS_RQ_RepCrash);		// запрос на повторную отсылку аварии
				StartTimer16(GPRS_TD_CrashSendRetry,12000);	//Запустить таймаут на квитирование
			}
		}*/
		GPRS_ReciveData();
		//запрос на отсылку Remote Display
		if(Timer8Stopp(TD_GPRS_RemoteDisplay) && ( GPRS_Flags & (1<<GPRS_flg_RemoteDispalyRQ) ) ){
			GPRS_SendRQ |= (1<<GPRS_RQ_Remote);
			GPRS_Flags &= ~(1<<GPRS_flg_RemoteDispalyRQ);
		}
		// Отправка текущих данных по GPRS каждые 5 мин
		GPRS_SendData();
		// Отправка по GPRS
		if(!GPRS_FlgSz_Out) GPRS_Arbitr();
	}

	GPRS_CriticalCMD_Process();
}
