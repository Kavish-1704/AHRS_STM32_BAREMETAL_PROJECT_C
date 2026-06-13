#ifndef TIM6_H
#define TIM6_H

#include "stm32f4xx.h"
#include <stdint.h>

// ==========================================
// Initialization
// ==========================================
void TIM6_Init(void);

// ==========================================
// Global Variables (Read-Only in main.c)
// ==========================================
// Using 'extern' tells the compiler these variables are 
// physically created in tim6.c, but main.c is allowed to look at them.

extern volatile float q[4];
extern volatile uint32_t isr_count; 

#endif // TIM6_H