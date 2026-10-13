#ifndef BSP_EC12D_H
#define BSP_EC12D_H

#include "gd32f4xx.h"

// 初始化旋转编码器
void bsp_ec12d_init(void);
// 处理旋转编码器外部中断
void on_ec12d_exti(void);

// 回调
extern void on_ec12d_clockwise(void);         // 顺时针
extern void on_ec12d_counter_clockwise(void); // 逆时针
extern void on_ec12d_key_press(void);         // 按键按下

#endif // BSP_EC12D_H