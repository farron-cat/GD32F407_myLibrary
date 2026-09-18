#include "msp_uart.h"
#include <stdio.h>

#define USART_RECEIVE_LENGTH 1024
// 串口接收缓冲区大小
uint8_t g_usart0_recv_buff[USART_RECEIVE_LENGTH]; // 接收缓冲区
// 接收到字符存放的位置
int g_usart0_recv_length = 0;

// 串口接收缓冲区大小
uint8_t g_usart2_recv_buff[USART_RECEIVE_LENGTH]; // 接收缓冲区
// 接收到字符存放的位置
int g_usart2_recv_length = 0;

static void GPIO_config()
{
    uint32_t usartx_tx_rcu = RCU_GPIOA;
    uint32_t usartx_tx_port = GPIOA;
    uint32_t usartx_tx_pin = GPIO_PIN_9;
    uint32_t usartx_tx_af = GPIO_AF_7;

    uint32_t usartx_rx_rcu = RCU_GPIOA;
    uint32_t usartx_rx_port = GPIOA;
    uint32_t usartx_rx_pin = GPIO_PIN_10;
    uint32_t usartx_rx_af = GPIO_AF_7;

    //================ GPIO配置 ================
    // tx
    // 1.打开时钟
    rcu_periph_clock_enable(usartx_tx_rcu);
    // 2.GPIO复用模式配置 GPIO_MODE_AF
    gpio_mode_set(usartx_tx_port, GPIO_MODE_AF, GPIO_PUPD_NONE, usartx_tx_pin);
    // 3.复用功能配置 GPIO_AF_7
    gpio_af_set(usartx_tx_port, usartx_tx_af, usartx_tx_pin);
    // 4.tx需要配置输出模式选项
    gpio_output_options_set(usartx_tx_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, usartx_tx_pin);

    // rx
    // 1.打开时钟
    rcu_periph_clock_enable(usartx_rx_rcu); // 配置时钟
    // 2.GPIO复用模式配置 GPIO_MODE_AF
    gpio_mode_set(usartx_rx_port, GPIO_MODE_AF, GPIO_PUPD_NONE, usartx_rx_pin);
    // 3.复用功能配置 GPIO_AF_7
    gpio_af_set(usartx_rx_port, usartx_rx_af, usartx_rx_pin);
    // gpio_output_options_set(usartx_rx_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, usartx_rx_pin);
}

static void USART_config()
{
    uint32_t usartx = USART0;
    uint32_t usartx_rcu = RCU_USART0;
    uint32_t usartx_irqn = USART0_IRQn;

    uint32_t usartx_p_baudrate = 115200;
    uint32_t usartx_p_parity = USART_PM_NONE;
    uint32_t usartx_p_wl = USART_WL_8BIT;
    uint32_t usartx_p_stop_bit = USART_STB_1BIT;
    uint32_t usartx_p_data_first = USART_MSBF_LSB;

    //================ USART 配置 ================
    // 1.打开USART外设时钟
    rcu_periph_clock_enable(RCU_USART0);

    // 2.USART属性配置
    // USART复位
    usart_deinit(usartx);

    usart_baudrate_set(usartx, usartx_p_baudrate);        // 波特率
    usart_parity_config(usartx, usartx_p_parity);         // 校验位
    usart_word_length_set(usartx, usartx_p_wl);           // 数据位数
    usart_stop_bit_set(usartx, usartx_p_stop_bit);        // 停止位
    usart_data_first_config(usartx, usartx_p_data_first); // 先发送高位还是低位

    // 3.发送和接收配置
    // 发送功能配置
    usart_transmit_config(usartx, USART_TRANSMIT_ENABLE);
    // 接收功能配置
    usart_receive_config(usartx, USART_RECEIVE_ENABLE);

    // 4.打开中断
    // NVIC中注册中断
    nvic_irq_enable(usartx_irqn, 2, 2);
    // read data buffer not empty interrupt
    // 读取数据缓冲区不为空中断 （一帧数据结束）
    usart_interrupt_enable(usartx, USART_INT_RBNE);
    // IDLE line detected interrupt
    // 检测到空闲中断   （一包数据结束）
    usart_interrupt_enable(usartx, USART_INT_IDLE);

    // 5.USART使能
    usart_enable(usartx);
}

