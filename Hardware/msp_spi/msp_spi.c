#include "msp_spi.h"

// 初始化
void msp_spi_init(void)
{
    // SCL MOSI
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_bit_set(GPIOA, GPIO_PIN_5 | GPIO_PIN_7); // 设置初始状态为高电平
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_5 | GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_5 | GPIO_PIN_7);

    // MISO
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6);
}

// 写入1个byte
void msp_spi_write_byte(uint8_t byte)
{
    // 0x55  0b01 010100
    for (uint8_t i = 0; i < 8; i++)
    {
        // SCL拉低  写入数据
        SPI_SCL_L;
        if (byte & 0x80) // 0b1000 0000
            SPI_MOSI_H;
        else
            SPI_MOSI_L;
        // 拉高 等待对方读取
        SPI_SCL_H;
        byte <<= 1;
    }
}

// 读取1个byte
uint8_t msp_spi_read_byte(void)
{
    uint8_t i, read = 0;
    for (i = 0; i < 8; i++)
    {
        SPI_SCL_L;
        read <<= 1;
        if (SPI_MISO_READ)
        {
            read++;
        }
        SPI_SCL_H;
    }
    return read;
}