#ifndef MSP_EXTI_H
#define MSP_EXTI_H

#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#define USE_EXTI0 1
#define USE_EXTI3 1
#define USE_EXTI5 1

#if USE_EXTI0

#define EXTI0_PORT_RCU  RCU_GPIOA
#define EXTI0_PORT      GPIOA
#define EXTI0_PIN       GPIO_PIN_0
#define EXTI0_PUPD      GPIO_PUPD_NONE
#define EXTI0_EXTI_PORT EXTI_SOURCE_GPIOA
#define EXTI0_EXTI_PIN  EXTI_SOURCE_PIN0
#define EXTI0_NUM       EXTI_0
#define EXTI0_TRIG_TYPE EXTI_TRIG_BOTH
#define EXTI0_IRQ       EXTI0_IRQn

#endif

#if USE_EXTI3

#define EXTI3_PORT_RCU  RCU_GPIOC
#define EXTI3_PORT      GPIOC
#define EXTI3_PIN       GPIO_PIN_3
#define EXTI3_PUPD      GPIO_PUPD_PULLUP
#define EXTI3_EXTI_PORT EXTI_SOURCE_GPIOC
#define EXTI3_EXTI_PIN  EXTI_SOURCE_PIN3
#define EXTI3_NUM       EXTI_3
#define EXTI3_TRIG_TYPE EXTI_TRIG_BOTH
#define EXTI3_IRQ       EXTI3_IRQn

#endif

#if USE_EXTI5

#define EXTI5_PORT_RCU  RCU_GPIOB
#define EXTI5_PORT      GPIOB
#define EXTI5_PIN       GPIO_PIN_5
#define EXTI5_PUPD      GPIO_PUPD_PULLUP
#define EXTI5_EXTI_PORT EXTI_SOURCE_GPIOB
#define EXTI5_EXTI_PIN  EXTI_SOURCE_PIN5
#define EXTI5_NUM       EXTI_5
#define EXTI5_TRIG_TYPE EXTI_TRIG_FALLING
#define EXTI5_IRQ       EXTI5_9_IRQn

#endif

// RTC(PCF8563) 中断标志：EXTI5 中断只置位，主循环清零并处理（避免软件 IIC 重入）
extern volatile uint8_t g_rtc_int_flag;

void msp_exti_init();

#endif // MSP_EXTI_H