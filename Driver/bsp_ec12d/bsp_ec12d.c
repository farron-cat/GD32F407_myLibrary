#include "bsp_ec12d.h"
#include "systick.h"
#include <stdio.h>

// A PD11 EXTI11
// B PD13
// D PD15 EXTI15

#ifndef __weak
#define __weak __attribute__((weak)) // GCC / armclang
#endif

__weak void on_ec12d_clockwise()
{
    printf("clockwise\n");
}

__weak void on_ec12d_counter_clockwise()
{
    printf("counter clockwise\n");
}

__weak void on_ec12d_key_press()
{
    printf("key press\n");
}

// 初始化旋转编码器
void bsp_ec12d_init()
{
    // 配置旋转编码器引脚B
    rcu_periph_clock_enable(RCU_GPIOD);
    gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_13);
}

// 处理旋转编码器外部中断
uint64_t last_time = 0;
FlagStatus last_state_a = SET; // 默认高电平
#define GET_CUR_STATE_B gpio_input_bit_get(GPIOD, GPIO_PIN_13)
void on_ec12d_exti()
{
    // 旋钮消抖
    uint64_t cur_time = get_us_cnt();
    if (cur_time - last_time <= 30000) // 30ms
        return;
    last_time = cur_time;

    // 读取旋钮A和B的状态
    FlagStatus cur_state_a = gpio_input_bit_get(GPIOD, GPIO_PIN_11);
    if (cur_state_a == last_state_a)
        return;
    last_state_a = cur_state_a;
    // 判断A引脚上升沿和下降沿
    if (cur_state_a == RESET) // A 下降沿
    {
        // 判断B引脚电平高低
        if (SET == GET_CUR_STATE_B)
        {
            // B高电平 正转
            on_ec12d_clockwise();
        }
        else
        {
            // B低电平 反转
            on_ec12d_counter_clockwise();
        }
    }
    else // A 上升沿
    {
        // 判断B引脚电平高低
        if (SET == GET_CUR_STATE_B)
        {
            // B高电平 反转
            on_ec12d_counter_clockwise();
        }
        else
        {
            // B低电平 正转
            on_ec12d_clockwise();
        }
    }
}