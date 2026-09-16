#include "bsp_keys.h"

// 抽取出按键引脚参数便于修改
KEY_PARAM keys[] = {
    {RCU_GPIOC, GPIOC, GPIO_PIN_0},   // KEY1
    {RCU_GPIOC, GPIOC, GPIO_PIN_1},   // KEY2
    {RCU_GPIOC, GPIOC, GPIO_PIN_2},   // KEY3
    {RCU_GPIOC, GPIOC, GPIO_PIN_3},   // KEY4
};

// 保存每个按键上一次的电平状态，默认为高电平
static FlagStatus key_preSta[KEY_MAX] = {SET, SET, SET, SET};

// 内部GPIO初始化函数
static void GPIO_config(rcu_periph_enum rcu, uint32_t port, uint32_t pin)
{
    rcu_periph_clock_enable(rcu);
    gpio_mode_set(port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, pin);
}

// 初始化独立按键
void bsp_keys_config()
{
    for (uint8_t i = 0; i < KEY_MAX; i++) {
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
    if(key_preSta[key] == SET && curSta == RESET)
    {
        event = KEY_EVENT_PRESSED;
        key_preSta[key] = curSta;
    }
    // 上升沿 松开
    else if (key_preSta[key] == RESET && curSta == SET)
    {
        event = KEY_EVENT_RELEASED;
        key_preSta[key] = curSta;
    }
    
    return event;
}
