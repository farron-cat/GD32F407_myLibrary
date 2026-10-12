#include "bsp_adkey.h"
#include "msp_adc.h"
#include <stdio.h>
#include <stdlib.h>

#define KEY1_PRESS_ENCODE 945
#define KEY2_PRESS_ENCODE 2043
#define KEY3_PRESS_ENCODE 3135

// ADKEY参数结构体数组
static ADKEY_PARAM adkeys[] = {
    {1, KEY1_PRESS_ENCODE, ADKEY_IDLE, 0},
    {2, KEY2_PRESS_ENCODE, ADKEY_IDLE, 0},
    {3, KEY3_PRESS_ENCODE, ADKEY_IDLE, 0},
};

// 初始化ADKEY 占位
void bsp_adkey_init(void)
{
    // 在msp_adc中初始化了
}

// 扫描单个按键 根据按键ADC编码值修改状态
static void scan_single_key(uint16_t encode, ADKEY_PARAM *key)
{
    switch (key->state)
    {
    case ADKEY_IDLE:
        if (abs(encode - key->press_encode) < 20)
        {
            key->state = ADKEY_PRESS_CONFORM;
            key->pre_time = get_us_cnt();
        }
        break;

    case ADKEY_PRESS_CONFORM:
        // 时间超过确认时间后电压仍然接近（按下）
        if ((abs(encode - key->press_encode) < 20) && ((get_us_cnt() - key->pre_time) > ADKEY_PRESS_CONFORM_TIM))
        {
            // 确认按下
            key->state = ADKEY_PRESS;
        }
        // 时间在确认时间内的电压不接近（松开）
        else if ((abs(encode - key->press_encode) >= 20) && ((get_us_cnt() - key->pre_time) <= ADKEY_PRESS_CONFORM_TIM))
        {
            // 是抖动
            key->state = ADKEY_IDLE;
        }
        break;

    case ADKEY_PRESS:
        // 时间在长按时间内的电压不接近（松开）
        if ((abs(encode - key->press_encode) >= 20) && ((get_us_cnt() - key->pre_time) <= ADKEY_LONG_PRESS_TIM))
        {
            // 确认单击
            key->state = ADKEY_IDLE;
            printf("KEY%d_CLICK\n", (int)key->id);
        }
        // 时间超过长按时间后电压仍然接近（按下）
        else if ((abs(encode - key->press_encode) < 20) && ((get_us_cnt() - key->pre_time) > ADKEY_LONG_PRESS_TIM))
        {
            // 确认长按
            key->state = ADKEY_LONG_PRESS;
            printf("KEY%d_LONG_PRESS\n", (int)key->id);
        }
        break;

    case ADKEY_LONG_PRESS:
        // 电压不接近（松开）
        if (abs(encode - key->press_encode) >= 20)
        {
            key->state = ADKEY_IDLE;
        }
        break;

    default:
        key->state = ADKEY_IDLE;
        break;
    }
}

// 根据按键ADC编码值修改状态
void bsp_adkey_scan(void)
{
    // 获取ADC编码值
    uint16_t encode = msp_adc_get(3);

    // 处理所有ADKEY
    for (uint8_t i = 0; i < ADKEY_LENGTH; i++)
    {
        scan_single_key(encode, &adkeys[i]);
    }
}