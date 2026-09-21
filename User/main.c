#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_battery_flows.h"
#include "bsp_keys.h"
#include "bsp_leds.h"
#include "msp_exti.h"
#include "msp_uart.h"

// TIMER5 全局4倍频  168000000hz <=> 1s
// 预分频器 16800 => 10000hz <=> 1s
#define PRESCALER 16800 - 1
#define PERIOD 10000 - 1
void Timer5_config(void)
{
    // 1.打开外设时钟
    rcu_periph_clock_enable(RCU_TIMER5);

    // 2.初始化定时器
    timer_parameter_struct timer_init_struct;
    timer_struct_para_init(&timer_init_struct);
    timer_init_struct.prescaler = PRESCALER;               // 定时器时钟预分频
    timer_init_struct.alignedmode = TIMER_COUNTER_EDGE;    // 定时器计数模式
    timer_init_struct.counterdirection = TIMER_COUNTER_UP; // 定时器计数方向
    timer_init_struct.period = PERIOD;                     // 定时器周期
    timer_init_struct.clockdivision = TIMER_CKDIV_DIV1;    // 定时器时钟分频
    timer_init_struct.repetitioncounter = 0U;              // 定时器重复计数器

    timer_init(TIMER5, &timer_init_struct);
    // 3.配置中断
    nvic_irq_enable(TIMER5_DAC_IRQn, 2, 2);
    timer_interrupt_flag_clear(TIMER5, TIMER_INT_UP);
    timer_interrupt_enable(TIMER5, TIMER_INT_UP);
    // 4.使能定时器
    timer_enable(TIMER5);
}

void TIMER5_DAC_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER5, TIMER_INT_UP) == SET)
    {
        timer_interrupt_flag_clear(TIMER5, TIMER_INT_UP);

        printf("TIMER5\r\n");
    }
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
    // Timer5_config();

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