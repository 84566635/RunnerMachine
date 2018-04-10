#ifndef __MAIN_H__
#define __MAIN_H__
#include <STC8F.H>
#include <intrins.h>

#define uchar unsigned char
#define uint unsigned int

//***********************Definction of Pins*****************************
//MFRC500
sbit     MF522_RST  =    P6^0;                   //RC500片选
sbit     MF522_NSS  =    P1^2;					 //对应SDA
sbit     MF522_SI   =    P1^3;
sbit     MF522_SO   =    P1^4;
sbit     MF522_SCK  =    P1^5;

//LED_GREEN
sbit     LED_GREEN  =    P5^5;
//Relay
sbit	 Relay_Start =	 P6^1;
sbit	 Relay_Stop = 	 P6^2;

//***********************Function Declaration*********************************
bit ReadUID(uchar *CardType, uchar *UID);
bit FindUID(uchar *UID,uint IAP_CurrentAddr);
#endif