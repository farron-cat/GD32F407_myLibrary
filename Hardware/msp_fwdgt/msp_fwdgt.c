#include "msp_fwdgt.h"
#include "systick.h"
#include <stdio.h>

// 独立看门狗
void msp_fwdgt_config(void)
{
    // 打开时钟
    rcu_osci_off(RCU_IRC32K);
    delay_1ms(1);
    rcu_osci_on(RCU_IRC32K);

    if (rcu_osci_stab_wait(RCU_IRC32K) == ERROR)
    { // 等待稳定
        printf("turn_on_osci_error\n");
        return;
    }

    // 写使能
    fwdgt_write_enable();
    // 配置分频系数 32000 / 32 = 1000
    fwdgt_prescaler_value_config(FWDGT_PSC_DIV32);
    // 配置重装载值
    fwdgt_reload_value_config(100);
    // 启动看门狗
    fwdgt_counter_reload();
    fwdgt_enable();
}