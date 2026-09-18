#ifndef BSP_BATTERY_FLOWS_H
#define BSP_BATTERY_FLOWS_H

#include "gd32f4xx.h"
#include "bsp_leds.h"
#include "systick.h"

// 状态机状态枚举
typedef enum {
    STATE_IDLE = 0,
    STATE_CHARGING,
    STATE_STOPING,
    STATE_SHOW
} BATTERY_STATE;

// 初始化充电流水灯
void bsp_battery_flow_init();

// 开始充电
void bsp_battery_flow_start();

// 增加电量
void bsp_battery_flow_update();

// 停止充电
void bsp_battery_flow_stop();

// 显示电量
void bsp_battery_flow_show();

// 状态机处理
void bsp_battery_flow_process();

#endif // BSP_BATTERY_FLOWS_H
