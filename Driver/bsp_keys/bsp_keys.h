#ifndef BSP_KEYS_H
#define BSP_KEYS_H

#include "gd32f4xx.h"
#include "gd32f4xx_rcu.h"

// 按键编号枚举
typedef enum {
    KEY1 = 0,
    KEY2,
    KEY3,
    KEY4,
    KEY_MAX // KEY_MAX 放最后正好是按键总数4
} KEY_NUM;

// 按键事件类型
typedef enum {
    KEY_EVENT_NONE = 0, // 无事件
    KEY_EVENT_PRESSED,  // 下降沿 按下
    KEY_EVENT_RELEASED  // 上升沿 松开
} KEY_EVENT;

// 按键引脚参数结构体
typedef struct {
    rcu_periph_enum rcu;
    uint32_t port;
    uint32_t pin;

} KEY_PARAM;

// 按键按下回调
extern void on_key_press(KEY_NUM key);

// 按键松开回调
extern void on_key_release(KEY_NUM key);

// 初始化独立按键
void bsp_keys_config();

// 获取按键状态
FlagStatus bsp_keys_get_state(KEY_NUM key);

// 扫描按键，返回事件枚举
KEY_EVENT bsp_key_scan(KEY_NUM key);

// 扫描所有按键
void bsp_keys_scan();
#endif // BSP_KEYS_H
