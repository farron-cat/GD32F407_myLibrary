#include "msp_spi.h"

// ============ 声    明 ============
#if SPI_HARD_SOFT_SWITCH
// 硬件SPI
static void msp_spi_hard_init(void);
static void msp_spi_hard_write_byte(uint8_t byte);
static uint8_t msp_spi_hard_read_byte(void);
static uint8_t msp_spi_hard_read_write_byte(uint8_t byte);

#else
// 软件SPI
static void msp_spi_soft_init(void);
static void msp_spi_soft_write_byte(uint8_t byte);
static uint8_t msp_spi_soft_read_byte(void);
static uint8_t msp_spi_soft_read_write_byte(uint8_t byte);
#endif

// ============ 外部接口 ============
// 初始化
void msp_spi_init(void)
{
#if SPI_HARD_SOFT_SWITCH
    // 硬件SPI
    msp_spi_hard_init();
#else
    // 软件SPI
    msp_spi_soft_init();
#endif
}

// 写入1个byte
void msp_spi_write_byte(uint8_t byte)
{
#if SPI_HARD_SOFT_SWITCH
    // 硬件SPI
    msp_spi_hard_write_byte(byte);
#else
    // 软件SPI
    msp_spi_soft_write_byte(byte);
#endif
}

// 读取1个byte
uint8_t msp_spi_read_byte(void)
{
#if SPI_HARD_SOFT_SWITCH
    // 硬件SPI
    return msp_spi_hard_read_byte();
#else
    // 软件SPI
    return msp_spi_soft_read_byte();
#endif
}

// 读写1个byte
uint8_t msp_spi_read_write_byte(uint8_t byte)
{
#if SPI_HARD_SOFT_SWITCH
    // 硬件SPI
    return msp_spi_hard_read_write_byte(byte);
#else
    // 软件SPI
    return msp_spi_soft_read_write_byte(byte);
#endif
}

// ============ 内部实现 ============
#if SPI_HARD_SOFT_SWITCH
// 硬件SPI
static void msp_spi_hard_init(void)
{
    // GPIO初始化
    // SCL
    rcu_periph_clock_enable(SPI_SCL_RCU);
    gpio_mode_set(SPI_SCL_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI_SCL_PIN);
    gpio_af_set(SPI_SCL_PORT, SPI_SCL_AF, SPI_SCL_PIN);
    // MOSI
    rcu_periph_clock_enable(SPI_MOSI_RCU);
    gpio_mode_set(SPI_MOSI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI_MOSI_PIN);
    gpio_af_set(SPI_MOSI_PORT, SPI_MOSI_AF, SPI_MOSI_PIN);
    // MISO
    rcu_periph_clock_enable(SPI_MISO_RCU);
    gpio_mode_set(SPI_MISO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI_MISO_PIN);
    gpio_af_set(SPI_MISO_PORT, SPI_MISO_AF, SPI_MISO_PIN);

    // SPI初始化
    spi_i2s_deinit(SPI_NUM);
    rcu_periph_clock_enable(SPI_RCU);
    spi_parameter_struct spi_init_struct;
    spi_struct_para_init(&spi_init_struct);

    spi_init_struct.device_mode = SPI_MASTER;              // 主从模式
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX; // 全双工
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;       // 8位数据帧
    spi_init_struct.nss = SPI_NSS_SOFT;                    // NSS信号由软件控制
    spi_init_struct.endian = SPI_ENDIAN_MSB;               // 高位在前
    spi_init_struct.clock_polarity_phase = SPI_PL_PH;      // 极性1，相位1
    spi_init_struct.prescale = SPI_PRES;                   // 时钟预分频

    spi_init(SPI_NUM, &spi_init_struct);

    // 每次传输1byte
    spi_i2s_data_frame_format_config(SPI_NUM, SPI_FRAMESIZE_8BIT);

    spi_enable(SPI_NUM);
}

static void msp_spi_hard_write_byte(uint8_t byte)
{
    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE))
        ;
    spi_i2s_data_transmit(SPI0, byte);
    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE))
        ;
    spi_i2s_data_receive(SPI0);
}

static uint8_t msp_spi_hard_read_byte(void)
{
    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE))
        ;
    spi_i2s_data_transmit(SPI0, 0x00); // 发送数据主要是为了控制时钟信号线
    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE))
        ;
    return spi_i2s_data_receive(SPI0);
}

static uint8_t msp_spi_hard_read_write_byte(uint8_t byte)
{
    while (RESET == spi_i2s_flag_get(SPI_NUM, SPI_FLAG_TBE))
        ;
    spi_i2s_data_transmit(SPI_NUM, dat); // 发送数据主要是为了控制时钟信号线
    while (RESET == spi_i2s_flag_get(SPI_NUM, SPI_FLAG_RBNE))
        ;
    return spi_i2s_data_receive(SPI_NUM);
}

#else
// 软件SPI
// 初始化
static void msp_spi_soft_init(void)
{
    // SCL
    rcu_periph_clock_enable(SPI_SCL_RCU);
    gpio_bit_set(SPI_SCL_PORT, SPI_SCL_PIN);
    gpio_mode_set(SPI_SCL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_SCL_PIN);
    gpio_output_options_set(SPI_SCL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI_SCL_PIN);
    // MOSI
    rcu_periph_clock_enable(SPI_MOSI_RCU);
    gpio_bit_set(SPI_MOSI_PORT, SPI_MOSI_PIN);
    gpio_mode_set(SPI_MOSI_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_MOSI_PIN);
    gpio_output_options_set(SPI_MOSI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI_MOSI_PIN);
    // MISO
    rcu_periph_clock_enable(SPI_MISO_RCU);
    gpio_mode_set(SPI_MISO_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_MISO_PIN);
    gpio_output_options_set(SPI_MISO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI_MISO_PIN);
}

// 写入1个byte
static void msp_spi_soft_write_byte(uint8_t byte)
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
static uint8_t msp_spi_soft_read_byte(void)
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

static uint8_t msp_spi_soft_read_write_byte(uint8_t byte)
{
    uint8_t i, read = 0;
    for (i = 0; i < 8; i++)
    {
        // 拉低SCL 写入
        SPI_SCL_L;
        read <<= 1;
        if (byte & 0x80) // 0b1000 0000
            SPI_MOSI_H;
        else
            SPI_MOSI_L;
        // 拉高SCL 读取
        SPI_SCL_H;
        //		read |= OLED_READ_FS0();
        if (SPI_MISO_READ)
        {
            read++;
        }
        byte <<= 1;
    }
    return read;
}

#endif