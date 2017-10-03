/*
	02.04.2012 * Kononenko *  Revised for vmd2_1 - LCDBackLight

	01.08.2012 Kononenko
	Добавлены символы 'є','Є','ї','Ї'. Апостроф: Alt+39. Корректировка Win2LCD[]

	07.10.2013 - Kononenko -  Revised for vmd2_2 (Use PORT_BACKLIGHT, DDR_BACKLIGHT, BACKLIGHT, BACKLIGHT_INVERSION)

	12.12.2013 - Kononenko -  Add LCD Full(!) reset one time per 20 frames

	19.12.2013 - Kononenko -  Revised for vmd2_3 (Use Port_LCD_RW, LCD_DDR_RW, LCD_RW)

	23.12.2013 - Kononenko -  Could be used for 4-bit or 8-bit width data bus - define Port_LCD_DB0

	08.02.2014 - Kononenko - При сильных помехах не всегда помогает - изображение сдвигается
				Восстанавливает 100% - команда RETURN HOME
				Но при этом на некоторых моделях ЖКИ первая строка заметно дёргается при переинициализации
				Дёргается на Fordata. Не дёргается на Winstar, Bolymin.
				Вводится #define NOT_USE_LCD_RETURN_HOME_CMD
	10.02.2014 - Kononenko - убрал паузу 5мс после команды RETURN HOME - иначе тормозит ЖКИ ~1раз в сек
	
	29.07.2015 - Kucherenko - RTYP и RMON заполняются, если не объявлен дефайн QslRD

	08.08.2016 - Potapov - "Мигание" теперь только при выводе на LCD, в LCD_Buf теперь всегда актуальные значения.

	10.03.2017 - Kononenko - Ранее LCD использовал отдельный таймер Timer_LCD. Для 3-го Modbus нужно освобождение таймера Timer_LCD. 
				Теперь для LCD используем Timer1 совместно с soft-timer 10мсек.
				Расчитан на использование Timer1: 16MHz clkI/O/8 prescaler; 10msec (20000 T1 ticks); use Output CompareB ISR. 
				Не портировано для vmd3
*/

#if defined (__AVR_ATxmega128A1__) 
	#error lcd.h not ported for vmd3!
#endif

#if  defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) ||defined (__AVR_ATmega2561__)
	#define TIMSK_LCD TIMSK1
	#define TIFR_LCD TIFR1
#else
	#define TIMSK_LCD TIMSK
	#define TIFR_LCD TIFR	
#endif

#if TypeLCD==216
	#define LCDXSz 16
	#define LCDYSz 2
#elif TypeLCD==420
	#define LCDXSz 20
	#define LCDYSz 4
#endif

char LCD_Buf[LCDYSz][LCDXSz+1];

#define CharUp			0xA7
#define CharDown		0xB7
// Win '|' <=> LCD Degree
#define CharCursor	0x10
#define CharHand		0x11

volatile static uint8_t LCD_Rst_Flg;
extern uint8_t CurrX, StrSize, CurrLCD, CurrPos, CurrField, Blink;

static prog_uint8_t Win2LCD[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x01, 0x7D, 0xE9, 0x7F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xD9, 0xA2, 0xFF, 0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0x06,
	0xFF, 0xFF, 0x49, 0x69, 0xFF, 0xFF, 0xFF, 0xDA, 0xB5, 0xFF, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x05,
	0x41, 0xA0, 0x42, 0xA1, 0xE0, 0x45, 0xA3, 0xA4, 0xA5, 0xA6, 0x4B, 0xA7, 0x4D, 0x48, 0x4F, 0xA8,
	0x50, 0x43, 0x54, 0xA9, 0xAA, 0x58, 0xE1, 0xAB, 0xAC, 0xE2, 0xAD, 0xAE, 0x62, 0xAF, 0xB0, 0xB1,
	0x61, 0xB2, 0xB3, 0xB4, 0xE3, 0x65, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0x6F, 0xBE,
	0x70, 0x63, 0xBF, 0x79, 0xE4, 0x78, 0xE5, 0xC0, 0xC1, 0xE6, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7
};
static prog_uint8_t CGRAM_LCD[8][8] = {
	{	// Cursor
		0b00000,
		0b00100,
		0b00110,
		0b11111,
		0b00110,
		0b00100,
		0b00000,
		0b00000
	},
	{	// Degree
		0b00110,
		0b01001,
		0b01001,
		0b00110,
		0b00000,
		0b00000,
		0b00000,
		0b00000
	},
	{	// Hand
		0b00000,
		0b01000,
		0b01000,
		0b01110,
		0b01111,
		0b11111,
		0b01111,
		0b00110
	},
	{	// 'э'
		0b00000,
		0b00000,
		0b01110,
		0b10001,
		0b11100,
		0b10001,
		0b01110,
		0b00000
	},
	{	// 'Є'
		0b01110,
		0b10001,
		0b10000,
		0b11100,
		0b10000,
		0b10001,
		0b01110,
		0b00000
	},
	{	// 'ї'
		0b01010,
		0b00000,
		0b01100,
		0b00100,
		0b00100,
		0b00100,
		0b01110,
		0b00000
	},
	{	// 'Ї'
		0b01010,
		0b01110,
		0b00100,
		0b00100,
		0b00100,
		0b00100,
		0b01110,
		0b00000
	}
};

