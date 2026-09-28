#include "bsp_pcf8563.h"
#include "msp_iic.h"

unsigned char bsp_pcf8563_read_byte(void)
{
    unsigned char dat;
    msp_iic_read_nbyte(PCF8563_ADDR, 0x02, &dat, 1);

    return dat;
}

// 读 秒 分
void bsp_pcf8563_read_time(Time_pcf *t)
{
    uint8_t timeBuffer[7] = {0};
    uint8_t Cent;

    msp_iic_read_nbyte(PCF8563_ADDR, 0x02, timeBuffer, 7);

    // 秒：VLsss ssss
    t->second = timeBuffer[0] & 0x7F; // 0x7F = 0111 1111 BCD码 (Binary Coded Decimal)：用4位2进制数表示1位十进制数的编码方式
    // 秒：VLsss ssss
    t->second = timeBuffer[0] & 0x7F; // 0x7F = 0111 1111 BCD码 (Binary Coded Decimal)：用4位2进制数表示1位十进制数的编码方式
    // 分： xmmm mmmm
    t->minutes = timeBuffer[1] & 0x7F;
    // 时： xxhh hhhh
    t->hour = timeBuffer[2] & 0x3F;
    // 天： xxDD DDDD
    t->day = timeBuffer[3] & 0x3F;
    // 周： xxxx xwww
    t->week = timeBuffer[4] & 0x07;

    // 世纪
    // 月:  CxxM MMMM
    t->month = timeBuffer[5] & 0x1F;
    Cent = timeBuffer[5] >> 7; // 0->20xx年， 1->21xx年

    if (Cent == 0)
    {
        t->year = timeBuffer[6] | (0x20 << 8); // eg.,2025
    }
    else
    {
        t->year = timeBuffer[6] | (0x21 << 8);
    }
}

// 设置时间
void bsp_pcf8563_set_time(void)
{
    //              second    minutes    hour    day    week    month    year
    uint8_t t_buff[7] = {0x00, 0x00, 0x14, 0x19, 0x03, 0x08, 0x26};

    msp_iic_write_nbyte(PCF8563_ADDR, 0x02, t_buff, 7);
}
