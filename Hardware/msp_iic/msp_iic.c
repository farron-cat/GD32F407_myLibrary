#include "msp_iic.h"

// ============ 声    明 ============
#if IIC_HARD_SOFT_SWITCH
// 硬件IIC
static void msp_iic_hard_init(void);
static uint8_t msp_iic_hard_write_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
static uint8_t msp_iic_hard_write_col_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t offset, uint16_t len);
static uint8_t msp_iic_hard_read_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
#else
// 软件IIC
static void msp_iic_soft_init(void);
static uint8_t msp_iic_soft_write_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
static uint8_t msp_iic_soft_write_col_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t offset, uint16_t len);
static uint8_t msp_iic_soft_read_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
#endif

// ============ 外部接口 ============
// IIC初始化
void msp_iic_init(void)
{
#if IIC_HARD_SOFT_SWITCH
    // 硬件IIC
    msp_iic_hard_init();
#else
    // 软件IIC
    msp_iic_soft_init();
#endif
}

// IIC写入n个字节
uint8_t msp_iic_write_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
#if IIC_HARD_SOFT_SWITCH
    // 硬件IIC
    return msp_iic_hard_write_nbyte(addr, reg, data, len);
#else
    // 软件IIC
    return msp_iic_soft_write_nbyte(addr, reg, data, len);
#endif
}

uint8_t msp_iic_write_col_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t offset, uint16_t len)
{
#if IIC_HARD_SOFT_SWITCH
    // 硬件IIC
    return msp_iic_hard_write_col_nbyte(addr, reg, data, offset, len);
#else
    // 软件IIC
    return msp_iic_soft_write_col_nbyte(addr, reg, data, offset, len);
#endif
}

// IIC读取n个字节
uint8_t msp_iic_read_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
#if IIC_HARD_SOFT_SWITCH
    // 硬件IIC
    return msp_iic_hard_read_nbyte(addr, reg, data, len);
#else
    // 软件IIC
    return msp_iic_soft_read_nbyte(addr, reg, data, len);
#endif
}

// ============ 内部实现 ============
#if IIC_HARD_SOFT_SWITCH
// 硬件IIC

static void msp_iic_hard_gpio_config(void)
{
    // IIC0_SCL PB6 AF4
    rcu_periph_clock_enable(SCL_RCU);
    gpio_mode_set(SCL_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SCL_PIN);
    gpio_output_options_set(SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_MAX, SCL_PIN);
    gpio_af_set(SCL_PORT, IIC_SCL_AF, SCL_PIN);

    // IIC0_SDA PB7 AF4
    rcu_periph_clock_enable(SDA_RCU);
    gpio_mode_set(SDA_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SDA_PIN);
    gpio_output_options_set(SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_MAX, SDA_PIN);
    gpio_af_set(SDA_PORT, SDA_AF, SDA_PIN);
}

uint32_t i2cx = IIC_NUM;
uint32_t i2cx_rcu = IIC_RCU;
uint32_t i2cx_speed = IIC_SPEED;

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

static void msp_iic_hard_init(void)
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
static uint8_t msp_iic_hard_write_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    /************* start ***********************/
    // 等待I2C闲置
    if (I2C_waitn(I2C_FLAG_I2CBSY))
        return IIC_BUS_BSY;
    // start
    i2c_start_on_bus(i2cx);
    // 等待I2C主设备成功发送起始信号
    if (I2C_wait(I2C_FLAG_SBSEND))
        return IIC_START_FAILED;

    /************* device address **************/
    // 发送设备地址 等待响应
    i2c_master_addressing(i2cx, addr << 1, I2C_TRANSMITTER);
    // 等待地址发送完成
    if (I2C_wait(I2C_FLAG_ADDSEND))
        return IIC_SEND_ADDR_FAILED;
    i2c_flag_clear(i2cx, I2C_FLAG_ADDSEND);

    /************ register address ************/
    // 寄存器地址
    // 等待发送数据缓冲区为空
    if (I2C_wait(I2C_FLAG_TBE))
        return IIC_SEND_REG_FAILED;

    // 发送数据
    i2c_data_transmit(i2cx, reg);

    // 等待数据发送完成
    if (I2C_wait(I2C_FLAG_BTC))
        return IIC_SEND_REG_FAILED;

    /***************** data ******************/
    // 发送数据
    uint32_t i;
    for (i = 0; i < len; i++)
    {
        uint32_t d = data[i];

        // 等待发送数据缓冲区为空
        if (I2C_wait(I2C_FLAG_TBE))
            return IIC_SEND_DAT_FAILED;

        // 发送数据
        i2c_data_transmit(i2cx, d);

        // 等待数据发送完成
        if (I2C_wait(I2C_FLAG_BTC))
            return IIC_SEND_DAT_FAILED;
    }
    /***************** stop ********************/
    // stop
    i2c_stop_on_bus(i2cx);
    while (I2C_CTL0(I2C0) & I2C_CTL0_STOP)
        ;

    i2c_ack_config(i2cx, I2C_ACK_ENABLE);
    return IIC_SUC;
}

