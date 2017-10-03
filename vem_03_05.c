/*
	Вега EMReader
	
	History List

	1.00 - Начальная версия (Опытная партия на Киевэнерго) Только прозрачный режим + Modbus конфиг и всё.
	
	3.00 - Серийная версия. Web-интерфейс. Активный режим (опрос ЭС + передача по UDP на сервак)
			05.11.2015 - Во время WebSession - не производить никаких действий в режиме GSM_ServerIdle
				Перезапуск модема каждые 24ч мин (ранее 61 мин)

	3.01 - 05.11.2015 - Исправлена ошибка с Modbus (Add to USART1_UDRE_vect Modbus_TX ISR)

	3.02 - 08.11.2015 - Читаем Ua,Ub,Uc,Ia,Ib,Ic,дату,время,код ошибок, анализ махинаций:магнит,вскрытие,клеемная крышка.

	3.03 - 25.02.2016 - Добавлен режим CSD.

	3.04 - 22.03.2016 -В файле electrometer.h реализовано конвеерное чтение буфера. Сначала читаем 127 байт, потом парсим их, читая в это время следующие 127 байт.
					  -в файле gprs.h добавлен Retry счетчика через 15сек после неудачного опроса один раз. Это убирает проблему "отсутствие связи".

	3.05 - 04.04.2016 - Добавлено чтение напряжений, токов, активной мощности для Меркурий 230.
						Более быстрая переконфигурация модема - прозрачный <-> непрозрачный ~15сек (35 сек ранее)
						При изменении типа счётчика меняется параметры UART.
						Убрали повторный опрос ЭС после выхода из прозрачного режима


	3.06 - 01.09.2016 - Поддержка sim800, уровни доступа 
	3.07 - 15.12.2016 - Данные раз в 30 мин, CSD 60 сек

*/

#define SW_VERSION 3
#define SW_SUBVERSION 7
#define SW_VERSION_Str "3.07"

#include <stdint.h>
#include <stdlib.h>

#define MODBUS
#define E_METER

#define sys_plc "sys_vem3.h"

#define InitInvers {0b00000011} //1-н.р. 0-н.з.
#define InitThermType {ThermOn};

//#define VEGA_DEBUG

#define CRASHLOGMAX 150

#ifdef MODBUS
	//----------- Modbus
	uint8_t MBCoil1[1];
	uint8_t MBInput1[1];
	uint16_t MBHoldReg1[38];
	uint16_t MBInReg1[64];
	#define Modbus_Map_List {	\
		{					\
			MBCoil1,	8,	\
			MBInput1,	8,	\
			MBHoldReg1,	38,	\
			MBInReg1,	30	\
		}					\
	}
	#define Modbus_Qt 1
	#define MB_Port_List {Port(1)}
	#define Init_Modbus_ Param { {EvenParity, 1, MBBR19200} }
	#define MB_Role_List {Role_Slave}
	enum {Port_Slave};
	#define Init_MB_NativeAddr {1}
#endif

//-----------
#define GSM_DEBUG_DELAY 1

#define GPRS_APN		"freecool.mts.net             "
//#define GPRS_APN		"vpni.kyivstar.net            "
#define GPRS_USER_NAME 	"                   "
#define GPRS_PSW 		"                   "



#define UDP_ServerIP_Init	{	\
	{10,1,64,4},				\
	{194,176,97,118},			\
	{0,0,0,0},					\
	{0,0,0,0} }
#define UDP_ServerPort_Init	{2026,2026,0,0}


#define TCP_ListenPort_Init 2020
#define TCP_ClientIP_Init	{	\
	{255,255,255,255},	\
	{0,0,0,0},			\
	{0,0,0,0},			\
	{0,0,0,0} }

#define SMS_NUMBER_INIT {"000000000000", "000000000000", "000000000000"}
#define CSD_ALLOWED_NUMBERS_INIT {"000000000000", "000000000000", "000000000000", "000000000000", \
	"000000000000", "000000000000",	"000000000000", "000000000000", "000000000000", "000000000000"}
