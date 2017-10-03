// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Данные в DataFlash записаны младшим байтом вперёд (как принято сохранять данные в С)
// Размер указателя avr gcc 16 бит - этого мало, поэтому для адресации используем обычную переменную 32 бита
// Библиотека может работать с размером страницы - define DF_PageSize 256,512,1024,2048 байт
// Время выполнения указано при скорости SPI 125кГц
// 07.10.2013 - Kononenko -  Revised for vmd2_2 (PORT_CS_DATAFLASH, DDR_CS_DATAFLASH, CS_DATAFLASH)
// 01.09.2016 - Potapov - change Timer8_xxxxx - > Timer8_ISR_xxxxx
// ~~~~~~~~~~~~~~~~~~~~~~
#ifdef DataFlash_PageSize
// ~~~~~~~~~~~~~~~~~~~~~~

#define DF_PageSize DataFlash_PageSize	// размер буфера микросхемы может быть 256,512,1024,2048 байт

//Table 11-1. Status Register Format
//Bit 7 Bit 6 Bit 5 Bit 4 Bit 3 Bit 2 Bit 1 Bit 0
//RDY/BUSY COMP 1 0 0 1 PROTECT PAGE SIZE
	#define RDY_BUSY	7
	#define COMP 		6
	#define PROTECT 	1
	#define PAGE_SIZE	0

// Состояние DataFlash 0 - Failed, 1 -Ok
uint8_t DF_Status = 1;
uint8_t DF_Timer ;

