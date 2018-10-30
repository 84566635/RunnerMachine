#ifndef __MAIN_H__
#define __MAIN_H__
#include <STC8.H>
#include <intrins.h>
#include <String.h>

#define uchar unsigned char
#define uint unsigned int

//系统时钟频率
#define FOSC 22118400L
//#define FOSC 24000000L
//#define FOSC 11059200L

//串口时钟频率
#define BAUD1 115200
#define BAUD2 115200
#define BAUD3 9600
#define BAUD4 115200

#define Timer0Hz 1000 //每1ms进入一次定时器中断

//存放单片机的全球唯一的ID号码
#define ID_Addr_RAM 0xf1
#define ID_Addr_ROM 0xfdf9

//**************************************************************


//蜂鸣器和跑步机
sbit     LED_GREEN  =    P5^5;
sbit	 Relay_Start =	 P6^1;
sbit	 Relay_Stop = 	 P6^2;


//*************************************************************
void ArrayReset(unsigned char *Array,unsigned char Ch,char Head,char Length);
void ReadMechineID(unsigned char *Array,uchar code *Addr);
#endif