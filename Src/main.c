#include "stm32f4xx.h"

// A simple delay function
void Delay(volatile int count) {
    while(count--) {}
}

int main(void) {
    // ==========================================
    // 1. POWER UP I2C & LED
    // ==========================================
    RCC->AHB1ENR |= (1<<0) | (1<<1); // Power GPIOA (LED) and GPIOB (I2C Pins)
    RCC->APB1ENR |= (1<<21);         // Power I2C1

    // ==========================================
    // 2. CONFIGURE GREEN LED (PA5)
    // ==========================================
    GPIOA->MODER &= ~(3<<(5*2));
    GPIOA->MODER |=  (1<<(5*2)); // Set as Output

    // Start with LED OFF
    GPIOA->ODR &= ~(1<<5);

    // ==========================================
    // 3. CONFIGURE I2C PINS (PB6, PB7)
    // ==========================================
    GPIOB->MODER &= ~((3<<(6*2)) | (3<<(7*2)));
    GPIOB->MODER |=  ((2<<(6*2)) | (2<<(7*2)));
    GPIOB->OTYPER |= ((1<<6) | (1<<7)); // Open Drain
    GPIOB->OSPEEDR |= ((2<<(6*2)) | (2<<(7*2)));
    GPIOB->PUPDR &= ~((3<<(6*2)) | (3<<(7*2)));
    GPIOB->PUPDR |=  ((1<<(6*2)) | (1<<(7*2))); // Pull-ups
    GPIOB->AFR[0] &= ~((0xF<<(6*4)) | (0xF<<(7*4)));
    GPIOB->AFR[0] |=  ((4<<(6*4)) | (4<<(7*4))); // AF4 for I2C1

    // ==========================================
    // 4. INITIALIZE I2C HARDWARE
    // ==========================================
    I2C1->CR1 |= (1<<15);  // Reset I2C
    I2C1->CR1 &= ~(1<<15); // Release Reset
    I2C1->CR2 = 16;        // 16 MHz Clock
    I2C1->CCR = 80;        // 100kHz Speed
    I2C1->TRISE = 17;
    I2C1->CR1 |= (1<<0);   // Turn I2C On

    // ==========================================
    // 5. THE HARDWARE PING LOOP
    // ==========================================
    while(1) {
        Delay(1000000); // Wait a moment between attempts

        // Turn LED OFF at the start of every loop
        GPIOA->ODR &= ~(1<<5);

        // --- STEP A: Start Condition ---
        I2C1->CR1 |= (1<<8);
        int timeout = 50000;
        while(!(I2C1->SR1 & (1<<0))) { if (--timeout == 0) break; }
        if (timeout == 0) continue; // Try again if frozen

        // --- STEP B: Send Write Address (0xD0) ---
        I2C1->DR = 0xD0;
        timeout = 50000;
        while(!(I2C1->SR1 & (1<<1))) { if (--timeout == 0) break; }
        if (timeout == 0) continue;
        volatile int dummy = I2C1->SR1; dummy = I2C1->SR2; // Clear ADDR

        // --- STEP C: Send Target Register (0x75 WHO_AM_I) ---
        I2C1->DR = 0x75;
        timeout = 50000;
        while(!(I2C1->SR1 & (1<<7))) { if (--timeout == 0) break; }
        if (timeout == 0) continue;

        // --- STEP D: Generate Re-Start ---
        I2C1->CR1 |= (1<<8);
        timeout = 50000;
        while(!(I2C1->SR1 & (1<<0))) { if (--timeout == 0) break; }
        if (timeout == 0) continue;

        // --- STEP E: Send Read Address (0xD1) ---
        I2C1->DR = 0xD1;
        timeout = 50000;
        while(!(I2C1->SR1 & (1<<1))) { if (--timeout == 0) break; }
        if (timeout == 0) continue;

        // --- STEP F: Read 1 Byte and STOP ---
        I2C1->CR1 &= ~(1<<10); // Disable ACK
        dummy = I2C1->SR1; dummy = I2C1->SR2; // Clear ADDR
        I2C1->CR1 |= (1<<9); // Generate STOP

        timeout = 50000;
        while(!(I2C1->SR1 & (1<<6))) { if (--timeout == 0) break; }
        if (timeout == 0) continue;

        // Extract the byte!
        uint8_t sensor_data = I2C1->DR;

        // ==========================================
        // 6. THE VERDICT
        // ==========================================
        if (sensor_data == 0x68) {
            // SUCCESS! Turn the Green LED Solid ON!
            GPIOA->ODR |= (1<<5);
            Delay(2000000); // Hold it on so you can clearly see it
        }
    }
}
