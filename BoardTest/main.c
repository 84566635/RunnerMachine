#include "main.h"
#include "uart.h"
#include <fun.h>
#include <timer.h>

bit ReadStatus;
bit FindStatus;

extern uint xdata TimeIndex;
extern uchar xdata Card_Info[10];
extern bit UID_Receive_Flag;

extern uchar MileageInfo[2];
extern bit Mileage_Finsh;
uint Mileage[2] = {0, 0};
uchar GymInfoCom_10s_Index = 0;
extern uint GymInfoCom_10s;
//**********************************************************************
void main()
{
    uchar i = 0;
    uchar temp = 0;
    uchar UID_Temp[4] = {0x00, 0x00, 0x00, 0x00};

    Init_Uart();
    InitTimer0();

    while (1)

    {
		//LED_GREEN = 0;
		Relay_Start = 0;
		delay1(3000);
		//LED_GREEN = 1;
		Relay_Start = 1;
		
		SendString(1,"Uart-1\r\n");
		delay1(1000);
		SendString(2,"Uart-2\r\n");
		delay1(1000);
		SendString(3,"Uart-3\r\n");
		delay1(1000);
		SendString(4,"Uart-4\r\n");
		delay1(3000);
		
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
