#include "bsp_ws2812_rgb_leds.h"
#include "msp_spi_onewire.h"

// ============ 编码原理(与 SPI 时钟强绑定，改 SPI 时钟必须重算) ============
// SPI1 = APB1 42MHz ÷ SPI_PSC_4 = 10.5MHz → 1 个 SPI 位 = 95.24ns
// 1 个 WS2812 位 = 13 个 SPI 位 = 1238ns ≈ 1.25us
//   位'0' -> 1111 0000 0000 0  T0H=381ns(规格400±150)  T0L=857ns(规格850±150)
//   位'1' -> 1111 1111 0000 0  T1H=762ns(规格800±150)  T1L=476ns(规格450±150)
// 1 个颜色字节 = 8 位 = 104 个 SPI 位 = 13 字节(104 正好是 8 的整数倍)
// 1 颗灯(GRB) = 3 个颜色字节 = 39 字节   →   4 颗灯 = 156 字节
// 任意颜色不用预生成表，由 ws2812_pack_byte() 现场展开(见下)

// 1 个 WS2812 位展开成的 13 个 SPI 位
#define WS2812_CELL_0 0x1E00u // 1111 0000 0000 0
#define WS2812_CELL_1 0x1FE0u // 1111 1111 0000 0

#define WS2812_BYTE_BITS   13                           // 1 个 WS2812 位 = 13 个 SPI 位
#define WS2812_COLOR_BYTES (8 * WS2812_BYTE_BITS / 8)   // 1 个颜色字节 = 13 字节
#define WS2812_LED_NUM     4                            // 灯珠数量
#define WS2812_LED_BYTES   (3 * WS2812_COLOR_BYTES)     // 1 颗灯(GRB) = 39 字节

// 4 颗灯的数据(每颗 39 字节)，bsp_ws2812_rgb_leds_send() 一次全发
static uint8_t buff[WS2812_LED_NUM * WS2812_LED_BYTES] = {0};

// 把 1 个颜色字节(8 位)现场展开成 13 个 SPI 字节(高位先行)
//   做法：依次把 8 个「13 位单元」拼进 32 位累加器，每凑够 8 位就吐 1 个字节
//   自检：0x00 → F0 07 80 3C 01 E0 0F 00 78 03 C0 1E 00
//         0xFF → FF 07 F8 3F C1 FE 0F F0 7F 83 FC 1F E0
static void ws2812_pack_byte(uint8_t dat, uint8_t *out)
{
    uint32_t acc = 0;  // 位累加器(只有低 bits 位是有效数据)
    uint8_t bits = 0;  // acc 里的有效位数
    uint8_t n = 0;     // 已输出的字节数

    for (int8_t i = 7; i >= 0; i--)
    {
        acc = (acc << WS2812_BYTE_BITS) | ((dat & (1 << i)) ? WS2812_CELL_1 : WS2812_CELL_0);
        bits += WS2812_BYTE_BITS;
        while (bits >= 8)
        {
            bits -= 8;
            out[n++] = (uint8_t)((acc >> bits) & 0xFF);
        }
    }
}

// 设置第 index 颗灯的颜色(0 ~ WS2812_LED_NUM-1)，rgb 形如 0xFF0000(红)
// 只改内存里的数据，要看到效果还得调用 bsp_ws2812_rgb_leds_send()
// WS2812 的发送顺序是 GRB，所以把 rgb 拆成 R/G/B 之后按 G、R、B 顺序打包
void bsp_ws2812_rgb_leds_set_color(uint8_t index, uint32_t rgb)
{
    if (index >= WS2812_LED_NUM)
        return;

    uint8_t *p = &buff[index * WS2812_LED_BYTES];
    uint8_t r = (uint8_t)(rgb >> 16);
    uint8_t g = (uint8_t)(rgb >> 8);
    uint8_t b = (uint8_t)rgb;

    ws2812_pack_byte(g, &p[0]);                      // G
    ws2812_pack_byte(r, &p[WS2812_COLOR_BYTES]);     // R
    ws2812_pack_byte(b, &p[2 * WS2812_COLOR_BYTES]); // B
}

void bsp_ws2812_rgb_leds_init(void)
{
    msp_spi_onewire_init();

    // 默认 4 颗全红(0xFF0000)，保持原来的现象；
    // 想单独改某颗灯，用 bsp_ws2812_rgb_leds_set_color()
    for (uint8_t i = 0; i < WS2812_LED_NUM; i++)
    {
        bsp_ws2812_rgb_leds_set_color(i, 0xFF0000);
    }
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

    for (uint16_t i = 0; i < sizeof(buff); i++)
    {
        msp_spi_onewire_write_byte(buff[i]);
    }

    __enable_irq();

    // ③ 帧尾复位：再连续拉低 ≥300us，4 颗灯一起锁存显示
    msp_spi_onewire_reset();
}
