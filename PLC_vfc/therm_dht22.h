/* ~~~~~~~~~~~~~
 	29.05.2014 * Kononenko *  Потокол для AM2302 (не официальное название dht22) 

	Для подключения необходим  #define RH_CHANNNEL 1 (0,1,2 - канал)

	Single-bus data is used for communication between MCU and AM2303
	
	В ф-ии Read_DHT22() выполняется чтение влажности и температуры каждые 1.0 сек
	Задержка первого чтения после включения питания не менее 2 сек
	Проверяется контрольная сумма и ограничивается время чтения(защита от зависания)

	Недостаток - необходимо запрещать прерывания ~5мс!
	
	TODO:
	1) 
	2) 
	3) 

~~~~~~~~~~~~~*/
uint16_t RHumidity;
uint8_t DHT22Error;

#define RH_Off_Mark		0xFFFF
#define RH_NoCon_Mark	0xFFFE

void Error_DHT22(const uint8_t CH){
	DS18B20_Error[CH]++;
	DHT22Error++;
	if(DHT22Error>10){
		RHumidity = RH_NoCon_Mark;
		Temperature[CH] = ADC_Off_Mark;
	}
}

// ~~~~~~~~~~~~~
void Read_DHT22(const uint8_t CH){

	if(erb(&ThermType[CH]) == ThermOff){
		Temperature[CH] = Therm_Off_Mark;
		DS18B20_Status[CH] = DS18B20_ERROR;
		RHumidity = RH_Off_Mark;
		return;
	}	

	//MASTER TX RESET PULSE 500us minimum
	cli(); OW_MasterPullLowBus(CH); sei();
	Delay_us(580);
	cli(); OW_MasterReleaseBus(CH); sei();

	//MASTER RX presence pulse 480мs minimum
	Delay_us(10);	//SLAVE waits
	
	uint8_t BitCounter = 0;
	uint8_t PrevBusState = 1;
	uint8_t NewBusState = 1;
	uint16_t TC_New=0;
	uint16_t TC_Prev=0;
	uint8_t RH[5];
	for(uint8_t i=0; i<5; i++)RH[i] = 0;

	//DGT22 read cycle
	cli();	
	uint16_t TC_Start=TCNT1;
	while(BitCounter < 42){
		
		NewBusState = OW_MasterReadBus(CH);	
		
		if( (PrevBusState!=0) && (NewBusState==0) ){//Detect fall front and measuare period
			
			TC_New = TCNT1;
			if(TC_Prev>TC_New) TC_Prev-=20000;
			
			if(BitCounter>=2){
				if( (TC_New-TC_Prev)>200 ){		// if period >100us Bit = 1 else Logic Bit = 0
					RH[(BitCounter-2)/8] |=( 1<<(7 - (BitCounter-2)%8) );
				}
			}
			TC_Prev = TC_New;
			BitCounter++;
		}
		PrevBusState = NewBusState;

		//Finish DHT22 read cycle if > 5,5мс
		uint16_t TC_Cur = TCNT1;
		if( TC_Start>TC_Cur){
			if(TC_Cur + 20000 - TC_Start > 5500*2){ Error_DHT22(CH);return;}
		}
		else{
			if(TC_Cur - TC_Start > 5500*2){ Error_DHT22(CH);return;}
		}
	}
	sei();

	uint8_t MyCRC=0;
	for(uint8_t i=0; i<4; i++) MyCRC += RH[i];
	if(MyCRC != RH[4]){ Error_DHT22(CH);return;}

	RHumidity = (RH[0]<<8) + RH[1];
	int16_t Tres;
	Tres = (int16_t)RH[3];
	Tres |= ((int16_t)RH[2]<<8);
	if(Tres != 850)	//DS18B20 power up value = 85.0C
		Temperature[CH] = (Tres+(int8_t)erb(&ThermCorr[CH].D))*(erw(&ThermCorr[CH].A)/1000.0);
	DHT22Error = 0;
}
// ~~~~~~~~~~~~~


