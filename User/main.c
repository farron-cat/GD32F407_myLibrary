#include "main.h"
#include "app.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
//============ 任务调度 ============
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "taskmanager.h"
#include "timers.h"
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

//============  配置宏  ============
#define STACK_MONITOR

//============  任 务  ============

#define START_TASK_NAME "start_task"
#define START_TASK_STACK_SIZE 512U
#define START_TASK_PRIORITY (tskIDLE_PRIORITY + 3)

// 信号量句柄
SemaphoreHandle_t iic_bus_mutex_semaphore_handle; // iic总线互斥信号量
SemaphoreHandle_t mutex_Semaphore_Handle;

// 任务句柄
TaskHandle_t app_task_input_touch_key_handle;
TaskHandle_t app_task_show_digital_leds_handle;
TaskHandle_t app_task_show_dot_matrix_handle;
TaskHandle_t app_task_show_188_leds_handle;
TaskHandle_t app_task_show_iic_oled_handle;
TaskHandle_t app_task_show_pcf8563_handle;
TaskHandle_t task_keys_scan_handle;

TaskHandle_t task1_handle;
TaskHandle_t task2_handle;
TaskHandle_t task3_handle;

//=========== 系统初始化 ===========
void system_init(void)
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
    printf("[heap] free=%u B, minimum ever free=%u B\r\n",
           (unsigned)xPortGetFreeHeapSize(),
           (unsigned)xPortGetMinimumEverFreeHeapSize());

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
}

//============ 启动任务 ============
static void start_task(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    system_init();

    iic_bus_mutex_semaphore_handle = xSemaphoreCreateMutex();

    xTaskCreate((TaskFunction_t)app_task_input_touch_key,
                (const char *)"app_task_input_touch_key",
                128,
                NULL,
                3,
                &app_task_input_touch_key_handle);

    xTaskCreate((TaskFunction_t)app_task_show_digital_leds,
                (const char *)"app_task_show_digital_leds",
                128,
                NULL,
                2,
                &app_task_show_digital_leds_handle);

    xTaskCreate((TaskFunction_t)app_task_show_dot_matrix,
                (const char *)"app_task_show_dot_matrix",
                128,
                NULL,
                2,
                &app_task_show_dot_matrix_handle);

    xTaskCreate((TaskFunction_t)app_task_show_188_leds_task,
                (const char *)"app_task_show_188_leds_task",
                128,
                NULL,
                2,
                &app_task_show_188_leds_handle);

    xTaskCreate((TaskFunction_t)app_task_show_iic_oled_task,
                (const char *)"app_task_show_iic_oled_task",
                128,
                NULL,
                2,
                &app_task_show_iic_oled_handle);

    xTaskCreate((TaskFunction_t)app_task_show_pcf8563_task,
                (const char *)"app_task_show_pcf8563_task",
                128,
                NULL,
                2,
                &app_task_show_pcf8563_handle);

    xTaskCreate((TaskFunction_t)app_task_keys_scan,
                (const char *)"app_task_keys_scan",
                128,
                NULL,
                3,
                &task_keys_scan_handle);

#ifdef STACK_MONITOR
    xTaskCreate((TaskFunction_t)task_stack_monitor,
                (const char *)"task_stack_monitor",
                128,
                NULL,
                1,
                NULL);
#endif

#ifdef TEST
    // 测试实验区域
    {
        // 创建信号量 互斥信号量，默认有1个
        mutex_Semaphore_Handle = xSemaphoreCreateMutex();

        xTaskCreate((TaskFunction_t)task1, // 指向实现任务功能的函数指针
                    (const char *)"task1", // 给任务起一个描述性的名字，主要用于调试和追踪（如在调试器中查看任务列表）
                    128,                   // 任务分配的堆栈大小，单位是“字（word）” 32位MCU中，1word=4byte 4*64=256byte
                    NULL,                  // 一个 void * 类型的值，会作为参数传递给任务函数的 pvParameters 形参 不能是局部变量
                    1,                     // 决定任务被调度器执行的优先顺序,数值越大，优先级越高
                    &task1_handle);        // 一个输出参数，xTaskCreate 会通过它返回所创建任务的句柄（Handle）

        xTaskCreate((TaskFunction_t)task2,
                    (const char *)"task2",
                    128,
                    NULL,
                    1,
                    &task2_handle);

        xTaskCreate((TaskFunction_t)task3,
                    (const char *)"task3",
                    128,
                    NULL,
                    1,
                    &task3_handle);
    }
#endif

    vTaskDelete(NULL); // 任务调度开始后，删除 start_task 任务自身
}

