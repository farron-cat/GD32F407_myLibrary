#ifndef BSP_BUZZER_H
#define BSP_BUZZER_H

#include "gd32f4xx.h"

// 初始化BUZZER
void bsp_buzzer_init(void);

// 播放特定频率声音 20~20000hz
void bsp_buzzer_play(uint16_t hz);

// 停止播放
void bsp_buzzer_stop(void);

#endif // BSP_BUZZER_H