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

/*!
    \brief    main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{

    systick_config();
    bsp_leds_config();
    bsp_keys_config();

    while (1)
    {

        // bsp_leds_open_all();
        // delay_1ms(1000);
        // bsp_leds_close_all();
        // delay_1ms(1000);

        // bsp_leds_open(LED2);
        // delay_1ms(1000);
        // bsp_leds_close(LED2);
        // delay_1ms(1000);

        // KEY1 按下/松开控制 LED1
        switch (bsp_keys_scan(KEY1))
        {
        case KEY_EVENT_PRESSED:
            bsp_leds_open(LED1);
            break;
        case KEY_EVENT_RELEASED:
            bsp_leds_close(LED1);
            break;
        default:
            break;
        }

        // KEY2 按下/松开控制 LED2
        switch (bsp_keys_scan(KEY2))
        {
        case KEY_EVENT_PRESSED:
            bsp_leds_open(LED2);
            break;
        case KEY_EVENT_RELEASED:
            bsp_leds_close(LED2);
            break;
        default:
            break;
        }

        // KEY3 按下/松开控制 LED3
        switch (bsp_keys_scan(KEY3))
        {
        case KEY_EVENT_PRESSED:
            bsp_leds_open(LED3);
            break;
        case KEY_EVENT_RELEASED:
            bsp_leds_close(LED3);
            break;
        default:
            break;
        }

        // KEY4 按下/松开控制 LED4
        switch (bsp_keys_scan(KEY4))
        {
        case KEY_EVENT_PRESSED:
            bsp_leds_open(LED4);
            break;
        case KEY_EVENT_RELEASED:
            bsp_leds_close(LED4);
            break;
        default:
            break;
        }
    }
}
