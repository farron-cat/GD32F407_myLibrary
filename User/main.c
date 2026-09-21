#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_battery_flows.h"
#include "bsp_keys.h"
#include "bsp_leds.h"
#include "msp_exti.h"
#include "msp_uart.h"

// PA2 TIMER1 CH2

// TIMER1 全局4倍频  168000000hz <=> 1s
// 预分频器 1680 => 100000hz <=> 1s

// PWM 周期1ms 占空比20% 0.2ms
// 100000hz <=> 1s
// 100hz <=> 1ms
// 20hz <=> 0.2ms

#define PRESCALER 1680 - 1
#define PERIOD (SystemCoreClock / (PRESCALER + 1) / 1000) - 1
void Timer1_config(void)
{
    // 1.打开外设时钟
    rcu_periph_clock_enable(RCU_TIMER1);

    // 2.初始化定时器
    timer_parameter_struct timer_init_struct;
    timer_struct_para_init(&timer_init_struct);
    timer_init_struct.prescaler = PRESCALER; // 定时器时钟预分频
    timer_init_struct.period = PERIOD;       // 定时器周期

    timer_init(TIMER1, &timer_init_struct);

    // 3.配置PWM输出通道
    timer_oc_parameter_struct ocpara;
    timer_channel_output_struct_para_init(&ocpara);
    // 通道P
    ocpara.outputstate = (uint16_t)TIMER_CCX_ENABLE; // 打开通道输出
    ocpara.ocpolarity = TIMER_OC_POLARITY_HIGH;
    ocpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
    // 通道N
    ocpara.outputnstate = TIMER_CCXN_DISABLE;
    ocpara.ocnpolarity = TIMER_OCN_POLARITY_HIGH;
    ocpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER1, TIMER_CH_2, &ocpara);
    // 4.输出模式配置
    timer_channel_output_mode_config(TIMER1, TIMER_CH_2, TIMER_OC_MODE_PWM0);
    // 5.设置占空比
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_2, (PERIOD + 1) * 0.5);

    // 6.使能定时器
    timer_enable(TIMER1);
}

void PA2_GPIO_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_2);
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
    // TIMER5初始化
    Timer1_config();
    PA2_GPIO_config();

    //============ 片外外设 ============
    // LED灯组初始化
    bsp_leds_config();
    // 按键初始化
    bsp_keys_config();

    printf("============ start ============\n");

    while (1)
    {
        ;
    }
}