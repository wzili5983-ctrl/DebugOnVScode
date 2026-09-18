#include "led.h"
#include "task_scheduler.h"
#include "app_tasks.h"

int main(void)
{
    u8 breath, siren;
    LED_Init();
    Tasks_Init();
    breath = Tasks_Register(Breath_Start, Breath_Update, Breath_Stop);
    siren = Tasks_Register(Siren_Start, Siren_Update, Siren_Stop);
    Tasks_Enable(breath, 1);
    /* Keep the controller running so it can scan PA0 while muted. */
    Tasks_Enable(siren, 1);
    while (1) Tasks_Run();
}
