/*
	29.07.2015 - Kucherenko - Initial version
*/
#ifdef QslRD

enum {Sl_RD, NoSl_RD};
prog_uint8_t Slave_RD[QslRD] = InitSlaveRD;

uint8_t FlagLinkRD,IndexMB_SB,MBCoilRKEY,old_MBCoilRNUM,TempRMON;
static uint8_t TD_Sl_RD_Write,TD_Sl_RD_Link;

void SlaveRD_Init(void){
	TD_Sl_RD_Write = Timer16SysAlloc(1);
	TD_Sl_RD_Link = Timer16SysAlloc(1);
	if(Timer16AllocErr()) ShowMsg(&MsgErr41);
}
void WriteRMON(uint16_t Value){
	for(uint8_t i = 0; i < LCDXSz*LCDYSz/2; i++){
		IntOff();
		*((uint16_t*)RMON+i) = Value;
		IntOn();
	}
}
void SlaveRD(void){
	IntOff();
	uint8_t MBCoilRNUM = *(uint8_t*)RNUM;
	IntOn();
	//при изменении RNUM(номер слейва) в масив содержания экрана записываем прочерки
	if(old_MBCoilRNUM != MBCoilRNUM){
		WriteRMON(0x2D2D);//"--"
	}
	//если в RemoteDisplay_F выбран Slave
	if(MBCoilRNUM){
		if(prb(Slave_RD+MBCoilRNUM-1) == Sl_RD){
			//проверка связи со слейвами
			if(Timer16Stopp(TD_Sl_RD_Write)){
				//каждую секунду в 1-й после экрана регистр (используется для проверки связи) записываем число 255 (TempRMON)
				StartTimer16(TD_Sl_RD_Write, 100);
				IntOff();
				*((uint8_t*)RMON+LCDXSz*LCDYSz) = TempRMON;
				IntOn();
			}

			IntOff();
			TempRMON = *((uint8_t*)RMON+LCDXSz*LCDYSz);//постоянно считываем содержимое 1-го после экрана регистра (используется для проверки связи)
			IntOn();
			if(TempRMON == 255){
				if(FlagLinkRD){
					if(Timer16Stopp(TD_Sl_RD_Link)){
						FlagLinkRD = 0;
						//если на протяжении времени демпфирования (таймер TD_Sl_RD) значение TempRMON всегда было = 255, то выводим прочерки
						WriteRMON(0x2D2D);//"--"
					}
				}
				else{
					StartTimer16(TD_Sl_RD_Link, 500);
					FlagLinkRD = 1;
				}
			}
			else{
				FlagLinkRD = 0;
				TempRMON = 255;
			}
			//обмен данными с выбраным слейвом
			if(!MB_SingleBusy(Port_Master)){
				switch(IndexMB_SB){
					case 0:
						old_MBCoilRNUM = MBCoilRNUM;
						IntOff();
						MBCoilRKEY = *(uint8_t*)RKEY;
						IntOn();
						if(MBCoilRKEY){
							MB_SingleRun(Port_Master, MBCoilRNUM, 15, ADR_RKEY*8, 6, &MBCoilRKEY);
							IndexMB_SB = 1;
						}
						else IndexMB_SB = 2;
						break;
					case 1:
						IntOff();
						*(uint8_t*)RKEY = 0;
						IntOn();
						IndexMB_SB = 2;
						break;
					case 2:
					#if RTYP+1 == RMON//если адрес RTYP находится рядом с адресом RMON
						MB_SingleRun(Port_Master, MBCoilRNUM, 4, ADR_RTYP, LCDXSz*LCDYSz/2+2, (uint8_t*)RTYP);
					#else
						MB_SingleRun(Port_Master, MBCoilRNUM, 4, ADR_RTYP, 1, (uint8_t*)RTYP);
						IndexMB_SB = 3;
						break;
					case 3:
						MB_SingleRun(Port_Master, MBCoilRNUM, 4, ADR_RMON, LCDXSz*LCDYSz/2+1, (uint8_t*)RMON);
					#endif
						IndexMB_SB = 0;
						break;
				}
				IntOff();
				*(uint16_t*)RTYP = (QslRD<<8)+*(uint16_t*)RTYP%256;
				IntOn();
			}
		}
		else{
			old_MBCoilRNUM = MBCoilRNUM;
			WriteRMON(0x5858);//"ХХ"
		}	
	}
	//если в RemoteDisplay_F выбран Master
	else{
		old_MBCoilRNUM = MBCoilRNUM;
		IntOff();
		*(uint16_t*)RTYP = (QslRD<<8) + R_PLC_Type;
		IntOn();
	
		for(uint8_t Y = 0; Y<LCDYSz; Y++){
			for(uint8_t X = 0; X<LCDXSz; X++){
				*((uint8_t*)RMON+Y*LCDXSz+X) = prb(Win2LCD+(unsigned char)LCD_Buf[Y][X]);
			}
		}
	}
}

#endif

