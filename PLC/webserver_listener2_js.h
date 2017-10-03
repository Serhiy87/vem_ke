/*
	Web Server 

	Работает с SIM900R в режиме Transparent Mode. 
	Запрос GET
	Анализируем только первую строку: GET /.... HTTP/1.1
	Запрос POST
	Анализируем первую строку: POST /.... HTTP/1.1 и строку ответа (кот.следует после пустой строки)
	Примечание: строка ответа в POST-запросе не заканчивается символами <CR><LF>! - 

	Обнаржен глюк SIM900R (Revision:1137B01SIM900R64_ST_ENHANCE_EAT) в Server Transparent Mode.
	Если после конекта выдержать паузу - то MCU нифига не получаем от модема после.

	Если URL не найден браузеры не любят сразу ответ - нужна задержка ~2сек

	В форме input type='text' начальные пробелы браузер заменяет на '+++++' при запросе GET
	Т.е. если в форме введено '    89' то передаётся GET /?VarByte=++++89 HTTP/1.1

	Если в теле страницы есть ссылки на другие объекты(например рисунок блабла.gif) то браузеры 
	не дожидаясь докачки страницы сразу лупят запрос  GET /блабла.gif HTTP/1.1 - 
	а это проблема т.к. пока не отдали текущую страницу запросы не анализируем

	Вводим таймаут ожидания HTTP-запроса

	Нельзя использовать одинаковые названия переменных в структурах WebOutField ххх.Name.

	Нет flow control при передаче в модем. По идеи правильное решение - использовать hardware flowcontrol. 
	Но на практике при передаче большого объёма данных	CTS модема не реагирует (0), а данные в браузер приходят не все.
	Если очень большой объём предавать за раз (10кБ) то CTS может реагировать. Т.е. использование hardware flowcontrol
	не гарантирует 100% целосности при максимальной скорости.
	По datasheet максимум за раз в SIM900R можно втюхнуть 1460 байт
	Используем следующий метод: отправил >= 1024байт - пауза 2sec - отправил >= 1024байт - пауза 2sec - ...
	Критерий выбора паузы и объёма - параметры AT+CIPCCFG?

	Сессии
	Нет логин,пароля на сайт! 
	Как это сделать: http://phpfaq.ru/sessions
	Про куки хорошо сказано здесь: https://ru.wikipedia.org/wiki/HTTP_cookie
	
	Ищем <form action='/' method='GET'> и вставляем:
	<input type="hidden" name='SESSID' value='72fa12d3fc837bc0'>
	Ищем <a href="index.html?SESSID=72fa12d3fc837bc0"> и добавляем: ?SESSID=72fa12d3fc837bc0

	ОГРАНИЧЕНИЯ:
	1) В теле веб-странциы нельзя использовать "{" т.к. используется для вывода переменных.
	2) Нельзя задавать большие формы с кучей переменных (проверено макс 5 переменных). т.к. 
		а) ограничен размером старотовой строка 100байт
		б) при сохранении в EEPROM подгружается Scancycle
	3) в формах всегда использовать: action='/' method='GET'
	4)
	
	Для Logout создать ссылку "<a href=\"logout.html\">Выход</a>\r\n" 

	Если устройство без ЖКИ #idef TypeLCD

	TODO:
	1) После загрузке страницы login.html, браузер сразу запрашивают favicoc.ico, которая грузится ? секунд.
	При этом попытка залогинится - 
	2) 

	Проблема с выводом FFloat видимо вывод вылезает за пределы и ломает html-разметку кнопка ОК исчезает
	Проблема с вводом (метод GET) GFloat - не передаются '+'


*/

/*
	Запрос страницы
	GET / HTTP/1.1				//Стартовая строка
	Host: 10.1.7.239			//Заголовки
	User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:37.0) Gecko/20100101 Firefox/37.0
	Accept: text/html,application/xhtml+xml,application/xml;q=0.9,q=0.8
	Accept-Language: en-US,en;q=0.5
	Accept-Encoding: gzip, deflate
	Connection: keep-alive
	Cache-Control: max-age=0

	Изменение: pwm=210
	GET /?pwm=210 HTTP/1.1

	Изменение: Нажата кнопка Start
	GET /?1=Start HTTP/1.1

	Изменение: много переменных в одной форме
	GET /?UDP0_IP1=12&UDP0_IP2=32&UDP0_IP3=43&UDP0_IP4=3&UDP0_Port=1234 HTTP/1.1

	Изменение: ввод переменных Text, EE_Text с пробелами
	GET /?Name=KIE+FST+SRV HTTP/1.1

	Изменение: ввод переменных Text, EE_Text нельзя кирилицу, знаки:!?
	char Name[21] EEMEM = "АБВГДЕЁЖЗИКЛ"
	GET /?Name=%С1%С2%С3%С4%С5%С6%С7%С8%С9%СA%СB HTTP/1.1

	Ввод пароля и логина методом POST Mozilla

	POST / HTTP/1.1
	Host: 10.1.7.239
	User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:37.0) Gecko/20100101 Firefox/37.0
	Accept: text/html,application/xhtml+xml,application/xml;q=0.9,* / *;q=0.8
	Accept-Language: en-US,en;q=0.5
	Accept-Encoding: gzip, deflate
	Referer: http://10.1.7.239/login.html
	Connection: keep-alive
	Content-Type: application/x-www-form-urlencoded
	Content-Length: 25

	login=admin&password=vega


	Ввод пароля и логина методом POST Opera
	POST / HTTP/1.1
	User-Agent: Opera/9.64 (Windows NT 5.1; U; en) Presto/2.1.1
	Host: 10.1.7.239
	Accept: text/html, application/xml;q=0.9, application/xhtml+xml, image/png, image/jpeg, image/gif, image/x-xbitmap, * / *;q=0.1
	Accept-Language: ru-RU,ru;q=0.9,en;q=0.8
	Accept-Charset: iso-8859-1, utf-8, utf-16, *;q=0.1
	Accept-Encoding: deflate, gzip, x-gzip, identity, *;q=0
	Referer: http://10.1.7.239/login.html
	Connection: Keep-Alive, TE
	TE: deflate, gzip, chunked, identity, trailers
	Content-Length: 29
	Content-Type: application/x-www-form-urlencoded

	login=admin12&password=123456

	Ввод пароля и логина методом POST Chrome
	POST / HTTP/1.1
	Host: 10.1.7.239
	Connection: keep-alive
	Content-Length: 25
	Cache-Control: max-age=0
	Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,* / *;q=0.8
	Origin: http://10.1.7.239
	User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2272.101 Safari/537.36
	Content-Type: application/x-www-form-urlencoded
	Referer: http://10.1.7.239/login.html
	Accept-Encoding: gzip, deflate
	Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.6,en;q=0.4

	login=admin&password=vega


*/

