#ifndef APP_H
#define APP_H

#include "bsp_keys.h"
#include "bsp_adkey.h"
#include "bsp_188_leds.h"
#include "bsp_digital_leds.h"
#include "bsp_ntc.h"
#include "bsp_iic_oled.h"
#include "bsp_dot_matrix.h"
#include "bsp_flash.h"
#include "bsp_spi_oled.h"
#include "bsp_pcf8563.h"
#include "bsp_touch_key.h"
#include "bsp_keypad.h"

#include "msp_exti.h"

#include "systick.h"

#include "bmp.h"

extern Time_pcf time;
extern Alarm alarm;

//============ input ============
void app_input_init(void);

// 按键任务
void app_input_key_task(void);
//
void app_input_adkey_task(void);
//
void app_input_touch_key_task(void);
//
void app_input_keypad_task(void);

//============ show ============
void app_show_init(void);
//
void app_show_188_leds_task(void);
//
void app_show_pcf8563_task(void);
//
void app_show_iic_oled_task(void);
//
void app_show_digital_leds_task(void);
//
void app_show_dot_matrix_task(void);
//
void app_show_ntc_task(void);

//============  test  ============
void app_test_flash(void);
//
void app_test_spi_oled(void);
//
void app_test_iic_oled(void);
//
void app_test_digital_leds(void);
//
void app_test_pcf8563_set_time_alarm(void);

#endif // APP_H