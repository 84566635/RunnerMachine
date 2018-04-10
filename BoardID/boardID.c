#include "main.h"
#include "uart.h"

#define ID_Addr_RAM 0xf1
#define ID_Addr_ROM 0xfdf9

uchar xdata Card_Info[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//uchar idata *ID = 0xf1;

//**********************************************************************
void main()
{
    uchar i = 0;
    uchar temp = 0;
//	uchar idata *iptr;
//	uchar code *cptr;
//	iptr = ID_Addr_RAM;
//	cptr = ID_Addr_ROM;

//    for (i = 0; i < 7; i++)
//    {
//        Card_Info[i] = *iptr++;
//    }
//	 for (i = 0; i < 7; i++)
//    {
//        Card_Info[i] = *cptr++;
//    }
	
	Init_Uart();
	ReadMechineID(Card_Info,ID_Addr_ROM);
	delay1(1500);
    while (1)
    {
        SendString(1, "\r\n");
        SendArrayHex(1, Card_Info, 7);
        SendString(1, "\r\n");
        delay1(1500);
    }
}

/**清除数组中指定区域，
 * Array：待清除的数组
 * Ch：清除区域准备填充的数据
 * Head：待清除的起始地址
 * Length：待清除的数据长度
 */
void ArrayReset(unsigned char *Array, unsigned char Ch, char Head, char Length)
{
    char i = 0;
    for (i = Head; i <= Length; i++)
    {
        Array[i] = Ch;
    }
}
/**延时 n ms
 * n：n ms
 */
void delay1(uint n)
{
    uchar i, j;
    while (n--) {
        i = 15;
        j = 90;
        do
        {
            while (--j);
        } while (--i);
    }
}
/**读取单片机的全球唯一的ID号码
 * Array：存放ID的数组
 * uchar code *Addr：采用ROM的读取方法
 */
void ReadMechineID(unsigned char *Array,uchar code *Addr)
{
	uchar i = 0;
	for (i = 0; i < 7; i++)
    {
        Array[i] = *Addr++;
    }
}