//#define VEGA_SN 20000
#define GSM_PASSWORD 22780

#define GSM "gsm_sim800r_vem.h"

// ~~~~~~~~~~~~~~~~~~~
// Digital inputs soft map
	#define DI_1		0
	#define DI_2		1
	#define DI_CFG		2
	#define DI_BL		3

// Digital outputs soft map
	#define DO_GSM		0
	#define DO_RS		1
	#define DO_BL		2

// Thermosensors soft map
	#define T1			0

// ~~~~~~~~~~~~~~~
// Програмные таймера
// Divisible by 8!
#define Timer8ApplManuNumber	010
#define Timer16ApplManuNumber	030
#define Timer32ApplManuNumber	020
#define Timer8ApplNumber	(Timer8ApplManuNumber  + 020)
#define Timer16ApplNumber	(Timer16ApplManuNumber + 020)
#define Timer32ApplNumber	(Timer32ApplManuNumber + 020)


//8-bit
#define TD_GPRS_RemoteDisplay	0
#define TD_BlinkBL				1
#define TD_COMM_LED				2
#define TD_GSM_LED				3
#define TD_SendTestMsg			4

//. . . . . . . . . . . . .
#define TD8_7					7


//16-bit
#define TD_Menu					0
#define TD_GotoDebug			1
#define TD_Normalization		2
#define TD_PID_FANSPEED			3
#define TD_OPC					4
#define TD_Filter				5
#define TD_TRoomHigh			6
#define TD_Electro				7
#define TD_FanCrash				8
#define TD_CheckFilter			9
#define TD_TRoomChangeMode		10
#define TD_TAirChangeMode		11
#define TD_CondDownRate			12
#define TD_FanStart				13
#define TD_Contact				14
#define TD_BlockWaitMode		15

//. . . . . . . . . . . . . . . .
#define TD16_22					22
#define TD16_23					23


//32-bit
#define TD_FreeCoolingFail				0
#define TD_DelayConditionerOperation	1
#define TD_CondStartC					2
#define TD_CondStartR					3
#define TD_LogOut						4
#define TD_Timer_Tune					5
#define TD_RepeatNormalization			6
#define TD_DualOperation				7
#define TD_Test							8
#define TD_ResetCoolError				9
#define TD_ResetContactorError			10
#define TD_FreeCoolWorkTime				11
#define TD_IncTfree2cond				12
//. . . . . . . . . . . . . . . . . . . .
#define TD32_14							14
#define TD32_15							15

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "system_vem2.h"

//----------- Modbus
#ifdef MODBUS
	MB_Master MB_Master_List[] = {};
#endif
//-----------

#define PASSWORDS_Qt 4
#define LOGIN_Init {"user","superuser","service","admin"}
#define PASS_Init {"11111","11100","19191","64688"}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Тестовые сообщения
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	prog_char TestMsg_str[]	= "1234567890123456";
	prog_char TestMsg_str[]	= "тестовое сообщение";
//	prog_char TestTotalOK_str[]	= "Общий тест пройден";
//	prog_char TestFreeOK_str[]	= "Тест фрикулинга пройден";
//	prog_char TestCoolOK_str[]	= "Тест кондиционирования пройден";
//	prog_char AutoPIDFinish_str[]	= "Автонастройка ПИД завершена";
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "menu_web.h"

//#include "webserver.h"
#include "webserver_vem_levels.h"

// ~~~~~~~~~~~~~~~~~~~~~~
//#include "electrometer.h"
#include "vem_electrometer.h"

E_METER_RAM E_Meter_RAM = {
	0,
	0,
	0,
	0,
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint16_t	User_PSW		EEMEM	= 0;

#define PASSWORDS_Qt 4
#define PASSWORD_Init { \
	11111,	\
	11100,	\
	19191,	\
	64688}

#define Sys_PSW 22780

uint8_t Login = 0;	//флаг блокировать или не блокировать?
uint8_t LogOutTime EEMEM = 10;	//1 минута
char Title[21] EEMEM = "No name             ";	//название объекта

