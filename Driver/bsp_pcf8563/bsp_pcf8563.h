#ifndef BSP_PCF8563_H
#define BSP_PCF8563_H

#include "gd32f4xx.h"

#define PCF8563_ADDR 0x51

// 时间结构体
typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t week;
    uint8_t hour;
    uint8_t minutes;
    uint8_t second;
} Time_pcf;

unsigned char bsp_pcf8563_read_byte(void);
void bsp_pcf8563_read_time(Time_pcf *t);
void bsp_pcf8563_set_time(void);

#endif // BSP_PCF8563_H