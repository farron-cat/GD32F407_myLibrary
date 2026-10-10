#ifndef _MSP_ADC_H
#define _MSP_ADC_H

#include "gd32f4xx.h"
#include <stdio.h>
#include <string.h>
#include "systick.h"

#define ADC_LEN 3
#define VREF    3.3

// adc初始化
void msp_adc_init();

// 获取特定通道数据
uint16_t msp_adc_get(uint8_t i);

#endif