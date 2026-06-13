#include "stm32f4xx.h"
#include "stdio.h"
static void UART_sendchar(char c){
    while (!(USART2->SR &(1<<7)));

    USART2->DR = c;
    
}

static void UART_sendstring(char *s){
    while(*s != '\0'){
        UART_sendchar(*s);
        s++;
    }
}

void UART_Init(){
RCC->AHB1ENR |=(1<<0);
    RCC->APB1ENR|= (1<<17);
    

    GPIOA->MODER &= ~(3<<(2*2));
    GPIOA->MODER |= (2<<(2*2));

    GPIOA->OTYPER &= ~(1<<2);
GPIOA->OSPEEDR &= ~(3<<(2*2));

    GPIOA->OSPEEDR |=(3<<(2*2));
    GPIOA->AFR[0] &= ~(0xF <<8);
    GPIOA->AFR[0] |= (7<<8);

    USART2->BRR = 138;
    USART2->CR1 |= (1<<3);
    USART2 -> CR1 |= (1<<13);

}

void UART_Print(float roll, float pitch, float yaw, uint32_t count) {
    char buffer[64]; 
    sprintf(buffer, "R:%.1f,P:%.1f,Y:%.1f,N:%lu\r\n", roll, pitch, yaw, count);
    UART_sendstring(buffer);
}

