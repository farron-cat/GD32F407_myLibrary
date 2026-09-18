#ifndef MSP_UART_H
#define MSP_UART_H

#include "gd32f4xx.h"

// 初始化USART0 到A9 A10
void USART0_config();

void USART2_config();

// 发送一byte数据
void send_byte(uint32_t usartx, uint8_t data);

// 发送多个byte数据
void send_data(uint32_t usartx, uint8_t *data, uint32_t len);

// 发送字符串
void send_string(uint32_t usartx, char *data);

// 接收处理回调
extern void on_usart0_recv(uint8_t *data, uint32_t len);

extern void on_usart2_recv(uint8_t *data, uint32_t len);

#endif // MSP_UART_H