void USART0_config()
{
    GPIO_config();
    USART_config();
}

void USART2_config()
{
    uint32_t usartx_tx_rcu = RCU_GPIOB;
    uint32_t usartx_tx_port = GPIOB;
    uint32_t usartx_tx_pin = GPIO_PIN_10;
    uint32_t usartx_tx_af = GPIO_AF_7;

    uint32_t usartx_rx_rcu = RCU_GPIOB;
    uint32_t usartx_rx_port = GPIOB;
    uint32_t usartx_rx_pin = GPIO_PIN_11;
    uint32_t usartx_rx_af = GPIO_AF_7;

    uint32_t usartx = USART2;
    uint32_t usartx_rcu = RCU_USART2;
    uint32_t usartx_irqn = USART2_IRQn;

    uint32_t usartx_p_baudrate = 115200;
    uint32_t usartx_p_parity = USART_PM_NONE;
    uint32_t usartx_p_wl = USART_WL_8BIT;
    uint32_t usartx_p_stop_bit = USART_STB_1BIT;
    uint32_t usartx_p_data_first = USART_MSBF_LSB;

    //================ GPIO配置 ================
    // tx
    // 1.打开时钟
    rcu_periph_clock_enable(usartx_tx_rcu);
    // 2.GPIO复用模式配置 GPIO_MODE_AF
    gpio_mode_set(usartx_tx_port, GPIO_MODE_AF, GPIO_PUPD_NONE, usartx_tx_pin);
    // 3.复用功能配置 GPIO_AF_7
    gpio_af_set(usartx_tx_port, usartx_tx_af, usartx_tx_pin);
    // 4.tx需要配置输出模式选项
    gpio_output_options_set(usartx_tx_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, usartx_tx_pin);

    // rx
    // 1.打开时钟
    rcu_periph_clock_enable(usartx_rx_rcu); // 配置时钟
    // 2.GPIO复用模式配置 GPIO_MODE_AF
    gpio_mode_set(usartx_rx_port, GPIO_MODE_AF, GPIO_PUPD_NONE, usartx_rx_pin);
    // 3.复用功能配置 GPIO_AF_7
    gpio_af_set(usartx_rx_port, usartx_rx_af, usartx_rx_pin);
    // gpio_output_options_set(usartx_rx_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, usartx_rx_pin);

    //================ USART 配置 ================
    // 1.打开USART外设时钟
    rcu_periph_clock_enable(RCU_USART2);

    // 2.USART属性配置
    // USART复位
    usart_deinit(usartx);

    usart_baudrate_set(usartx, usartx_p_baudrate);        // 波特率
    usart_parity_config(usartx, usartx_p_parity);         // 校验位
    usart_word_length_set(usartx, usartx_p_wl);           // 数据位数
    usart_stop_bit_set(usartx, usartx_p_stop_bit);        // 停止位
    usart_data_first_config(usartx, usartx_p_data_first); // 先发送高位还是低位

    // 3.发送和接收配置
    // 发送功能配置
    usart_transmit_config(usartx, USART_TRANSMIT_ENABLE);
    // 接收功能配置
    usart_receive_config(usartx, USART_RECEIVE_ENABLE);

    // 4.打开中断
    // NVIC中注册中断
    nvic_irq_enable(usartx_irqn, 2, 2);
    // read data buffer not empty interrupt
    // 读取数据缓冲区不为空中断 （一帧数据结束）
    usart_interrupt_enable(usartx, USART_INT_RBNE);
    // IDLE line detected interrupt
    // 检测到空闲中断   （一包数据结束）
    usart_interrupt_enable(usartx, USART_INT_IDLE);

    // 5.USART使能
    usart_enable(usartx);
}

