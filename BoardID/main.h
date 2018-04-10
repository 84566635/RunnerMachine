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

//**************************************************************

//*************************************************************
void ArrayReset(unsigned char *Array,unsigned char Ch,char Head,char Length);
void delay1(uint n);
void ReadMechineID(unsigned char *Array,uchar code *Addr);
#endif