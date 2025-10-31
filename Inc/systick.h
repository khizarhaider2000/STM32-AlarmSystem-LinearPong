#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "stm32l5xx.h"

// --------------------------------------------------------
// Type definitions
// --------------------------------------------------------
typedef unsigned int Time_t;
#define TIME_MAX ((Time_t)-1)

// --------------------------------------------------------
// Function prototypes
// --------------------------------------------------------

// Initializes the SysTick timer (1 ms tick)
void StartSysTick(void);

// Waits for the next SysTick interrupt (used for timing control)
void WaitForSysTick(void);

// Blocks for t milliseconds (busy wait)
void msDelay(int t);

// Returns the current system time in milliseconds
Time_t TimeNow(void);

// Returns time elapsed since a given timestamp
Time_t TimePassed(Time_t since);

#endif /* SYSTICK_H_ */
