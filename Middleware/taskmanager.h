#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "app.h"

typedef void (*task_callback)(void);

// 任务状态
typedef enum {
    STOP, // 停止
    START // 执行
} TASK_STATE;

// Task结构体
typedef struct {
    task_callback func; // 任务函数
    TASK_STATE state;   // 任务状态
    uint64_t period;    // 任务执行周期  10
    uint64_t cnt;       // 任务执行时间计数器
} TASK;

// 任务计数更新切换状态
void task_switch();
// 任务调度
void task_schedule();

#endif // TASKMANAGER_H