#include "bsp_spi_oled.h"
#include <stdlib.h>

// 反显函数
void bsp_spi_oled_color_turn(uint8_t i)
{
    if (i == 0)
    {
        bsp_spi_oled_write_byte(0xA6, OLED_CMD); // 正常显示
    }
    if (i == 1)
    {
        bsp_spi_oled_write_byte(0xA7, OLED_CMD); // 反色显示
    }
}

// 屏幕旋转180度
void bsp_spi_oled_display_turn(uint8_t i)
{
    if (i == 0)
    {
        bsp_spi_oled_write_byte(0xC8, OLED_CMD); // 正常显示
        bsp_spi_oled_write_byte(0xA1, OLED_CMD);
    }
    if (i == 1)
    {
        bsp_spi_oled_write_byte(0xC0, OLED_CMD); // 反转显示
        bsp_spi_oled_write_byte(0xA0, OLED_CMD);
    }
}

// 开启OLED显示
static void OLED_DisPlay_On(void)
{
    bsp_spi_oled_write_byte(0x8D, OLED_CMD); // 电荷泵使能
    bsp_spi_oled_write_byte(0x14, OLED_CMD); // 开启电荷泵
    bsp_spi_oled_write_byte(0xAF, OLED_CMD); // 点亮屏幕
}

// 关闭OLED显示
static void OLED_DisPlay_Off(void)
{
    bsp_spi_oled_write_byte(0x8D, OLED_CMD); // 电荷泵使能
    bsp_spi_oled_write_byte(0x10, OLED_CMD); // 关闭电荷泵
    bsp_spi_oled_write_byte(0xAE, OLED_CMD); // 关闭屏幕
}

// 向SSD1306写入一个字节。
// mode:数据/命令标志 0,表示命令;1,表示数据;
void bsp_spi_oled_write_byte(uint8_t dat, uint8_t cmd)
{
    uint8_t i;
    if (cmd) // 数据 拉高DC
        OLED_DC_Set();
    else // 指令 拉低DC
        OLED_DC_Clr();

    // 拉低片选 开始通信
    OLED_CS_Clr();

    // 发送1个byte
    OLED_WRITE_BYTE(dat);

    // // 0x55  0b01 010100
    // for (i = 0; i < 8; i++)
    // {
    //     // SCL拉低  写入数据
    //     OLED_SCL_Clr();
    //     if (dat & 0x80) // 0b1000 0000
    //         OLED_SDA_Set();
    //     else
    //         OLED_SDA_Clr();
    //     // 拉高 等待对方读取
    //     OLED_SCL_Set();
    //     dat <<= 1;
    // }

    // 拉高片选 结束通信
    OLED_CS_Set();
    OLED_DC_Set();
}

// 清屏函数
void bsp_spi_oled_clear(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        bsp_spi_oled_write_byte(0xb0 + i, OLED_CMD); // 设置页地址
        bsp_spi_oled_write_byte(0x10, OLED_CMD);     // 设置列地址的高4位
        bsp_spi_oled_write_byte(0x00, OLED_CMD);     // 设置列地址的低4位
        for (n = 0; n < 128; n++)
        {
            bsp_spi_oled_write_byte(0x00, OLED_DATA); // 清除所有数据
        }
    }
}

// 设置起始地址
void bsp_spi_oled_address(uint8_t x, uint8_t y)
{
    bsp_spi_oled_write_byte(0xb0 + y, OLED_CMD);                 // 设置页地址
    bsp_spi_oled_write_byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD); // 设置列地址的高4位
    bsp_spi_oled_write_byte((x & 0x0f), OLED_CMD);               // 设置列地址的低4位
}

// 显示128x64点阵图像
void bsp_spi_oled_display_128x64(uint8_t *dp)
{
    uint8_t i, j;
    for (i = 0; i < 8; i++)
    {
        bsp_spi_oled_address(0, i);
        for (j = 0; j < 128; j++)
        {
            bsp_spi_oled_write_byte(*dp, OLED_DATA); // 写数据到OLED,每写完一个8位的数据后列地址自动加1
            dp++;
        }
    }
}

