#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

/**
PA9		TXD
PA10	RXD
**/

#define USART_RECEIVE_LENGTH 1024
// 串口接收缓冲区大小
uint8_t g_recv_buff[USART_RECEIVE_LENGTH]; // 接收缓冲区
// 接收到字符存放的位置
int g_recv_length = 0;

static void USART_config()
{
    uint32_t usartx_tx_rcu = RCU_GPIOA;
    uint32_t usartx_tx_port = GPIOA;
    uint32_t usartx_tx_pin = GPIO_PIN_9;
    uint32_t usartx_tx_af = GPIO_AF_7;

    uint32_t usartx_rx_rcu = RCU_GPIOA;
    uint32_t usartx_rx_port = GPIOA;
    uint32_t usartx_rx_pin = GPIO_PIN_10;
    uint32_t usartx_rx_af = GPIO_AF_7;

    uint32_t usartx = USART0;
    uint32_t usartx_rcu = RCU_USART0;
    uint32_t usartx_irqn = USART0_IRQn;

    uint32_t usartx_p_baudrate = 115200;
    uint32_t usartx_p_parity = USART_PM_NONE;
    uint32_t usartx_p_wl = USART_WL_8BIT;
    uint32_t usartx_p_stop_bit = USART_STB_1BIT;
    uint32_t usartx_p_data_first = USART_MSBF_LSB;

    /************** gpio config **************/
    // tx
    rcu_periph_clock_enable(usartx_tx_rcu); // 配置时钟
    gpio_mode_set(usartx_tx_port, GPIO_MODE_AF, GPIO_PUPD_NONE, usartx_tx_pin);
    gpio_af_set(usartx_tx_port, usartx_tx_af, usartx_tx_pin);
    gpio_output_options_set(usartx_tx_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, usartx_tx_pin);
    // rx
    rcu_periph_clock_enable(usartx_rx_rcu); // 配置时钟
    gpio_mode_set(usartx_rx_port, GPIO_MODE_AF, GPIO_PUPD_NONE, usartx_rx_pin);
    gpio_af_set(usartx_rx_port, usartx_rx_af, usartx_rx_pin);
    // gpio_output_options_set(usartx_rx_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, usartx_rx_pin);

    /************** usart config **************/
    // 串口时钟
    rcu_periph_clock_enable(RCU_USART0);
    // USART复位
    usart_deinit(usartx);

    usart_baudrate_set(usartx, usartx_p_baudrate);        // 波特率
    usart_parity_config(usartx, usartx_p_parity);         // 校验位
    usart_word_length_set(usartx, usartx_p_wl);           // 数据位数
    usart_stop_bit_set(usartx, usartx_p_stop_bit);        // 停止位
    usart_data_first_config(usartx, usartx_p_data_first); // 先发送高位还是低位

    // 发送功能配置
    usart_transmit_config(usartx, USART_TRANSMIT_ENABLE);

    // 接收功能配置
    usart_receive_config(usartx, USART_RECEIVE_ENABLE);
    // 接收中断配置
    nvic_irq_enable(usartx_irqn, 2, 2);
    // usart int rbne
    usart_interrupt_enable(usartx, USART_INT_RBNE);
    usart_interrupt_enable(usartx, USART_INT_IDLE);

    // 使能串口
    usart_enable(usartx);
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

void USART0_IRQHandler(void)
{
    if ((usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)) == SET)
    {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);
        uint16_t value = usart_data_receive(USART0);
        g_recv_buff[g_recv_length] = value;
        g_recv_length++;
    }
    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE) == SET)
    {
        // 读取缓冲区,清空缓冲区
        usart_data_receive(USART0);
        g_recv_buff[g_recv_length] = '\0';

        // TODO: g_recv_buff为接收的数据，g_recv_length为接收的长度
        printf("rec:%s", g_recv_buff);

        g_recv_length = 0;
    }
}

int main(void)
{
    systick_config();
    USART_config();

    while (1)
    {
    }
}
