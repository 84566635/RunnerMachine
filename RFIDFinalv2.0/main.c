#include "main.h"
#include "uart.h"
#include <fun.h>
#include <timer.h>

extern uint xdata TimeIndex;
extern uchar xdata Card_Info[10];

/**CorrectCardInfo[20]卡信息保存
 * aa 55：帧头:[0:1]--2byte
 * 06：[2]--1byte:发送卡信息的命令
 * card type：[3:4]--2byte
 * card UID：[5:8]--4byte
 * cc cc：[9:10]--2byte作为 card UID 和 mechine ID 的分隔符
 * mechine ID：[11:17]--7byte
 * ff ff：[18:19]--2byte为帧尾
 */
uchar CorrectCardInfo[20] = {0xaa, 0x55, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0xcc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff};
extern bit UID_Receive_Flag;
//存放mechine ID 数组，共计7个字节
uchar xdata mechine_ID[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

extern uchar MileageInfo[2];
extern bit Mileage_Finsh;
extern uchar xdata GymInfoBuff[100];
uchar Mileage[2] = {0, 0};
uint FullMileage[2] = {0, 0};
uchar GymInfoCom_10s_Index = 0;
extern uint GymInfoCom_10s;
//**********************************************************************
void main()
{
    uchar i = 0, k = 0;
    uchar temp = 0;
    uchar UID_Temp[4] = {0x00, 0x00, 0x00, 0x00};

    Init_Uart();
    InitTimer0();

	//get the mechine ID and save it in mechine_ID
    ReadMechineID(mechine_ID, ID_Addr_ROM);
	delay1(200);
	for(i = 0;i < 7; i++)
	{
		CorrectCardInfo[11 + i] = mechine_ID[i];
	}    
    
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
                for(k = 0; k < 10; k++)
                {
                    CorrectCardInfo[k] = Card_Info[k];
                }

                SendArrayHex(2, Card_Info, 10);	//发送卡信息

                ArrayReset(Card_Info, 0, 3, 6); //清除缓存区，以备下一次读卡做准备
            }
            else
            {
                LED_GREEN = 0;
                delay1(2000);
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

            FullMileage[1] = (MileageInfo[0] << 8) + MileageInfo[1];
            if(FullMileage[0] != FullMileage[1])
            {
                Relay_Start = 0;
                GymInfoCom_10s = 0;
            }
            FullMileage[0] = FullMileage[1];

            memset(MileageInfo, 0x00, 2);			//清除里程信息

            SendArrayHex(2, GymInfoBuff, 100); 		//发送跑步机状态信息
            SendArrayHex(2, CorrectCardInfo, 20);	//发送卡信息

            memset(GymInfoBuff, 0x00, 100);	   		//清除跑步机状态信息
            Mileage_Finsh = 0;				  		//100字节标志位清零
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
/**读取单片机的全球唯一的ID号码
 * Array：存放ID的数组
 * uchar code *Addr：采用ROM的读取方法
 */
void ReadMechineID(unsigned char *Array, uchar code *Addr)
{
    uchar i = 0;
    for (i = 0; i < 7; i++)
    {
        Array[i] = *Addr++;
    }
}
