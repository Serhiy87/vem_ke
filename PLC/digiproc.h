/*
	13.03.2011 Kononenko 
	Add  PowL10

	27.03.2013 Kononenko
	uint8_t	Log10(uint16_t Numb) -> uint8_t	Log10(uint32_t Numb)

	31.03.2017 Kucherenko - Go AVRStudio4 on AtmelStudio7:
	typedef struct {...} PROGMEM PGM_uDataPoint; -> typedef const struct {...} PROGMEM PGM_uDataPoint;

*/
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef _DIGIPROC_H_
#define _DIGIPROC_H_
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <string.h>

// ~~~~~~~~~~~~~~~
uint16_t
Pow10(uint8_t Exp)
{
	uint16_t Pow = 1;
	while(Exp--)
		Pow *= 10;
	return Pow;
}
// ~~~~~~~~~~~~~~~
uint32_t
PowL10(uint8_t Exp)
{
	uint32_t Pow = 1;
	while(Exp--)
		Pow *= 10;
	return Pow;
}
// ~~~~~~~~~~~~~~~~~
uint8_t
Log10(uint32_t Numb)
{
	uint8_t Pos;
	for(Pos=0; Numb/=10; Pos++);
	return Pos;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint16_t
DigiFilt(const uint16_t *Array, uint8_t Edge)
{
	uint8_t Sz = Edge*4;
	uint8_t _Edge = Sz-Edge;
	uint16_t Buf[Sz];
	uint32_t Sum = 0;
	uint16_t Swap;
	uint8_t i, j;
	memcpy(Buf, Array, Sz*2);
	for(i=0; i<Edge; i++)
		for(j=i+1; j<Sz; j++)
			if(Buf[j] < Buf[i]) {
				Swap = Buf[i];
				Buf[i] = Buf[j];
				Buf[j] = Swap;
			}
	for(i=Sz-1; i>=_Edge; i--)
		for(j=i-1; j>=Edge; j--)
			if(Buf[j] > Buf[i]) {
				Swap = Buf[i];
				Buf[i] = Buf[j];
				Buf[j] = Swap;
			}
	for(i=Edge; i<_Edge; Sum+=Buf[i++]);
	return Sum/(Edge*2);
}

// ~~~~~~~~~~~~~~~~~~~~
// Binary coded decimal
// ~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~
uint8_t
CodBCD(uint8_t Number)
{
	return (Number/10<<4)+Number%10;
}

// ~~~~~~~~~~~~~~~~~~
uint8_t
DecodBCD(uint8_t BCD)
{
	return 10*(BCD>>4)+(BCD&0x0F);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Lineariation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef const struct {
	uint16_t x;
	uint16_t y;
} PROGMEM PGM_uDataPoint;

typedef struct {
	int32_t x1, x2;
	int16_t y1, y2;
} LineGraph;

typedef struct {
	uint32_t x1, x2;
	int16_t y1, y2;
} uLineGraph;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int16_t
LineFunc(const LineGraph *LG, int32_t x)
{
	int16_t y;
	if (LG->x1==LG->x2)
		y = LG->y1;
	else {
		float x1=LG->x1, x2=LG->x2, y1=LG->y1, y2=LG->y2;
		y = (x-x2)*(y2-y1)/(x2-x1)+y2;
	}
	return y;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int16_t
uLineFunc(const uLineGraph *LG, uint32_t x)
{
	int16_t y;
	if (LG->x1==LG->x2)
		y = LG->y1;
	else {
		float x1=LG->x1, x2=LG->x2, y1=LG->y1, y2=LG->y2;
		y = (x-x2)*(y2-y1)/(x2-x1)+y2;
	}
	return y;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int16_t
PGM_uBrokLine(PGM_uDataPoint *Brok, uint16_t x)
{
	uLineGraph Segm;
	Segm.x2 = prw(&Brok->x);
	do {
		Segm.x1 = Segm.x2;
		Segm.x2 = prw(&(++Brok)->x);
	} while(x>Segm.x2);
	Segm.y1 = (int16_t)prw(&(Brok-1)->y);
	Segm.y2 = (int16_t)prw(&Brok->y);
	return uLineFunc(&Segm, x);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int16_t
InterBrokLine(
	uint8_t WiAbsc, int16_t (*TblAbsc)[WiAbsc], uint8_t Absc,
	uint8_t WiOrdi, int16_t (*TblOrdi)[WiOrdi], uint8_t Ordi,
	uint8_t Len,
	int16_t x
)
{
	LineGraph Segm;
	uint8_t i = 0;
	Segm.x2=TblAbsc[0][Absc];
	do {
		Segm.x1 = Segm.x2;
		Segm.x2 = TblAbsc[++i][Absc];
	} while(x>Segm.x2 && i<Len);
	Segm.y1 = TblOrdi[i-1][Ordi];
	Segm.y2 = TblOrdi[i][Ordi];
	return LineFunc(&Segm, x);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int16_t
EE_InterBrokLine(
	uint8_t WiAbsc, uint16_t (*TblAbsc)[WiAbsc], uint8_t Absc,
	uint8_t WiOrdi, uint16_t (*TblOrdi)[WiOrdi], uint8_t Ordi,
	uint8_t Len,
	int16_t x
)
{
	LineGraph Segm;
	uint8_t i = 0;
	Segm.x2 = (int16_t)erw(TblAbsc[0]+Absc);
	do {
		Segm.x1 = Segm.x2;
		Segm.x2 = (int16_t)erw(TblAbsc[++i]+Absc);
	} while(x>Segm.x2 && i<Len);
	Segm.y1 = (int16_t)erw(TblOrdi[i-1]+Ordi);
	Segm.y2 = (int16_t)erw(TblOrdi[i]+Ordi);
	return LineFunc(&Segm, x);
}

#define TblBrokLine(Wi, Tbl, Absc, Ordi, Len, x) InterBrokLine(Wi, Tbl, Absc, Wi, Tbl, Ordi, Len, x)
#define EE_TblBrokLine(Wi, Tbl, Absc, Ordi, Len, x) EE_InterBrokLine(Wi, Tbl, Absc, Wi, Tbl, Ordi, Len, x)
#define ArrBrokLine(Wi, Tbl, Absc, Arr, Len, x) InterBrokLine(Wi, Tbl, Absc, 1, (int16_t**)Arr, 0, Len, x)
#define EE_ArrBrokLine(Wi, Tbl, Absc, Arr, Len, x) EE_InterBrokLine(Wi, Tbl, Absc, 1, (uint16_t**)Arr, 0, Len, x)

// ~~~~~~~~~~~~~~~~~~~~
#endif	// _DIGIPROC_H_
// ~~~~~~~~~~~~~~~~~~~~