// 显示16x16点阵图像、汉字、生僻字或16x16点阵的其他图标
void bsp_spi_oled_display_16x16(uint8_t x, uint8_t y, uint8_t *dp)
{
    uint8_t i, j;
    for (j = 0; j < 2; j++)
    {
        bsp_spi_oled_address(x, y);
        for (i = 0; i < 16; i++)
        {
            bsp_spi_oled_write_byte(*dp, OLED_DATA); // 写数据到OLED,每写完一个8位的数据后列地址自动加1
            dp++;
        }
        y++;
    }
}

// 显示8x16点阵图像、ASCII, 或8x16点阵的自造字符、其他图标
void bsp_spi_oled_display_8x16(uint8_t x, uint8_t y, uint8_t *dp)
{
    uint8_t i, j;
    for (j = 0; j < 2; j++)
    {
        bsp_spi_oled_address(x, y);
        for (i = 0; i < 8; i++)
        {
            bsp_spi_oled_write_byte(*dp, OLED_DATA); // 写数据到LCD,每写完一个8位的数据后列地址自动加1
            dp++;
        }
        y++;
    }
}

// 显示5*7点阵图像、ASCII, 或5x7点阵的自造字符、其他图标
void bsp_spi_oled_display_5x7(uint8_t x, uint8_t y, uint8_t *dp)
{
    uint8_t i;
    bsp_spi_oled_address(x, y);
    for (i = 0; i < 6; i++)
    {
        bsp_spi_oled_write_byte(*dp, OLED_DATA);
        dp++;
    }
}

// 替换为msp_spi中的方法

// // 送指令到晶联讯字库IC
// void Send_Command_to_ROM(uint8_t dat)
// {
//     uint8_t i;
//     for (i = 0; i < 8; i++)
//     {
//         OLED_SCL_Clr();
//         if (dat & 0x80)
//         {
//             OLED_SDA_Set();
//         }
//         else
//         {
//             OLED_SDA_Clr();
//         }
//         dat <<= 1;
//         OLED_SCL_Set();
//     }
// }

// // 从晶联讯字库IC中取汉字或字符数据（1个字节）
// uint8_t Get_data_from_ROM(void)
// {
//     uint8_t i, read = 0;
//     for (i = 0; i < 8; i++)
//     {
//         OLED_SCL_Clr();
//         read <<= 1;
//         if (OLED_READ_FS0())
//         {
//             read++;
//         }
//         OLED_SCL_Set();
//     }
//     return read;
// }

// 从相关地址（addrHigh：地址高字节,addrMid：地址中字节,addrLow：地址低字节）中连续读出DataLen个字节的数据到 pbuff的地址
// 连续读取
void bsp_spi_oled_get_data_form_ROM(uint8_t addrHigh, uint8_t addrMid, uint8_t addrLow, uint8_t *pbuff, uint8_t DataLen)
{
    uint8_t i;
    OLED_ROM_CS_Clr();
    Send_Command_to_ROM(0x03);
    Send_Command_to_ROM(addrHigh);
    Send_Command_to_ROM(addrMid);
    Send_Command_to_ROM(addrLow);
    for (i = 0; i < DataLen; i++)
    {
        *(pbuff + i) = Get_data_from_ROM();
    }
    OLED_ROM_CS_Set();
}

