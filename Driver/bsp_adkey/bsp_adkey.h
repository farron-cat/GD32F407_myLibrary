#ifndef BSP_ADKEY_H
#define BSP_ADKEY_H

#include "gd32f4xx.h"

#define ADKEY_PRESS_CONFORM_TIM 30000  // 消抖确认时间 30000us 30ms
#define ADKEY_LONG_PRESS_TIM    500000 // 长按时间 500000us 500ms

// ADKEY状态枚举
typedef enum {
    ADKEY_IDLE = 0,      // 空闲
    ADKEY_PRESS_CONFORM, // 消抖确认
    ADKEY_PRESS,         // 按下
    ADKEY_LONG_PRESS,    // 长按
    ADKEY_LENGTH         // 长度 4
} ADKEY_STATE;

// ADKEY参数结构体
typedef struct {
    uint8_t id;            // 按键ID
    uint16_t press_encode; // 按下对应的ADC编码
    ADKEY_STATE state;     // 状态
    uint64_t pre_time;     // 上一次时间
} ADKEY_PARAM;

// 初始化ADKEY
void bsp_adkey_init(void);
// ADKEY扫描
void bsp_adkey_scan(void);

#endif // BSP_ADKEY_H