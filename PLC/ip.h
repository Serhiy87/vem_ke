#ifndef __IP_H
#define __IP_H
/* 
 File			: ip.h
 Operation with IP-address
 
 SetIP - set IP address
 IP_compare_Const - compare IP address with constant IP address

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
typedef struct{
	uint8_t IP1;
	uint8_t IP2;
	uint8_t IP3;
	uint8_t IP4;
} IP_Addr;

//~~~~~~~~~~~~~~
void SetIP(IP_Addr* IP, uint8_t IP1, uint8_t IP2, uint8_t IP3, uint8_t IP4){
	IP->IP1 = IP1;
	IP->IP2 = IP2;
	IP->IP3 = IP3;
	IP->IP4 = IP4;
}
uint8_t IP_compare_Const(IP_Addr* IP, uint8_t IP1, uint8_t IP2, uint8_t IP3, uint8_t IP4){
	if((IP->IP1 == IP1) && (IP->IP2 == IP2) && (IP->IP3 == IP3) && (IP->IP4 == IP4)) return 1;
	else return 0;
}
uint8_t IP_E_compare_Const(IP_Addr* IP, uint8_t IP1, uint8_t IP2, uint8_t IP3, uint8_t IP4){
	if((erb(&IP->IP1) == IP1) && (erb(&IP->IP2) == IP2) && (erb(&IP->IP3) == IP3) && (erb(&IP->IP4) == IP4)) return 1;
	else return 0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#endif /* !__IP_H */
