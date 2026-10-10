#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_188_leds.h"
#include "bsp_battery_flows.h"
#include "bsp_buzzer.h"
#include "bsp_flash.h"
#include "bsp_iic_oled.h"
#include "bsp_keys.h"
#include "bsp_leds.h"
#include "bsp_ntc.h"
#include "bsp_pcf8563.h"
#include "bsp_spi_oled.h"

#include "msp_adc.h"
#include "msp_exti.h"
#include "msp_fwdgt.h"
#include "msp_iic.h"
#include "msp_rtc.h"
#include "msp_spi.h"
#include "msp_uart.h"
#include "msp_wwdgt.h"

#include "bmp.h"

// 串口接收回调
void on_usart_recv(USART_HandleTypeDef *huart)
{
    huart->recv_buff[huart->recv_length] = '\0';
    if (huart->usartx == USART0)
    {
        printf("usart0_recv:%s\n", huart->recv_buff);
        // 配置剩余参数
        dma_periph_address_config(DMA1, DMA_CH0, (uint32_t)huart->recv_buff);
        dma_transfer_number_config(DMA1, DMA_CH0, huart->recv_length);
        // 启动DMA传输
        dma_channel_enable(DMA1, DMA_CH0);
    }
}

// IIC OLED 测试
void oled_test(void)
{
    uint8_t t = ' ';

    bsp_iic_oled_show_picture(0, 0, 128, 64, BMP1, 1);
    bsp_iic_oled_refresh();
    delay_1ms(500);
    bsp_iic_oled_clear();
    bsp_iic_oled_show_chinese(0, 0, 0, 16, 1);   // 中
    bsp_iic_oled_show_chinese(18, 0, 1, 16, 1);  // 景
    bsp_iic_oled_show_chinese(36, 0, 2, 16, 1);  // 园
    bsp_iic_oled_show_chinese(54, 0, 3, 16, 1);  // 电
    bsp_iic_oled_show_chinese(72, 0, 4, 16, 1);  // 子
    bsp_iic_oled_show_chinese(90, 0, 5, 16, 1);  // 技
    bsp_iic_oled_show_chinese(108, 0, 6, 16, 1); // 术
    bsp_iic_oled_show_string(8, 16, "ZHONGJINGYUAN", 16, 1);
    bsp_iic_oled_show_string(20, 32, "2014/05/01", 16, 1);
    bsp_iic_oled_show_string(0, 48, "ASCII:", 16, 1);
    bsp_iic_oled_show_string(63, 48, "CODE:", 16, 1);
    bsp_iic_oled_show_char(48, 48, t, 16, 1); // 显示ASCII字符
    t++;
    if (t > '~')
        t = ' ';
    bsp_iic_oled_show_number(103, 48, t, 3, 16, 1);
    bsp_iic_oled_refresh();

    delay_1ms(500);
    bsp_iic_oled_clear();
    bsp_iic_oled_show_chinese(0, 0, 0, 16, 1);   // 16*16 中
    bsp_iic_oled_show_chinese(16, 0, 0, 24, 1);  // 24*24 中
    bsp_iic_oled_show_chinese(24, 20, 0, 32, 1); // 32*32 中
    bsp_iic_oled_show_chinese(64, 0, 0, 64, 1);  // 64*64 中
    bsp_iic_oled_refresh();

    delay_1ms(500);
    bsp_iic_oled_clear();
    bsp_iic_oled_show_string(0, 0, "ABC", 8, 1);   // 6*8 “ABC”
    bsp_iic_oled_show_string(0, 8, "ABC", 12, 1);  // 6*12 “ABC”
    bsp_iic_oled_show_string(0, 20, "ABC", 16, 1); // 8*16 “ABC”
    bsp_iic_oled_show_string(0, 36, "ABC", 24, 1); // 12*24 “ABC”
    bsp_iic_oled_refresh();

    delay_1ms(500);
    bsp_iic_oled_scroll_display(11, 4, 1);
}

// OLED 显示 RTC 时间
void oled_show_rtc(Time_pcf *t)
{
    char buf[32] = {0};

    // 1. 先显示 BMP1 整幅背景（128x64）
    //    这一步会覆盖上次 GRAM，保证背景干净
    bsp_iic_oled_show_picture(0, 0, 128, 64, BMP1, 1);

    // 2. 在 BMP1 原本 "2014/05/01" 的位置覆盖显示日期
    //    原测试代码用的是 (20, 32)，这里保持一致
    sprintf(buf, "20%02d/%02d/%02d", t->year % 100, t->month, t->day);
    bsp_iic_oled_show_string(36, 38, buf, 8, 1);

    // 3. 在日期右侧显示星期
    // sprintf(buf, "W%d", t->week);
    // OLED_ShowString(100, 32, buf, 8, 1);

    // 4. 在 BMP1 原本 "ASCII: CODE:" 的位置覆盖显示时间
    //    原测试代码用的是 (0, 48) 和 (63, 48)，这里把时分秒放到中间
    sprintf(buf, "%02d:%02d:%02d", t->hour, t->minutes, t->second);
    bsp_iic_oled_show_string(36, 2, buf, 8, 1);

    // 5. 统一刷新到 OLED
    bsp_iic_oled_refresh();
}

