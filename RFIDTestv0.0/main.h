#ifndef __MAIN_H__
#define __MAIN_H__
#include <STC8.H>
#include <intrins.h>
#include <String.h>

#define uchar unsigned char
#define uint unsigned int

#define FOSC 24000000L
//#define FOSC 11059200L

#define BAUD1 115200//115200
#define BAUD2 115200
#define BAUD3 9600//115200
#define BAUD4 115200

#define Timer0Hz 1000 //每1ms进入一次定时器中断

//**************************************************************


//蜂鸣器和跑步机
sbit     LED_GREEN  =    P5^5;
sbit	 Relay_Start =	 P6^1;
sbit	 Relay_Stop = 	 P6^2;


//*************************************************************
void ArrayReset(unsigned char *Array,unsigned char Ch,char Head,char Length);
bit FindUID1();
#endif