// IIC写入Nbyte  addr:7bit地址
// (uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
static uint8_t msp_iic_hard_write_col_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t offset, uint16_t len)
{
    /************* start ***********************/
    // 等待I2C闲置
    if (I2C_waitn(I2C_FLAG_I2CBSY))
        return IIC_BUS_BSY;
    // start
    i2c_start_on_bus(i2cx);
    // 等待I2C主设备成功发送起始信号
    if (I2C_wait(I2C_FLAG_SBSEND))
        return IIC_START_FAILED;

    /************* device address **************/
    // 发送设备地址 等待响应
    i2c_master_addressing(i2cx, addr << 1, I2C_TRANSMITTER);
    // 等待地址发送完成
    if (I2C_wait(I2C_FLAG_ADDSEND))
        return IIC_SEND_ADDR_FAILED;
    i2c_flag_clear(i2cx, I2C_FLAG_ADDSEND);

    /************ register address ************/
    // 寄存器地址
    // 等待发送数据缓冲区为空
    if (I2C_wait(I2C_FLAG_TBE))
        return IIC_SEND_REG_FAILED;

    // 发送数据
    i2c_data_transmit(i2cx, reg);

    // 等待数据发送完成
    if (I2C_wait(I2C_FLAG_BTC))
        return IIC_SEND_REG_FAILED;

    /***************** data ******************/
    // 发送数据
    uint32_t i;
    for (i = 0; i < len; i++)
    {
        uint32_t d = data[i * offset];

        // 等待发送数据缓冲区为空
        if (I2C_wait(I2C_FLAG_TBE))
            return IIC_SEND_DAT_FAILED;

        // 发送数据
        i2c_data_transmit(i2cx, d);

        // 等待数据发送完成
        if (I2C_wait(I2C_FLAG_BTC))
            return IIC_SEND_DAT_FAILED;
    }
    /***************** stop ********************/
    // stop
    i2c_stop_on_bus(i2cx);
    while (I2C_CTL0(I2C0) & I2C_CTL0_STOP)
        ;

    i2c_ack_config(i2cx, I2C_ACK_ENABLE);
    return IIC_SUC;
}

static uint8_t msp_iic_hard_read_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    /************* start ***********************/
    // 等待I2C空闲
    if (I2C_waitn(I2C_FLAG_I2CBSY))
        return IIC_BUS_BSY;
    // 发送启动信号
    i2c_start_on_bus(i2cx);
    // 等待I2C主设备成功发送起始信号
    if (I2C_wait(I2C_FLAG_SBSEND))
        return IIC_START_FAILED;

    /************* device address **************/
    // 发送从设备地址
    i2c_master_addressing(i2cx, addr << 1, I2C_TRANSMITTER);

    if (I2C_wait(I2C_FLAG_ADDSEND))
        return IIC_SEND_ADDR_FAILED;
    i2c_flag_clear(i2cx, I2C_FLAG_ADDSEND);

    /********** register address **************/
    // 等待发送缓冲区
    if (I2C_wait(I2C_FLAG_TBE))
        return IIC_SEND_REG_FAILED;

    // 发送寄存器地址
    i2c_data_transmit(i2cx, reg);

    // 等待发送数据完成
    if (I2C_wait(I2C_FLAG_BTC))
        return IIC_SEND_REG_FAILED;

    /************* start ***********************/
    // 发送再启动信号
    i2c_start_on_bus(i2cx);

    if (I2C_wait(I2C_FLAG_SBSEND))
        return IIC_START_FAILED;

    /************* device address **************/
    // 发送从设备地址
    i2c_master_addressing(i2cx, addr << 1, I2C_RECEIVER);
    if (I2C_wait(I2C_FLAG_ADDSEND))
        return IIC_SEND_ADDR_FAILED;
    i2c_flag_clear(i2cx, I2C_FLAG_ADDSEND);

    // ack
    i2c_ack_config(i2cx, I2C_ACK_ENABLE);
    // 接收一个数据后，自动发送ACK
    i2c_ackpos_config(i2cx, I2C_ACKPOS_CURRENT);
    // 确认ACK已启用
    if (I2C_wait(I2C_CTL0(i2cx) & I2C_CTL0_ACKEN))
        return IIC_AUTO_ACK_FAILED;

    for (uint16_t i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            // 在读取最后一个字节之前，禁用ACK，配置为自动NACK
            i2c_ack_config(i2cx, I2C_ACK_DISABLE);
        }

        // 等待接收缓冲区不为空
        if (I2C_wait(I2C_FLAG_RBNE))
            return IIC_READ_FAILED;

        data[i] = i2c_data_receive(i2cx);
    }

    /***************** stop ********************/
    i2c_stop_on_bus(i2cx);
    while (I2C_CTL0(I2C0) & I2C_CTL0_STOP)
        ;

    i2c_ack_config(i2cx, I2C_ACK_ENABLE);
    return IIC_SUC;
}