uint32_t fontaddr = 0;
void bsp_spi_oled_display_GB2312_string(uint8_t x, uint8_t y, uint8_t *text)
{
    uint8_t i = 0;
    uint8_t addrHigh, addrMid, addrLow;
    uint8_t fontbuf[32];
    while (text[i] > 0x00)
    {
        if ((text[i] >= 0xb0) && (text[i] <= 0xf7) && (text[i + 1] >= 0xa1))
        {
            // 国标简体（GB2312）汉字在晶联讯字库IC中的地址由以下公式来计算：
            // Address = ((MSB - 0xB0) * 94 + (LSB - 0xA1)+ 846)*32+ BaseAdd;BaseAdd=0
            // 由于担心8位单片机有乘法溢出问题，所以分三部取地址
            fontaddr = (text[i] - 0xb0) * 94;
            fontaddr += (text[i + 1] - 0xa1) + 846;
            fontaddr = fontaddr * 32;

            addrHigh = (fontaddr & 0xff0000) >> 16; // 地址的高8位,共24位
            addrMid = (fontaddr & 0xff00) >> 8;     // 地址的中8位,共24位
            addrLow = (fontaddr & 0xff);            // 地址的低8位,共24位

            bsp_spi_oled_get_data_form_ROM(addrHigh, addrMid, addrLow, fontbuf, 32);
            // 取32个字节的数据，存到"fontbuf[32]"
            bsp_spi_oled_display_16x16(x, y, fontbuf);
            // 显示汉字到LCD上，y为页地址，x为列地址，fontbuf[]为数据
            x += 16;
            i += 2;
        }
        else if ((text[i] >= 0xa1) && (text[i] <= 0xa3) && (text[i + 1] >= 0xa1))
        {

            fontaddr = (text[i] - 0xa1) * 94;
            fontaddr += (text[i + 1] - 0xa1);
            fontaddr = fontaddr * 32;

            addrHigh = (fontaddr & 0xff0000) >> 16;
            addrMid = (fontaddr & 0xff00) >> 8;
            addrLow = (fontaddr & 0xff);

            bsp_spi_oled_get_data_form_ROM(addrHigh, addrMid, addrLow, fontbuf, 32);
            bsp_spi_oled_display_16x16(x, y, fontbuf);
            x += 16;
            i += 2;
        }
        else if ((text[i] >= 0x20) && (text[i] <= 0x7e))
        {
            unsigned char fontbuf[16];
            fontaddr = (text[i] - 0x20);
            fontaddr = (unsigned long)(fontaddr * 16);
            fontaddr = (unsigned long)(fontaddr + 0x3cf80);

            addrHigh = (fontaddr & 0xff0000) >> 16;
            addrMid = (fontaddr & 0xff00) >> 8;
            addrLow = fontaddr & 0xff;

            bsp_spi_oled_get_data_form_ROM(addrHigh, addrMid, addrLow, fontbuf, 16);
            bsp_spi_oled_display_8x16(x, y, fontbuf);
            x += 8;
            i += 1;
        }
        else
            i++;
    }
}

void bsp_spi_oled_display_string_5x7(uint8_t x, uint8_t y, uint8_t *text)
{
    uint8_t i = 0;
    uint8_t addrHigh, addrMid, addrLow;
    while (text[i] > 0x00)
    {
        if ((text[i] >= 0x20) && (text[i] <= 0x7e))
        {
            uint8_t fontbuf[8];
            fontaddr = (text[i] - 0x20);
            fontaddr = (unsigned long)(fontaddr * 8);
            fontaddr = (unsigned long)(fontaddr + 0x3bfc0);

            addrHigh = (fontaddr & 0xff0000) >> 16;
            addrMid = (fontaddr & 0xff00) >> 8;
            addrLow = fontaddr & 0xff;

            bsp_spi_oled_get_data_form_ROM(addrHigh, addrMid, addrLow, fontbuf, 8);
            bsp_spi_oled_display_5x7(x, y, fontbuf);
            x += 6;
            i += 1;
        }
        else
            i++;
    }
}

