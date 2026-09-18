#include "bsp_battery_flows.h"

static uint8_t flow_flag = 0;
static uint8_t battery = 0;
static BATTERY_STATE battery_state = STATE_IDLE;
static uint8_t stoping_cnt = 0;

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
    // 改变状态
    battery_state = STATE_CHARGING;
}

// 增加电量
void bsp_battery_flow_update()
{
    battery += 1;
    flow_flag = battery;
}

// 停止充电
void bsp_battery_flow_stop()
{
    // 改变状态
    battery_state = STATE_STOPING;
}

// 显示电量
void bsp_battery_flow_show()
{
    // 改变状态
    battery_state = STATE_SHOW;
}

// 状态机处理
// 每500ms调用一次
void bsp_battery_flow_process()
{

    switch (battery_state)
    {
    case STATE_IDLE:
        bsp_leds_close_all();
        break;
    case STATE_CHARGING:
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

        flow_flag++;
        if (flow_flag > 4)
            flow_flag = battery;
        break;
    case STATE_STOPING:
        // 常量的灯闪烁三次再熄灭
        stoping_cnt++;
        // stoping_cnt计数到6停止
        if (stoping_cnt >= 6)
        {
            stoping_cnt = 0;
            battery_state = STATE_IDLE;
        }
        // stoping_cnt偶数亮，奇数灭
        else if (stoping_cnt % 2 == 0)
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
        }
        else
        {
            // 熄灭
            bsp_leds_close_all();
        }

        break;
    case STATE_SHOW:
        if (stoping_cnt >= 6)
        {
            stoping_cnt = 0;
            battery_state = STATE_IDLE;
        }

        if (battery > 0)
            bsp_leds_open(LED1);
        else
            bsp_leds_close(LED1);

        if (battery > 1)
            bsp_leds_open(LED2);
        else
            bsp_leds_close(LED2);

        if (battery > 2)
            bsp_leds_open(LED3);
        else
            bsp_leds_close(LED3);

        if (battery > 3)
            bsp_leds_open(LED4);
        else
            bsp_leds_close(LED4);

        stoping_cnt++;
    }
}