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
        if (UID_Receive_Flag) 					//new Card Flash
        {
            for (i = 0; i <= 3; i++)
            {
                UID_Temp[i] = Card_Info[5 + i];
            }
            if (FindUID(UID_Temp)) 				//There is a same Card in the List
            {
                
				Relay_Start = 0; 				//跑步机继电器闭合，正常工作
				GymInfoCom_10s = 0;
                SendArrayHex(2, Card_Info, 10);	//发送卡信息
				
				//SendArrayHex(1, Card_Info, 10);	//发送卡信息
				
                ArrayReset(Card_Info, 0, 3, 6); //清除缓存区，以备下一次读卡做准备
            }
            else
            {
                LED_GREEN = 0;
                delay1(3000);
                LED_GREEN = 1;
            }
            UID_Receive_Flag = 0;
        }

        if (Mileage_Finsh)						//里程读取是否结束？
        {

            Mileage[1] = MileageInfo[1]; 		//计算里程
            if (Mileage[0] != Mileage[1])
            {
                Relay_Start = 0;
				GymInfoCom_10s = 0;
            }
            Mileage[0] = Mileage[1];

            //SendArrayHex(1, MileageInfo, 2);
            memset(MileageInfo, 0x00, 2);
            Mileage_Finsh = 0;
        }
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
