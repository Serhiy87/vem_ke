/*
	23.12.2013 - Kononenko - Initial version
*/

#ifdef HSC_Reg

volatile uint16_t HSC_WorkReg[HSC_Reg];
volatile uint16_t HSC_Result[HSC_Reg];
uint16_t HSCounter[HSC_Reg];

uint8_t TD_HSC;

// ~~~~~~~~~~~
void
HSC_Init(void)
{
	PCMSK1 = (1<<PCINT11) | (1<<PCINT10);
	PCICR = (1<<PCIE1);

	// High speed counter Timer
	TD_HSC = Timer8SysAlloc(1);	
}

// ~~~~~~~~~~
ISR(PCINT1_vect){
	static uint8_t prev_state;
	uint8_t new_state = PINJ;
	if( (new_state^prev_state) & (1<<PINJ1) ) {HSC_WorkReg[0]++;} 
	if( (new_state^prev_state) & (1<<PINJ2) ) {HSC_WorkReg[1]++;} 
	prev_state = new_state;
}
// ~~~~~~~~~~
inline void HSC_Process(void){
	if(Timer8Stopp(TD_HSC)){
		StartTimer8(TD_HSC,100);
		HSC_Result[0] = HSC_WorkReg[0];
		HSC_Result[1] = HSC_WorkReg[1];
		HSC_WorkReg[0]=0;
		HSC_WorkReg[1]=0;
	}
}
// ~~~~~~~~~~
void
HSC_In(void)
{
	IntOff();
	HSCounter[0] = (HSC_Result[0]>>1);
	HSCounter[1] = (HSC_Result[1]>>1);
	IntOn();	
}

#endif	// def HSC_Reg
// ~~~~~~~~~~~~~~~~~~~
