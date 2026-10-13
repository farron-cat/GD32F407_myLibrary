#ifndef BSP_WS2812_RGB_LEDS_H
#define BSP_WS2812_RGB_LEDS_H

#include "gd32f4xx.h"

void bsp_ws2812_rgb_leds_init(void);

// 设置第 index 颗灯的颜色(0~3)，rgb 形如 0xFF0000 = 红 / 0x00FF00 = 绿 / 0x0000FF = 蓝
// 只改内存数据，改完要调用 bsp_ws2812_rgb_leds_send() 才会显示
void bsp_ws2812_rgb_leds_set_color(uint8_t index, uint32_t rgb);

void bsp_ws2812_rgb_leds_send(void);

#endif // BSP_WS2812_RGB_LEDS_H