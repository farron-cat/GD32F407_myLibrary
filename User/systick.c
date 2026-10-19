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
 *  - SysTick 已经交给 FreeRTOS 做 1ms 系统节拍
 *  - 微秒级延时改用 TIMER6 自由运行计数器(1MHz)，
 *    不依赖 DWT/调试单元，调度器启动前后、中断里都能用
 *  - 微秒时间戳由系统节拍每 1ms 累加一次
 */

#define US_TIMER TIMER6
#define US_TIMER_RCU RCU_TIMER6

static volatile uint64_t us_cnt = 0;

/* 把 TIMER6 配置成 1MHz 自由运行计数器，只初始化一次 */
static void us_timer_init(void)
{
    static uint8_t inited = 0U;

    if (0U == inited)
    {
        timer_parameter_struct timer_init_struct;
        uint32_t us_psc;
        inited = 1U;

        rcu_periph_clock_enable(US_TIMER_RCU);
        timer_deinit(US_TIMER);

        timer_struct_para_init(&timer_init_struct);
        /* RCU_TIMER_PSC_MUL4 下 APB1 定时器时钟等于 SystemCoreClock(168MHz) */
        /* 1MHz 计数：分频 = 定时器时钟/1MHz - 1，时钟异常时按 168MHz 兜底 */
        us_psc = (uint32_t)(SystemCoreClock / 1000000U);
        if ((0U == us_psc) || (us_psc > 65536U))
        {
            us_psc = 168U;
        }
        timer_init_struct.prescaler = (uint16_t)(us_psc - 1U);
        timer_init_struct.period = 0xFFFFFFFFU; /* 32 位自由运行 */
        timer_init_struct.counterdirection = TIMER_COUNTER_UP;
        timer_init(US_TIMER, &timer_init_struct);

        timer_enable(US_TIMER);
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
    uint32_t start;

    us_timer_init();
    start = TIMER_CNT(US_TIMER);

    /* 32 位无符号减法自动处理计数器回绕 */
    while ((TIMER_CNT(US_TIMER) - start) < count)
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
