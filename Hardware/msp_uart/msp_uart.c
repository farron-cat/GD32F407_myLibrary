#include "msp_uart.h"
#include <stdio.h>

#define USART_RECEIVE_LENGTH 1024
// 串口接收缓冲区大小
uint8_t g_recv_buff[USART_RECEIVE_LENGTH]; // 接收缓冲区
// 接收到字符存放的位置
int g_recv_length = 0;

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

// 发送一byte数据
void send_byte(uint8_t data)
{
    // 通过USART发送
    usart_data_transmit(USART0, data);
    // 判断缓冲区是否已经空了
    // FlagStatus state = usart_flag_get(USART_NUM,USART_FLAG_TBE);
    while (RESET == usart_flag_get(USART0, USART_FLAG_TBE))
        ;
}

// 发送多个byte数据
void send_data(uint8_t *data, uint32_t len)
{
    while (data && len--)
    {
        send_byte(*data);
        data++;
    }
}

// 发送字符串
void send_string(char *data)
{
    // 满足: 1.data指针不为空  2.发送的数据不是\0结束标记
    while (data && *data)
    {
        send_byte((uint8_t)(*data));
        data++;
    }
}

// 重写fputc方法  调用printf,会自动调用这个方法实现打印
int fputc(int ch, FILE *f)
{
    send_byte((uint8_t)ch);
    return ch;
}

// USART0中断处理函数必须叫这个
// 在startup.s中配置了中断向量表 line124
void USART0_IRQHandler(void)
{
    // 处理读取数据缓冲区不为空中断 （每接收一个字节（一帧）就会有一个RBNE中断）
    if ((usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)) == SET)
    {
        // 一定要手动清空标志位
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);

        // 存入一个字节到接收缓冲区
        uint16_t value = usart_data_receive(USART0);
        g_recv_buff[g_recv_length] = value;
        g_recv_length++;
    }

    // 处理检测到空闲中断 （一包数据接收完成产生一个IDLE中断）
    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE) == SET)
    {
        // 读取缓冲区,清空缓冲区

        // 软件先读USART_STATO,再读USART_DATA可清除该位
        usart_data_receive(USART0);

        // 结尾添加'\0'便于字符串处理
        g_recv_buff[g_recv_length] = '\0';

        // g_recv_buff为接收的数据，g_recv_length为接收的长度
        // 原样发送回去
        printf("rec:%s", g_recv_buff);

        // 清空接收缓冲区
        g_recv_length = 0;
    }
}