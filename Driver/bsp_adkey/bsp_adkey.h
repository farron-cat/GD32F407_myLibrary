#ifndef BSP_ADKEY_H
#define BSP_ADKEY_H

#include "gd32f4xx.h"

#define ADKEY_PRESS_CONFORM_TIM 30000  // 消抖确认时间 30000us 30ms
#define ADKEY_LONG_PRESS_TIM    500000 // 长按时间 500000us 500ms
#define ADKEY_DOUBLE_WAIT_TIM   300000 // 双击等待时间 300000us 300ms

// ADKEY状态枚举
typedef enum {
    ADKEY_IDLE = 0,             // 空闲
    ADKEY_PRESS_CONFORM,        // 消抖确认
    ADKEY_PRESS,                // 按下
    ADKEY_LONG_PRESS,           // 长按
    ADKEY_DOUBLE_WAIT,          // 等待第二次按下
    ADKEY_DOUBLE_PRESS_CONFORM, // 双击确认
    ADKEY_DOUBLE_PRESS,         // 双击
    ADKEY_LENGTH                // 长度 7
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

// 单击回调函数
extern void on_adkey_click(uint8_t id);
// 长按回调函数
extern void on_adkey_long_press(uint8_t id);

#endif // BSP_ADKEY_H