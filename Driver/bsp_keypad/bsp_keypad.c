#include "bsp_keypad.h"

/* 扫描相位后的稳定延时（us）
   行/列释放后要靠内部上拉（典型几十 kΩ）把线拉高，
   与排线+键盘的寄生电容（约 50~150pF）形成数 us 的上升时间，
   2us 往往还没越过输入门限，会把"未升上去的线"误判成按下。
   若后续在硬件上加了 4.7k~10k 外部上拉，可适当减小该值。 */
#define KP_SETTLE_US 20

void bsp_keypad_init(void)
{
    rcu_periph_clock_enable(KP_RCU);

    /* 准双向口：输出 + 开漏 + 内部上拉
       写 0 -> 输出级把引脚真正拉低
       写 1 -> 输出级关断（高阻），由内部上拉拉高
       注意：必须是 GPIO_MODE_OUTPUT，输入模式下输出级关闭，
             引脚只是悬空，写 0 拉不低、写 1 也拉不高。 */
    gpio_mode_set(KP_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, KP_ALL_PINS);
    gpio_output_options_set(KP_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, KP_ALL_PINS);

    /* 默认全部释放（外部/内部上拉拉高） */
    gpio_bit_set(KP_PORT, KP_ALL_PINS);
}

uint8_t KeypadScanHex(void)
{
    uint8_t scaned_key_col;
    uint8_t scaned_key_row;
    uint8_t scaned_key_hex = 0xFF;

    /* ---- 列检测：行全低，列全高阻(上拉) ---- */
    KROW1_L();
    KROW2_L();
    KROW3_L();
    KROW4_L();
    KCOL1_H();
    KCOL2_H();
    KCOL3_H();
    KCOL4_H();

    delay_1us(KP_SETTLE_US);

    scaned_key_col = ((uint8_t)KCOL1_READ() << 0) | ((uint8_t)KCOL2_READ() << 1) | ((uint8_t)KCOL3_READ() << 2) | ((unsigned char)KCOL4_READ() << 3);

    if (scaned_key_col != 0x0F)
    {
        /* ---- 行检测：列全低，行全高阻(上拉) ---- */
        KCOL1_L();
        KCOL2_L();
        KCOL3_L();
        KCOL4_L();
        KROW1_H();
        KROW2_H();
        KROW3_H();
        KROW4_H();

        delay_1us(KP_SETTLE_US);

        scaned_key_row = ((uint8_t)KROW1_READ() << 0) | ((uint8_t)KROW2_READ() << 1) | ((uint8_t)KROW3_READ() << 2) | ((unsigned char)KROW4_READ() << 3);

        scaned_key_hex = (scaned_key_col << 4) | scaned_key_row;
    }

    return scaned_key_hex;
}

uint8_t KeypadHexCodeRemap(uint8_t hex_code)
{
    switch (hex_code)
    {
    case 0xEE:
        return 1;
    case 0xDE:
        return 2;
    case 0xBE:
        return 3;
    case 0x7E:
        return 4;

    case 0xED:
        return 5;
    case 0xDD:
        return 6;
    case 0xBD:
        return 7;
    case 0x7D:
        return 8;

    case 0xEB:
        return 9;
    case 0xDB:
        return 10;
    case 0xBB:
        return 11;
    case 0x7B:
        return 12;

    case 0xE7:
        return 13;
    case 0xD7:
        return 14;
    case 0xB7:
        return 15;
    case 0x77:
        return 16;

    default:
        return 0xFF;
    }
}

unsigned char KeypadScan(void)
{
    uint8_t hex = KeypadScanHex();
    return KeypadHexCodeRemap(hex);
}