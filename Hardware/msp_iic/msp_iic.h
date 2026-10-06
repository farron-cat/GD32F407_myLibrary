#ifndef MSP_IIC_H
#define MSP_IIC_H

#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#define SCL_RCU  RCU_GPIOB
#define SCL_PORT GPIOB
#define SCL_PIN  GPIO_PIN_6
#define SDA_RCU  RCU_GPIOB
#define SDA_PORT GPIOB
#define SDA_PIN  GPIO_PIN_7

typedef enum {
    IIC_SUC = 0,          // 成功
    IIC_SEND_ADDR_FAILED, // 发送设备地址失败
    IIC_SEND_REG_FAILED,  // 发送寄存器失败
    IIC_SEND_DAT_FAILED,  // 发送数据失败
    IIC_BUS_BSY,          // 总线繁忙
    IIC_START_FAILED,     // 发送起始信号失败
    IIC_AUTO_ACK_FAILED,  // 启动ACK自动应答失败
    IIC_READ_FAILED       // 读取1byte数据失败
} IIC_STA;

// 软硬件IIC选择
// 1:硬件IIC  0:软件IIC
#define IIC_HARD_SOFT_SWITCH 0

#if IIC_HARD_SOFT_SWITCH
// 硬件IIC

#define IIC_SCL_AF GPIO_AF_4
#define SDA_AF     GPIO_AF_4

#define IIC_RCU    RCU_I2C0
#define IIC_NUM    I2C0
#define IIC_SPEED  400000

#else
// 软件IIC

#define IIC_SCL_H   gpio_bit_set(GPIOB, GPIO_PIN_6)
#define IIC_SCL_L   gpio_bit_reset(GPIOB, GPIO_PIN_6)

#define IIC_SDA_H   gpio_bit_set(GPIOB, GPIO_PIN_7)
#define IIC_SDA_L   gpio_bit_reset(GPIOB, GPIO_PIN_7)

#define IIC_SDA_IN  gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_7)
#define IIC_SDA_OUT gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_7)
#define IIC_SDA_STA gpio_input_bit_get(GPIOB, GPIO_PIN_7)

// 100kbit/s 100000 <=> 1000000us
//  1 <=> 10us
//  SCL高低分别持续5us
#define IIC_DELAY   delay_1us(2)

#endif

// IIC初始化
void msp_iic_init(void);

// IIC写入n个字节
uint8_t msp_iic_write_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);

// 适配OLED，按列发送len个字节，offset填入data的列数
uint8_t msp_iic_write_col_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t offset, uint16_t len);

// IIC读取n个字节
uint8_t msp_iic_read_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);

#endif // MSP_IIC_H