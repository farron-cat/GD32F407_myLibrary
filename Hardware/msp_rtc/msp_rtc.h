#ifndef MSP_RTC_H
#define MSP_RTC_H

#include "gd32f4xx.h"

// 0x1A  0010 0110
// 26->0x26   26/10 = 2   26%10 = 6
#define DEC2BCD(val) ((((val) / 10) << 4) | ((val) % 10))
// 0x26->26  0x26>>4=2  0x26&0x0F = 6
#define BCD2DEC(val) (((val) >> 4) * 10 + ((val) & 0x0F))

// 日历时间结构体
typedef struct
{
    uint16_t year; // 年里包含世纪
    uint8_t month;
    uint8_t day;
    uint8_t week;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} Time;

typedef enum {
    HXTAL = 0,
    LXTAL,
    IRC32K
} RTC_CLOCK_SOURCE;

void msp_rtc_init(RTC_CLOCK_SOURCE rcs);

void msp_rtc_read(Time *time);

void msp_rtc_write(Time *time);

void msp_rtc_alarm_config(Time *time);

#endif // MSP_RTC_H