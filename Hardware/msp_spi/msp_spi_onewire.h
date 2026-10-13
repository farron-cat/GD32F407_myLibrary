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

// 帧复位：用 SPI 连续发 0x00 把数据线连续拉低 ≥300us
// 单总线器件靠这段低电平判定"一帧结束"并锁存(WS2812B >50us / V5 >280us)；
// 发数据前也建议调用一次，保证第 1 个数据位有干净的前导低电平
void msp_spi_onewire_reset(void);

#endif // MSP_SPI_ONEWIRE_H