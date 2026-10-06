#ifndef MSP_SPI_HARD_H
#define MSP_SPI_HARD_H

#include "gd32f4xx.h"

void msp_spi_hard_init(void);

void msp_spi_hard_write_byte(uint8_t byte);

uint8_t msp_spi_hard_read_byte(void);

#endif // MSP_SPI_HARD_H