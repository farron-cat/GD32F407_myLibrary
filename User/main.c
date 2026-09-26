#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_battery_flows.h"
#include "bsp_buzzer.h"
#include "bsp_keys.h"
#include "bsp_leds.h"
#include "msp_exti.h"
#include "msp_rtc.h"
#include "msp_uart.h"

Time time;
uint8_t dat[1024] = {0};

void on_usart_recv(USART_HandleTypeDef *huart)
{
    huart->recv_buff[huart->recv_length] = '\0';
    if (huart->usartx == USART0)
    {
        printf("usart0_recv:%s\n", huart->recv_buff);
        // 配置剩余参数
        dma_periph_address_config(DMA1, DMA_CH0, (uint32_t)huart->recv_buff);
        dma_transfer_number_config(DMA1, DMA_CH0, huart->recv_length);
        // 启动DMA传输
        dma_channel_enable(DMA1, DMA_CH0);
    }
}

void DMA_m2m_config(void)
{
    // 1.开启外设时钟
    rcu_periph_clock_enable(RCU_DMA1); // 只有DMA1支持m2m
    // 2.配置DMA
    dma_single_data_parameter_struct init_struct;
    dma_single_data_para_struct_init(&init_struct);

    init_struct.direction = DMA_MEMORY_TO_MEMORY;
    // init_struct.periph_addr = ？？;
    init_struct.memory0_addr = (uint32_t)dat;

    init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    // init_struct.number = ？？;

    init_struct.periph_inc = DMA_PERIPH_INCREASE_ENABLE;
    init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
    init_struct.priority = DMA_PRIORITY_LOW;

    dma_single_data_mode_init(DMA1, DMA_CH0, &init_struct);

    // 配置中断
    nvic_irq_enable(DMA1_Channel0_IRQn, 2, 2);
    // 清理标志位
    dma_interrupt_flag_clear(DMA1, DMA_CH0, DMA_INT_FLAG_FTF);
    // 开启传输完成中断
    dma_interrupt_enable(DMA1, DMA_CH0, DMA_INT_FTF);

    // 3.启动DMA传输
    // dma_channel_enable(DMA1, DMA_CH0); // 没有配置好还不能启动
}

void DMA1_Channel0_IRQHandler(void)
{
    if (dma_interrupt_flag_get(DMA1, DMA_CH0, DMA_INT_FLAG_FTF) == SET)
    {
        // 清标志位
        dma_interrupt_flag_clear(DMA1, DMA_CH0, DMA_INT_FLAG_FTF);

        printf("dat:%s\n", dat);

        // 关闭DMA
        dma_channel_disable(DMA1, DMA_CH0);
    }
}

void msp_fwdgt_config(void)
{
    // 打开时钟
    rcu_osci_off(RCU_IRC32K);
    delay_1ms(1);
    rcu_osci_on(RCU_IRC32K);

    if (rcu_osci_stab_wait(RCU_IRC32K) == ERROR)
    { // 等待稳定
        printf("turn_on_osci_error\n");
        return;
    }

    // 写使能
    fwdgt_write_enable();
    // 配置分频系数 32000 / 32 = 1000
    fwdgt_prescaler_value_config(FWDGT_PSC_DIV32);
    // 配置重装载值
    fwdgt_reload_value_config(100);
    // 启动看门狗
    fwdgt_counter_reload();
    fwdgt_enable();
}

void msp_wwdgt_config(void)
{
    // 打开时钟
    rcu_periph_clock_enable(RCU_WWDGT);
    // 42mhz
    // 42000000hz / 4096 = 10,253.90625hz
    // 10,253.90625hz / 1 = 10,253.90625ms
    // 10,253.90625hz <=> 1000000us
    // 数1个数 <=> 97.523us
    // 配置窗口看门狗属性
    // 最短喂狗时间   47*97.5us = 4582.5us
    // 最长喂狗时间   64*97.5us = 6241.5us
    wwdgt_config(0x7F, 0x50, WWDGT_CFG_PSC_DIV1);
    // 启动看门狗
    wwdgt_enable();
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
    // RTC
    msp_rtc_init(HXTAL);

    //============ 片外外设 ============
    // LED灯组初始化
    bsp_leds_config();
    // 按键初始化
    bsp_keys_config();
    // 蜂鸣器初始化
    bsp_buzzer_init();

    printf("============ start ============\n");
    printf("SystemCoreClock = %u\r\n", (unsigned int)SystemCoreClock);

    // DMA搬运
    DMA_m2m_config();

    time.year = 2026;
    time.month = 9;
    time.day = 24;
    time.week = 4;
    time.hour = 14;
    time.min = 59;
    time.sec = 55;
    msp_rtc_write(&time);

    time.day = 24;
    time.hour = 15;
    time.min = 0;
    time.sec = 0;
    // 闹钟配置
    msp_rtc_alarm_config(&time);

    // 独立看门狗
    // msp_fwdgt_config();
    // 窗口看门狗
    msp_wwdgt_config(); // 前面的初始化会消耗时间

    uint8_t cnt = 0;
    while (1)
    {
        delay_1ms(5);

        // 喂狗
        wwdgt_counter_update(0x7F);

        cnt++;
    }
}