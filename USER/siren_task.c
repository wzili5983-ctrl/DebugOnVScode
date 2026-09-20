#include "app_tasks.h"

#define KEY_DEBOUNCE_US 20000UL
static u32 elapsed, key_elapsed;
static u8 sounding, key_candidate, key_stable;

void Siren_Stop(void)
{
    sounding = 0;
    GPIO_ResetBits(GPIOF, GPIO_Pin_8);
}

void Siren_Start(void)
{
    GPIO_InitTypeDef gpio;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_0;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &gpio);
    elapsed = 0;
    key_elapsed = 0;
    /* A key held at startup must be released before it can trigger. */
    key_candidate = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
    key_stable = key_candidate;
    Siren_Stop();
}

void Siren_Update(u32 elapsed_us)
{
    u32 phase, period;
    u8 pressed = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
    if (pressed != key_candidate) {
        key_candidate = pressed;
        key_elapsed = 0;
    } else if (key_elapsed < KEY_DEBOUNCE_US) {
        if (elapsed_us >= KEY_DEBOUNCE_US - key_elapsed)
            key_elapsed = KEY_DEBOUNCE_US;
        else
            key_elapsed += elapsed_us;
        if (key_elapsed == KEY_DEBOUNCE_US && key_stable != key_candidate) {
            key_stable = key_candidate;
            if (key_stable) {
                sounding = !sounding;
                elapsed = 0;
            }
        }
    }

    if (!sounding) {
        GPIO_ResetBits(GPIOF, GPIO_Pin_8);
        return;
    }
    elapsed = (elapsed + elapsed_us % 500000UL) % 500000UL;
    period = elapsed >= 250000UL ? 834UL : 1428UL;
    phase = elapsed % 250000UL;
    if (phase % period < period / 2UL) GPIO_SetBits(GPIOF, GPIO_Pin_8);
    else GPIO_ResetBits(GPIOF, GPIO_Pin_8);
}
