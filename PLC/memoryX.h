// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ������ � DataFlash �������� ������� ������ ����� (��� ������� ��������� ������ � �)
// ������ ��������� avr gcc 16 ��� - ����� ����, ������� ��� ��������� ���������� ������� ���������� 32 ����
// ���������� ����� �������� � �������� �������� - define DF_PageSize 256,512,1024,2048 ����
// ����� ���������� ������� ��� �������� SPI 125���
// 07.10.2013 - Kononenko -  Revised for vmd2_2 (PORT_CS_DATAFLASH, DDR_CS_DATAFLASH, CS_DATAFLASH)
// 01.09.2016 - Potapov - change Timer8_xxxxx - > Timer8_ISR_xxxxx
// ~~~~~~~~~~~~~~~~~~~~~~
#ifdef DataFlash_PageSize
// ~~~~~~~~~~~~~~~~~~~~~~

#define DF_PageSize DataFlash_PageSize	// ������ ������ ���������� ����� ���� 256,512,1024,2048 ����

//Table 11-1. Status Register Format
//Bit 7 Bit 6 Bit 5 Bit 4 Bit 3 Bit 2 Bit 1 Bit 0
//RDY/BUSY COMP 1 0 0 1 PROTECT PAGE SIZE
	#define RDY_BUSY	7
	#define COMP 		6
	#define PROTECT 	1
	#define PAGE_SIZE	0

// ��������� DataFlash 0 - Failed, 1 -Ok
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
// ����� ���������� �� 16���
void Wait_DF(void){
	// ������ �� ���������!!!
	if(DF_Status){
		// 2-� ������� ����� - 2*(40�� Page Erase and Programming Time)
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
// �Power of 2� Binary Page Size Option
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
	Wait_DF(); // ���� ��� ����������� ������ ����� ���������
	DF_NoCS();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ����� ���������� �� 48���
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
// ����� ���������� �� 48���
void DF_CommNoData(uint8_t Opcode, uint8_t Adr1, uint8_t Adr2, uint8_t Adr3){
	DF_Comm(Opcode, Adr1, Adr2, Adr3);
	DF_NoCS();
}
// ����� ���������� �� 80 ���
void DF_StartReadPage(uint32_t addr){
	DF_Comm(0xD2, DF_ConvertAddr(addr));
	SPDR = 0;		// �������� ����
	WaitSPI();
	SPDR = 0;		// �������� ����
	WaitSPI();
	SPDR = 0;		// �������� ����
	WaitSPI();
	SPDR = 0;		// �������� ����
	WaitSPI();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ����� ���������� �� 88 ���
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
// �� ������ ��������� - ����� ���������� �� 176 ���
// �� �� ����� �������� - ����� ���������� �� 96 ���
uint16_t erwX(uint32_t addr){
	if(DF_Status){
		uint16_t Result;
		if( (addr & (DF_PageSize-1)) >= DF_PageSize-1 ){
			// �� ������ ���������
			Result = (uint16_t)erbX(addr) + ((uint16_t)erbX(addr+1)<<8);
		}
		else{
		// �� �� ����� ��������
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
// �� ������ ��������� - ����� ���������� �� 192 ���
// �� �� ����� �������� - ����� ���������� �� 192 ���
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
// ����� ���������� �� 160��� + size* 8���
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
// ����� ������ ������� 14�� + ����� �� �������� 0,2�� + ����� �� �������� �� SPI 0,152��
void ewbX(uint32_t addr, uint8_t val){
	if(DF_Status){
	// Main Memory Page to Buffer Transfer
		DF_CommNoData(0x53, DF_ConvertAddr(addr));	//	tXFR Page to Buffer Transfer Time Typ 200 �s
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
// �� ������ ��������� - 2*(����� ������ ������� 14�� + ����� �� �������� 0,2�� + ����� �� �������� �� SPI 0,152��) 
// �� �� ����� �������� - ����� ������ ������� 14�� + ����� �� �������� 0,2�� + ����� �� �������� �� SPI 0,160�� 

void ewwX(uint32_t addr, uint16_t val){
	if(DF_Status){
		if( (addr & (DF_PageSize-1)) >= DF_PageSize-1 ){
			// �� ������ ���������
			ewbX(addr,(uint8_t)(val & 0x00FF));
			ewbX(addr+1,(uint8_t)((val >> 8) & 0x00FF));
		}
		else{
		// �� �� ����� ��������
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
// 2*(����� ������ ������� 14�� + ����� �� �������� 0,2�� + ����� �� �������� �� SPI 0,144��) + 0,032��
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
// 2*(����� ������ ������� 14�� + ����� �� �������� 0,2�� + ����� �� �������� �� SPI 0,144��) + size*0,008��
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