#define LCD_Delay 50	// 40 us needs

static uint8_t TD_LCD;

#ifdef BACKLIGHT
	void LCDBackLightInit(void);
	void LCDBackLightOn(void);
	void LCDBackLightOff(void);
	void LCDBackLightStartTimer(void);
#endif

#ifdef Port_LCD_DB0	//8-bit width data bus
	// ~~~~~~~~~~~~~~
	static void
	Send(uint8_t Val)
	{
		if (Val &(1<<0)) Port_LCD_DB0 |=DB0; else Port_LCD_DB0 &=~DB0;
		if (Val &(1<<1)) Port_LCD_DB1 |=DB1; else Port_LCD_DB1 &=~DB1;
		if (Val &(1<<2)) Port_LCD_DB2 |=DB2; else Port_LCD_DB2 &=~DB2;
		if (Val &(1<<3)) Port_LCD_DB3 |=DB3; else Port_LCD_DB3 &=~DB3;
		if (Val &(1<<4)) Port_LCD_DB4 |=DB4; else Port_LCD_DB4 &=~DB4;
		if (Val &(1<<5)) Port_LCD_DB5 |=DB5; else Port_LCD_DB5 &=~DB5;
		if (Val &(1<<6)) Port_LCD_DB6 |=DB6; else Port_LCD_DB6 &=~DB6;
		if (Val &(1<<7)) Port_LCD_DB7 |=DB7; else Port_LCD_DB7 &=~DB7;

		Port_LCD_E |=LCD_E;
		_delay_us(1);		// 450 ns needs
		Port_LCD_E &=~LCD_E;
		_delay_us(1);		// 10 ns needs
	}

#else 	//4-bit width data bus
	// ~~~~~~~~~~~~~~~~~~
	static void
	SendTetr(uint8_t Val)
	{
		if (Val &(1<<0)) Port_LCD_DB4 |=DB4; else Port_LCD_DB4 &=~DB4;
		if (Val &(1<<1)) Port_LCD_DB5 |=DB5; else Port_LCD_DB5 &=~DB5;
		if (Val &(1<<2)) Port_LCD_DB6 |=DB6; else Port_LCD_DB6 &=~DB6;
		if (Val &(1<<3)) Port_LCD_DB7 |=DB7; else Port_LCD_DB7 &=~DB7;

		Port_LCD_E |=LCD_E;
		_delay_us(1);		// 450 ns needs
		Port_LCD_E &=~LCD_E;
		_delay_us(1);		// 10 ns needs
	}

	// ~~~~~~~~~~~~~~
	static void
	Send(uint8_t Val)
	{
		SendTetr(Val>>4);
		SendTetr(Val&0xF);
	}
#endif
// ~~~~~~~~~~~~~~~~
static void
Send_I(uint8_t Val)
{
	Send(Val);
	Delay_us(LCD_Delay);
}

// ~~~~~~~~~~~
static void
LCD_FuncSet0(void)
{
	Port_LCD_DB4  |=DB4;
	Port_LCD_DB5  |=DB5;
	Port_LCD_DB6 &=~DB6;
	Port_LCD_DB7 &=~DB7;
}
// ~~~~~~~~~~~
#ifndef Port_LCD_DB0
	static void
	LCD_FuncSet1(void)
	{
		Port_LCD_DB4 &=~DB4;
		Port_LCD_DB5  |=DB5;
		Port_LCD_DB6 &=~DB6;
		Port_LCD_DB7 &=~DB7;
	}
#endif
// ~~~~~~~~~~~
static void
LCD_Rst0(void)
{
	cli();
	Port_LCD_RS &=~LCD_RS;
	LCD_FuncSet0();
	sei();
}

// ~~~~~~~~~~~
static void
LCD_Rst1(void)
{
	LCD_FuncSet0();
	Delay_us(200);				// 100 us needs
	LCD_FuncSet0();
	Delay_us(LCD_Delay);
	#ifdef Port_LCD_DB0	
		LCD_FuncSet0();
	#else
		LCD_FuncSet1();
	#endif
//// BF(Busy flag) can be checked follow
	Delay_us(LCD_Delay);
	Port_LCD_RS |=LCD_RS;
}

