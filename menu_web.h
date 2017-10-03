/*
	Если у устройства нет ЖКИ - вместо menu.h
*/
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

typedef const struct {
	uint8_t Type;
	volatile void *Var;
	PGM_P const*EnumList;
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

typedef struct {
	char *Pos;
	uint8_t Prec, Size;
	PGM_P *Txt;
} T_Param;

static uint32_t F_Buf;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
EventFunc(const MenuEvent *Event)
{
	MenuEvent Func = prp(Event);
	if (Func)
		Func();
}
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
void Put_TextValAlignLeft(char* Val, T_Param *Param){
	char Format[9];
	//char Format[8] = "%10.10s";
	sprintf(Format,"%%-%u.%us", Param->Size, Param->Size);
	sprintf(Param->Pos, Format, Val);
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
// ~~~~~~~~~~~
//Message
typedef const struct {
	uint8_t  MsgCode;
} PROGMEM MsgPage;
MsgPage *CurrMsg;

MsgPage Msg_FlashFail		= {12};	
MsgPage Msg_EEPROMFail		= {13};
MsgPage Msg_FlashEEPROMFail	= {14};
MsgPage MsgErr41			= {41};

// ~~~~~~~~~~
uint8_t
HideMsg(void)
{
	if (CurrMsg) {
		//EventFunc(&CurrMsg->Exit);
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
	//IntOff();
	//MBInReg0[MB_IR_Message] = prb(&CurrMsg->MsgCode);
	//IntOn();
	//LED_MSG_On();
	//EventFunc(&CurrMsg->Load);
}





