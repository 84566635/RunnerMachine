#include "uart.h"
#include "main.h"
#include "E2PROM.h"
#include "MFRC522.h"
#include "String.h"

#define IAP_DefaultADDR 0x0000

uchar xdata data1[10] = {0,1,2,3,4,5,6,7,8,9};

uchar xdata msg[15];
uchar xdata g_ucTempbuf[20];//The Buffer of Card information
uchar xdata CardType[2];//The Buffer of CardType
uchar xdata UID[4];//The Buffer of Card_ID_number

//uchar E2PROM_Index = 0;
WORD IAP_CurrentAddr = IAP_DefaultADDR;
uchar WriteFlag = 0;

void main()
{
	uchar ReadStatus = 0;
	uchar FindStatus = 0;
    uchar i = 0;
	uchar temp = 0;
	
	Init_Uart();
	SendString("Initing...");
	
	InitRc522();
	
	IapEraseSector(IAP_DefaultADDR);//E2PROM擦除，只需第一次擦除就好
	delay1(10);

	LED_GREEN = 1;
	
	while(1)
	{
		
		ReadStatus = ReadUID(CardType, UID);
		if(ReadStatus == 1)//ReadCarding is successful
		{
			SendString("card type:");//Printting Card—Type 2 Byte
			SendArrayStr(2, CardType);
            SendString("\r\n");
            SendString("Card_Serial_number:\n");//´òÓ¡¿¨ÐòÁÐ
            SendArrayStr(4, UID);
			SendString("\r\n");
			PcdHalt();
			ReadUID(CardType,UID);
		}		
		SendString("\r\n");
			
	}

}
/*************************读取卡号*************************************
 * CardType:读到的卡类型，2Byte
 * UID：读到的卡ID号，4Byte
 * return :status 0:fail,1:success
 */
bit ReadUID(uchar *CardType, uchar *UID)
{
    uchar xdata status = 0;
    uchar i;
    status = PcdRequest(PICC_REQALL, g_ucTempbuf);////Searching Card
    if (status != MI_OK)
    {
        InitRc522();
        return 0;
    }
    for(i = 0; i < 2; i++)
    {
        (*CardType++) = g_ucTempbuf[i];
    }
    
    status = PcdAnticoll(g_ucTempbuf);//防冲撞
    if (status != MI_OK)
    {
        return 0;
    }
    for(i = 0; i <= 3; i++)
    {
        (*UID++) = g_ucTempbuf[3-i];
    }
    return 1;
}
/**********************在E2PROM中寻卡**********************************
 * UID:需要寻找的卡ID号，默认在第一页中寻找，512Byte
 * IAP_CurrentAdress：记录寻找到的位置
 * return:status 0:There is no same Card in E2PROM;
 * 				 1:There has a same Card in E2PROM。
 */ 
bit FindUID(uchar *UID,WORD IAP_CurrentAddr)
{
	uchar i = 0;
	//E2PROM中ID号遍历，查找相同卡号,共计可以存储128个ID 
	for(i = 0;i <= 127;i ++)
	{
		IapReadFlash(IAP_DefaultADDR + 4 * i,4,msg);
		if(strcmp(UID,msg) == 0)//如果存在相同卡号
		{
			IAP_CurrentAddr = i;
			return 1;
		}
	}
	return 0;
}