// ~~~~~~~~~~~~
void
LCD_Init (void)
{
	uint8_t i, j;

	// LCD pins
	#ifdef Port_LCD_RW
		Port_LCD_RW &= ~LCD_RW;
		DDR_LCD_RW |= LCD_RW;
	#endif

	Port_LCD_RS &= ~LCD_RS;	
	DDR_LCD_RS |=LCD_RS;

	Port_LCD_E &= ~LCD_E;	
	DDR_LCD_E |=LCD_E;

	#ifdef Port_LCD_DB0	
		DDR_LCD_DB0 |=DB0;
		DDR_LCD_DB1 |=DB1;
		DDR_LCD_DB2 |=DB2;
		DDR_LCD_DB3 |=DB3;
	#endif
	DDR_LCD_DB4 |=DB4;
	DDR_LCD_DB5 |=DB5;
	DDR_LCD_DB6 |=DB6;
	DDR_LCD_DB7 |=DB7;

	// LCD initialization
	Delay_ms(50);		// 40 ms needs to 2.7 V

	LCD_Rst0();
	Delay_ms(5);		// 4.1 ms needs
	LCD_Rst1();

	Port_LCD_RS &=~LCD_RS;

	#ifdef Port_LCD_DB0	
		Send_I(0b00111000);	//; 8-bits DL, 2 lines, 5x7 dots
	#else
		Send_I(0b00101000);	//; 4-bits DL, 2 lines, 5x7 dots
	#endif
	Send_I(0b00001100);	//; Display on, cursor off, blink off	
	Send_I(0b00000110);	//; cursor shift right, no display shift


	Send_I(0b01000000);	//; Установить кусор в CG RAM по адресу 0.
	Port_LCD_RS |=LCD_RS;
	for (i=0; i<8; i++)
		for (j=0; j<8; j++)
			Send_I(prb(CGRAM_LCD[i]+j));

	Port_LCD_RS &=~LCD_RS;
	Send(0b00000001);
	Delay_ms(5);				// By trial
	Port_LCD_RS |=LCD_RS;

	// Frame Timer
	TD_LCD = Timer8SysAlloc(1);

	for (i=0; i<LCDYSz; i++)
		for (j=0; j<LCDXSz; j++)
			LCD_Buf[i][j] = ' ';

	#ifdef RTYP
		#ifndef QslRD
			*(uint8_t*)RTYP = R_PLC_Type;
		#endif	
	#endif

	#ifdef BACKLIGHT
		LCDBackLightInit();
	#endif
}

// ~~~~~~~~~~
uint8_t
LCD_Rdy(void)
{
	uint8_t Rdy = Timer8Stopp(TD_LCD);
	Rdy = Rdy && !(TIMSK_LCD &(1<<OCIE1B));
	if (Rdy)
		StartTimer8(TD_LCD, 6);
	return Rdy;
}
// ~~~~~~~~~~
void
LCD_Out(void)
{
	static uint8_t N;
	uint16_t LCD_delay = 100;	//50us

	if( (N>=20) && !LCD_Rst_Flg ){
		N = 0;
		LCD_Rst0();
		LCD_Rst_Flg = 1;
		LCD_delay = 10000;	//5ms
	}
	else {
		N++;
	}

	cli();
	LCD_delay = TCNT1 + LCD_delay;
	if(LCD_delay >= 20000) OCR1B = LCD_delay - 20000;
	else OCR1B = LCD_delay;
	TIFR_LCD |= (1<<OCF1B);
	TIMSK_LCD |= (1<<OCIE1B);
	sei();
}

