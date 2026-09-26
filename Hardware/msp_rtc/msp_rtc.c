#include "msp_rtc.h"
#include "systick.h"
#include <stdio.h>

static RTC_CLOCK_SOURCE rtc_clock_source = HXTAL;

static void msp_rtc_config(RTC_CLOCK_SOURCE rcs)
{

    // 打开PMU时钟
    rcu_periph_clock_enable(RCU_PMU);
    // 通过PMU解除备份域写保护
    // pmu_deinit();
    pmu_backup_write_enable();
    // 备份域重置
    rcu_bkp_reset_enable();
    rcu_bkp_reset_disable();

    if (rcs == HXTAL)
    {
        // 外部高速晶振 8mhz
        // 打开RTC时钟
        rcu_periph_clock_enable(RCU_RTC);
        // 配置RTC时钟源
        // 8000000 / 25 = 320000
        rcu_rtc_div_config(RCU_RTC_HXTAL_DIV25);
        rcu_rtc_clock_config(RCU_RTCSRC_HXTAL_DIV_RTCDIV);
    }
    else if (rcs == LXTAL)
    {
        // 外部32768hz震荡时钟
        rcu_osci_off(RCU_LXTAL);
        delay_1ms(1);
        rcu_osci_on(RCU_LXTAL); // 打开震荡时钟

        if (rcu_osci_stab_wait(RCU_LXTAL) == ERROR)
        { // 等待稳定
            printf("turn_on_osci_error\n");
            return;
        }

        // 打开RTC时钟
        rcu_periph_clock_enable(RCU_RTC);
        // 配置RTC时钟源(外部高速晶振 32000)
        rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
    }
    else if (rcs == IRC32K)
    {
        // 内部32K震荡时钟
        rcu_osci_off(RCU_IRC32K);
        delay_1ms(1);
        rcu_osci_on(RCU_IRC32K); // 打开震荡时钟

        if (rcu_osci_stab_wait(RCU_IRC32K) == ERROR)
        { // 等待稳定
            printf("turn_on_osci_error\n");
            return;
        }

        rcu_periph_clock_enable(RCU_RTC);
        // 3.配置RTC时钟源(外部高速晶振 32000)
        rcu_rtc_clock_config(RCU_RTCSRC_IRC32K);
    }

    // 等待影子寄存器同步
    if (rtc_register_sync_wait() == ERROR)
    {
        printf("rtc_register_sync_wait error\n");
        return;
    }
    printf("rtc_register_sync_wait ok\n");
}

// 初始化RTC时钟
void msp_rtc_init(RTC_CLOCK_SOURCE rcs)
{
    rtc_clock_source = rcs;
    // 初始化rtc
    msp_rtc_config(rcs);
    Time time = {0};
    msp_rtc_write(&time);
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
    if (rtc_clock_source == HXTAL)
    {
        rtc_initpara_struct.factor_asyn = 0x7F; /*!< RTC asynchronous prescaler value: 0x0 - 0x7F */
        rtc_initpara_struct.factor_syn = 0x9C3; /*!< RTC synchronous prescaler value: 0x0 - 0x7FFF */
    }
    else if (rtc_clock_source == LXTAL)
    {
        rtc_initpara_struct.factor_asyn = 0x7F; /*!< RTC asynchronous prescaler value: 0x0 - 0x7F */
        rtc_initpara_struct.factor_syn = 0xFF;  /*!< RTC synchronous prescaler value: 0x0 - 0x7FFF */
    }
    else if (rtc_clock_source == IRC32K)
    {
        rtc_initpara_struct.factor_asyn = 0x7F; /*!< RTC asynchronous prescaler value: 0x0 - 0x7F */
        rtc_initpara_struct.factor_syn = 0xF9;  /*!< RTC synchronous prescaler value: 0x0 - 0x7FFF */
    }
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

void msp_rtc_alarm_config(Time *time)
{
    rtc_alarm_struct alarm_struct;
    alarm_struct.alarm_mask = RTC_ALARM_DATE_MASK | RTC_ALARM_HOUR_MASK | RTC_ALARM_MINUTE_MASK; /*!< RTC alarm mask */
    alarm_struct.weekday_or_date = RTC_ALARM_DATE_SELECTED;                                      /*!< specify RTC alarm is on date or weekday */
    alarm_struct.alarm_day = DEC2BCD(time->day);                                                 /*!< RTC alarm date or weekday value*/
    alarm_struct.alarm_hour = DEC2BCD(time->hour);                                               /*!< RTC alarm hour value */
    alarm_struct.alarm_minute = DEC2BCD(time->min);                                              /*!< RTC alarm minute value: 0x0 - 0x59(BCD format) */
    alarm_struct.alarm_second = DEC2BCD(time->sec);                                              /*!< RTC alarm second value: 0x0 - 0x59(BCD format) */
    alarm_struct.am_pm = RTC_AM;

    rtc_alarm_config(RTC_ALARM0, &alarm_struct);

    // 清标志位
    rtc_flag_clear(RTC_FLAG_ALRM0);
    exti_flag_clear(EXTI_17);
    // 配置 NVIC 中断
    nvic_irq_enable(RTC_Alarm_IRQn, 2, 2);
    // 配置 EXTI 外部中断线
    exti_init(EXTI_17, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    // 配置EXTI中断与使能
    exti_interrupt_flag_clear(EXTI_17); // EXTI线编号17是RTC闹钟
    exti_interrupt_enable(EXTI_17);
    // 配置RTC闹钟中断与使能
    rtc_flag_clear(RTC_FLAG_ALRM0);
    rtc_interrupt_enable(RTC_INT_ALARM0);

    rtc_alarm_enable(RTC_ALARM0);
}

#ifndef __weak
#define __weak __attribute__((weak)) // GCC / armclang
#endif
__weak void on_rtc_alarm()
{
    printf("============ alarm ============\n");
}

// 闹钟中断处理函数
void RTC_Alarm_IRQHandler()
{
    if (exti_interrupt_flag_get(EXTI_17) == SET)
    {
        exti_interrupt_flag_clear(EXTI_17);
        rtc_flag_clear(RTC_FLAG_ALRM0);

        on_rtc_alarm();
    }
}