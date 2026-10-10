#include "msp_wwdgt.h"

// 窗口看门狗
void msp_wwdgt_config(void)
{
    // 打开时钟
    rcu_periph_clock_enable(RCU_WWDGT);
    // 42mhz
    // 42000000hz / 4096 = 10,253.90625hz
    // 10,253.90625hz / 1 = 10,253.90625ms
    // 10,253.90625hz <=> 1000000us
    // 数1个数 <=> 97.523us
    // 配置窗口看门狗属性
    // 最短喂狗时间   47*97.5us = 4582.5us
    // 最长喂狗时间   64*97.5us = 6241.5us
    wwdgt_config(0x7F, 0x50, WWDGT_CFG_PSC_DIV1);
    // 启动看门狗
    wwdgt_enable();
}