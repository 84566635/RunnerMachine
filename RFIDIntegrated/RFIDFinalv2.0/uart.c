#include "uart.h"

#define GymInfoSize 100	//跑步机信息缓存区的大小

bit busy1, busy2, busy3, busy4; //串口发送标志
uchar xdata _16_2_str[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

//15个字节的UID缓存区，其中卡类型：6byte+7byte；卡号为：11byte + 10byte + 9byte + 8byte
uchar xdata UID_buffer[15] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/**Card_Info[10]卡信息保存
 * aa 55：帧头:[0:1]--2byte
 * 06：[2]--1byte:发送卡信息的命令
 * card type：[3:4]--2byte
 * card UID：[5:8]--4byte
 * ff：[9]--2byte为帧尾
 */
extern uchar xdata Card_Info[10] = {0xaa, 0x55, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc};
extern uchar UID_Index = 0;
extern bit UID_Receive_Flag = 0; //完整卡信息读取完成标志
extern uchar TimeReadUID;
/**GymInfoBuff[20]跑步机运行状态缓存区
 * 帧头(aa + 55) + 01/02/03/04 + 状态信息
 */
extern uchar xdata GymInfoBuff[GymInfoSize] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
extern uchar MileageInfo[2] = {0x00, 0x00};
extern uchar TimeGymInfo;
extern uchar GymInfoIndex = 0; //里程保存指针
extern bit Mileage_Finsh = 0;
/************************************************/

/**串口发送字符  
 * dat：待发送的字符
 * Uart_Port：需要使用的串口号
 */
void SendData(unsigned char Uart_Port, unsigned char dat)
{
    switch (Uart_Port)
    {
    case 1:
        while (busy1)
            ;
        busy1 = 1;
        SBUF = dat;
        break;
    case 2:
        while (busy2)
            ;
        busy2 = 1;
        S2BUF = dat;
        break;
    case 3:
        while (busy3)
            ;
        busy3 = 1;
        S3BUF = dat;
        break;
    case 4:
        while (busy4)
            ;
        busy4 = 1;
        S4BUF = dat;
        break;
    default:
        break;
    }
}

/**串口发送字符串
 * Uart_Port：需要使用的串口号
 * s：待发送的字符串
 */
void SendString(unsigned char Uart_Port, char *s)
{
    while (*s)
    {
        SendData(Uart_Port, *s++);
    }
}
void SendArrayHex(unsigned char Uart_Port, char *s, uint n)
{
    while (n--)
    {
        SendData(Uart_Port, *s++);
    }
}
/********************Initialization********************************/
void Init_Uart()
{
    Init_UART1();
    Init_UART2();
    Init_UART3();
    Init_UART4();

    EA = 1;
}
void Init_UART1()
{
    SCON = 0x50;
    TL1 = (65536 - (FOSC / 4 / BAUD1));
    TH1 = (65536 - (FOSC / 4 / BAUD1)) >> 8;
    AUXR |= 0X40; //
    AUXR = 0x40;  //定时器1为1T模式
    TMOD = 0x00;  //定时器1为模式0(16位自动重载)
    TR1 = 1;
    ES = 1; //使能串口1中断

    busy1 = 0;
}

void Init_UART2()
{
    S2CON = 0x50;
    T2L = (65536 - (FOSC / 4 / BAUD2));
    T2H = (65536 - (FOSC / 4 / BAUD2)) >> 8;
    AUXR |= 0X14; //T2为1T模式，并启动定时器2
    IE2 |= 0x01;  //使能串口2中断
    busy2 = 0;
}

void Init_UART3()
{
    S3CON = 0x50;
    T3L = (65536 - (FOSC / 4 / BAUD3));
    T3H = (65536 - (FOSC / 4 / BAUD3)) >> 8;
    T4T3M |= 0x0a;
    IE2 |= 0x08; //使能串口3中断
    busy3 = 0;
}

void Init_UART4()
{
    S4CON = 0x50;
    T4L = (65536 - (FOSC / 4 / BAUD4));
    T4H = (65536 - (FOSC / 4 / BAUD4)) >> 8;
    T4T3M |= 0xa0;
    IE2 |= 0x10; //使能串口4中断
    busy4 = 0;
}

/**************UART_Interrupt_Fuctions****************************/
void UART1_Isr() interrupt 4 using 1
{
    if (TI)
    {
        TI = 0;    //清中断标志
        busy1 = 0; //清忙标志
    }
    if (RI)
    {
        RI = 0;
    }
}

void UART2_Isr() interrupt 8
{
    if (S2CON & 0x02)
    {
        S2CON &= ~0x02; //清中断标志
        busy2 = 0;      //清忙标志
    }
    if (S2CON & 0x01)
    {
        S2CON &= ~0x01; //清中断标志
    }
}

void Uart3Isr() interrupt 17
{
    uchar buffer = 0;
    if (S3CON & 0x02)
    {
        S3CON &= ~0x02;
        busy3 = 0;
    }
    if (S3CON & 0x01)
    {
        S3CON &= ~0x01;

        buffer = S3BUF;
        TimeReadUID = 0;
        Read_Card_UID(UID_buffer, Card_Info, buffer);
    }
}

void UART4_Isr() interrupt 18
{
    uchar Buffer = 0;
	uchar i = 0;
    if (S4CON & 0x02)
    {
        S4CON &= ~0x02; //清中断标志
        busy4 = 0;      //清忙标志
    }
    if (S4CON & 0x01)
    {
        S4CON &= ~0x01; //清中断标志

        Buffer = S4BUF;
		
		//S2BUF = Buffer;//直接透传给WIFI模块（串口2）
        TimeGymInfo = 0;
		
		
        GetMileage(GymInfoBuff, MileageInfo, Buffer);//100字节接收完毕之后，跟上卡号
		
        //S4BUF = Buffer;

        
    }
}
/**读取卡信息
 * UID_buffer：UID_buffer 缓存区
 * Card_Info：卡信息存储区
 * Msg：一个字节输入
 */
void Read_Card_UID(uchar *UID_buffer, uchar *Card_Info, uchar Msg)
{
    uchar i = 0;
    UID_buffer[UID_Index++] = Msg;
    if (UID_Index == 15)
    {
        UID_Index = 0;
		//get the card type
        for (i = 0; i < 2; i++)
        {
            Card_Info[3 + i] = UID_buffer[5 + i];
        }
		//get the card UID
        for (i = 0; i < 4; i++)
        {
            Card_Info[5 + i] = UID_buffer[10 - i];
        }
        UID_Receive_Flag = 1;
        memset(UID_buffer, 0x00, 15);
    }
    else if (UID_Index == 12)
    {
        UID_Index = 0;
		//get the card type
        for (i = 0; i < 2; i++)
        {
            Card_Info[3 + i] = UID_buffer[5 + i];
        }
		//get the card UID
        for (i = 0; i < 4; i++)
        {
            Card_Info[5 + i] = UID_buffer[10 - i];
        }
        UID_Receive_Flag = 1;
        memset(UID_buffer, 0x00, 12);
    }
    else
    {
    }
}
/**数据包解析
 * Input:
 *      GymInfoBuff[]：接收到的数据缓存
 *      MileageInfo[]：准备保存的里程
 *      Data_Length：检测的数据包的最小长度
 * Return:1：取到了里程数据；0：未取到里程数据
 */
void GetMileage(unsigned char *GymInfoBuff, unsigned char *MileageInfo, uchar Msg)
{
    uchar i = 0;
    uchar MileageIndex = 0;
    GymInfoBuff[GymInfoIndex++] = Msg;
    if (GymInfoIndex == GymInfoSize)
    {
        GymInfoIndex = 0;
        for (i = 0; i < GymInfoSize; i++)
        {
            if ((GymInfoBuff[i + 0] == 0xaa) && (GymInfoBuff[i + 1] == 0x55) && (GymInfoBuff[i + 2] == 0x03)) //如果是 aa 55 03,则是符合要求的信息，取到里程信息
            {

                MileageIndex = i + 5;
                MileageInfo[0] = GymInfoBuff[MileageIndex];
                MileageInfo[1] = GymInfoBuff[MileageIndex + 1];
                Mileage_Finsh = 1;
                break;
            }
            else
            {
                continue;
            }
        }
        //memset(GymInfoBuff, 0x00, GymInfoSize);
		//Relay_Stop = ~Relay_Stop;
    }
}
