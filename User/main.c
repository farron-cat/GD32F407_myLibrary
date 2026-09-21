#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_battery_flows.h"
#include "bsp_buzzer.h"
#include "bsp_keys.h"
#include "bsp_leds.h"
#include "msp_exti.h"
#include "msp_uart.h"

// PA02 TIMER1 CH2
// PD12 TIMER3 CH0
// PD13 TIMER3 CH1
// PD14 TIMER3 CH2
// PD15 TIMER3 CH3

// TIMER1 全局4倍频  168000000hz <=> 1s
// 预分频器 1680 => 100000hz <=> 1s

// PWM 周期1ms 占空比20% 0.2ms
// 100000hz <=> 1s
// 100hz <=> 1ms
// 20hz <=> 0.2ms

#define PRESCALER 1680 - 1
#define PERIOD (SystemCoreClock / (PRESCALER + 1) / 1000) - 1

void Timer3_config(void)
{
    // 1.打开外设时钟
    rcu_periph_clock_enable(RCU_TIMER3);

    // 2.初始化定时器
    timer_parameter_struct timer_init_struct;
    timer_struct_para_init(&timer_init_struct);
    timer_init_struct.prescaler = PRESCALER; // 定时器时钟预分频
    timer_init_struct.period = PERIOD;       // 定时器周期

    timer_init(TIMER3, &timer_init_struct);

    // 3.配置PWM输出通道
    timer_oc_parameter_struct ocpara;
    timer_channel_output_struct_para_init(&ocpara);
    ocpara.outputstate = (uint16_t)TIMER_CCX_ENABLE; // 打开通道输出
    timer_channel_output_config(TIMER3, TIMER_CH_0, &ocpara);
    timer_channel_output_config(TIMER3, TIMER_CH_3, &ocpara);

    // 4.输出模式配置
    timer_channel_output_mode_config(TIMER3, TIMER_CH_0, TIMER_OC_MODE_PWM0);
    // 5.设置占空比
    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_0, (PERIOD + 1) * 0.5);
    // 6.使能定时器
    timer_enable(TIMER3);
}

void PD12_GPIO_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOD);
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_af_set(GPIOD, GPIO_AF_2, GPIO_PIN_12);
}

int main(void)
{
    // 配置整个工程优先级分组 抢占:0~3  响应:0~3
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
    // 时钟倍频(全局配置 所有定时器都是168Mhz)
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
    // 系统滴答时钟初始化
    systick_config();

    //============ 片上外设 ============
    // USART0和USART2初始化
    msp_uart_init();
    // EXTI0 PA0 和 EXTI3 PC3初始化
    msp_exti_init();

    //============ 片外外设 ============
    // LED灯组初始化
    bsp_leds_config();
    PD12_GPIO_config();
    Timer3_config();
    // 按键初始化
    bsp_keys_config();
    // 蜂鸣器初始化
    bsp_buzzer_init();

    printf("============ start ============\n");

    int duty_percent = 100; // 当前占空比，单位 %
    int step = -1;          // 每次变化 1%

    // 蜂鸣器测试
    delay_1ms(1000);
    bsp_buzzer_play(500);
    delay_1ms(1000);
    bsp_buzzer_play(1000);

    // 停止
    delay_1ms(1000);
    bsp_buzzer_stop();
    delay_1ms(1000);
    bsp_buzzer_play(2000);
    delay_1ms(1000);
    bsp_buzzer_stop();

    while (1)
    {
        // 计算比较值：CCR = (ARR+1) * duty% / 100
        uint16_t ccr = (uint16_t)(((uint32_t)(PERIOD + 1) * duty_percent) / 100);
        timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_0, ccr);
        timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_3, ccr);

        delay_1ms(10); // 每 10ms 变一次，90 步约 0.9 秒一个来回

        duty_percent += step;

        if (duty_percent <= 10)
        {
            duty_percent = 10;
            step = 1; // 到达 10% 后反向增加
        }
        else if (duty_percent >= 100)
        {
            duty_percent = 100;
            step = -1; // 到达 100% 后反向减少
        }
    }
}