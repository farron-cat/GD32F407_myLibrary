#ifndef BSP_DOT_MATRIX_H
#define BSP_DOT_MATRIX_H

#include "gd32f4xx.h"

extern unsigned char num_display_map[12][8];

void bsp_dot_matrix_init(void);

void bsp_dot_matrix_clear(void);

void bsp_dot_matrix_display(uint8_t *show);

void bsp_dot_matrix_display_num(uint8_t num);

void bsp_dot_matrix_test_tick(void);

#endif // BSP_DOT_MATRIX_H