#else
// 软件IIC

static void msp_iic_soft_init(void)
{
    // 配置GPIO
    // SCL PB6
    rcu_periph_clock_enable(SCL_RCU);
    gpio_mode_set(SCL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SCL_PIN);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, SCL_PIN);

    // SDA PB7
    rcu_periph_clock_enable(SDA_RCU);
    gpio_mode_set(SDA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SDA_PIN);
    gpio_output_options_set(SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, SDA_PIN);
}

// 开始信号
static void msp_iic_start(void)
{
    // SCL高电平时，SDA下降沿

    IIC_SDA_OUT;
    // SDA高电平持续
    IIC_SDA_H;
    IIC_DELAY;
    // SCL高电平持续
    IIC_SCL_H;
    IIC_DELAY;
    // SDA低电平持续
    IIC_SDA_L;
    IIC_DELAY;
    // SCL低电平持续
    IIC_SCL_L;
    IIC_DELAY;
}

// 结束信号
static void msp_iic_stop(void)
{
    // SCL高电平时，SDA上升沿

    IIC_SDA_OUT;
    // SDA低电平持续
    IIC_SDA_L;
    IIC_DELAY;
    // SCL高电平持续
    IIC_SCL_H;
    IIC_DELAY;
    // SDA高电平持续
    IIC_SDA_H;
    IIC_DELAY;
}

// 发送一个字节 高位先行
static void msp_iic_send_byte(uint8_t byte)
{
    // SDA在SCL高电平时发送数据
    // 从高位开始发送，每次发送一位

    IIC_SDA_OUT;
    // SCL低电平持续
    IIC_SCL_L;
    IIC_DELAY;

    for (uint8_t i = 0; i < 8; i++)
    {
        // SDA根据数据配置电平
        if ((byte >> (7 - i)) & 0x01)
        {
            IIC_SDA_H;
        }
        else
        {
            IIC_SDA_L;
        }

        // SCL高电平持续
        IIC_SCL_H;
        IIC_DELAY;

        // SCL低电平持续
        IIC_SCL_L;
        IIC_DELAY;
    }
}

// 等待应答 0收到从机应答 1未收到从机应答
static uint8_t msp_iic_wait_ack(void)
{
    // SCL确定会在低电平，可以改变SDA电平
    // 主机主动拉低SDA，交出控制权

    IIC_SDA_OUT;
    // SDA高电平持续
    // 主动拉高SDA，保证后续拉低的回复是可信的
    IIC_SDA_H;
    IIC_DELAY;

    // 交出控制权
    IIC_SDA_IN;
    IIC_DELAY;

    // SCL高电平持续
    IIC_SCL_H;
    IIC_DELAY;

    // 检查SDA电平
    if (IIC_SDA_STA)
    {
        return 1;
    } // 读 SDA
    IIC_SCL_L;
    IIC_DELAY; // 成功时拉低 SCL
    return 0;
}