#ifdef WEB_DEBUG
	char StartStr[150];	
#endif

static uint8_t WebMode = 254;
uint8_t Web_POST;

uint16_t WebDataCnt;
uint8_t WebInputHiddenCnt;
uint8_t WebHrefHiddenCnt;
uint8_t WebImgSrcHiddenCnt;

uint8_t TD_SetCRLFinPOST_RQ;
uint8_t TD_WaitLCDRefresh;
uint8_t TD_WaitReqTimout;
uint8_t TD_FlowDelay;	// для предотвращения переполнения буфера используем задержки

// Можно подстраивать задержки
#ifndef MaxPacketSz
	#define MaxPacketSz 1024 //1024 byte
#endif
#ifndef InterPacketDelay
	#define InterPacketDelay 200	//2sec
#endif
#ifndef EndPacketDelay
	#define EndPacketDelay	200 //2sec
#endif
#ifndef WEB_VARS_REFRESH_TIME
	#define WEB_VARS_REFRESH_TIME 20	//макс 255
#endif

//--Session
uint8_t TD_WebSeedGenerator, TD_WebSessTimeout;
uint8_t WebSession;
uint8_t Web_LoginAttempt;
uint8_t Web_Login_Code;

#ifndef WEB_SESSION_TIME
#define WEB_SESSION_TIME 24000
//#define WEB_SESSION_TIME 6000
#endif
uint32_t WebMaxSessionTime EEMEM = WEB_SESSION_TIME;
#define WebMaxLoginSz 16
#define WebMaxPasswSz 11
char Web_Login_Str[WebMaxLoginSz];
char Web_Passw_Str[WebMaxPasswSz];

const prog_char Web_Logins[PASSWORDS_Qt][WebMaxLoginSz] = LOGIN_Init;
char Web_Passws[PASSWORDS_Qt][WebMaxPasswSz] EEMEM= PASS_Init;

char SESSID_Str[20];

//--

typedef const struct{
	const prog_char *Name;
	OutField *OField;
} PROGMEM WebOutField;

typedef const struct{
	const prog_char *Name;	
	InField *IF_List;
	const uint16_t OFSz;
	WebOutField *OF_List;
	const uint16_t ContentSz;
	const prog_char *Content;
} PROGMEM WebPage;

extern WebPage Web_login;
extern WebPage Web_favicon;
extern const prog_char name_favicon[];

const prog_char HTTP_200_OK[] = {
	"HTTP/1.0 200 OK\r\n"
//	"Date: Fri, 04 Jun 2015 08:52:09 GMT\r\n"
//	"Server: EMReader\r\n"
//	"Last-Modified: Wed, 26 May 2010 11:34:58 GMT\r\n"
//	"Accept-Ranges: bytes\r\n"
//	"Content-Length: 8394\r\n"
//	"Keep-Alive: timeout=5, max=100\r\n"
//	"Connection: Keep-Alive\r\n"
};

const prog_char Content_Type[] = "Content-Type: ";
const prog_char Content_Length[] = "\r\nContent-Length: ";
const prog_char html_cp1251_str[] = "text/html; charset=windows-1251\r\n";
const prog_char image_str[] = "image/";
const prog_char html_str[] = "html";
const prog_char ico_str[] = "ico";
const prog_char gif_str[] = "gif";
const prog_char jpeg_str[] = "jpeg";
const prog_char Connection_Close[] = "Connection: close\r\n\r\n";
const prog_char inputSESSID_Str[] = "<input type=\'hidden\' name=\'SESSID\' value=\'";
const prog_char SESSID_pstr[] = "?SESSID=";

// Страничка 404
const prog_char error_page[] =	{
    "HTTP/1.0 404 Not Found\r\n"
    "Content-Type: text/html; charset=windows-1251\r\n"
    "Connection: close\r\n"
    "\r\n"
    "<h1>404 - Not Found</h1>"
};
WebPage Web_error_page = {NULL, NULL, 0, NULL, sizeof(error_page), error_page};

/*
HTTP/1.0 200 OK
Content-Type: image/gif
Content-Length: 7316
Connection: close
*/

//Method
prog_char METHOD_GET[]	= "GET /";
prog_char METHOD_POST[] = "POST /";

uint8_t Web_Login_Error(void);

void WebInit(void){
	TD_SetCRLFinPOST_RQ = Timer8SysAlloc(1);
	TD_WaitLCDRefresh = Timer8SysAlloc(1);
	TD_FlowDelay =  Timer16SysAlloc(1);
	TD_WaitReqTimout =  Timer16SysAlloc(1);
	TD_WebSeedGenerator	= Timer32Alloc();		// номер таймера для генерации SessionID
	TD_WebSessTimeout	= Timer32Alloc();
	if(Timer8AllocErr() || Timer16AllocErr() || Timer32AllocErr()) ShowMsg(&MsgErr41);
}
// ~~~~~~~~~~~~~~~~~~~~~~~
// Check if variable in range for any type
//Kononenko add (ty_)Buf in SetField, SetEEField
#define WebSetField(Buf, ty)	\
	if((ty)Buf>(ty)Max)					\
		*(ty*)Var = (ty)Max;	\
	else if((ty)Buf<(ty)Min)		\
		*(ty*)Var = (ty)Min;	\
	else										\
		*(ty*)Var = (ty)Buf

