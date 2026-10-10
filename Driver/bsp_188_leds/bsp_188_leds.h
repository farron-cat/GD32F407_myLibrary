#ifndef BSP_188_LEDS_H
#define BSP_188_LEDS_H

#include "gd32f4xx.h"

// 引脚定义宏
#define PIN1_RCU  RCU_GPIOE
#define PIN1_PORT GPIOE
#define PIN1_PIN  GPIO_PIN_15

#define PIN2_RCU  RCU_GPIOE
#define PIN2_PORT GPIOE
#define PIN2_PIN  GPIO_PIN_13

#define PIN3_RCU  RCU_GPIOE
#define PIN3_PORT GPIOE
#define PIN3_PIN  GPIO_PIN_11

#define PIN4_RCU  RCU_GPIOE
#define PIN4_PORT GPIOE
#define PIN4_PIN  GPIO_PIN_9

#define PIN5_RCU  RCU_GPIOE
#define PIN5_PORT GPIOE
#define PIN5_PIN  GPIO_PIN_7

// 引脚参数结构体
typedef struct {
    rcu_periph_enum rcu;
    uint32_t port;
    uint32_t gpio_pin;
} LED188_PIN_PARAM;

// 引脚索引结构体
typedef enum {
    PIN1 = 0,
    PIN2,
    PIN3,
    PIN4,
    PIN5,
    LED188_PIN_NUM // 5
} LED188_PIN_INDEX;

// 初始化188数码管
void bsp_188_leds_init();

void bsp_188_leds_show();
void bsp_188_leds_scan();
void bsp_188_leds_set_num(uint8_t num);
void bsp_188_leds_clear();

#endif // BSP_188_LEDS_H