// 显示2个数字
// x,y :起点坐标
// num1：要显示的小数
// len :数字的位数
void bsp_spi_oled_show_number(uint8_t x, uint8_t y, float num1, uint8_t len)
{
    uint8_t i;
    uint32_t t, num;
    x = x + len * 8 + 8;                                // 要显示的小数最低位的横坐标
    num = num1 * 100;                                   // 将小数左移两位并转化为整数
    bsp_spi_oled_display_GB2312_string(x - 24, y, "."); // 显示小数点
    for (i = 0; i < len; i++)
    {
        t = num % 10;   // 取个位数的数值
        num = num / 10; // 将整数右移一位
        x -= 8;
        if (i == 2)
        {
            x -= 8;
        } // 当显示出来两个小数之后，空出小数点的位置
        switch (t)
        {
        case 0:
            bsp_spi_oled_display_GB2312_string(x, y, "0");
            break;
        case 1:
            bsp_spi_oled_display_GB2312_string(x, y, "1");
            break;
        case 2:
            bsp_spi_oled_display_GB2312_string(x, y, "2");
            break;
        case 3:
            bsp_spi_oled_display_GB2312_string(x, y, "3");
            break;
        case 4:
            bsp_spi_oled_display_GB2312_string(x, y, "4");
            break;
        case 5:
            bsp_spi_oled_display_GB2312_string(x, y, "5");
            break;
        case 6:
            bsp_spi_oled_display_GB2312_string(x, y, "6");
            break;
        case 7:
            bsp_spi_oled_display_GB2312_string(x, y, "7");
            break;
        case 8:
            bsp_spi_oled_display_GB2312_string(x, y, "8");
            break;
        case 9:
            bsp_spi_oled_display_GB2312_string(x, y, "9");
            break;
        }
    }
}

// OLED的初始化
void bsp_spi_oled_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);

    gpio_bit_set(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);
    gpio_bit_set(GPIOC, GPIO_PIN_5);

    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_2 | GPIO_PIN_3);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_2 | GPIO_PIN_3);

    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_5);

    delay_1ms(200);

    bsp_spi_oled_write_byte(0xAE, OLED_CMD); //--turn off oled panel
    bsp_spi_oled_write_byte(0x00, OLED_CMD); //---set low column address
    bsp_spi_oled_write_byte(0x10, OLED_CMD); //---set high column address
    bsp_spi_oled_write_byte(0x40, OLED_CMD); //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    bsp_spi_oled_write_byte(0x81, OLED_CMD); //--set contrast control register
    bsp_spi_oled_write_byte(0xCF, OLED_CMD); // Set SEG Output Current Brightness
    bsp_spi_oled_write_byte(0xA1, OLED_CMD); //--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    bsp_spi_oled_write_byte(0xC8, OLED_CMD); // Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    bsp_spi_oled_write_byte(0xA6, OLED_CMD); //--set normal display
    bsp_spi_oled_write_byte(0xA8, OLED_CMD); //--set multiplex ratio(1 to 64)
    bsp_spi_oled_write_byte(0x3f, OLED_CMD); //--1/64 duty
    bsp_spi_oled_write_byte(0xD3, OLED_CMD); //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    bsp_spi_oled_write_byte(0x00, OLED_CMD); //-not offset
    bsp_spi_oled_write_byte(0xd5, OLED_CMD); //--set display clock divide ratio/oscillator frequency
    bsp_spi_oled_write_byte(0x80, OLED_CMD); //--set divide ratio, Set Clock as 100 Frames/Sec
    bsp_spi_oled_write_byte(0xD9, OLED_CMD); //--set pre-charge period
    bsp_spi_oled_write_byte(0xF1, OLED_CMD); // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    bsp_spi_oled_write_byte(0xDA, OLED_CMD); //--set com pins hardware configuration
    bsp_spi_oled_write_byte(0x12, OLED_CMD);
    bsp_spi_oled_write_byte(0xDB, OLED_CMD); //--set vcomh
    bsp_spi_oled_write_byte(0x40, OLED_CMD); // Set VCOM Deselect Level
    bsp_spi_oled_write_byte(0x20, OLED_CMD); //-Set Page Addressing Mode (0x00/0x01/0x02)
    bsp_spi_oled_write_byte(0x02, OLED_CMD); //
    bsp_spi_oled_write_byte(0x8D, OLED_CMD); //--set Charge Pump enable/disable
    bsp_spi_oled_write_byte(0x14, OLED_CMD); //--set(0x10) disable
    bsp_spi_oled_write_byte(0xA4, OLED_CMD); // Disable Entire Display On (0xa4/0xa5)
    bsp_spi_oled_write_byte(0xA6, OLED_CMD); // Disable Inverse Display On (0xa6/a7)
    bsp_spi_oled_clear();
    bsp_spi_oled_write_byte(0xAF, OLED_CMD); /*display ON*/
}
