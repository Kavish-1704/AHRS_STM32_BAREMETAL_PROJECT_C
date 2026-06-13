#ifndef UART_H
#define UART_H

#include "stm32f4xx.h"
#include <stdint.h>

// Layer 0: Setup
void UART_Init(void);

// Layer 2: The formatted output
void UART_Print(float roll, float pitch, float yaw, uint32_t count);

#endif // UART_H