#include <timer.h>

extern uint xdata TimeIndex = 0;
extern uchar TimeReadUID = 0;//读卡器定时器计时
extern uchar UID_Index;
extern uchar TimeGymInfo = 0;//跑步机数据定时器计时
extern uchar GymInfoIndex;
extern uint GymInfoCom_10s = 0;//每一定时间比较一下里程值
void InitTimer0()
{
    AUXR |= 0x80;                         //定时器时钟1T模式
    TMOD &= 0xF0;                         //设置定时器
    TL0 = (65536 - FOSC / Timer0Hz);      //设置定时初值
    TH0 = (65536 - FOSC / Timer0Hz) >> 8; //设置定时初值
    TF0 = 0;                              //清楚TF0标志
    TR0 = 1;                              //定时器0开始计时
    ET0 = 1;
    EA = 1;
}

void Timer0Isr() interrupt 1
{
    TimeIndex = TimeIndex + 1;
    TimeReadUID = TimeReadUID + 1;
    TimeGymInfo = TimeGymInfo + 1;
	GymInfoCom_10s = GymInfoCom_10s + 1;
	
    if (TimeIndex >= 500)
    {
        TimeIndex = 0;
        UID_Index = 0;
		GymInfoIndex = 0;
    }

    if (TimeReadUID >= 100)//100ms间隔，读卡号
    {
        TimeReadUID = 0;
        UID_Index = 0;
    }
    if (TimeGymInfo >= 200)//200ms间隔，读取里程
    {
        TimeGymInfo = 0;
        GymInfoIndex = 0;
    }
	if(GymInfoCom_10s == 65000)
	{
		P6 |= 0x02;
		GymInfoCom_10s = 0;
	}
}