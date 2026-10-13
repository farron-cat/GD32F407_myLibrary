#include "msp_spi_onewire.h"

// 0:软实现  1:硬实现
#define SPI_HARD_SOFT 1

#define SPI_SCL_RCU RCU_GPIOB
#define SPI_SCL_PORT GPIOB
#define SPI_SCL_PIN GPIO_PIN_13

#define SPI_MOSI_RCU RCU_GPIOB
#define SPI_MOSI_PORT GPIOB
#define SPI_MOSI_PIN GPIO_PIN_15

#define SPI_MISO_RCU RCU_GPIOB
#define SPI_MISO_PORT GPIOB
#define SPI_MISO_PIN GPIO_PIN_14

#if SPI_HARD_SOFT
#define SPI_SCL_AF GPIO_AF_5
#define SPI_MOSI_AF GPIO_AF_5
#define SPI_MISO_AF GPIO_AF_5

#define SPI_RCU RCU_SPI1
#define SPI_NUM SPI1
#define SPI_PL_PH SPI_CK_PL_HIGH_PH_2EDGE
#define SPI_PRES SPI_PSC_4
#else
// SCL  PA5
#define SPI_SCL_L gpio_bit_reset(SPI_SCL_PORT, SPI_SCL_PIN) // SCL
#define SPI_SCL_H gpio_bit_set(SPI_SCL_PORT, SPI_SCL_PIN)
// MOSI  PA7
#define SPI_MOSI_L gpio_bit_reset(SPI_MOSI_PORT, SPI_MOSI_PIN) // SDA
#define SPI_MOSI_H gpio_bit_set(SPI_MOSI_PORT, SPI_MOSI_PIN)
// MISO  PA6
#define SPI_READ_MISO gpio_input_bit_get(SPI_MISO_PORT, SPI_MISO_PIN) // FS0
#endif

#if SPI_HARD_SOFT
// IIC初始化
static void hard_spi_init();
// IIC写入Nbyte  addr:7bit地址  offset:下一个地址偏移
static void hard_spi_write_byte(uint8_t dat);
// IIC读取Nbyte  addr:7bit地址
static uint8_t hard_spi_read_byte();
#else
// IIC初始化
static void soft_spi_init();
// IIC写入Nbyte  addr:7bit地址  offset:下一个地址偏移
static void soft_spi_write_byte(uint8_t dat);
// IIC读取Nbyte  addr:7bit地址
static uint8_t soft_spi_read_byte();
#endif

// SCL:PA5
// MOSI:PA7
// MISO:PA6
/******************************** 对外暴露方法 ********************************/
// 初始化
void msp_spi_onewire_init()
{
#if SPI_HARD_SOFT
    hard_spi_init();
#else
    soft_spi_init();
#endif
}

// 写入byte方法
void msp_spi_onewire_write_byte(uint8_t dat)
{
#if SPI_HARD_SOFT
    hard_spi_write_byte(dat);
#else
    soft_spi_write_byte(dat);
#endif
}

// 读取byte方法
uint8_t msp_spi_onewire_read_byte()
{
#if SPI_HARD_SOFT
    return hard_spi_read_byte();
#else
    return soft_spi_read_byte();
#endif
}

/******************************** SPI硬实现 ********************************/
#if SPI_HARD_SOFT
// 初始化
static void hard_spi_init()
{
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

    // 初始化SPI
    spi_i2s_deinit(SPI_NUM);
    rcu_periph_clock_enable(SPI_RCU);
    spi_parameter_struct spi_struct;
    spi_struct_para_init(&spi_struct);

    spi_struct.device_mode = SPI_MASTER;              // 角色
    spi_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX; // 全双工
    spi_struct.frame_size = SPI_FRAMESIZE_8BIT;       // 每帧发送1byte
    spi_struct.nss = SPI_NSS_SOFT;                    // 软件片选  手动操作片选线
    spi_struct.clock_polarity_phase = SPI_PL_PH;      // 极性和相位
    spi_struct.prescale = SPI_PRES;                   // SPI时钟预分频  42M/4 = 10.5M
    spi_struct.endian = SPI_ENDIAN_MSB;               // 高位先行

    spi_init(SPI_NUM, &spi_struct);
    // 每次传输1byte
    spi_i2s_data_frame_format_config(SPI_NUM, SPI_FRAMESIZE_8BIT);

    spi_enable(SPI_NUM);
}

// 写入byte方法
static void hard_spi_write_byte(uint8_t dat)
{
    while (RESET == spi_i2s_flag_get(SPI_NUM, SPI_FLAG_TBE))
        ;
    spi_i2s_data_transmit(SPI_NUM, dat);
    //	while(RESET == spi_i2s_flag_get(SPI_NUM, SPI_FLAG_RBNE));
    //	spi_i2s_data_receive(SPI_NUM);
}

// 读取byte方法
static uint8_t hard_spi_read_byte()
{
    while (RESET == spi_i2s_flag_get(SPI_NUM, SPI_FLAG_TBE))
        ;
    spi_i2s_data_transmit(SPI_NUM, 0x00); // 发送数据主要是为了控制时钟信号线
    while (RESET == spi_i2s_flag_get(SPI_NUM, SPI_FLAG_RBNE))
        ;
    return spi_i2s_data_receive(SPI_NUM);
}
#else
/******************************** SPI软实现 ********************************/
// 初始化
static void soft_spi_init()
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

// 写入byte方法
static void soft_spi_write_byte(uint8_t dat)
{
    // 0x55  0b01 010100
    for (uint8_t i = 0; i < 8; i++)
    {
        // SCL拉低  写入数据
        SPI_SCL_L;
        if (dat & 0x80) // 0b1000 0000
            SPI_MOSI_H;
        else
            SPI_MOSI_L;
        // 拉高 等待对方读取
        SPI_SCL_H;
        dat <<= 1;
    }
}

// 读取byte方法
static uint8_t soft_spi_read_byte()
{
    uint8_t read = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        // 拉低SCL 写入
        SPI_SCL_L;
        read <<= 1;
        // 拉高SCL 读取
        SPI_SCL_H;
        //		read |= OLED_READ_FS0();
        if (SPI_READ_MISO)
        {
            read++;
        }
    }
    return read;
}
#endif