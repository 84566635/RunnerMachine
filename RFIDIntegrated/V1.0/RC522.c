#include "main.h"
#include "mfrc522.h"

#define MAXRLEN 18

//*****************************************************************
//功 能： 寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返 回: 成功返回MI_OK
//*****************************************************************
char PcdRequest(uchar req_code, uchar *pTagType)
{
    char status;
    uint  unLen;
    uchar ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08); //清除 MRCrypto1on,要用软件清零
    WriteRawRC(BitFramingReg, 0x07); //startsend=0,rxalign=0, 在 FIFO 中 存 放 的 位 置 ，TXlastbit=7
    SetBitMask(TxControlReg, 0x03); //TX2rfen=1,TX1RFen=1, 传递调制的 13.56MHZ 的载波信号

    ucComMF522Buf[0] = req_code;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);

    if ((status == MI_OK) && (unLen == 0x10))
    {
        *pTagType     = ucComMF522Buf[0];
        *(pTagType + 1) = ucComMF522Buf[1];
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

//*****************************************************************
//功 能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返 回: 成功返回MI_OK
//*****************************************************************
char PcdAnticoll(uchar *pSnr)
{
    char status;
    uchar i, snr_check = 0;
    uint  unLen;
    uchar ucComMF522Buf[MAXRLEN];


    ClearBitMask(Status2Reg, 0x08); //清除 MRCrypto1on ，要用软件清零
    WriteRawRC(BitFramingReg, 0x00); //表示最后一个字节所有位都发送
    ClearBitMask(CollReg, 0x80); //CollRegCollReg 0 冲突结束后冲突位被置零

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);

    if (status == MI_OK)
    {
        for (i = 0; i < 4; i++)
        {
            *(pSnr + i)  = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {
            status = MI_ERR;
        }
    }

    SetBitMask(CollReg, 0x80); //CollRegCollReg 在 106kbps 良好的防冲突情况下该位置 1
    return status;
}

//*****************************************************************
//功 能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返 回: 成功返回MI_OK
//*****************************************************************
char PcdSelect(uchar *pSnr)
{
    char status;
    uchar i;
    uint  unLen;
    uchar ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i = 0; i < 4; i++)
    {
        ucComMF522Buf[i + 2] = *(pSnr + i);
        ucComMF522Buf[6]  ^= *(pSnr + i);
    }
    CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);

    ClearBitMask(Status2Reg, 0x08); //清零MFcryon

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);

    if ((status == MI_OK) && (unLen == 0x18))
    {
        status = MI_OK;
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

//*****************************************************************
//功 能：验证卡片密码
//参数功能: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返 回: 成功返回MI_OK
//*****************************************************************
char PcdAuthState(uchar auth_mode, uchar addr, uchar *pKey, uchar *pSnr)
{
    char status;
    uint  unLen;
    uchar i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = auth_mode; //验证 A 密钥
    ucComMF522Buf[1] = addr;	  //addr[IN] ：块地址
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 2] = *(pKey + i);
    }
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 8] = *(pSnr + i);
    }
//   memcpy(&ucComMF522Buf[2], pKey, 6);
//   memcpy(&ucComMF522Buf[8], pSnr, 4);

    status = PcdComMF522(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }

    return status;
}

//*****************************************************************
//功 能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出数据，16字节
//返 回: 成功返回MI_OK
//*****************************************************************
char PcdRead(uchar addr, uchar *pData)
{
    char status;
    uint  unLen;
    uchar i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x90))
//   {   memcpy(pData, ucComMF522Buf, 16);   }
    {
        for (i = 0; i < 16; i++)
        {
            *(pData + i) = ucComMF522Buf[i];
        }
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

//*****************************************************************
//功 能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返 回: 成功返回MI_OK
//*****************************************************************
char PcdWrite(uchar addr, uchar *pData)
{
    char status;
    uint  unLen;
    uchar i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pData + i);
        }
        CalulateCRC(ucComMF522Buf, 16, &ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }

    return status;
}

//*****************************************************************
//功 能：扣款和充值
//参数说明: dd_mode[IN]：命令字
//               0xC0 = 扣款
//               0xC1 = 充值
//          addr[IN]：钱包地址
//          pValue[IN]：4字节增（减）值，低位在前
//返 回: 成功返回MI_OK
//*****************************************************************
char PcdValue(uchar dd_mode, uchar addr, uchar *pValue)
{
    char status;
    uint  unLen;
    uchar i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = dd_mode;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        // memcpy(ucComMF522Buf, pValue, 4);
        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pValue + i);
        }
        CalulateCRC(ucComMF522Buf, 4, &ucComMF522Buf[4]);
        unLen = 0;
        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen);
        if (status != MI_ERR)
        {
            status = MI_OK;
        }
    }

    if (status == MI_OK)
    {
        ucComMF522Buf[0] = PICC_TRANSFER;
        ucComMF522Buf[1] = addr;
        CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }
    return status;
}