// 发送一byte数据
void send_byte(uint32_t usartx, uint8_t data)
{
    // 通过USART发送
    usart_data_transmit(usartx, data);
    // 判断缓冲区是否已经空了
    // FlagStatus state = usart_flag_get(USART_NUM,USART_FLAG_TBE);
    while (RESET == usart_flag_get(usartx, USART_FLAG_TBE))
        ;
}

// 发送多个byte数据
void send_data(uint32_t usartx, uint8_t *data, uint32_t len)
{
    while (data && len--)
    {
        send_byte(usartx, *data);
        data++;
    }
}

// 发送字符串
void send_string(uint32_t usartx, char *data)
{
    // 满足: 1.data指针不为空  2.发送的数据不是\0结束标记
    while (data && *data)
    {
        send_byte(usartx, (uint8_t)(*data));
        data++;
    }
}

// 重写fputc方法  调用printf,会自动调用这个方法实现打印
int fputc(int ch, FILE *f)
{
    send_byte(USART0, (uint8_t)ch);
    return ch;
}

#ifndef __weak
#define __weak __attribute__((weak)) // GCC / armclang
#endif

// 接收回调
// 弱引用  可以被其他文件重定义
__weak void on_usart0_recv(uint8_t *data, uint32_t len)
{
    // 结尾添加'\0'便于字符串处理
    g_usart0_recv_buff[g_usart0_recv_length] = '\0';

    // g_recv_buff为接收的数据，g_recv_length为接收的长度
    // 原样发送回去
    printf("rec:%s", data);
}

// USART0中断处理函数必须叫这个
// 在startup.s中配置了中断向量表 line124
// 中断中不能写耗时的任务
void USART0_IRQHandler(void)
{
    // 处理读取数据缓冲区不为空中断 （每接收一个字节（一帧）就会有一个RBNE中断）
    if ((usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)) == SET)
    {
        // 一定要手动清空标志位
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);

        // 存入一个字节到接收缓冲区
        uint16_t value = usart_data_receive(USART0);
        g_usart0_recv_buff[g_usart0_recv_length] = value;
        g_usart0_recv_length++;
    }

    // 处理检测到空闲中断 （一包数据接收完成产生一个IDLE中断）
    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE) == SET)
    {
        // 读取缓冲区,清空缓冲区

        // 软件先读USART_STATO,再读USART_DATA可清除该位
        usart_data_receive(USART0);

        // 回调函数
        on_usart0_recv(g_usart0_recv_buff, g_usart0_recv_length);

        // 清空接收缓冲区
        g_usart0_recv_length = 0;
    }
}

// 接收回调
// 弱引用  可以被其他文件重定义
__weak void on_usart2_recv(uint8_t *data, uint32_t len)
{
    // 结尾添加'\0'便于字符串处理
    g_usart2_recv_buff[g_usart2_recv_length] = '\0';

    // g_recv_buff为接收的数据，g_recv_length为接收的长度
    // 原样发送回去
    send_string(USART2, data);
}

// USART2中断处理函数必须叫这个
void USART2_IRQHandler(void)
{
    // 处理读取数据缓冲区不为空中断 （每接收一个字节（一帧）就会有一个RBNE中断）
    if ((usart_interrupt_flag_get(USART2, USART_INT_FLAG_RBNE)) == SET)
    {
        // 一定要手动清空标志位
        usart_interrupt_flag_clear(USART2, USART_INT_FLAG_RBNE);

        // 存入一个字节到接收缓冲区
        uint16_t value = usart_data_receive(USART2);
        g_usart2_recv_buff[g_usart2_recv_length] = value;
        g_usart2_recv_length++;
    }

    // 处理检测到空闲中断 （一包数据接收完成产生一个IDLE中断）
    if (usart_interrupt_flag_get(USART2, USART_INT_FLAG_IDLE) == SET)
    {
        // 读取缓冲区,清空缓冲区

        // 软件先读USART_STATO,再读USART_DATA可清除该位
        usart_data_receive(USART2);

        // 回调函数
        on_usart2_recv(g_usart2_recv_buff, g_usart2_recv_length);

        // 清空接收缓冲区
        g_usart2_recv_length = 0;
    }
}