#ifndef MSP_SPI_H
#define MSP_SPI_H

#include "gd32f4xx.h"

// 软件SPI
// SCL PA5
#define SPI_SCL_L gpio_bit_reset(GPIOA, GPIO_PIN_5)
#define SPI_SCL_H gpio_bit_set(GPIOA, GPIO_PIN_5)

// MOSI PA7
#define SPI_MOSI_L gpio_bit_reset(GPIOA, GPIO_PIN_7)
#define SPI_MOSI_H gpio_bit_set(GPIOA, GPIO_PIN_7)

// MISO PA6
#define SPI_MISO_READ gpio_input_bit_get(GPIOA, GPIO_PIN_6)

// 初始化
void msp_spi_init(void);

// 写入1个byte
void msp_spi_write_byte(uint8_t byte);

// 读取1个byte
uint8_t msp_spi_read_byte(void);

#endif // MSP_SPI_H