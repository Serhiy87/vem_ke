/*
	menu.h - меню для БАУ с клавиатурой Пуск, Стоп, Вверх, Вниз, Влево, Вправо.

	Desripton: 
	Меню выводит MenuOut, которая использует PutLine - вывод строки в LCD_Buf.
	В PutLine осуществляется подмена {{{ на переменные - вызывается PutField
	Также в PutLine осуществляется подмена }}} на переменные. здесь два варианта 
	Если не редактируется переменная - вызывается та же PutField
	Если редактируется - вызывается та же PutBuf или PutBufStr, взависимости от типа переменной

	В обработчиках MenuEnterRight, MenuEscLeft, MenuUp, MenuDown выполняется навигация по меню 
	и изменение переменных. В начале ввода значение переменный копируется в буфер функция Field2Buf.
	При окончании ввода значение буфера переносится в переменую функция Buf2Field.
	Операции сдвига позицию мигающего символа, изменение мигающего символа, переход на следующую переменную
	содержатся в данных обработчиках.

	Используются такие глобальные переменные:
	CurrPage - указатель на текущую структуру MenuPage
	CurrLine - текущая строка меню (там где стоит курсор) Зависит от размера меню. Отсчёт с 0.
	CurrField - текущая редактируемая переменная. Отсчёт с 0. Если CurrField==NullPos - нет ввода сейчас.
	CurrPos - текущая позиция в редактируемой переменной. Отсчёт с N..0.N = размер перменной -1.Если NullPos - нет ввода сейчас.
	CurrLCD - текущая строка индикатора (там где стоит курсор). Зависит от размера индикатора. Для 4х20 CurrLCD = 0..3 

	01.11.2011 Kononenko 
	Add  define new variable type xx_Longint (read only!)
	Add  define new variable type FFloat and GFloat (read only!)

	07.03.2012 Kononenko 
	Use strlcpy insteat strcpy to avoid LCD_Buf overrange for Text variable

	06.12.2012 Kononenko add EE_Text. Text, EE_Text OutField width==12 only!

	11.02.2013 Kononenko
	Если структура MenuLine[] менее размеров экрана - дополняется пробелами - для экономии PROGMEM

	06.03.2013 Kononenko
	Fix problem with Multi - in MenuUp, MenuDown
	Add check:	if(CurrField!=NullPos && CurrPos!=NullPos)


	Спецкомбинация для выравнивания строки 
	<> по центру 
	<< по левому краю
	>> по правому краю 
	Строка дополняется пробелами автоматически.

	26.03.2013 Кононенко
	Modify EE_Text. Text OutField any width!
	Create uint8_t GetCurrFieldSize(void)
	Учитывается только ограничения DIGIT (0..9), остальные игнорируются.
	Если строка меньше поля ввода/вывода
		При выводе дополняется пробелами и выравнивается по правому краю
		При редактировании дополняется пробелами и выравнивается по левому краю
	Если строка больше поля ввода/вывода - выводятся первые символы, далее обрезается.
	При сохранении содержимое BufStr копируется в строку "как есть" - длина строки не меняется.
	ВНИМАНИЕ! Длина буфера копирования задаётся длиной строки - т.е. если строка>LCDXsz символа - в конце будет мусор!

	28.03.2013 Kononenko
	Add OutField for Types:	Longint,EE_Longint,s_Longint,s_EE_Longint,z_Longint,z_EE_Longint,FFloat,EE_FFloat,GFloat,EE_GFloat
	BufStr is not empty. Init BufStr by current var value.

	12.06.2013 Kononenko
	Изменён приоритет в обработчике MenuEnter: сначало редактирование переменных, потом переход в др.меню

	27.03.2015 Kononenko
	Добавил вывод в hex-формате zh_Byte, zh_Word, zh_Longint

	08.08.2016 Potapov
	Добавил CurrX, StrSize, ExtBlink1, Y1; для работы с удаленным дисплеем в Listener2. Теперь символы заменяются нижними пробелами в lcd.h, непосредственно перед выводом
	на экран.

	30.03.2017 Kucherenko - Go AVRStudio4 on AtmelStudio7:
	typedef struct {...} PROGMEM InField; -> typedef const struct {...} PROGMEM InField;
	... All variables PROGMEM -> ...const ... PROGMEM
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "digiproc.h"
#include "multilang.h"

enum {
	Byte,	EE_Byte,	s_Byte,		s_EE_Byte,		z_Byte,		z_EE_Byte,
	zh_Byte,
	Word,	EE_Word,	s_Word,		s_EE_Word,		z_Word,		z_EE_Word,		c_Word,		zc_Word,
	zh_Word,
	Longint,EE_Longint,	s_Longint,	s_EE_Longint,	z_Longint,	z_EE_Longint,	c_Longint,	zc_Longint,
	zh_Longint,
	FFloat,	EE_FFloat, 	GFloat,		EE_GFloat,
	Enum,	EE_Enum,
	Bit,	EE_Bit,
	Text,	EE_Text
};

typedef void(*MenuEvent)(void);
typedef uint8_t (*MenuKey)(void);

typedef const struct {
	uint8_t Type;
	volatile void *Var;
	PGM_P const *EnumList;
	uint8_t Prec;
} PROGMEM InField;

typedef const struct {
	uint8_t Type;
	void *Var;
	uint32_t Min, Max;
	MenuEvent Act;
	PGM_P const *EnumList;
	uint8_t Prec;
} PROGMEM OutField;

typedef const struct {
	char Str[LCDXSz+1];
	InField *IF_List;
	OutField *OF_List;
	const struct MenuStruct *InnPage;
} PROGMEM MenuLine;
typedef const struct MenuStruct {
	MenuLine *LineList;
	uint8_t LineNumber, FixNumber;
	const struct MenuStruct *OutPage;
	MenuEvent Load, Run, Exit;
	MenuKey Enter, Esc, Up, Down;
} PROGMEM MenuPage;

typedef const struct {
	char Str[LCDXSz+1];
	InField *IF_List;
} PROGMEM MsgLine;
typedef const struct {
	MsgLine LineList[LCDYSz];
	MenuEvent Load, Run, Exit;
} PROGMEM MsgPage;

enum {StandartKey, CustomKey};

#define SetMenu(List) List,sizeof(List)/sizeof(MenuLine)

#define NullPos 0xFF

typedef void(*T_KeyAct)(void);

extern MenuPage InitMenu;
MenuPage *CurrPage = &InitMenu;
uint8_t CurrLine, CurrField=NullPos, CurrPos=NullPos, CurrLCD, CurrX, StrSize, Y1;
//uint8_t ExtBlink;

MenuPage *NextPage;

static MsgPage *CurrMsg;

#ifdef InitMsg
	extern MsgPage InitMsg;
#endif

uint8_t MenuDepth;
struct {
	MenuPage *Page;
	uint8_t LCD, Line;
} MenuStack[10];

uint8_t Blink;


// ~~~~~~~~~~~


static uint32_t F_Buf;
static int32_t sF_Buf;
static uint8_t Minus;
// ~~~~~~~~~~~
char BufStr[LCDXSz+1];
uint8_t CurrFieldSize;

void InitBufStr(OutField *Field){
	for(uint8_t i =0; i<LCDXSz; i++) BufStr[i]=' ';
	BufStr[LCDXSz] ='\0';
	CurrFieldSize = 0;
	MultiSymbol = 0;
}

// ~~~~~~~~~~~
#define GetFix() prb(&CurrPage->FixNumber)
#define GetLine (MenuLine*)prp(&CurrPage->LineList)
#define GetField ((OutField*)prp(&(GetLine+CurrLine)->OF_List)+CurrField)

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
EventFunc(const MenuEvent *Event)
{
	MenuEvent Func = prp(Event);
	if (Func)
		Func();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~
static uint8_t
KeyFunc(const MenuKey *Key)
{
	MenuKey Func=prp(Key);
	return Func && Func()==CustomKey;
}

// ~~~~~~~~~~~~~~~~~~~~~
void
GotoMenu(MenuPage *Menu)
{
	EventFunc(&CurrPage->Exit);
	if(prp(&Menu->OutPage))
		MenuDepth=0;
	else {
		MenuStack[MenuDepth].Page = CurrPage;
		MenuStack[MenuDepth].Line = CurrLine;
		MenuStack[MenuDepth++].LCD = CurrLCD;
	}
	CurrPage = Menu;
	CurrLine = CurrLCD = GetFix();
	CurrField = CurrPos = NullPos;
	EventFunc(&CurrPage->Load);
}

// ~~~~~~~~~~
uint8_t
HideMsg(void)
{
	if (CurrMsg) {
		EventFunc(&CurrMsg->Exit);
		CurrMsg = NULL;
		return 1;
	}
	else
		return 0;
}

// ~~~~~~~~~~~~~~~~~~
void
ShowMsg(MsgPage *Msg)
{
	HideMsg();
	CurrMsg = Msg;
	EventFunc(&CurrMsg->Load);
}

// ~~~~~~~~~~~
void
MenuInit(void)
{
	CurrLCD = CurrLine = GetFix();
	#ifdef InitMsg
		ShowMsg(&InitMsg);
	#endif
	EventFunc(&CurrPage->Load);
}

typedef struct {
	char *Pos;
	uint8_t Prec, Size;
	PGM_P const *Txt;
} T_Param;

// ~~~~~~~~~~~~~~~~~~~~
void
PutEnum(T_Param *Param)
{
	strcpy_P(Param->Pos, prp(Param->Txt));
	for(uint8_t i=strlen(Param->Pos); i<Param->Size; *(Param->Pos+i++)=' ');
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
PutVal(uint16_t Val, char Type, T_Param *Param)
{
	uint16_t Idx = (Type=='w'?0xFFFF:0xFF)-Val;
	if(Param->Txt && Idx<2) {
		Param->Txt += Idx;
		PutEnum(Param);
		return;
	}
	char Format[] = "% u.%. u";
	Format[1] = '0'+Param->Size;
	if(Param->Prec) {
		Format[1] -= Param->Prec+1;
		Format[6] = '0'+Param->Prec;
		uint16_t Div=Pow10(Param->Prec);
		sprintf(Param->Pos, Format, Val/Div, Val%Div);
	}
	else {
		Format[3] = '\0';
		sprintf(Param->Pos, Format, Val);
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
Put_sVal(int16_t Val, char Type, T_Param *Param)
{
	uint16_t Idx = (Type=='w'?0x8000:0x80)+Val;
	if(Param->Txt && Idx<2) {
		Param->Txt += Idx;
		PutEnum(Param);
		return;
	}
	char Format[] = "%+ d.%. u";
	if(Val==0)
		Format[1] = ' ';
	Format[2] = '0'+Param->Size;
	if(Param->Prec) {
		Format[2] -= Param->Prec+1;
		Format[7] = '0'+Param->Prec;
		int16_t Div=Pow10(Param->Prec);
		sprintf(Param->Pos, Format, Val/Div, (uint16_t)abs(Val)%Div);
		if(Val<0) {
			while(*Param->Pos==' ')
				Param->Pos++;
			*Param->Pos = '-';
		}
	}
	else {
		Format[4] = '\0';
		sprintf(Param->Pos, Format, Val);
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
Put_zVal(uint16_t Val, char Type, T_Param *Param)
{
	uint16_t Idx = (Type=='w'?0xFFFF:0xFF)-Val;
	if(Param->Txt && Idx<2) {
		Param->Txt += Idx;
		PutEnum(Param);
		return;
	}
	char Format[] = "%0 u.%. u";
	Format[2] = '0'+Param->Size;
	if(Param->Prec) {
		Format[2] -= Param->Prec+1;
		Format[7] = '0'+Param->Prec;
		uint16_t Div=Pow10(Param->Prec);
		sprintf(Param->Pos, Format, Val/Div, Val%Div);
	}
	else {
		Format[4] = '\0';
		sprintf(Param->Pos, Format, Val);
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
Put_zVal_h(uint32_t Val, char Type, T_Param *Param)
{
	if(Type=='b') sprintf(Param->Pos, "%02hx", (uint8_t)Val);
	if(Type=='w') sprintf(Param->Pos, "%04x", (uint16_t)Val);
	if(Type=='l') sprintf(Param->Pos, "%08lx", Val);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// PutDVal - print unsigned Longint in Buffer 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
PutDVal(uint32_t Val, T_Param *Param)
{
	uint32_t Idx = 0xFFFFFFFF-Val;
	if(Param->Txt && Idx<2) {
		Param->Txt += Idx;
		PutEnum(Param);
		return;
	}

	if(Param->Size==10 && Param->Prec==0){
		sprintf(Param->Pos, "%10lu", Val);
		return;
	}
	char Format[] = "% lu.%. lu";
	Format[1] = '0'+Param->Size;
	if(Param->Prec) {
		Format[1] -= Param->Prec+1;
		Format[7] = '0'+Param->Prec;
		uint32_t Div=PowL10(Param->Prec);
		sprintf(Param->Pos, Format, Val/Div, Val%Div);
	}
	else {
		Format[4] = '\0';
		sprintf(Param->Pos, Format, Val);
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Put_sDVal - print signed Longint in Buffer 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
Put_sDVal(int32_t Val, T_Param *Param)
{
	uint32_t Idx = 0x80000000+Val;
	if(Param->Txt && Idx<2) {
		Param->Txt += Idx;
		PutEnum(Param);
		return;
	}
	// standart specificator to print Longint in string
	if(Param->Size>9){
		char Format[] = "%+1 ld";
		Format[3] = '0'+Param->Size%10;
		sprintf(Param->Pos, Format, Val);
	}
	else{
		char Format[] = "%+ ld";
		Format[2] = '0'+Param->Size;
		sprintf(Param->Pos, Format, Val);
	}
	//Delete sign if zero
	if(!Val)Param->Pos[Param->Size-2] = ' ';

	if(Param->Prec){
		//Insert decimal point
		for(uint8_t i=0; i< Param->Size - Param->Prec; i++){
			Param->Pos[i] = Param->Pos[i+1];
		}
		Param->Pos[Param->Size - Param->Prec-1] = '.';
		//Fill zeros and shift sign for correct view
		char c = Param->Pos[Param->Size - Param->Prec-2];
		if(c ==' ' || c =='+' || c =='-'){
			for(uint8_t i=Param->Size-1; i>1; i--){
				if(Param->Pos[i] == ' '|| Param->Pos[i] == '+' || Param->Pos[i] == '-' ) Param->Pos[i]='0';
				if(Param->Pos[i] == '.'){
					Param->Pos[i-1]='0';
					if(Val>0)Param->Pos[i-2]='+';
					if(Val<0)Param->Pos[i-2]='-';
					return;
				}
			}
		}
	}

}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Put_zDVal - print unsigned Longint with leading zeroes in Buffer 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void 
Put_zDVal(uint32_t Val, T_Param *Param)
{
	uint32_t Idx = 0xFFFFFFFF-Val;
	if(Param->Txt && Idx<2) {
		Param->Txt += Idx;
		PutEnum(Param);
		return;
	}

	if(Param->Size==10 && Param->Prec==0){
		sprintf(Param->Pos, "%010lu", Val);
		return;
	}
	char Format[] = "%0 lu.%. lu";
	Format[2] = '0'+Param->Size;
	if(Param->Prec) {
		Format[2] -= Param->Prec+1;
		Format[8] = '0'+Param->Prec;
		uint32_t Div=PowL10(Param->Prec);
		sprintf(Param->Pos, Format, Val/Div, Val%Div);
	}
	else {
		Format[5] = '\0';
		sprintf(Param->Pos, Format, Val);
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Put_FVal -  The double argument is rounded and converted to decimal notation in the format "[-]ddd.ddd"
// Max presicion - 8digits Why???
// Therefore Max Field size - 9
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Put_FFVal(float Val, T_Param *Param){
	char Format[] = "%+ . f";
	Format[2] = '0'+Param->Size;
	Format[4] = '0'+Param->Prec;	
	sprintf(Param->Pos, Format, Val);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Put_GFVal -  The double argument is converted in style f or e (or F or E for G conversions). 
// The precision specifies the number of significant digits. 
// If the precision is missing, 6 digits are given; if the precision is zero, it is treated as 1.
// Style e is used if the exponent from its conversion is less than -4 or greater than or equal to the precision.
// Trailing zeros are removed from the fractional part of the result; 
// a decimal point appears only if it is followed by at least one digit.
// Max presicion - 8digits Why???
// Therefore Max Field size - 9
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Put_GFVal(float Val, T_Param *Param){
	if(Param->Size < 10){
		char Format[] = "%+ . g";
		Format[2] = '0'+Param->Size;
		Format[4] = '0'+Param->Prec;
		sprintf(Param->Pos, Format, Val);
	}
	else{
		char Format[] = "%+1 . g";
		Format[3] = '0'+Param->Size-10;	
		Format[5] = '0'+Param->Prec;
		sprintf(Param->Pos, Format, Val);
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Put_TextVal -  
// if, for example, Filed width =12 -> sprintf(Param->Pos,"%12.12s",Val)
// min and max width = Param->Size and align right
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Put_TextValAlignRight(char* Val, T_Param *Param){
	char Format[8];
	//char Format[8] = "%10.10s";
	sprintf(Format,"%%%u.%us", Param->Size, Param->Size);
	sprintf(Param->Pos, Format, Val);
}
void Put_TextValAlignLeft(char* Val, T_Param *Param){
	char Format[9];
	//char Format[8] = "%10.10s";
	sprintf(Format,"%%-%u.%us", Param->Size, Param->Size);
	sprintf(Param->Pos, Format, Val);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
PutField(uint8_t Type, void *Var, T_Param *Param)
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
		Put_TextValAlignRight(Var, Param);
		break;
	case EE_Text:{
			char TempStr[LCDXSz+1];
			erbl(TempStr, Var, Param->Size);
			Put_TextValAlignRight(TempStr, Param);
		}
		break;
	}
}

// ~~~~~~~~~~~~~~~~~~~~~
void
Put_bVal(T_Param *Param)
{
	char Format[] = "%0 li.%. u";
	Format[2] = '0'+Param->Size;
	if(Param->Prec) {
		Format[2] -= Param->Prec+1;
		Format[8] = '0'+Param->Prec;
		uint16_t Div=Pow10(Param->Prec);
		sprintf(Param->Pos, Format, F_Buf/Div, F_Buf%Div);
	}
	else{
		Format[5] = '\0';
		sprintf(Param->Pos, Format, F_Buf);
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~
void
Put_sbVal(T_Param *Param)
{
	char Format[] = "%0+ li.%. u";
	Format[3] = '0'+Param->Size;
	if(Param->Prec) {
		Format[3] -= Param->Prec+1;
		Format[9] = '0'+Param->Prec;
		int16_t Div=Pow10(Param->Prec);
		sprintf(Param->Pos, Format, sF_Buf/Div, (uint32_t)labs(sF_Buf)%Div);
		if(sF_Buf<0) {
			while(*Param->Pos==' ')
				Param->Pos++;
			*Param->Pos = '-';
		}
	}
	else {
		Format[6] = '\0';
		sprintf(Param->Pos, Format, sF_Buf);
	}
	if(Minus)
		*Param->Pos = '-';
}

// ~~~~~~~~~~~~~~~~~~~~~
static void
PosBlink(T_Param *Param, char Cursor)
{
	CurrX=Param->Pos+Param->Size-CurrPos-(Param->Prec&&CurrPos>=Param->Prec?2:1)-LCD_Buf[Y1];
	StrSize=1;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
ValBlink(char *Pos, uint8_t Sz)
{
	CurrX=Pos-LCD_Buf[Y1];
	StrSize=Sz; 
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline static void
PutBuf(uint8_t Type, T_Param *Param)
{
	switch(Type) {
	case Byte: case EE_Byte: case z_Byte: case z_EE_Byte:
	case Word: case EE_Word: case z_Word: case z_EE_Word:
		Put_bVal(Param);
		PosBlink(Param,' ');
		break;
	case s_Byte: case s_EE_Byte:
	case s_Word: case s_EE_Word:
		Put_sbVal(Param);
		PosBlink(Param,' ');
		break;
	case c_Word:
		PutVal(F_Buf, 'w', Param);
		ValBlink(Param->Pos, Param->Size);
		break;
	case zc_Word:
		Put_zVal(F_Buf, 'w', Param);
		ValBlink(Param->Pos, Param->Size);
		break;
	case Enum: case EE_Enum: case Bit: case EE_Bit:
		Param->Txt += (uint8_t)F_Buf;
		PutEnum(Param);
		ValBlink(Param->Pos, Param->Size);
		break;
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// When use BufStr for variable edit
inline static void
PutBufStr(uint8_t Type, T_Param *Param)
{	
	char Format[6];
	sprintf(Format,"%%.%us", Param->Size);
	sprintf(Param->Pos, Format, BufStr);
	Param->Prec = 0;
	PosBlink(Param,'_');
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
PutLine(uint8_t LineN, uint8_t Y)
{
	MenuLine *Line=GetLine+LineN;
	PGM_P CurrStr=(PGM_P)(Line->Str);
	InField *IF = prp(&Line->IF_List);
	OutField *OF=prp(&Line->OF_List);
	uint8_t OF_N = 0;

	uint8_t X;
	uint8_t X0=0;
	uint8_t X1=LCDXSz;
	int8_t Xs = 0;
	//--- ">>" Align right
	if(prc(CurrStr+0) == '>' && prc(CurrStr+1) == '>'){
		X0 = LCDXSz - strlen_P(CurrStr) + 2;
		Xs = X0-2;
		for(X=0; X<X0; X++) LCD_Buf[Y][X]=' ';
	}
	//--- "<<" Align left
	if(prc(CurrStr+0) == '<' && prc(CurrStr+1) == '<'){
		X1 = strlen_P(CurrStr) - 2;
		X0 = 0;
		Xs = X0-2;
	}
	//--- "<>" Align center
	if(prc(CurrStr+0) == '<' && prc(CurrStr+1) == '>'){
		X0 = (LCDXSz - strlen_P(CurrStr) + 2) / 2;
		Xs = X0-2;
		X1 = X0 + (strlen_P(CurrStr) - 2);
		for(X=0; X<X0; X++) LCD_Buf[Y][X]=' ';		
	}
	//---
	for(X=X0; X<X1; X++){
		char Char;
		T_Param Param;
		Param.Pos = LCD_Buf[Y]+X;
		Param.Size = 0;
		switch(Char=prc(CurrStr+X-Xs)) {
		case '{':
			while((Char=prc(CurrStr + X - Xs + ++Param.Size))=='{');
			Param.Prec = prb(&IF->Prec);
			Param.Txt = prp(&IF->EnumList);
			PutField(prb(&IF->Type), prp(&IF->Var), &Param);
			IF++;
			break;
		case '}':
			while((Char=prc(CurrStr+X - Xs + ++Param.Size))=='}');
			Y1=Y;
			Param.Prec = prb(&(OF+OF_N)->Prec);
			Param.Txt = prp(&(OF+OF_N)->EnumList);
			if(LineN==CurrLine && OF_N==CurrField)
				//add new
				if( (prb(&(OF+OF_N)->Type) == Text) || (prb(&(OF+OF_N)->Type) == EE_Text ) ||
					(prb(&(OF+OF_N)->Type) == Longint) || (prb(&(OF+OF_N)->Type) == z_Longint ) ||
					(prb(&(OF+OF_N)->Type) == EE_Longint) || (prb(&(OF+OF_N)->Type) == z_EE_Longint ) ||
					(prb(&(OF+OF_N)->Type) == s_Longint) || (prb(&(OF+OF_N)->Type) == s_EE_Longint ) ||
					(prb(&(OF+OF_N)->Type) == FFloat) || (prb(&(OF+OF_N)->Type) == GFloat ) ||
					(prb(&(OF+OF_N)->Type) == EE_FFloat) || (prb(&(OF+OF_N)->Type) == EE_GFloat )
				){
					PutBufStr(prb(&(OF+OF_N)->Type), &Param);
				}
				else{ 
					PutBuf(prb(&(OF+OF_N)->Type), &Param);
				}			
			else
				PutField(prb(&(OF+OF_N)->Type), prp(&(OF+OF_N)->Var), &Param);
			OF_N++;
			break;
		}
		LCD_Buf[Y][X+=Param.Size] = Char;

	}
	X=X1;
	for(;X<LCDXSz; X++) LCD_Buf[Y][X] = ' ';

}
// ~~~~~~~~~~~~~~~~~~
inline static void
PutMsgLine(uint8_t Y)
{
	#if TypeLCD==840
		const MsgLine *Line=(MsgLine*)prp(&CurrMsg->LineList)+Y;
	#else
		const MsgLine *Line=CurrMsg->LineList+Y;		
	#endif
	PGM_P CurrStr=(PGM_P)(Line->Str);
	InField *IF = prp(&Line->IF_List);

	uint8_t X;
	uint8_t X0=0;
	uint8_t X1=LCDXSz;
	int8_t Xs = 0;
	//--- ">>" Align right
	if(prc(CurrStr+0) == '>' && prc(CurrStr+1) == '>'){
		X0 = LCDXSz - strlen_P(CurrStr) + 2;
		Xs = X0-2;
		for(X=0; X<X0; X++) LCD_Buf[Y][X]=' ';
	}
	//--- "<<" Align left
	if(prc(CurrStr+0) == '<' && prc(CurrStr+1) == '<'){
		X1 = strlen_P(CurrStr) - 2;
		X0 = 0;
		Xs = X0-2;
	}
	//--- "<>" Align center
	if(prc(CurrStr+0) == '<' && prc(CurrStr+1) == '>'){
		X0 = (LCDXSz - strlen_P(CurrStr) + 2) / 2;
		Xs = X0-2;
		X1 = X0 + (strlen_P(CurrStr) - 2);
		for(X=0; X<X0; X++) LCD_Buf[Y][X]=' ';		
	}
	//---

	for(X=X0; X<X1; X++){
		char Char;
		if((Char=prc(CurrStr+X-Xs))=='{') {
			T_Param Param;
			Param.Pos = LCD_Buf[Y]+X;
			Param.Size = 0;
			while((Char=prc(CurrStr+X-Xs+ ++Param.Size))=='{');
			Param.Prec = prb(&IF->Prec);
			Param.Txt = prp(&IF->EnumList);
			PutField(prb(&IF->Type), prp(&IF->Var), &Param);
			IF++;
			X += Param.Size;
		}
		LCD_Buf[Y][X] = Char;
	}
	X=X1;
	for(;X<LCDXSz; X++) LCD_Buf[Y][X] = ' ';
}
// ~~~~~~~~~~~~~~~~~~
inline static void
PutBlankLine(uint8_t Y)
{
	uint8_t X=0;
	while(X<LCDXSz)	LCD_Buf[Y][X++] = ' ';
}
// ~~~~~~~~~~
void
MenuOut(void)
{
	uint8_t Y;
	if(CurrMsg) {
		for(Y=0; Y<LCDYSz; PutMsgLine(Y++));
		EventFunc(&CurrMsg->Run);
		return;
	}
	uint8_t Fix=GetFix();
	uint8_t Dist=CurrLine-CurrLCD;
	for(Y=0; Y<Fix; Y++)
		if(Y >= prb(&CurrPage->LineNumber))	PutBlankLine(Y);
		else PutLine(Y, Y);
	for(; Y<LCDYSz; Y++)
		if(Y >= prb(&CurrPage->LineNumber))	PutBlankLine(Y);
		else PutLine(Dist+Y, Y);
	if(prb(&CurrPage->LineNumber)>LCDYSz || Fix<LCDYSz-1 || prp(&(GetLine+CurrLine)->OF_List))
		LCD_Buf[CurrLCD][0] = CharCursor;
	if(++Blink>16)
		Blink=0;
	EventFunc(&CurrPage->Run);
}
// ~~~~~~~~~~~~~~~~~~~~~~~
// Return size of CurrField. Use to determine size of Text, EE_Text OutField vars
uint8_t GetCurrFieldSize(void){
	uint8_t Size;
	MenuLine *Line=GetLine+CurrLine;
	PGM_P CurrStr=(PGM_P)(Line->Str);
	uint8_t OF_N = 0;
	for(uint8_t X=0; X<LCDXSz; X++){
		Size = 0;
		while(prc(CurrStr+X)=='}'){
			X++;
			Size++;
		}
		if(Size){
			if(OF_N == CurrField) return Size;
			OF_N++;
		}
	}
	return 0;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// используется для задания MultiSymbol при вводе переменных
uint8_t GetMultiSymbol(PGM_P Str_P, char Char){
	PGM_P ptr_P= strchr_P(Str_P, Char);
	if(ptr_P) return (uint8_t)(ptr_P - Str_P);
	else return 0;
}
// ~~~~~~~~~~~~~~~~~~~~~~~
static void
Field2Buf(OutField *Field)
{
	void *Var = prp(&Field->Var);
	uint8_t BitN=prb(&Field->Prec), Mask=1<<BitN%8, *BitVar=(uint8_t*)Var+BitN/8;
	uint8_t Pos=Log10(prd(&Field->Max));
	uint8_t Oldstyle = 0;

	switch(prb(&Field->Type)) {
	case Byte: case z_Byte:
		F_Buf = *(uint8_t*)Var;
		CurrPos = Pos;
		Oldstyle = 1;
		break;
	case s_Byte:
		sF_Buf = *(int8_t*)Var;
		CurrPos = Pos+1;
		Oldstyle = 1;
		break;
	case EE_Byte: case z_EE_Byte:
		F_Buf = erb((uint8_t*)Var);
		CurrPos = Pos;
		Oldstyle = 1;
		break;
	case s_EE_Byte:
		sF_Buf = (int8_t)erb((uint8_t*)Var);
		CurrPos = Pos+1;
		Oldstyle = 1;
		break;

	case Word: case z_Word:
		F_Buf = *(uint16_t*)Var;
		CurrPos = Pos;
		Oldstyle = 1;
		break;
	case EE_Word: case z_EE_Word:
		F_Buf = erw((uint16_t*)Var);
		CurrPos = Pos;
		Oldstyle = 1;
		break;
	case s_Word:
		sF_Buf = *(int16_t*)Var;
		CurrPos = Pos+1;
		Oldstyle = 1;
		break;
	case s_EE_Word:
		sF_Buf = (int16_t)erw((uint16_t*)Var);
		CurrPos = Pos+1;
		Oldstyle = 1;
		break;
	case c_Word: case zc_Word:
		F_Buf = *(uint16_t*)Var;
		Oldstyle = 1;
		break;

	case Enum:
		F_Buf = *(uint8_t*)Var;
		Oldstyle = 1;
		break;
	case EE_Enum:
		F_Buf = erb((uint8_t*)Var);
		Oldstyle = 1;
		break;
	case Bit:
		F_Buf = (*BitVar &Mask) > 0;
		Oldstyle = 1;
		break;
	case EE_Bit:
		F_Buf = (erb(BitVar) &Mask) > 0;
		Oldstyle = 1;
		break;
	}

	Minus=0;
	if(Oldstyle) return;

	InitBufStr(Field);
	CurrFieldSize = GetCurrFieldSize();
	CurrPos = CurrFieldSize - 1;
	float fTemp;
	T_Param Param;
	Param.Size = CurrFieldSize;
	Param.Pos = BufStr;
	Param.Prec = prb(&Field->Prec);
	Param.Txt = NULL;

	switch(prb(&Field->Type)) {
	case Longint:
		PutDVal(*(uint32_t*)Var, &Param);
		MultiSymbol = GetMultiSymbol(Digits_P, BufStr[0]);
		break;
	case z_Longint:
		Put_zDVal(*(uint32_t*)Var, &Param);
		MultiSymbol = GetMultiSymbol(Digits_P, BufStr[0]);
		break;
	case EE_Longint: 
		PutDVal(erd((uint32_t*)Var), &Param);
		MultiSymbol = GetMultiSymbol(Digits_P, BufStr[0]);
		break;
	case z_EE_Longint:
		Put_zDVal(erd((uint32_t*)Var), &Param);
		MultiSymbol = GetMultiSymbol(Digits_P, BufStr[0]);
		break;
	case s_Longint:
		Put_sDVal(*(int32_t*)Var, &Param);
		MultiSymbol = GetMultiSymbol(Digits_P, BufStr[0]);
		break;
	case s_EE_Longint:
		Put_sDVal((int32_t)erd((uint32_t*)Var), &Param);
		MultiSymbol = GetMultiSymbol(Digits_P, BufStr[0]);
		break;


	case FFloat:
		Put_FFVal(*(float*)Var, &Param);
		MultiSymbol = GetMultiSymbol(Digits_P, BufStr[0]);
		break;
	case EE_FFloat:
		erbl(&fTemp, Var, 4);
		Put_FFVal(fTemp, &Param);
		MultiSymbol = GetMultiSymbol(Digits_P, BufStr[0]);
	case GFloat:
		Put_GFVal(*(float*)Var, &Param);
		MultiSymbol = GetMultiSymbol(Digits_P, BufStr[0]);
	case EE_GFloat:
		erbl(&fTemp, Var, 4);
		Put_GFVal(fTemp, &Param);
		MultiSymbol = GetMultiSymbol(Digits_P, BufStr[0]);
		break;


	case Text:
		Put_TextValAlignLeft((char*)Var, &Param);
		if( (prd(&Field->Min) == DIGIT) && (prd(&Field->Max) == DIGIT) )
			MultiSymbol = GetMultiSymbol(OnlyDigits_P, BufStr[0]);
		else MultiSymbol = GetMultiSymbol(Symbols_P, BufStr[0]);
		
		break;
	case EE_Text:{
			char TempStr[LCDXSz+1];
			erbl(TempStr, Var, CurrFieldSize);
			Put_TextValAlignLeft(TempStr, &Param);
			if( (prd(&Field->Min) == DIGIT) && (prd(&Field->Max) == DIGIT) )
				MultiSymbol = GetMultiSymbol(OnlyDigits_P, BufStr[0]);
			else MultiSymbol = GetMultiSymbol(Symbols_P, BufStr[0]);
		}
		break;
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~
// Check if variable in range for any type
//Kononenko add (ty_)Buf in SetField, SetEEField
#define SetField(Buf, ty)	\
	if(Buf>(ty)Max)					\
		*(ty*)Var = (ty)Max;	\
	else if(Buf<(ty)Min)		\
		*(ty*)Var = (ty)Min;	\
	else										\
		*(ty*)Var = Buf

#define SetEEField(Buf, ty1, ty2, ty3)	\
	if(Buf>(ty1)Max)											\
		ew##ty3((ty2*)Var, Max);						\
	else if(Buf<(ty1)Min)									\
		ew##ty3((ty2*)Var, Min);						\
	else																	\
		ew##ty3((ty2*)Var, Buf)

#define SetField32(Buf, ty)	\
	if((ty)Buf>(ty)Max)					\
		*(ty*)Var = (ty)Max;	\
	else if((ty)Buf<(ty)Min)		\
		*(ty*)Var = (ty)Min;	\
	else										\
		*(ty*)Var = (ty)Buf

#define SetEEField32(Buf, ty1, ty2, ty3)	\
	if((ty1)Buf>(ty1)Max)											\
		ew##ty3((ty2*)Var, Max);						\
	else if((ty1)Buf<(ty1)Min)									\
		ew##ty3((ty2*)Var, Min);						\
	else																	\
		ew##ty3((ty2*)Var, (ty2)Buf)
// ~~~~~~~~~~~~~~~~~~~~~~~
//Change BufStr due to precision for correct sscanf reading BufStr
void PresicionHanler(OutField *Field){
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
// ~~~~~~~~~~~~~~~~~~~~~~~
// Изменения и дополнения для посимвольно вводимых переменных
// для остальных переменных всё по-старому
static void
Buf2Field(OutField *Field)
{
	void *Var = prp(&Field->Var);
	uint8_t BitN=prb(&Field->Prec), Mask=1<<BitN%8, *BitVar=(uint8_t*)Var+BitN/8;
	uint8_t EE_BitVar;
	uint32_t Min=prd(&Field->Min), Max=prd(&Field->Max);

	uint32_t Temp=0;
	float fTemp=0;

	switch(prb(&Field->Type)) {
	case Byte: case z_Byte: case Enum:
		SetField(F_Buf, uint8_t);
		break;
	case s_Byte:
		SetField(sF_Buf, int8_t);
		break;
	case EE_Byte: case z_EE_Byte: case EE_Enum:
		SetEEField(F_Buf, uint8_t, uint8_t, b);
		break;
	case s_EE_Byte:
		SetEEField(sF_Buf, int8_t, uint8_t, b);
		break;
	case Word: case z_Word:
		SetField(F_Buf, uint16_t);
		break;
	case EE_Word: case z_EE_Word:
		SetEEField(F_Buf, uint16_t, uint16_t, w);
		break;
	case s_Word:
		SetField(sF_Buf, int16_t);
		break;
	case s_EE_Word:
		SetEEField(sF_Buf, int16_t, uint16_t, w);
		break;
	case c_Word: case zc_Word:
		*(uint16_t*)Var = F_Buf;
		break;


	case Longint: case z_Longint:
		//if(Limit_uByteVar(Temp)) sscanf(BufStr,"%ud", (unsigned short int*)&Temp);
		PresicionHanler(Field);
		sscanf(BufStr,"%lud", (uint32_t*)&Temp);
		SetField32(Temp, uint32_t);
		break;
	case EE_Longint: case z_EE_Longint:
		PresicionHanler(Field);
		if(sscanf(BufStr,"%lud", (uint32_t*)&Temp) == 1){
			SetEEField32(Temp, uint32_t, uint32_t, d);
		}		
		break;
	case s_Longint:
		PresicionHanler(Field);
		//if(Limit_sByteVar(Temp)) sscanf(BufStr,"%hd", (short int*)&Temp);
		sscanf(BufStr,"%ld", (int32_t*)&Temp);
		SetField32(Temp, int32_t);
		break;
	case s_EE_Longint:
		PresicionHanler(Field);
		if(sscanf(BufStr,"%ld", (int32_t*)&Temp) == 1){
			SetEEField32(Temp, int32_t, uint32_t, d);
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


	case Bit:
		if(F_Buf)
			*BitVar |=Mask;
		else
			*BitVar &=~Mask;
		break;
	case EE_Bit:
		EE_BitVar=erb(BitVar);
		ewb(BitVar, F_Buf ? EE_BitVar |Mask : EE_BitVar &~Mask);
		break;


	case Text:{
		//ограничение при вводе длинной строки
		uint8_t Field_width = strlen((char*)Var);
		if(Field_width > LCDXSz) Field_width = LCDXSz+1;
		memcpy(Var,BufStr,Field_width);
		}
		break;
	case EE_Text:{
			char TempStr[LCDXSz+1];
			erbl(TempStr, Var, LCDXSz+1);
			uint8_t Width = strlen(TempStr);
			memcpy(TempStr, BufStr, Width);
			ewbl(TempStr, Var, Width);
		}
		break;

	}
	EventFunc(&Field->Act);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~
static int32_t
Inc(int8_t Dir, uint32_t Buf)
{
	uint32_t Pos = PowL10(CurrPos);
	uint8_t Dig=Buf%(Pos*10)/Pos;
	if(Dig==(Dir==1?9:0)) {
			Dir = -Dir;
			Pos *= 9;
	}
	return Pos*Dir;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
MenuUD(const MenuKey *Key, int8_t Dir, uint8_t LimLine, uint8_t LimLCD, uint8_t LimVar)
{
	if(CurrMsg || KeyFunc(Key))
		return;
	if(CurrField==NullPos) {
		if(LimLine) {
			CurrLine -= Dir;
			if(LimLCD)
				CurrLCD -= Dir;
		}
		return;
	}
	Blink=0;
	OutField *Field=GetField;
	switch(prb(&Field->Type)) {
	case Byte: case EE_Byte: case z_Byte: case z_EE_Byte:
	case Word: case EE_Word: case z_Word: case z_EE_Word:
		F_Buf += Inc(Dir, F_Buf);
		break;
	case s_Byte: case s_EE_Byte:
	case s_Word: case s_EE_Word:
		if(CurrPos==Log10(prd(&Field->Max))+1) {
			if(sF_Buf==0)
				Minus = !Minus;
			else
				sF_Buf = -sF_Buf;
		}
		else if(sF_Buf>0 || sF_Buf==0 && !Minus)
			sF_Buf += Inc(Dir, sF_Buf);
		else if(sF_Buf<0 || sF_Buf==0 && Minus)
			Minus = (sF_Buf-=Inc(Dir, -sF_Buf))==0;
		break;
	case c_Word: case zc_Word:
		if(LimVar) {
			F_Buf += Dir;
			Buf2Field(Field);
			Field2Buf(Field);
		}
		break;
	case Enum: case EE_Enum:
		if(LimVar)
			F_Buf += Dir;
		break;
	case Bit: case EE_Bit:
		F_Buf = !F_Buf;
		break;
	case Text: case EE_Text:
		if( (prd(&Field->Min) == DIGIT) && (prd(&Field->Max) == DIGIT) ){
			if(Dir==1){	if(++MultiSymbol > 9) MultiSymbol = 0;}
			else{		if(--MultiSymbol == 255) MultiSymbol = 9;}
		}
		else{
			if(Dir==1){	if(++MultiSymbol >= strlen_P(Symbols_P)) MultiSymbol = 0;}
			else{		if(--MultiSymbol == 255) MultiSymbol = strlen_P(Symbols_P)-1;}
		}
		if(CurrPos!=NullPos)
			BufStr[CurrFieldSize - 1 - CurrPos] = prc(Symbols_P + MultiSymbol);
		break;
	case Longint: case z_Longint: case EE_Longint: case z_EE_Longint: case s_Longint: case s_EE_Longint:
	case FFloat: case EE_FFloat: case GFloat: case EE_GFloat:
		if(Dir==1)	{if(++MultiSymbol >= strlen_P(Digits_P)) MultiSymbol = 0;}
		else 		{if(--MultiSymbol == 255) MultiSymbol = strlen_P(Digits_P)-1;}
		if(CurrPos!=NullPos)
			BufStr[CurrFieldSize - 1 - CurrPos] = prc(Digits_P + MultiSymbol);
		break;
	}
}

// ~~~~~~~~~
void
MenuUp(void)
{
	uint8_t Fix=GetFix();
	MenuUD(&CurrPage->Up, 1, CurrLine>Fix, CurrLCD>Fix, F_Buf<prd(&GetField->Max));
}

// ~~~~~~~~~~~
void
MenuDown(void)
{
	MenuUD(&CurrPage->Down, -1, prb(&CurrPage->LineNumber)-CurrLine>1, CurrLCD<LCDYSz-1, F_Buf>prd(&GetField->Min));
}

// ~~~~~~~~~~~~
void
MenuRight(void)
{
}

// ~~~~~~~~~~~~~
void
MenuLeft(void)
{
}

// ~~~~~~~~~~~~
void
MenuEnter(void)
{
}

// ~~~~~~~~~~
void
MenuEsc(void)
{
}

// ~~~~~~~~~~~~~~~~~
void
MenuEnterRight(void)
{
//	1) Hide current Message
	if(HideMsg()||KeyFunc(&CurrPage->Enter))
		return;

	MenuLine *Line=GetLine+CurrLine;
	OutField *Field=prp(&Line->OF_List);

// 2) Variable
	if(Field){

	// Define variable quantity 
		uint8_t FieldNumber = 0;
		PGM_P CurrStr=(PGM_P)(Line->Str);
		char Char1, Char2=0;
		uint8_t i=0;
		while(i<LCDXSz) {
			Char1 = Char2;
			Char2 = prc(CurrStr+i++);
			if (Char2=='}' && Char1!='}')
				FieldNumber++;
		}
		// Для переменных типа Bit (Звонок:выкл) сразу производится изменение
		if(FieldNumber==1 && prb(&Field->Type)==Bit) {
			uint8_t BitN=prb(&Field->Prec);
			*((uint8_t*)prp(&Field->Var)+BitN/8) ^=(1<<BitN%8);
			EventFunc(&Field->Act);
			return;
		}

	// Сдвигаем позицию мигающей цифры
		if(CurrPos!=NullPos && --CurrPos!=NullPos){
			OutField *Field1=GetField;
			switch(prb(&Field1->Type)){
			case Text: case EE_Text:
				if( (prd(&Field1->Min) == DIGIT) && (prd(&Field1->Max) == DIGIT) )
					MultiSymbol = GetMultiSymbol(OnlyDigits_P, BufStr[CurrFieldSize - 1 - CurrPos]);
				else MultiSymbol = GetMultiSymbol(Symbols_P, BufStr[CurrFieldSize - 1 - CurrPos]);
				break;
			case Longint: case z_Longint: case EE_Longint: case z_EE_Longint: case s_Longint: case s_EE_Longint:
			case FFloat: case EE_FFloat: case GFloat: case EE_GFloat:
				MultiSymbol = GetMultiSymbol(Digits_P, BufStr[CurrFieldSize - 1 - CurrPos]);
				break;		
			}
			return;
		}

		if(CurrField!=NullPos) {
			Buf2Field(Field+CurrField);
			if(CurrField==NullPos)
				return;
		}
		// Переход к редактированию следующей переменной
		if(++CurrField==FieldNumber)
			CurrField=NullPos;
		else
			Field2Buf(Field+CurrField);
	}

// 3) Go to menu
	if( CurrLine >= prb(&CurrPage->LineNumber) )
		return;

	if(NextPage || (NextPage=prp(&Line->InnPage))) {
		GotoMenu(NextPage);
		NextPage=NULL;
		return;
	}
}

// ~~~~~~~~~~~~~~
void
MenuEscLeft(void)
{
//	1) Hide current Message
	if(HideMsg()||KeyFunc(&CurrPage->Esc))
		return;

// 2) Variable
	if(CurrField!=NullPos) {
		if(CurrPos!=NullPos) {
			OutField *Field=GetField;
			uint8_t Type=prb(&Field->Type);
			uint8_t Pos;

			if( (Type == Text) || (Type == EE_Text) ||
			 (Type == Longint) || (Type == EE_Longint)|| (Type == s_Longint) || (Type == s_EE_Longint) || (Type == z_Longint) || (Type == z_EE_Longint) ||
			 (Type == FFloat) || (Type == EE_FFloat) || (Type == GFloat) || (Type == EE_GFloat) ) 
			 	Pos = CurrFieldSize-1;
			else Pos=Log10(prd(&Field->Max));

			if(Type==s_Byte || Type==s_EE_Byte || Type==s_Word || Type==s_EE_Word)
				Pos++;
			if(++CurrPos>Pos)
				CurrPos = NullPos;
			else{
				switch(Type){
				case Text: case EE_Text:
					if( (prd(&Field->Min) == DIGIT) && (prd(&Field->Max) == DIGIT) )
						MultiSymbol = GetMultiSymbol(OnlyDigits_P, BufStr[CurrFieldSize - 1 - CurrPos]);
					else MultiSymbol = GetMultiSymbol(Symbols_P, BufStr[CurrFieldSize - 1 - CurrPos]);
					break;
				case Longint: case z_Longint: case EE_Longint: case z_EE_Longint: case s_Longint: case s_EE_Longint:
				case FFloat: case EE_FFloat: case GFloat: case EE_GFloat:
					MultiSymbol = GetMultiSymbol(Digits_P, BufStr[CurrFieldSize - 1 - CurrPos]);
					break;		
				}
				return;
			}
		}
		// Переход к редактированию предыдущей переменной
		if(--CurrField!=NullPos)
			Field2Buf(GetField);
		return;
	}

// 3) Go to menu
	EventFunc(&CurrPage->Exit);
	if(MenuDepth) {
		CurrPage=MenuStack[--MenuDepth].Page;
		CurrLine=MenuStack[MenuDepth].Line;
		CurrLCD=MenuStack[MenuDepth].LCD;
	}
	else {
		if(NextPage) {
			CurrPage=NextPage;
			NextPage=NULL;
		}
		else
			CurrPage=prp(&CurrPage->OutPage);
		CurrLine=CurrLCD=GetFix();
	}
	EventFunc(&CurrPage->Load);
}
