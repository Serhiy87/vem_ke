/*
	01.10.2014 - Kononenko - Only ISR(TIMER0_COMP_vect) for Atmega16 or Atmega32
	04.10.2014 - Kononenko - uint8_t->uint16_t Modbus_Map.xxxxx_Qt
	17.10.2015 - Kononenko - Use Modbus DRE ISR. Пока нет поддержки Xmega!
	12.08.2016 - Kononenko - Устранено зависание Master на DRE interrupt при малом Timeout
	11.07.2016 - Kononenko - Master зависал на прерывании DRE при малом Timeout. Scancycle ~ 800мс! 
							Добавил режим MB_NoReply. Доп.запрет прерывания DRE
	01.09.2016 - Kononenko - Уменьшение времени ISR. Вся обработка в основном цикле. MB_TimerFlag
							Перенёс MB_NativeAddr, Set_MB_Addr из modbus_s.h
							Фильтрация пакетов по адресу в MB_Timer_ISR - иначе Slave может не успеть за другим Slave обработать чужой запрос
	10.03.2016 - Kononenko - Add Modbus 3 for vmd2_3
	30.03.2017 Kucherenko - Go AVRStudio4 on AtmelStudio7:
	struct {...} PROGMEM Modbus_Map[Modbus_Qt] -> struct {...} PROGMEM const Modbus_Map[Modbus_Qt]
	... All variables PROGMEM -> ...const ... PROGMEM

*/
#define MBMinAddr 1
#define MBMaxAddr 247
#ifndef Init_MB_NativeAddr
	#define Init_MB_NativeAddr {MBMinAddr}
#endif
uint8_t MB_NativeAddr[Modbus_Qt];

#ifndef MB_ADDR_DIP_SW
	static uint8_t EEMEM MB_NativeAddr_EE[Modbus_Qt] = Init_MB_NativeAddr;
	// ~~~~~~~~~~~~~~~~~~~~~~
	void
	Set_MB_Addr(uint8_t MB_N)
	{
		ewb(MB_NativeAddr_EE+MB_N, MB_NativeAddr[MB_N]);
	}
#endif

#ifndef Modbus_Qt
	#define Modbus_Qt 1
#endif

enum {NoParity, EvenParity, OddParity};
enum {MBBR1200, MBBR2400, MBBR4800, MBBR9600, MBBR14400, MBBR19200, MBBR28800, MBBR38400, MBBR57600};
#define Std_Modbus_Param {EvenParity, 1, MBBR19200}
#ifndef Init_Modbus_Param
	#define Init_Modbus_Param {Std_Modbus_Param}
#endif
struct {
	uint8_t Parity, StopBits, BitRate;
} MB_Param[Modbus_Qt] EEMEM = Init_Modbus_Param;
uint8_t MB_BitRate_RAM[Modbus_Qt];

enum {Role_Slave, Role_Master};
#ifndef MB_Role_List
	#define MB_Role_List {Role_Slave}
#endif
static prog_uint8_t MB_Role[Modbus_Qt] = MB_Role_List;

volatile uint16_t MB_CPT[Modbus_Qt][8];

struct {
	volatile uint8_t *Coil;
	uint16_t Coil_Qt;
	volatile uint8_t *Input;
	uint16_t Input_Qt;
	volatile uint16_t *HoldReg;
	uint16_t HoldReg_Qt;
	volatile uint16_t *InReg;
	uint16_t InReg_Qt;
} PROGMEM const Modbus_Map[Modbus_Qt] = Modbus_Map_List;

uint8_t MB_Frame[Modbus_Qt][0x100];
uint8_t MB_Frame_Sz[Modbus_Qt];

enum {IllegFunc=1, IllegAddr, IllegData};

enum {MB_Initi, MB_Idle, MB_Recept, MB_Trans, MB_Proc, MB_NoReply};
volatile uint8_t MB_State[Modbus_Qt];

volatile uint8_t MB_TimerFlag;

static uint8_t MB_Err;

