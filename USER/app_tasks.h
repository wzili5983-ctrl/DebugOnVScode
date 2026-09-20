#ifndef APP_TASKS_H
#define APP_TASKS_H
#include "sys.h"
void Breath_Start(void);
void Breath_Update(u32 elapsed_us);
void Breath_Stop(void);
void Siren_Start(void);
void Siren_Update(u32 elapsed_us);
void Siren_Stop(void);
#endif
