#include "msp_iic.h"

void msp_iic_init(void)
{
    // 配置GPIO
    // SCL PB6
    // SDA PB7
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_6 | GPIO_PIN_7);

    //
}

void msp_iic_start(void)
{
    // SCL高电平时，SDA下降沿

    SDA_OUT;
    // SDA高电平持续
    SDA_H;
    IIC_DELAY;
    // SCL高电平持续
    SCL_H;
    IIC_DELAY;
    // SDA低电平持续
    SDA_L;
    IIC_DELAY;
    // SCL低电平持续
    SCL_L;
    IIC_DELAY;
}

static void msp_iic_stop(void)
{
    // SCL高电平时，SDA上升沿

    SDA_OUT;
    // SDA低电平持续
    SDA_L;
    IIC_DELAY;
    // SCL高电平持续
    SCL_H;
    IIC_DELAY;
    // SDA高电平持续
    SDA_H;
    IIC_DELAY;
}

static void msp_iic_send_byte(uint8_t byte)
{
    // SDA在SCL高电平时发送数据
    // 从高位开始发送，每次发送一位

    SDA_OUT;
    // SCL低电平持续
    SCL_L;
    IIC_DELAY;

    for (uint8_t i = 0; i < 8; i++)
    {
        // SDA根据数据配置电平
        if ((byte >> (7 - i)) & 0x01)
        {
            SDA_H;
        }
        else
        {
            SDA_L;
        }

        // SCL高电平持续
        SCL_H;
        IIC_DELAY;

        // SCL低电平持续
        SCL_L;
        IIC_DELAY;
    }
}

static uint8_t msp_iic_wait_ack(void)
{
    // SCL确定会在低电平，可以改变SDA电平
    // 主机主动拉低SDA，交出控制权

    SDA_OUT;
    // SDA高电平持续
    // 主动拉高SDA，保证后续拉低的回复是可信的
    SDA_H;
    IIC_DELAY;

    // 交出控制权
    SDA_IN;
    IIC_DELAY;

    // SCL高电平持续
    SCL_H;
    IIC_DELAY;

    // 检查SDA电平
    if (SDA_STA)
    {
        return 1;
    } // 读 SDA
    SCL_L;
    IIC_DELAY; // 成功时拉低 SCL
    return 0;
}

// addr 7bit地址 + 1bit读写位
void msp_iic_write_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    // 起始信号
    msp_iic_start();
    // 设备地址（写地址）
    msp_iic_send_byte(addr << 1 | 0x0);
    // 等待响应
    if (msp_iic_wait_ack())
    {
        printf("IIC device not found!\n");
        return;
    }
    // 寄存器地址
    msp_iic_send_byte(reg);
    // 等待响应
    if (msp_iic_wait_ack())
    {
        printf("IIC device not acknowledged!\n");
        return;
    }
    // 循环发送数据
    for (uint16_t i = 0; i < len; i++)
    {
        msp_iic_send_byte(data[i]);
        if (msp_iic_wait_ack())
        {
            printf("IIC device not acknowledged");
            return;
        }
    }
    // 停止信号
    msp_iic_stop();
}

static uint8_t msp_iic_recv_byte(void)
{
    uint8_t byte = 0;

    SDA_IN;
    // SCL低电平持续
    SCL_L;
    IIC_DELAY;

    for (uint8_t i = 0; i < 8; i++)
    {

        // 等待从机写入SDA

        // SCL高电平持续
        SCL_H;

        // 读取SDA电平
        if (SDA_STA)
        {
            byte |= (0x01 << (7 - i));
        }
        else
        {
            byte &= ~(0x01 << (7 - i));
        }
        IIC_DELAY;

        // SCL低电平持续
        SCL_L;
        IIC_DELAY;
    }
    return byte;
}

static void msp_iic_send_ack(void)
{
    SDA_OUT;

    // SDA低电平持续
    SDA_L;
    IIC_DELAY;

    // SCL高电平持续
    SCL_H;
    IIC_DELAY;

    // 等待从机读取

    // SCL低电平持续
    SCL_L;
    IIC_DELAY;
}

static void msp_iic_send_nack(void)
{
    SDA_OUT;

    // SDA高电平持续
    SDA_H;
    IIC_DELAY;

    // SCL高电平持续
    SCL_H;
    IIC_DELAY;

    // 等待从机读取

    // SCL低电平持续
    SCL_L;
    IIC_DELAY;
}

// addr 7bit地址 + 1bit读写位
void msp_iic_read_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    // 起始信号
    msp_iic_start();
    // 设备地址（写地址）
    msp_iic_send_byte(addr << 1 | 0x0);
    // 等待响应
    if (msp_iic_wait_ack())
    {
        printf("read1: IIC device not found!\n");
        return;
    }
    // 寄存器地址
    msp_iic_send_byte(reg);
    // 等待响应
    if (msp_iic_wait_ack())
    {
        printf("read2: IIC device not found!\n");
        return;
    }

    // 起始信号
    msp_iic_start();
    // 设备地址（读地址）
    msp_iic_send_byte(addr << 1 | 0x1);
    // 等待响应
    if (msp_iic_wait_ack())
    {
        printf("read3: IIC device not found!\n");
        return;
    }
    // 循环读取数据
    for (uint16_t i = 0; i < len - 1; i++)
    {
        // 读取
        data[i] = msp_iic_recv_byte();
        // 发送响应
        msp_iic_send_ack();
    }
    // 读取最后一字节
    data[len - 1] = msp_iic_recv_byte();
    // 发送空响应
    msp_iic_send_nack();
    // 停止信号
    msp_iic_stop();
}