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

uint8_t arr1[4] = {1, 2, 3, 4};
uint8_t arr2[4] = {0};

void DMA_m2m_config(void)
{
    // 1.开启外设时钟
    rcu_periph_clock_enable(RCU_DMA1); // 只有DMA1支持m2m
    // 2.配置DMA
    dma_single_data_parameter_struct init_struct;
    dma_single_data_para_struct_init(&init_struct);

    init_struct.direction = DMA_MEMORY_TO_MEMORY;
    init_struct.periph_addr = (uint32_t)arr1;
    init_struct.memory0_addr = (uint32_t)arr2;

    init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    init_struct.number = 4U;

    init_struct.periph_inc = DMA_PERIPH_INCREASE_ENABLE;
    init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
    init_struct.priority = DMA_PRIORITY_LOW;

    dma_single_data_mode_init(DMA1, DMA_CH0, &init_struct);

    // 配置中断
    nvic_irq_enable(DMA1_Channel0_IRQn, 0, 0);
    // 清理标志位
    dma_interrupt_flag_clear(DMA1, DMA_CH0, DMA_INT_FLAG_FTF);
    // 开启传输完成中断
    dma_interrupt_enable(DMA1, DMA_CH0, DMA_INT_FTF);

    // 3.启动DMA传输
    dma_channel_enable(DMA1, DMA_CH0);
}

void DMA1_Channel0_IRQHandler(void)
{
    if (dma_interrupt_flag_get(DMA1, DMA_CH0, DMA_INT_FLAG_FTF) == SET)
    {
        // 清标志位
        dma_interrupt_flag_clear(DMA1, DMA_CH0, DMA_INT_FLAG_FTF);

        for (uint8_t i = 0; i < sizeof(arr2) / sizeof(arr2[0]); i++)
        {
            printf("arr[%d]=%d\n", (int)i, (int)arr2[i]);
        }

        // 关闭DMA
        dma_channel_disable(DMA1, DMA_CH0);
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

    //============ 片外外设 ============
    // LED灯组初始化
    bsp_leds_config();
    // 按键初始化
    bsp_keys_config();
    // 蜂鸣器初始化
    bsp_buzzer_init();

    printf("============ start ============\n");
    printf("SystemCoreClock = %u\r\n", (unsigned int)SystemCoreClock);

    for (uint8_t i = 0; i < sizeof(arr2) / sizeof(arr2[0]); i++)
    {
        printf("arr[%d]=%d\n", (int)i, (int)arr2[i]);
    }

    // DMA搬运
    DMA_m2m_config();

    while (1)
    {
    }
}