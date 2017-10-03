// ~~~~~~~~~~~
// 06.02.2014 
// Kononenko - for bug detection and fixing

// ��������, ��� ���������� ���������� ������ �������� ������ ���
// ������ ��� ������� MenuOut(void) - PutLine(Y, Y); 
// ���������� ����������� ������ �� PROGMEM � ������� ���
// ������ MenuOut ��� ������ �� ��������.

// ���� ����������� ��������� ����������� �� ���������� ������.
// ����� �������� ~>12 ����� - ���������� ��������� �� ���� - � ������� 1-10����� �������� ���.

// Modbus �� ������ �� ������ ����.
// 
// 07.02.2014 ������ ������� ������ Magic1
//
// 13.02.2014 ������� Reboot
// 
// ~~~~~~~~~~~

uint8_t ShowBug = 0;
uint8_t Magic1 EEMEM = 0;
uint8_t Magic2 EEMEM = 0;
uint8_t Bug1[10];
uint8_t Bug2[10];
uint8_t Bug3[10];
uint8_t Bug4[10];
uint8_t Bug5[10];
uint8_t Bug6[10];
uint8_t Bug7[10];
uint8_t Bug8[10];
uint8_t Bug9[10];
uint8_t Bug10[10];

// ~~~~~~~~~~~
void Msg_Bug_Load(void){
	ShowBug=0;
}

MsgPage Msg_Bug = {
	{
		{"    Bug!!!      "},
		{"                "}
	},	Msg_Bug_Load, NULL, NULL};

// ~~~~~~~~~~~

void InitBug(void){
	uint8_t i;
	for(i=0;i<10;i++)Bug1[i]=0;
	for(i=0;i<10;i++)Bug2[i]=0;
	for(i=0;i<10;i++)Bug3[i]=0;
	for(i=0;i<10;i++)Bug4[i]=0;
	for(i=0;i<10;i++)Bug5[i]=0;
	for(i=0;i<10;i++)Bug6[i]=0;
	for(i=0;i<10;i++)Bug7[i]=0;
	for(i=0;i<10;i++)Bug8[i]=0;
	for(i=0;i<10;i++)Bug9[i]=0;
	for(i=0;i<10;i++)Bug10[i]=0;
}
uint8_t FixBug(uint8_t Code){
	uint8_t Bug=0;
	uint8_t i;
	for(i=0;i<10;i++) if(Bug1[i]!=0) Bug=1;
	for(i=0;i<10;i++) if(Bug2[i]!=0) Bug=2;
	for(i=0;i<10;i++) if(Bug3[i]!=0) Bug=3;
	for(i=0;i<10;i++) if(Bug4[i]!=0) Bug=4;
	for(i=0;i<10;i++) if(Bug5[i]!=0) Bug=5;
	for(i=0;i<10;i++) if(Bug6[i]!=0) Bug=6;
	for(i=0;i<10;i++) if(Bug7[i]!=0) Bug=7;
	for(i=0;i<10;i++) if(Bug8[i]!=0) Bug=8;
	for(i=0;i<10;i++) if(Bug9[i]!=0) Bug=9;
	for(i=0;i<10;i++) if(Bug10[i]!=0) Bug=10;
	if(Bug){
		ewb(&Magic1, erb(&Magic1)+1);
		ShowBug = 1;
		return Code;
	}
	else return 0;
}
void Reboot(void){
	cli();
	while(1){}
}

// ~~~~~~~~~~~