//============ 任务函数 ============
void app_task_input_touch_key(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    while (1)
    {
        xSemaphoreTake(iic_bus_mutex_semaphore_handle, portMAX_DELAY);
        app_input_touch_key_task();
        xSemaphoreGive(iic_bus_mutex_semaphore_handle);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

void app_task_show_digital_leds(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    while (1)
    {
        app_show_digital_leds_task();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

void app_task_show_dot_matrix(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    while (1)
    {
        app_show_dot_matrix_task();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

void app_task_show_188_leds_task(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    while (1)
    {
        app_show_188_leds_task();
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    vTaskDelete(NULL);
}

void app_task_show_iic_oled_task(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    while (1)
    {
        xSemaphoreTake(iic_bus_mutex_semaphore_handle, portMAX_DELAY);
        app_show_iic_oled_task();
        xSemaphoreGive(iic_bus_mutex_semaphore_handle);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelete(NULL);
}

void app_task_show_pcf8563_task(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    while (1)
    {
        xSemaphoreTake(iic_bus_mutex_semaphore_handle, portMAX_DELAY);
        app_show_pcf8563_task();
        xSemaphoreGive(iic_bus_mutex_semaphore_handle);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);
}

void app_task_keys_scan(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    while (1)
    {
        bsp_keys_scan();
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelete(NULL);
}

#ifdef STACK_MONITOR
//============ 监控任务 ============
void task_stack_monitor(void *pvParameters)
{
    while (1)
    {
        printf("--- Stack Watermarks ---\r\n");
        printf("keys_scan      : %u\r\n", uxTaskGetStackHighWaterMark(task_keys_scan_handle));
        printf("touch_key      : %u\r\n", uxTaskGetStackHighWaterMark(app_task_input_touch_key_handle));
        printf("digital_leds   : %u\r\n", uxTaskGetStackHighWaterMark(app_task_show_digital_leds_handle));
        printf("dot_matrix     : %u\r\n", uxTaskGetStackHighWaterMark(app_task_show_dot_matrix_handle));
        printf("188_leds       : %u\r\n", uxTaskGetStackHighWaterMark(app_task_show_188_leds_handle));
        printf("iic_oled       : %u\r\n", uxTaskGetStackHighWaterMark(app_task_show_iic_oled_handle));
        printf("pcf8563        : %u\r\n", uxTaskGetStackHighWaterMark(app_task_show_pcf8563_handle));
        vTaskDelay(pdMS_TO_TICKS(5000)); // 每 5 秒打印一次
    }
}
#endif

#ifdef TEST
//============ 测试任务 ============
void task1(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    while (1)
    {
        // 拿取信号量，没有信号一直等待
        xSemaphoreTake(mutex_Semaphore_Handle, portMAX_DELAY);
        printf("1");
        // 给予信号量, 谁用了信号量，谁释放
        xSemaphoreGive(mutex_Semaphore_Handle);
        vTaskDelay(pdMS_TO_TICKS(300)); // 让出 CPU 给其他任务，与delay_1ms忙等不同
    }

    // 释放任务资源 （好习惯）
    vTaskDelete(task1_handle);
    // 也可以这么写 vTaskDelete(NULL); // 释放自己
}

void task2(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    while (1)
    {
        // 拿取信号量，没有信号一直等待
        xSemaphoreTake(mutex_Semaphore_Handle, portMAX_DELAY);
        printf("2");
        // 给予信号量, 谁用了信号量，谁释放
        xSemaphoreGive(mutex_Semaphore_Handle);
        vTaskDelay(pdMS_TO_TICKS(500)); // 让出 CPU 给其他任务，与delay_1ms忙等不同
    }

    // 释放任务资源 （好习惯）
    vTaskDelete(task2_handle);
}

void task3(void *pvParameters)
{
    (void)pvParameters; // 消除编译器的"未使用参数"警告

    while (1)
    {
        printf("3");
        vTaskDelay(pdMS_TO_TICKS(1000)); // 让出 CPU 给其他任务，与delay_1ms忙等不同
    }

    // 释放任务资源 （好习惯）
    vTaskDelete(task3_handle);
}
#endif

//============ 回调函数 ============

// 串口接收回调
void on_usart_recv(USART_HandleTypeDef *huart)
{
    huart->recv_buff[huart->recv_length] = '\0';
    if (huart->usartx == USART0)
    {
        // printf("usart0_recv:%s\n", huart->recv_buff);
        send_string(huart, huart->recv_buff);
    }
}

// 独立按键回调
void on_key_press(KEY_NUM key)
{
    if (key == KEY1)
    {
    }
    else if (key == KEY2)
    {
    }
    else if (key == KEY3)
    {
    }
    else if (key == KEY4)
    {
    }
}

//============ 主 函 数 ============
int main(void)
{
    xTaskCreate((TaskFunction_t)start_task,
                (const char *)START_TASK_NAME,
                START_TASK_STACK_SIZE,
                NULL,
                START_TASK_PRIORITY,
                NULL);

    // 开始调度
    vTaskStartScheduler();

    // 只有堆内存不足、调度器启动失败时才会执行到这里
    while (1)
    {
        printf("heap full!\r\n");
    }
}
