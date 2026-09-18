#ifndef MSP_UART_H
#define MSP_UART_H

#include "gd32f4xx.h"

// 初始化USART0 到A9 A10
void USART0_config();

// 发送一byte数据
void send_byte(uint8_t data);

// 发送多个byte数据
void send_data(uint8_t *data, uint32_t len);

// 发送字符串
void send_string(char *data);

#endif // MSP_UART_H