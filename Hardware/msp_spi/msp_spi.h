#ifndef MSP_SPI_H
#define MSP_SPI_H

#include "gd32f4xx.h"

// SCL PA5
#define SPI_SCL_RCU  RCU_GPIOA
#define SPI_SCL_PORT GPIOA
#define SPI_SCL_PIN  GPIO_PIN_5
// MOSI PA7
#define SPI_MOSI_RCU  RCU_GPIOA
#define SPI_MOSI_PORT GPIOA
#define SPI_MOSI_PIN  GPIO_PIN_7
// MISO PA6
#define SPI_MISO_RCU  RCU_GPIOA
#define SPI_MISO_PORT GPIOA
#define SPI_MISO_PIN  GPIO_PIN_6

// 软硬件SPI选择
// 1:硬件SPI  0:软件SPI
#define SPI_HARD_SOFT_SWITCH 0

#if SPI_HARD_SOFT_SWITCH
// 硬件SPI
#define SPI_SCL_AF  GPIO_AF_5
#define SPI_MOSI_AF GPIO_AF_5
#define SPI_MISO_AF GPIO_AF_5

#define SPI_RCU     RCU_SPI0
#define SPI_NUM     SPI0
#define SPI_PL_PH   SPI_CK_PL_HIGH_PH_2EDGE
#define SPI_PRES    SPI_PSC_4

#else
// 软件SPI
// SCL PA5
#define SPI_SCL_L     gpio_bit_reset(GPIOA, GPIO_PIN_5)
#define SPI_SCL_H     gpio_bit_set(GPIOA, GPIO_PIN_5)
// MOSI PA7
#define SPI_MOSI_L    gpio_bit_reset(GPIOA, GPIO_PIN_7)
#define SPI_MOSI_H    gpio_bit_set(GPIOA, GPIO_PIN_7)
// MISO PA6
#define SPI_MISO_READ gpio_input_bit_get(GPIOA, GPIO_PIN_6)

#endif // SPI_HARD_SOFT_SWITCH

// 初始化
void msp_spi_init(void);

// 写入1个byte
void msp_spi_write_byte(uint8_t byte);

// 读取1个byte
uint8_t msp_spi_read_byte(void);

#endif // MSP_SPI_H