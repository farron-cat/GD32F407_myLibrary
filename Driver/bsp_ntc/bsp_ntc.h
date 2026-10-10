#ifndef _BSP_NTC_H
#define _BSP_NTC_H

#include "msp_adc.h"

#define NTC_GET_ENCODE() msp_adc_get(2)

// 初始化热敏电阻
void bsp_ntc_init(void);

// 获取热敏电阻温度值
int8_t bsp_ntc_get_tem(void);

#endif