#include "msp_uart.h"
#include <stdio.h>

USART_HandleTypeDef husart0 = {
    .usartx_tx_rcu = RCU_GPIOA,
    .usartx_tx_port = GPIOA,
    .usartx_tx_pin = GPIO_PIN_9,
    .usartx_tx_af = GPIO_AF_7,

    .usartx_rx_rcu = RCU_GPIOA,
    .usartx_rx_port = GPIOA,
    .usartx_rx_pin = GPIO_PIN_10,
    .usartx_rx_af = GPIO_AF_7,

    .usartx = USART0,
    .usartx_rcu = RCU_USART0,
    .usartx_irqn = USART0_IRQn,

    .usartx_p_baudrate = 115200,
    .usartx_p_parity = USART_PM_NONE,
    .usartx_p_wl = USART_WL_8BIT,
    .usartx_p_stop_bit = USART_STB_1BIT,
    .usartx_p_data_first = USART_MSBF_LSB,

};

USART_HandleTypeDef husart2 = {
    .usartx_tx_rcu = RCU_GPIOB,
    .usartx_tx_port = GPIOB,
    .usartx_tx_pin = GPIO_PIN_10,
    .usartx_tx_af = GPIO_AF_7,

    .usartx_rx_rcu = RCU_GPIOB,
    .usartx_rx_port = GPIOB,
    .usartx_rx_pin = GPIO_PIN_11,
    .usartx_rx_af = GPIO_AF_7,

    .usartx = USART2,
    .usartx_rcu = RCU_USART2,
    .usartx_irqn = USART2_IRQn,

    .usartx_p_baudrate = 115200,
    .usartx_p_parity = USART_PM_NONE,
    .usartx_p_wl = USART_WL_8BIT,
    .usartx_p_stop_bit = USART_STB_1BIT,
    .usartx_p_data_first = USART_MSBF_LSB,

};

static void GPIO_config(USART_HandleTypeDef *huart)
{
    //================ GPIO配置 ================
    // tx
    // 1.打开时钟
    rcu_periph_clock_enable(huart->usartx_tx_rcu);
    // 2.GPIO复用模式配置 GPIO_MODE_AF
    gpio_mode_set(huart->usartx_tx_port, GPIO_MODE_AF, GPIO_PUPD_NONE, huart->usartx_tx_pin);
    // 3.复用功能配置 GPIO_AF_7
    gpio_af_set(huart->usartx_tx_port, huart->usartx_tx_af, huart->usartx_tx_pin);
    // 4.tx需要配置输出模式选项
    gpio_output_options_set(huart->usartx_tx_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, huart->usartx_tx_pin);

    // rx
    // 1.打开时钟
    rcu_periph_clock_enable(huart->usartx_rx_rcu); // 配置时钟
    // 2.GPIO复用模式配置 GPIO_MODE_AF
    gpio_mode_set(huart->usartx_rx_port, GPIO_MODE_AF, GPIO_PUPD_NONE, huart->usartx_rx_pin);
    // 3.复用功能配置 GPIO_AF_7
    gpio_af_set(huart->usartx_rx_port, huart->usartx_rx_af, huart->usartx_rx_pin);
    // gpio_output_options_set(usartx_rx_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, usartx_rx_pin);
}

static void USART_config(USART_HandleTypeDef *huart)
{
    //================ USART 配置 ================
    // 1.打开USART外设时钟
    rcu_periph_clock_enable(huart->usartx_rcu);

    // 2.USART属性配置
    // USART复位
    usart_deinit(huart->usartx);

    usart_baudrate_set(huart->usartx, huart->usartx_p_baudrate);        // 波特率
    usart_parity_config(huart->usartx, huart->usartx_p_parity);         // 校验位
    usart_word_length_set(huart->usartx, huart->usartx_p_wl);           // 数据位数
    usart_stop_bit_set(huart->usartx, huart->usartx_p_stop_bit);        // 停止位
    usart_data_first_config(huart->usartx, huart->usartx_p_data_first); // 先发送高位还是低位

    // 3.发送和接收配置
    // 发送功能配置
    usart_transmit_config(huart->usartx, USART_TRANSMIT_ENABLE);
    // 接收功能配置
    usart_receive_config(huart->usartx, USART_RECEIVE_ENABLE);

    // 4.打开中断
    // NVIC中注册中断
    nvic_irq_enable(huart->usartx_irqn, 2, 2);
    // read data buffer not empty interrupt
    // 读取数据缓冲区不为空中断 （一帧数据结束）
    usart_interrupt_enable(huart->usartx, USART_INT_RBNE);
    // IDLE line detected interrupt
    // 检测到空闲中断   （一包数据结束）
    usart_interrupt_enable(huart->usartx, USART_INT_IDLE);

    // 5.USART使能
    usart_enable(huart->usartx);
}