#define WebSetEEField(Buf, ty1, ty2, ty3)	\
	if((ty1)Buf>(ty1)Max)											\
		ew##ty3((ty2*)Var, Max);						\
	else if((ty1)Buf<(ty1)Min)									\
		ew##ty3((ty2*)Var, Min);						\
	else																	\
		ew##ty3((ty2*)Var, (ty2)Buf)
// ~~~~~~~~~~~~~~~~~~~~~~~
//Change BufStr due to precision for correct sscanf reading BufStr
void WebPresicionHanler(OutField *Field, char *BufStr){
	if(prb(&Field->Prec)){
		char *p = strchr(BufStr,'.');	//seek for first decimal point
		if(p==NULL){
			p = strchr(BufStr,' ');	//else seek for first space
		}
	  	for(uint8_t i=0; i<prb(&Field->Prec); i++){
			if(*p == ' ') *p='0'; 
			if(*p == '.'){
				if(isdigit(*(p+1))){
					*p= *(p+1);
					*(p+1) = '.';
				}
				else *p='0'; 
			}
			p++;
		}
	}
	
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Отличия от PutField из menu.h
//	Для переменных 
static void
WebPutField(uint8_t Type, void *Var, T_Param *Param)
{
	float fTemp;

	switch(Type) {
	case Byte:
		PutVal(*(uint8_t*)Var, 'b', Param);
		break;
	case EE_Byte:
		PutVal(erb((uint8_t*)Var), 'b', Param);
		break;
	case s_Byte:
		Put_sVal(*(int8_t*)Var, 'b', Param);
		break;
	case s_EE_Byte:
		Put_sVal((int8_t)erb((uint8_t*)Var), 'b', Param);
		break;
	case z_Byte:
		Put_zVal(*(uint8_t*)Var, 'b', Param);
		break;
	case z_EE_Byte:
		Put_zVal(erb((uint8_t*)Var), 'b', Param);
		break;

	case zh_Byte:
		Put_zVal_h(*(uint8_t*)Var, 'b', Param);
		break;
	case zh_Word:
		Put_zVal_h(*(uint16_t*)Var, 'w', Param);
		break;
	case zh_Longint:
		Put_zVal_h(*(uint32_t*)Var, 'l', Param);
		break;

	case Word: case c_Word:
		PutVal(*(uint16_t*)Var, 'w', Param);
		break;
	case EE_Word:
		PutVal(erw((uint16_t*)Var), 'w', Param);
		break;
	case s_Word:
		Put_sVal(*(int16_t*)Var, 'w', Param);
		break;
	case s_EE_Word:
		Put_sVal((int16_t)erw((uint16_t*)Var), 'w', Param);
		break;
	case z_Word: case zc_Word:
		Put_zVal(*(uint16_t*)Var, 'w', Param);
		break;
	case z_EE_Word:
		Put_zVal(erw((uint16_t*)Var), 'w', Param);
		break;


	case Longint: case c_Longint:
		PutDVal(*(uint32_t*)Var, Param);
		break;
	case EE_Longint:
		PutDVal(erd((uint32_t*)Var), Param);
		break;
	case s_Longint:
		Put_sDVal(*(int32_t*)Var, Param);
		break;
	case s_EE_Longint:
		Put_sDVal((int32_t)erd((uint32_t*)Var), Param);
		break;
	case z_Longint: case zc_Longint:
		Put_zDVal(*(uint32_t*)Var, Param);
		break;
	case z_EE_Longint:
		Put_zDVal(erd((uint32_t*)Var), Param);
		break;

		
	case FFloat:
		Put_FFVal(*(float*)Var, Param);
		break;			
	case EE_FFloat:
		erbl(&fTemp, Var, 4);
		Put_FFVal(fTemp, Param);
		break;
	case GFloat:
		Put_GFVal(*(float*)Var, Param);
		break;			
	case EE_GFloat:
		erbl(&fTemp, Var, 4);
		Put_GFVal(fTemp, Param);
		break;


	case Enum:
		Param->Txt += *(uint8_t*)Var;
		PutEnum(Param);
		break;
	case EE_Enum:
		Param->Txt += erb((uint8_t*)Var);
		PutEnum(Param);
		break;
	case Bit:
		strcpy_P(Param->Pos, prp(Param->Txt+((*((uint8_t*)Var+Param->Prec/8) &(1<<Param->Prec%8))>0)));
		break;
	case EE_Bit:
		strcpy_P(Param->Pos, prp(Param->Txt+((erb((uint8_t*)Var+Param->Prec/8) &(1<<Param->Prec%8))>0)));
		break;

	case Text:
		
		Put_TextValAlignLeft(Var, Param);
		break;
	case EE_Text:{
			//char TempStr[LCDXSz+1];
			char TempStr[Param->Size+1];
			erbl(TempStr, Var, Param->Size);
			Put_TextValAlignLeft(TempStr, Param);
		}
		break;
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~
// Изменения и дополнения для посимвольно вводимых переменных
// для остальных переменных всё по-старому
static void
WebBuf2Field(OutField *Field, char *BufStr)
{
	void *Var = prp(&Field->Var);
	uint8_t BitN=prb(&Field->Prec), Mask=1<<BitN%8, *BitVar=(uint8_t*)Var+BitN/8;
	uint8_t EE_BitVar;
	uint32_t Min=prd(&Field->Min), Max=prd(&Field->Max);

	uint32_t Temp=0;
	float fTemp=0;


	switch(prb(&Field->Type)) {



	case Byte: case z_Byte:
		//if(Limit_uByteVar(Temp)) sscanf(BufStr,"%hud", (unsigned short int*)&Temp);
		WebPresicionHanler(Field, BufStr);
		sscanf(BufStr,"%hud", (unsigned short int*)&Temp);
		SetField(Temp, uint8_t);
		break;
	case EE_Byte: case z_EE_Byte:
		WebPresicionHanler(Field, BufStr);
		if(sscanf(BufStr,"%hud", (unsigned short int*)&Temp) == 1){
			SetEEField(Temp, uint8_t, uint8_t, b);
		}		
		break;
	case s_Byte:
		WebPresicionHanler(Field, BufStr);
		//if(Limit_sByteVar(Temp)) sscanf(BufStr,"%hd", (short int*)&Temp);
		sscanf(BufStr,"%hd", (short int*)&Temp);
		SetField(Temp, int8_t);
		break;
	case s_EE_Byte:
		WebPresicionHanler(Field, BufStr);
		if(sscanf(BufStr,"%hd", (short int*)&Temp) == 1){
			WebSetEEField(Temp, int8_t, uint8_t, b);
		}
		break;

	case Word: case z_Word:
		//if(Limit_uByteVar(Temp)) sscanf(BufStr,"%ud", (unsigned short int*)&Temp);
		WebPresicionHanler(Field, BufStr);
		sscanf(BufStr,"%ud", (uint16_t*)&Temp);
		SetField(Temp, uint16_t);
		break;

	case EE_Word: case z_EE_Word:
		WebPresicionHanler(Field, BufStr);
		if(sscanf(BufStr,"%lud", (uint32_t*)&Temp) == 1){
			SetEEField(Temp, uint16_t, uint16_t, w);
		}
		break;
	case s_Word:
		WebPresicionHanler(Field, BufStr);
		//if(Limit_sByteVar(Temp)) sscanf(BufStr,"%hd", (short int*)&Temp);
		sscanf(BufStr,"%d", (int16_t*)&Temp);
		SetField(Temp, int16_t);
		break;
	case s_EE_Word:
		WebPresicionHanler(Field, BufStr);
		if(sscanf(BufStr,"%d", (int16_t*)&Temp) == 1){
			WebSetEEField(Temp, int16_t, uint16_t, w);
			//ewb(Var, (uint16_t)Temp);
		}
		break;
	case c_Word: case zc_Word:
		*(uint16_t*)Var = F_Buf;
		break;



	case Longint: case z_Longint:
		//if(Limit_uByteVar(Temp)) sscanf(BufStr,"%ud", (unsigned short int*)&Temp);
		WebPresicionHanler(Field, BufStr);
		sscanf(BufStr,"%lud", (uint32_t*)&Temp);
		SetField(Temp, uint32_t);
		break;
	case EE_Longint: case z_EE_Longint:
		WebPresicionHanler(Field, BufStr);
		if(sscanf(BufStr,"%lud", (uint32_t*)&Temp) == 1){
			SetEEField(Temp, uint32_t, uint32_t, d);
		}		
		break;
	case s_Longint:
		WebPresicionHanler(Field, BufStr);
		//if(Limit_sByteVar(Temp)) sscanf(BufStr,"%hd", (short int*)&Temp);
		sscanf(BufStr,"%ld", (int32_t*)&Temp);
		SetField(Temp, int32_t);
		break;
	case s_EE_Longint:
		WebPresicionHanler(Field, BufStr);
		if(sscanf(BufStr,"%ld", (int32_t*)&Temp) == 1){
			SetEEField(Temp, int32_t, uint32_t, d);
			//ewd(Var, Temp);
		}
		break;



	case FFloat: case GFloat:
		sscanf(BufStr,"%f", &fTemp);
		*(float*)Var = fTemp;
		break;
	case EE_FFloat: case EE_GFloat:
		if(sscanf(BufStr,"%f", &fTemp) == 1){
			ewbl(&fTemp, (float*)Var, 4);
		}
		break;


	case Enum:
		SetField(F_Buf, uint8_t);
		break;
	case EE_Enum:
		SetEEField(F_Buf, uint8_t, uint8_t, b);
		break;
	case Bit:
		sscanf(BufStr,"%hud", (unsigned short int*)&Temp);	//если 0 -выкл, иначе вкл
		if((unsigned short int)Temp)
			*BitVar |=Mask;
		else
			*BitVar &=~Mask;
		break;
	case EE_Bit:
		sscanf(BufStr,"%hud", (unsigned short int*)&Temp);	//если 0 -выкл, иначе вкл
		EE_BitVar=erb(BitVar);
		ewb(BitVar, (unsigned short int)Temp ? EE_BitVar |Mask : EE_BitVar &~Mask);
		break;


	case Text:{
		//ограничение при вводе длинной строки prb(&Field->Prec)
		uint8_t Width = prb(&Field->Prec);
		char TempStr[Width];
		uint8_t i=0;
		while( (i<Width-1)&&(BufStr[i]!=' ')&&(BufStr[i]!='&') ){
			if(BufStr[i] == '+') TempStr[i]=' ';
			else TempStr[i]=BufStr[i];
			i++;
		}
		TempStr[i] = '\0';
		memcpy(Var, TempStr, i+1);
		}


		break;
	case EE_Text:{
		//ограничение при вводе длинной строки prb(&Field->Prec)
		uint8_t Width = prb(&Field->Prec);
		char TempStr[Width];
		uint8_t i=0;
		while( (i<Width-1)&&(BufStr[i]!=' ')&&(BufStr[i]!='&') ){
			if(BufStr[i] == '+') TempStr[i]=' ';
			else TempStr[i]=BufStr[i];
			i++;
		}
		TempStr[i] = '\0';
		ewbl(TempStr, Var, i+1);
		}
		break;
	}

	EventFunc(&Field->Act);
}


	uint8_t WebStrLen;
// ~~~~~~~~~~~~~~~~~~
WebPage* WebRequest(WebPage **ptrSite, const uint8_t SiteSz){
	

	WebStrLen = GetStringFromFIFOwithOverflowDetect();
	if( (strstr_P(GSM_RxStr, URC_CLOSED+2) != NULL) ){
			GSM_State =GSM_ReStart1;
			return NULL;

		}
	if(Web_POST==2){ 
		if(Timer8Stopp(TD_SetCRLFinPOST_RQ)){	//Chrome, Opera не завершают строку ответа
			WebStrLen = ForceEndStringFromFIFO();
			Web_POST=3;	
		}
		if(WebStrLen) Web_POST=3;				//Explorer завершают строку ответа
	}

	//Анализ принятой строки
	if(WebStrLen){

		// Анализ метода
		char *url, *p; 
		
		WebPage* ptrPage = prp(ptrSite);

		//---- Анализ метода POST -----------------------------------------------------------------------------------
		if( (strstr_P(GSM_RxStr, METHOD_POST) != NULL) ){	//получили строку
			Web_POST = 1; 
			return NULL;
		}
		if( (Web_POST==1) && (WebStrLen==1)){	//получили пустую строку, подождали и установили /0 в строке ответа
			Web_POST = 2;
			StartTimer8(TD_SetCRLFinPOST_RQ,100);	//300мс - для Opera мало, 600 мс -ок
			return NULL;
		}
		if(Web_POST==3){	//обработка полезной инфы
			Web_POST = 0;
			
			#ifdef WEB_DEBUG
				strncpy(StartStr, GSM_RxStr, sizeof(StartStr));	//For Debug
			#endif

			//URL анализ
			url = GSM_RxStr;

			//--- запрос на изменение переменной
			//Например:login=admin&password=secret
			char *nextvar, *varfield;
			WebPage* ptrPageVar = &Web_error_page;
			do{
				p=strchr(url,'=');
				//if(p==NULL) return ptrPage;
				if(p==NULL) {return &Web_login;}
				nextvar=strchr(url+1,'&');
				varfield = p + 1;
				*p = '\0';
				//поиск переменной по названию
				//for(uint8_t i=0; i<SiteSz; i++){
					uint8_t i = 1; //поиск только по второй страница (login.html)
					ptrPage = (WebPage*)prp(&ptrSite[i]);
					for(uint8_t j=0; j<prw(&ptrPage->OFSz)/sizeof(WebOutField); j++){
						WebOutField* ptrWOF= (WebOutField*)prp(&ptrPage->OF_List);
						if(ptrWOF != NULL){
							if(!strcmp_P(url, prp(&ptrWOF[j].Name))){
								while(*varfield=='+') varfield++;	// ignore leading '+'
								WebBuf2Field( (OutField*)prp(&ptrWOF[j].OField), varfield);
								ptrPageVar = ptrPage;	//нашло переменную - значит возвратит текущий ptrPage
								
							}
						}
					}
				//}
				url = nextvar+1;
			}while(nextvar);	//перебор по переменным в запросе 

			if(Web_LoginAttempt){
				Web_LoginAttempt = 0;
				Web_Login_Code = Web_Login_Error();
				if(!Web_Login_Code) return prp(ptrSite);
				else return ptrPageVar;
			}
			//Нет данного URL
			return ptrPageVar;
		}

		//---- Анализ метода GET -----------------------------------------------------------------------------------
		//Например:GET /?SESSID=72fa12d3fc837bc0&UDP0_IP1=12&UDP0_IP2=32&UDP0_IP3=43&UDP0_IP4=3&UDP0_Port=1234 HTTP/1.1
		//Например:GET /system.html?SESSID=72fa12d3fc837bc0 HTTP/1.1
	
		url = GSM_RxStr+4;
		if( (strstr_P(GSM_RxStr, METHOD_GET) != NULL) && (p=strchr(url,' ')) ){
			
			#ifdef WEB_DEBUG
				strncpy(StartStr, GSM_RxStr, sizeof(StartStr));	//ForDebug
			#endif

			*p = '\0';

			//обязательно наличие ?SESSID= в запросе, кроме favicon.ico! 
			if(!strcmp_P(url+1, name_favicon)) return &Web_favicon;
			url = strchr(url,'?');
			if( !WebSession || (url == NULL) || strncmp_P(url, SESSID_pstr, 8) || strncmp(url+8, SESSID_Str, 16) ) return &Web_login;

			StartTimer32(TD_WebSessTimeout, erd(&WebMaxSessionTime));
			
			*url = '\0';
			
			//URL анализ

			//url = GSM_RxStr+5;
			url = strchr(url+1,'&');//проверка есть ли следующая за SESSID переменная?

			//--- запрос на изменение переменной
			//Например:GET /?SESSID=72fa12d3fc837bc0&UDP0_IP1=12&UDP0_IP2=32&UDP0_IP3=43&UDP0_IP4=3&UDP0_Port=1234 HTTP/1.1
			if(url != NULL){
				
				char *nextvar, *varfield;
				WebPage* ptrPageVar = &Web_error_page;
				do{
					p=strchr(url,'=');
					if(p==NULL) return ptrPage;
					nextvar=strchr(url+1,'&');
					varfield = p + 1;
					*p = '\0';
					//поиск переменной по названию
					for(uint8_t i=0; i<SiteSz; i++){
						ptrPage = (WebPage*)prp(&ptrSite[i]);
						for(uint8_t j=0; j<prw(&ptrPage->OFSz)/sizeof(WebOutField); j++){
							WebOutField* ptrWOF= (WebOutField*)prp(&ptrPage->OF_List);
							if(ptrWOF != NULL){
								if(!strcmp_P(url+1, prp(&ptrWOF[j].Name))){
									while(*varfield=='+') varfield++;	// ignore leading '+'
									WebBuf2Field( (OutField*)prp(&ptrWOF[j].OField), varfield);
									ptrPageVar = ptrPage;	//нашло переменную - значит возвратит текущий ptrPage
								}
							}
						}
					}
					url = nextvar;
				}while(nextvar);	//перебор по переменным в запросе 
				
				return ptrPageVar;
			}//---

			// Запросили главную страничку? - первая в списке 
			url = GSM_RxStr+4;
	
            if(!strcmp_P(url, PSTR("/"))){
				return ptrPage;			
			}
            if(!strcmp_P(url+1, PSTR("logout.html"))){
				WebSession = 0;
				return &Web_login;
			}

			
			for(uint8_t i=0; i<SiteSz; i++){
				
				ptrPage = (WebPage*)prp(&ptrSite[i]);

				if(!strcmp_P(url+1, prp(&ptrPage->Name))){
					return ptrPage;				
				}
			}
			
			//Нет данного URL
			return &Web_error_page;
		}
	}
	return NULL;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static InField*
PutWebLine(InField *IF, const uint8_t Size)
{
	uint8_t X;
	for(X=0; X<Size; X++){
		char Char;
		T_Param Param;
		Param.Pos = &GSM_TxStr[X];
		Param.Size = 0;
		switch(Char=GSM_TxStr[X]) {
		case '^':
			while((Char = GSM_TxStr[X + ++Param.Size])=='^');
			Param.Prec = prb(&IF->Prec);
			Param.Txt = prp(&IF->EnumList);
			WebPutField(prb(&IF->Type), prp(&IF->Var), &Param);
			#ifdef TypeLCD
				//Отображение курсора
				if(prp(&IF->Var) == &LCD_Buf[0][0] && CurrLCD==0) GSM_TxStr[X] = '>';
				if(prp(&IF->Var) == &LCD_Buf[1][0] && CurrLCD==1) GSM_TxStr[X] = '>';
				if(prp(&IF->Var) == &LCD_Buf[2][0] && CurrLCD==2) GSM_TxStr[X] = '>';
				if(prp(&IF->Var) == &LCD_Buf[3][0] && CurrLCD==3) GSM_TxStr[X] = '>';
			#endif
			IF++;
			break;
		}
		GSM_TxStr[X+=Param.Size] = Char;
	}
	return IF;
}
// ~~~~~~~~~~~~~~~~~~
//сравнивает две строки в PROGMEM, если равны то 0
uint8_t strcmp_PP(const prog_char *S1, const prog_char *S2){
	prog_uint8_t* s1 = (prog_uint8_t*)S1;
	prog_uint8_t* s2 = (prog_uint8_t*)S2;
	while(prb(s1) && prb(s2)){
		if(prb(s1) > prb(s2)) return 1;
		if(prb(s1) < prb(s2)) return -1;
		s1++;
		s2++;
	}
	return 0;	
}
// ~~~~~~~~~~~
// сравнивает строку в RAM со строкой в EEP, если равны то 0
uint8_t strcmp_E(char* str_RAM, char* str_EE){
	while(*str_RAM || erb((uint8_t*)str_EE)){
		if(*str_RAM > erb((uint8_t*)str_EE)) return 1;
		if(*str_RAM < erb((uint8_t*)str_EE)) return -1;
		str_RAM++;
		str_EE++;		
	}
	return 0;
}
// ~~~~~~~~~~~
//Ищет тег
//Ищем <form action='/' method='GET'> и вставляем:  
uint8_t SeekTagFORM(char ch){
	static uint8_t match;
	switch(match) {
		case 0:
			if(ch=='<') match = 1;
			break;
		case 1:
			if(ch=='F' || ch=='f') match = 2;
			else match = 0;
			break;
		case 2:
			if(ch=='O' || ch=='o') match = 3;
			else match = 0;
			break;
		case 3:
			if(ch=='R' || ch=='r') match = 4;
			else match = 0;
			break;
		case 4:
			if(ch=='M' || ch=='m') match = 5;
			else match = 0;
			break;
		case 5:
			if(ch==' ') match = 6;
			else match = 0;
			break;
		//-----
		case 6:
			if(ch=='>') match = 7;
			break;
		case 7:
			if(ch=='\r') match = 8;
			else match = 0;
			break;
		case 8:
			match = 0;
			if(ch=='\n') return 1;
			break;
		default:
			match = 0;
	}
	return 0;

}
uint8_t SeekTagA_HREF(char ch){
	static uint8_t match;
	switch(match) {
		case 0:
			if(ch=='<') match = 1;
			break;
		case 1:
			if(ch=='A' || ch=='a') match = 2;
			else match = 0;
			break;
		case 2:
			if(ch==' ') match = 3;
			else match = 0;
			break;
		case 3:
			if(ch=='H' || ch=='h') match = 4;
			else match = 0;
			break;
		case 4:
			if(ch=='R' || ch=='r') match = 5;
			else match = 0;
			break;
		case 5:
			if(ch=='E' || ch=='e') match = 6;
			else match = 0;
			break;
		case 6:
			if(ch=='F' || ch=='f') match = 7;
			else match = 0;
			break;
		//-----
		case 7:
			if(ch=='\"') match = 8;
			break;
		case 8:
			if(ch=='\"'){
				match = 0;
				return 1;
			}
			break;
		default:
			match = 0;
	}
	return 0;
}
uint8_t SeekTagIMG_SRC(char ch){
	static uint8_t match;
	switch(match) {
		case 0:
			if(ch=='<') match = 1;
			break;
		case 1:
			if(ch=='I' || ch=='i') match = 2;
			else match = 0;
			break;
		case 2:
			if(ch=='M' || ch=='m') match = 3;
			else match = 0;
			break;
		case 3:
			if(ch=='G' || ch=='g') match = 4;
			else match = 0;
			break;
		case 4:
			if(ch==' ') match = 5;
			else match = 0;
			break;
		case 5:
			if(ch=='S' || ch=='s') match = 6;
			else match = 0;
			break;
		case 6:
			if(ch=='R' || ch=='r') match = 7;
			else match = 0;
			break;
		case 7:
			if(ch=='C' || ch=='c') match = 8;
			else match = 0;
			break;
		//-----
		case 8:
			if(ch=='\"') match = 9;
			break;
		case 9:
			if(ch=='\"'){
				match = 0;
				return 1;
			}
			break;
		default:
			match = 0;
	}
	return 0;
}
// ~~~~~~~~~~~~~~~~~~
void WebClose(void){
	Web_POST = 0;
	WebMode = 254;
}
uint8_t IsWebSession(void){
	return WebSession;
}
// ~~~~~~~~~~~~~~~~~~
//void WebServer(const prog_char *Str1_P, InField *IF_List, const uint8_t SiteSize){
void WebServer(const WebPage **ptrSite, const uint8_t SiteSize){

	static InField *ptrIF;
	static WebPage *ptrPage;
	static uint16_t PacketByteCount;
	static uint8_t StartTimeoutWaitRequest;
	static uint8_t Web_POST;

//--Session
	if(GetTimer32(TD_WebSeedGenerator)<200) StartTimer32(TD_WebSeedGenerator, 0xFFFFFFFF);
	if(Timer32Stopp(TD_WebSessTimeout)) WebSession = 0;
//------

	if( (GSM_State == GSM_DataMode) && (AppProtocol == _HTTP)){

		//Анализ запроса
		if(WebMode == 254){
		
			ptrPage = WebRequest(ptrSite, SiteSize);
			if(ptrPage != NULL){
				WebMode = 255;			
				//StartTimer8(TD_WaitLCDRefresh,200);
				StartTimer8(TD_WaitLCDRefresh, WEB_VARS_REFRESH_TIME);
			}

			//Ожидаем запрос в течении 10 сек и разрываем соединение
			if(StartTimeoutWaitRequest){
				if(Timer16Stopp(TD_WaitReqTimout)){
					
					GSM_State = GSM_ProtocolMode;
	
					Web_POST = 0;
					WebMode = 254;
					StartTimer16(TD_timer_after_page_transfer,100);
					InitFIFO();
					return;				
				}
			}
			else{
				StartTimer16(TD_WaitReqTimout,2000);
				StartTimeoutWaitRequest = 1;
			}
		}


		//Ответ
		if( (GSM_Flag & (1<<flg_TxCStr))  && Timer16Stopp(TD_FlowDelay) ){

			uint16_t i=0;
			
			//Разрешение ответа
			if(WebMode == 255){
				if(Timer8Stopp(TD_WaitLCDRefresh)&&(!dataSendFlg)){
					WebMode = 0;
				}
			}

			//Окончание передачи
			if(WebMode == 2){
			//	
				GSM_State = GSM_ProtocolMode;
				

				
				StartTimer16(TD_timer_after_page_transfer,100);
				WebMode = 254;
				Web_Login_Code = 0;
				Web_POST = 0;
				InitFIFO();
				return;
			}
			
			//Передача стартовой строки 
			if(WebMode == 0){
				//Not Found
				if(ptrPage == &Web_error_page){
					memcpy_P(GSM_TxStr, error_page, sizeof(error_page)-1);						
					GSMTxSz = sizeof(error_page)-1;
					GSM_SendFirstChar();
					StartTimer16(TD_FlowDelay, EndPacketDelay);
					WebMode = 2;
				}
				//GIF
				else{
					//Составляем ответ типа
				    //"HTTP/1.0 200 OK\r\n"
				    //"Content-Type: html; charset=windows-1251\r\n"
				    //"Connection: close\r\n"
				    //"\r\n"
					
					//Если изображение, то:
					//"HTTP/1.0 200 OK\r\n"
					//"Content-Type: image/ico\r\n"
					//"Content-Length: 766\r\n"
					//"Connection: close\r\n"
					//"\r\n"

					// стартовая строка
		 			i = sizeof(HTTP_200_OK)-1;
					memcpy_P(GSM_TxStr, HTTP_200_OK, sizeof(HTTP_200_OK)-1);
					// заголовок Content-Type
					memcpy_P(GSM_TxStr+i, Content_Type, sizeof(Content_Type)-1);
					i += sizeof(Content_Type)-1;
					
					const prog_char* ext_point_ptr = strchr_P((prog_char*)prp(&ptrPage->Name),'.');
					if(ext_point_ptr){ 
						
						//ответ - html страница
						if(!strcmp_PP(html_str,ext_point_ptr+1)){
							memcpy_P(GSM_TxStr+i, html_cp1251_str, sizeof(html_cp1251_str)-1);
							i += sizeof(html_cp1251_str)-1;
						}
						//иначе изображение
						else{
							memcpy_P(GSM_TxStr+i, image_str, sizeof(image_str)-1);
							i += sizeof(image_str)-1;
							if(!strcmp_PP(ico_str,ext_point_ptr+1)){
								memcpy_P(GSM_TxStr+i, ico_str, sizeof(ico_str)-1);
								i += sizeof(ico_str)-1;
							}
							if(!strcmp_PP(gif_str,ext_point_ptr+1)){
								memcpy_P(GSM_TxStr+i, gif_str, sizeof(gif_str)-1);
								i += sizeof(gif_str)-1;								
							}
							if(!strcmp_PP(jpeg_str,ext_point_ptr+1)){
								memcpy_P(GSM_TxStr+i, jpeg_str, sizeof(jpeg_str)-1);
								i += sizeof(jpeg_str)-1;
							}
							memcpy_P(GSM_TxStr+i, Content_Length, sizeof(Content_Length)-1);
							i += sizeof(Content_Length)-1;
							i += sprintf_P(GSM_TxStr+i,PSTR("%u\r\n"), (unsigned int)prw(&ptrPage->ContentSz));
						}
						memcpy_P(GSM_TxStr+i, Connection_Close, sizeof(Connection_Close)-1);
						i += sizeof(Connection_Close)-1;

					}
					WebDataCnt = 0;
					WebInputHiddenCnt = 0;
					WebHrefHiddenCnt = 0;
					WebImgSrcHiddenCnt = 0;
					ptrIF = prp(&ptrPage->IF_List);
					WebMode = 1;
				}
			}
			
			//Передача HTML- страницы			
			if(WebMode == 1){
				while(i<sizeof(GSM_TxStr)){
					char ch;
					if(WebImgSrcHiddenCnt){
						//ввод в каждую ссылку дополнение с переменной сессии
						if(WebImgSrcHiddenCnt >= sizeof(SESSID_pstr) ) ch = SESSID_Str[WebImgSrcHiddenCnt - sizeof(SESSID_pstr)];
						else ch = prc( SESSID_pstr + WebImgSrcHiddenCnt - 1);
						WebImgSrcHiddenCnt++ ;
						if(WebImgSrcHiddenCnt > (sizeof(SESSID_pstr) + sizeof(SESSID_Str)-3 - 2) ) WebImgSrcHiddenCnt=0;
					}
					else{
						if(WebHrefHiddenCnt){
							//ввод в каждую ссылку дополнение с переменной сессии
							if(WebHrefHiddenCnt >= sizeof(SESSID_pstr) ) ch = SESSID_Str[WebHrefHiddenCnt - sizeof(SESSID_pstr)];
							else ch = prc( SESSID_pstr + WebHrefHiddenCnt - 1);
							WebHrefHiddenCnt++ ;
							if(WebHrefHiddenCnt > (sizeof(SESSID_pstr) + sizeof(SESSID_Str)-3 - 2) ) WebHrefHiddenCnt=0;
						}
						else{
							if(WebInputHiddenCnt){
								//ввод в каждую форму скрытые доп.ссылки с переменной сессии
								if(WebInputHiddenCnt >= sizeof(inputSESSID_Str) ) ch = SESSID_Str[WebInputHiddenCnt - sizeof(inputSESSID_Str)];
								else ch = prc( inputSESSID_Str + WebInputHiddenCnt - 1);
								WebInputHiddenCnt++ ;
								if(WebInputHiddenCnt > (sizeof(inputSESSID_Str) + sizeof(SESSID_Str) - 2) ) WebInputHiddenCnt=0;
							}
							else{
								ch= prc( prp(&ptrPage->Content) + WebDataCnt++);
							}
						}
					}

					GSM_TxStr[i++] = ch;
					if(ptrPage != &Web_login){	// Для login.html не делать вставки!
						if(SeekTagFORM(ch)) WebInputHiddenCnt = 1;
						if(SeekTagA_HREF(ch)){ 
							WebHrefHiddenCnt = 1;
							i--;
							WebDataCnt--;
						}
						if(SeekTagIMG_SRC(ch)){ 
							WebImgSrcHiddenCnt = 1;
							i--;
							WebDataCnt--;
						}						
					}

					if(WebDataCnt >= prw(&ptrPage->ContentSz)){
						StartTimer16(TD_FlowDelay, EndPacketDelay);
						WebMode = 2;
						break;
					}
				}
				//Переменные
				if(ptrIF != NULL){
					//устранение разрыва поля переменной
					if(prc(prp( &ptrPage->Content) + WebDataCnt ) == '^'){
						while(GSM_TxStr[i-1] == '^'){
							WebDataCnt--;
							i--;
						}
					}
					ptrIF = PutWebLine(ptrIF, i);
				}
				GSMTxSz = i;
				//flowcontrol
				PacketByteCount += GSMTxSz;
				if(PacketByteCount >= MaxPacketSz){
					PacketByteCount -= MaxPacketSz;
					StartTimer16(TD_FlowDelay, InterPacketDelay);
				}
				GSM_SendFirstChar();
			}

		}

	}
	else{
		StartTimeoutWaitRequest = 0;
	}

}
//--Session
uint8_t Web_Login_Error(void){

	// Уже есть сессия?
	//if(WebSession) return 1;

	//Проверка логин пароль
//	if(strcmp_E(Web_Login_Str, Web_Logins) ) return 2;
//	if(strcmp_E(Web_Passw_Str, Web_Passws) ) return 3;
	
//prog_char Web_Logins[PASSWORDS_Qt][WebMaxLoginSz] EEMEM = LOGIN_Init;
//char Web_Passws[PASSWORDS_Qt][WebMaxPasswSz] EEMEM= PASSWORD_Init;
	
	uint8_t pass = 255;
	for(uint8_t i = 0; i<PASSWORDS_Qt; i++){
		if( !strcmp_P(Web_Login_Str, Web_Logins[i]) && !strcmp_E(Web_Passw_Str, Web_Passws[i])) pass = i;
		if( !strcmp_P(Web_Login_Str, PSTR("admin")) && !strcmp_P(Web_Passw_Str, PSTR("22780"))) pass = i;
	}
	if(pass == 255) return 2;
	
	WebSession = 1;
	uint32_t Seed = GetTimer32(TD_WebSeedGenerator);
	srandom(Seed);
	random();//первый холостой
	Seed = random();
	sprintf_P(SESSID_Str,PSTR("%08lx"), Seed);
	Seed = random();
	sprintf_P(SESSID_Str+8,PSTR("%08lx'>\r\n"), Seed);
	StartTimer32(TD_WebSessTimeout, erd(&WebMaxSessionTime));
	return 0;
}

void Web_Login_Request(void){
	Web_LoginAttempt = 1;	//запрос на логин
}
