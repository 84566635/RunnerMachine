#ifndef __E2PROM_H__
#define __E2PROM_H__

#include <STC8F.H>
#include <intrins.h>
#include <String.h>

typedef unsigned char BYTE;
typedef unsigned int WORD;

//测试工作频率为 11.0592MHz
//sfr IAP_DATA =  0xC2;   //IAP数据寄存器
//sfr IAP_ADDRH = 0xC3;   //IAP地址寄存器高字节
//sfr IAP_ADDRL = 0xC4;   //IAP地址寄存器低字节
//sfr IAP_CMD =   0xC5;   //IAP命令寄存器
//sfr IAP_TRIG =  0xC6;   //IAP命令触发寄存器
//sfr IAP_CONTR = 0xC7;   //IAP控制寄存器

#define CMD_IDLE    0   //空闲模式
#define CMD_READ   1   //IAP字节读取命令
#define CMD_PROGRAM 2   //IAP字节编程命令
#define CMD_ERASE   3   //IAP扇区擦除命令

#define WT_30M  0x80    //不同时钟频率对应的IAP使能命令
#define WT_24M  0x81
#define WT_20M  0x82
#define WT_12M  0x83
#define WT_6M   0x84
#define WT_3M   0x85
#define WT_2M   0x86
#define WT_1M   0x87

//--------------------------------------------------
void IapIdle();
BYTE IapReadByte(WORD addr);
void IapProgramByte(WORD addr, BYTE dat);
void IapEraseSector(WORD addr);
int IapProgramFlash(WORD addrOrigin,BYTE* Msg);
void IapReadFlash(WORD addrOrigin,int Length,BYTE* ReadMsg);
void Delaymus(unsigned char m);
void Delay1us();

#endif