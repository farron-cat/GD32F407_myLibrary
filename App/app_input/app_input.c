#include "app.h"

void app_input_init(void)
{
    ;
}

// 按键任务
void app_input_key_task(void)
{
}

//
void app_input_adkey_task(void)
{
    ;
}

void app_input_touch_key_task(void)
{
    // 触摸按键中断事件处理
    if (g_touch_key_int_flag)
    {
        g_touch_key_int_flag = 0;
        on_touch_key_interrupt();
    }
}

void app_input_keypad_task(void)
{
    // 矩阵键盘扫描 与188数码管引脚冲突，不能同时使用
    int key = KeypadScan();
    if (key != 255)
        printf("key: %d\n", key);
}
