#include "bsp_adkey.h"
#include "msp_adc.h"
#include <stdio.h>
#include <stdlib.h>

#define KEY1_PRESS_ENCODE 945
#define KEY1_RELEASE_ENCODE 4052

#define KEY2_PRESS_ENCODE 2043
#define KEY2_RELEASE_ENCODE 4052

#define KEY3_PRESS_ENCODE 3135
#define KEY3_RELEASE_ENCODE 4052

ADKEY_STATE key1_state = ADKEY_IDLE;
uint64_t key1_pre_time = 0;

ADKEY_STATE key2_state = ADKEY_IDLE;
uint64_t key2_pre_time = 0;

ADKEY_STATE key3_state = ADKEY_IDLE;
uint64_t key3_pre_time = 0;

void bsp_adkey_init(void)
{
    // 在msp_adc中初始化了
}

// 根据按键ADC编码值修改状态
void bsp_adkey_scan(void)
{
    uint16_t encode = msp_adc_get(3);

    switch (key1_state)
    {
    case ADKEY_IDLE:
        // KEY1
        if (abs(encode - KEY1_PRESS_ENCODE) < 20)
        {
            key1_state = ADKEY_PRESS_CONFORM;
            key1_pre_time = get_us_cnt();
        }
        break;

    case ADKEY_PRESS_CONFORM:
        // KEY1
        // 时间超过确认时间后电压仍然接近（按下）
        if ((abs(encode - KEY1_PRESS_ENCODE) < 20) && ((get_us_cnt() - key1_pre_time) > ADKEY_PRESS_CONFORM_TIM))
        {
            // 确认按下
            key1_state = ADKEY_PRESS;
        }
        // 时间在确认时间内的电压不接近（松开）
        else if ((abs(encode - KEY1_PRESS_ENCODE) >= 20) && ((get_us_cnt() - key1_pre_time) <= ADKEY_PRESS_CONFORM_TIM))
        {
            // 是抖动
            key1_state = ADKEY_IDLE;
        }
        break;

    case ADKEY_PRESS:
        // KEY1
        // 时间在长按时间内的电压不接近（松开）
        if ((abs(encode - KEY1_PRESS_ENCODE) >= 20) && ((get_us_cnt() - key1_pre_time) <= ADKEY_LONG_PRESS_TIM))
        {
            // 确认单击
            key1_state = ADKEY_IDLE;
            printf("KEY1_CLICK\n");
        }
        // 时间超过长按时间后电压仍然接近（按下）
        else if ((abs(encode - KEY1_PRESS_ENCODE) < 20) && ((get_us_cnt() - key1_pre_time) > ADKEY_LONG_PRESS_TIM))
        {
            // 确认长按
            key1_state = ADKEY_LONG_PRESS;
            printf("KEY1_LONG_PRESS\n");
        }
        break;

    case ADKEY_LONG_PRESS:
        // KEY1
        // 电压不接近（松开）
        if (abs(encode - KEY1_PRESS_ENCODE) >= 20)
        {
            key1_state = ADKEY_IDLE;
        }
        break;

    default:
        key1_state = ADKEY_IDLE;
        break;
    }

    switch (key2_state)
    {
    case ADKEY_IDLE:
        // KEY2
        if (abs(encode - KEY2_PRESS_ENCODE) < 20)
        {
            key2_state = ADKEY_PRESS_CONFORM;
            key2_pre_time = get_us_cnt();
        }
        break;

    case ADKEY_PRESS_CONFORM:
        // KEY2
        // 时间超过确认时间后电压仍然接近（按下）
        if ((abs(encode - KEY2_PRESS_ENCODE) < 20) && ((get_us_cnt() - key2_pre_time) > ADKEY_PRESS_CONFORM_TIM))
        {
            // 确认按下
            key2_state = ADKEY_PRESS;
        }
        // 时间在确认时间内的电压不接近（松开）
        else if ((abs(encode - KEY2_PRESS_ENCODE) >= 20) && ((get_us_cnt() - key2_pre_time) <= ADKEY_PRESS_CONFORM_TIM))
        {
            // 是抖动
            key2_state = ADKEY_IDLE;
        }
        break;

    case ADKEY_PRESS:
        // KEY2
        // 时间在长按时间内的电压不接近（松开）
        if ((abs(encode - KEY2_PRESS_ENCODE) >= 20) && ((get_us_cnt() - key2_pre_time) <= ADKEY_LONG_PRESS_TIM))
        {
            // 确认单击
            key2_state = ADKEY_IDLE;
            printf("KEY2_CLICK\n");
        }
        // 时间超过长按时间后电压仍然接近（按下）
        else if ((abs(encode - KEY2_PRESS_ENCODE) < 20) && ((get_us_cnt() - key2_pre_time) > ADKEY_LONG_PRESS_TIM))
        {
            // 确认长按
            key2_state = ADKEY_LONG_PRESS;
            printf("KEY2_LONG_PRESS\n");
        }
        break;

    case ADKEY_LONG_PRESS:
        // KEY2
        // 电压不接近（松开）
        if (abs(encode - KEY2_PRESS_ENCODE) >= 20)
        {
            key2_state = ADKEY_IDLE;
        }
        break;

    default:
        key2_state = ADKEY_IDLE;
        break;
    }

    switch (key3_state)
    {
    case ADKEY_IDLE:
        // KEY3
        if (abs(encode - KEY3_PRESS_ENCODE) < 20)
        {
            key3_state = ADKEY_PRESS_CONFORM;
            key3_pre_time = get_us_cnt();
        }
        break;

    case ADKEY_PRESS_CONFORM:

        // KEY3
        // 时间超过确认时间后电压仍然接近（按下）
        if ((abs(encode - KEY3_PRESS_ENCODE) < 20) && ((get_us_cnt() - key3_pre_time) > ADKEY_PRESS_CONFORM_TIM))
        {
            // 确认按下
            key3_state = ADKEY_PRESS;
        }
        // 时间在确认时间内的电压不接近（松开）
        else if ((abs(encode - KEY3_PRESS_ENCODE) >= 20) && ((get_us_cnt() - key3_pre_time) <= ADKEY_PRESS_CONFORM_TIM))
        {
            // 是抖动
            key3_state = ADKEY_IDLE;
        }
        break;

    case ADKEY_PRESS:
        // KEY3
        // 时间在长按时间内的电压不接近（松开）
        if ((abs(encode - KEY3_PRESS_ENCODE) >= 20) && ((get_us_cnt() - key3_pre_time) <= ADKEY_LONG_PRESS_TIM))
        {
            // 确认单击
            key3_state = ADKEY_IDLE;
            printf("KEY3_CLICK\n");
        }
        // 时间超过长按时间后电压仍然接近（按下）
        else if ((abs(encode - KEY3_PRESS_ENCODE) < 20) && ((get_us_cnt() - key3_pre_time) > ADKEY_LONG_PRESS_TIM))
        {
            // 确认长按
            key3_state = ADKEY_LONG_PRESS;
            printf("KEY3_LONG_PRESS\n");
        }
        break;

    case ADKEY_LONG_PRESS:
        // KEY3
        // 电压不接近（松开）
        if (abs(encode - KEY3_PRESS_ENCODE) >= 20)
        {
            key3_state = ADKEY_IDLE;
        }
        break;

    default:
        key3_state = ADKEY_IDLE;
        break;
    }
}