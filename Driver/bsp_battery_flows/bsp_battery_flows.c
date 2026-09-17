#include "bsp_battery_flows.h"

static uint8_t flow_flag = 0;
static uint8_t battery = 0;
static BATTERY_STATE battery_state = STATE_IDLE;

// 初始化充电流水灯
void bsp_battery_flow_init()
{
    // 读取battery值
    battery = 1;
    flow_flag = battery;
}

// 开始充电
void bsp_battery_flow_start()
{
    while (1)
    {
        if (flow_flag > 0)
            bsp_leds_open(LED1);
        else
            bsp_leds_close(LED1);

        if (flow_flag > 1)
            bsp_leds_open(LED2);
        else
            bsp_leds_close(LED2);

        if (flow_flag > 2)
            bsp_leds_open(LED3);
        else
            bsp_leds_close(LED3);

        if (flow_flag > 3)
            bsp_leds_open(LED4);
        else
            bsp_leds_close(LED4);

        delay_1ms(1000);
        flow_flag++;
        if (flow_flag > 4)
            flow_flag = battery;
    }
}

void bsp_battery_flow_update()
{
    battery += 1;
    flow_flag = battery;
}

void bsp_battery_flow_stop()
{
    // 常量的灯闪烁三次  再熄灭
    for (uint8_t i = 0; i < 3; i++)
    {
        // 根据电量确定需要亮灯的个数
        if (battery > 0)
        {
            bsp_leds_open(LED1);
        }
        else
        {
            bsp_leds_close(LED1);
        }

        if (battery > 1)
        {
            bsp_leds_open(LED2);
        }
        else
        {
            bsp_leds_close(LED2);
        }

        if (battery > 2)
        {
            bsp_leds_open(LED3);
        }
        else
        {
            bsp_leds_close(LED3);
        }

        if (battery > 3)
        {
            bsp_leds_open(LED4);
        }
        else
        {
            bsp_leds_close(LED4);
        }

        delay_1ms(500);

        // 熄灭
        bsp_leds_close_all();
        delay_1ms(500);
    }
}

void bsp_battery_flow_process()
{

    switch (battery_state)
    {
    case STATE_IDLE:

        break;
    case STATE_CHARGING:

        break;
    case STATE_STOPING:

        break;
    }
}