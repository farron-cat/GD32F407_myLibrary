#include "bsp_pcf8563.h"
#include "msp_iic.h"
#include <stdio.h>

// BCD转十进制，十进制转BCD
#define DEC2BCD(dec) ((((dec) / 10) << 4) | ((dec) % 10))
#define BCD2DEC(bcd) ((((bcd) >> 4) * 10) + ((bcd) & 0x0F))

static uint8_t sta_c2 = 0; // 状态控制寄存器 0x01 缓存

void bsp_pcf8563_init(void)
{
    // 软件 IIC 由 msp_iic_init() 完成；这里留空
}

// 读一个字节(秒)
uint8_t bsp_pcf8563_read_byte(void)
{
    uint8_t d = 0;
    msp_iic_read_nbyte(PCF8563_ADDR, 0x02, &d, 1);
    return d;
}

// 写时间（十进制 → BCD
void bsp_pcf8563_set_time(Time_pcf *t)
{
    if (t->year < 1900 || t->year > 2099)
        return;

    uint8_t timeBuffer[7] = {0};

    timeBuffer[0] = DEC2BCD(t->second);  // 秒
    timeBuffer[1] = DEC2BCD(t->minutes); // 分
    timeBuffer[2] = DEC2BCD(t->hour);    // 时
    timeBuffer[3] = DEC2BCD(t->day);     // 日
    timeBuffer[4] = t->week & 0x07;      // 周(0~6，非BCD)
    timeBuffer[5] = DEC2BCD(t->month);   // 月

    if (t->year > 1999) // 2000 年之后世纪位置 1
        timeBuffer[5] |= (1 << 7);

    timeBuffer[6] = DEC2BCD(t->year % 100); // 年后两位

    msp_iic_write_nbyte(PCF8563_ADDR, 0x02, timeBuffer, 7);
}

// 读时间（BCD → 十进制）
void bsp_pcf8563_read_time(Time_pcf *t)
{
    uint8_t timeBuffer[7] = {0};

    msp_iic_read_nbyte(PCF8563_ADDR, 0x02, timeBuffer, 7);

    t->second = BCD2DEC(timeBuffer[0] & 0x7F);  // VLsss ssss
    t->minutes = BCD2DEC(timeBuffer[1] & 0x7F); // xmmm mmmm
    t->hour = BCD2DEC(timeBuffer[2] & 0x3F);    // xxhh hhhh
    t->day = BCD2DEC(timeBuffer[3] & 0x3F);     // xxDD DDDD
    t->week = timeBuffer[4] & 0x07;             // xxxx xwww
    t->month = BCD2DEC(timeBuffer[5] & 0x1F);   // CxxM MMMM

    t->year = BCD2DEC(timeBuffer[6]); // xxYY YYYY

    if ((timeBuffer[5] >> 7) & 0x01)
        t->year += 2000;
    else
        t->year += 1900;
}

// 设置闹钟
void bsp_pcf8563_set_alarm(Alarm *alarm)
{
    uint8_t timeBuffer[4] = {0};

    timeBuffer[0] = DEC2BCD(alarm->min & 0x7F);
    timeBuffer[1] = DEC2BCD(alarm->hour & 0x7F);

    // day：如果 bit7=1（屏蔽），原样保留；否则 BCD 转换
    if (alarm->day & 0x80)
        timeBuffer[2] = alarm->day;
    else
        timeBuffer[2] = DEC2BCD(alarm->day);

    // week：保留 bit7，低 3 位有效
    if (alarm->week & 0x80)
        timeBuffer[3] = 0x80;
    else
        timeBuffer[3] = alarm->week & 0x07;

    msp_iic_write_nbyte(PCF8563_ADDR, 0x09, timeBuffer, 4);
}

// 开启闹钟
void bsp_pcf8563_alarm_enable(void)
{
    msp_iic_read_nbyte(PCF8563_ADDR, 0x01, &sta_c2, 1);
    sta_c2 |= (1 << 1);  // AIE = 1，允许闹钟中断
    sta_c2 &= ~(1 << 3); // AF  = 0，清标志
    msp_iic_write_nbyte(PCF8563_ADDR, 0x01, &sta_c2, 1);
}

// 清除闹钟
void bsp_pcf8563_alarm_clear(void)
{
    msp_iic_read_nbyte(PCF8563_ADDR, 0x01, &sta_c2, 1);
    sta_c2 &= ~(1 << 3); // AF = 0
    msp_iic_write_nbyte(PCF8563_ADDR, 0x01, &sta_c2, 1);
}

// 配置定时器
// clock: 0x80=1Hz  0x81=64Hz  0x82=1/60Hz  0x83=1/3600Hz
void bsp_pcf8563_set_timer(uint8_t clock, uint8_t cnt)
{
    uint8_t timeBuffer[2] = {0};

    timeBuffer[0] = 0x80 | (clock & 0x7F); // 打开定时器 + 时钟频率
    timeBuffer[1] = cnt;                   // 计数值

    msp_iic_write_nbyte(PCF8563_ADDR, 0x0E, timeBuffer, 2);
}

// 开启定时器
void bsp_pcf8563_timer_enable(void)
{
    msp_iic_read_nbyte(PCF8563_ADDR, 0x01, &sta_c2, 1);
    sta_c2 |= (1 << 0);  // TIE = 1
    sta_c2 &= ~(1 << 2); // TF  = 0
    msp_iic_write_nbyte(PCF8563_ADDR, 0x01, &sta_c2, 1);
}

// 清除定时器
void bsp_pcf8563_timer_clear(void)
{
    msp_iic_read_nbyte(PCF8563_ADDR, 0x01, &sta_c2, 1);
    sta_c2 &= ~(1 << 2); // TF = 0
    msp_iic_write_nbyte(PCF8563_ADDR, 0x01, &sta_c2, 1);
}

#ifndef __weak
#define __weak __attribute__((weak)) // GCC / armclang
#endif

// 闹钟回调函数
__weak void on_rtc_alarm(void)
{
    printf("alarm\n");
}
// 定时器回调函数
__weak void on_rtc_timer(void)
{
    printf("timer\n");
}

// PCF8563 中断事件处理
// 不再在 EXTI5 中断里执行，改由主循环检测到 g_rtc_int_flag 后调用（避免软件 IIC 重入）
void on_rtc_int(void)
{
    uint8_t sta = 0;

    // 读状态控制寄存器 0x01；读取失败直接返回，避免用旧缓存值误判
    if (msp_iic_read_nbyte(PCF8563_ADDR, 0x01, &sta, 1) != IIC_SUC)
        return;

    // 判断 AF 标志位
    if ((sta >> 3) & 0x01)
    {
        bsp_pcf8563_alarm_clear();
        on_rtc_alarm();
    }

    // 判断 TF 标志位
    if ((sta >> 2) & 0x01)
    {
        bsp_pcf8563_timer_clear();
        on_rtc_timer();
    }
}