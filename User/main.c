#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_battery_flows.h"
#include "bsp_keys.h"
#include "bsp_leds.h"
#include "msp_exti.h"
#include "msp_uart.h"

int main(void)
{
    systick_config();
    USART0_config();
    USART2_config();

    msp_exti_init();

    uint8_t cnt = 0U;

    while (1)
    {
        ;
    }
}