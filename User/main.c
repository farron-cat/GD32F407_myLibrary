#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_battery_flows.h"
#include "bsp_keys.h"
#include "msp_uart.h"

// 按键按下回调
void on_key_press(KEY_NUM key)
{
    if (key == KEY1)
    {
        bsp_battery_flow_start();
    }
    else if (key == KEY2)
    {
        bsp_battery_flow_update();
    }
    else if (key == KEY3)
    {
        bsp_battery_flow_stop();
    }
    else if (key == KEY4)
    {
        bsp_battery_flow_show();
    }
}

// // 按键松开回调
// void on_key_release(KEY_NUM key)
// {
//     bsp_leds_close(key + 1);
// }

int main(void)
{
    systick_config();
    USART0_config();
    bsp_keys_config();

    bsp_battery_flow_init();

    uint8_t cnt = 0U;

    while (1)
    {
        cnt++;
        // 20ms扫描一次按键
        bsp_keys_scan();
        if (cnt >= 25)
        {
            cnt = 0;
            // 500ms电池流水灯状态机处理一次
            bsp_battery_flow_process();
        }
        delay_1ms(20);

        if (bsp_keys_scan(KEY1) == KEY_EVENT_PRESSED)
        {
            printf("KEY1 pressed\r\n");
        }
    }
}
