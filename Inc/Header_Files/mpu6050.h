#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

#define MPU6050_ADDR 0x68

typedef struct {
    int16_t Accel_X;
    int16_t Accel_Y;
    int16_t Accel_Z;
    int16_t Gyro_X;
    int16_t Gyro_Y;
    int16_t Gyro_Z;
    int16_t Temp;
} MPU6050_Data;

uint8_t MPU6050_WhoAmI(void);
void    MPU6050_Init(void);
void    MPU6050_ReadRawData(MPU6050_Data *out);

#endif