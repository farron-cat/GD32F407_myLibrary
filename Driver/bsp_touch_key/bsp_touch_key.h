#ifndef BSP_TOUCH_KEY_H
#define BSP_ TOUCH_KEY_H

#include "gd32f4xx.h"
#include "msp_iic.h"

// SC12B

#define TOUCH_KEY_ADDR                         0x40

#define TOUCH_KEY_WRITE_NBYTE(addr, reg, p, n) msp_iic_write_nbyte(addr, reg, p, n)
#define TOUCH_KEY_READ_NBYTE(addr, reg, p, n)  msp_iic_read_nbyte(addr, reg, p, n)

// 特殊寄存器配置宏 移植自手册示例
#define SDA_OUT_OR_IN   TRISAbits.TRISA0 // 定义 SDA 输入输出方向
#define SCL_OUT_OR_IN   TRISAbits.TRISA2 // 定义 SCL 输入输出方向
#define SDA             LATAbits.LATA0   // 定义 SDA 数据线输出
#define SDA_IN          PORTAbits.RA0    // 定义 SDA 数据线读取
#define SCL             LATAbits.LATA2   // 定义 SDA 数据线输出
#define SC12B_ADDR      0x40             // ASEL 引脚悬空，设备的地址
#define SC12B_ADDR1     0x42             // ASEL 引脚为低电平，设备的地址
#define SC12B_ADDR2     0x44             // ASEL 引脚为高电平，设备的地址
#define SenSet0_REG     0x00             // CIN4 通道灵敏度的设置地址
#define SenSetCOM_REG   0x01             // 其他通道灵敏度的设置地址
#define CTRL0_REG       0x02             // CTRL0 控制寄存器设置地址
#define CTRL1_REG       0x03             // CTRL1 控制寄存器设置地址
#define Output_REG      0x08             // 触摸状态寄存器输出地址
#define SAMP_REG        0x0A             // 触摸数据值存器输出地址
#define RTM0            0                // 3 个采样周期有效，1 个采样周期判断无效
#define RTM1            1                // 4 个采样周期有效，2 个采样周期判断无效
#define RTM2            2                // 5 个采样周期有效，3 个采样周期判断无效
#define RTM3            3                // 6 个采样周期有效，4 个采样周期判断无效
#define KVF_STOP_CORREC (1u << 2)        // 按键有效，触摸不校准
#define KVF_50S_CORREC  (0u << 2)        // 按下有效后，50S 开始校准
#define HOLD            (1u << 3)        // 基线保持不校准
#define NOTHOLD         (0u << 3)        // 基线持续校准
#define SLPCYC_LGT      (0u << 5)        // 无穷大
#define SLPCYC_0R5T     (1u << 5)        // 休眠后采样间隔 60MS
#define SLPCYC_1R5T     (2u << 5)        // 休眠后采样间隔 180MS
#define SLPCYC_2R5T     (3u << 5)        // 休眠后采样间隔 300MS
#define SLPCYC_3R5T     (4u << 5)        // 休眠后采样间隔 420MS
#define SLPCYC_4R5T     (5u << 5)        // 休眠后采样间隔 540MS
#define SLPCYC_5R5T     (6u << 5)        // 休眠后采样间隔 660MS
#define SLPCYC_6R5T     (7u << 5)        // 休眠后采样间隔 780MS
#define FAST_TO_SLEEP   (1u << 4)        // 快速进入休眠
#define SLOW_TO_SLEEP   (0u << 4)        // 75S 进入休眠

void bsp_touch_key_init(void);

void on_touch_key_interrupt(void);

// 回调函数
extern void on_touch_key_press(uint8_t key);
extern void on_touch_key_release(uint8_t key);

#endif // BSP_TOUCH_KEY_H