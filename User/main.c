#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
//============ 任务调度 ============
#include "FreeRTOS.h"
#include "task.h"
#include "taskmanager.h"
//============ 片上外设 ============
#include "msp_adc.h"
#include "msp_exti.h"
#include "msp_fwdgt.h"
#include "msp_iic.h"
#include "msp_rtc.h"
#include "msp_spi.h"
#include "msp_uart.h"
#include "msp_wwdgt.h"
//============ 片外外设 ============
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
//============  资 源  ============
#include "bmp.h"

// 应用任务：就是原来 main() 里的 while(1) 主循环
#define APP_TASK_NAME "app"
#define APP_TASK_STACK_SIZE 512U
#define APP_TASK_PRIORITY (tskIDLE_PRIORITY + 3)

// 任务句柄
static TaskHandle_t app_task_handle = NULL;
TaskHandle_t task1_handle;
TaskHandle_t task2_handle;
TaskHandle_t task_keys_scan_handle;

// 之前taskmanager的任务调度 TODO:一步步将任务移动到FreeRTOS的task中
static void app_task(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    while (1)
    {
        // 执行 task_switch() 置位的任务（与原裸机主循环一致）
        task_schedule();

        // 让出 CPU 给空闲/定时器任务，1ms 扫描仍由节拍钩子完成
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    // 释放任务资源 （好习惯）
    vTaskDelete(app_task_handle);
}

uint32_t n = 0;

// 临界区测试
void task1(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    for (uint32_t i = 0; i < 1000000; i++)
    {
        uint32_t j = n + 1;
        n = j;
    }
    printf("task1:%d\n", n);

    // 释放任务资源 （好习惯）
    vTaskDelete(task1_handle);
    // 也可以这么写 vTaskDelete(NULL); // 释放自己
}

void task2(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    for (uint32_t i = 0; i < 1000000; i++)
    {
        uint32_t j = n + 1;
        n = j;
    }
    printf("task2:%d\n", n);

    // 释放任务资源 （好习惯）
    vTaskDelete(task2_handle);
}

void task_keys_scan(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    while (1)
    {
        bsp_keys_scan();
        vTaskDelay(100);
    }

    vTaskDelete(NULL);
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

void on_key_press(KEY_NUM key)
{
    if (key == KEY1)
    {
        printf("suspend task1\n");
        vTaskSuspend(task1_handle);
    }
    else if (key == KEY2)
    {
        printf("resume task1\n");
        vTaskResume(task1_handle);
    }
}

//============ 主 函 数 ============
int main(void)
{
    // 配置整个工程优先级分组 抢占:0~15  响应:无
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    // 时钟倍频(全局配置 所有定时器都是168Mhz)
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
#ifndef SYS_SUPPORT_OS
    // 系统滴答时钟初始化
    systick_config();
#endif
    //============ 片上外设 ============
    // USART0和USART2初始化
    msp_uart_init();
    printf("\r\n[boot] SystemCoreClock = %u\r\n", (unsigned int)SystemCoreClock);
    // EXTI0 PA0 和 EXTI3 PC3初始化
    msp_exti_init();
    // RTC
    msp_rtc_init(HXTAL);
    printf("[init] rtc ok\r\n");
    // IIC
    msp_iic_init();
    // ADC
    msp_adc_init();
    printf("[init] adc ok\r\n");
    // SPI
    msp_spi_init();
    printf("[init] msp ok\r\n");

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
    printf("[init] oled ok\r\n");
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
    printf("[init] bsp ok\r\n");

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

    xTaskCreate((TaskFunction_t)task1, // 指向实现任务功能的函数指针
                (const char *)"task1", // 给任务起一个描述性的名字，主要用于调试和追踪（如在调试器中查看任务列表）
                128,                   // 任务分配的堆栈大小，单位是“字（word）” 32位MCU中，1word=4byte 4*64=256byte
                NULL,                  // 一个 void * 类型的值，会作为参数传递给任务函数的 pvParameters 形参 不能是局部变量
                2,                     // 决定任务被调度器执行的优先顺序,数值越大，优先级越高
                &task1_handle);        // 一个输出参数，xTaskCreate 会通过它返回所创建任务的句柄（Handle）

    xTaskCreate((TaskFunction_t)task2,
                (const char *)"task2",
                128,
                NULL,
                2,
                &task2_handle);

    xTaskCreate((TaskFunction_t)task_keys_scan,
                (const char *)"task_keys_scan",
                64,
                NULL,
                3,
                &task_keys_scan_handle);

    vTaskStartScheduler();

    // 只有堆内存不足、调度器启动失败时才会执行到这里
    while (1)
    {
    }
}
