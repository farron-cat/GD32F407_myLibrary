#ifndef BSP_PCF8563_H
#define BSP_PCF8563_H

#include "gd32f4xx.h"

#define PCF8563_ADDR 0x51

// 时间结构体（成员均为十进制，eg. 2026-09-28 14:59:55 week=1）
typedef struct {
    uint16_t year;   // 2000 ~ 2099
    uint8_t month;   // 1 ~ 12
    uint8_t day;     // 1 ~ 31
    uint8_t week;    // 0 ~ 6
    uint8_t hour;    // 0 ~ 23
    uint8_t minutes; // 0 ~ 59
    uint8_t second;  // 0 ~ 59
} Time_pcf;          // 有重复Time需要提出去

// 闹钟结构体
typedef struct {
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t week;
} Alarm;

void bsp_pcf8563_init(void); // 因为一致性保留
uint8_t bsp_pcf8563_read_byte(void);

void bsp_pcf8563_set_time(Time_pcf *t);  // 写时间(十进制)
void bsp_pcf8563_read_time(Time_pcf *t); // 读时间(十进制)

void bsp_pcf8563_set_alarm(Alarm *alarm);
void bsp_pcf8563_alarm_enable(void);
void bsp_pcf8563_alarm_clear(void);

void bsp_pcf8563_set_timer(uint8_t clock, uint8_t cnt);
void bsp_pcf8563_timer_enable(void);
void bsp_pcf8563_timer_clear(void);

void on_rtc_alarm(void);
void on_rtc_timer(void);
void on_rtc_int(void); // EXTI5 中断入口调用

#endif // BSP_PCF8563_H