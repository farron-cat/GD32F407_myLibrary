#include "bsp_dot_matrix.h"
#include "msp_spi.h"
#include "systick.h"

#define RCU_MAX7219_CS RCU_GPIOA
#define PORT_MAX7219_CS GPIOA
#define GPIO_MAX7219_CS GPIO_PIN_8

#define MAX7219_CS(X) gpio_bit_write(PORT_MAX7219_CS, GPIO_MAX7219_CS, X ? SET : RESET)

#define Write_Max7219_byte(dat) msp_spi_write_byte(dat)

unsigned char num_display_map[12][8] = {
    // 一共12行
    {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x66, 0x38}, /*"0",0*/
    {0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x18}, /*"1",1*/
    {0x7C, 0x42, 0x02, 0x04, 0x08, 0x30, 0x42, 0x7E}, /*"2",2*/
    {0x7C, 0x46, 0x04, 0x18, 0x06, 0x02, 0x42, 0x3C}, /*"3",3*/
    {0x0C, 0x14, 0x14, 0x24, 0x44, 0x3C, 0x04, 0x0C}, /*"4",4*/
    {0x3E, 0x40, 0x58, 0x64, 0x02, 0x02, 0x42, 0x3C}, /*"5",5*/
    {0x3E, 0x40, 0x48, 0x76, 0x42, 0x42, 0x42, 0x3C}, /*"6",6*/
    {0x7E, 0x04, 0x04, 0x08, 0x10, 0x10, 0x10, 0x10}, /*"7",7*/
    {0x7C, 0x42, 0x62, 0x3C, 0x44, 0x42, 0x42, 0x3C}, /*"8",8*/
    {0x7C, 0x42, 0x42, 0x46, 0x3A, 0x02, 0x44, 0x38}, /*"9",9*/
};

static void bsp_dot_matrix_gpio_config(void)
{
    /* 开启时钟 */
    rcu_periph_clock_enable(RCU_MAX7219_CS);

    /* 配置CS */ // CS
    gpio_mode_set(PORT_MAX7219_CS, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_MAX7219_CS);
    gpio_output_options_set(PORT_MAX7219_CS, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_MAX7219_CS);
    gpio_bit_set(PORT_MAX7219_CS, GPIO_MAX7219_CS);
}

static void Write_Max7219(uint8_t address, uint8_t dat)
{
    MAX7219_CS(0);               // 片选拉低
    Write_Max7219_byte(address); // 写入地址，即点阵行号1-8
    Write_Max7219_byte(dat);     // 写入数据，即该行显示内容
    MAX7219_CS(1);               // 片选拉高
}

static void bsp_dot_matrix_max7219_init(void)
{
    unsigned int i = 0;
    Write_Max7219(0x09, 0x00); // 译码方式：不进行译码
    Write_Max7219(0x0a, 0x00); // 亮度
    Write_Max7219(0x0b, 0x07); // 扫描界限；8个数码管显示
    Write_Max7219(0x0c, 0x01); // 掉电模式：0，普通模式：1
    Write_Max7219(0x0f, 0x00); // 显示测试：1；测试结束，正常显示：0
}

void bsp_dot_matrix_init(void)
{
    bsp_dot_matrix_gpio_config();
    bsp_dot_matrix_max7219_init();
}

void bsp_dot_matrix_clear(void)
{
    uint8_t i = 0;
    for (i = 1; i < 9; i++) // 1~8行 进行更新
    {
        Write_Max7219(i, 0x00); // 清空
    }
}

void bsp_dot_matrix_display(uint8_t *show)
{
    uint8_t i = 0;
    for (i = 1; i < 9; i++) // 1~8行 进行更新
    {
        Write_Max7219(i, show[i - 1]);
    }
}

void bsp_dot_matrix_display_num(uint8_t num)
{
    bsp_dot_matrix_display(num_display_map[num]);
}

// 每 1 秒返回一次 1
static uint8_t bsp_dot_matrix_is_1s(void)
{
    static uint64_t last_cnt = 0;
    uint64_t cnt = get_us_cnt();

    if (cnt - last_cnt >= 1000000ULL) // 1,000,000 us = 1 s
    {
        last_cnt = cnt;
        return 1;
    }
    return 0;
}

void bsp_dot_matrix_test_tick(void)
{
    static uint16_t number = 0;

    if (bsp_dot_matrix_is_1s())
    {
        number++;
        if (number > 9)
        {
            number = 0;
        }
        bsp_dot_matrix_display_num(number);
    }
}