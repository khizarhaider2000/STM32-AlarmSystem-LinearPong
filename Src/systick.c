// Manage the system timer

#include "systick.h"

#define SYSTICKS 4000  // 1ms with 4MHz clock

static volatile Time_t sysTime = 0;

void StartSysTick() {
    sysTime = 0;
    SysTick->LOAD = (uint32_t)(SYSTICKS - 1);  // Set reload register value
    SCB->SHPR[12+SysTick_IRQn] = 7 << 5;       // Set interrupt priority
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}

// Interrupt handler
void SysTick_Handler(void) {
    sysTime++;
}

// Wait for system time to change
void WaitForSysTick(void) {
    int wasTime = sysTime;
    while (sysTime == wasTime)
        __asm volatile ("wfi");  // keep CPU asleep until next interrupt
}

// Delay measured in milliseconds
void msDelay(int t) {
    for (int i = 0; i < t; i++)
        WaitForSysTick();
}

// Obtain the current system time
Time_t TimeNow(void) {
    return sysTime;
}

// Calculate elapsed time since a previous event
Time_t TimePassed(Time_t since) {
    Time_t now = sysTime;
    if (now >= since)
        return now - since;
    else  // handle rollover
        return now + 1 + TIME_MAX - since;
}
