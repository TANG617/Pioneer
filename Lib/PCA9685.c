//
// Created by 李唐 on 2023/11/13.
//

#include "PCA9685.h"

bool PAL_PCA9685_IsSleep(PAL_PCA9685* target){
  if(target->i2c_rx==NULL) return true;
  uint8_t mode;
  HAL_StatusTypeDef status;
  status = target->i2c_rx(target->addr,PCA9685_REG_MODE1,&mode);
  if(status!=HAL_OK) return true;
  mode &= 0x01<<PCA9685_BIT_SLEEP;
  return mode>>PCA9685_BIT_SLEEP;
}

PAL_PCA9685_State PAL_PCA9685_Sleep(PAL_PCA9685* target,bool sleep){
  if(target->i2c_rx==NULL) return PCA9685_ERROR;
  if(target->i2c_tx==NULL) return PCA9685_ERROR;

  uint8_t mode;
  HAL_StatusTypeDef status;
  status = target->i2c_rx(target->addr,PCA9685_REG_MODE1,&mode);
  if(status!=HAL_OK) return (PAL_PCA9685_State)status;

  if(sleep) mode |= 0x01<<PCA9685_BIT_SLEEP;
  else mode &= ~(0x01<<PCA9685_BIT_SLEEP);
  status = target->i2c_tx(target->addr,PCA9685_REG_MODE1,mode);
  if(status==HAL_OK&&sleep) HAL_Delay(500);
  return (PAL_PCA9685_State)status;
}

PAL_PCA9685_State PAL_PCA9685_SetFrequency(PAL_PCA9685* target ,float freq){
  if(PAL_PCA9685_IsSleep(target)) return PCA9685_SLEEP;
  if(target->i2c_tx==NULL) return PCA9685_ERROR;
  if(target->i2c_rx==NULL) return PCA9685_ERROR;

  float preScale = (float)target->osc/(4096.0f*freq)-1;
  if(preScale<3||preScale>255) return PCA9685_ERROR;

  HAL_StatusTypeDef status;
  PAL_PCA9685_State res;

  res = PAL_PCA9685_Sleep(target,true);
  if(res!=PCA9685_ACCEPTED) return res;

  status = target->i2c_tx(target->addr,PCA9685_REG_FREQ,(uint8_t)(preScale+0.5f));
  if(status!=HAL_OK) return (PAL_PCA9685_State)status;

  res = PAL_PCA9685_Sleep(target,false);
  if(res!=PCA9685_ACCEPTED) return res;

  return PCA9685_ACCEPTED;
}

float PAL_PCA9685_GetFrequency(PAL_PCA9685* target){
  uint8_t preScale;
  if(target->i2c_rx==NULL) return 0x0000;
  HAL_StatusTypeDef status = target->i2c_rx(target->addr,PCA9685_REG_FREQ,&preScale);
  if(status!=HAL_OK) return 0x0000;
  return (float)(target->osc)/(4096.0f*((float)preScale+1.0f));
}

PAL_PCA9685_State PAL_PCA9685_SetOutput(PAL_PCA9685* target,uint8_t channel,float ratio,float delay){
  if(ratio<0||ratio>1) return PCA9685_ERROR;
  if(delay<0||delay>1) return PCA9685_ERROR;
  if(target->i2c_tx==NULL) return PCA9685_ERROR;
  if(target->i2c_rx==NULL) return PCA9685_ERROR;

  if(PAL_PCA9685_IsSleep(target)) return PCA9685_SLEEP;

  uint16_t on,off;
  if(ratio==1){on=4096;off=0;}
  else if(ratio==0){on=0;off=4096;}
  else{
    if(ratio+delay<=1.0f){
      on = (uint16_t)(4096.0f*delay+0.5f);
      off = (uint16_t)(4096.0f*(delay+ratio)+0.5f);
    }else{
      on = (uint16_t)(4096.0f*delay+0.5f);
      off = (uint16_t)(4096.0f*(delay+ratio-1.0f)+0.5f);
    }
  }

  HAL_StatusTypeDef status;
  status = target->i2c_tx(target->addr,PCA9685_REG_BASE_ON_L+channel*4,(uint8_t)on);
  if(status!=HAL_OK) return (PAL_PCA9685_State)status;
  status = target->i2c_tx(target->addr,PCA9685_REG_BASE_ON_H+channel*4,(uint8_t)(on>>8));
  if(status!=HAL_OK) return (PAL_PCA9685_State)status;
  status = target->i2c_tx(target->addr,PCA9685_REG_BASE_OFF_L+channel*4,(uint8_t)off);
  if(status!=HAL_OK) return (PAL_PCA9685_State)status;
  status = target->i2c_tx(target->addr,PCA9685_REG_BASE_OFF_H+channel*4,(uint8_t)(off>>8));

  return (PAL_PCA9685_State)status;
}

