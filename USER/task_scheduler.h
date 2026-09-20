#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H
#include "sys.h"
#define TASK_CAPACITY 8U
#define TASK_INVALID_ID 255U
typedef void (*TaskAction)(void);
typedef void (*TaskUpdate)(u32 elapsed_us);
void Tasks_Init(void);
u8 Tasks_Register(TaskAction start, TaskUpdate update, TaskAction stop);
u8 Tasks_Enable(u8 id, u8 enabled);
void Tasks_Run(void);
#endif
