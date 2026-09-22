#include "bsp_keypad.h"

void bsp_keypad_init(void)
{
    rcu_periph_clock_enable(KP_RCU);

    gpio_mode_set(KP_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, KP_ALL_PINS);
    gpio_output_options_set(KP_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, KP_ALL_PINS);

    /* 默认全部拉高（高阻 + 上拉） */
    gpio_bit_set(KP_PORT, KP_ALL_PINS);
}

unsigned char KeypadScanHex(void)
{
    unsigned char scaned_key_col;
    unsigned char scaned_key_row;
    unsigned char scaned_key_hex = 0xFF;

    /* ---- 列检测：行全低，列全高阻(上拉) ---- */
    KROW1_L();
    KROW2_L();
    KROW3_L();
    KROW4_L();
    KCOL1_H();
    KCOL2_H();
    KCOL3_H();
    KCOL4_H();

    KeypadDelayUs(2);

    scaned_key_col = ((unsigned char)KCOL1_READ() << 0) | ((unsigned char)KCOL2_READ() << 1) | ((unsigned char)KCOL3_READ() << 2) | ((unsigned char)KCOL4_READ() << 3);

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

        KeypadDelayUs(2);

        scaned_key_row = ((unsigned char)KROW1_READ() << 0) | ((unsigned char)KROW2_READ() << 1) | ((unsigned char)KROW3_READ() << 2) | ((unsigned char)KROW4_READ() << 3);

        scaned_key_hex = (scaned_key_col << 4) | scaned_key_row;
    }

    return scaned_key_hex;
}

unsigned char KeypadHexCodeRemap(unsigned char hex_code)
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