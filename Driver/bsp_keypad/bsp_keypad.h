#ifndef BSP_KEYPAD_H
#define BSP_KEYPAD_H

#include "gd32f4xx.h"

/* 键盘统一挂在 GPIOE */
#define KP_PORT GPIOE
#define KP_RCU  RCU_GPIOE

/* 行（输出低时选中某一行） */
#define KROW1_PIN GPIO_PIN_8  /* PE8  */
#define KROW2_PIN GPIO_PIN_9  /* PE9  */
#define KROW3_PIN GPIO_PIN_10 /* PE10 */
#define KROW4_PIN GPIO_PIN_11 /* PE11 */

/* 列（读回列电平判断哪一列被按下） */
#define KCOL1_PIN   GPIO_PIN_12 /* PE12 */
#define KCOL2_PIN   GPIO_PIN_13 /* PE13 */
#define KCOL3_PIN   GPIO_PIN_14 /* PE14 */
#define KCOL4_PIN   GPIO_PIN_15 /* PE15 */

#define KP_ALL_PINS (KROW1_PIN | KROW2_PIN | KROW3_PIN | KROW4_PIN | \
                     KCOL1_PIN | KCOL2_PIN | KCOL3_PIN | KCOL4_PIN)

/* ---- 行操作：开漏输出，写 0 拉低、写 1 高阻（由内部上拉拉高） ---- */
#define KROW1_L() gpio_bit_reset(KP_PORT, KROW1_PIN)
#define KROW2_L() gpio_bit_reset(KP_PORT, KROW2_PIN)
#define KROW3_L() gpio_bit_reset(KP_PORT, KROW3_PIN)
#define KROW4_L() gpio_bit_reset(KP_PORT, KROW4_PIN)
#define KROW1_H() gpio_bit_set(KP_PORT, KROW1_PIN)
#define KROW2_H() gpio_bit_set(KP_PORT, KROW2_PIN)
#define KROW3_H() gpio_bit_set(KP_PORT, KROW3_PIN)
#define KROW4_H() gpio_bit_set(KP_PORT, KROW4_PIN)

/* ---- 列操作 ---- */
#define KCOL1_L() gpio_bit_reset(KP_PORT, KCOL1_PIN)
#define KCOL2_L() gpio_bit_reset(KP_PORT, KCOL2_PIN)
#define KCOL3_L() gpio_bit_reset(KP_PORT, KCOL3_PIN)
#define KCOL4_L() gpio_bit_reset(KP_PORT, KCOL4_PIN)
#define KCOL1_H() gpio_bit_set(KP_PORT, KCOL1_PIN)
#define KCOL2_H() gpio_bit_set(KP_PORT, KCOL2_PIN)
#define KCOL3_H() gpio_bit_set(KP_PORT, KCOL3_PIN)
#define KCOL4_H() gpio_bit_set(KP_PORT, KCOL4_PIN)

/* ---- 读引脚电平 ---- */
#define KROW1_READ() gpio_input_bit_get(KP_PORT, KROW1_PIN)
#define KROW2_READ() gpio_input_bit_get(KP_PORT, KROW2_PIN)
#define KROW3_READ() gpio_input_bit_get(KP_PORT, KROW3_PIN)
#define KROW4_READ() gpio_input_bit_get(KP_PORT, KROW4_PIN)
#define KCOL1_READ() gpio_input_bit_get(KP_PORT, KCOL1_PIN)
#define KCOL2_READ() gpio_input_bit_get(KP_PORT, KCOL2_PIN)
#define KCOL3_READ() gpio_input_bit_get(KP_PORT, KCOL3_PIN)
#define KCOL4_READ() gpio_input_bit_get(KP_PORT, KCOL4_PIN)

#endif // BSP_KEYPAD_H