// ~~~~~~~~~~~~~~~~~~~~~~
// 40 us needs
ISR(TIMER1_COMPB_vect){

	uint16_t LCD_delay = 100;	//50us

	if (LCD_Rst_Flg) {
		switch(LCD_Rst_Flg){
			case 1:
				LCD_FuncSet0();
				break;
			case 2:
				LCD_FuncSet0();
				break;
			case 3:
				#ifdef Port_LCD_DB0	
					LCD_FuncSet0();
				#else
					LCD_FuncSet1();
				#endif
				break;
			case 4:
				#ifdef Port_LCD_DB0	
					Send(0b00111000);	//; 8-bits DL, 2 lines, 5x7 dots
				#else
					Send(0b00101000);	//; 4-bits DL, 2 lines, 5x7 dots
				#endif
				break;
			case 5:
				Send(0b00001100);	//; Display on, cursor off, blink off	
				break;
			case 6:
				Send(0b00000110);	//; cursor shift right, no display shift
				break;
			case 7:
				Send(0b01000000);	//; Установить кусор в CG RAM по адресу 0.
				Port_LCD_RS |=LCD_RS;
				break;
			case 8:{
					static uint8_t i,j;
					LCD_Rst_Flg = 7;
					Send(prb(CGRAM_LCD[i]+j));
					if(j<7) j++;
					else{
						j=0;
						if(i<7)i++;
						else{i=0;LCD_Rst_Flg = 8;}
					}
				}
				break;
			case 9:
				Port_LCD_RS &=~LCD_RS;
				#ifdef NOT_USE_LCD_RETURN_HOME_CMD
					Send(128);			// Установить курсор в нулевую позицию
				#else
					Send(0b00000010);	//RETURN HOME					
				#endif
				Port_LCD_RS |=LCD_RS;
			default:
				LCD_Rst_Flg = 255;
		}
		if(LCD_Rst_Flg==1) LCD_delay = 400;	//200us

		LCD_Rst_Flg++;
	}
	else {
		static uint8_t X, Y;
		static prog_uint8_t LineAddr[] = {128, 192, 148, 212};
		unsigned char cursor_symb;
		
		if (X<LCDXSz) {
			if((X>=CurrX)&&(X<(CurrX+StrSize))&&(Y==CurrLCD)){
				cursor_symb='_';
			}
			else
			{
				cursor_symb=0;
			}

			Send(
			#ifdef RMON
				#ifndef QslRD
					*((uint8_t*)RMON+Y*LCDXSz+X) =
				#endif
			#endif
				(((Blink>=8 && Blink<=10 )|| Blink>=14)&&cursor_symb&&(CurrField!=255))?prb(Win2LCD+(unsigned char)cursor_symb):(prb(Win2LCD+(unsigned char)LCD_Buf[Y][X])));
				X++;
		}
		else {
			X = 0;
			if (++Y==LCDYSz) {
				Y = 0;
				TIMSK_LCD &= ~(1<<OCIE1B);	//stop Output CompareB interrups
			}
			Port_LCD_RS &=~LCD_RS;
			Send(prb(LineAddr+Y));
			Port_LCD_RS |=LCD_RS;
		}
	}
	LCD_delay = TCNT1 + LCD_delay;
	if(LCD_delay >= 20000) OCR1B = LCD_delay - 20000;
	else OCR1B = LCD_delay;
	TIFR_LCD |= (1<<OCF1B);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef BACKLIGHT
	
	static uint8_t TD_BackLight;
	uint8_t BackLightMode;
	enum BACKLIGHT_TIMES{ BCKLT_10sec, BCKLT_30sec, BCKLT_1min,  BCKLT_3min,  BCKLT_10min};
	static uint8_t EEMEM BackLightTimeEE = BCKLT_10sec;

	void LCDBackLightInit(void){
		IntOff();	
		DDR_BACKLIGHT |= BACKLIGHT;
		IntOn();	
		TD_BackLight = Timer16SysAlloc(1);
		if(BackLightMode != 2){
			LCDBackLightOn();
			LCDBackLightStartTimer();
		}
	}
	void LCDBackLightOn(void){
		IntOff();	
		#ifdef BACKLIGHT_INVERSION
			PORT_BACKLIGHT &= ~BACKLIGHT;
		#else
			PORT_BACKLIGHT |= BACKLIGHT;
		#endif
		IntOn();	
	}
	void LCDBackLightOff(void){
		IntOff();	
		#ifdef BACKLIGHT_INVERSION
			PORT_BACKLIGHT |= BACKLIGHT;
		#else
			PORT_BACKLIGHT &= ~BACKLIGHT;
		#endif
		IntOn();
	}
	void LCDBackLightStartTimer(void){
		switch(erb(&BackLightTimeEE)){
			case BCKLT_10sec:
				StartTimer16(TD_BackLight, 1000);
				break;
			case BCKLT_30sec:
				StartTimer16(TD_BackLight, 3000);
				break;
			case BCKLT_1min:
				StartTimer16(TD_BackLight, 6000);
				break;
			case BCKLT_3min:
				StartTimer16(TD_BackLight, 18000);
				break;	
			case BCKLT_10min:
				StartTimer16(TD_BackLight, 60000);
				break;
			default:
				StartTimer16(TD_BackLight, 3000);
				break;			
		}
	}
	void LCDBackLightControl(void){
		switch(BackLightMode){
		case 1: if(Timer16Stopp(TD_BackLight)) LCDBackLightOff();
			break;
		case 2:
			break;
		default: LCDBackLightOn();
		}
	}

#endif
//~~~~~~~~~~~~~~~~~~~~~~~~~~~
