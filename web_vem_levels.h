uint8_t TempWebAction;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		login.html
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const prog_char name_login[] = "login.html";

	 prog_char Login_Welcome_Str[]	= "Welcome to EMReader! Please log in!              ";
	 prog_char Login_Busy_Str[]		= "EMReader is busy now. Another user start session.";
	 prog_char Login_Login_Str[]		= "Login is not valid!                              ";
	prog_char Login_PSW_Str[]		= "Password is not valid!                           ";

	prog_str   Web_LoginMsg_List[] = { Login_Welcome_Str, Login_Busy_Str, Login_Login_Str, Login_PSW_Str};

InField login_InField[] = {
	{EE_Text, &Title, NULL, sizeof(Title)},
	{Enum, &Web_Login_Code, Web_LoginMsg_List,  0},
	{Word, &ICCID1, NULL,  0},
	{z_Longint, &ICCID2, NULL,  0},
};


const prog_char login_page[] = {
"<HTML>\r\n"
"<HEAD>\r\n"
"<TITLE>{{{{{{{{{{{{{{{{{{{{</TITLE>\r\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">\r\n"
"</HEAD>\r\n"
"<BODY ALIGN=CENTER>\r\n"
"{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{</BR></BR>\r\n"
"ID {{{{ {{{{{{{{{</BR></BR>\r\n"
"<FORM action=\'/\' method=\'POST\'>\r\n"
"\tLogin <input type=\'text\' name=\'login\' size=\'15\'></BR></BR>\r\n"
"\tPassword <input type=\'password\' name=\'password\' size=\'10\'></BR></BR>\t\r\n"
"\t<input type=\'submit\' value=\'OK\'></BR>\r\n"
"</FORM>\r\n"
"</BODY>\r\n"
"</HTML>\r\n"
};

const prog_char vars_Login[] 	= "login";
OutField Web_LoginOF[] = { {Text, &Web_Login_Str, DIGIT, ENG_CAPS, NULL, NULL, sizeof(Web_Login_Str)} };
const prog_char vars_Passw[] 	= "password";
OutField Web_PasswOF[] = { {Text, &Web_Passw_Str, DIGIT, ENG_CAPS, Web_Login_Request, NULL, sizeof(Web_Passw_Str)} };

WebOutField vars_loginOField[] = {
	{vars_Login, Web_LoginOF},
	{vars_Passw, Web_PasswOF},
};

WebPage Web_login = { 
	name_login, 
	login_InField,	//index_InField,
	sizeof(vars_loginOField),
	vars_loginOField,
	sizeof(login_page),
	login_page,
	0
};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		index.html
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const prog_char name_index[] = "index.html";


	
InField index_IField[] = {
	{EE_Text, &Title, NULL, sizeof(Title)},
	{EE_Text, &Title, NULL, sizeof(Title)},
	{Word, &ICCID1, NULL,  0},
	{z_Longint, &ICCID2, NULL,  0},

	{Longint, &E_Meter_RAM.Ap, NULL, 3},
	{Longint, &E_Meter_RAM.Am, NULL, 3},
	{Longint, &E_Meter_RAM.Rp, NULL, 3},
	{Longint, &E_Meter_RAM.Rm, NULL, 3},
	{Byte, &GSM_RSSI, NULL, 0},
	{Longint, &LiveTime, NULL,  2},

};

const prog_char index_page[] = {
"<HTML>\r\n"
"<HEAD>\r\n"
"<TITLE>{{{{{{{{{{{{{{{{{{{{</TITLE>\r\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">\r\n"
"</HEAD>\r\n"
"<BODY>\r\n"
"\r\n"
"<H2>EMReader. {{{{{{{{{{{{{{{{{{{{</H2>\r\n"
"ID: {{{{ {{{{{{{{{ \tSW: "SW_VERSION_Str"</BR>\r\n"
"<HR>\r\n"
"<a href=\"index.html\">Главная</a> | \r\n"
"<a href=\"emeter.html\">Настройки счетчика</a> | \r\n"
"<a href=\"network.html\">Сетевые настройки</a> | \r\n"
"<a href=\"gsm.html\">GSM/GPRS</a> | \r\n"
"<a href=\"csd.html\">CSD</a> | \r\n"
"<a href=\"system.html\">Система</a> | \r\n"
"<a href=\"logout.html\">Выход</a>\r\n"
"<HR>\r\n"
"<h3>Счётчики электроэнергии:</h3>\r\n"
"A+ {{{{{{{{{{{ кВт*ч | A- {{{{{{{{{{{ кВт*ч | R+ {{{{{{{{{{{ кВАр*ч | R- {{{{{{{{{{{ кВАр*ч\r\n"
"<h3>Уровень сигнала:</h3>\r\n"
" {{ (0-31)</BR>\r\n"
"<h3>Наработка :</h3>\r\n"
"{{{{{{{{{{{ сек\r\n"
"</BR>\r\n"
"</BODY>\r\n"
"</HTML>\r\n"
};





