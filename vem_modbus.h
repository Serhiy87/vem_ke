void Modbus_ReadHoldReg(void);

// ~~~~~~~~~~~
void Modbus_App_Init(void){

	Modbus_ReadHoldReg();
}

void 
GSMApply(void){//===================================================== Применение настроек GSM

	ewbl(&MBHoldReg1[1], &GPRS_apn, sizeof(GPRS_apn));
	wdt_reset();
	ewbl(&MBHoldReg1[16], &GPRS_apn_user_name, sizeof(GPRS_apn_user_name));
	wdt_reset();
	ewbl(&MBHoldReg1[26], &GPRS_apn_psw, sizeof(GPRS_apn_psw));
	wdt_reset();
	eww(&TCP_ListenPort, MBHoldReg1[36]);
	eww(&TCP_CONNECT_timeout, MBHoldReg1[37]);

	ewb(&TCP_ServerIP->IP1, MBHoldReg1[38]);
	ewb(&TCP_ServerIP->IP2, MBHoldReg1[39]);
	ewb(&TCP_ServerIP->IP3, MBHoldReg1[40]);
	ewb(&TCP_ServerIP->IP4, MBHoldReg1[41]);
	eww(&TCP_ServerPort[0], MBHoldReg1[42]);
	eww(&SerialNum, MBHoldReg1[43]);
	
}

void Modbus_ReadHoldReg(void){
	IntOff();
	MBHoldReg1[0] = erw(&EM_UART_Param);
	erbl(&MBHoldReg1[1], &GPRS_apn, sizeof(GPRS_apn));
	erbl(&MBHoldReg1[16], &GPRS_apn_user_name, sizeof(GPRS_apn_user_name));
	erbl(&MBHoldReg1[26], &GPRS_apn_psw, sizeof(GPRS_apn_psw));
	MBHoldReg1[36] = erw(&TCP_ListenPort);
	MBHoldReg1[37] = erw(&TCP_CONNECT_timeout);
	MBHoldReg1[38] = erb(&TCP_ServerIP->IP1);
	MBHoldReg1[39] = erb(&TCP_ServerIP->IP2);
	MBHoldReg1[40] = erb(&TCP_ServerIP->IP3);
	MBHoldReg1[41] = erb(&TCP_ServerIP->IP4);
	MBHoldReg1[42] = erw(&TCP_ServerPort[0]);
	MBHoldReg1[43] = erw(&SerialNum);
	IntOn();
}

// ~~~~~~~~~~~
void Modbus_App(void){

	IntOff();
	MBInput1[0] = DigInput.State[0];

	MBInReg1[0]  = (SW_VERSION << 8) + 	SW_SUBVERSION;
	MBInReg1[1]  = MemFail;
	MBInReg1[2]  = (uint16_t)(LiveTime & 0x0000FFFF);
	MBInReg1[3]  = (uint16_t)(LiveTime>>16);
	MBInReg1[4]  = (uint16_t)Temperature[0];	
	MBInReg1[5]  = (uint16_t)GSM_State;
	MBInReg1[6]  = GSM_MyIP.IP1;
	MBInReg1[7]  = GSM_MyIP.IP2;
	MBInReg1[8]  = GSM_MyIP.IP3;
	MBInReg1[9]  = GSM_MyIP.IP4;
	MBInReg1[10] = GSM_ClientIP.IP1;
	MBInReg1[11] = GSM_ClientIP.IP2;
	MBInReg1[12] = GSM_ClientIP.IP3;
	MBInReg1[13] = GSM_ClientIP.IP4;
	MBInReg1[14] = GSM_RX_FIFOMax;
	MBInReg1[15] = GSM_RX_FIFOMax_Transp;
	MBInReg1[16] = EM_RX_FIFOMax;
	uint64_t *ptr64 = (uint64_t*)&MBInReg1[17];
	ptr64[0] = ICCID;
	MBInReg1[21] = (uint16_t)GSM_StateBeforeReset;
	MBInReg1[22] = (uint16_t)Web_POST;
	MBInReg1[23] = (uint16_t)AppProtocol;
	MBInReg1[24] = (uint16_t)WebSession;
	MBInReg1[25] = (uint16_t)WebMode;
	MBInReg1[26] = (uint16_t)WebStrLen;

	IntOn();
	
	//Применение общих настроек GSM
	if(MBCoil1[0] & (1<<0)){
		GSMApply();						//Запись из MB RAM в EEPROM
		Modbus_ReadHoldReg();			//Чтение из EEPROM в регистры MB
		ResetGSMState();				//Перезапуск модема
		IntOff();
		MBCoil1[0] &= ~(1<<0); 			//Сброс маркера в карте Modbus
		IntOn();
	}
	
	IntOff();
	if(MBHoldReg1[0] != erw(&EM_UART_Param)){
		eww(&EM_UART_Param, MBHoldReg1[0]);
		//EM_SetUARTParam(erw(&EM_UART_Param));	//применение настроек UART
	}
	IntOn();
}


