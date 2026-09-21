#ifndef BSP_DIGITAL_LEDS_H
#define BSP_DIGITAL_LEDS_H

#include "gd32f4xx.h"
#include "systick.h"

#define SEG_CLR 0xFF

#define SEG_0   0xC0
#define SEG_1   0xF9
#define SEG_2   0xA4
#define SEG_3   0xB0
#define SEG_4   0x99
#define SEG_5   0x92
#define SEG_6   0x82
#define SEG_7   0xF8
#define SEG_8   0x80
#define SEG_9   0x90

#define SEG_A   0x88
#define SEG_B   0x83
#define SEG_C   0xC6
#define SEG_D   0xA1
#define SEG_E   0x86
#define SEG_F   0x8E

#define SEG_R   0xCE
#define SEG_S   0x92
#define SEG_O   0x8C
#define SEG_N   0xAB
#define SEG_L   0xC7
#define SEG_H   0x89
#define SEG_G   0x82

// 边缘段码（共阳，0=亮）
#define SEG_EDGE_A 0xFE // 上边
#define SEG_EDGE_B 0xFD // 右上
#define SEG_EDGE_C 0xFB // 右下
#define SEG_EDGE_D 0xF7 // 下边
#define SEG_EDGE_E 0xEF // 左下
#define SEG_EDGE_F 0xDF // 左上

#define DIG_1      0x01
#define DIG_2      0x02
#define DIG_3      0x04
#define DIG_4      0x08
#define DIG_5      0x10
#define DIG_6      0x20
#define DIG_7      0x40
#define DIG_8      0x80

void bsp_digital_leds_init(void);

void bsp_digital_leds_send_byte(uint8_t dat);

void bsp_digital_leds_send_code(unsigned char segCode, unsigned char digCode);

void send_code_to_buffer(unsigned char segCode, unsigned char pos);

void clear_buffer(void);

void bsp_digital_leds_scan(void);

#endif // BSP_DIGITAL_LEDS_H