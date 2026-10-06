#ifndef __OLED_H
#define __OLED_H

#include "gd32f4xx.h"
#include "systick.h"
#include "msp_spi.h"

// 替换宏
#define OLED_WRITE_BYTE(byte)     msp_spi_write_byte(byte)
#define Send_Command_to_ROM(byte) OLED_WRITE_BYTE(byte)
#define Get_data_from_ROM()       msp_spi_read_byte()

// SCL=SCLK
// SDA=MOSI
// DC=DC
// CS=CS1
// FS0=MOSI
// CS2=CS2
//-----------------OLED端口定义----------------
// SCL  PA5
// #define OLED_SCL_Clr() gpio_bit_reset(GPIOA, GPIO_PIN_5) // SCL
// #define OLED_SCL_Set() gpio_bit_set(GPIOA, GPIO_PIN_5)
// MOSI  PA7
// #define OLED_SDA_Clr() gpio_bit_reset(GPIOA, GPIO_PIN_7) // SDA
// #define OLED_SDA_Set() gpio_bit_set(GPIOA, GPIO_PIN_7)
// DC  PA2
#define OLED_DC_Clr() gpio_bit_reset(GPIOA, GPIO_PIN_2) // DC
#define OLED_DC_Set() gpio_bit_set(GPIOA, GPIO_PIN_2)
// CS1 	PA3
#define OLED_CS_Clr() gpio_bit_reset(GPIOA, GPIO_PIN_3) // CS1
#define OLED_CS_Set() gpio_bit_set(GPIOA, GPIO_PIN_3)
// MISO  PA6
// #define OLED_READ_FS0() gpio_input_bit_get(GPIOA, GPIO_PIN_6) // FS0
// CS2   PC5
#define OLED_ROM_CS_Clr() gpio_bit_reset(GPIOC, GPIO_PIN_5) // CS2
#define OLED_ROM_CS_Set() gpio_bit_set(GPIOC, GPIO_PIN_5)

#define OLED_CMD          0 // 写命令
#define OLED_DATA         1 // 写数据

void bsp_spi_oled_color_turn(uint8_t i);
void bsp_spi_oled_display_turn(uint8_t i);
void bsp_spi_oled_write_byte(uint8_t dat, uint8_t cmd);
void bsp_spi_oled_clear(void);
void bsp_spi_oled_address(uint8_t x, uint8_t y);
void bsp_spi_oled_display_128x64(uint8_t *dp);
void bsp_spi_oled_display_16x16(uint8_t x, uint8_t y, uint8_t *dp);
void bsp_spi_oled_display_8x16(uint8_t x, uint8_t y, uint8_t *dp);
void bsp_spi_oled_display_5x7(uint8_t x, uint8_t y, uint8_t *dp);
// void Send_Command_to_ROM(uint8_t dat);
// uint8_t Get_data_from_ROM(void);
void bsp_spi_oled_get_data_form_ROM(uint8_t addrHigh, uint8_t addrMid, uint8_t addrLow, uint8_t *pbuff, uint8_t DataLen);
void bsp_spi_oled_display_GB2312_string(uint8_t x, uint8_t y, uint8_t *text);
void bsp_spi_oled_display_string_5x7(uint8_t x, uint8_t y, uint8_t *text);
void bsp_spi_oled_show_number(uint8_t x, uint8_t y, float num, uint8_t len);

void bsp_spi_oled_init(void);
#endif
