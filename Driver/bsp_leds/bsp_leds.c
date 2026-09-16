#include "bsp_leds.h"

// LED_PARAM leds[] = {
// 	{RCU_GPIOC,GPIOC,GPIO_PIN_6}, //0
// 	{RCU_GPIOD,GPIOD,GPIO_PIN_8}, //1
// 	{RCU_GPIOD,GPIOD,GPIO_PIN_9},
// 	{RCU_GPIOD,GPIOD,GPIO_PIN_10},
// 	{RCU_GPIOD,GPIOD,GPIO_PIN_11},
// 	{RCU_GPIOD,GPIOD,GPIO_PIN_12},
// 	{RCU_GPIOD,GPIOD,GPIO_PIN_13},
// 	{RCU_GPIOD,GPIOD,GPIO_PIN_14},
// 	{RCU_GPIOD,GPIOD,GPIO_PIN_15},
// };

// 内部GPIO初始化函数
static void GPIO_config(rcu_periph_enum rcu, uint32_t port, uint32_t pin)
{
    rcu_periph_clock_enable(rcu);
    gpio_mode_set(port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pin);
    gpio_output_options_set(port, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, pin);
}

void bsp_leds_config()
{

    // LED_SW LED灯组开关 低电平开
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_6);

    // LED1-8 LED灯 低电平点亮，高电平熄灭
    rcu_periph_clock_enable(RCU_GPIOD);
    gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

    // LED_SW 低电平打开灯组
    gpio_bit_reset(GPIOC, GPIO_PIN_6);

    // 默认灯都熄灭
    gpio_bit_set(GPIOD, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
}

// 打开所有的灯
void bsp_leds_open_all()
{
    gpio_bit_reset(GPIOD, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
}

// 关闭所有的灯
void bsp_leds_close_all()
{
    gpio_bit_set(GPIOD, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
}

// 打开一盏灯
void bsp_leds_open(LED_NUM led)
{
    switch (led)
    {
    case LED1:
        gpio_bit_reset(GPIOD, GPIO_PIN_8);
        break;
    case LED2:
        gpio_bit_reset(GPIOD, GPIO_PIN_9);
        break;
    case LED3:
        gpio_bit_reset(GPIOD, GPIO_PIN_10);
        break;
    case LED4:
        gpio_bit_reset(GPIOD, GPIO_PIN_11);
        break;
    case LED5:
        gpio_bit_reset(GPIOD, GPIO_PIN_12);
        break;
    case LED6:
        gpio_bit_reset(GPIOD, GPIO_PIN_13);
        break;
    case LED7:
        gpio_bit_reset(GPIOD, GPIO_PIN_14);
        break;
    case LED8:
        gpio_bit_reset(GPIOD, GPIO_PIN_15);
        break;
    default:
        break; // 可选，处理非法值
    }
}

// 关闭一盏灯
void bsp_leds_close(LED_NUM led)
{
    switch (led)
    {
    case LED1:
        gpio_bit_set(GPIOD, GPIO_PIN_8);
        break;
    case LED2:
        gpio_bit_set(GPIOD, GPIO_PIN_9);
        break;
    case LED3:
        gpio_bit_set(GPIOD, GPIO_PIN_10);
        break;
    case LED4:
        gpio_bit_set(GPIOD, GPIO_PIN_11);
        break;
    case LED5:
        gpio_bit_set(GPIOD, GPIO_PIN_12);
        break;
    case LED6:
        gpio_bit_set(GPIOD, GPIO_PIN_13);
        break;
    case LED7:
        gpio_bit_set(GPIOD, GPIO_PIN_14);
        break;
    case LED8:
        gpio_bit_set(GPIOD, GPIO_PIN_15);
        break;
    default:
        break; // 可选，处理非法值
    }
}