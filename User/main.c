#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_keys.h"
#include "msp_uart.h"

/**
PA9		TXD
PA10	RXD
**/

int main(void)
{
    systick_config();
    USART0_config();
    bsp_keys_config();

    while (1)
    {
        if (bsp_keys_scan(KEY1) == KEY_EVENT_PRESSED)
        {
            printf("KEY1 pressed\r\n");
        }
    }
}
