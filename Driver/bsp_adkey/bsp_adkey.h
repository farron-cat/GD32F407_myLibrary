#ifndef BSP_ADKEY_H
#define BSP_ADKEY_H

#include "gd32f4xx.h"

// ADKEY状态枚举
typedef enum {
    ADKEY_IDLE = 0,      // 空闲
    ADKEY_PRESS_CONFORM, // 消抖确认
    ADKEY_PRESS,         // 按下
    ADKEY_LONG_PRESS     // 长按
} ADKEY_STATE;

#define ADKEY_PRESS_CONFORM_TIM 30000  // 消抖确认时间 30000us 30ms
#define ADKEY_LONG_PRESS_TIM    500000 // 长按时间 500000us 500ms

// 初始化ADKEY
void bsp_adkey_init(void);
// ADKEY扫描
void bsp_adkey_scan(void);

#endif // BSP_ADKEY_H