#include "bsp_188_leds.h"
// 引脚参数表
static LED188_PIN_PARAM pins[] = {
    {PIN1_RCU, PIN1_PORT, PIN1_PIN},
    {PIN2_RCU, PIN2_PORT, PIN2_PIN},
    {PIN3_RCU, PIN3_PORT, PIN3_PIN},
    {PIN4_RCU, PIN4_PORT, PIN4_PIN},
    {PIN5_RCU, PIN5_PORT, PIN5_PIN},
};

// 设置引脚为高阻
#define PIN_IN(pin) gpio_mode_set(pins[pin].port, GPIO_MODE_INPUT, GPIO_PUPD_NONE, pins[pin].gpio_pin)
// 设置所有引脚为高阻
#define PIN_ALL_IN \
    PIN_IN(PIN1);  \
    PIN_IN(PIN2);  \
    PIN_IN(PIN3);  \
    PIN_IN(PIN4);  \
    PIN_IN(PIN5)

// 设置引脚输出 1=高电平 或 0=低电平
#define PIN_OUT(pin, val)                                                                \
    gpio_mode_set(pins[pin].port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pins[pin].gpio_pin); \
    gpio_bit_write(pins[pin].port, pins[pin].gpio_pin, val)

// 初始化188数码管
void bsp_188_leds_init()
{
    for (uint8_t i = 0; i < LED188_PIN_NUM; i++)
    {
        rcu_periph_clock_enable(pins[i].rcu);
        gpio_output_options_set(pins[i].port, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, pins[i].gpio_pin);
    }
}

void bsp_188_leds_test()
{
    // 所有高阻
    PIN_ALL_IN;
    //
    PIN_OUT(PIN3, SET);
    PIN_OUT(PIN5, RESET);
}