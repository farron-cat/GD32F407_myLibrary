#ifndef MSP_UART_H
#define MSP_UART_H

#include "gd32f4xx.h"

#define USART_RECEIVE_LENGTH 1024

typedef struct {

    uint32_t usartx_tx_rcu;
    uint32_t usartx_tx_port;
    uint32_t usartx_tx_pin;
    uint32_t usartx_tx_af;

    uint32_t usartx_rx_rcu;
    uint32_t usartx_rx_port;
    uint32_t usartx_rx_pin;
    uint32_t usartx_rx_af;

    uint32_t usartx;
    uint32_t usartx_rcu;
    uint32_t usartx_irqn;

    uint32_t usartx_p_baudrate;
    uint32_t usartx_p_parity;
    uint32_t usartx_p_wl;
    uint32_t usartx_p_stop_bit;
    uint32_t usartx_p_data_first;

    uint8_t recv_buff[USART_RECEIVE_LENGTH];
    uint16_t recv_length;

} USART_HandleTypeDef;

// 初始化USART0 到A9 A10
void msp_uart_init();

// 发送一byte数据
void send_byte(USART_HandleTypeDef *huart, uint8_t data);

// 发送多个byte数据
void send_data(USART_HandleTypeDef *huart, uint8_t *data, uint32_t len);

// 发送字符串
void send_string(USART_HandleTypeDef *huart, char *data);

// 接收处理回调
extern void on_usart_recv(USART_HandleTypeDef *huart);

#endif // MSP_UART_H