WebPage Web_index = { 
	name_index, 
	index_IField,	//index_InField,
	0,
	NULL, 
	sizeof(index_page),
	index_page,
	0
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		emeter.html
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const prog_char name_emeter[] = "emeter.html";

	prog_char Space_Str[]		= "        ";
	prog_char Selected_Str[]	= "selected";

	prog_str OFF_List[] = {	Selected_Str, Space_Str};
	prog_str ON_List[] = {	Space_Str, Selected_Str};

	prog_str EM_NONE_List[] = {	Selected_Str, Space_Str, Space_Str, Space_Str, Space_Str};
	prog_str LANDIS_GYR_ZMR110_List[] = {	Space_Str, Selected_Str, Space_Str, Space_Str, Space_Str};
	prog_str ELVIN_List[] = {	Space_Str, Space_Str, Selected_Str, Space_Str, Space_Str};
	prog_str MERCURY_230_List[] = {	Space_Str, Space_Str, Space_Str, Selected_Str, Space_Str};
	prog_str ELVIN_MODEM_List[] = {	Space_Str, Space_Str, Space_Str, Space_Str, Selected_Str};

	prog_str BR1200_List[] = {	Selected_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str};
	prog_str BR2400_List[] = {	Space_Str, Selected_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str};
	prog_str BR4800_List[] = {	Space_Str, Space_Str, Selected_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str};
	prog_str BR9600_List[] = {	Space_Str, Space_Str, Space_Str, Selected_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str};
	prog_str BR14400_List[] = {	Space_Str, Space_Str, Space_Str, Space_Str, Selected_Str, Space_Str, Space_Str, Space_Str, Space_Str};
	prog_str BR19200_List[] = {	Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Selected_Str, Space_Str, Space_Str, Space_Str};
	prog_str BR28800_List[] = {	Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Selected_Str, Space_Str, Space_Str};
	prog_str BR34800_List[] = {	Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Selected_Str, Space_Str};
	prog_str BR57600_List[] = {	Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Space_Str, Selected_Str};

	prog_str BIT5_List[] = { Selected_Str, Space_Str, Space_Str, Space_Str};
	prog_str BIT6_List[] = { Space_Str, Selected_Str, Space_Str, Space_Str};
	prog_str BIT7_List[] = { Space_Str, Space_Str, Selected_Str, Space_Str};
	prog_str BIT8_List[] = { Space_Str, Space_Str, Space_Str, Selected_Str};

	prog_str PAR_N_List[] = { Selected_Str, Space_Str, Space_Str};
	prog_str PAR_E_List[] = { Space_Str, Selected_Str, Space_Str};
	prog_str PAR_O_List[] = { Space_Str, Space_Str, Selected_Str};

	prog_str STB_1_List[] = { Selected_Str, Space_Str};
	prog_str STB_2_List[] = { Space_Str, Selected_Str};
	
 InField emeter_IField[] = {
	{EE_Text, &Title, NULL, sizeof(Title)},
	{EE_Text, &Title, NULL, sizeof(Title)},
	{Word, &ICCID1, NULL,  0},
	{z_Longint, &ICCID2, NULL,  0},

	{Longint, &E_Meter_RAM.Ap, NULL, 3},
	{Longint, &E_Meter_RAM.Am, NULL, 3},
	{Longint, &E_Meter_RAM.Rp, NULL, 3},
	{Longint, &E_Meter_RAM.Rm, NULL, 3},

	{EE_Enum, &EMeterType, EM_NONE_List,  0},
	{EE_Enum, &EMeterType, LANDIS_GYR_ZMR110_List,  0},
	{EE_Enum, &EMeterType, ELVIN_List,  0},
	{EE_Enum, &EMeterType, MERCURY_230_List,  0},
	{EE_Enum, &EMeterType, ELVIN_MODEM_List,  0},

	{Enum, &EM_UART_Rate, BR1200_List,  0},
	{Enum, &EM_UART_Rate, BR2400_List,  0},
	{Enum, &EM_UART_Rate, BR4800_List,  0},
	{Enum, &EM_UART_Rate, BR9600_List,  0},
	{Enum, &EM_UART_Rate, BR14400_List,  0},
	{Enum, &EM_UART_Rate, BR19200_List,  0},
	{Enum, &EM_UART_Rate, BR28800_List,  0},
	{Enum, &EM_UART_Rate, BR34800_List,  0},
	{Enum, &EM_UART_Rate, BR57600_List,  0},

	{Enum, &EM_UART_BitQt, BIT5_List,  0},
	{Enum, &EM_UART_BitQt, BIT6_List,  0},
	{Enum, &EM_UART_BitQt, BIT7_List,  0},
	{Enum, &EM_UART_BitQt, BIT8_List,  0},
	
	{Enum, &EM_UART_Parity, PAR_N_List,  0},
	{Enum, &EM_UART_Parity, PAR_E_List,  0},
	{Enum, &EM_UART_Parity, PAR_O_List,  0},

	{Enum, &EM_UART_StopBits, STB_1_List,  0},
	{Enum, &EM_UART_StopBits, STB_2_List,  0},

	{EE_Enum, &AutoSend, OFF_List,  0},
	{EE_Enum, &AutoSend, ON_List,  0},
	{EE_Word, &CurrDataPeriod, NULL,  0},

};

const prog_char emeter_page[] = {
"<HTML>\r\n"
"<HEAD>\r\n"
"<TITLE>{{{{{{{{{{{{{{{{{{{{</TITLE>\r\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">\r\n"
"</HEAD>\r\n"
"<BODY>\r\n"
"\r\n"
"<H2>EMReader. {{{{{{{{{{{{{{{{{{{{</H2>\r\n"
"ID: {{{{ {{{{{{{{{ \tSW: "SW_VERSION_Str"</BR>\r\n"
"<HR>\r\n"
"<a href=\"index.html\">Главная</a> | \r\n"
"<a href=\"emeter.html\">Настройки счетчика</a> | \r\n"
"<a href=\"network.html\">Сетевые настройки</a> | \r\n"
"<a href=\"gsm.html\">GSM/GPRS</a> | \r\n"
"<a href=\"csd.html\">CSD</a> | \r\n"
"<a href=\"system.html\">Система</a> | \r\n"
"<a href=\"logout.html\">Выход</a>\r\n"
"<HR>\r\n"
"<h3>Счётчики электроэнергии:</h3>\r\n"
"A+ {{{{{{{{{{{ кВт*ч | A- {{{{{{{{{{{ кВт*ч | R+ {{{{{{{{{{{ кВАр*ч | R- {{{{{{{{{{{ кВАр*ч\r\n"
"</BR>\r\n"
"<h3>Тип электросчётчика:</h3>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\t<SELECT NAME=\"EMeterType\"> \r\n"
"<OPTION VALUE=0 {{{{{{{{>Нет \r\n"
"<OPTION VALUE=1 {{{{{{{{>Landis&Gyr\r\n"
"<OPTION VALUE=2 {{{{{{{{>ЭЛВИН\r\n"
"<OPTION VALUE=3 {{{{{{{{>Меркурий230\r\n"
"<OPTION VALUE=4 {{{{{{{{>ЭЛВИН модем\r\n"
"\t</SELECT> \r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"\r\n"
"<h3>Параметры RS232/RS485:</h3>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\r\n"
"\tСкорость:\r\n"
"\t<SELECT NAME=\"Rate\"> \r\n"
"\t<OPTION VALUE=0 {{{{{{{{>1200bps\r\n"
"\t<OPTION VALUE=1 {{{{{{{{>2400bps\r\n"
"\t<OPTION VALUE=2 {{{{{{{{>4800bps\r\n"
"\t<OPTION VALUE=3 {{{{{{{{>9600bps\r\n"
"\t<OPTION VALUE=4 {{{{{{{{>14400bps\r\n"
"\t<OPTION VALUE=5 {{{{{{{{>19200bps\r\n"
"\t<OPTION VALUE=6 {{{{{{{{>28800bps\r\n"
"\t<OPTION VALUE=7 {{{{{{{{>38400bps\r\n"
"\t<OPTION VALUE=8 {{{{{{{{>57600bps\r\n"
"\t</SELECT>\r\n"
"\r\n"
"\tКоличество бит:\r\n"
"\t<SELECT NAME=\"BitQt\"> \r\n"
"\t<OPTION VALUE=0 {{{{{{{{>5\r\n"
"\t<OPTION VALUE=1 {{{{{{{{>6\r\n"
"\t<OPTION VALUE=2 {{{{{{{{>7\r\n"
"\t<OPTION VALUE=3 {{{{{{{{>8\r\n"
"\t</SELECT> \r\n"
"\r\n"
"\tЧётность:\r\n"
"\t<SELECT NAME=\"Parity\"> \r\n"
"\t<OPTION VALUE=0 {{{{{{{{>нет\r\n"
"\t<OPTION VALUE=1 {{{{{{{{>четный\r\n"
"\t<OPTION VALUE=2 {{{{{{{{>нечётный\r\n"
"\t</SELECT> \r\n"
"\r\n"
"\tСтоп-биты:\r\n"
"\t<SELECT NAME=\"StopBits\"> \r\n"
"\t<OPTION VALUE=0 {{{{{{{{>1\r\n"
"\t<OPTION VALUE=1 {{{{{{{{>2\r\n"
"\t</SELECT> \r\n"
"\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<h3>Опрос счётчика:</h3></BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\t<SELECT NAME=\"AutoSend\"> \r\n"
"\t<OPTION VALUE=0 {{{{{{{{>Нет \r\n"
"\t<OPTION VALUE=1 {{{{{{{{>Да\r\n"
"\t</SELECT> \r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
" Период:\r\n"
"\t<input type=\'text\' name=\'CurrDataPeriod\' size=\'5\' value=\'{{{{{\'>\r\n"
"\tсек\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"</BODY>\r\n"
"</HTML>\r\n"
};

const prog_char var_EMeterType[] 	= "EMeterType";
OutField Web_EMeterType[] = { {EE_Byte, &EMeterType, 	0, 	4, 	ChangeUARTParam, 	NULL, 0} };

const prog_char var_EM_UART_Rate[] 	= "Rate";
OutField Web_EM_UART_Rate[] = { {Byte, &EM_UART_Rate, 	0, 	8, 	NULL, 	NULL, 0} };

const prog_char var_EM_UART_BitQt[] 	= "BitQt";
OutField Web_EM_UART_BitQt[] = { {Byte, &EM_UART_BitQt, 	0, 	3, 	NULL, 	NULL, 0} };

const prog_char var_EM_UART_Parity[] 	= "Parity";
OutField Web_EM_UART_Parity[] = { {Byte, &EM_UART_Parity, 	0, 	2, 	NULL, 	NULL, 0} };

const prog_char var_EM_UART_StopBits[] 	= "StopBits";
OutField Web_EM_UART_StopBits[] = { {Byte, &EM_UART_StopBits, 	0, 	1, 	ApplyUARTParam, 	NULL, 0} };

const prog_char var_AutoSend[] 	= "AutoSend";
OutField Web_AutoSend[] 		= { {EE_Byte, &AutoSend, 0, 1, NULL, NULL, 0} };

const prog_char var_CurrDataPeriod[] 	= "CurrDataPeriod";
OutField Web_CurrDataPeriod[] = { {EE_Word, &CurrDataPeriod, 	60, 	65535, 	NULL, 	NULL, 0} };

WebOutField emeter_OField[] = {
	{var_EMeterType, Web_EMeterType},
	{var_EM_UART_Rate, Web_EM_UART_Rate},
	{var_EM_UART_BitQt, Web_EM_UART_BitQt},
	{var_EM_UART_Parity, Web_EM_UART_Parity},
	{var_EM_UART_StopBits, Web_EM_UART_StopBits},
	{var_CurrDataPeriod, Web_CurrDataPeriod},
	{var_AutoSend, Web_AutoSend}
};

WebPage Web_emeter = { 
	name_emeter, 
	emeter_IField,	//index_InField,
	sizeof(emeter_OField),
	emeter_OField, 
	sizeof(emeter_page),
	emeter_page,
	2
};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		favicon.ico
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const prog_char name_favicon[] = "favicon.ico";

const prog_char favicon[1406]=
{
	0x00,0x00,0x01,0x00,0x01,0x00,0x10,0x10,0x00,0x00,0x01,
	0x00,0x08,0x00,0x68,0x05,0x00,0x00,0x16,0x00,0x00,0x00,
	0x28,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x20,0x00,0x00,
	0x00,0x01,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x27,0x1F,0xED,0x00,
	0xA4,0xA1,0xF8,0x00,0x59,0x53,0xF1,0x00,0x75,0x70,0xF4,
	0x00,0x2D,0x25,0xEE,0x00,0xC7,0xC5,0xFA,0x00,0x39,0x32,
	0xEF,0x00,0xF0,0xEF,0xFE,0x00,0x8B,0x87,0xF5,0x00,0xA7,
	0xA4,0xF8,0x00,0x3C,0x35,0xEF,0x00,0xFF,0xFF,0xFF,0x00,
	0xC3,0xC1,0xFA,0x00,0x33,0x2B,0xEE,0x00,0x68,0x62,0xF3,
	0x00,0xF6,0xF5,0xFE,0x00,0xAD,0xAA,0xF8,0x00,0xB9,0xB7,
	0xF9,0x00,0x2C,0x24,0xEE,0x00,0x29,0x21,0xED,0x00,0x3B,
	0x34,0xEF,0x00,0xC2,0xC0,0xFA,0x00,0x25,0x1D,0xED,0x00,
	0xDB,0xDA,0xFC,0x00,0x32,0x2A,0xEE,0x00,0xE8,0xE7,0xFD,
	0x00,0x93,0x8F,0xF6,0x00,0x80,0x7B,0xF5,0x00,0x2B,0x23,
	0xEE,0x00,0xE1,0xE0,0xFD,0x00,0x7C,0x78,0xF4,0x00,0x34,
	0x2D,0xEE,0x00,0xA2,0x9F,0xF7,0x00,0xD7,0xD6,0xFC,0x00,
	0x9B,0x98,0xF7,0x00,0x24,0x1C,0xED,0x00,0xB8,0xB5,0xF9,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x0B,0x0B,
	0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,
	0x0B,0x0B,0x0B,0x00,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x0A,0x0B,0x0B,0x23,0x23,0x23,
	0x23,0x1C,0x1E,0x09,0x01,0x0E,0x16,0x23,0x23,0x23,0x18,
	0x0B,0x0B,0x23,0x23,0x23,0x0D,0x21,0x0B,0x0B,0x0B,0x0B,
	0x24,0x16,0x23,0x23,0x18,0x0B,0x0B,0x23,0x23,0x23,0x20,
	0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x03,0x23,0x23,0x18,0x0B,
	0x0B,0x23,0x23,0x16,0x07,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,
	0x0C,0x23,0x23,0x18,0x0B,0x0B,0x23,0x23,0x18,0x0B,0x0B,
	0x0B,0x0B,0x0B,0x0B,0x0B,0x1D,0x23,0x23,0x18,0x0B,0x0B,
	0x23,0x23,0x04,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x17,
	0x23,0x23,0x18,0x0B,0x0B,0x23,0x23,0x23,0x19,0x0B,0x0B,
	0x0B,0x0B,0x0B,0x0B,0x11,0x23,0x23,0x18,0x0B,0x0B,0x23,
	0x23,0x23,0x10,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x1B,0x23,
	0x23,0x18,0x0B,0x0B,0x23,0x23,0x23,0x02,0x0B,0x0B,0x0B,
	0x0B,0x0B,0x0F,0x1F,0x23,0x23,0x18,0x0B,0x0B,0x23,0x23,
	0x23,0x23,0x15,0x0B,0x0B,0x0B,0x0B,0x1A,0x23,0x23,0x23,
	0x18,0x0B,0x0B,0x23,0x23,0x23,0x23,0x14,0x19,0x0B,0x0B,
	0x05,0x13,0x23,0x23,0x23,0x18,0x0B,0x0B,0x23,0x23,0x23,
	0x23,0x23,0x06,0x22,0x08,0x12,0x23,0x23,0x23,0x23,0x18,
	0x0B,0x0B,0x00,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x0A,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,
	0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

WebPage Web_favicon = { 
	name_favicon, 
	NULL,	
	0,
	NULL,
	sizeof(favicon),
	favicon,
	0
};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		access_error.html
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const prog_char name_access_error[] = "access_error.html";


const prog_char name_access_error_page[] = {
"<HTML>\r\n"
"<HEAD>\r\n"
"<TITLE>Ошибка доступа</TITLE>\r\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">\r\n"
"</HEAD>\r\n"
"<center>\r\n"
"<head>\r\n"
"<h3>Ваш уровень доступа не позволяет просматривать эту страницу</h3>\r\n"
"<a href=\"index.html\">Назад на главную страницу</a>\r\n"
"</html>\r\n"
};

WebPage Web_name_access_error = { 
	name_access_error, 
	NULL,	//index_InField,
	0,
	NULL, 
	sizeof(name_access_error_page),
	name_access_error_page,
	0
};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		network.html
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const prog_char name_network[] = "network.html";

InField network_IField[] = {
	{EE_Text, &Title, NULL, sizeof(Title)},
	{EE_Text, &Title, NULL, sizeof(Title)},
	{Word, &ICCID1, NULL,  0},
	{z_Longint, &ICCID2, NULL,  0},
	{EE_Word, &TCP_ListenPort, NULL, 0},
	{EE_Word, &TCP_CONNECT_timeout, NULL, 2},
	{EE_Longint, &WebMaxSessionTime, NULL, 2},
	{EE_Byte, &TCP_ClientIP[0].IP1, NULL, 0},
	{EE_Byte, &TCP_ClientIP[0].IP2, NULL, 0},
	{EE_Byte, &TCP_ClientIP[0].IP3, NULL, 0},
	{EE_Byte, &TCP_ClientIP[0].IP4, NULL, 0},
	{EE_Byte, &TCP_ClientIP[1].IP1, NULL, 0},
	{EE_Byte, &TCP_ClientIP[1].IP2, NULL, 0},
	{EE_Byte, &TCP_ClientIP[1].IP3, NULL, 0},
	{EE_Byte, &TCP_ClientIP[1].IP4, NULL, 0},
	{EE_Byte, &TCP_ClientIP[2].IP1, NULL, 0},
	{EE_Byte, &TCP_ClientIP[2].IP2, NULL, 0},
	{EE_Byte, &TCP_ClientIP[2].IP3, NULL, 0},
	{EE_Byte, &TCP_ClientIP[2].IP4, NULL, 0},
	{EE_Byte, &TCP_ClientIP[3].IP1, NULL, 0},
	{EE_Byte, &TCP_ClientIP[3].IP2, NULL, 0},
	{EE_Byte, &TCP_ClientIP[3].IP3, NULL, 0},
	{EE_Byte, &TCP_ClientIP[3].IP4, NULL, 0},
	{EE_Byte, &UDP_ServerIP[0].IP1, NULL, 0},
	{EE_Byte, &UDP_ServerIP[0].IP2, NULL, 0},
	{EE_Byte, &UDP_ServerIP[0].IP3, NULL, 0},
	{EE_Byte, &UDP_ServerIP[0].IP4, NULL, 0},
	{EE_Word, &UDP_ServerPort[0], NULL, 0},
	{EE_Byte, &UDP_ServerIP[1].IP1, NULL, 0},
	{EE_Byte, &UDP_ServerIP[1].IP2, NULL, 0},
	{EE_Byte, &UDP_ServerIP[1].IP3, NULL, 0},
	{EE_Byte, &UDP_ServerIP[1].IP4, NULL, 0},
	{EE_Word, &UDP_ServerPort[1], NULL, 0},
	{EE_Byte, &UDP_ServerIP[2].IP1, NULL, 0},
	{EE_Byte, &UDP_ServerIP[2].IP2, NULL, 0},
	{EE_Byte, &UDP_ServerIP[2].IP3, NULL, 0},
	{EE_Byte, &UDP_ServerIP[2].IP4, NULL, 0},
	{EE_Word, &UDP_ServerPort[2], NULL, 0},
	{EE_Byte, &UDP_ServerIP[3].IP1, NULL, 0},
	{EE_Byte, &UDP_ServerIP[3].IP2, NULL, 0},
	{EE_Byte, &UDP_ServerIP[3].IP3, NULL, 0},
	{EE_Byte, &UDP_ServerIP[3].IP4, NULL, 0},
	{EE_Word, &UDP_ServerPort[3], NULL, 0},
};

const prog_char network_page[] = {
"<HTML>\r\n"
"<HEAD>\r\n"
"<TITLE>{{{{{{{{{{{{{{{{{{{{</TITLE>\r\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">\r\n"
"</HEAD>\r\n"
"<BODY>\r\n"
"\r\n"
"<H2>EMReader. {{{{{{{{{{{{{{{{{{{{</H2>\r\n"
"ID: {{{{ {{{{{{{{{ \tSW: "SW_VERSION_Str"</BR>\r\n"
"<HR>\r\n"
"<a href=\"index.html\">Главная</a> | \r\n"
"<a href=\"emeter.html\">Настройки счетчика</a> | \r\n"
"<a href=\"network.html\">Сетевые настройки</a> | \r\n"
"<a href=\"gsm.html\">GSM/GPRS</a> | \r\n"
"<a href=\"csd.html\">CSD</a> | \r\n"
"<a href=\"system.html\">Система</a> | \r\n"
"<a href=\"logout.html\">Выход</a>\r\n"
"<HR>\r\n"
"\r\n"
"<h3>TCP/IP Server</h3>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tListen port:\r\n"
"\t<input type=\'text\' name=\'TCP_ListenPort\' size=\'5\' value=\'{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tTCP/IP connection timeout:\r\n"
"\t<input type=\'text\' name=\'TCP_CONNECT_timeout\' size=\'7\' value=\'{{{{{{\'>sec\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tSession timeout:\r\n"
"\t<input type=\'text\' name=\'WebMaxSessionTime\' size=\'7\' value=\'{{{{{{{{{{\'>sec\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"\r\n"
"<h3>IP Firewall</h3>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tIP1:\r\n"
"\t<input type=\'text\' name=\'Cli0_IP1\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'Cli0_IP2\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'Cli0_IP3\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'Cli0_IP4\' size=\'3\' value=\'{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tIP2:\r\n"
"\t<input type=\'text\' name=\'Cli1_IP1\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'Cli1_IP2\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'Cli1_IP3\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'Cli1_IP4\' size=\'3\' value=\'{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tIP3:\r\n"
"\t<input type=\'text\' name=\'Cli2_IP1\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'Cli2_IP2\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'Cli2_IP3\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'Cli2_IP4\' size=\'3\' value=\'{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tIP4:\r\n"
"\t<input type=\'text\' name=\'Cli3_IP1\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'Cli3_IP2\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'Cli3_IP3\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'Cli3_IP4\' size=\'3\' value=\'{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<FORM action=\'/\' method=\'GET\'>\r\n"
"\t<input type=\'submit\' name=\'ResetGSM\' value=\'ResetGSM\'>\r\n"
"</FORM>\r\n"
"</BR>\r\n"
"\r\n"
"\r\n"
"<h3>Отсылка текущих данных:</h3>\r\n"
"</BR>\r\n"
"\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"UDP Server1 IP:\r\n"
"\t<input type=\'text\' name=\'UDP01\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'UDP02\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'UDP03\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'UDP04\' size=\'3\' value=\'{{{\'>\r\n"
"Port:\r\n"
"\t<input type=\'text\' name=\'UDP0_Port\' size=\'5\' value=\'{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"UDP Server2 IP:\r\n"
"\t<input type=\'text\' name=\'UDP11\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'UDP12\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'UDP13\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'UDP14\' size=\'3\' value=\'{{{\'>\r\n"
"Port:\r\n"
"\t<input type=\'text\' name=\'UDP1_Port\' size=\'5\' value=\'{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"UDP Server3 IP:\r\n"
"\t<input type=\'text\' name=\'UDP21\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'UDP22\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'UDP23\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'UDP24\' size=\'3\' value=\'{{{\'>\r\n"
"Port:\r\n"
"\t<input type=\'text\' name=\'UDP2_Port\' size=\'5\' value=\'{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"UDP Server4 IP:\r\n"
"\t<input type=\'text\' name=\'UDP31\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'UDP32\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'UDP33\' size=\'3\' value=\'{{{\'>.\r\n"
"\t<input type=\'text\' name=\'UDP34\' size=\'3\' value=\'{{{\'>\r\n"
"Port:\r\n"
"\t<input type=\'text\' name=\'UDP3_Port\' size=\'5\' value=\'{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"\r\n"
" </BODY>\r\n"
"</HTML>\r\n"
};

const prog_char var_TCP_ListenPort[] = "TCP_ListenPort";
OutField Web_TCP_ListenPort[]		= { {EE_Word, &TCP_ListenPort, 0, 65535, Modbus_ReadHoldReg, NULL, 0} };
const prog_char var_TCP_CONNECT_timeout[] = "TCP_CONNECT_timeout";
OutField Web_TCP_CONNECT_timeout[]		= { {EE_Word, &TCP_CONNECT_timeout, 0, 65535, Modbus_ReadHoldReg, NULL, 2} };
const prog_char var_WebMaxSessionTime[] = "WebMaxSessionTime";
OutField Web_WebMaxSessionTime[]		= { {EE_Longint, &WebMaxSessionTime, 0, 999999999, NULL, NULL, 2} };
const prog_char var_Cli0_IP1[] 	= "Cli0_IP1";
OutField Web_Cli0_IP1[] = { {EE_Byte, &TCP_ClientIP[0].IP1, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli0_IP2[] 	= "Cli0_IP2";
OutField Web_Cli0_IP2[] = { {EE_Byte, &TCP_ClientIP[0].IP2, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli0_IP3[] 	= "Cli0_IP3";
OutField Web_Cli0_IP3[] = { {EE_Byte, &TCP_ClientIP[0].IP3, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli0_IP4[] 	= "Cli0_IP4";
OutField Web_Cli0_IP4[] = { {EE_Byte, &TCP_ClientIP[0].IP4, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli1_IP1[] 	= "Cli1_IP1";
OutField Web_Cli1_IP1[] = { {EE_Byte, &TCP_ClientIP[1].IP1, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli1_IP2[] 	= "Cli1_IP2";
OutField Web_Cli1_IP2[] = { {EE_Byte, &TCP_ClientIP[1].IP2, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli1_IP3[] 	= "Cli1_IP3";
OutField Web_Cli1_IP3[] = { {EE_Byte, &TCP_ClientIP[1].IP3, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli1_IP4[] 	= "Cli1_IP4";
OutField Web_Cli1_IP4[] = { {EE_Byte, &TCP_ClientIP[1].IP4, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli2_IP1[] 	= "Cli2_IP1";
OutField Web_Cli2_IP1[] = { {EE_Byte, &TCP_ClientIP[2].IP1, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli2_IP2[] 	= "Cli2_IP2";
OutField Web_Cli2_IP2[] = { {EE_Byte, &TCP_ClientIP[2].IP2, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli2_IP3[] 	= "Cli2_IP3";
OutField Web_Cli2_IP3[] = { {EE_Byte, &TCP_ClientIP[2].IP3, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli2_IP4[] 	= "Cli2_IP4";
OutField Web_Cli2_IP4[] = { {EE_Byte, &TCP_ClientIP[2].IP4, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli3_IP1[] 	= "Cli3_IP1";
OutField Web_Cli3_IP1[] = { {EE_Byte, &TCP_ClientIP[3].IP1, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli3_IP2[] 	= "Cli3_IP2";
OutField Web_Cli3_IP2[] = { {EE_Byte, &TCP_ClientIP[3].IP2, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli3_IP3[] 	= "Cli3_IP3";
OutField Web_Cli3_IP3[] = { {EE_Byte, &TCP_ClientIP[3].IP3, 0, 255, NULL, NULL, 0} };
const prog_char var_Cli3_IP4[] 	= "Cli3_IP4";
OutField Web_Cli3_IP4[] = { {EE_Byte, &TCP_ClientIP[3].IP4, 0, 255, NULL, NULL, 0} };


const prog_char var_UDP0_IP1[] 	= "UDP01";
OutField Web_UDP0_IP1[] = { {EE_Byte, &UDP_ServerIP[0].IP1, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP0_IP2[] 	= "UDP02";
OutField Web_UDP0_IP2[] = { {EE_Byte, &UDP_ServerIP[0].IP2, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP0_IP3[] 	= "UDP03";
OutField Web_UDP0_IP3[] = { {EE_Byte, &UDP_ServerIP[0].IP3, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP0_IP4[] 	= "UDP04";
OutField Web_UDP0_IP4[] = { {EE_Byte, &UDP_ServerIP[0].IP4, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP1_IP1[] 	= "UDP11";
OutField Web_UDP1_IP1[] = { {EE_Byte, &UDP_ServerIP[1].IP1, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP1_IP2[] 	= "UDP12";
OutField Web_UDP1_IP2[] = { {EE_Byte, &UDP_ServerIP[1].IP2, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP1_IP3[] 	= "UDP13";
OutField Web_UDP1_IP3[] = { {EE_Byte, &UDP_ServerIP[1].IP3, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP1_IP4[] 	= "UDP14";
OutField Web_UDP1_IP4[] = { {EE_Byte, &UDP_ServerIP[1].IP4, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP2_IP1[] 	= "UDP21";
OutField Web_UDP2_IP1[] = { {EE_Byte, &UDP_ServerIP[2].IP1, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP2_IP2[] 	= "UDP22";
OutField Web_UDP2_IP2[] = { {EE_Byte, &UDP_ServerIP[2].IP2, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP2_IP3[] 	= "UDP23";
OutField Web_UDP2_IP3[] = { {EE_Byte, &UDP_ServerIP[2].IP3, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP2_IP4[] 	= "UDP24";
OutField Web_UDP2_IP4[] = { {EE_Byte, &UDP_ServerIP[2].IP4, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP3_IP1[] 	= "UDP31";
OutField Web_UDP3_IP1[] = { {EE_Byte, &UDP_ServerIP[3].IP1, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP3_IP2[] 	= "UDP32";
OutField Web_UDP3_IP2[] = { {EE_Byte, &UDP_ServerIP[3].IP2, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP3_IP3[] 	= "UDP33";
OutField Web_UDP3_IP3[] = { {EE_Byte, &UDP_ServerIP[3].IP3, 0, 255, NULL, NULL, 0} };
const prog_char var_UDP3_IP4[] 	= "UDP34";
OutField Web_UDP3_IP4[] = { {EE_Byte, &UDP_ServerIP[3].IP4, 0, 255, NULL, NULL, 0} };

const prog_char var_UDP0_Port[] 	= "UDP0_Port";
OutField Web_UDP0_Port[] = { {EE_Word, &UDP_ServerPort[0], 0, 65535, NULL, NULL, 0} };
const prog_char var_UDP1_Port[] 	= "UDP1_Port";
OutField Web_UDP1_Port[] = { {EE_Word, &UDP_ServerPort[1], 0, 65535, NULL, NULL, 0} };
const prog_char var_UDP2_Port[] 	= "UDP2_Port";
OutField Web_UDP2_Port[] = { {EE_Word, &UDP_ServerPort[2], 0, 65535, NULL, NULL, 0} };
const prog_char var_UDP3_Port[] 	= "UDP3_Port";
OutField Web_UDP3_Port[] = { {EE_Word, &UDP_ServerPort[3], 0, 65535, NULL, NULL, 0} };

const prog_char var_ResetGSM[] 		= "ResetGSM";
OutField Web_ResetGSM[] = { {Byte, &TempWebAction, 0, 255, ResetGSMState, NULL, 0} };

WebOutField network_OField[] = {
	{var_TCP_ListenPort, Web_TCP_ListenPort},
	{var_TCP_CONNECT_timeout, Web_TCP_CONNECT_timeout},
	{var_WebMaxSessionTime, Web_WebMaxSessionTime},
	{var_ResetGSM, Web_ResetGSM},
	{var_Cli0_IP1, Web_Cli0_IP1},
	{var_Cli0_IP2, Web_Cli0_IP2},
	{var_Cli0_IP3, Web_Cli0_IP3},
	{var_Cli0_IP4, Web_Cli0_IP4},
	{var_Cli1_IP1, Web_Cli1_IP1},
	{var_Cli1_IP2, Web_Cli1_IP2},
	{var_Cli1_IP3, Web_Cli1_IP3},
	{var_Cli1_IP4, Web_Cli1_IP4},
	{var_Cli2_IP1, Web_Cli2_IP1},
	{var_Cli2_IP2, Web_Cli2_IP2},
	{var_Cli2_IP3, Web_Cli2_IP3},
	{var_Cli2_IP4, Web_Cli2_IP4},
	{var_Cli3_IP1, Web_Cli3_IP1},
	{var_Cli3_IP2, Web_Cli3_IP2},
	{var_Cli3_IP3, Web_Cli3_IP3},
	{var_Cli3_IP4, Web_Cli3_IP4},
	{var_UDP0_IP1, Web_UDP0_IP1},
	{var_UDP0_IP2, Web_UDP0_IP2},
	{var_UDP0_IP3, Web_UDP0_IP3},
	{var_UDP0_IP4, Web_UDP0_IP4},
	{var_UDP1_IP1, Web_UDP1_IP1},
	{var_UDP1_IP2, Web_UDP1_IP2},
	{var_UDP1_IP3, Web_UDP1_IP3},
	{var_UDP1_IP4, Web_UDP1_IP4},
	{var_UDP2_IP1, Web_UDP2_IP1},
	{var_UDP2_IP2, Web_UDP2_IP2},
	{var_UDP2_IP3, Web_UDP2_IP3},
	{var_UDP2_IP4, Web_UDP2_IP4},
	{var_UDP3_IP1, Web_UDP3_IP1},
	{var_UDP3_IP2, Web_UDP3_IP2},
	{var_UDP3_IP3, Web_UDP3_IP3},
	{var_UDP3_IP4, Web_UDP3_IP4},
	{var_UDP0_Port, Web_UDP0_Port},
	{var_UDP1_Port, Web_UDP1_Port},
	{var_UDP2_Port, Web_UDP2_Port},
	{var_UDP3_Port, Web_UDP3_Port},
};

WebPage Web_network = { 
	name_network, 
	network_IField,	//index_InField, 
	sizeof(network_OField),
	network_OField,
	sizeof(network_page),
	network_page,
	4
};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		gsm.html
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const prog_char name_gsm[] = "gsm.html";

InField gsm_IField[] = {
	{EE_Text, &Title, NULL, sizeof(Title)},
	{EE_Text, &Title, NULL, sizeof(Title)},
	{Word, &ICCID1, NULL,  0},
	{z_Longint, &ICCID2, NULL,  0},
	{Byte, &GSM_RSSI, NULL, 0},
	{EE_Text, &GPRS_apn, NULL, sizeof(GPRS_apn) },
	{EE_Text, &GPRS_apn_user_name, NULL, sizeof(GPRS_apn_user_name) },
	{EE_Text, &GPRS_apn_psw, NULL, sizeof(GPRS_apn_psw) },
/*	{EE_Enum, &SMS_On, OFF_List,  0},
	{EE_Enum, &SMS_On, ON_List,  0},
	{EE_Text, SMS_Number, NULL, sizeof(MaxTelephN)},
	{EE_Text, SMS_Number+1, NULL, sizeof(MaxTelephN)},
	{EE_Text, SMS_Number+2, NULL, sizeof(MaxTelephN)},*/
};

const prog_char gsm_page[] = {
"<HTML>\r\n"
"<HEAD>\r\n"
"<TITLE>{{{{{{{{{{{{{{{{{{{{</TITLE>\r\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">\r\n"
"</HEAD>\r\n"
"<BODY>\r\n"
"\r\n"
"<H2>EMReader. {{{{{{{{{{{{{{{{{{{{</H2>\r\n"
"ID: {{{{ {{{{{{{{{ \tSW: "SW_VERSION_Str"</BR>\r\n"
"<HR>\r\n"
"<a href=\"index.html\">Главная</a> | \r\n"
"<a href=\"emeter.html\">Настройки счетчика</a> | \r\n"
"<a href=\"network.html\">Сетевые настройки</a> | \r\n"
"<a href=\"gsm.html\">GSM/GPRS</a> | \r\n"
"<a href=\"csd.html\">CSD</a> | \r\n"
"<a href=\"system.html\">Система</a> | \r\n"
"<a href=\"logout.html\">Выход</a>\r\n"
"<HR>\r\n"
"Уровень сигнала {{ (0-31)</BR>\r\n"
"<h3>GPRS</h3></BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tAPN name: \r\n"
"\t<input type=\'text\' name=\'GPRS_apn\' size=\'29\' value=\'{{{{{{{{{{{{{{{{{{{{{{{{{{{{{\'>\r\n"
//"\t<input type=\'text\' name=\'GPRS_apn2\' size=\'19\' value=\'{{{{{{{{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tUser name: \r\n"
"\t<input type=\'text\' name=\'GPRS_apn_user_name\' size=\'19\' value=\'{{{{{{{{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tPassword: \r\n"
"\t<input type=\'text\' name=\'GPRS_apn_psw\' size=\'19\' value=\'{{{{{{{{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form\r\n"
"</BR>\r\n"
"<FORM action=\'/\' method=\'GET\'>\r\n"
"\t<input type=\'submit\' name=\'ResetGSM\' value=\'ResetGSM\'>\r\n"
"</FORM>\r\n"
"</BR></BR>\r\n"
/*
"<h3>SMS</h3></BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tSMS рассылка:\r\n"
"\t<SELECT NAME=\"SMSOn\"> \r\n"
"\t<OPTION VALUE=0 {{{{{{{{>Нет \r\n"
"\t<OPTION VALUE=1 {{{{{{{{>Да\r\n"
"\t</SELECT> \r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tНомер1: +\r\n"
"\t<input type=\'text\' name=\'SMS_Number1\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tНомер2: + \r\n"
"\t<input type=\'text\' name=\'SMS_Number2\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tНомер3: +\r\n"
"\t<input type=\'text\' name=\'SMS_Number3\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"\r\n"*/
" </BODY>\r\n"
"</HTML>\r\n"
};

const prog_char var_GPRS_apn1[] 	= "GPRS_apn";
OutField Web_GPRS_apn1[] 		= { {EE_Text, &GPRS_apn, DIGIT, ENG_CAPS, Modbus_ReadHoldReg, NULL, sizeof(GPRS_apn)} };
//const prog_char var_GPRS_apn2[] 	= "GPRS_apn2";
//OutField Web_GPRS_apn2[] 		= { {EE_Text, &GPRS_apn2, DIGIT, ENG_CAPS, NULL, NULL, sizeof(GPRS_apn2)} };
const prog_char var_GPRS_apn_user_name[] 	= "GPRS_apn_user_name";
OutField Web_GPRS_apn_user_name[]= { {EE_Text, &GPRS_apn_user_name, DIGIT, ENG_CAPS, Modbus_ReadHoldReg, NULL, sizeof(GPRS_apn_user_name)} };
const prog_char var_GPRS_apn_psw[] 	= "GPRS_apn_psw";
OutField Web_GPRS_apn_psw[]	= { {EE_Text, &GPRS_apn_psw, DIGIT, ENG_CAPS, Modbus_ReadHoldReg, NULL, sizeof(GPRS_apn_psw)} };
const prog_char var_SMS[] 	= "SMSOn";
OutField Web_SMS[] 			= { {EE_Byte, &SMS_On, 0, 1, NULL, NULL, 0} };
const prog_char var_SMS_Number1[] 	= "SMS_Number1";
OutField Web_SMS_Number1[]			= { {EE_Text, &SMS_Number[0][0], DIGIT, DIGIT, 	NULL, 	NULL, MaxTelephN} };
const prog_char var_SMS_Number2[] 	= "SMS_Number2";
OutField Web_SMS_Number2[] 			= { {EE_Text, &SMS_Number[1][0], DIGIT,	DIGIT, 	NULL, 	NULL, MaxTelephN} };
const prog_char var_SMS_Number3[] 	= "SMS_Number3";
OutField Web_SMS_Number3[] 			= { {EE_Text, &SMS_Number[2][0], DIGIT, DIGIT, 	NULL,	NULL, MaxTelephN} };

WebOutField gsm_OField[] = {
	{var_GPRS_apn1, Web_GPRS_apn1},
//	{var_GPRS_apn2, Web_GPRS_apn2},
	{var_GPRS_apn_user_name, Web_GPRS_apn_user_name},
	{var_GPRS_apn_psw, Web_GPRS_apn_psw},
	{var_SMS, Web_SMS},
	{var_SMS_Number1, Web_SMS_Number1},
	{var_SMS_Number2, Web_SMS_Number2},
	{var_SMS_Number3, Web_SMS_Number3},
};

WebPage Web_gsm = { 
	name_gsm, 
	gsm_IField,	//index_InField, 
	sizeof(gsm_OField),
	gsm_OField,
	sizeof(gsm_page),
	gsm_page,
	4
};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		csd.html
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const prog_char name_csd[] = "csd.html";

InField csd_IField[] = {
	{EE_Text, &Title, NULL, sizeof(Title)},
	{EE_Text, &Title, NULL, sizeof(Title)},
	{Word, &ICCID1, NULL,  0},
	{z_Longint, &ICCID2, NULL,  0},
	{EE_Enum, &CLIP_On, OFF_List,  0},
	{EE_Enum, &CLIP_On, ON_List,  0},
	{EE_Text, CSD_AllowedNumbers, NULL, MaxTelephN},
	{EE_Text, CSD_AllowedNumbers+1, NULL, MaxTelephN},
	{EE_Text, CSD_AllowedNumbers+2, NULL, MaxTelephN},
	{EE_Text, CSD_AllowedNumbers+3, NULL, MaxTelephN},
	{EE_Text, CSD_AllowedNumbers+4, NULL, MaxTelephN},
	{EE_Text, CSD_AllowedNumbers+5, NULL, MaxTelephN},
	{EE_Text, CSD_AllowedNumbers+6, NULL, MaxTelephN},
	{EE_Text, CSD_AllowedNumbers+7, NULL, MaxTelephN},
	{EE_Text, CSD_AllowedNumbers+8, NULL, MaxTelephN},
	{EE_Text, CSD_AllowedNumbers+9, NULL, MaxTelephN},
};

const prog_char csd_page[] = {
"<HTML>\r\n"
"<HEAD>\r\n"
"<TITLE>{{{{{{{{{{{{{{{{{{{{</TITLE>\r\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">\r\n"
"</HEAD>\r\n"
"<BODY>\r\n"
"\r\n"
"<H2>EMReader. {{{{{{{{{{{{{{{{{{{{</H2>\r\n"
"ID: {{{{ {{{{{{{{{ \tSW: "SW_VERSION_Str"</BR>\r\n"
"<HR>\r\n"
"<a href=\"index.html\">Главная</a> | \r\n"
"<a href=\"emeter.html\">Настройки счетчика</a> | \r\n"
"<a href=\"network.html\">Сетевые настройки</a> | \r\n"
"<a href=\"gsm.html\">GSM/GPRS</a> | \r\n"
"<a href=\"csd.html\">CSD</a> | \r\n"
"<a href=\"system.html\">Система</a> | \r\n"
"<a href=\"logout.html\">Выход</a>\r\n"
"<HR>\r\n"

"<form action=\'/\' method=\'GET\'>\r\n"
"\tПроверка номера:\r\n"
"\t<SELECT NAME=\"CLIPOn\"> \r\n"
"\t<OPTION VALUE=0 {{{{{{{{>Нет \r\n"
"\t<OPTION VALUE=1 {{{{{{{{>Да\r\n"
"\t</SELECT> \r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"


"<h3>Разрешённые номера</h3></BR>\r\n"

"<form action=\'/\' method=\'GET\'>\r\n"
"\t№1: \r\n"
"\t<input type=\'text\' name=\'CSD0\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"

"<form action=\'/\' method=\'GET\'>\r\n"
"\t№2: \r\n"
"\t<input type=\'text\' name=\'CSD1\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"

"<form action=\'/\' method=\'GET\'>\r\n"
"\t№3: \r\n"
"\t<input type=\'text\' name=\'CSD2\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"

"<form action=\'/\' method=\'GET\'>\r\n"
"\t№4: \r\n"
"\t<input type=\'text\' name=\'CSD3\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"

"<form action=\'/\' method=\'GET\'>\r\n"
"\t№5: \r\n"
"\t<input type=\'text\' name=\'CSD4\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"

"<form action=\'/\' method=\'GET\'>\r\n"
"\t№6: \r\n"
"\t<input type=\'text\' name=\'CSD5\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"

"<form action=\'/\' method=\'GET\'>\r\n"
"\t№7: \r\n"
"\t<input type=\'text\' name=\'CSD6\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"

"<form action=\'/\' method=\'GET\'>\r\n"
"\t№8: \r\n"
"\t<input type=\'text\' name=\'CSD7\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"

"<form action=\'/\' method=\'GET\'>\r\n"
"\t№9: \r\n"
"\t<input type=\'text\' name=\'CSD8\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"

"<form action=\'/\' method=\'GET\'>\r\n"
"\t№10: \r\n"
"\t<input type=\'text\' name=\'CSD9\' size=\'12\' value=\'{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"


"</BR>\r\n"



" </BODY>\r\n"
"</HTML>\r\n"
};

const prog_char var_CLIP[] 			= "CLIPOn";
OutField Web_CLIP[] 				= { {EE_Byte, &CLIP_On, 0, 1, NULL, NULL, 0} };
const prog_char var_CSD_Number0[] 	= "CSD0";
OutField Web_CSD_Number0[]			= { {EE_Text, &CSD_AllowedNumbers[0][0], DIGIT, DIGIT, 	NULL, 	NULL, MaxTelephN} };
const prog_char var_CSD_Number1[] 	= "CSD1";
OutField Web_CSD_Number1[] 			= { {EE_Text, &CSD_AllowedNumbers[1][0], DIGIT,	DIGIT, 	NULL, 	NULL, MaxTelephN} };
const prog_char var_CSD_Number2[] 	= "CSD2";
OutField Web_CSD_Number2[] 			= { {EE_Text, &CSD_AllowedNumbers[2][0], DIGIT,	DIGIT, 	NULL, 	NULL, MaxTelephN} };
const prog_char var_CSD_Number3[] 	= "CSD3";
OutField Web_CSD_Number3[] 			= { {EE_Text, &CSD_AllowedNumbers[3][0], DIGIT,	DIGIT, 	NULL, 	NULL, MaxTelephN} };
const prog_char var_CSD_Number4[] 	= "CSD4";
OutField Web_CSD_Number4[] 			= { {EE_Text, &CSD_AllowedNumbers[4][0], DIGIT,	DIGIT, 	NULL, 	NULL, MaxTelephN} };
const prog_char var_CSD_Number5[] 	= "CSD5";
OutField Web_CSD_Number5[] 			= { {EE_Text, &CSD_AllowedNumbers[5][0], DIGIT,	DIGIT, 	NULL, 	NULL, MaxTelephN} };
const prog_char var_CSD_Number6[] 	= "CSD6";
OutField Web_CSD_Number6[] 			= { {EE_Text, &CSD_AllowedNumbers[6][0], DIGIT,	DIGIT, 	NULL, 	NULL, MaxTelephN} };
const prog_char var_CSD_Number7[] 	= "CSD7";
OutField Web_CSD_Number7[] 			= { {EE_Text, &CSD_AllowedNumbers[7][0], DIGIT,	DIGIT, 	NULL, 	NULL, MaxTelephN} };
const prog_char var_CSD_Number8[] 	= "CSD8";
OutField Web_CSD_Number8[] 			= { {EE_Text, &CSD_AllowedNumbers[8][0], DIGIT,	DIGIT, 	NULL, 	NULL, MaxTelephN} };
const prog_char var_CSD_Number9[] 	= "CSD9";
OutField Web_CSD_Number9[] 			= { {EE_Text, &CSD_AllowedNumbers[9][0], DIGIT,	DIGIT, 	NULL, 	NULL, MaxTelephN} };


WebOutField csd_OField[] = {
	{var_CLIP, Web_CLIP},
	{var_CSD_Number0, Web_CSD_Number0},
	{var_CSD_Number1, Web_CSD_Number1},
	{var_CSD_Number2, Web_CSD_Number2},
	{var_CSD_Number3, Web_CSD_Number3},
	{var_CSD_Number4, Web_CSD_Number4},
	{var_CSD_Number5, Web_CSD_Number5},
	{var_CSD_Number6, Web_CSD_Number6},
	{var_CSD_Number7, Web_CSD_Number7},
	{var_CSD_Number8, Web_CSD_Number8},
	{var_CSD_Number9, Web_CSD_Number9},
};

WebPage Web_csd = { 
	name_csd, 
	csd_IField,	//index_InField, 
	sizeof(csd_OField),
	csd_OField,
	sizeof(csd_page),
	csd_page,
	2
};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		system.html
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const prog_char name_system[] = "system.html";

InField system_IField[] = {
	{EE_Text, &Title, NULL, sizeof(Title)},
	{EE_Text, &Title, NULL, sizeof(Title)},
	{Word, &ICCID1, NULL,  0},
	{z_Longint, &ICCID2, NULL,  0},
	{EE_Text, &Title, NULL, sizeof(Title)},
	{Longint, &LiveTime, NULL,  2},
	{s_Word, &Temperature[0], NULL,  1},
	{s_EE_Word, &Thigh, NULL,  1},
	{s_EE_Word, &Tlow, NULL,  1},
	{Byte, &GSM_RX_FIFOMax_Transp, NULL,  0},
	{Byte, &EM_RX_FIFOMax, NULL,  0},
};


const prog_char system_page[] = {
"<HTML>\r\n"
"<HEAD>\r\n"
"<TITLE>{{{{{{{{{{{{{{{{{{{{</TITLE>\r\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">\r\n"
"</HEAD>\r\n"
"<BODY>\r\n"
"\r\n"
"<H2>EMReader. {{{{{{{{{{{{{{{{{{{{</H2>\r\n"
"ID: {{{{ {{{{{{{{{ \tSW: "SW_VERSION_Str"</BR>\r\n"
"<HR>\r\n"
"<a href=\"index.html\">Главная</a> | \r\n"
"<a href=\"emeter.html\">Настройки счетчика</a> | \r\n"
"<a href=\"network.html\">Сетевые настройки</a> | \r\n"
"<a href=\"gsm.html\">GSM/GPRS</a> | \r\n"
"<a href=\"csd.html\">CSD</a> | \r\n"
"<a href=\"system.html\">Система</a> | \r\n"
"<a href=\"logout.html\">Выход</a>\r\n"
"<HR>\r\n"
"\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tНазвание объекта:\r\n"
"\t<input type=\'text\' name=\'Title\' size=\'40\' value=\'{{{{{{{{{{{{{{{{{{{{\'>\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"\r\n"
"Наработка :{{{{{{{{{{{ сек\r\n"
"<FORM action=\'/\' method=\'GET\'>\r\n"
"\t<input type=\'submit\' name=\'Reboot\' value=\'Reboot\'>\r\n"
"</FORM>\r\n"
"Версия ПО: "SW_VERSION_Str"\r\n"
"<FORM action=\'/\' method=\'GET\'>\r\n"
"\t<input type=\'submit\' name=\'Bootloader\' value=\'Bootloader\' color=\'RED\'>\r\n"
"</FORM>\r\n"
"</BR>\r\n"
"<FORM action=\'/\' method=\'GET\'>\r\n"
"\t<input type=\'submit\' name=\'BLupgrade\' value=\'BLupgrade\'>\r\n"
"</FORM>\r\n"
"</BR>\r\n"
"<H2>Температура</H2>\r\n"
"Температура:{{{{{{{&#176C</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tТвысокая:\r\n"
"\t<input type=\'text\' name=\'Thigh\' size=\'5\' value=\'{{{{{{{\'>&#176C\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"<form action=\'/\' method=\'GET\'>\r\n"
"\tТнизкая:\r\n"
"\t<input type=\'text\' name=\'Tlow\' size=\'5\' value=\'{{{{{{{\'>&#176C\r\n"
"\t<input type=\'submit\' value=\'OK\'>\r\n"
"</form>\r\n"
"</BR>\r\n"
"Буфер GSM->EM: {{{/(max 250)байт</BR>\r\n"
"Буфер EM->GSM: {{{/(max 250)байт</BR>\r\n"
"\r\n"
"</BODY>\r\n"
"</HTML>\r\n"
};

const prog_char var_Title[] 	= "Title";
OutField Web_Title[] = { {EE_Text, &Title, DIGIT, ENG_CAPS, NULL, NULL, sizeof(Title)} };
const prog_char var_Reboot[] 		= "Reboot";
OutField Web_Reboot[] = { {Byte, &TempWebAction, 0, 255, Reboot, NULL, 0} };
const prog_char var_Bootloader[] 		= "Bootloader";
OutField Web_Bootloader[] = { {Byte, &TempWebAction, 0, 255, Start_Bootloader, NULL, 0} };
const prog_char var_BLupgrade[] 		= "BLupgrade";
OutField Web_BLupgrade[] = { {Byte, &TempWebAction, 0, 255, Start_Updater, NULL, 0} };
const prog_char var_Thigh[] 	= "Thigh";
OutField Web_Thigh[] = { {s_EE_Word, &Thigh, -1000, 1500, NULL, NULL, 1} };
const prog_char var_Tlow[] 	= "Tlow";
OutField Web_Tlow[] = { {s_EE_Word, &Tlow, -1000, 1500, NULL, NULL, 1} };

WebOutField system_OField[] = {
	{var_Title, Web_Title},
	{var_Reboot, Web_Reboot},
	{var_Bootloader, Web_Bootloader},
	{var_BLupgrade, Web_BLupgrade},
	{var_Thigh, Web_Thigh},
	{var_Tlow, Web_Tlow},
};

WebPage Web_system = { 
	name_system, 
	system_IField,
	sizeof(system_OField),
	system_OField,
	sizeof(system_page),
	system_page,
	4
};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const WebPage* const WebSite[] PROGMEM = {
	&Web_index,
	&Web_login,
	&Web_favicon,
	&Web_emeter,
	&Web_network,
	&Web_gsm,
	&Web_csd,
	&Web_system,
	&Web_name_access_error
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

