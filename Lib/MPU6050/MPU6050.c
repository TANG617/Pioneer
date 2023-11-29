#include "MPU6050.h"
#include "stm32f1xx_hal.h"
#include "inv_mpu.h"
#include "debug.h"
#include "math.h"

I2C_HandleTypeDef *i2c=NULL;

void I2C_Select(I2C_HandleTypeDef *hi2c){i2c=hi2c;}

HAL_StatusTypeDef I2C_Read(uint8_t dev,uint8_t mem,uint8_t len,uint8_t* data){
  return HAL_I2C_Mem_Read(i2c,dev,mem,1,data,len,1000);
}

HAL_StatusTypeDef I2C_Write(uint8_t dev,uint8_t mem,uint8_t len,uint8_t* data){
  return HAL_I2C_Mem_Write(i2c,dev,mem,1,data,len,1000);
}

void I2C_Search(uint8_t addr){
  if(i2c==NULL) FS_Debug("I2C Failed!\n");
  if(addr!=0x0u){
    if(HAL_I2C_IsDeviceReady(i2c,addr,1,1000)==HAL_OK)
      FS_Debug("Device Alive\n");
    else FS_Debug("Device Offline\n");
  }else for(uint8_t i=1;i<255;i++)
      if(HAL_I2C_IsDeviceReady(i2c,i,1,1000)==HAL_OK){
        FS_Debug("Device 0x%X Alive\n",i);
        HAL_Delay(1000);
      }
}

MPU6050 mpu;

uint8_t MPU_Init(){
  uint8_t res=mpu_dmp_init(),i=0;
  while(res){
    if(++i>=5) return 0;
    HAL_Delay(200);
    //FS_Debug("MPU Init Failed[%u:%u]\n",i,res);
    res=mpu_dmp_init();
  }
  HAL_Delay(10);
  //FS_Debug("MPU Init Succeed[%u,%u]\n",i,0);
  mpu.accX=mpu.accY=mpu.accZ=0;
  mpu.gyrX=mpu.gyrY=mpu.gyrZ=0;
  mpu.pitch=mpu.roll=mpu.yaw=0;
  return 1;
}

void MPU_ReadAcc(){
  uint8_t buff[6];
  I2C_Read(MPU6050_ADDR,MPU6050_ACC,6,buff);
  mpu.accX=(int16_t)(buff[0]<<8|buff[1])/16384.0f;
  mpu.accY=(int16_t)(buff[2]<<8|buff[3])/16384.0f;
  mpu.accZ=(int16_t)(buff[4]<<8|buff[5])/16384.0f;
}

void MPU_ReadGyr(){
  uint8_t buff[6];
  I2C_Read(MPU6050_ADDR,MPU6050_GYR,6,buff);
  mpu.gyrX=(int16_t)(buff[0]<<8|buff[1])/131.0f;
  mpu.gyrY=(int16_t)(buff[2]<<8|buff[3])/131.0f;
  mpu.gyrZ=(int16_t)(buff[4]<<8|buff[5])/131.0f;
}

uint8_t MPU_ReadDmp(){
  if(mpu_dmp_get_data(&(mpu.pitch),&(mpu.roll),&(mpu.yaw))==0){
    MPU_ReadAcc();
    MPU_ReadGyr();
    return 0;
  }else return 1;
}