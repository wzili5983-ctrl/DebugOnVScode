#include "task_scheduler.h"
typedef struct {
    TaskAction start, stop;
    TaskUpdate update;
    u32 previous;
    u8 enabled;
} Task;
static Task tasks[TASK_CAPACITY];
static u8 count;

void Tasks_Init(void)
{
    RCC_ClocksTypeDef clocks;
    u32 timer_hz;
    count = 0;
    RCC_GetClocksFreq(&clocks);
    timer_hz = clocks.PCLK1_Frequency;
    if (clocks.PCLK1_Frequency != clocks.HCLK_Frequency)
        timer_hz *= 2U;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM2->CR1 = 0;
    TIM2->PSC = timer_hz / 1000000UL - 1U;
    TIM2->ARR = 0xFFFFFFFFUL;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR = 0;
    TIM2->CNT = 0;
    TIM2->CR1 = TIM_CR1_CEN;
}

u8 Tasks_Register(TaskAction start, TaskUpdate update, TaskAction stop)
{
    u8 id;
    if (!update || count >= TASK_CAPACITY) return TASK_INVALID_ID;
    id = count++;
    tasks[id].start = start;
    tasks[id].update = update;
    tasks[id].stop = stop;
    tasks[id].enabled = 0;
    return id;
}

u8 Tasks_Enable(u8 id, u8 enabled)
{
    Task *task;
    if (id >= count) return 0;
    task = &tasks[id];
    enabled = enabled != 0;
    if (task->enabled == enabled) return 1;
    task->enabled = enabled;
    if (enabled) {
        if (task->start) task->start();
        task->previous = TIM2->CNT;
    } else if (task->stop) task->stop();
    return 1;
}

void Tasks_Run(void)
{
    u8 i;
    u32 now, elapsed;
    for (i = 0; i < count; i++) {
        if (!tasks[i].enabled) continue;
        now = TIM2->CNT;
        elapsed = now - tasks[i].previous;
        tasks[i].previous = now;
        tasks[i].update(elapsed);
    }
}
