#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "msp_adc.h"
#include "msp_exti.h"
#include "msp_fwdgt.h"
#include "msp_iic.h"
#include "msp_rtc.h"
#include "msp_spi.h"
#include "msp_uart.h"
#include "msp_wwdgt.h"

#include "bsp_188_leds.h"
#include "bsp_adkey.h"
#include "bsp_battery_flows.h"
#include "bsp_buzzer.h"
#include "bsp_digital_leds.h"
#include "bsp_dot_matrix.h"
#include "bsp_ec12d.h"
#include "bsp_flash.h"
#include "bsp_iic_oled.h"
#include "bsp_keypad.h"
#include "bsp_keys.h"
#include "bsp_leds.h"
#include "bsp_ntc.h"
#include "bsp_pcf8563.h"
#include "bsp_spi_oled.h"
#include "bsp_touch_key.h"

#include "FreeRTOS.h"
#include "task.h"
#include "taskmanager.h"

#include "bmp.h"

/* 应用任务：就是原来 main() 里的 while(1) 主循环 */
#define APP_TASK_NAME       "app"
#define APP_TASK_STACK_SIZE 512U
#define APP_TASK_PRIORITY   (tskIDLE_PRIORITY + 3)

static TaskHandle_t app_task_handle = NULL;

static void app_task(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        /* 执行 task_switch() 置位的任务（与原裸机主循环一致） */
        task_schedule();

        /* 让出 CPU 给空闲/定时器任务，1ms 扫描仍由节拍钩子完成 */
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// 串口接收回调
void on_usart_recv(USART_HandleTypeDef *huart)
{
    huart->recv_buff[huart->recv_length] = '\0';
    if (huart->usartx == USART0)
    {
        printf("usart0_recv:%s\n", huart->recv_buff);
    }
}

//============ 主 函 数 ============
int main(void)
{
    // 配置整个工程优先级分组 抢占:0~3  响应:0~3
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
    // 时钟倍频(全局配置 所有定时器都是168Mhz)
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
#ifndef SYS_SUPPORT_OS
    // 系统滴答时钟初始化
    systick_config();
#endif
    //============ 片上外设 ============
    // USART0和USART2初始化
    msp_uart_init();
    // EXTI0 PA0 和 EXTI3 PC3初始化
    msp_exti_init();
    // RTC
    msp_rtc_init(HXTAL);
    // IIC
    msp_iic_init();
    // ADC
    msp_adc_init();
    // SPI
    msp_spi_init();

    //============ 片外外设 ============
    // LED灯组初始化
    bsp_leds_config();
    // 按键初始化
    bsp_keys_config();
    // 蜂鸣器初始化
    bsp_buzzer_init();
    // oled初始化
    bsp_iic_oled_init();
    bsp_spi_oled_init();
    // pcf8563初始化
    bsp_pcf8563_init();
    // flash
    bsp_flash_init();
    // NTC
    bsp_ntc_init();
    // 188LED
    bsp_188_leds_init();
    // 矩阵键盘初始化
    // bsp_keypad_init();
    // ADKEY
    bsp_adkey_init();
    // 旋转编码器
    bsp_ec12d_init();
    // 数码管初始化
    bsp_digital_leds_init();
    // 触摸按键
    bsp_touch_key_init();
    // 点阵屏
    bsp_dot_matrix_init();
    bsp_dot_matrix_clear();

    //============  APP  ============
    app_input_init();
    app_show_init();

    printf("============ start ============\n");
    printf("SystemCoreClock = %u\r\n", (unsigned int)SystemCoreClock);

    //============ 测 试 ============
    app_test_flash();
    app_test_spi_oled();
    app_test_iic_oled();
    app_test_digital_leds();
    app_test_pcf8563_set_time_alarm();

    // 独立看门狗
    // msp_fwdgt_config();
    // 窗口看门狗
    // msp_wwdgt_config(); // 前面的初始化会消耗时间

    // 创建应用任务（原 main 主循环）并启动 FreeRTOS 调度器
    xTaskCreate((TaskFunction_t)app_task,
                (const char *)APP_TASK_NAME,
                APP_TASK_STACK_SIZE,
                NULL,
                APP_TASK_PRIORITY,
                &app_task_handle);

    vTaskStartScheduler();

    // 只有堆内存不足、调度器启动失败时才会执行到这里
    while (1)
    {
    }
}
