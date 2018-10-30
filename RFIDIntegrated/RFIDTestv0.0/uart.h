#ifndef __UART_H__
#define __UART_H__

#include <STC8.H>
#include <main.h>
#include <intrins.h>

void SendData(unsigned char Uart_Port, unsigned char dat);
void SendString(unsigned char Uart_Port,char *s);
void SendArrayHex(unsigned char Uart_Port,char *s,uint n);
void Read_Card_UID(uchar *UID_buffer,uchar *Card_Info,uchar Msg);
unsigned char GetMileage(unsigned char *GymInfoBuff, unsigned char *MileageInfo, uchar Msg);
void Init_Uart();
void Init_UART1();
void Init_UART2();
void Init_UART3();
void Init_UART4();

#endif