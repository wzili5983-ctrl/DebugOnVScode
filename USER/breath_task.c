#include "app_tasks.h"

#define KEY_DEBOUNCE_US 20000UL

typedef struct {
    u16 pin;
    u8 candidate, stable;
    u32 elapsed;
} BreathKey;
static BreathKey keys[3];
static u8 running;
/* Software PWM and brightness phase, both expressed in microseconds. */
static u32 pwm_phase;
static u32 breath_phase;

#define PWM_PERIOD_US       2000UL   /* 500 Hz, above visible flicker */
#define BREATH_PERIOD_US 4000000UL   /* one complete fade in/out cycle */

/* Return one event per debounced press, including a debounced release. */
static u8 Key_Pressed(BreathKey *key, u32 elapsed_us)
{
    u8 pressed = GPIO_ReadInputDataBit(GPIOE, key->pin) == Bit_RESET;
    if (pressed != key->candidate) {
        key->candidate = pressed;
        key->elapsed = 0;
    } else if (key->elapsed < KEY_DEBOUNCE_US) {
        if (elapsed_us >= KEY_DEBOUNCE_US - key->elapsed)
            key->elapsed = KEY_DEBOUNCE_US;
        else
            key->elapsed += elapsed_us;
        if (key->elapsed == KEY_DEBOUNCE_US && key->stable != pressed) {
            key->stable = pressed;
            return pressed;
        }
    }
    return 0;
}

void Breath_Stop(void)
{
    running = 0;
    GPIO_SetBits(GPIOF, GPIO_Pin_9);
    pwm_phase = 0;
    breath_phase = 0;
}

void Breath_Start(void)
{
    GPIO_InitTypeDef gpio;
    u8 i;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &gpio);
    keys[0].pin = GPIO_Pin_4;
    keys[1].pin = GPIO_Pin_2;
    keys[2].pin = GPIO_Pin_3;
    for (i = 0; i < 3; i++) {
        keys[i].candidate = GPIO_ReadInputDataBit(GPIOE, keys[i].pin) == Bit_RESET;
        keys[i].stable = keys[i].candidate;
        keys[i].elapsed = 0;
    }
    running = 1;
    pwm_phase = 0;
    breath_phase = 0;
    GPIO_SetBits(GPIOF, GPIO_Pin_9);
}

void Breath_Update(u32 elapsed_us)
{
    u8 faster = Key_Pressed(&keys[0], elapsed_us);
    u8 slower = Key_Pressed(&keys[1], elapsed_us);
    u8 stop = Key_Pressed(&keys[2], elapsed_us);

    /* Stop has priority; opposite speed keys pressed together cancel out. */
    if (stop || keys[2].stable) {
        Breath_Stop();
    } else if (faster != slower) {
        running = 1;
    }
    if (!running) return;

    /* Advance a non-blocking software PWM and a triangular brightness ramp. */
    pwm_phase = (pwm_phase + elapsed_us) % PWM_PERIOD_US;
    breath_phase = (breath_phase + elapsed_us) % BREATH_PERIOD_US;
    {
        u32 fade = (breath_phase < BREATH_PERIOD_US / 2U)
                 ? breath_phase
                 : BREATH_PERIOD_US - breath_phase;
        u32 duty = (fade * PWM_PERIOD_US) / (BREATH_PERIOD_US / 2U);
        /* PF9 LED is active-low: pull low during the on portion of PWM. */
        if (pwm_phase < duty)
            GPIO_ResetBits(GPIOF, GPIO_Pin_9);
        else
            GPIO_SetBits(GPIOF, GPIO_Pin_9);
    }
}
