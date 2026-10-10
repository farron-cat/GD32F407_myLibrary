#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_battery_flows.h"
#include "bsp_buzzer.h"
#include "bsp_iic_oled.h"
#include "bsp_keys.h"
#include "bsp_leds.h"
#include "bsp_pcf8563.h"
#include "bsp_spi_oled.h"
#include "msp_exti.h"
#include "msp_iic.h"
#include "msp_rtc.h"
// #include "msp_spi.h"
#include "bsp_188_leds.h"
#include "bsp_flash.h"
#include "msp_uart.h"

#include "bmp.h"

Time time;
uint8_t dat[1024] = {0};

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

void DMA_m2m_config(void)
{
    // 1.开启外设时钟
    rcu_periph_clock_enable(RCU_DMA1); // 只有DMA1支持m2m
    // 2.配置DMA
    dma_single_data_parameter_struct init_struct;
    dma_single_data_para_struct_init(&init_struct);

    init_struct.direction = DMA_MEMORY_TO_MEMORY;
    // init_struct.periph_addr = ？？;
    init_struct.memory0_addr = (uint32_t)dat;

    init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    // init_struct.number = ？？;

    init_struct.periph_inc = DMA_PERIPH_INCREASE_ENABLE;
    init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
    init_struct.priority = DMA_PRIORITY_LOW;

    dma_single_data_mode_init(DMA1, DMA_CH0, &init_struct);

    // 配置中断
    nvic_irq_enable(DMA1_Channel0_IRQn, 2, 2);
    // 清理标志位
    dma_interrupt_flag_clear(DMA1, DMA_CH0, DMA_INT_FLAG_FTF);
    // 开启传输完成中断
    dma_interrupt_enable(DMA1, DMA_CH0, DMA_INT_FTF);

    // 3.启动DMA传输
    // dma_channel_enable(DMA1, DMA_CH0); // 没有配置好还不能启动
}

void DMA1_Channel0_IRQHandler(void)
{
    if (dma_interrupt_flag_get(DMA1, DMA_CH0, DMA_INT_FLAG_FTF) == SET)
    {
        // 清标志位
        dma_interrupt_flag_clear(DMA1, DMA_CH0, DMA_INT_FLAG_FTF);

        printf("dat:%s\n", dat);

        // 关闭DMA
        dma_channel_disable(DMA1, DMA_CH0);
    }
}

void msp_fwdgt_config(void)
{
    // 打开时钟
    rcu_osci_off(RCU_IRC32K);
    delay_1ms(1);
    rcu_osci_on(RCU_IRC32K);

    if (rcu_osci_stab_wait(RCU_IRC32K) == ERROR)
    { // 等待稳定
        printf("turn_on_osci_error\n");
        return;
    }

    // 写使能
    fwdgt_write_enable();
    // 配置分频系数 32000 / 32 = 1000
    fwdgt_prescaler_value_config(FWDGT_PSC_DIV32);
    // 配置重装载值
    fwdgt_reload_value_config(100);
    // 启动看门狗
    fwdgt_counter_reload();
    fwdgt_enable();
}

void msp_wwdgt_config(void)
{
    // 打开时钟
    rcu_periph_clock_enable(RCU_WWDGT);
    // 42mhz
    // 42000000hz / 4096 = 10,253.90625hz
    // 10,253.90625hz / 1 = 10,253.90625ms
    // 10,253.90625hz <=> 1000000us
    // 数1个数 <=> 97.523us
    // 配置窗口看门狗属性
    // 最短喂狗时间   47*97.5us = 4582.5us
    // 最长喂狗时间   64*97.5us = 6241.5us
    wwdgt_config(0x7F, 0x50, WWDGT_CFG_PSC_DIV1);
    // 启动看门狗
    wwdgt_enable();
}

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

uint16_t adc_results[2];
// P=>M
void msp_adc_dma_config(void)
{
    // 重置DMA
    dma_deinit(DMA1, DMA_CH0);
    // 1.开启外设时钟
    rcu_periph_clock_enable(RCU_DMA1);
    // 2.配置DMA
    dma_single_data_parameter_struct init_struct;
    dma_single_data_para_struct_init(&init_struct);

    init_struct.direction = DMA_PERIPH_TO_MEMORY;
    init_struct.memory0_addr = (uint32_t)adc_results;
    init_struct.periph_addr = (uint32_t)(&ADC_RDATA(ADC0));

    init_struct.periph_memory_width = DMA_PERIPH_WIDTH_16BIT; // 根据源地址确定
    init_struct.number = 2;

    init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    init_struct.circular_mode = DMA_CIRCULAR_MODE_ENABLE;
    init_struct.priority = DMA_PRIORITY_LOW;

    dma_single_data_mode_init(DMA1, DMA_CH0, &init_struct);

    // 配置DMA通道的子外设 查表
    dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI0);
    // 清理DMA搬运完成标志位
    dma_flag_clear(DMA1, DMA_CH0, DMA_FLAG_FTF);

    // 3.启动DMA传输
    dma_channel_enable(DMA1, DMA_CH0);
}

