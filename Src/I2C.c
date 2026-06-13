#include "stm32f4xx.h"
#include <stdio.h>

typedef enum {
    I2C_OK      = 0,  // 0 means everything worked
    I2C_ERROR   = 1,  // 1 means something broke
    I2C_BUSY    = 2   // 2 means the hardware is currently in use
} I2C_Status;

void I2C_init(){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;// enabling the rcc clock for gpiob
    GPIOB->MODER &= ~(((3<<(6*2))|(3<<(7*2))));// first clearing the bits the moder 
    GPIOB->MODER |=(((2<<(6*2))|(2<<(7*2))));// setting both tht pb6 adn pb7 to 10 i.e alternate function 
    GPIOB->OTYPER |=(1<<6) | (1<<7);// setting both 6 and 7 to open drain that is required for i2c 
    GPIOB->OSPEEDR |=(2<<(6*2)) | (2<<(7*2));// setting the speed to high on both 
    GPIOB->PUPDR &= ~((3<<(6*2)) | (3<<(7*2)));// no internal pull or push resistors 
    GPIOB->AFR[0] |= (4<<(6*4)) | (4<<(7*4));// creating the alternate funcitons for both pb6 -> scl and pb7 -> sdl
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;// enabling he i2c1 clock 
    I2C1->CR2 = 45;//setting the peripheral clock freq
    I2C1->CCR =(1<<14) | 37;// fast mode with 400khz speed 
    I2C1->TRISE = 14;// setting the maximum rise time 
    I2C1->CR1 |= 1;//enabling the i2c peripherals 
}
static I2C_Status I2C_GenerateStart(void){
    I2C1->CR1 |= (1<<8);// generating the start condition for i2c 
    while(!(I2C1->SR1 & (1<<0)));// continously checking for the status register so that the start bit is enabled 
    return I2C_OK;

}
static I2C_Status I2C_ClearADRR(){
    volatile uint8_t dummy = I2C1->SR1;
    dummy = I2C1->SR2;
    (void)dummy;// important function to clear the address flag after sending  the slave address, it is done in i2c by first reading the sr1 into a variable and then reading sr2 to the same variable 
    return I2C_OK;
}

static I2C_Status I2C_SendAddress(uint8_t addr, uint8_t direction){

    uint8_t final_addr = (addr<<1)|direction;// the final address of a slave contains the address and the least significant bit contains the direction of propogation of the data i.e read by 1 and write by 0
    I2C1->DR = final_addr;//writing the final address to the data regiser 
   
    uint32_t timeout =10000;
    while(!(I2C1->SR1 &(1<<1))){
        if (--timeout == 0) return I2C_ERROR;// checking for the acknowledgement bit so that if a slave dosent respond we can give an error 
    };
     I2C_ClearADRR();//clearning the address
    return I2C_OK;



}

static I2C_Status I2C_SendByte(uint8_t data){
    uint32_t timeout =10000;
    while(!(I2C1->SR1 &(1<<7))){// asking if the data register is empty or not 
        if (--timeout == 0) return I2C_ERROR;
    };;

    I2C1->DR = data;// writing the data to the data register 

    return I2C_OK;

}

static I2C_Status I2C_GenerateRestart(void){//giving the start condition again without releasing the bus 
    I2C1->CR1 |= (1<<8);
    uint32_t timeout =10000;
    while(!(I2C1->SR1 & (1<<0))){
        if (--timeout == 0) return I2C_ERROR;
    };;
    return I2C_OK;
}
static I2C_Status I2C_ReceiveByte(uint8_t *data){
    I2C1->CR1 &= ~(1<<10);//disabling the ack bit 
    I2C1->CR1 |= (1<<9);//generate stop condition 
    I2C_ClearADRR();//clearing the address
    uint32_t timeout =10000;
    while(!(I2C1->SR1 & (1<<6))){//waiting for rxne to be 1 i.e data reached 
        if (--timeout == 0) return I2C_ERROR;
    }
    *data = I2C1->DR;//writing the data to the data register 
    return I2C_OK;
}

static I2C_Status I2C_GenerateStop(void){
    I2C1->CR1 |= (1<<9);//generate stop contition 
    uint32_t timeout =10000;
    while(I2C1->SR2 &(1<<1)){//checks if the bus is ideal 
        if (--timeout == 0) return I2C_ERROR;
    }
    return I2C_OK;
}

I2C_Status I2C_Write_Register(uint8_t dev_add,uint8_t reg_add,uint8_t data){

    I2C_GenerateStart();
    I2C_SendAddress( dev_add, 0);
    I2C_SendByte( reg_add);
    I2C_SendByte( data);
    I2C_GenerateStop();
     return I2C_OK;
}

I2C_Status I2C_Read_Register(uint8_t dev_add,uint8_t reg_add,uint8_t *data){
    I2C_GenerateStart();
    I2C_SendAddress( dev_add,0);
    I2C_SendByte(  reg_add);
    I2C_GenerateRestart();
    I2C_SendAddress( dev_add , 1);
    I2C_ReceiveByte(data);I2C_GenerateStop();
     return I2C_OK;
}

I2C_Status I2C_ReadBursts(uint8_t dev_add,uint8_t reg_add,uint8_t *buf , uint8_t len){
        I2C_GenerateStart();
    I2C_SendAddress( dev_add,0);
    I2C_SendByte(  reg_add);
    I2C_GenerateRestart();
    I2C_SendAddress( dev_add , 1);
    while(len>0){
        if(len ==1){
            I2C1->CR1 &= ~(1<<10);
            I2C1->CR1 |= (1<<9);
        }
        while(!(I2C1->SR1 & (1<<6)));
        *buf = I2C1->DR;
        buf++;
        len--;



    }
     return I2C_OK;

}