//*****************************************************************
//功 能：备份钱包
//参数说明: sourceaddr[IN]：源地址
//          goaladdr[IN]：目的地址
//返 回: 成功返回MI_OK
//*****************************************************************
char PcdBakValue(uchar sourceaddr, uchar goaladdr)
{
    char status;
    uint  unLen;
    uchar ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_RESTORE;
    ucComMF522Buf[1] = sourceaddr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        ucComMF522Buf[0] = 0;
        ucComMF522Buf[1] = 0;
        ucComMF522Buf[2] = 0;
        ucComMF522Buf[3] = 0;
        CalulateCRC(ucComMF522Buf, 4, &ucComMF522Buf[4]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen);
        if (status != MI_ERR)
        {
            status = MI_OK;
        }
    }

    if (status != MI_OK)
    {
        return MI_ERR;
    }

    ucComMF522Buf[0] = PICC_TRANSFER;
    ucComMF522Buf[1] = goaladdr;

    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    return status;
}


//*****************************************************************
//功 能：命令卡进入休眠状态
//返 回: 成功返回MI_OK
//*****************************************************************
char PcdHalt(void)
{
    char status;
    uint  unLen;
    uchar ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    return MI_OK;
}

//*****************************************************************
//用MF522计算CRC16函数
//*****************************************************************
void CalulateCRC(uchar *pIndata, uchar len, uchar *pOutData)
{
    uchar i, n;
    ClearBitMask(DivIrqReg, 0x04);		//取消当前命令
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);		//FlushBuffer 清除 ErrReg 的标志位
    for (i = 0; i < len; i++)
    {
        WriteRawRC(FIFODataReg, *(pIndata + i));
    }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i != 0) && !(n & 0x04));		//当 CRCIRq 所有数据被处理完毕该位置位
    pOutData[0] = ReadRawRC(CRCResultRegL);	//显示计算出来的 CRC 值
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

//*****************************************************************
//功 能：复位RC522
//返 回: 成功返回MI_OK
//*****************************************************************
char PcdReset(void)
{
    MF522_RST = 1;
    _nop_();//这里延时10ns,需要注意
    MF522_RST = 0;
    _nop_();
    MF522_RST = 1;
    _nop_();
    WriteRawRC(CommandReg, PCD_RESETPHASE);
    _nop_();

    WriteRawRC(ModeReg, 0x3D);           //和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL, 30);         //定时器的低8位数据
    WriteRawRC(TReloadRegH, 0);			 //定时器的高8位数据
    WriteRawRC(TModeReg, 0x8D);			 //定时器模式寄存器，定时器减值计数
    WriteRawRC(TPrescalerReg, 0x3E);	 //实际值是0xD3E，这部分主要是设置定时器寄存器
    WriteRawRC(TxAutoReg, 0x40);		 // 必须要，设置逻辑 1，强制 100%ASK 调制

    return MI_OK;
}

//*****************************************************************
//功 能：读RC632寄存器
//参数说明：Address[IN]:寄存器地址
//返 回：读出的值
//*****************************************************************
uchar ReadRawRC(uchar Address)
{
    uchar i, ucAddr;
    uchar ucResult = 0;

    MF522_SCK = 0;
    MF522_NSS = 0;
    ucAddr = ((Address << 1) & 0x7E) | 0x80;

    for(i = 8; i > 0; i--)
    {
        MF522_SI = ((ucAddr & 0x80) == 0x80);
        MF522_SCK = 1;
        ucAddr <<= 1;
        MF522_SCK = 0;
    }

    for(i = 8; i > 0; i--)
    {
        MF522_SCK = 1;
        ucResult <<= 1;
        ucResult |= (bit)MF522_SO;
        MF522_SCK = 0;
    }

    MF522_NSS = 1;
    MF522_SCK = 1;
    return ucResult;
}

