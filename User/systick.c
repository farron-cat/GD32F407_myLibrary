/*!
    \file    systick.c
    \brief   the systick configuration file

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

#include "systick.h"
#include "gd32f4xx.h"

#ifndef SYS_SUPPORT_OS
volatile static uint32_t delay;
volatile static uint64_t us_cnt;

/*!
    \brief    configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void systick_config(void)
{
    /* setup systick timer for 1000000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000000U))
    { // 1us
        /* capture error */
        while (1)
        {
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

/*!
    \brief    delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_1ms(uint32_t count)
{
    delay = count * 1000;

    while (0U != delay)
    {
    }
}

/*!
    \brief    delay a time in microsecond
    \param[in]  count: count in microsecond
    \param[out] none
    \retval     none
*/
void delay_1us(uint32_t count)
{
    delay = count;

    while (0U != delay)
    {
    }
}

/*!
    \brief    delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void delay_decrement(void)
{
    us_cnt++;

    if (0U != delay)
    {
        delay--;
    }
}

uint64_t get_us_cnt(void)
{
    return us_cnt;
}
#else

#include "FreeRTOS.h"
#include "task.h"

/*
 * FreeRTOS 环境下：
 *  - SysTick 已经交给 FreeRTOS 做 1ms 系统节拍，不能再当 1us 延时基准
 *  - 延时改用 Cortex-M4 内核的 DWT 周期计数器，调度器启动前后、中断里都能用
 *  - 微秒时间戳由系统节拍钩子每 1ms 累加一次
 */

static volatile uint64_t us_cnt = 0;

/* 使能 DWT 周期计数器（只做一次） */
static void dwt_delay_init(void)
{
    static uint8_t inited = 0;

    if (0U == inited)
    {
        inited = 1U;
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0U;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/*!
    \brief    delay a time in microsecond
    \param[in]  count: count in microsecond
    \param[out] none
    \retval     none
*/
void delay_1us(uint32_t count)
{
    uint32_t start, ticks;

    dwt_delay_init();

    ticks = count * (SystemCoreClock / 1000000U);
    start = DWT->CYCCNT;

    /* 32 位计数器回绕时差值依然正确 */
    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}

/*!
    \brief    delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_1ms(uint32_t count)
{
    while (0U != count)
    {
        delay_1us(1000U);
        count--;
    }
}

/*!
    \brief    get microsecond counter in FreeRTOS environment
    \param[in]  none
    \param[out] none
    \retval     microsecond count (64-bit)
*/
uint64_t get_us_cnt(void)
{
    return us_cnt;
}

/*!
    \brief    update microsecond counter, called by vApplicationTickHook() every tick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void delay_tick_update(void)
{
    us_cnt += (uint64_t)(1000000UL / (uint32_t)configTICK_RATE_HZ);
}

#endif
