#ifndef BALANCEVEHICLE_IMU_H
#define BALANCEVEHICLE_IMU_H

#include "stm32f1xx_hal.h"

extern I2C_HandleTypeDef *i2c;
void I2C_Select(I2C_HandleTypeDef *hi2c);
void I2C_Search(uint8_t addr);
HAL_StatusTypeDef I2C_Read(uint8_t dev,uint8_t mem,uint8_t len,uint8_t* data);
HAL_StatusTypeDef I2C_Write(uint8_t dev,uint8_t mem,uint8_t len,uint8_t* data);

#define MPU6050_ADDR 0xD0
#define MPU6050_ACC 0x3Bu
#define MPU6050_GYR 0x41u

typedef struct{
  float accX,accY,accZ;
  float gyrX,gyrY,gyrZ;
  float pitch,roll,yaw;
} MPU6050;
extern MPU6050 mpu;
uint8_t MPU_Init();
void MPU_ReadAcc();
void MPU_ReadGyr();
uint8_t MPU_ReadDmp();

#endif //BALANCEVEHICLE_IMU_H