#define DF_ConvertAddr(addr) (uint8_t)(((addr)>>16) & 0x000000FF), (uint8_t)(((addr)>>8) & 0x000000FF), (uint8_t)((addr) & 0x000000FF)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DF_CS(void){
	cli();
	SPCR |=(1<<CPOL);
	//PORTF &= ~(1<<PF3);	//CS
	PORT_CS_DATAFLASH &= ~CS_DATAFLASH;
	sei();
}
void DF_NoCS(void){
	cli();
	//PORTF |= (1<<PF3);	//CS
	PORT_CS_DATAFLASH |= CS_DATAFLASH;
	sei();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// время выполнения от 16мкс
void Wait_DF(void){
	// Защита от зависания!!!
	if(DF_Status){
		// 2-х кратный запас - 2*(40мс Page Erase and Programming Time)
		StartTimer8_ISR(DF_Timer,8);
		do{
			DF_CS();
			SPDR = 0xD7;	// Status Register Read (Opcode D7H)
			WaitSPI();
			SPDR = 0;
			WaitSPI();
			DF_NoCS();
		}
		while( ( (SPDR & ( (1<<RDY_BUSY) | (1<<PROTECT) | (1<<PAGE_SIZE) ) ) != ( (1<<RDY_BUSY) | (1<<PAGE_SIZE) ) )
				&& !Timer8Stopp_ISR(DF_Timer) );
		if(Timer8Stopp_ISR(DF_Timer)){
			ShowMsg(&MsgErr31);
			DF_Status = 0;
		}
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// “Power of 2” Binary Page Size Option
// Programming the Configuration Register
void DF_Init(void){
	DF_Timer = Timer8_ISR_SysAlloc(1);

	cli();
	SPCR |=(1<<CPOL);
	PORT_CS_DATAFLASH |= CS_DATAFLASH;
	DDR_CS_DATAFLASH |= CS_DATAFLASH;
	PORT_CS_DATAFLASH &= ~CS_DATAFLASH;
	sei();

	SPDR = 0x3D;
	WaitSPI();
	SPDR = 0x2A;
	WaitSPI();
	SPDR = 0x80;
	WaitSPI();
	SPDR = 0xA6;
	WaitSPI();
	DF_NoCS();
	//	tP Page Programming Time (256/264 bytes) 4 ms max
	DF_CS();
	Wait_DF(); // Если нет правильного ответа выдаёт сообщение
	DF_NoCS();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// время выполнения от 48мкс
uint8_t DF_Comm(uint8_t Opcode, uint8_t Adr1, uint8_t Adr2, uint8_t Adr3){
	Wait_DF();
	DF_CS();
	SPDR = Opcode;		// Status Register Read (Opcode D7H)
	WaitSPI();
	SPDR = Adr1;
	WaitSPI();
	SPDR = Adr2;
	WaitSPI();
	SPDR = Adr3;
	WaitSPI();
	return 1;
}
// время выполнения от 48мкс
void DF_CommNoData(uint8_t Opcode, uint8_t Adr1, uint8_t Adr2, uint8_t Adr3){
	DF_Comm(Opcode, Adr1, Adr2, Adr3);
	DF_NoCS();
}
// время выполнения от 80 мкс
void DF_StartReadPage(uint32_t addr){
	DF_Comm(0xD2, DF_ConvertAddr(addr));
	SPDR = 0;		// Холостой байт
	WaitSPI();
	SPDR = 0;		// Холостой байт
	WaitSPI();
	SPDR = 0;		// Холостой байт
	WaitSPI();
	SPDR = 0;		// Холостой байт
	WaitSPI();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// время выполнения от 88 мкс
uint8_t erbX(uint32_t addr){
	if(DF_Status){
		DF_StartReadPage(addr);
		SPDR = 0;
		WaitSPI();
		DF_NoCS();
		return SPDR;
	}
	else return 0;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// на разных страницах - время выполнения от 176 мкс
// всё на одной странице - время выполнения от 96 мкс
uint16_t erwX(uint32_t addr){
	if(DF_Status){
		uint16_t Result;
		if( (addr & (DF_PageSize-1)) >= DF_PageSize-1 ){
			// на разных страницах
			Result = (uint16_t)erbX(addr) + ((uint16_t)erbX(addr+1)<<8);
		}
		else{
		// всё на одной странице
			DF_StartReadPage(addr);
			SPDR = 0;
			WaitSPI();
			Result = SPDR;
			SPDR = 0;
			WaitSPI();
			Result += SPDR<<8;
			DF_NoCS();
		}
		return Result;
	}
	else return 0;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// на разных страницах - время выполнения от 192 мкс
// всё на одной странице - время выполнения от 192 мкс
uint32_t erdX(uint32_t addr){
	if(DF_Status){
		uint32_t Result = 0;
		uint32_t i;
		uint8_t c=0;
		DF_StartReadPage(addr);
		for(i = (addr & (DF_PageSize-1)); i<DF_PageSize && c<4; i++){
			SPDR = 0;
			WaitSPI();
			Result += (uint32_t)SPDR<<(c*8);
			c++;
		}
		DF_NoCS();
		DF_StartReadPage(addr+c);
		for(i = c; i<4; i++){
			SPDR = 0;
			WaitSPI();
			Result += (uint32_t)SPDR<<(c*8);
			c++;
		}
		DF_NoCS();
		return Result;
	}
	else return 0;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// время выполнения от 160мкс + size* 8мкс
void erblX(uint32_t addr, void *dst, uint8_t size)
{
	if(DF_Status){
		uint32_t i;
		uint8_t c=0;
		DF_StartReadPage(addr);
		for(i = (addr & (DF_PageSize-1)); i<DF_PageSize && c<size; i++){
			SPDR = 0;
			WaitSPI();
			*((uint8_t*)dst+c) = SPDR;
			c++;
		}
		DF_NoCS();
		DF_StartReadPage(addr+c);
		for(i = c; i<size; i++){
			SPDR = 0;
			WaitSPI();
			*((uint8_t*)dst+c) = SPDR;
			c++;
		}
		DF_NoCS();
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Время записи типично 14мс + время на передачу 0,2мс + время на передачу по SPI 0,152мс
void ewbX(uint32_t addr, uint8_t val){
	if(DF_Status){
	// Main Memory Page to Buffer Transfer
		DF_CommNoData(0x53, DF_ConvertAddr(addr));	//	tXFR Page to Buffer Transfer Time Typ 200 µs
	// Buffer Write
		DF_Comm(0x84, DF_ConvertAddr(addr));
		SPDR = val;
		WaitSPI();
		DF_NoCS();
	//Buffer to Main Memory Page Program with Built-in Erase
		DF_CommNoData(0x83, DF_ConvertAddr(addr));	//	tEP	Page Erase and Programming Time	(256/264 bytes) Max 35 ms
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// на разных страницах - 2*(Время записи типично 14мс + время на передачу 0,2мс + время на передачу по SPI 0,152мс) 
// всё на одной странице - Время записи типично 14мс + время на передачу 0,2мс + время на передачу по SPI 0,160мс 

void ewwX(uint32_t addr, uint16_t val){
	if(DF_Status){
		if( (addr & (DF_PageSize-1)) >= DF_PageSize-1 ){
			// на разных страницах
			ewbX(addr,(uint8_t)(val & 0x00FF));
			ewbX(addr+1,(uint8_t)((val >> 8) & 0x00FF));
		}
		else{
		// всё на одной странице
			DF_CommNoData(0x53, DF_ConvertAddr(addr));
			DF_Comm(0x84, DF_ConvertAddr(addr));
			SPDR = (uint8_t)(val & 0x00FF);
			WaitSPI();
			SPDR = (uint8_t)((val >> 8) & 0x00FF);
			WaitSPI();
			DF_NoCS();
			DF_CommNoData(0x83, DF_ConvertAddr(addr));
		}
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2*(Время записи типично 14мс + время на передачу 0,2мс + время на передачу по SPI 0,144мс) + 0,032мс
void ewdX(uint32_t addr, uint32_t val){
	if(DF_Status){
		uint32_t i;
		uint8_t c=0;
		DF_CommNoData(0x53, DF_ConvertAddr(addr));
		DF_Comm(0x84, DF_ConvertAddr(addr));
		for(i = (addr & (DF_PageSize-1)); i<DF_PageSize && c<4; i++){
			SPDR = (uint8_t)((val>>(c*8)) & 0x000000FF);
			WaitSPI();
			c++;
		}
		DF_NoCS();
		DF_CommNoData(0x83, DF_ConvertAddr(addr));

		DF_CommNoData(0x53, DF_ConvertAddr(addr+c));
		DF_Comm(0x84, DF_ConvertAddr(addr+c));
		for(i = c; i<4; i++){
			SPDR = (uint8_t)((val>>(c*8)) & 0x000000FF);
			WaitSPI();
			c++;
		}
		DF_NoCS();
		DF_CommNoData(0x83, DF_ConvertAddr(addr+c));
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2*(время записи типично 14мс + время на передачу 0,2мс + время на передачу по SPI 0,144мс) + size*0,008мс
void ewblX(void *src, uint32_t addr, uint8_t size){
	if(DF_Status){
		uint32_t i;
		uint8_t c=0;
		DF_CommNoData(0x53, DF_ConvertAddr(addr));
		DF_Comm(0x84, DF_ConvertAddr(addr));
		for(i = (addr & (DF_PageSize-1)); i<DF_PageSize && c<size; i++){
			SPDR = *((uint8_t*)src+c);
			WaitSPI();
			c++;
		}
		DF_NoCS();
		DF_CommNoData(0x83, DF_ConvertAddr(addr));

		DF_CommNoData(0x53, DF_ConvertAddr(addr+c));
		DF_Comm(0x84, DF_ConvertAddr(addr+c));
		for(i = c; i<size; i++){
			SPDR = *((uint8_t*)src+c);
			WaitSPI();
			c++;
		}
		DF_NoCS();
		DF_CommNoData(0x83, DF_ConvertAddr(addr+c));
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#else
	#define DF_Init()
#endif	//def DataFlash_PageSize
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
