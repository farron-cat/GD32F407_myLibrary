#include "bsp_adkey.h"
#include "msp_adc.h"
#include <stdio.h>

void bsp_adkey_init(void)
{
    // 在msp_adc中初始化了
}

static uint8_t sample_cnt = 0;
uint16_t sample_sum = 0;
uint16_t sample_max = 0;
uint16_t sample_min = 4095;

void bsp_adkey_scan(void)
{
    uint16_t encode = msp_adc_get(3);
    sample_sum += encode;
    if (encode > sample_max)
    {
        sample_max = encode;
    }
    if (encode < sample_min)
    {
        sample_min = encode;
    }
    sample_cnt++;
    if (sample_cnt == 10)
    {
        sample_cnt = 0;

        encode = (sample_sum - sample_max - sample_min) / 8;
        float vol = encode * VREF / 4096;
        printf("vol:%f\n", vol);
        sample_sum = 0;
        sample_max = 0;
        sample_min = 4095;
    }
}