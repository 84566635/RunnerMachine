#ifndef __UART_H__
#define __UART_H__

#include <STC8F.H>
#include <intrins.h>

#define FOSC 24000000L
#define BAUD 115200

void Init_Uart();
void SendData(unsigned char dat);
void SendString(char*s);
void delay1(unsigned int z);
void send_16_2_str(unsigned char temp);
void SendArrayStr(unsigned int n, unsigned char *p);

#endif