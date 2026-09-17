#include "bsp_keys.h"

// 抽取出按键引脚参数便于修改
KEY_PARAM keys[] = {
    {RCU_GPIOC, GPIOC, GPIO_PIN_0}, // KEY1
    {RCU_GPIOC, GPIOC, GPIO_PIN_1}, // KEY2
    {RCU_GPIOC, GPIOC, GPIO_PIN_2}, // KEY3
    {RCU_GPIOC, GPIOC, GPIO_PIN_3}, // KEY4
};

// 用一个字节的位图保存所有按键的历史状态，0xFF 表示默认高电平
static uint8_t preStas = 0xFF;

#define GET_STA(i) ((preStas >> i) & 0x01)
#define SET_HIGH(i) (preStas |= (0x01 << i))
#define SET_LOW(i) (preStas &= ~(0x01 << i))

#ifndef __weak
#define __weak __attribute__((weak)) // GCC / armclang
#endif
// 按键按下回调
// 弱引用  可以被其他文件重定义
__weak void on_key_press(KEY_NUM key)
{
}

// 按键松开回调
__weak void on_key_release(KEY_NUM key)
{
}

// 内部GPIO初始化函数
static void GPIO_config(rcu_periph_enum rcu, uint32_t port, uint32_t pin)
{
    rcu_periph_clock_enable(rcu);
    gpio_mode_set(port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, pin);
}

// 初始化独立按键
void bsp_keys_config()
{
    for (uint8_t i = 0; i < KEY_MAX; i++)
    {
        GPIO_config(keys[i].rcu, keys[i].port, keys[i].pin);
    }
}

// 获取按键状态
FlagStatus bsp_keys_get_state(KEY_NUM key)
{
    return gpio_input_bit_get(keys[key].port, keys[key].pin);
}

// 扫描按键，返回事件枚举
KEY_EVENT bsp_keys_scan(KEY_NUM key)
{
    FlagStatus curSta = bsp_keys_get_state(key);
    KEY_EVENT event = KEY_EVENT_NONE;

    // 下降沿 按下
    if (GET_STA(key) == SET && curSta == RESET)
    {
        on_key_press(key);
        event = KEY_EVENT_PRESSED;
        SET_LOW(key);
    }
    // 上升沿 松开
    else if (GET_STA(key) == RESET && curSta == SET)
    {
        on_key_release(key);
        event = KEY_EVENT_RELEASED;
        SET_HIGH(key);
    }

    return event;
}
