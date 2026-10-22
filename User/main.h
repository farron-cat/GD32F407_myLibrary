/*!
    \file    main.h
    \brief   the header file of main

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

#ifndef __MAIN_H
#define __MAIN_H

//=========== 系统初始化 ===========
void system_init(void);

//============ 启动任务 ============
static void start_task(void *pvParameters);

//============ 任务函数 ============
static void app_task(void *pvParameters);

void app_task_input_touch_key(void *pvParameters);

void app_task_show_digital_leds(void *pvParameters);

void app_task_show_dot_matrix(void *pvParameters);

void app_task_show_188_leds_task(void *pvParameters);

void app_task_show_iic_oled_task(void *pvParameters);

void app_task_show_pcf8563_task(void *pvParameters);

//============ 监控任务 ============
void task_stack_monitor(void *pvParameters);

//============ 测试任务 ============
void task1(void *pvParameters);

void task2(void *pvParameters);

void task3(void *pvParameters);

void app_task_keys_scan(void *pvParameters);

#endif // __MAIN_H
