#ifndef MSP_IIC_HARD_H
#define MSP_IIC_HARD_H

#include "gd32f4xx.h"

void msp_iic_hard_init(void);
uint8_t msp_iic_hard_write_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
uint8_t msp_iic_hard_read_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);

#endif // MSP_IIC_HARD_H