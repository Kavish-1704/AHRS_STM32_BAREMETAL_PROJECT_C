#include "stm32f4xx.h"
#include <stdint.h>
#include "i2c.h"
#define MPU6050_ADDR 0x68   // The I2C address of the MPU-6050 (if AD0 pin is grounded)
#define WHO_AM_I_REG 0x75   // The register that holds the ID

#include "mpu6050.h"
uint8_t MPU6050_WhoAmI(void) {
    uint8_t my_data = 0;
    
    // Call the burst read function: 
    // Device = 0x68, Target Register = 0x75, Buffer =  Length = 1 byte
    I2C_ReadBursts(MPU6050_ADDR, WHO_AM_I_REG, &my_data, 1);
    
    return my_data;
}

void MPU6050_Init(void){
I2C_Write_Register(0x68,0x6B,0x00);
I2C_Write_Register(0x68,0x19,0x07);
I2C_Write_Register(0x68,0x1B,0x00);
I2C_Write_Register(0x68,0x1C,0x00);
}
void MPU6050_ReadRawData(MPU6050_Data *out){
    uint8_t buffer[14];
   I2C_ReadBursts(0x68,0x3B,buffer,14);
   
  out->Accel_X = (int16_t)((buffer[0] << 8)  | buffer[1]);
    out->Accel_Y = (int16_t)((buffer[2] << 8)  | buffer[3]);
    out->Accel_Z = (int16_t)((buffer[4] << 8)  | buffer[5]);

    out->Temp    = (int16_t)((buffer[6] << 8)  | buffer[7]);

    out->Gyro_X  = (int16_t)((buffer[8] << 8)  | buffer[9]);
    out->Gyro_Y  = (int16_t)((buffer[10] << 8) | buffer[11]);
    out->Gyro_Z  = (int16_t)((buffer[12] << 8) | buffer[13]);


}
