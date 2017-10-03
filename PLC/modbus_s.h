/*
	17.09.2012 - Kononenko - Add define MB_ADDR_DIP_SW
	04.10.2014 - Kononenko - uint8_t->uint16_t Modbus_Map.xxxxx_Qt
	01.09.2016 - Kononenko - Remove MB_NativeAddr, Set_MB_Addr to modbus.h
*/
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline static uint8_t
MB_S_ReadDiscr(uint8_t MB_N, uint16_t StartAddr)
{
	if (MB_Frame[MB_N][0]==0)
		return 0xFF;
	if (MB_Frame_Sz[MB_N]!=8)
		return MB_Frame[MB_N][2]=IllegData;
	uint16_t Qt = MB_FrameReg(MB_N, 4);
	if (StartAddr+Qt>(MB_Frame[MB_N][1]==1?prw(&Modbus_Map[MB_N].Coil_Qt):prw(&Modbus_Map[MB_N].Input_Qt)))
		return MB_Frame[MB_N][2]=IllegAddr;
	uint8_t *Discr = MB_Frame[MB_N][1]==1 ? prp(&Modbus_Map[MB_N].Coil) : prp(&Modbus_Map[MB_N].Input);
	MB_Frame[MB_N][2] = Qt/8+(Qt%8>0);
	MB_Frame_Sz[MB_N] = MB_Frame[MB_N][2]+5;
	for (uint8_t i=0; i<Qt; i++) {
		uint16_t j = StartAddr+i;
		uint8_t Byte=3+i/8, Bit=1<<i%8;
		if (Discr[j/8] &(1<<j%8))
			MB_Frame[MB_N][Byte] |=Bit;
		else
			MB_Frame[MB_N][Byte] &=~Bit;
	}
	return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline static uint8_t
MB_S_ReadReg(uint8_t MB_N, uint16_t StartAddr)
{
	if (MB_Frame[MB_N][0]==0)
		return 0xFF;
	if (MB_Frame_Sz[MB_N]!=8)
		return MB_Frame[MB_N][2]=IllegData;
	if (StartAddr+MB_Frame[MB_N][5]>(MB_Frame[MB_N][1]==3?prw(&Modbus_Map[MB_N].HoldReg_Qt):prw(&Modbus_Map[MB_N].InReg_Qt)))
		return MB_Frame[MB_N][2]=IllegAddr;
	uint16_t *Reg = (uint16_t*)(MB_Frame[MB_N][1]==3?prp(&Modbus_Map[MB_N].HoldReg):prp(&Modbus_Map[MB_N].InReg)) + StartAddr;
	MB_Frame_Sz[MB_N] = (MB_Frame[MB_N][2] = MB_Frame[MB_N][5]*2) + 5;
	uint8_t Qt = MB_Frame[MB_N][5];
	for (uint8_t i=0; i<Qt; i++) {
		uint8_t j = i*2;
		MB_Frame[MB_N][3+j] = Reg[i]>>8;
		MB_Frame[MB_N][4+j] = Reg[i];
	}
	return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline static uint8_t
MB_S_PresetSingle(uint8_t MB_N, uint16_t StartAddr)
{
	if (MB_Frame_Sz[MB_N]!=8)
		return MB_Frame[MB_N][2]=IllegData;
	if (StartAddr+1>prw(&Modbus_Map[MB_N].HoldReg_Qt))
		return MB_Frame[MB_N][2]=IllegAddr;
	MB_Frame_Sz[MB_N] = 8;
	*((uint16_t*)prp(&Modbus_Map[MB_N].HoldReg)+StartAddr) = MB_FrameReg(MB_N, 4);
	return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline static uint8_t
MB_S_Force(uint8_t MB_N, uint16_t StartAddr)
{
	uint16_t Qt = MB_FrameReg(MB_N, 4);
	if (MB_Frame_Sz[MB_N]!=MB_Frame[MB_N][6]+9 || Qt/8+(Qt%8>0)!=MB_Frame[MB_N][6])
		return MB_Frame[MB_N][2]=IllegData;
	if (StartAddr+Qt > prw(&Modbus_Map[MB_N].Coil_Qt))
		return MB_Frame[MB_N][2]=IllegAddr;
	uint8_t *Coil = prp(&Modbus_Map[MB_N].Coil);
	MB_Frame_Sz[MB_N] = 8;
	for (uint8_t i=0; i<Qt; i++) {
		uint16_t j = StartAddr+i;
		uint16_t Byte=j/8, Bit=1<<j%8;
		if (MB_Frame[MB_N][7+i/8] &(1<<i%8))
			Coil[Byte] |=Bit;
		else
			Coil[Byte] &=~Bit;
	}
	return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
inline static uint8_t
MB_S_Preset(uint8_t MB_N, uint16_t StartAddr)
{
	if (MB_Frame_Sz[MB_N]!=MB_Frame[MB_N][6]+9 || MB_Frame[MB_N][5]*2!=MB_Frame[MB_N][6])
		return MB_Frame[MB_N][2]=IllegData;
	if (StartAddr+MB_Frame[MB_N][5]>prw(&Modbus_Map[MB_N].HoldReg_Qt))
		return MB_Frame[MB_N][2]=IllegAddr;
	MB_Frame_Sz[MB_N] = 8;
	uint16_t *HoldReg = prp(&Modbus_Map[MB_N].HoldReg);
	for (uint8_t i=0; i<MB_Frame[MB_N][5]; i++) {
		uint8_t j = i*2;
		HoldReg[StartAddr+i] = MB_FrameReg(MB_N, 7+j);
	}
	return 0;
}

// ~~~~~~~~~~~~~~~~~~
inline static uint8_t
Process(uint8_t MB_N)
{
	uint8_t Err;
	uint16_t StartAddr = MB_FrameReg(MB_N, 2);
	switch (MB_Frame[MB_N][1]) {
	case 1: case 2:
		Err = MB_S_ReadDiscr(MB_N, StartAddr);
		break;
	case 3: case 4:
		Err = MB_S_ReadReg(MB_N, StartAddr);
		break;
	case 6:
		Err = MB_S_PresetSingle(MB_N, StartAddr);
		break;
	case 0x0F:
		Err = MB_S_Force(MB_N, StartAddr);
		break;
	case 0x10:
		Err = MB_S_Preset(MB_N, StartAddr);
		break;
	default:
		Err = MB_Frame[MB_N][2] = IllegFunc;
		break;
	}
	if (MB_Frame[MB_N][0]) {
		if (Err) {
			MB_Frame[MB_N][1] |=(1<<7);
			MB_Frame_Sz[MB_N] = 5;
		}
		SetCRC_MB(MB_N);
	}
	return Err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~
inline static void
MB_S_CtrlProc(uint8_t MB_N)
{
	if (CommErr(MB_N)) {
		MB_State[MB_N] = MB_Idle;
		return;
	}
	if (MB_Frame[MB_N][0]!=0 && MB_Frame[MB_N][0]!=MB_NativeAddr[MB_N]) {
		MB_State[MB_N] = MB_Idle;
		return;
	}
	USART_Func(MB_N, LED);
	MB_CPT[MB_N][3]++;
	if (Process(MB_N))
		MB_CPT[MB_N][2]++;
	if (MB_Frame[MB_N][0])
		MB_Transm(MB_N);
	else {
		MB_CPT[MB_N][4]++;
		MB_State[MB_N] = MB_Idle;
		USART_Func(MB_N, LED);
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~
void
MB_S_Timer_ISR(uint8_t MB_N)
{
	switch (MB_State[MB_N]) {
	case MB_Initi:
		MB_State[MB_N] = MB_Idle;
		break;
	case MB_Recept:
		MB_State[MB_N] = MB_Proc;
		MB_S_CtrlProc(MB_N);
		break;
	}
}

// ~~~~~~~~~~~~~~~~~~
void
MB_S_Tx(uint8_t MB_N)
{
	USART_Func(MB_N, LED);
}

// ~~~~~~~~~~~~~~~~~~~~
void
MB_S_Init(uint8_t MB_N)
{
	#ifndef MB_ADDR_DIP_SW
		MB_NativeAddr[MB_N] = erb(MB_NativeAddr_EE+MB_N);
	#endif
}