// 写入n字节
// addr 7bit地址 + 1bit读写位
static uint8_t msp_iic_soft_write_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    // 起始信号
    msp_iic_start();
    // 设备地址（写地址）
    msp_iic_send_byte(addr << 1 | 0x0);
    // 等待响应
    if (msp_iic_wait_ack())
    {
        printf("IIC device not found!\n");
        return IIC_SEND_ADDR_FAILED;
    }
    // 寄存器地址
    msp_iic_send_byte(reg);
    // 等待响应
    if (msp_iic_wait_ack())
    {
        printf("IIC device not acknowledged!\n");
        return IIC_SEND_REG_FAILED;
    }
    // 循环发送数据
    for (uint16_t i = 0; i < len; i++)
    {
        msp_iic_send_byte(data[i]);
        if (msp_iic_wait_ack())
        {
            printf("IIC device not acknowledged\n");
            return IIC_SEND_DAT_FAILED;
        }
    }
    // 停止信号
    msp_iic_stop();
    return IIC_SUC;
}

// 按列写入n字节
// addr 7bit地址 + 1bit读写位
static uint8_t msp_iic_soft_write_col_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t offset, uint16_t len)
{
    // 起始信号
    msp_iic_start();
    // 设备地址（写地址）
    msp_iic_send_byte(addr << 1 | 0x0);
    // 等待响应
    if (msp_iic_wait_ack())
    {
        printf("IIC device not found!\n");
        return IIC_SEND_ADDR_FAILED;
    }
    // 寄存器地址
    msp_iic_send_byte(reg);
    // 等待响应
    if (msp_iic_wait_ack())
    {
        printf("IIC device not acknowledged!\n");
        return IIC_SEND_REG_FAILED;
    }
    // 循环发送数据
    for (uint16_t i = 0; i < len; i++)
    {
        msp_iic_send_byte(data[i * offset]);
        if (msp_iic_wait_ack())
        {
            printf("IIC device not acknowledged\n");
            return IIC_SEND_DAT_FAILED;
        }
    }
    // 停止信号
    msp_iic_stop();
    return IIC_SUC;
}

// 接收一字节 高位先行
static uint8_t msp_iic_recv_byte(void)
{
    uint8_t byte = 0;

    IIC_SDA_IN;
    // SCL低电平持续
    IIC_SCL_L;
    IIC_DELAY;

    for (uint8_t i = 0; i < 8; i++)
    {

        // 等待从机写入SDA

        // SCL高电平持续
        IIC_SCL_H;

        // 读取SDA电平
        if (IIC_SDA_STA)
        {
            byte |= (0x01 << (7 - i));
        }
        else
        {
            byte &= ~(0x01 << (7 - i));
        }
        IIC_DELAY;

        // SCL低电平持续
        IIC_SCL_L;
        IIC_DELAY;
    }
    return byte;
}

// 发送应答
static void msp_iic_send_ack(void)
{
    IIC_SDA_OUT;

    // SDA低电平持续
    IIC_SDA_L;
    IIC_DELAY;

    // SCL高电平持续
    IIC_SCL_H;
    IIC_DELAY;

    // 等待从机读取

    // SCL低电平持续
    IIC_SCL_L;
    IIC_DELAY;
}

// 发送空应答
static void msp_iic_send_nack(void)
{
    IIC_SDA_OUT;

    // SDA高电平持续
    IIC_SDA_H;
    IIC_DELAY;

    // SCL高电平持续
    IIC_SCL_H;
    IIC_DELAY;

    // 等待从机读取

    // SCL低电平持续
    IIC_SCL_L;
    IIC_DELAY;
}

// 读取n字节
// addr 7bit地址 + 1bit读写位
static uint8_t msp_iic_soft_read_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    // 起始信号
    msp_iic_start();
    // 设备地址（写地址）
    msp_iic_send_byte(addr << 1 | 0x0);
    // 等待响应
    if (msp_iic_wait_ack())
    {
        printf("read1: IIC device not found!\n");
        return IIC_SEND_ADDR_FAILED;
    }
    // 寄存器地址
    msp_iic_send_byte(reg);
    // 等待响应
    if (msp_iic_wait_ack())
    {
        printf("read2: IIC device not found!\n");
        return IIC_SEND_REG_FAILED;
    }

    // 起始信号
    msp_iic_start();
    // 设备地址（读地址）
    msp_iic_send_byte(addr << 1 | 0x1);
    // 等待响应
    if (msp_iic_wait_ack())
    {
        printf("read3: IIC device not found!\n");
        return IIC_SEND_ADDR_FAILED;
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
    return IIC_SUC;
}

#endif