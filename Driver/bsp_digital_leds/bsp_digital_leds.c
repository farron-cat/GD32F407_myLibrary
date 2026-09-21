#include "bsp_digital_leds.h"

#define DIGLED_PORT GPIOD
#define DIGLED_RCU RCU_GPIOD

#define SER_PIN GPIO_PIN_0
#define RCLK_PIN GPIO_PIN_1
#define SRCLK_PIN GPIO_PIN_4

/* 便捷操作宏 */
#define SER_H() gpio_bit_set(DIGLED_PORT, SER_PIN)
#define SER_L() gpio_bit_reset(DIGLED_PORT, SER_PIN)
#define RCLK_H() gpio_bit_set(DIGLED_PORT, RCLK_PIN)
#define RCLK_L() gpio_bit_reset(DIGLED_PORT, RCLK_PIN)
#define SRCLK_H() gpio_bit_set(DIGLED_PORT, SRCLK_PIN)
#define SRCLK_L() gpio_bit_reset(DIGLED_PORT, SRCLK_PIN)

#define DIG_NUM 8
unsigned char displayBuffer[DIG_NUM];
const unsigned char digCodeTable[DIG_NUM] =
    {
        DIG_1, /* position 0 → 第 1 位 */
        DIG_2, /* position 1 → 第 2 位 */
        DIG_3, /* position 2 → 第 3 位 */
        DIG_4, /* position 3 → 第 4 位 */
        DIG_5, /* position 4 → 第 5 位 */
        DIG_6, /* position 5 → 第 6 位 */
        DIG_7, /* position 6 → 第 7 位 */
        DIG_8, /* position 7 → 第 8 位 */
};

void bsp_digital_leds_init(void)
{
    rcu_periph_clock_enable(DIGLED_RCU);
    gpio_mode_set(DIGLED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SER_PIN | RCLK_PIN | SRCLK_PIN);
    gpio_output_options_set(DIGLED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, SER_PIN | RCLK_PIN | SRCLK_PIN);

    SER_L();
    RCLK_L();
    SRCLK_L();
}

void bsp_digital_leds_send_byte(uint8_t dat)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        /* 先发送 bit7，再 bit6 ... 最后 bit0 */
        if ((dat >> (7 - i)) & 0x01)
        {
            SER_H();
        }
        else
        {
            SER_L();
        }

        /* 产生移位时钟上升沿，数据被移入 */
        SRCLK_L();
        SRCLK_H(); /* 上升沿移入 */
    }
}

void bsp_digital_leds_send_code(unsigned char segCode, unsigned char digCode)
{
    /* 段码：先发，移完 8 位后会落到第二片 595 */
    bsp_digital_leds_send_byte(segCode);

    /* 位码：后发，移完 8 位后留在第一片 595 */
    bsp_digital_leds_send_byte(digCode);

    /* 所有位移完后，产生锁存脉冲 */
    RCLK_L();
    RCLK_H();
}

void send_code_to_buffer(unsigned char segCode, unsigned char pos)
{
    if (pos < DIG_NUM)
    {
        displayBuffer[pos] = segCode;
    }
}

void clear_buffer(void)
{
    unsigned char i;
    for (i = 0; i < DIG_NUM; i++)
    {
        displayBuffer[i] = SEG_CLR;
    }
}

/* 刷新周期：1ms / 位；8 位 → 一帧 8ms → 约 125Hz，无可见闪烁 */
#define DIG_SCAN_INTERVAL_US (1000U)

/**
 * @brief 数码管扫描，需在主循环或周期性任务中反复调用
 * @note  用 get_us_cnt() 做非阻塞定时，每次调用最多刷新一位
 * @note  调用频率越高，时间抖动越小；建议 ≥ 10kHz（即每 100us 调一次）
 */
void bsp_digital_leds_scan(void)
{
    static uint64_t last_refresh_us = 0;
    static unsigned char position = 0;

    uint64_t now = get_us_cnt();

    if (now - last_refresh_us >= DIG_SCAN_INTERVAL_US)
    {
        last_refresh_us = now;

        /* 段码 + 位码 */
        bsp_digital_leds_send_byte(displayBuffer[position]);
        bsp_digital_leds_send_byte(digCodeTable[position]);

        /* 锁存上升沿 */
        RCLK_H();
        RCLK_L();

        position = (position + 1) % DIG_NUM;
    }
}
