/**************************************************************************************************
File			: multilang

 Compiler		: gcc 4.3.3

 Author			: Svyatoslav Kononenko

 Date			: December 2012

 Hardware		: vmi, vc4, vmd2

 Description	: обеспечивает ввод редактируемых переменных Text,EE_Text,Longint,EE_Longint,s_Longint,s_EE_Longint,z_Longint,z_EE_Longint,FFloat,EE_FFloat,GFloat,EE_GFloat 

				Переменная MultiSymbol обеспечивает изменение символа при повторном нажатии клавиши
				21.10.2013 * Kononenko *  Add *"# in Symbols_P 
				
 TODO			: 
				

**************************************************************************************************/

//static uint8_t MultiSymbol;
//static uint8_t PrevKeyForMultiSymbolSwitch;	//Кода цифровых клавиш 0..9, 255 - есть переход, 254 - нет перехода. Используется для автомтаического сдвига курсора вправо при вводе переменной Text
//static uint8_t TD_MultiSymbolSwitch;
//static uint8_t TextLanguage;

enum TEXT_LANGUAGE {DIGIT, ENG_SMALL, ENG_CAPS, RUS_SMALL, RUS_CAPS, UKR_SMALL, UKR_CAPS};
/*
prog_char DIGIT_Str[] 		= "123";
prog_char ENG_SMALL_Str[] 	= "eng";
prog_char ENG_CAPS_Str[]	= "ENG";
prog_char RUS_SMALL_Str[]	= "rus";
prog_char RUS_CAPS_Str[]	= "RUS";
prog_char UKR_SMALL_Str[]	= "ukr";
prog_char UKR_CAPS_Str[]	= "UKR";
prog_str LanguageStr[] = {DIGIT_Str, ENG_SMALL_Str, ENG_CAPS_Str, RUS_SMALL_Str, RUS_CAPS_Str, UKR_SMALL_Str, UKR_CAPS_Str};
*/
prog_char Symbols_P[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZабвгдеёєжзиіїйклмнопрстуфхцчшщъыьэюяАБВГДЕЁЄЖЗИІЇЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ +-=<>$%&.,?!'@:()\"|*#/";
prog_char OnlyDigits_P[] = "0123456789";
prog_char Digits_P[] = "0123456789e+-.";