#if	F_CPU==16000000UL
	typedef const struct {
		uint16_t UBRR;
		uint8_t Time35;
	} PROGMEM BR_Val;
	BR_Val BR_Tbl[] = {
		{0x342, 255},		// 1200
		{0x1A0, 251},		// 2400
		{0x0CF, 125},		// 4800
		{0x067,  63},		// 9600
		{0x044,  42},		// 14400
		{0x033,  31},		// 19200
		{0x022,  27},		// 28800
		{0x019,  27},		// 38400
		{0x010,  27}		// 57600
	};
#elif	F_CPU==32000000UL
	typedef const struct {
		uint16_t UBRR;
		uint16_t Time35;
	} PROGMEM BR_Val;
	BR_Val BR_Tbl[] = {
		{0x342, 1004},	// 1200
		{0x1A0,  502},	// 2400
		{0x0CF,  250},	// 4800
		{0x067,  126},	// 9600
		{0x044,   84},	// 14400
		{0x033,   62},	// 19200
		{0x022,   54},	// 28800
		{0x019,   54},	// 38400
		{0x010,   54}		// 57600
	};
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// USARTs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__)
	#define MB_Port_List {{		\
		&UDR,										\
		&UCSRA, &UCSRB, &UCSRC,	\
		&UBRRH, &UBRRL,					\
		RS485,									\
		MB_LED,									\
		MB_LED_Err_On,					\
		MB_LED_Err_Off					\
	}}
#elif defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
	#define Port(n) {													\
		&UDR##n,																\
		&UCSR##n##A, &UCSR##n##B, &UCSR##n##C,	\
		&UBRR##n##H, &UBRR##n##L,								\
		RS485_##n,															\
		MB_LED##n,															\
		MB_LED_Err_On_##n,											\
		MB_LED_Err_Off_##n											\
	}

	#define Port_0 Port(0)
	#define Port_1 Port(1)
	#if defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		#define Port_2 Port(2)
	#endif

	#ifndef MB_Port_List
		#define MB_Port_List {Port(1)}
	#endif
#elif defined (__AVR_ATxmega128A1__)
	#define Port(X, n) {		\
		&USART##X##n,					\
		RS485_##X##n,					\
		MB_LED_##X##n,				\
		MB_LED_Err_On_##X##n,	\
		MB_LED_Err_Off_##X##n	\
	}

	#define Port_0 Port(D, 0)
	#define Port_1 Port(D, 1)
	#define Port_2 Port(E, 0)

	#ifndef MB_Port_List
		#define MB_Port_List {Port_1}
	#endif
#endif