// FLASH 测试
void flash_test(void)
{
    unsigned char buff[20] = {0};
    // 获取GD25Q32的设备ID
    printf("ID = %X\r\n", bsp_flash_read_id());

    // 读取0地址长度为7个字节的数据到buff
    bsp_flash_read(buff, 0, 10);
    // 输出读取到的数据
    printf("buff: %s\r\n", buff);
    delay_1ms(200);
    // 往0地址写入6个字节的数据 “hello”
    bsp_flash_write((uint8_t *)"hello", 0, 10);

    // 等待写入完成
    delay_1ms(200);

    // 读取0地址长度为7个字节的数据到buff
    bsp_flash_read(buff, 0, 10);
    // 输出读取到的数据
    printf("buff: %s\r\n", buff);

    delay_1ms(1000);
}

//============ 主 函 数 ============
int main(void)
{
    // 配置整个工程优先级分组 抢占:0~3  响应:0~3
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
    // 时钟倍频(全局配置 所有定时器都是168Mhz)
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
    // 系统滴答时钟初始化
    systick_config();

    //============ 片上外设 ============
    // USART0和USART2初始化
    msp_uart_init();
    // EXTI0 PA0 和 EXTI3 PC3初始化
    msp_exti_init();
    // RTC
    msp_rtc_init(HXTAL);
    // IIC
    msp_iic_init();
    // ADC
    msp_adc_init();
    // SPI
    msp_spi_init();

    //============ 片外外设 ============
    // LED灯组初始化
    bsp_leds_config();
    // 按键初始化
    bsp_keys_config();
    // 蜂鸣器初始化
    bsp_buzzer_init();
    // oled初始化
    bsp_iic_oled_init();
    bsp_spi_oled_init();
    // pcf8563初始化
    bsp_pcf8563_init();
    // flash
    bsp_flash_init();
    // NTC
    bsp_ntc_init();
    // 188LED
    bsp_188_leds_init();

    printf("============ start ============\n");
    printf("SystemCoreClock = %u\r\n", (unsigned int)SystemCoreClock);

    // 独立看门狗
    // msp_fwdgt_config();
    // 窗口看门狗
    // msp_wwdgt_config(); // 前面的初始化会消耗时间

    Time_pcf time;
    Alarm alarm;

    // 设置时间
    time.year = 2026;
    time.month = 9;
    time.day = 28;
    time.week = 1;
    time.hour = 14;
    time.minutes = 59;
    time.second = 55;
    bsp_pcf8563_set_time(&time);

    // 设置闹钟 15:00 周日
    alarm.hour = 15;
    alarm.min = 0;
    alarm.day = 28;
    alarm.week = 1;
    bsp_pcf8563_set_alarm(&alarm);
    bsp_pcf8563_alarm_enable();

    // IIC屏幕显示图片
    bsp_iic_oled_show_picture(0, 0, 128, 64, BMP1, 1);

    bsp_iic_oled_refresh();

    // SPI屏幕显示图片
    // bsp_spi_oled_display_128x64(BMP1);
    // SPI屏幕显示中文
    bsp_spi_oled_display_GB2312_string(0, 0, "秀");

    // FLASH 测试
    flash_test();

    char buf[64] = {0};

    uint8_t cnt = 0;
    uint32_t num = 0;
    while (1)
    {
        // PCF8563 中断事件处理：原在 EXTI5 中断中执行，移到主循环以避免软件 IIC 重入
        if (g_rtc_int_flag)
        {
            g_rtc_int_flag = 0;
            on_rtc_int();
        }

        // 188数码管显示NTC温度
        bsp_188_leds_set_num(bsp_ntc_get_tem());

        // IIC OLED 测试
        // oled_test();

        // 读取时间
        bsp_pcf8563_read_time(&time);
        // IIC屏幕图片上叠加时间
        oled_show_rtc(&time);

        // 串口打印时间
        // printf("20%02d-%02d-%02d %02d:%02d:%02d Week:%d\r\n",
        //        time.year % 100, time.month, time.day,
        //        time.hour, time.minutes, time.second, time.week);

        // 串口打印NTC温度
        // printf("NTC Temperature: %d C\r\n", bsp_ntc_get_tem());

        // 喂狗
        // wwdgt_counter_update(0x7F);

        cnt++;
    }
}