#ifndef BSP_LEDS_H
#define BSP_LEDS_H

#include "gd32f4xx.h"
#include "gd32f4xx_rcu.h"

typedef enum {
    LED1 = 1,
    LED2,
    LED3,
    LED4,
    LED5,
    LED6,
    LED7,
    LED8
} LED_NUM;

// LED引脚参数结构体
typedef struct {
    rcu_periph_enum rcu;
    uint32_t port;
    uint32_t pin;
} LED_PARAM;

// 初始化LED灯
void bsp_leds_config();

// 打开所有的灯
void bsp_leds_open_all();

// 关闭所有的灯
void bsp_leds_close_all();

// 打开一盏灯
void bsp_leds_open(LED_NUM led);

// 关闭一盏灯
void bsp_leds_close(LED_NUM led);

#endif // BSP_LEDS_H