// 电位器 PC4 ADC0_IN14
void msp_adc_config(void)
{
    // 配置GPIO
    // 打开时钟
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_4);

    // 配置ADC
    /* 重置 */
    adc_deinit();
    /* 使能时钟 */
    rcu_periph_clock_enable(RCU_ADC0);
    /* 设置分频系数 21Mhz(根据时钟树确定要小于49Mhz)*/
    adc_clock_config(ADC_ADCCK_PCLK2_DIV4);
    /* 设置同步模式(独立模式) */
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
    /* 设置单次模式还是连续转换(单次转换) */
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);
    /* 设置扫描还是非扫描模式(非扫描模式) */
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    /* 设置是否打开插入通道(不打开) */
    adc_special_function_config(ADC0, ADC_INSERTED_CHANNEL_AUTO, DISABLE);
    /* 设置分辨率 */
    adc_resolution_config(ADC0, ADC_RESOLUTION_12B);
    /* 设置数据对齐 */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);

    /* 设置转换通道个数(包括常规通道组和插入通道组) */
    adc_channel_length_config(ADC0, ADC_ROUTINE_CHANNEL, 2);
    /* 设置转换哪一个通道以及所处序列位置 */
    adc_routine_channel_config(ADC0, 0, ADC_CHANNEL_14, ADC_SAMPLETIME_15);

    adc_routine_channel_config(ADC0, 1, ADC_CHANNEL_16, ADC_SAMPLETIME_15);

    // 内部通道需要单独打开
    adc_channel_16_to_18(ADC_TEMP_VREF_CHANNEL_SWITCH, ENABLE);
    adc_flag_clear(ADC0, ADC_FLAG_EOC);
    // // 配置常规通道每一个通道转换完成都会产生EOC标志位
    // adc_end_of_conversion_config(ADC0, ADC_EOC_SET_CONVERSION);

    /* 配置DMA */
    // 每个通道转换完成都会产生DMA搬运请求
    adc_dma_request_after_last_disable(ADC0);
    adc_dma_mode_enable(ADC0);

    /* 使能ADC */
    adc_enable(ADC0);
    /* 内部校准(需要delay等待) */
    delay_1ms(1);
    // 校准
    adc_calibration_enable(ADC0);

    // 将采集放入转换通道
    adc_software_trigger_enable(ADC0, ADC_ROUTINE_CHANNEL);
}

void msp_adc_get(void)
{

    // 获取电位器
    uint16_t encode = adc_results[0];
    float vol = (encode * 3.3) / 4096;
    printf("vol:%f\n", vol);

    // 获取内部温度
    encode = adc_results[1];
    // printf("encode = %d\r\n", encode);

    // 根据基准电压计算实际电压
    vol = (encode * 3.3f) / 4096;
    // printf("vol = %.2fV\r\n", vol);

    // 计算得到温度值 (datasheet中找到公式)
    // (v25 - v) / avg_slope + 25
    float temp = (1.45f - vol) * 1000 / 4.1f + 25;
    printf("temp = %.2f\r\n", temp);
}

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
    // DMA
    msp_adc_dma_config();
    // ADC 内部温度
    msp_adc_config();
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

    // 188LED
    bsp_188_leds_init();

    printf("============ start ============\n");
    printf("SystemCoreClock = %u\r\n", (unsigned int)SystemCoreClock);

    // // DMA搬运
    // DMA_m2m_config();

    // time.year = 2026;
    // time.month = 9;
    // time.day = 24;
    // time.week = 4;
    // time.hour = 14;
    // time.min = 59;
    // time.sec = 55;
    // msp_rtc_write(&time);

    // time.day = 24;
    // time.hour = 15;
    // time.min = 0;
    // time.sec = 0;
    // // 闹钟配置
    // msp_rtc_alarm_config(&time);

    // 独立看门狗
    // msp_fwdgt_config();
    // 窗口看门狗
    // msp_wwdgt_config(); // 前面的初始化会消耗时间

    // uint8_t dat[7] = {0};

    // dat[0] = 0x55; // 0x37
    // dat[1] = 0x59;
    // dat[2] = 0x23;
    // dat[3] = 0x26;
    // dat[4] = 6;
    // dat[5] = 0x09 | (1 << 7);
    // dat[6] = 0x26;

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

    bsp_iic_oled_show_picture(0, 0, 128, 64, BMP1, 1);
    bsp_iic_oled_refresh();

    // bsp_spi_oled_display_128x64(BMP1);
    bsp_spi_oled_display_GB2312_string(0, 0, "秀儿");

    flash_test();

    char buf[64] = {0};

    uint8_t cnt = 0;
    uint32_t num = 0;
    while (1)
    {
        num = get_us_cnt();
        bsp_188_leds_clear();
        bsp_188_leds_set_num(num / 1000000 % 1000);

        // oled_test();

        // delay_1ms(1000);

        msp_adc_get();

        bsp_pcf8563_read_time(&time);

        oled_show_rtc(&time);
        // sprintf(buf, "20%02d-%02d-%02d W:%d\r\n",
        //         time.year % 100, time.month, time.day, time.week);

        // OLED_ShowString(0, 0, buf, 16, 1);

        // sprintf(buf, "%02d:%02d:%02d\r\n",
        //         time.hour, time.minutes, time.second);

        // OLED_ShowString(0, 16, buf, 16, 1);

        // OLED_Refresh();

        printf("20%02d-%02d-%02d %02d:%02d:%02d Week:%d\r\n",
               time.year % 100, time.month, time.day,
               time.hour, time.minutes, time.second, time.week);

        // 喂狗
        // wwdgt_counter_update(0x7F);

        cnt++;
    }
}