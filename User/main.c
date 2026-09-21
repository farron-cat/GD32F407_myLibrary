#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_battery_flows.h"
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

void BUZZER_GPIO_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_9);
}

void Timer5_config(void)
{
    // 1.打开外设时钟
    rcu_periph_clock_enable(RCU_TIMER5);

    // 2.初始化定时器
    timer_parameter_struct timer_init_struct;
    timer_struct_para_init(&timer_init_struct);
    timer_init_struct.prescaler = 1680 - 1;                // 定时器时钟预分频
    timer_init_struct.alignedmode = TIMER_COUNTER_EDGE;    // 定时器计数模式
    timer_init_struct.counterdirection = TIMER_COUNTER_UP; // 定时器计数方向
    timer_init_struct.period = 100 - 1;                    // 定时器周期
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

        gpio_bit_toggle(GPIOB, GPIO_PIN_9);
        printf("TIMER5\r\n");
    }
}

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

    // 4.输出模式配置
    timer_channel_output_mode_config(TIMER3, TIMER_CH_0, TIMER_OC_MODE_PWM0);
    // 5.设置占空比
    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_0, (PERIOD + 1) * 0.5);
    // 6.使能定时器
    timer_enable(TIMER3);
}

void PA2_GPIO_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_2);
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
    // TIMER1初始化
    Timer1_config();
    PA2_GPIO_config();

    Timer5_config();

    //============ 片外外设 ============
    // LED灯组初始化
    bsp_leds_config();
    Timer3_config();
    PD12_GPIO_config();

    // BUZZER_GPIO_config();
    // 按键初始化
    bsp_keys_config();

    printf("============ start ============\n");

    int duty_percent = 100; // 当前占空比，单位 %
    int step = -1;          // 每次变化 1%

    while (1)
    {
        // 计算比较值：CCR = (ARR+1) * duty% / 100
        uint16_t ccr = (uint16_t)(((uint32_t)(PERIOD + 1) * duty_percent) / 100);
        timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_0, ccr);

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