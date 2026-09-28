#ifndef MSP_IIC_H
#define MSP_IIC_H

#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#define SCL_H   gpio_bit_set(GPIOB, GPIO_PIN_6)
#define SCL_L   gpio_bit_reset(GPIOB, GPIO_PIN_6)

#define SDA_H   gpio_bit_set(GPIOB, GPIO_PIN_7)
#define SDA_L   gpio_bit_reset(GPIOB, GPIO_PIN_7)

#define SDA_IN  gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_7)
#define SDA_OUT gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_7)
#define SDA_STA gpio_input_bit_get(GPIOB, GPIO_PIN_7)

// 100kbit/s 100000 <=> 1000000us
//  1 <=> 10us
//  SCL高低分别持续5us
#define IIC_DELAY delay_1us(2)

void msp_iic_init(void);

void msp_iic_write_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);

void msp_iic_read_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);

#endif // MSP_IIC_H