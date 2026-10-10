#include "bsp_188_leds.h"
#include "systick.h"

// 引脚参数表
static LED188_PIN_PARAM pins[] = {
    {PIN1_RCU, PIN1_PORT, PIN1_PIN},
    {PIN2_RCU, PIN2_PORT, PIN2_PIN},
    {PIN3_RCU, PIN3_PORT, PIN3_PIN},
    {PIN4_RCU, PIN4_PORT, PIN4_PIN},
    {PIN5_RCU, PIN5_PORT, PIN5_PIN},
};

// 设置引脚为高阻
#define PIN_IN(pin)                                                                         \
    do                                                                                      \
    {                                                                                       \
        gpio_mode_set(pins[pin].port, GPIO_MODE_INPUT, GPIO_PUPD_NONE, pins[pin].gpio_pin); \
    } while (0)

// 设置所有引脚为高阻
#define PIN_ALL_IN    \
    do                \
    {                 \
        PIN_IN(PIN1); \
        PIN_IN(PIN2); \
        PIN_IN(PIN3); \
        PIN_IN(PIN4); \
        PIN_IN(PIN5); \
    } while (0)

// 设置引脚输出 1=高电平 或 0=低电平
#define PIN_OUT(pin, val)                                                                    \
    do                                                                                       \
    {                                                                                        \
        gpio_mode_set(pins[pin].port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pins[pin].gpio_pin); \
        gpio_bit_write(pins[pin].port, pins[pin].gpio_pin, val);                             \
    } while (0)

// K2 K1 C1 B1 G2 F2 E2 D2 C2 B2 A2 G3 F3 E3 D3 C3 B3 A3
#define A3 (1 << 0)
#define B3 (1 << 1)
#define C3 (1 << 2)
#define D3 (1 << 3)
#define E3 (1 << 4)
#define F3 (1 << 5)
#define G3 (1 << 6)

#define A2 (1 << 7)
#define B2 (1 << 8)
#define C2 (1 << 9)
#define D2 (1 << 10)
#define E2 (1 << 11)
#define F2 (1 << 12)
#define G2 (1 << 13)

#define B1 (1 << 14)
#define C1 (1 << 15)

#define K1 (1 << 16)
#define K2 (1 << 17)

static uint32_t leds_status = 0x0;

// 个位数字码表
uint8_t one_digit_nums[] = {
    A3 | B3 | C3 | D3 | E3 | F3,      // 0
    B3 | C3,                          // 1
    A3 | B3 | D3 | E3 | G3,           // 2
    A3 | B3 | C3 | D3 | G3,           // 3
    B3 | C3 | F3 | G3,                // 4
    A3 | C3 | D3 | F3 | G3,           // 5
    A3 | C3 | D3 | E3 | F3 | G3,      // 6
    A3 | B3 | C3,                     // 7
    A3 | B3 | C3 | D3 | E3 | F3 | G3, // 8
    A3 | B3 | C3 | D3 | F3 | G3,      // 9
};

// 优化掉 十位数字码表 由个位<<7得到
// uint16_t ten_digit_nums[] = {
//     A2 | B2 | C2 | D2 | E2 | F2,      // 0
//     B2 | C2,                          // 1
//     A2 | B2 | D2 | E2 | G2,           // 2
//     A2 | B2 | C2 | D2 | G2,           // 3
//     B2 | C2 | F2 | G2,                // 4
//     A2 | C2 | D2 | F2 | G2,           // 5
//     A2 | C2 | D2 | E2 | F2 | G2,      // 6
//     A2 | B2 | C2,                     // 7
//     A2 | B2 | C2 | D2 | E2 | F2 | G2, // 8
//     A2 | B2 | C2 | D2 | F2 | G2,      // 9
// };

// 初始化188数码管
void bsp_188_leds_init()
{
    for (uint8_t i = 0; i < LED188_PIN_NUM; i++)
    {
        rcu_periph_clock_enable(pins[i].rcu);
        gpio_output_options_set(pins[i].port, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, pins[i].gpio_pin);
    }
}

