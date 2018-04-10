#include "uart.h"
unsigned char xdata _16_2_str[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
bit busy;
//bit recive_flag = 0;

void Init_Uart()
{
	SCON = 0x50;
    T2L = (65536 - (FOSC / 4 / BAUD));
    T2H = (65536 - (FOSC / 4 / BAUD)) >> 8;
    AUXR = 0X14;
    AUXR |= 0X01;
    ES = 1;
    EA = 1;
}
void SendData(unsigned char dat)
{
    while(busy);
    ACC = dat;
    busy = 1;
    SBUF = ACC;
}

void SendString(char*s)
{
    while (*s)
    {
        SendData(*s++);
    }
}
void delay1(unsigned int z)
{
    unsigned int x, y;
    for(x = z; x > 0; x--)
        for(y = 110; y > 0; y--);
}

void Uart() interrupt 4 using 1
{
    /*
	if (RI) {
        RI=0;
        P0=SBUF;
        P22=RB8;
		recive_flag = 1;
    }
	*/
    if(TI)
    {
        TI = 0;
        busy = 0;
    }
}
/*******************按照16进制串口打印********************************
*参数 uchar temp:需要16进制串口打印的数据
*/
void send_16_2_str(unsigned char temp)
{
    unsigned char str[2] = {'0', '0'};
    unsigned char low;
    unsigned char high;
    low = temp & 0x0f;
    high = (temp & 0xf0) >> 4;
    str[0] = _16_2_str[high];
    str[1] = _16_2_str[low];
    SendData(str[0]);//打印高8位
	SendData(str[1]);//打印低8位
}
void SendArrayStr(unsigned int n, unsigned char *p)
{
    while(n--)
    {
        send_16_2_str(*p++);
    }
}