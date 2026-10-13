#ifndef MSP_SPI_ONEWIRE_H
#define MSP_SPI_ONEWIRE_H

#include "gd32f4xx.h"
#include <stdio.h>
#include <string.h>
#include "systick.h"

// 初始化
void msp_spi_onewire_init();

// 写入byte方法
void msp_spi_onewire_write_byte(uint8_t dat);

// 读取byte方法
uint8_t msp_spi_onewire_read_byte();

#endif // MSP_SPI_ONEWIRE_H