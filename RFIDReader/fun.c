#include <fun.h>

#define ID_NUM 8
// uchar CardType[2] = {0x00, 0x0};
// uchar UID[4] = {0x00, 0x00, 0x00, 0x00};
uchar xdata g_ucTempbuf[20];
const uchar xdata UIDlist[ID_NUM][4] = {
{0x89,0x85,0xb9,0x64},
{0x89,0x85,0x83,0x67},
{0x79,0xB9,0x59,0x29},
{0x58,0x8C,0x1B,0x30},
{0x24,0x3E,0xE3,0xB9},
{0x56,0xA9,0xF7,0x70},
{0x58,0x93,0x11,0x10},
{0x8A,0xDA,0x7C,0x04}
};

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
/**寻找指定UID的卡号
 * UID：需要匹配的卡号
 */
/*
bit FindUID(char *UID)
{
    char* UID_Temp = UID;
	uchar xdata i,k;
    for (i = 0; i < ID_NUM; i ++)
    {
        UID_Temp = UID;
		for (k = 0; k < 4;k ++)
        {
            if ((*UID_Temp++) != UIDlist[i][k])
            {
				break;
            }
			return 1;
       }
    }
    return 0;
}
*/