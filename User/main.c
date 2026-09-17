/*!
    \file    main.c
    \brief   led spark with systick

    \version 2024-01-15, V3.2.0, firmware for GD32F4xx
*/

/*
    Copyright (c) 2024, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_keys.h"
#include "bsp_leds.h"

static void USART_config()
{
    /************** gpio config **************/
    // 配置时钟
    rcu_periph_clock_enable(RCU_GPIOA);
    // 配置模式
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);
    // 配置复用功能
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);
    // 配置输出参数
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /************** usart config **************/
    // 串口时钟
    rcu_periph_clock_enable(RCU_USART0);
    // USART复位
    usart_deinit(USART0);
    // 波特率
    usart_baudrate_set(USART0, 115200);
    // 校验位
    usart_parity_config(USART0, USART_PM_NONE);
    // 数据位数
    usart_word_length_set(USART0, USART_WL_8BIT);
    // 停止位
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    // 先发送高位还是低位
    usart_data_first_config(USART0, USART_MSBF_LSB);
    // 发送功能配置
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    // 使能串口
    usart_enable(USART0);
}

// 发送一byte数据
static void send_byte(uint8_t data)
{
    // 通过USART发送
    usart_data_transmit(USART0, data);
    // 判断缓冲区是否已经空了
    // FlagStatus state = usart_flag_get(USART_NUM,USART_FLAG_TBE);
    while (RESET == usart_flag_get(USART0, USART_FLAG_TBE))
        ;
}

// 发送多个byte数据
void send_data_array(uint8_t *data, uint32_t len)
{
    while (data && len--)
    {
        send_byte(*data);
        data++;
    }
}

// 发送字符串
static void send_string(char *data)
{
    // 满足: 1.data指针不为空  2.发送的数据不是\0结束标记
    while (data && *data)
    {
        send_byte((uint8_t)(*data));
        data++;
    }
}

int fputc(int ch, FILE *f)
{
    send_byte((uint8_t)ch);
    return ch;
}

// 按键按下回调
void on_key_press(KEY_NUM key)
{
    bsp_leds_open(key + 1);
}

// 按键松开回调
void on_key_release(KEY_NUM key)
{
    bsp_leds_close(key + 1);
}

int main(void)
{

    systick_config();
    bsp_leds_config();
    bsp_keys_config();

    USART_config();

    uint8_t cnt = 0U;

    while (1)
    {
        bsp_keys_scan(KEY1);
        bsp_keys_scan(KEY2);
        bsp_keys_scan(KEY3);
        bsp_keys_scan(KEY4);

        // // KEY1 按下/松开控制 LED1
        // switch (bsp_keys_scan(KEY1))
        // {
        // case KEY_EVENT_PRESSED:
        //     bsp_leds_open(LED1);
        //     break;
        // case KEY_EVENT_RELEASED:
        //     bsp_leds_close(LED1);
        //     break;
        // default:
        //     break;
        // }

        // // KEY2 按下/松开控制 LED2
        // switch (bsp_keys_scan(KEY2))
        // {
        // case KEY_EVENT_PRESSED:
        //     bsp_leds_open(LED2);
        //     break;
        // case KEY_EVENT_RELEASED:
        //     bsp_leds_close(LED2);
        //     break;
        // default:
        //     break;
        // }

        // // KEY3 按下/松开控制 LED3
        // switch (bsp_keys_scan(KEY3))
        // {
        // case KEY_EVENT_PRESSED:
        //     bsp_leds_open(LED3);
        //     break;
        // case KEY_EVENT_RELEASED:
        //     bsp_leds_close(LED3);
        //     break;
        // default:
        //     break;
        // }

        // // KEY4 按下/松开控制 LED4
        // switch (bsp_keys_scan(KEY4))
        // {
        // case KEY_EVENT_PRESSED:
        //     bsp_leds_open(LED4);
        //     break;
        // case KEY_EVENT_RELEASED:
        //     bsp_leds_close(LED4);
        //     break;
        // default:
        //     break;
        // }

        // send_byte(cnt++);
        // send_string("hello\r\n");
        // printf("hello %d\r\n", cnt++);
        // delay_1ms(10000);
    }
}
