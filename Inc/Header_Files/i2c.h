#ifndef I2C_H
#define I2C_H

#include "stm32f4xx.h"
#include <stdint.h>

typedef enum {
    I2C_OK    = 0,
    I2C_ERROR = 1,
    I2C_BUSY  = 2
} I2C_Status;

void       I2C_init(void);
I2C_Status I2C_Write_Register(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
I2C_Status I2C_Read_Register (uint8_t dev_addr, uint8_t reg_addr, uint8_t *data);
I2C_Status I2C_ReadBursts    (uint8_t dev_addr, uint8_t reg_addr, uint8_t *buf, uint8_t len);

#endif
