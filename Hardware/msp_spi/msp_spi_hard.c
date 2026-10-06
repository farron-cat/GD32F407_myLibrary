#include "msp_spi_hard.h"

void msp_spi_hard_init(void)
{
    // GPIO初始化
    // SCL PA5 MOSI PA7 MISO PA6
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    gpio_af_set(GPIOA, GPIO_AF_5, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

    // SPI初始化
    spi_i2s_deinit(SPI0);
    rcu_periph_clock_enable(RCU_SPI0);
    spi_parameter_struct spi_init_struct;
    spi_struct_para_init(&spi_init_struct);

    spi_init_struct.device_mode = SPI_MASTER;                      // 主从模式
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;         // 全双工
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;               // 8位数据帧
    spi_init_struct.nss = SPI_NSS_SOFT;                            // NSS信号由软件控制
    spi_init_struct.endian = SPI_ENDIAN_MSB;                       // 高位在前
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE; // 极性1，相位1
    spi_init_struct.prescale = SPI_PSC_4;                          // 时钟预分频

    spi_init(SPI0, &spi_init_struct);

    // 每次传输1byte
    spi_i2s_data_frame_format_config(SPI0, SPI_FRAMESIZE_8BIT);

    spi_enable(SPI0);
}

void msp_spi_hard_write_byte(uint8_t byte)
{
    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE))
        ;
    spi_i2s_data_transmit(SPI0, byte);
    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE))
        ;
    spi_i2s_data_receive(SPI0);
}

uint8_t msp_spi_hard_read_byte(void)
{
    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE))
        ;
    spi_i2s_data_transmit(SPI0, 0x00); // 发送数据主要是为了控制时钟信号线
    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE))
        ;
    return spi_i2s_data_receive(SPI0);
}