#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
	typedef struct {
		volatile uint8_t *udr;
		volatile uint8_t *ucsra, *ucsrb, *ucsrc;
		volatile uint8_t *ubrrh, *ubrrl;
		void(*RS485)(void);
		void(*LED)(void);
		void(*LED_Err_On)(void);
		void(*LED_Err_Off)(void);
	} MB_Port_t;

	typedef volatile uint8_t usart_t;
	#define usart_data(usart) (*usart)

	#define Modbus_ISR(n)								\
		ISR(USART##n##_RX_vect) {Modbus_RX(&UDR##n);}	\
		ISR(USART##n##_TX_vect) {Modbus_TX(&UDR##n);}	\
		ISR(USART##n##_UDRE_vect) {Modbus_TX(&UDR##n);}

	#define USART_Reg(N, Reg) *(uint8_t*)prp(&MB_Port[N].Reg)

	#define usart_field udr
#elif defined (__AVR_ATxmega128A1__)
	typedef struct {
		USART_t *usart;
		void(*RS485)(void);
		void(*LED)(void);
		void(*LED_Err_On)(void);
		void(*LED_Err_Off)(void);
	} MB_Port_t;

	typedef USART_t usart_t;
	#define usart_data(usart) (usart->DATA)

	#define Modbus_ISR(X, n)																	\
		ISR(USART##X##n##_RXC_vect) {Modbus_RX(&USART##X##n);}	\
		ISR(USART##X##n##_TXC_vect) {Modbus_TX(&USART##X##n);}

	#define usart_field usart
#endif

static const MB_Port_t MB_Port[Modbus_Qt] PROGMEM = MB_Port_List;

#define USART_Func(N, Func) ((void(*)(void))prp(&MB_Port[MB_N].Func))()

// ~~~~~~~~~~~~~~~~~~~~~~
static uint8_t
MB_Port_N(usart_t *usart)
{
	uint8_t MB_N;
	for (MB_N=0; prp(&MB_Port[MB_N].usart_field)!=usart; MB_N++);
	return MB_N;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Hard Timers
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~
//static void
void
StartTimer0(void)
{
	#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__)
		TCCR0  = ((((1<<WGM01) &~(1<<WGM00)  &~(1<<COM01)  &~(1<<COM00)) |(1<<CS02)) &~(1<<CS01)) |(1<<CS00);
	#elif defined (__AVR_ATmega128__)
		TCCR0  =   ((1<<WGM01) &~(1<<WGM00)  &~(1<<COM01)  &~(1<<COM00)) |(1<<CS02)   |(1<<CS01)  |(1<<CS00);
	#elif defined (__AVR_ATmega1280__)  || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		TCCR0A =    (1<<WGM01) &~(1<<WGM00)  &~(1<<COM0A1) &~(1<<COM0A0) &~(1<<COM0B1) &~(1<<COM0B0);
		TCCR0B = ((~(1<<WGM02)                                           |(1<<CS02)) &~(1<<CS01)) |(1<<CS00);
	#endif
	#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__)
		TIMSK |=(1<<OCIE0);
	#elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		TIMSK0 |=(1<<OCIE0A);
	#endif
}

// ~~~~~~~~~~~~~~
//static void
void
StartTimer2(void)
{
	#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__)
		TCCR2  =   ((1<<WGM21) &~(1<<WGM20)  &~(1<<COM21)  &~(1<<COM20)) |(1<<CS22)   |(1<<CS21)  |(1<<CS20);
	#elif defined (__AVR_ATmega128__)
		TCCR2  = ((((1<<WGM21) &~(1<<WGM20)  &~(1<<COM21)  &~(1<<COM20)) |(1<<CS22)) &~(1<<CS21)) |(1<<CS20);
	#elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		TCCR2A =    (1<<WGM21) &~(1<<WGM20)  &~(1<<COM2A1) &~(1<<COM2A0) &~(1<<COM2B1) &~(1<<COM2B0);
		TCCR2B =   ~(1<<WGM22)                                           |(1<<CS22)   |(1<<CS21)  |(1<<CS20);
	#endif
	#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__)
		TIMSK |=(1<<OCIE2);
	#elif  defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		TIMSK2 |=(1<<OCIE2A);
	#endif
}
// ~~~~~~~~~~~~~~
#if defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
	//static void
	void
	StartTimer4(void)
	{
		TCCR4A = 0;
		TCCR4B = (1<<WGM42) | (1<<CS42) | (0<<CS41) | (1<<CS40);	// clk/1024
		TIMSK4 |=(1<<OCIE4A);
	}
#endif
// ~~~~~~~~~~~~~~

#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__)
	#define Timer_n(n) {&TCNT##n, &OCR##n,		StartTimer##n}
#elif  defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
	#define Timer_n(n) {(uint8_t*)&TCNT##n, (uint8_t*)&OCR##n##A,	StartTimer##n}
#elif defined (__AVR_ATxmega128A1__)
	#define Timer_X(X) &TC##X##1
#endif

#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
	typedef struct {
		volatile uint8_t *TCNT, *OCR;
		void(*Start)(void);
	} MB_Timer_t;

	#ifndef MB_TimerList
		#if  defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
			#define MB_TimerList {Timer_n(0), Timer_n(2), Timer_n(4)}
		#else
			#define MB_TimerList {Timer_n(0), Timer_n(2)}
		#endif
	#endif

	#define tc_f .TCNT
#elif defined (__AVR_ATxmega128A1__)
	typedef TC1_t *MB_Timer_t;

	#ifndef MB_TimerList
		#define MB_TimerList {Timer_X(C), Timer_X(D)}
	#endif

	#define tc_f
#endif

static const MB_Timer_t MB_Timer[] PROGMEM = MB_TimerList;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef volatile uint8_t tc_t;

#define MB_StartTimer(N) ((void(*)(void))prp(&MB_Timer[N].Start))()
#define MB_ClearTimer(N) *(uint8_t*)prp(&MB_Timer[N].TCNT) = 0

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif defined (__AVR_ATxmega128A1__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef TC1_t tc_t;

// ~~~~~~~~~~~~~~~~~~~~~~~~
static void
MB_StartTimer(uint8_t MB_N)
{
	TC1_t *tc = prp(MB_Timer+MB_N);
	tc->CTRLA = TC_CLKSEL_DIV1024_gc;
	tc->INTCTRLA = (tc->INTCTRLA &~TC1_OVFINTLVL_gm) |TC_OVFINTLVL_HI_gc;
}

#define MB_ClearTimer(N) ((TC1_t*)prp(MB_Timer+MB_N))->CNT=0

// ~~~
#endif
// ~~~

// ~~~~~~~~~~~~~~~~~~~
static void
MB_Timer_ISR(tc_t *tc)
{
	#if defined (__AVR_ATxmega128A1__)
		tc->INTCTRLA = (tc->INTCTRLA &~TC1_OVFINTLVL_gm) |TC_OVFINTLVL_OFF_gc;
		tc->CTRLA = TC_CLKSEL_OFF_gc;
	#endif
	uint8_t MB_N;
	for (MB_N=0; prp(&MB_Timer[MB_N]tc_f)!=tc; MB_N++);

	
	switch (prb(MB_Role+MB_N)){
	case Role_Slave:
		if(MB_Frame[MB_N][0]==MB_NativeAddr[MB_N]){	//если Slave не видит своего адреса - не устанавливать MB_TimerFlag
			MB_TimerFlag |= (1<<MB_N);
		}
		else{
			MB_State[MB_N] = MB_Idle;
			MB_CPT[MB_N][0]++;
		}
		break;
	case Role_Master:
		MB_TimerFlag |= (1<<MB_N);
		break;
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ISR(TIMER0_COMP_vect)
{
	TCCR0 &=~(1<<CS02) &~(1<<CS01) &~(1<<CS00);
	TIMSK &=~(1<<OCIE0);
	MB_Timer_ISR(&TCNT0);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif defined (__AVR_ATmega128__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ISR(TIMER0_COMP_vect)
{
	TCCR0 &=~(1<<CS02) &~(1<<CS01) &~(1<<CS00);
	TIMSK &=~(1<<OCIE0);
	MB_Timer_ISR(&TCNT0);
}

// ~~~~~~~~~~~~~~~~~~
ISR(TIMER2_COMP_vect)
{
	TCCR2 &=~(1<<CS22) &~(1<<CS21) &~(1<<CS20);
	TIMSK &=~(1<<OCIE2);
	MB_Timer_ISR(&TCNT2);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif  defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) ||defined (__AVR_ATmega2561__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ISR(TIMER0_COMPA_vect)
{
	TCCR0B &=~(1<<CS02) &~(1<<CS01) &~(1<<CS00);
	TIMSK0 &=~(1<<OCIE0A);
	MB_Timer_ISR(&TCNT0);
}

// ~~~~~~~~~~~~~~~~~~~
ISR(TIMER2_COMPA_vect)
{
	TCCR2B &=~(1<<CS22) &~(1<<CS21) &~(1<<CS20);
	TIMSK2 &=~(1<<OCIE2A);
	MB_Timer_ISR(&TCNT2);
}
// ~~~~~~~~~~~~~~~~~~~
ISR(TIMER4_COMPA_vect)
{
	TCCR4B &=~(1<<CS42) &~(1<<CS41) &~(1<<CS40);
	TIMSK4 &=~(1<<OCIE4A);
	MB_Timer_ISR((uint8_t*)&TCNT4);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif defined (__AVR_ATxmega128A1__)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ISR(TCC1_OVF_vect)
{
	MB_Timer_ISR(&TCC1);
}

// ~~~~~~~~~~~~~~~
ISR(TCD1_OVF_vect)
{
	MB_Timer_ISR(&TCD1);
}

// ~~~
#endif
// ~~~

// ~~~~~~~~~~~~~~~~~~~~
void
SetParity(uint8_t MB_N)
{
	uint8_t Parity = erb(&MB_Param[MB_N].Parity), S_B = erb(&MB_Param[MB_N].StopBits);
	#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		uint8_t ucsrc = (~(1<<6) |(1<<2) |(1<<1)) &~(1<<0);
		if (Parity==NoParity) {
			ucsrc &=~(1<<5) &~(1<<4);
			switch (S_B) {
			case 1:
				ucsrc &=~(1<<3);
				break;
			case 2:
				ucsrc  |=(1<<3);
				break;
			}
		}
		else {
			ucsrc &=~(1<<3);
			switch (Parity) {
			case EvenParity:
				ucsrc = (ucsrc |(1<<5)) &~(1<<4);
				break;
			case OddParity:
				ucsrc =  ucsrc |(1<<5)   |(1<<4);
				break;
			}
		}
		#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__)
			ucsrc  |=(1<<7);
		#elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
			ucsrc &=~(1<<7);
		#endif
		USART_Reg(MB_N, ucsrc) = ucsrc;
	#elif defined (__AVR_ATxmega128A1__)
		USART_t *usart = prp(&MB_Port[MB_N].usart);
		usart->CTRLC &=~USART_PMODE_gm;
		if (Parity==NoParity) {
			usart->CTRLC |=USART_PMODE_DISABLED_gc;
			switch (S_B) {
			case 1:
				usart->CTRLC &=~USART_SBMODE_bm;
				break;
			case 2:
				usart->CTRLC  |=USART_SBMODE_bm;
				break;
			}
		}
		else {
			usart->CTRLC &=~USART_SBMODE_bm;
			switch (Parity) {
			case EvenParity:
				usart->CTRLC |=USART_PMODE_EVEN_gc;
				break;
			case OddParity:
				usart->CTRLC |=USART_PMODE_ODD_gc;
				break;
			}
		}
	#endif
	if (Parity!=NoParity && S_B!=1)
		ewb(&MB_Param[MB_N].StopBits, 1);
}

// ~~~~~~~~~~~~~~~~~~
void
SetBAUD(uint8_t MB_N)
{
	BR_Val *ptrBR_Val = BR_Tbl + (MB_BitRate_RAM[MB_N] = erb(&MB_Param[MB_N].BitRate));
	#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		USART_Reg(MB_N, ubrrl) = prb((uint8_t*)&ptrBR_Val->UBRR);
		USART_Reg(MB_N, ubrrh) = prb((uint8_t*)&ptrBR_Val->UBRR+1);

		*(uint8_t*)prp(&MB_Timer[MB_N].OCR) = prb(&ptrBR_Val->Time35);
	#elif defined (__AVR_ATxmega128A1__)
		USART_t *usart = prp(&MB_Port[MB_N].usart);
		usart->BAUDCTRLA = prb((uint8_t*)&ptrBR_Val->UBRR);
		usart->BAUDCTRLB = prb((uint8_t*)&ptrBR_Val->UBRR+1) + USART_BSCALE0_bm;

		((TC1_t*)prp(MB_Timer+MB_N))->PER = prw(&ptrBR_Val->Time35);
	#endif
}

// ~~~~~~~~~~
void
MB_Init(void)
{
	MB_PLC_Init();
	#if defined (__AVR_ATxmega128A1__)
		// USARTD0
		PORTD.OUT |=(1<<3);
		PORTD.DIR |=(1<<3);
		// USARTD1
		PORTD.OUT |=(1<<7);
		PORTD.DIR |=(1<<7);
		// USARTE0
		PORTE.OUT |=(1<<3);
		PORTE.DIR |=(1<<3);
	#endif
	for (uint8_t MB_N=0; MB_N<Modbus_Qt; MB_N++) {
		void MB_S_Init(uint8_t);
		void MB_M_Init(uint8_t);
		MB_State[MB_N] = MB_Initi;
		#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
			USART_Reg(MB_N, ucsra) = ~(1<<1) &~(1<<0);
			USART_Reg(MB_N, ucsrb) = (((1<<7) &~(1<<6) &~(1<<5)) |(1<<4) |(1<<3)) &~(1<<2);
		#elif defined (__AVR_ATxmega128A1__)
			USART_t *usart = prp(&MB_Port[MB_N].usart);
			usart->CTRLA = USART_RXCINTLVL_HI_gc |USART_TXCINTLVL_OFF_gc |USART_DREINTLVL_OFF_gc;
			usart->CTRLB = (USART_RXEN_bm |USART_TXEN_bm) &~USART_CLK2X_bm &~USART_MPCM_bm;
			usart->CTRLC = USART_CMODE_ASYNCHRONOUS_gc |USART_CHSIZE_8BIT_gc /*|USART_UDORD_bm*/;

			TC1_t *tc = prp(MB_Timer+MB_N);
			tc->CTRLB = TC_WGMODE_NORMAL_gc &~TC1_CCBEN_bm &~TC1_CCAEN_bm;
			tc->CTRLD = TC_EVACT_OFF_gc;
			//tc->CTRLE = TC1_BYTEM_bm;
			tc->INTCTRLA |=TC_ERRINTLVL_OFF_gc;
			tc->INTCTRLB = 0;
		#endif
		SetParity(MB_N);
		SetBAUD(MB_N);
		switch (prb(MB_Role+MB_N)) {
		case Role_Slave:
			MB_S_Init(MB_N);
			break;
		case Role_Master:
			MB_M_Init(MB_N);
			break;
		}
		MB_StartTimer(MB_N);
	}
}

// ~~~~~~~~~~~~~~~~~~~
static void
MBRecept(uint8_t MB_N)
{
	uint8_t Buf;
	if (MB_Frame_Sz[MB_N]>=0x100) {
		#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
			Buf = USART_Reg(MB_N, udr);
		#elif defined (__AVR_ATxmega128A1__)
			Buf = ((USART_t*)prp(&MB_Port[MB_N].usart))->DATA;
		#endif
		MB_Err &=(1<<MB_N);
		return;
	}
	#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		Buf = USART_Reg(MB_N, ucsra);
		MB_Frame[MB_N][MB_Frame_Sz[MB_N]++] = USART_Reg(MB_N, udr);
	#elif defined (__AVR_ATxmega128A1__)
		USART_t *usart = prp(&MB_Port[MB_N].usart);
		Buf = usart->STATUS;
		MB_Frame[MB_N][MB_Frame_Sz[MB_N]++] = usart->DATA;
	#endif
	if (Buf &(1<<3))
		MB_CPT[MB_N][7]++;
	if (Buf &((1<<4) |(1<<3) |(1<<2)))
		MB_Err |=(1<<MB_N);
}

// ~~~~~~~~~~~~~~~~~~~~
void
MB_Transm(uint8_t MB_N)
{
	MB_State[MB_N] = MB_Trans;
	USART_Func(MB_N, RS485);
	#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		//USART_Reg(MB_N, ucsrb) |=(1<<6);
		USART_Reg(MB_N, udr) = MB_Frame[MB_N][0];

		USART_Reg(MB_N, ucsra) |=(1<<6);	//Clear TxC pending interrupt		
		USART_Reg(MB_N, ucsrb) |=(1<<5);

	#elif defined (__AVR_ATxmega128A1__)
		USART_t *usart = prp(&MB_Port[MB_N].usart);
		usart->CTRLA = (usart->CTRLA &~USART_TXCINTLVL_gm) |USART_TXCINTLVL_HI_gc;
		usart->DATA = MB_Frame[MB_N][0];
	#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~
void
Modbus_RX(usart_t *usart)
{
	uint8_t MB_N = MB_Port_N(usart);
	switch (MB_State[MB_N]) {
	case MB_Initi:
		MB_Frame[MB_N][0] = usart_data(usart);
		MB_ClearTimer(MB_N);
		break;
	case MB_Idle: 
		MB_State[MB_N] = MB_Recept;
		MB_Err &=~(1<<MB_N);
		MB_Frame_Sz[MB_N] = 0;
		MBRecept(MB_N);
		MB_StartTimer(MB_N);
		break;
	case MB_Recept:
		/*if(TCNT2>=prb(&(BR_Tbl+MB_BitRate_RAM[MB_N])->Time15))
			MB_Err &=(1<<MB_N);*/
		MBRecept(MB_N);
		MB_ClearTimer(MB_N);
		break;
	case MB_Trans: case MB_Proc: 
	case MB_NoReply: //в MB_NoReply нельзя переходить в MB_Recept т.к. в MB_M_Cycle может перейти MB_Trans
		usart_data(usart);
		break;
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~
void
Modbus_TX(usart_t *usart)
{
	uint8_t MB_N = MB_Port_N(usart);
	static uint8_t Count[Modbus_Qt];
	if (Count[MB_N] == MB_Frame_Sz[MB_N]-2){
		USART_Reg(MB_N, ucsrb) &=~(1<<5);
		USART_Reg(MB_N, ucsrb) |=(1<<6);
		USART_Reg(MB_N, ucsra) |=(1<<6);	//Clear TxC pending interrupt		
	}
	if (++Count[MB_N]<MB_Frame_Sz[MB_N]) {
		usart_data(usart) = MB_Frame[MB_N][Count[MB_N]];
		return;
	}

	void MB_S_Tx(uint8_t);
	void MB_M_Tx(uint8_t);
	#if defined (__AVR_ATmega16__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
		USART_Reg(MB_N, ucsrb) &=~((1<<6)|(1<<5)); 	// Доп. Запрет USART Data Register Empty Interrupt
	#elif defined (__AVR_ATxmega128A1__)
		usart->CTRLA = (usart->CTRLA &~USART_TXCINTLVL_gm) |USART_TXCINTLVL_OFF_gc;
	#endif
	USART_Func(MB_N, RS485);
	Count[MB_N] = 0;
	//StartTime3_5();
	MB_State[MB_N] = MB_Idle;
	switch (prb(MB_Role+MB_N)) {
	case Role_Slave:
		MB_S_Tx(MB_N);
		break;
	case Role_Master:
		MB_M_Tx(MB_N);
		break;
	}
}

// ~~~~~~~~~~~~~~~~~~
uint8_t
CommErr(uint8_t MB_N)
{
	uint8_t Sz = MB_Frame_Sz[MB_N]-2;
	uint8_t Err = MB_Err &(1<<MB_N) || MB_Frame_Sz[MB_N]<3 || *(uint16_t*)(MB_Frame[MB_N]+Sz)!=CRC(rrb, MB_Frame[MB_N], Sz);
	if (Err) {
		MB_CPT[MB_N][1]++;
		USART_Func(MB_N, LED_Err_On);
	}
	else {
		MB_CPT[MB_N][0]++;
		USART_Func(MB_N, LED_Err_Off);
	}
	return Err;
}

// ~~~~~~~~~~~~~~~~~~~~
void
SetCRC_MB(uint8_t MB_N)
{
	uint8_t Sz = MB_Frame_Sz[MB_N]-2;
	*(uint16_t*)(MB_Frame[MB_N]+Sz) = CRC(rrb, MB_Frame[MB_N], Sz);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint16_t
MB_FrameReg(uint8_t MB_N, uint8_t i)
{
	return MB_Frame[MB_N][i]<<8 | MB_Frame[MB_N][i+1];
}

// ~~~~~~~~~~~
void
MB_Cycle(void)
{
	void MB_S_Timer_ISR(uint8_t);
	void MB_M_Timer_ISR(uint8_t);

	for (uint8_t MB_N=0; MB_N<Modbus_Qt; MB_N++){
		if(MB_TimerFlag & (1<<MB_N)){
			cli();
			MB_TimerFlag &= ~(1<<MB_N);
			sei();
			switch (prb(MB_Role+MB_N)) {
			case Role_Slave:
				MB_S_Timer_ISR(MB_N);
				break;
			case Role_Master:
				MB_M_Timer_ISR(MB_N);
				break;
			}
		}
	}

	void MB_M_Cycle(uint8_t);

	for (uint8_t MB_N=0; MB_N<Modbus_Qt; MB_N++)
		if (prb(MB_Role+MB_N)==Role_Master)
			MB_M_Cycle(MB_N);
}

// ~~~~~~~~~~~~~~~~~~~~~~~
void
MB_CPT_Clear(uint8_t MB_N)
{
	for (uint8_t i=0; i<8; MB_CPT[MB_N][i++]=0);
}

#include "modbus_s.h"
#include "modbus_m.h"
