#include "msp_iic_hard.h"

static void msp_iic_hard_gpio_config(void)
{
    // IIC0_SCL PB6
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_MAX, GPIO_PIN_6);
    gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_6);

    // IIC0_SDA PB7
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_MAX, GPIO_PIN_7);
    gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_7);
}

uint32_t i2cx = I2C0;
uint32_t i2cx_rcu = RCU_I2C0;
uint32_t i2cx_speed = 400000;

static void msp_iic_hard_iic_config(void)
{
    i2c_deinit(i2cx);
    // 时钟配置
    rcu_periph_clock_enable(i2cx_rcu);
    // I2C速率配置
    i2c_clock_config(i2cx, i2cx_speed, I2C_DTCY_2);

    // 使能i2c
    // i2c_mode_addr_config(i2cx, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00);
    i2c_enable(i2cx);

    // i2c ack enable
    i2c_ack_config(i2cx, I2C_ACK_ENABLE);
}

void msp_iic_hard_init(void)
{
    msp_iic_hard_gpio_config();
    msp_iic_hard_iic_config();
}

#define TIMEOUT 50000
static uint8_t I2C_wait(uint32_t flag)
{
    uint16_t cnt = 0;

    while (!i2c_flag_get(i2cx, flag))
    {
        cnt++;
        if (cnt > TIMEOUT)
            return 1;
    }
    return 0;
}

static uint8_t I2C_waitn(uint32_t flag)
{
    uint16_t cnt = 0;

    while (i2c_flag_get(i2cx, flag))
    {
        cnt++;
        if (cnt > TIMEOUT)
            return 1;
    }
    return 0;
}

// IIC写入Nbyte  addr:7bit地址
// (uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
uint8_t msp_iic_hard_write_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    /************* start ***********************/
    // 等待I2C闲置
    if (I2C_waitn(I2C_FLAG_I2CBSY))
        return 1;
    // start
    i2c_start_on_bus(i2cx);
    // 等待I2C主设备成功发送起始信号
    if (I2C_wait(I2C_FLAG_SBSEND))
        return 2;

    /************* device address **************/
    // 发送设备地址 等待响应
    i2c_master_addressing(i2cx, addr << 1, I2C_TRANSMITTER);
    // 等待地址发送完成
    if (I2C_wait(I2C_FLAG_ADDSEND))
        return 3;
    i2c_flag_clear(i2cx, I2C_FLAG_ADDSEND);

    /************ register address ************/
    // 寄存器地址
    // 等待发送数据缓冲区为空
    if (I2C_wait(I2C_FLAG_TBE))
        return 4;

    // 发送数据
    i2c_data_transmit(i2cx, reg);

    // 等待数据发送完成
    if (I2C_wait(I2C_FLAG_BTC))
        return 5;

    /***************** data ******************/
    // 发送数据
    uint32_t i;
    for (i = 0; i < len; i++)
    {
        uint32_t d = data[i];

        // 等待发送数据缓冲区为空
        if (I2C_wait(I2C_FLAG_TBE))
            return 6;

        // 发送数据
        i2c_data_transmit(i2cx, d);

        // 等待数据发送完成
        if (I2C_wait(I2C_FLAG_BTC))
            return 7;
    }
    /***************** stop ********************/
    // stop
    i2c_stop_on_bus(i2cx);
    while (I2C_CTL0(I2C0) & I2C_CTL0_STOP)
        ;

    i2c_ack_config(i2cx, I2C_ACK_ENABLE);
    return 0;
}

uint8_t msp_iic_hard_read_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    /************* start ***********************/
    // 等待I2C空闲
    if (I2C_waitn(I2C_FLAG_I2CBSY))
        return 1;
    // 发送启动信号
    i2c_start_on_bus(i2cx);
    // 等待I2C主设备成功发送起始信号
    if (I2C_wait(I2C_FLAG_SBSEND))
        return 2;

    /************* device address **************/
    // 发送从设备地址
    i2c_master_addressing(i2cx, addr << 1, I2C_TRANSMITTER);

    if (I2C_wait(I2C_FLAG_ADDSEND))
        return 3;
    i2c_flag_clear(i2cx, I2C_FLAG_ADDSEND);

    /********** register address **************/
    // 等待发送缓冲区
    if (I2C_wait(I2C_FLAG_TBE))
        return 4;

    // 发送寄存器地址
    i2c_data_transmit(i2cx, reg);

    // 等待发送数据完成
    if (I2C_wait(I2C_FLAG_BTC))
        return 5;

    /************* start ***********************/
    // 发送再启动信号
    i2c_start_on_bus(i2cx);

    if (I2C_wait(I2C_FLAG_SBSEND))
        return 6;

    /************* device address **************/
    // 发送从设备地址
    i2c_master_addressing(i2cx, addr << 1, I2C_RECEIVER);
    if (I2C_wait(I2C_FLAG_ADDSEND))
        return 7;
    i2c_flag_clear(i2cx, I2C_FLAG_ADDSEND);

    // ack
    i2c_ack_config(i2cx, I2C_ACK_ENABLE);
    // 接收一个数据后，自动发送ACK
    i2c_ackpos_config(i2cx, I2C_ACKPOS_CURRENT);
    // 确认ACK已启用
    if (I2C_wait(I2C_CTL0(i2cx) & I2C_CTL0_ACKEN))
        return 8;

    for (uint16_t i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            // 在读取最后一个字节之前，禁用ACK，配置为自动NACK
            i2c_ack_config(i2cx, I2C_ACK_DISABLE);
        }

        // 等待接收缓冲区不为空
        if (I2C_wait(I2C_FLAG_RBNE))
            return 9;

        data[i] = i2c_data_receive(i2cx);
    }

    /***************** stop ********************/
    i2c_stop_on_bus(i2cx);
    while (I2C_CTL0(I2C0) & I2C_CTL0_STOP)
        ;

    i2c_ack_config(i2cx, I2C_ACK_ENABLE);
    return 0;
}