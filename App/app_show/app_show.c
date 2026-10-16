#include "app.h"

void oled_show_rtc(Time_pcf *t);

void app_show_init(void)
{
    ;
}

void app_show_188_leds_task(void)
{
    // 188数码管显示NTC温度
    bsp_188_leds_set_num(bsp_ntc_get_tem());
}

void app_show_pcf8563_task(void)
{
    // PCF8563 中断事件处理：原在 EXTI5 中断中执行，移到主循环以避免软件 IIC 重入
    if (g_rtc_int_flag)
    {
        g_rtc_int_flag = 0;
        on_rtc_interrupt();
    }
}

void app_show_iic_oled_task(void)
{
    // 读取时间
    bsp_pcf8563_read_time(&time);
    // IIC屏幕图片上叠加时间
    oled_show_rtc(&time);
}

void app_show_digital_leds_task(void)
{
    // 数码管测试 每秒加1
    bsp_digital_leds_test_tick();
}

void app_show_dot_matrix_task(void)
{
    // 点阵屏测试
    bsp_dot_matrix_test_tick();
}

void app_show_ntc_task(void)
{
    // 串口打印NTC温度
    printf("NTC Temperature: %d C\r\n", bsp_ntc_get_tem());
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