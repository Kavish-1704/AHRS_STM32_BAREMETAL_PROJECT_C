#include "stm32f4xx.h"
#include "i2c.h"
#include "uart.h"
#include "mpu6050.h"
#include "TIM6.h"
#include "math.h"
extern volatile uint32_t isr_count;
extern volatile float q[4];


void Delay_ms(uint32_t ms) {

    SysTick->LOAD = 16000 - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = 5;
    for (uint32_t i = 0; i < ms; i++) {
        while ((SysTick->CTRL & (1 << 16)) == 0) {
        }
    }
    SysTick->CTRL = 0;
}
int main(void){
	  char boot_msg[] = "BOOT OK\n";
		  for(int i = 0; boot_msg[i]!='\0';i++){
		    while(!(USART2->SR & (1<<7)) );
		    USART2->DR = boot_msg[i];
		  }


  I2C_init();
  MPU6050_Init();
  UART_Init();
  TIM6_Init();


  while(1){

float local_q[4];
float roll, pitch, yaw;

  __disable_irq();
local_q[0] = q[0];
    local_q[1] = q[1];
    local_q[2] = q[2];
    local_q[3] = q[3];
  __enable_irq();

float sinp = 2.0f * (local_q[0]*local_q[2] - local_q[3]*local_q[1]);

    if (sinp > 1.0f)  sinp = 1.0f;
    if (sinp < -1.0f) sinp = -1.0f;
    pitch = asinf(sinp) * 57.29578f;

    roll = atan2f(2.0f * (local_q[0]*local_q[1] + local_q[2]*local_q[3]),
                  1.0f - 2.0f * (local_q[1]*local_q[1] + local_q[2]*local_q[2])) * 57.29578f;

    yaw = atan2f(2.0f * (local_q[0]*local_q[3] + local_q[1]*local_q[2]),
                 1.0f - 2.0f * (local_q[2]*local_q[2] + local_q[3]*local_q[3])) * 57.29578f;

                 UART_Print(roll, pitch, yaw, isr_count);

    Delay_ms(100);
}
}

