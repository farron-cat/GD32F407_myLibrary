#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_battery_flows.h"
#include "bsp_keys.h"
#include "bsp_leds.h"
#include "msp_uart.h"

// 按键按下回调
void on_key_press(KEY_NUM key)
{
    if (key == KEY1)
    {
        bsp_battery_flow_start();
        printf("KEY1 pressed\r\n");
    }
    else if (key == KEY2)
    {
        bsp_battery_flow_update();
        printf("KEY2 pressed\r\n");
    }
    else if (key == KEY3)
    {
        bsp_battery_flow_stop();
        printf("KEY3 pressed\r\n");
    }
    else if (key == KEY4)
    {
        bsp_battery_flow_show();
        printf("KEY4 pressed\r\n");
    }
}

// // 按键松开回调
// void on_key_release(KEY_NUM key)
// {
//     bsp_leds_close(key + 1);
// }

void GPIO_config(void)
{
    // 1.配置引脚GPIO模式
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_0);
}

void EXTI0_config(void)
{
    // 2.选择EXTI检测线路连接引脚(SYSCFG)
    rcu_periph_clock_enable(RCU_SYSCFG);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN0);
    // 3. 配置EXTI检测极性(上升沿/下降沿/BOTH触发)
    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    // 4. 告诉NVIC要处理EXTI中断(NVIC中注册)
    nvic_irq_enable(EXTI0_IRQn, 2, 2);
    // 5. 清除中断标志位
    exti_interrupt_flag_clear(EXTI_0);
    // 打开EXTI中断
    exti_interrupt_enable(EXTI_0);
}

void EXTI_PA0_config(void)
{
    GPIO_config();
    EXTI0_config();
}

int main(void)
{
    systick_config();
    USART0_config();
    USART2_config();

    // bsp_leds_config();
    // bsp_keys_config();

    // bsp_battery_flow_init();

    EXTI_PA0_config();

    uint8_t cnt = 0U;

    while (1)
    {
        // cnt++;
        // // 20ms扫描一次按键
        // bsp_keys_scan();
        // if (cnt >= 25)
        // {
        //     cnt = 0;
        //     // 500ms电池流水灯状态机处理一次
        //     bsp_battery_flow_process();
        // }
        // delay_1ms(20);
    }
}

// 6.实现中断处理函数
void EXTI0_IRQHandler(void)
{
    // 中断处理函数中的套路：先判断中断标志位，再清除中断标志位
    if (exti_interrupt_flag_get(EXTI_0) == SET)
    {
        exti_interrupt_flag_clear(EXTI_0);

        // 做点事
        // printf("EXTI0_IRQHandler\r\n");
        // 通过读取PA0引脚电平，判断是上升沿还是下降沿
        if (gpio_input_bit_get(GPIOA, GPIO_PIN_0) == SET)
        {
            // 上升沿
            printf("EXTI0_IRQHandler: rising edge\r\n");
        }
        else
        {
            // 下降沿
            printf("EXTI0_IRQHandler: falling edge\r\n");
        }
    }
}