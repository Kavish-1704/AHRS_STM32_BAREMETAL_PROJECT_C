#include "stm32f4xx.h"
#include "mpu6050.h"
#include "math.h"
volatile float q[4] = {1.0f,0.0f,0.0f,0.0f};
volatile float eInt[3] ={0.0f,0.0f,0.0f};
float Kp = 2.0f;
float Ki = 0.005f;
float dt = 0.001f;
volatile uint32_t isr_count = 0;
void TIM6_Init(void){
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = 15;
    TIM6->ARR = 999;

    TIM6->DIER |= (1<<0);
    NVIC_SetPriority(TIM6_DAC_IRQn,1);
    NVIC_EnableIRQ(TIM6_DAC_IRQn);
    TIM6->CR1 |=(1<<0);

}
float invSqrt(float x) {
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}
void TIM6_DAC_IRQHandler(){
    TIM6->SR &= ~(1<<0);


    MPU6050_Data raw_data;
    MPU6050_ReadRawData(&raw_data);

    float ax = raw_data.Accel_X/16384.0f;
    float ay = raw_data.Accel_Y/16384.0f;
    float az = raw_data.Accel_Z/16384.0f;

    float gx = (raw_data.Gyro_X/131.0f)*0.0174533f;
    float gy = (raw_data.Gyro_Y/131.0f)*0.0174533f;
    float gz = (raw_data.Gyro_Z/131.0f)*0.0174533f;

    float norm = ax*ax +ay*ay+az*az;

    if(norm > 0.25f && norm<2.25f){
        norm = invSqrt(norm);
        ax *= norm;ay *= norm;az *=norm;

        float vx = 2.0f*(q[1]*q[3] - q[0]*q[2]);
        float vy = 2.0f*(q[0]*q[1] +q[2]*q[3]);
        float vz = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];

        float ex = ay*vz - az*vy;
        float ey= az*vx - ax*vz;
        float ez= ax*vy - ay*vx;

        eInt[0] += ex*dt;
        eInt[1] += ey*dt;
        eInt[2] += ez*dt;

        gx = gx +(Kp*ex) +(Ki*eInt[0]);
        gy = gy +(Kp*ey) +(Ki*eInt[1]);
        gz = gz +(Kp*ez) +(Ki*eInt[2]);


    }
    float q0_old = q[0]; float q1_old = q[1]; 
    float q2_old = q[2]; float q3_old = q[3];

    q[0] += (-q1_old*gx - q2_old*gy - q3_old*gz) * (dt * 0.5f);
    q[1] += ( q0_old*gx + q2_old*gz - q3_old*gy) * (dt * 0.5f);
    q[2] += ( q0_old*gy - q1_old*gz + q3_old*gx) * (dt * 0.5f);
    q[3] += ( q0_old*gz + q1_old*gy - q2_old*gx) * (dt * 0.5f);

    norm = 1.0f/sqrtf(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    q[0] *= norm; q[1] *= norm; q[2] *= norm; q[3] *= norm;
    isr_count++;

}