PAL_PCA9685_State PAL_PCA9685_SetAll(PAL_PCA9685* target,float ratio,float delay){
  if(ratio<0||ratio>1) return PCA9685_ERROR;
  if(delay<0||delay>1) return PCA9685_ERROR;
  if(target->i2c_tx==NULL) return PCA9685_ERROR;
  if(target->i2c_rx==NULL) return PCA9685_ERROR;

  if(PAL_PCA9685_IsSleep(target)) return PCA9685_SLEEP;

  uint16_t on,off;
  if(ratio==1){on=4096;off=0;}
  else if(ratio==0){on=0;off=4096;}
  else{
    if(ratio+delay<=1.0f){
      on = (uint16_t)(4096.0f*delay+0.5f);
      off = (uint16_t)(4096.0f*(delay+ratio)+0.5f);
    }else{
      on = (uint16_t)(4096.0f*delay+0.5f);
      off = (uint16_t)(4096.0f*(delay+ratio-1.0f)+0.5f);
    }
  }

  HAL_StatusTypeDef status;
  status = target->i2c_tx(target->addr,PCA9685_REG_ALL_ON_L,(uint8_t)on);
  if(status!=HAL_OK) return (PAL_PCA9685_State)status;
  status = target->i2c_tx(target->addr,PCA9685_REG_ALL_ON_H,(uint8_t)(on>>8));
  if(status!=HAL_OK) return (PAL_PCA9685_State)status;
  status = target->i2c_tx(target->addr,PCA9685_REG_ALL_OFF_L,(uint8_t)off);
  if(status!=HAL_OK) return (PAL_PCA9685_State)status;
  status = target->i2c_tx(target->addr,PCA9685_REG_ALL_OFF_H,(uint8_t)(off>>8));

  return (PAL_PCA9685_State)status;
}

float PAL_PCA9685_GetRatio(PAL_PCA9685* target,uint8_t channel){
  if(target->i2c_tx==NULL) return 2;
  if(target->i2c_rx==NULL) return 2;

  uint8_t on_L,on_H,off_L,off_H;
  HAL_StatusTypeDef status;
  status = target->i2c_rx(target->addr,PCA9685_REG_BASE_ON_L+channel*4,&on_L);
  if(status!=HAL_OK) return 2;
  status = target->i2c_rx(target->addr,PCA9685_REG_BASE_ON_H+channel*4,&on_H);
  if(status!=HAL_OK) return 2;
  status = target->i2c_rx(target->addr,PCA9685_REG_BASE_OFF_L+channel*4,&off_L);
  if(status!=HAL_OK) return 2;
  status = target->i2c_rx(target->addr,PCA9685_REG_BASE_OFF_H+channel*4,&off_H);
  if(status!=HAL_OK) return 2;

  float on = (float)on_H*256+(float)on_L;
  float off = (float)off_H*256+(float)off_L;
  if(on>=4096) return 1.0f;
  if(off>=4096) return 0.0f;
  if(on<=off) return (off-on)/4096;
  else return 1.0f-(on-off)/4096;
}

float PAL_PCA9685_GetDelay(PAL_PCA9685* target,uint8_t channel){
  if(target->i2c_tx==NULL) return 2;
  if(target->i2c_rx==NULL) return 2;
  uint8_t on_L,on_H;
  HAL_StatusTypeDef status;
  status = target->i2c_rx(target->addr,PCA9685_REG_BASE_ON_L+channel*4,&on_L);
  if(status!=HAL_OK) return 2;
  status = target->i2c_rx(target->addr,PCA9685_REG_BASE_ON_H+channel*4,&on_H);
  if(status!=HAL_OK) return 2;

  float on = (float)on_H*256+(float)on_L;
  return on/4096;
}
