#include "bsp_ws2812_rgb_leds.h"
#include "msp_spi_onewire.h"
#include <string.h>

// ============ 编码原理(与 SPI 时钟强绑定，改 SPI 时钟必须重算表) ============
// SPI1 = APB1 42MHz ÷ SPI_PSC_4 = 10.5MHz → 1 个 SPI 位 = 95.24ns
// 1 个 WS2812 位 = 13 个 SPI 位 = 1238ns ≈ 1.25us
//   位'0' -> 1111 0000 0000 0  T0H=381ns(规格400±150)  T0L=857ns(规格850±150)
//   位'1' -> 1111 1111 0000 0  T1H=762ns(规格800±150)  T1L=476ns(规格450±150)
// 1 个颜色字节 = 8 位 = 104 个 SPI 位 = 13 字节(104 正好是 8 的整数倍)
// 1 颗灯(GRB) = 3 个颜色字节 = 39 字节
// 重算方法: 把每个 bit 展开成 13 位, 按 MSB 先行拼成字节流
//   cell = {0 : 0b1111000000000, 1 : 0b1111111100000}

// 0x00 的 13 字节 = 8 个 '0' 位单元
static const uint8_t ws2812_byte_00[13] = {
    0xF0,
    0x07,
    0x80,
    0x3C,
    0x01,
    0xE0,
    0x0F,
    0x00,
    0x78,
    0x03,
    0xC0,
    0x1E,
    0x00,
};

// 0xFF 的 13 字节 = 8 个 '1' 位单元
static const uint8_t ws2812_byte_FF[13] = {
    0xFF,
    0x07,
    0xF8,
    0x3F,
    0xC1,
    0xFE,
    0x0F,
    0xF0,
    0x7F,
    0x83,
    0xFC,
    0x1F,
    0xE0,
};

// 1 颗灯的数据：G R B 三个颜色字节的展开流
static uint8_t buff[39] = {0};

// 换颜色(GRB 顺序)：在 init 里按下面组合做 memcpy 即可
//   红 G=00 R=FF B=00   绿 G=FF R=00 B=00   蓝 G=00 R=00 B=FF
//   白 G=FF R=FF B=FF   黑 G=00 R=00 B=00
// 其它颜色值(非 0x00/0xFF)需要按上面的公式重新展开成 13 位再打包

void bsp_ws2812_rgb_leds_init(void)
{
    msp_spi_onewire_init();

    // 组装 1 颗灯的数据：G=0x00 / R=0xFF / B=0x00 → 红
    // (换颜色只改这三行的组合，见文件头"换颜色"说明)
    memcpy(&buff[0], ws2812_byte_00, sizeof(ws2812_byte_00));  // G
    memcpy(&buff[13], ws2812_byte_FF, sizeof(ws2812_byte_FF)); // R
    memcpy(&buff[26], ws2812_byte_00, sizeof(ws2812_byte_00)); // B
}

void bsp_ws2812_rgb_leds_send(void)
{
    // ① 帧前复位：先让数据线连续拉低 ≥300us
    //    WS2812 靠"上升沿"定界，前面必须有一段低电平把芯片的清零/帧头对齐；
    //    否则空闲期间被 R6(10K 上拉到 +5V)拉高的长高电平会被第 1 颗芯片
    //    当成多出来的 1 个数据位('1')，使第 1 颗灯的 G 变成 0x80 → 显示成橙黄
    msp_spi_onewire_reset();

    // ② 数据位流期间绝对不能被打断：
    //    SysTick 是 1us 一次中断，且每 1ms 还会在中断里跑 188 数码管/ADKEY 扫描(数 us)，
    //    一旦落在位流中间就会把某一位的高电平拉长 → 该位被从 '0' 读成 '1' → 灯乱色
    __disable_irq();

    for (uint8_t j = 0; j < 4; j++)
    {
        for (uint8_t i = 0; i < 39; i++)
        {
            msp_spi_onewire_write_byte(buff[i]);
        }
    }

    __enable_irq();

    // ③ 帧尾复位：再连续拉低 ≥300us，4 颗灯一起锁存显示
    msp_spi_onewire_reset();
}
