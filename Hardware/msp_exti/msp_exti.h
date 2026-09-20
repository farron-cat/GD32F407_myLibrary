#ifndef MSP_EXTI_H
#define MSP_EXTI_H

#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#define USE_EXTI0 1
#define USE_EXTI3 1

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

void msp_exti_init();

#endif // MSP_EXTI_H