uint16_t Thigh	EEMEM = 500;
uint16_t Tlow	EEMEM = -200;

uint16_t Usupply;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifdef GSM
	#include "vem_gprs.h"
#endif
#ifdef MODBUS
	#include "vem_modbus.h"
#endif

#include "vem_debug.h"

//#include "web.h"
#include "web_vem_levels.h"

void ToggleLED(uint8_t DO){
	if((uint8_t)LiveTime & (1<<2)) ResDigOut(DO);
	else SetDigOut(DO);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
ApplInit(){

	Modbus_App_Init();

	#ifdef GSM
		GPRS_AppInit();
	#endif

	WebInit();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
ApplCycle()
{

//	if(FixBug(1)) Reboot();

	WebServer((WebPage **)&WebSite, sizeof(WebSite)/sizeof(WebPage*));

//Приём, передача по GPRS
	#ifdef GSM
		GPRS_App();
	#endif
	
	#ifdef MODBUS
		Modbus_App();
	#endif

	/*//Seend Test Msg 
	if( DI_State(DI_BL) ){
		//if(Timer16Stopp(TD_SendTestMsg)){
			GPRS_NewMsg(GPRS_INFO, TestMsg_str);
		//}
	}
	else{
		//StartTimer8(TD_SendTestMsg, 50);
	}*/

	/*	if( DI_State(DI_BL) == RiseFront){
			GPRS_NewMsg(GPRS_INFO, TestMsg_str);
	
		}
	*/	


	// GSM_DebugMode
	if(UART_Soft){
		if(GSM_DebugMode){
			if( DI_State(DI_BL) ){
				if(Timer16Stopp(TD_GotoDebug)){
					StopDebug();
					StartTimer16(TD_GotoDebug,500);
				}
			}
			else StartTimer16(TD_GotoDebug,50);
		}
		else{
			if( DI_State(DI_BL) ){
				if(Timer16Stopp(TD_GotoDebug)){
					StartDebug();
					StartTimer16(TD_GotoDebug,500);
				}
			}
			else StartTimer16(TD_GotoDebug,100);
		}
	}
	else{
		if( DI_State(DI_BL) ){
			if(Timer16Stopp(TD_GotoDebug)){
				GPRS_NewMsg(GPRS_INFO, TestMsg_str);
				StartTimer16(TD_GotoDebug,500);
			}
		}
		else StartTimer16(TD_GotoDebug,100);
	}

	//Управление DO_BL
	if(GSM_DebugMode){
		if(Timer8Stopp(TD_BlinkBL)){
			StartTimer8(TD_BlinkBL,5);
			TogDigOut(DO_BL);
		}
	}
	else ResDigOut(DO_BL);

	//---- Управление светодиодом GSM_LED при приёме/передаче через UART0
	if(GSM_LED == 1){
		StartTimer8(TD_GSM_LED,5);
		GSM_LED = 2;
	}
	if(GSM_LED ==2){
		if(Timer8Stopp(TD_GSM_LED)) GSM_LED = 0;
	}
	// Управление светодиодом DO_GSM
	if(GSM_State == GSM_ServerIdle){
		if(GSM_LED) ToggleLED(DO_GSM);
		else SetDigOut(DO_GSM);
	}
	else{
		if(GSM_LED) ToggleLED(DO_GSM);
		else ResDigOut(DO_GSM);
	}

/*
	//---- Управление светодиодом COMM_LED при приёме/передаче через UART1
	if(COMM_LED == 1){
		StartTimer8(TD_COMM_LED,5);
		COMM_LED = 2;
	}
	if(COMM_LED ==2){
		if(Timer8Stopp(TD_COMM_LED)) COMM_LED = 0;
	}
	// Управление светодиодом COMM_LED 
	if(EMeter_Link()){
		if(COMM_LED) ToggleLED(DO_RS);
		else SetDigOut(DO_RS);
	}
	else{
		if(COMM_LED) ToggleLED(DO_RS);
		else ResDigOut(DO_RS);
	}
	*/


}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
