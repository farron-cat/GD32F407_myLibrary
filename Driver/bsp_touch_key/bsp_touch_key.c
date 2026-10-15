#include "bsp_touch_key.h"
#include "util_weak.h"

void bsp_touch_key_init(void)
{
    // 特殊功能寄存器配置 移植自手册示例程序
    unsigned char databuf;
    databuf = 0x79;
    TOUCH_KEY_WRITE_NBYTE(SC12B_ADDR, SenSet0_REG, &databuf, 1);
    databuf = 0x79;
    TOUCH_KEY_WRITE_NBYTE(SC12B_ADDR, SenSetCOM_REG, &databuf, 1);
    databuf = SLPCYC_3R5T | SLOW_TO_SLEEP | HOLD | KVF_50S_CORREC | RTM3;
    TOUCH_KEY_WRITE_NBYTE(SC12B_ADDR, CTRL0_REG, &databuf, 1);
    databuf = 0b1000;
    TOUCH_KEY_WRITE_NBYTE(SC12B_ADDR, CTRL1_REG, &databuf, 1);
}

__weak void on_touch_key_press(uint8_t key)
{
    printf("key_%d press\n", (int)key);
}

__weak void on_touch_key_release(uint8_t key)
{
    printf("key_%d release\n", (int)key);
}

static uint16_t pre_key_state = 0x0000;
void on_touch_key_interrupt(void)
{
    // 读取按键值
    uint8_t key_data[2];
    TOUCH_KEY_READ_NBYTE(TOUCH_KEY_ADDR, 0x08, key_data, 2);
    printf("touch key value: %#x %#x\n", key_data[0], key_data[1]);

    // key_data[0] 0x CH0 CH1 CH2 CH3  CH4 CH5 CH6 CH7
    // key_data[1] 0x CH8 CH9 CH10 CH11 0000
    // state 0x 0000 CH0 CH1 CH2 CH3  CH4 CH5 CH6 CH7  CH8 CH9 CH10 CH11
    uint16_t key_state = (key_data[0] << 4) | (key_data[1] >> 4);
    uint16_t key_change = key_state ^ pre_key_state;
    for (int8_t i = 0; i < 12; i++)
    {
        if ((key_change >> i) & 0x0001)
        {
            // 按下
            if ((key_state >> i) & 0x0001)
            {
                on_touch_key_press(12 - i);
            }
            // 松开
            else
            {
                on_touch_key_release(12 - i);
            }
        }
    }
    pre_key_state = key_state;
}