//*****************************************************************
//功 能：写RC632寄存器
//参数说明：Address[IN]:寄存器地址
//          value[IN]:写入的值
//*****************************************************************
void WriteRawRC(uchar Address, uchar value)
{
    uchar i, ucAddr;

    MF522_SCK = 0;
    MF522_NSS = 0;
    ucAddr = ((Address << 1) & 0x7E);

    for(i = 8; i > 0; i--)
    {
        MF522_SI = ((ucAddr & 0x80) == 0x80);
        MF522_SCK = 1;
        ucAddr <<= 1;
        MF522_SCK = 0;
    }

    for(i = 8; i > 0; i--)
    {
        MF522_SI = ((value & 0x80) == 0x80);
        MF522_SCK = 1;
        value <<= 1;
        MF522_SCK = 0;
    }
    MF522_NSS = 1;
    MF522_SCK = 1;
}

//*****************************************************************
//功 能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
//*****************************************************************
void SetBitMask(uchar reg, uchar mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp | mask); // set bit mask
}

//*****************************************************************
//功 能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
//*****************************************************************
void ClearBitMask(uchar reg, uchar mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
}

//*****************************************************************
//功 能：通过RC522和ISO14443卡通讯
//参数说明： Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
//*****************************************************************
char PcdComMF522(uchar Command,
                 uchar *pInData,
                 uchar InLenByte,
                 uchar *pOutData,
                 uint  *pOutLenBit)
{
    char status = MI_ERR;
    uchar irqEn   = 0x00;
    uchar waitFor = 0x00;
    uchar lastBits;
    uchar n;
    uint i;
    switch (Command)
    {
    case PCD_AUTHENT:
        irqEn   = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE:// 发送并接收数据
        irqEn   = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    WriteRawRC(ComIEnReg, irqEn | 0x80);// 容许除定时器中断请求以为得所有中断请求
    ClearBitMask(ComIrqReg, 0x80);// 屏蔽位清除
    WriteRawRC(CommandReg, PCD_IDLE);// 取消当前命令
    SetBitMask(FIFOLevelReg, 0x80);// 清除 FIFO 中的读写指针

    for (i = 0; i < InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pInData[i]);// 写寻卡命令
    }
    WriteRawRC(CommandReg, Command);// 发送并接收数据


    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg, 0x80);// 相当于启动发送 STARTSENG
    }

    i = 2000;//根据时钟频率调整，操作M1卡最大等待时间25ms
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    }
    while ((i != 0) && !(n & 0x01) && !(n & waitFor));
    ClearBitMask(BitFramingReg, 0x80);// 相当于清除发送 STARTSENG

    if (i != 0)// 定时时间到， i，没有递减到 0
    {
        if(!(ReadRawRC(ErrorReg) & 0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }
            if (Command == PCD_TRANSCEIVE)
            {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n * 8;
                }
                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAXRLEN)
                {
                    n = MAXRLEN;
                }
                for (i = 0; i < n; i++)
                {
                    pOutData[i] = ReadRawRC(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }

    }


    SetBitMask(ControlReg, 0x80);          // stop timer now
    WriteRawRC(CommandReg, PCD_IDLE);
    return status;
}


//*****************************************************************
//开启天线
//每次启动或关闭天线发射之间至少应该有1ms的间隔
//*****************************************************************
void PcdAntennaOn()
{
    uchar i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);//tx12RFEN==11; ,打开发射管脚
    }
}

//*****************************************************************
//关闭天线
//*****************************************************************
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);//tx12RFEN==00; ,禁止发射管脚
}
//*****************************************************************
//rc522初始化
//*****************************************************************
void InitRc522(void)
{
  PcdReset();
  PcdAntennaOff(); 
  PcdAntennaOn();
}

//*****************************************************************
//设置 RC522 的工作方式
//*****************************************************************
/*
char M500PcdConfigISOType(unsigned char data type)
{
    if (type == 'A') //ISO14443_A
    {
        ClearBitMask(Status2Reg, 0x08); //状态 2 寄存器
        WriteRawRC(ModeReg, 0x3D); //3F //和 Mifare 卡通讯， CRC 初始值 0x6363
        WriteRawRC(RxSelReg, 0x86); //84 选择内部接收器设置，内部模拟部分调制信号，
        发送数据后，延迟 6 个位时钟，接收
        WriteRawRC(RFCfgReg, 0x7F); //4F 配置接收器 48dB 最大增益
        WriteRawRC(TReloadRegL, 30); //tmoLength);// TReloadVal = 'h6a =tmoLength(dec)
        WriteRawRC(TReloadRegH, 0); //实际值是 OXD3E 这部分主要是设置定时器寄存器，
        WriteRawRC(TModeReg, 0x8D);
        WriteRawRC(TPrescalerReg, 0x3E);
        delay_ns(1000);
        PcdAntennaOn();
    }
    else {
        return -1;
    }
    return MI_OK;
}
*/