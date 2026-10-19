#include "taskmanager.h"

static TASK tasks_list[] = {
    {app_input_touch_key_task, STOP, 10, 1},
    {app_show_digital_leds_task, STOP, 10, 2},
    {app_show_dot_matrix_task, STOP, 10, 3},

    {app_show_188_leds_task, STOP, 20, 5},
    {app_show_iic_oled_task, STOP, 100, 10},

    {app_show_pcf8563_task, STOP, 1000, 20},
    // {app_show_ntc_task, STOP, 500, 30},
};

static uint8_t tasks_len = sizeof(tasks_list) / sizeof(tasks_list[0]);
;

// 任务状态切换
void task_switch()
{
    for (uint8_t i = 0; i < tasks_len; i++)
    {
        if (tasks_list[i].cnt == 0)
            continue;

        // 倒计时
        tasks_list[i].cnt--;
        if (tasks_list[i].cnt == 0)
        {
            // 计数到0，执行任务
            tasks_list[i].state = START;
        }
    }
}

// 任务调度
void task_schedule()
{
    for (uint8_t i = 0; i < tasks_len; i++)
    {
        if (tasks_list[i].state == START)
        {
            tasks_list[i].func();
            tasks_list[i].cnt = tasks_list[i].period;
            tasks_list[i].state = STOP;
        }
    }
}
