#include "msp_spi_onewire.h"

// 0:软实现  1:硬实现
#define SPI_HARD_SOFT 1

// WS2812 只用一根数据线(DIN)，所以本模块只配 MOSI(PB15)
#define SPI_MOSI_RCU RCU_GPIOB
#define SPI_MOSI_PORT GPIOB
#define SPI_MOSI_PIN GPIO_PIN_15

// ⚠ 以下两个引脚在本板上不能配(SCK/MISO 对 WS2812 无用)：
//   PB13 = 网络 CAN_TX(P3.25 + TJA1050-1.2 TXD)：配成 SPI1_SCK 输出后，
//          10.5MHz 方波会被 TJA1050 直接搬到 CANH/CANL → CAN 总线被垃圾帧占死
//   PB14 = 只引到排针 P3.28，没有接任何器件
// #define SPI_SCL_RCU RCU_GPIOB
// #define SPI_SCL_PORT GPIOB
// #define SPI_SCL_PIN GPIO_PIN_13
// #define SPI_MISO_RCU RCU_GPIOB
// #define SPI_MISO_PORT GPIOB
// #define SPI_MISO_PIN GPIO_PIN_14

#if SPI_HARD_SOFT
#define SPI_MOSI_AF GPIO_AF_5

// SPI1 挂在 APB1(168M PLL: AHB=168M / APB1=DIV4 = 42MHz)，SPI_PSC_4 → 10.5MHz
// (旧注释"84M/4 = 21M"是 SPI0/APB2 的账，SPI1 不在 APB2)
// 本工程 WS2812 采用「1 个 WS2812 位 = 13 个 SPI 位」：13 / 10.5M = 1238ns ≈ 1.25us
// 对应编码表见 Driver/bsp_ws2812_rgb_leds/bsp_ws2812_rgb_leds.c
#define SPI_RCU RCU_SPI1
#define SPI_NUM SPI1
#define SPI_PL_PH SPI_CK_PL_HIGH_PH_2EDGE
#define SPI_PRES SPI_PSC_4
#else
// ⚠ 软件分支在本板上不可用：它依赖 SCL/MISO(原定 PB13/PB14)，而 PB13 = CAN_TX
//   要用软实现请先把 SCL 换到真正空闲的引脚，并恢复上面的 SPI_SCL_*/SPI_MISO_* 定义
#error "SPI_HARD_SOFT=0 的软实现依赖已禁用的 PB13/PB14，请先改引脚定义"
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

// 单总线(WS2812 等)：MOSI(PB15) 接 DIN
// SPI1 = APB1 42MHz ÷ 4 = 10.5MHz

// 帧复位长度(字节)：0x00 = 8 个 0 位 = 连续低电平 762ns/字节
// 400 字节 ≈ 305us(WS2812B 需 >50us；WS2812B-V5 需 >280us)
#define SPI_ONEWIRE_RESET_BYTES 400

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

// 帧复位：用 SPI 连续发 SPI_ONEWIRE_RESET_BYTES 个 0x00，把数据线连续拉低
//   0x00 的 8 个 bit 全是 0 → 每个字节 = 连续低电平 8 / 10.5MHz = 762ns
//   400 字节 = 305us，满足 WS2812B(>50us) 与 WS2812B-V5(>280us)
// ⚠ 这里必须"由 SPI 自己打低"，不能"切 GPIO 拉低再切回 AF"：
//   切 AF 期间 MOSI 是高阻，会被板上 R6(10K 上拉到 +5V)拉高，
//   第 1 颗单总线器件会把那段长高电平当成多出来的 1 个数据位('1') → 数据错位
// 本函数只用 SPI 写字节，不依赖 SysTick，可在关中断状态下调用
void msp_spi_onewire_reset(void)
{
#if SPI_HARD_SOFT
    for (uint16_t i = 0; i < SPI_ONEWIRE_RESET_BYTES; i++)
    {
        hard_spi_write_byte(0x00);
    }
#endif
    // 软实现分支若恢复使用，请在此处连续拉低数据线 300us 以上
}

/******************************** SPI硬实现 ********************************/
#if SPI_HARD_SOFT
// 初始化
static void hard_spi_init()
{
    // 只配 MOSI(PB15)：WS2812 是单向单线器件，SCK/MISO 不接、也不能配(见文件头注释)
    rcu_periph_clock_enable(SPI_MOSI_RCU);
    gpio_mode_set(SPI_MOSI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI_MOSI_PIN);
    gpio_output_options_set(SPI_MOSI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI_MOSI_PIN);
    gpio_af_set(SPI_MOSI_PORT, SPI_MOSI_AF, SPI_MOSI_PIN);

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