void bsp_188_leds_show()
{
    // 高阻重置 1高电平
    PIN_ALL_IN;
    PIN_OUT(PIN1, 1);
    if (leds_status & A3)
        PIN_OUT(PIN2, 0);
    if (leds_status & C3)
        PIN_OUT(PIN3, 0);
    if (leds_status & E3)
        PIN_OUT(PIN4, 0);
    delay_1ms(1);
    // 高阻重置 2高电平
    PIN_ALL_IN;
    PIN_OUT(PIN2, 1);
    if (leds_status & B3)
        PIN_OUT(PIN1, 0);
    if (leds_status & A2)
        PIN_OUT(PIN3, 0);
    if (leds_status & C1)
        PIN_OUT(PIN4, 0);
    if (leds_status & K2)
        PIN_OUT(PIN5, 0);
    delay_1ms(1);
    // 高阻重置 3高电平
    PIN_ALL_IN;
    PIN_OUT(PIN3, 1);
    if (leds_status & D3)
        PIN_OUT(PIN1, 0);
    if (leds_status & B2)
        PIN_OUT(PIN2, 0);
    if (leds_status & B1)
        PIN_OUT(PIN4, 0);
    if (leds_status & K1)
        PIN_OUT(PIN5, 0);
    delay_1ms(1);
    // 高阻重置 4高电平
    PIN_ALL_IN;
    PIN_OUT(PIN4, 1);
    if (leds_status & F3)
        PIN_OUT(PIN1, 0);
    if (leds_status & D2)
        PIN_OUT(PIN2, 0);
    if (leds_status & C2)
        PIN_OUT(PIN3, 0);
    delay_1ms(1);
    // 高阻重置 5高电平
    PIN_ALL_IN;
    PIN_OUT(PIN5, 1);
    if (leds_status & G3)
        PIN_OUT(PIN1, 0);
    if (leds_status & E2)
        PIN_OUT(PIN2, 0);
    if (leds_status & F2)
        PIN_OUT(PIN3, 0);
    if (leds_status & G2)
        PIN_OUT(PIN4, 0);
    delay_1ms(1);
}

static uint8_t scan_step = 0;

void bsp_188_leds_scan(void)
{
    switch (scan_step)
    {
    case 0:
        PIN_ALL_IN;
        PIN_OUT(PIN1, 1);
        if (leds_status & A3)
            PIN_OUT(PIN2, 0);
        if (leds_status & C3)
            PIN_OUT(PIN3, 0);
        if (leds_status & E3)
            PIN_OUT(PIN4, 0);
        break;
    case 1:
        PIN_ALL_IN;
        PIN_OUT(PIN2, 1);
        if (leds_status & B3)
            PIN_OUT(PIN1, 0);
        if (leds_status & A2)
            PIN_OUT(PIN3, 0);
        if (leds_status & C1)
            PIN_OUT(PIN4, 0);
        if (leds_status & K2)
            PIN_OUT(PIN5, 0);
        break;
    case 2:
        PIN_ALL_IN;
        PIN_OUT(PIN3, 1);
        if (leds_status & D3)
            PIN_OUT(PIN1, 0);
        if (leds_status & B2)
            PIN_OUT(PIN2, 0);
        if (leds_status & B1)
            PIN_OUT(PIN4, 0);
        if (leds_status & K1)
            PIN_OUT(PIN5, 0);
        break;
    case 3:
        PIN_ALL_IN;
        PIN_OUT(PIN4, 1);
        if (leds_status & F3)
            PIN_OUT(PIN1, 0);
        if (leds_status & D2)
            PIN_OUT(PIN2, 0);
        if (leds_status & C2)
            PIN_OUT(PIN3, 0);
        break;
    case 4:
        PIN_ALL_IN;
        PIN_OUT(PIN5, 1);
        if (leds_status & G3)
            PIN_OUT(PIN1, 0);
        if (leds_status & E2)
            PIN_OUT(PIN2, 0);
        if (leds_status & F2)
            PIN_OUT(PIN3, 0);
        if (leds_status & G2)
            PIN_OUT(PIN4, 0);
        break;
    }
    scan_step++;
    if (scan_step >= 5)
        scan_step = 0;
}

void bsp_188_leds_set_num(uint8_t num)
{
    if (num > 199)
        return;

    if (num > 99)
    {
        // 显示百位
        leds_status |= B1 | C1;
    }
    if (num > 9)
    {
        // 显示十位
        leds_status |= (one_digit_nums[num % 100 / 10] << 7);
    }
    // 显示个位
    leds_status |= one_digit_nums[num % 10];
}

void bsp_188_leds_clear()
{
    // PIN_ALL_IN;
    leds_status = 0;
}