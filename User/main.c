#include "main.h"
#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_battery_flows.h"
#include "bsp_buzzer.h"
#include "bsp_keys.h"
#include "bsp_leds.h"
#include "msp_exti.h"
#include "msp_uart.h"

// 0x1A  0010 0110
// 26->0x26   26/10 = 2   26%10 = 6
#define DEC2BCD(val) ((((val) / 10) << 4) | ((val) % 10))
// 0x26->26  0x26>>4=2  0x26&0x0F = 6
#define BCD2DEC(val) (((val) >> 4) * 10 + ((val) & 0x0F))

// 日历时间结构体
typedef struct
{
    uint16_t year; // 年里包含世纪
    uint8_t month;
    uint8_t day;
    uint8_t week;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} Time;

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

void msp_rtc_config(void)
{

    // 打开PMU时钟
    rcu_periph_clock_enable(RCU_PMU);
    // 通过PMU解除备份域写保护
    // pmu_deinit();
    pmu_backup_write_enable();
    // 备份域重置
    rcu_bkp_reset_enable();
    rcu_bkp_reset_disable();

    // 打开RTC时钟
    rcu_periph_clock_enable(RCU_RTC);
    // 复位RTC
    // rtc_deinit();
    // 配置RTC时钟源 使用外部高速晶振
    // 8000000 / 25 = 320000
    rcu_rtc_div_config(RCU_RTC_HXTAL_DIV25);
    rcu_rtc_clock_config(RCU_RTCSRC_HXTAL_DIV_RTCDIV);
    // 等待影子寄存器同步
    if (rtc_register_sync_wait() == ERROR)
    {
        printf("rtc_register_sync_wait error\n");
        return;
    }
    printf("rtc_register_sync_wait ok\n");

    // 初始化RTC
    // rtc_parameter_struct rtc_init_struct;
    // // f_rct_clock/(f_a +1)/(f_s+1) = 320000
    // // 异步预分频尽可能大
    // rtc_init_struct.factor_asyn = 0x7f;  /*!< RTC asynchronous prescaler value: 0x0 - 0x7F */
    // rtc_init_struct.factor_syn = 0x09c3; /*!< RTC synchronous prescaler value: 0x0 - 0x7FFF */

    // rtc_init_struct.year = 0x26;     /*!< RTC year value: 0x0 - 0x99(BCD format) */
    // rtc_init_struct.month = 0x09;    /*!< RTC month value */
    // rtc_init_struct.date = 0x24;     /*!< RTC date value: 0x1 - 0x31(BCD format) */
    // rtc_init_struct.day_of_week = 4; /*!< RTC weekday value */
    // rtc_init_struct.hour = 0x11;     /*!< RTC hour value */
    // rtc_init_struct.minute = 0x59;   /*!< RTC minute value: 0x0 - 0x59(BCD format) */
    // rtc_init_struct.second = 0x50;   /*!< RTC second value: 0x0 - 0x59(BCD format) */
    // rtc_init_struct.am_pm = RTC_AM;  /*!< RTC AM/PM value */
    // rtc_init_struct.display_format = RTC_24HOUR;
    // rtc_init(&rtc_init_struct);
}

void msp_rtc_read(Time *time)
{
    rtc_parameter_struct rtc_initpara_struct;
    rtc_current_time_get(&rtc_initpara_struct);

    time->year = 2000 + BCD2DEC(rtc_initpara_struct.year);
    time->month = BCD2DEC(rtc_initpara_struct.month);
    time->day = BCD2DEC(rtc_initpara_struct.date);
    time->week = rtc_initpara_struct.day_of_week;
    time->hour = BCD2DEC(rtc_initpara_struct.hour);
    time->min = BCD2DEC(rtc_initpara_struct.minute);
    time->sec = BCD2DEC(rtc_initpara_struct.second);

    // printf("sec=%#x, min=%#x, hour=%#x\n", (int)rtc_initpara_struct.second, (int)rtc_initpara_struct.minute, (int)rtc_initpara_struct.hour);
    // printf("day=%#x, week=%d,month=%#x, year=%#x\n", (int)rtc_initpara_struct.date, (int)rtc_initpara_struct.day_of_week, (int)rtc_initpara_struct.month, (int)rtc_initpara_struct.year);
}

void msp_rtc_write(Time *time)
{
    rtc_parameter_struct rtc_initpara_struct;
    // 同步和异步预分频率320000/factor_asyn/factor_syn = 1
    // f_rtcclk/(f_a+1)/(f_s+1) = 1hz
    rtc_initpara_struct.factor_asyn = 0x7F; /*!< RTC asynchronous prescaler value: 0x0 - 0x7F */
    rtc_initpara_struct.factor_syn = 0x9C3; /*!< RTC synchronous prescaler value: 0x0 - 0x7FFF */
    // 设置初始时间
    rtc_initpara_struct.year = DEC2BCD(time->year % 100); /*!< RTC year value: 0x0 - 0x99(BCD format) */
    rtc_initpara_struct.month = DEC2BCD(time->month);     /*!< RTC month value */
    rtc_initpara_struct.date = DEC2BCD(time->day);        /*!< RTC date value: 0x1 - 0x31(BCD format) */
    rtc_initpara_struct.day_of_week = 4;                  /*!< RTC weekday value */
    rtc_initpara_struct.hour = DEC2BCD(time->hour);       /*!< RTC hour value */
    rtc_initpara_struct.minute = DEC2BCD(time->min);      /*!< RTC minute value: 0x0 - 0x59(BCD format) */
    rtc_initpara_struct.second = DEC2BCD(time->sec);      /*!< RTC second value: 0x0 - 0x59(BCD format) */
    rtc_initpara_struct.am_pm = RTC_AM;                   /*!< RTC AM/PM value */
    rtc_initpara_struct.display_format = RTC_24HOUR;      /*!< RTC time notation 24小时制/12小时制*/
    rtc_init(&rtc_initpara_struct);
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
    msp_rtc_config();
    //============ 片外外设 ============
    // LED灯组初始化
    bsp_leds_config();
    // 按键初始化
    bsp_keys_config();
    // 蜂鸣器初始化
    bsp_buzzer_init();

    printf("============ start ============\n");
    printf("SystemCoreClock = %u\r\n", (unsigned int)SystemCoreClock);

    // DMA搬运
    DMA_m2m_config();

    time.year = 2026;
    time.month = 9;
    time.day = 24;
    time.week = 4;
    time.hour = 14;
    time.min = 59;
    time.sec = 55;
    msp_rtc_write(&time);

    uint8_t cnt = 0;
    while (1)
    {
        if (cnt == 50)
        {
            cnt = 0;
            // 读取RTC
            msp_rtc_read(&time);
            printf("sec=%d, min=%d, hour=%d\n", (int)time.sec, (int)time.min, (int)time.hour);
            printf("day=%d, week=%d, month=%d, year=%d\n", (int)time.day, (int)time.week, (int)time.month, (int)time.year);
        }
        delay_1ms(20);
        cnt++;
    }
}