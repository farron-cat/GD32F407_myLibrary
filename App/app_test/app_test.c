#include "app.h"

void flash_test(void);
void oled_test(void);

Time_pcf time;
Alarm alarm;

void app_test_flash(void)
{
    // FLASH 测试
    flash_test();
}

void app_test_iic_oled(void)
{
    // IIC屏幕显示图片
    bsp_iic_oled_show_picture(0, 0, 128, 64, BMP1, 1);
    bsp_iic_oled_refresh();
}

void app_test_spi_oled(void)
{
    // SPI屏幕显示图片
    // bsp_spi_oled_display_128x64(BMP1);
    // SPI屏幕显示中文
    bsp_spi_oled_display_GB2312_string(0, 0, "不要瞎搞");
}

void app_test_digital_leds(void)
{
    // 数码管测试
    // 先刷新一次缓冲区，让数码管上电就显示 0000 0000
    clear_buffer();
    bsp_digital_leds_all_zero();
}

void app_test_pcf8563_set_time_alarm(void)
{
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