static void GPIO_USART_config(USART_HandleTypeDef *huart)
{
    GPIO_config(huart);
    USART_config(huart);
}

void msp_uart_init()
{
    GPIO_USART_config(&husart0);
    GPIO_USART_config(&husart2);
}

// 发送一byte数据
void send_byte(USART_HandleTypeDef *huart, uint8_t data)
{
    // 通过USART发送
    usart_data_transmit(huart->usartx, data);
    // 判断缓冲区是否已经空了
    // FlagStatus state = usart_flag_get(USART_NUM,USART_FLAG_TBE);
    while (RESET == usart_flag_get(huart->usartx, USART_FLAG_TBE))
        ;
}

// 发送多个byte数据
void send_data(USART_HandleTypeDef *huart, uint8_t *data, uint32_t len)
{
    while (data && len--)
    {
        send_byte(huart, *data);
        data++;
    }
}

// 发送字符串
void send_string(USART_HandleTypeDef *huart, char *data)
{
    // 满足: 1.data指针不为空  2.发送的数据不是\0结束标记
    while (data && *data)
    {
        send_byte(huart, (uint8_t)(*data));
        data++;
    }
}

// 重写fputc方法  调用printf,会自动调用这个方法实现打印
int fputc(int ch, FILE *f)
{
    send_byte(&husart0, (uint8_t)ch);
    return ch;
}

#ifndef __weak
#define __weak __attribute__((weak)) // GCC / armclang
#endif

// 接收回调
// 弱引用  可以被其他文件重定义
__weak void on_usart_recv(USART_HandleTypeDef *huart)
{
    // 结尾添加'\0'便于字符串处理
    huart->recv_buff[huart->recv_length] = '\0';

    // g_recv_buff为接收的数据，g_recv_length为接收的长度
    // 原样发送回去
    send_string(huart, (char *)huart->recv_buff);
}

void uart_irq_handle(USART_HandleTypeDef *huart)
{
    // 处理读取数据缓冲区不为空中断 （每接收一个字节（一帧）就会有一个RBNE中断）
    if ((usart_interrupt_flag_get(huart->usartx, USART_INT_FLAG_RBNE)) == SET)
    {
        // 一定要手动清空标志位
        usart_interrupt_flag_clear(huart->usartx, USART_INT_FLAG_RBNE);

        // 存入一个字节到接收缓冲区
        uint16_t value = usart_data_receive(huart->usartx);
        huart->recv_buff[huart->recv_length] = value;
        huart->recv_length++;
    }

    // 处理检测到空闲中断 （一包数据接收完成产生一个IDLE中断）
    if (usart_interrupt_flag_get(huart->usartx, USART_INT_FLAG_IDLE) == SET)
    {
        // 读取缓冲区,清空缓冲区

        // 软件先读USART_STATO,再读USART_DATA可清除该位
        usart_data_receive(huart->usartx);

        // 回调函数
        on_usart_recv(huart);

        // 清空接收缓冲区
        huart->recv_length = 0;
    }
}

// USART0中断处理函数必须叫这个
// 在startup.s中配置了中断向量表 line124
// 中断中不能写耗时的任务
void USART0_IRQHandler(void)
{
    USART_HandleTypeDef *p = &husart0;
    uart_irq_handle(p);
}

// USART2中断处理函数必须叫这个
void USART2_IRQHandler(void)
{
    USART_HandleTypeDef *p = &husart2;
    uart_irq_handle(p);
}