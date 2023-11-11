#include "pal_mpu6050.h"
#include "pal_mpu6050_dmp.h"
#include "pal_utils.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"

#ifdef PAL_FUNC_MPU6050

#define CHIP_RESET_DELAY        100
#define FIFO_RESET_DELAY        10
#define FIFO_MAX_PACKET_SIZE    12
#define DMP_FIRMWARE_CHUNK_SIZE 16
#define SENSOR_INIT_DELAY       50
#define BYPASS_SETTING_DELAY    3
#define SELF_TEST_TRIES         2

static uint32_t test_gyro_sens    = 32768/250;
static uint32_t test_accel_sens   = 32768/16;
static uint8_t test_reg_rate_div  = 0;
static uint8_t test_reg_lpf       = 1;
static uint8_t test_reg_gyro_fsr  = 0;
static uint8_t test_reg_accel_fsr = 0x18;
static uint16_t test_wait_ms      = 50;
static uint8_t packet_thresh      = 5;
static float min_dps              = 10.0f;
static float max_dps              = 105.0f;
static float max_gyro_var         = 0.14f;
static float min_gravity          = 0.3f;
static float max_gravity          = 0.95f;
static float max_accel_var        = 0.14f;

const int8_t MPU6050_ORIENTATION_DEFAULT[9] = {1,0,0,
                                               0,1,0,
                                               0,0,1};

void (*PAL_MPU6050_Log)(const char* message) = NULL;

#define LOG PAL_MPU6050_Log

/*============================Self Definable============================*/

__weak PAL_MPU6050_State PAL_MPU6050_Init(PAL_MPU6050* target, PAL_MPU6050_DMP* dmp){
  if(target->addr!=0x68&&target->addr!=0x69) target->addr = MPU6050_I2C_ADDR;
  target->addr<<=1;
  if(dmp!=NULL) target->dmp = dmp;
  else target->dmp = (PAL_MPU6050_DMP*)malloc(sizeof(PAL_MPU6050_DMP)*1);

  uint8_t data[6],rev;
  //reset device
  data[0] = MPU6050_BIT_RESET;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_PWR_MGMT1,data,1)!=HAL_OK) {
    if(LOG!=NULL) LOG("MPU_INIT: mpu reset fail\n");
    return MPU6050_TRANS_FAIL;
  }
  PAL_Delay(CHIP_RESET_DELAY*1000);
  //wakeup device
  data[0] = 0x00;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_PWR_MGMT1,data,1)!=HAL_OK) {
    if(LOG!=NULL) LOG("MPU_INIT: mpu wakeup fail\n");
    return MPU6050_TRANS_FAIL;
  }
  //check product revision
  if(PAL_MPU6050_Read(target->addr,MPU6050_REG_ACCEL_OFFS,data,6)!=HAL_OK) {
    if(LOG!=NULL) LOG("MPU_INIT: read accel_offset fail\n");
    return MPU6050_TRANS_FAIL;
  }
  rev = ((data[5]&0x01)<<2)|((data[3]&0x01)<<1)|(data[1]&0x01);
  if(rev!=0){
    if(rev==1) target->accel_half = true;
    else if(rev==2) target->accel_half = false;
    else {
      if(LOG!=NULL) LOG("MPU_INIT: unsupported product\n");
      return MPU6050_INCORRECT;
    }
  }else{
    if(PAL_MPU6050_Read(target->addr,MPU6050_REG_PRODUCT_ID,data,1)!=HAL_OK) {
      if(LOG!=NULL) LOG("MPU_INIT: read prod_id fail\n");
      return MPU6050_TRANS_FAIL;
    }
    rev = data[0]&0x0F;
    if(rev==0) {
      if(LOG!=NULL) LOG("MPU_INIT: incompatible device\n");
      return MPU6050_INCORRECT;
    }else if(rev==4) target->accel_half = true;
    else target->accel_half = false;
  }
  //pre-load data
  target->sensors = 0xFF;
  target->gyro_fsr = 0xFFFF;
  target->accel_fsr = 0xFF;
  target->lpf = 0xFFFF;
  target->sample_rate = 0xFFFF;
  target->fifo_sensors = 0xFF;
  target->bypass_en = 0xFF;
  target->clock_source = MPU6050_CLK_PLL;
  target->active_low_int = true;
  target->int_latched = false;
  target->lp_accel_mode = false;
  target->dmp_en = false;
  target->dmp_loaded = false;
  target->dmp_sample_rate = 0x0000;

  PAL_MPU6050_State status;
  //gyroscope fsr = 2000dps
  status = PAL_MPU6050_SetGyroFSR(target,2000);
  if(status!=MPU6050_ACCEPTED) {if(LOG!=NULL) LOG("MPU_INIT: set gyroscope fsr error\n");return status;}
  //accelerator fsr = 2g
  status = PAL_MPU6050_SetAccelFSR(target,2);
  if(status!=MPU6050_ACCEPTED) {if(LOG!=NULL) LOG("MPU_INIT: set accelerator fsr error\n");return status;}
  //low-pass filter = 42Hz
  status = PAL_MPU6050_SetLPF(target,42);
  if(status!=MPU6050_ACCEPTED) {if(LOG!=NULL) LOG("MPU_INIT: set low-pass filter error\n");return status;}
  //chip sample rate = 50Hz
  status = PAL_MPU6050_SetSampleRate(target,50);
  if(status!=MPU6050_ACCEPTED) {if(LOG!=NULL) LOG("MPU_INIT: set sample rate error\n");return status;}
  //Disable all FIFO
  status = PAL_MPU6050_SetConfigFIFO(target,0x00);
  if(status!=MPU6050_ACCEPTED) {if(LOG!=NULL) LOG("MPU_INIT: config fifo error\n");return status;}
  //Disable bypass mode
  status = PAL_MPU6050_SetBypassMode(target,false);
  if(status!=MPU6050_ACCEPTED) {if(LOG!=NULL) LOG("MPU_INIT: set bypass mode error\n");return status;}
  //Launch XYZ accelerator and XYZ gyroscope
  status = PAL_MPU6050_SetSensors(target,0x00);//or (MPU6050_SENS_ACCEL|MPU6050_SENS_GYRO_ALL)
  if(status!=MPU6050_ACCEPTED) {if(LOG!=NULL) LOG("MPU_INIT: set sensors error\n");return status;}
  if(LOG!=NULL) LOG("MPU_INIT: passed\n");
  return MPU6050_ACCEPTED;
}

__weak PAL_MPU6050_State PAL_MPU6050_Calibrate(PAL_MPU6050* target,bool load_gyro,bool load_accel){
  uint8_t result;
  int32_t gyro[3],accel[3];
  PAL_MPU6050_State status = PAL_MPU6050_SelfTest(target,gyro,accel,&result);
  if(status!=MPU6050_ACCEPTED) return status;
  if(result!=0x03) return MPU6050_INCORRECT;


  float gyro_sens=0;
  if(load_gyro){
    status = PAL_MPU6050_CalcGyroSens(target,&gyro_sens);
    if(status!=MPU6050_ACCEPTED) return status;
  }
  gyro[0] = (int32_t)((float)gyro[0]*gyro_sens);
  gyro[1] = (int32_t)((float)gyro[1]*gyro_sens);
  gyro[2] = (int32_t)((float)gyro[2]*gyro_sens);
  status = PAL_MPU6050_DMP_SetGyroBias(target,gyro);
  if(status!=MPU6050_ACCEPTED) return status;

  float accel_sens=0;
  if(load_accel){
    status = PAL_MPU6050_CalcAccelSens(target,&accel_sens);
    if(status!=MPU6050_ACCEPTED) return status;
  }
  accel[0] = (int32_t)((float)accel[0]*accel_sens);
  accel[1] = (int32_t)((float)accel[1]*accel_sens);
  accel[2] = (int32_t)((float)accel[2]*accel_sens);
  status = PAL_MPU6050_DMP_SetAccelBias(target,accel);
  if(status!=MPU6050_ACCEPTED) return status;

  return MPU6050_ACCEPTED;
}

void PAL_MPU6050_SetLog(void (*func)(const char* message)){
  PAL_MPU6050_Log = func;
}

/*===========================Configuration API==========================*/

PAL_MPU6050_State PAL_MPU6050_SetSensors(PAL_MPU6050* target,uint8_t sensors){
  uint8_t data;
  if(sensors&MPU6050_SENS_GYRO_ALL) data = MPU6050_CLK_PLL;
  else if(sensors) data = MPU6050_CLK_INTERNAL;
  else data = MPU6050_BIT_SLEEP;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_PWR_MGMT1,&data,1)!=HAL_OK){
    target->sensors = 0x00;
    return MPU6050_TRANS_FAIL;
  }

  data=0x00;
  if(!(sensors&MPU6050_SENS_GYRO_X)) data|=MPU6050_BIT_STBY_XG;
  if(!(sensors&MPU6050_SENS_GYRO_Y)) data|=MPU6050_BIT_STBY_YG;
  if(!(sensors&MPU6050_SENS_GYRO_Z)) data|=MPU6050_BIT_STBY_ZG;
  if(!(sensors&MPU6050_SENS_ACCEL )) data|=MPU6050_BIT_STBY_XYZA;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_PWR_MGMT2,&data,1)!=HAL_OK){
    target->sensors = 0x00;
    return MPU6050_TRANS_FAIL;
  }


  if(sensors&&(sensors!=MPU6050_SENS_ACCEL)){
    PAL_MPU6050_State status = PAL_MPU6050_SetIntLatched(target,false);
    if(status!=MPU6050_ACCEPTED) return status;
  }
  target->sensors = sensors;
  target->lp_accel_mode = false;
  PAL_Delay(SENSOR_INIT_DELAY*1000);
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_SetBypassMode(PAL_MPU6050* target,bool enable){
  if(target->bypass_en==enable) return MPU6050_ACCEPTED;
  uint8_t tmp;

  if(enable){
    if(PAL_MPU6050_Read(target->addr,MPU6050_REG_USER_CTRL,&tmp,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    tmp &= ~MPU6050_BIT_AUX_IF_EN;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_USER_CTRL,&tmp,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    PAL_Delay(BYPASS_SETTING_DELAY*1000);

    tmp = MPU6050_BIT_BYPASS_EN;
    if(target->active_low_int) tmp |= MPU6050_BIT_ACTL;
    if(target->int_latched) tmp |= (MPU6050_BIT_LATCH_EN|MPU6050_BIT_ANY_RD_CLR);
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_INT_PIN_CFG,&tmp,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
  }else{
    if(PAL_MPU6050_Read(target->addr,MPU6050_REG_USER_CTRL,&tmp,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    if(target->sensors&MPU6050_SENS_COMPASS) tmp |= MPU6050_BIT_AUX_IF_EN;
    else tmp &= ~MPU6050_BIT_AUX_IF_EN;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_USER_CTRL,&tmp,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    PAL_Delay(BYPASS_SETTING_DELAY*1000);

    tmp = target->active_low_int?MPU6050_BIT_ACTL:0x00;
    if(target->int_latched) tmp |= (MPU6050_BIT_LATCH_EN|MPU6050_BIT_ANY_RD_CLR);
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_INT_PIN_CFG,&tmp,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
  }

  target->bypass_en = enable;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_SetIntEnable(PAL_MPU6050* target,bool enable){
  uint8_t tmp;
  if(target->dmp_en){
    tmp = enable?MPU6050_BIT_DMP_INT_EN:0x00;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_INT_EN,&tmp,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    target->int_en = tmp;
  }else{
    if(!target->sensors) return MPU6050_ERROR;
    if(enable&&target->int_en) return MPU6050_ACCEPTED;
    tmp = enable?MPU6050_BIT_DATA_RDY_EN:0x00;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_INT_EN,&tmp,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    target->int_en = tmp;
  }
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_SetIntLevel(PAL_MPU6050* target,bool active_low){
  target->active_low_int = active_low;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_SetIntLatched(PAL_MPU6050* target,bool enable){
  if(target->int_latched==enable) return MPU6050_ACCEPTED;
  uint8_t tmp;

  if(enable) tmp = MPU6050_BIT_LATCH_EN | MPU6050_BIT_ANY_RD_CLR;
  else tmp = 0;
  if(target->bypass_en) tmp |= MPU6050_BIT_BYPASS_EN;
  if(target->active_low_int) tmp |= MPU6050_BIT_ACTL;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_INT_PIN_CFG,&tmp,1)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  target->int_latched = enable;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_SetLpAccelMode(PAL_MPU6050* target,uint8_t rate){
  if(rate>40) return MPU6050_INCORRECT;
  uint8_t tmp[2];
  PAL_MPU6050_State status;
  if(rate==0){
    status = PAL_MPU6050_SetIntLatched(target,false);
    if(status!=MPU6050_ACCEPTED) return status;
    tmp[0] = 0;
    tmp[1] = MPU6050_BIT_STBY_XYZG;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_PWR_MGMT1,tmp,2)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    target->lp_accel_mode = false;
    return MPU6050_ACCEPTED;
  }

  status = PAL_MPU6050_SetIntLatched(target,true);
  if(status!=MPU6050_ACCEPTED) return status;
  tmp[0] = MPU6050_BIT_LPA_CYCLE;
  if(rate==1){
    tmp[1] = MPU6050_LPA_1_25HZ;
    status = PAL_MPU6050_SetLPF(target,5);
  }else if(rate<=5){
    tmp[1] = MPU6050_LPA_5HZ;
    status = PAL_MPU6050_SetLPF(target,5);
  }else if(rate<=20){
    tmp[1] = MPU6050_LPA_20HZ;
    status = PAL_MPU6050_SetLPF(target,10);
  }else{
    tmp[1] = MPU6050_LPA_40HZ;
    status = PAL_MPU6050_SetLPF(target,20);
  }
  if(status!=MPU6050_ACCEPTED) return status;
  tmp[1] = (tmp[1]<<6)|MPU6050_BIT_STBY_XYZG;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_PWR_MGMT1,tmp,2)!=HAL_OK)
    return MPU6050_TRANS_FAIL;

  target->sensors = MPU6050_SENS_ACCEL;
  target->lp_accel_mode = true;
  target->clock_source = MPU6050_CLK_INTERNAL;
  status = PAL_MPU6050_SetConfigFIFO(target,0x00);
  return status;
}

PAL_MPU6050_State PAL_MPU6050_SetStateDMP(PAL_MPU6050* target,bool enable){
  if(target->dmp_en==enable) return MPU6050_ACCEPTED;
  uint8_t tmp;
  PAL_MPU6050_State status;

  if(enable){
    if(!target->dmp_loaded) return MPU6050_INCORRECT;
    status = PAL_MPU6050_SetIntEnable(target,false);
    if(status!=MPU6050_ACCEPTED) return status;
    status = PAL_MPU6050_SetBypassMode(target,false);
    if(status!=MPU6050_ACCEPTED) return status;
    status = PAL_MPU6050_SetSampleRate(target,target->dmp_sample_rate);
    if(status!=MPU6050_ACCEPTED) return status;
    tmp=0x00;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_FIFO_EN,&tmp,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    target->dmp_en = true;
    status = PAL_MPU6050_SetIntEnable(target,true);
    if(status!=MPU6050_ACCEPTED) return status;
    status = PAL_MPU6050_ResetFIFO(target);
  }else{
    status = PAL_MPU6050_SetIntEnable(target,false);
    if(status!=MPU6050_ACCEPTED) return status;
    tmp = target->fifo_sensors;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_FIFO_EN,&tmp,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    target->dmp_en = false;
    status = PAL_MPU6050_ResetFIFO(target);
  }

  return status;
}

PAL_MPU6050_State PAL_MPU6050_GetStateDMP(PAL_MPU6050* target,bool* enable){
  enable[0] = target->dmp_en;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_SetConfigFIFO(PAL_MPU6050*target, uint8_t sensor){
  if(!target->sensors) return MPU6050_ERROR;
  if(target->dmp_en) return MPU6050_ACCEPTED;
  uint8_t prev;
  PAL_MPU6050_State res,status;
  sensor &= ~MPU6050_SENS_COMPASS;

  prev = target->fifo_sensors;
  target->fifo_sensors = sensor&target->sensors;
  if(target->fifo_sensors!=sensor) res = MPU6050_ERROR;
  else res = MPU6050_ACCEPTED;

  if(sensor||target->lp_accel_mode) status = PAL_MPU6050_SetIntEnable(target,true);
  else status = PAL_MPU6050_SetIntEnable(target,false);
  if(status!=MPU6050_ACCEPTED) return status;

  if(sensor&& PAL_MPU6050_ResetFIFO(target)!=MPU6050_ACCEPTED){
    target->fifo_sensors = prev;
    return MPU6050_ERROR;
  }
  return res;
}

PAL_MPU6050_State PAL_MPU6050_GetConfigFIFO(PAL_MPU6050* target,uint8_t* sensor){
  sensor[0] = target-> fifo_sensors;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_SetLPF(PAL_MPU6050* target,uint16_t lpf){
  uint8_t data;
  if(!target->sensors) return MPU6050_ERROR;

  if(lpf==0) data = MPU6050_FILTER_256HZ_NOLPF2;
  else if(lpf>=188) data = MPU6050_FILTER_188HZ;
  else if(lpf>=98) data = MPU6050_FILTER_98HZ;
  else if(lpf>=42) data = MPU6050_FILTER_42HZ;
  else if(lpf>=20) data = MPU6050_FILTER_20HZ;
  else if(lpf>=10) data = MPU6050_FILTER_10HZ;
  else data = MPU6050_FILTER_5HZ;

  if(target->lpf==data) return MPU6050_ACCEPTED;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_LPF,&data,1)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  target->lpf = data;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_GetLPF(PAL_MPU6050* target,uint16_t* lpf){
  switch(target->lpf){
    case MPU6050_FILTER_188HZ : lpf[0] = 188;
                                break;
    case MPU6050_FILTER_98HZ  : lpf[0] = 98;
                                break;
    case MPU6050_FILTER_42HZ  : lpf[0] = 42;
                                break;
    case MPU6050_FILTER_20HZ  : lpf[0] = 20;
                                break;
    case MPU6050_FILTER_10HZ  : lpf[0] = 10;
                                break;
    case MPU6050_FILTER_5HZ   : lpf[0] = 5;
                                break;
    case MPU6050_FILTER_2100HZ_NOLPF:
    case MPU6050_FILTER_256HZ_NOLPF2:
    default : lpf[0] = 0;break;
  }
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_SetSampleRate(PAL_MPU6050* target,uint16_t rate){
  if(!target->sensors) return MPU6050_ERROR;
  if(target->dmp_en) return MPU6050_INCORRECT;

  uint8_t data;
  PAL_MPU6050_State status;
  if(target->lp_accel_mode){
      if(rate&&rate<=40){
        status = PAL_MPU6050_SetLpAccelMode(target,rate);
        return status;
      }
    status = PAL_MPU6050_SetLpAccelMode(target,0);
    if(status!=MPU6050_ACCEPTED) return status;
  }
  if(rate<4) rate=4;
  if(rate>1000) rate=1000;
  data = 1000/rate-1;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_RATE_DIV,&data,1)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  target->sample_rate = rate;
  status = PAL_MPU6050_SetLPF(target,target->sample_rate>>1);
  return status;
}

PAL_MPU6050_State PAL_MPU6050_GetSampleRate(PAL_MPU6050* target,uint16_t* rate){
  if(target->dmp_en) return MPU6050_INCORRECT;
  rate[0] = target->sample_rate;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_SetGyroFSR(PAL_MPU6050* target,uint16_t fsr){
  if(!target->sensors) return MPU6050_ERROR;

  uint8_t data;
  switch(fsr){
    case 250  : data = MPU6050_FSR_250DPS<<3;
                break;
    case 500  : data = MPU6050_FSR_500DPS<<3;
                break;
    case 1000 : data = MPU6050_FSR_1000DPS<<3;
                break;
    case 2000 : data = MPU6050_FSR_2000DPS<<3;
                break;
    default : return MPU6050_INCORRECT;
  }

  if(target->gyro_fsr==(data>>3)) return MPU6050_ACCEPTED;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_GYRO_CFG,&data,1)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  target->gyro_fsr = data>>3;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_GetGyroFSR(PAL_MPU6050* target,uint16_t* fsr){
  switch(target->gyro_fsr){
    case MPU6050_FSR_250DPS  :  fsr[0] = 250;
                                break;
    case MPU6050_FSR_500DPS  :  fsr[0] = 500;
                                break;
    case MPU6050_FSR_1000DPS :  fsr[0] = 1000;
                                break;
    case MPU6050_FSR_2000DPS :  fsr[0] = 2000;
                                break;
    default : fsr[0]=0;return MPU6050_INCORRECT;
  }
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_SetAccelFSR(PAL_MPU6050* target,uint8_t fsr){
  if(!target->sensors) return MPU6050_ERROR;

  uint8_t data;
  switch(fsr){
    case 2  : data = MPU6050_FSR_2G<<3;
              break;
    case 4  : data = MPU6050_FSR_4G<<3;
              break;
    case 8  : data = MPU6050_FSR_8G<<3;
              break;
    case 16 : data = MPU6050_FSR_16G<<3;
              break;
    default : return MPU6050_INCORRECT;
  }

  if(target->accel_fsr==(data>>3)) return MPU6050_ACCEPTED;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_ACCEL_CFG,&data,1)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  target->accel_fsr = data>>3;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_GetAccelFSR(PAL_MPU6050* target,uint8_t* fsr){
  switch(target->accel_fsr){
    case MPU6050_FSR_2G  :  fsr[0] = 2;
                            break;
    case MPU6050_FSR_4G  :  fsr[0] = 4;
                            break;
    case MPU6050_FSR_8G  :  fsr[0] = 8;
                            break;
    case MPU6050_FSR_16G :  fsr[0] = 16;
                            break;
    default : fsr[0]=0;return MPU6050_INCORRECT;
  }
  if(target->lp_accel_mode) fsr[0]<<=1;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_CalcGyroSens(PAL_MPU6050* target,float* scale){
  uint16_t fsr;
  if(PAL_MPU6050_GetGyroFSR(target,&fsr)!=MPU6050_ACCEPTED||fsr==0)
    return MPU6050_ERROR;
  scale[0] = 32768.0f/(float)fsr;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_CalcAccelSens(PAL_MPU6050* target,float* scale){
  uint8_t fsr;
  if(PAL_MPU6050_GetAccelFSR(target,&fsr)!=MPU6050_ACCEPTED||fsr==0)
    return MPU6050_ERROR;
  scale[0] = 32768.0f/(float)fsr;
  return MPU6050_ACCEPTED;
}

/*===========================Data Operate API===========================*/

PAL_MPU6050_State PAL_MPU6050_GetRawGyro(PAL_MPU6050* target,int16_t* data){
  uint8_t raw[6];
  if(!(target->sensors&MPU6050_SENS_GYRO_ALL))
    return MPU6050_ERROR;
  if(PAL_MPU6050_Read(target->addr,MPU6050_REG_RAW_GYRO,raw,6)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  data[0] = (int16_t)((raw[0]<<8)|raw[1]);
  data[1] = (int16_t)((raw[2]<<8)|raw[3]);
  data[2] = (int16_t)((raw[4]<<8)|raw[5]);
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_GetRawAccel(PAL_MPU6050* target,int16_t* data){
  uint8_t raw[6];
  if(!(target->sensors&MPU6050_SENS_ACCEL))
    return MPU6050_ERROR;
  if(PAL_MPU6050_Read(target->addr,MPU6050_REG_RAW_ACCEL,raw,6)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  data[0] = (int16_t)((raw[0]<<8)|raw[1]);
  data[1] = (int16_t)((raw[2]<<8)|raw[3]);
  data[2] = (int16_t)((raw[4]<<8)|raw[5]);
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_GetRawTemp(PAL_MPU6050* target,int16_t* data){
  uint8_t raw[2];
  if(!target->sensors) return MPU6050_ERROR;
  if(PAL_MPU6050_Read(target->addr,MPU6050_REG_RAW_TEMP,raw,2)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  data[0] = (int16_t)((raw[0]<<8)|raw[1]);
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_CalculateTemp(PAL_MPU6050* target,float* temp){
  int16_t data;
  PAL_MPU6050_State status = PAL_MPU6050_GetRawTemp(target,&data);
  if(status!=MPU6050_ACCEPTED) return status;
  temp[0] = MPU6050_TEMP_OFFSET + (float)data/MPU6050_TEMP_SENS;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_ReadFIFO(PAL_MPU6050* target,int16_t* gyro,int16_t* accel,uint8_t* sensors,uint8_t* more){
  if(target->dmp_en) return MPU6050_INCORRECT;
  if(!target->sensors) return MPU6050_ERROR;
  if(!target->fifo_sensors) return MPU6050_ERROR;

  sensors[0]=0;
  uint8_t data[FIFO_MAX_PACKET_SIZE];
  uint8_t packet_size=0;
  uint16_t fifo_count,index=0;

  if(target->fifo_sensors&MPU6050_SENS_GYRO_X) packet_size+=2;
  if(target->fifo_sensors&MPU6050_SENS_GYRO_Y) packet_size+=2;
  if(target->fifo_sensors&MPU6050_SENS_GYRO_Z) packet_size+=2;
  if(target->fifo_sensors&MPU6050_SENS_ACCEL)  packet_size+=6;
  if(PAL_MPU6050_Read(target->addr,MPU6050_REG_FIFO_COUNT,data,2)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  fifo_count = (data[0]<<8)|data[1];

  if(fifo_count<packet_size) {
    more[0]=0;
    return MPU6050_INCORRECT;
  }
  if(fifo_count>(target->max_fifo>>1)){
    if(PAL_MPU6050_Read(target->addr,MPU6050_REG_INT_ST,data,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    if(data[0]&MPU6050_BIT_FIFO_OVERFLOW){
      PAL_MPU6050_ResetFIFO(target);
      return MPU6050_ERROR;
    }
  }

  if(PAL_MPU6050_Read(target->addr,MPU6050_REG_FIFO_RW,data,packet_size)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  more[0] = fifo_count/packet_size-1;

  if(index!=packet_size && target->fifo_sensors&MPU6050_SENS_ACCEL){
    accel[0] = (int16_t)((data[index+0]<<8)|data[index+1]);
    accel[1] = (int16_t)((data[index+2]<<8)|data[index+3]);
    accel[2] = (int16_t)((data[index+4]<<8)|data[index+5]);
    sensors[0] |= MPU6050_SENS_ACCEL;
    index+=6;
  }
  if(index!=packet_size && target->fifo_sensors&MPU6050_SENS_GYRO_X){
    gyro[0] = (int16_t)((data[index+0]<<8)|data[index+1]);
    sensors[0] |= MPU6050_SENS_GYRO_X;
    index+=2;
  }
  if(index!=packet_size && target->fifo_sensors&MPU6050_SENS_GYRO_Y){
    gyro[1] = (int16_t)((data[index+0]<<8)|data[index+1]);
    sensors[0] |= MPU6050_SENS_GYRO_Y;
    index+=2;
  }
  if(index!=packet_size && target->fifo_sensors&MPU6050_SENS_GYRO_Z){
    gyro[2] = (int16_t)((data[index+0]<<8)|data[index+1]);
    sensors[0] |= MPU6050_SENS_GYRO_Z;
  }
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_StreamFIFO(PAL_MPU6050* target, uint16_t length,uint8_t* data,uint8_t* more){
  if(!target->dmp_en){
    if(LOG!=NULL) LOG("MPU: [Stream]dmp not launched\n");
    return MPU6050_INCORRECT;
  }
  if(!target->sensors) {
    if(LOG!=NULL) LOG("MPU: [Stream]sensor not open\n");
    return MPU6050_ERROR;
  }

  uint8_t tmp[2];
  uint16_t fifo_count;
  if(PAL_MPU6050_Read(target->addr,MPU6050_REG_FIFO_COUNT,tmp,2)!=HAL_OK) {
    if(LOG!=NULL) LOG("MPU: [Stream]read count fail\n");
    return MPU6050_TRANS_FAIL;
  }
  fifo_count = (tmp[0]<<8)|tmp[1];
  if(fifo_count<length){
    more[0]=0;
    if(LOG!=NULL) LOG("MPU: [Stream]data not ready\n");
    return MPU6050_INCORRECT;
  }
  if(fifo_count>(target->max_fifo>>1)){
    if(PAL_MPU6050_Read(target->addr,MPU6050_REG_INT_ST,tmp,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    if(tmp[0]&MPU6050_BIT_FIFO_OVERFLOW){
      if(LOG!=NULL) LOG("MPU: [Stream]fifo overflow\n");
      PAL_MPU6050_ResetFIFO(target);
      return MPU6050_ERROR;
    }
  }

  if(PAL_MPU6050_Read(target->addr,MPU6050_REG_FIFO_RW,data,length)!=HAL_OK){
    if(LOG!=NULL) LOG("MPU: [Stream]read data fail\n");
    return MPU6050_TRANS_FAIL;
  }
  more[0] = fifo_count/length-1;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_ResetFIFO(PAL_MPU6050* target){
  if(!target->sensors) return MPU6050_ERROR;

  uint8_t data = 0x00;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_INT_EN,&data,1)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_FIFO_EN,&data,1)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_USER_CTRL,&data,1)!=HAL_OK)
    return MPU6050_TRANS_FAIL;

  if(target->dmp_en){
    data = MPU6050_BIT_FIFO_RST|MPU6050_BIT_DMP_RST;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_USER_CTRL,&data,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    PAL_Delay(FIFO_RESET_DELAY*1000);

    data = MPU6050_BIT_DMP_EN|MPU6050_BIT_FIFO_EN;
    if(target->sensors&MPU6050_SENS_COMPASS) data |= MPU6050_BIT_AUX_IF_EN;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_USER_CTRL,&data,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;

    data = (target->int_en?MPU6050_REG_INT_EN:0x00);
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_INT_EN,&data,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;

    data = 0x00;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_FIFO_EN,&data,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
  }else{
    data = MPU6050_BIT_FIFO_RST;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_USER_CTRL,&data,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;

    if(target->bypass_en||!(target->sensors&MPU6050_SENS_COMPASS)) data = MPU6050_BIT_FIFO_EN;
    else data = MPU6050_BIT_FIFO_EN|MPU6050_BIT_AUX_IF_EN;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_USER_CTRL,&data,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    PAL_Delay(FIFO_RESET_DELAY*1000);

    data = (target->int_en?MPU6050_BIT_DATA_RDY_EN:0x00);
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_INT_EN,&data,1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    if(PAL_MPU6050_Write(target->addr,MPU6050_REG_FIFO_EN,&(target->fifo_sensors),1)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
  }
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_ReadWriteMem(PAL_MPU6050* target, uint16_t mem_addr,uint16_t length,uint8_t* data,bool read){
  uint8_t tmp[2];
  if(length==0||data==NULL) return MPU6050_ERROR;

  tmp[0] = (uint8_t)(mem_addr>>8);
  tmp[1] = (uint8_t)(mem_addr&0xFF);
  if(tmp[1]+length>MPU6050_BANK_SIZE) return MPU6050_INCORRECT;

  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_BANK_SEL,tmp,2)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  HAL_StatusTypeDef (*func)(uint8_t,uint8_t,uint8_t*,uint16_t);
  func = (read?PAL_MPU6050_Read:PAL_MPU6050_Write);
  if(func(target->addr,MPU6050_REG_MEM_RW,data,length)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_ProgramDMP(PAL_MPU6050* target,uint8_t*firmware,uint16_t length,uint16_t start_addr,uint16_t sample_rate){
  if(target->dmp_loaded) return MPU6050_INCORRECT;
  if(length==0||firmware==NULL) return MPU6050_ERROR;

  uint8_t chunk[DMP_FIRMWARE_CHUNK_SIZE],tmp[2];
  uint16_t current_size;
  PAL_MPU6050_State status;
  for(uint16_t addr=0;addr<length;addr+=current_size){
    current_size = min(DMP_FIRMWARE_CHUNK_SIZE,length-addr);

    status = PAL_MPU6050_ReadWriteMem(target,addr,current_size,firmware+addr,false);
    if(status!=MPU6050_ACCEPTED) return status;
    status = PAL_MPU6050_ReadWriteMem(target,addr,current_size,chunk,true);
    if(status!=MPU6050_ACCEPTED) return status;

    if(memcmp(firmware+addr,chunk,current_size)!=0) return MPU6050_ERROR;
  }

  tmp[0] = start_addr>>8;
  tmp[1] = start_addr&0xFF;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_PRGM_START,tmp,2)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  target->dmp_loaded = 1;
  target->dmp_sample_rate = sample_rate;
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_ReadRegister(PAL_MPU6050* target,uint8_t reg,uint8_t* data){
  if(reg==MPU6050_REG_FIFO_RW||reg==MPU6050_REG_MEM_RW) return MPU6050_INCORRECT;
  if(reg>=MPU6050_REG_NUMBER) return MPU6050_INCORRECT;
  if(PAL_MPU6050_Read(target->addr,reg,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  return MPU6050_ACCEPTED;
}

bool PAL_MPU6050_RegHealthyCheck(PAL_MPU6050* target){
  uint8_t data;
  for(uint8_t reg=0;reg<MPU6050_REG_NUMBER;reg++){
    if(reg==MPU6050_REG_FIFO_RW||reg==MPU6050_REG_MEM_RW) continue;
    if(PAL_MPU6050_Read(target->addr,reg,&data,1)!=HAL_OK)
      return false;
  }
  return true;
}

PAL_MPU6050_State PAL_MPU6050_GetIntState(PAL_MPU6050* target, uint16_t* status){
  uint8_t tmp[2];
  if(!target->sensors) return MPU6050_ERROR;
  if(PAL_MPU6050_Read(target->addr,MPU6050_REG_INT_ST,tmp,2)!=HAL_OK)
    return MPU6050_TRANS_FAIL;
  status[0] = ((uint16_t)tmp[0]<<8)|(uint16_t)tmp[1];
  return MPU6050_ACCEPTED;
}

uint16_t PAL_MPU6050_Row2Scale(const int8_t* row){
  uint16_t res;
  if(row[0]>0) res=0;
  else if(row[0]<0) res=4;
  else if(row[1]>0) res=1;
  else if(row[1]<0) res=5;
  else if(row[2]>0) res=2;
  else if(row[2]<0) res=6;
  else res=7;//error
  return res;
}

uint16_t PAL_MPU6050_Matrix2Scalar(const int8_t* mtx){
  uint16_t scalar=0x00;
  scalar |= PAL_MPU6050_Row2Scale(mtx);
  scalar |= PAL_MPU6050_Row2Scale(mtx+3)<<3;
  scalar |= PAL_MPU6050_Row2Scale(mtx+6)<<6;
  return scalar;
}

/*============================Self-Test API=============================*/

PAL_MPU6050_State PAL_MPU6050_SelfTest(PAL_MPU6050* target,int32_t* gyro,int32_t* accel,uint8_t* result){
#if SELF_TEST_TRIES>255
#error Too Much Tries of Self-Test!
#endif
  int32_t gyro_st[3],accel_st[3];
  bool gyro_error,accel_error,dmp_was_on;
  uint8_t gyro_result,accel_result;
  uint8_t try,limit = SELF_TEST_TRIES;
  uint8_t accel_fsr,fifo_sensors,sensors_on;
  uint16_t gyro_fsr,sample_rate,lpf;
  PAL_MPU6050_State status;

  if(target->dmp_en){
    status = PAL_MPU6050_SetStateDMP(target,false);
    if(status!=MPU6050_ACCEPTED) return status;
    dmp_was_on = true;
  }else dmp_was_on = false;

  status = PAL_MPU6050_GetGyroFSR(target,&gyro_fsr);
  if(status!=MPU6050_ACCEPTED) return status;
  status = PAL_MPU6050_GetAccelFSR(target,&accel_fsr);
  if(status!=MPU6050_ACCEPTED) return status;
  PAL_MPU6050_GetLPF(target,&lpf);
  status = PAL_MPU6050_GetSampleRate(target,&sample_rate);
  if(status!=MPU6050_ACCEPTED) return status;
  PAL_MPU6050_GetConfigFIFO(target,&fifo_sensors);
  sensors_on = target->sensors;

  for(try=0;try<limit;try++)
    if(PAL_MPU6050_GetChipBiases(target,gyro,accel,false)==MPU6050_ACCEPTED) break;
  if(try==limit){
    result[0]=0;
    goto RESTORE;
  }
  for(try=0;try<limit;try++)
    if(PAL_MPU6050_GetChipBiases(target,gyro_st,accel_st,true)==MPU6050_ACCEPTED) break;
  if(try==limit){
    result[0]=0;
    goto RESTORE;
  }
  accel_error = (PAL_MPU6050_AccelTest(target,accel,accel_st,&accel_result)!=MPU6050_ACCEPTED);
  gyro_error  = (PAL_MPU6050_GyroTest (target,gyro ,gyro_st ,&gyro_result )!=MPU6050_ACCEPTED);
  result[0] = 0;
  if(gyro_error ==false) result[0] |= 0x01;
  if(accel_error==false) result[0] |= 0x02;

  RESTORE:
  target->gyro_fsr = 0xFF;
  target->accel_fsr = 0xFF;
  target->lpf = 0xFFFF;
  target->sample_rate = 0xFFFF;
  target->sensors = 0xFF;
  target->fifo_sensors = 0xFF;
  target->clock_source = MPU6050_CLK_PLL;
  status = PAL_MPU6050_SetGyroFSR(target,gyro_fsr);
  if(status!=MPU6050_ACCEPTED) return status;
  status = PAL_MPU6050_SetAccelFSR(target,accel_fsr);
  if(status!=MPU6050_ACCEPTED) return status;
  status = PAL_MPU6050_SetLPF(target,lpf);
  if(status!=MPU6050_ACCEPTED) return status;
  status = PAL_MPU6050_SetSampleRate(target,sample_rate);
  if(status!=MPU6050_ACCEPTED) return status;
  status = PAL_MPU6050_SetSensors(target,sensors_on);
  if(status!=MPU6050_ACCEPTED) return status;
  status = PAL_MPU6050_SetConfigFIFO(target,fifo_sensors);
  if(dmp_was_on){
    status = PAL_MPU6050_SetStateDMP(target,true);
    if(status!=MPU6050_ACCEPTED) return status;
  }

  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_GetChipBiases(PAL_MPU6050* target,int32_t* gyro,int32_t* accel,bool hw_test){
  uint8_t data[FIFO_MAX_PACKET_SIZE],packet_count;
  uint16_t fifo_count;
  //reset device first
  data[0] = 0x01;
  data[1] = 0x00;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_PWR_MGMT1,data,2)!=HAL_OK) return MPU6050_TRANS_FAIL;
  PAL_Delay(2*CHIP_RESET_DELAY*1000);
  //flush registers
  data[0] = 0x00;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_INT_EN,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_FIFO_EN,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_PWR_MGMT1,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_I2C_MASTER,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_USER_CTRL,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  //reset fifo&dmp
  data[0] = MPU6050_BIT_FIFO_RST|MPU6050_BIT_DMP_RST;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_USER_CTRL,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  PAL_Delay(2*FIFO_RESET_DELAY*10000);
  //set LPF
  data[0] = test_reg_lpf;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_LPF,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  //set Chip Rate
  data[0] = test_reg_rate_div;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_RATE_DIV,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  //set gyro_fsr
  data[0] = test_reg_gyro_fsr;
  if(hw_test) data[0] |= 0xE0;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_GYRO_CFG,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  //set accel_fsr
  data[0] = test_reg_accel_fsr;
  if(hw_test) data[0] |= 0xE0;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_ACCEL_CFG,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  //hardware test delay
  if(hw_test) PAL_Delay(2*CHIP_RESET_DELAY*1000);
  //Fill FIFO for test_wait_ms milliseconds
  data[0] = MPU6050_BIT_FIFO_EN;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_USER_CTRL,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  data[0] = MPU6050_SENS_GYRO_ALL|MPU6050_SENS_ACCEL;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_FIFO_EN,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  PAL_Delay(test_wait_ms*1000);
  data[0] = 0x00;
  if(PAL_MPU6050_Write(target->addr,MPU6050_REG_FIFO_EN,data,1)!=HAL_OK) return MPU6050_TRANS_FAIL;
  if(PAL_MPU6050_Read(target->addr,MPU6050_REG_FIFO_COUNT,data,2)!=HAL_OK) return MPU6050_TRANS_FAIL;

  fifo_count = (data[0]<<8)|data[1];
  packet_count = fifo_count/FIFO_MAX_PACKET_SIZE;
  memset(gyro,0,sizeof(int32_t)*3);
  memset(accel,0,sizeof(int32_t)*3);

  for(uint8_t i=0;i<packet_count;i++){
    int16_t accel_cur[3],gyro_cur[3];
    if(PAL_MPU6050_Read(target->addr,MPU6050_REG_FIFO_RW,data,FIFO_MAX_PACKET_SIZE)!=HAL_OK)
      return MPU6050_TRANS_FAIL;
    accel_cur[0] = (int16_t)((data[0]<<8)|data[1]);
    accel_cur[1] = (int16_t)((data[2]<<8)|data[3]);
    accel_cur[2] = (int16_t)((data[4]<<8)|data[5]);
    accel[0] += (int32_t)accel_cur[0];
    accel[1] += (int32_t)accel_cur[1];
    accel[2] += (int32_t)accel_cur[2];
    gyro_cur[0] = (int16_t)((data[6]<<8)|data[7]);
    gyro_cur[1] = (int16_t)((data[8]<<8)|data[9]);
    gyro_cur[2] = (int16_t)((data[10]<<8)|data[11]);
    gyro[0] += (int32_t)gyro_cur[0];
    gyro[1] += (int32_t)gyro_cur[1];
    gyro[2] += (int32_t)gyro_cur[2];
  }
  gyro[0] = (int32_t)((int64_t)gyro[0]<<16/test_gyro_sens/packet_count);
  gyro[1] = (int32_t)((int64_t)gyro[1]<<16/test_gyro_sens/packet_count);
  gyro[2] = (int32_t)((int64_t)gyro[2]<<16/test_gyro_sens/packet_count);
  accel[0] = (int32_t)((int64_t)accel[0]<<16/test_accel_sens/packet_count);
  accel[1] = (int32_t)((int64_t)accel[1]<<16/test_accel_sens/packet_count);
  accel[2] = (int32_t)((int64_t)accel[2]<<16/test_accel_sens/packet_count);
  //process gravity
  if(accel[2]>0) accel[2]-=65536;
  else accel[2]+=65536;

  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_GyroTest(PAL_MPU6050* target,int32_t* bias_regular,int32_t* bias_st,uint8_t* result){
  uint8_t tmp[3];
  float st_shift,st_shit_cust,st_shift_var;
  result[0] = 0;

  if(PAL_MPU6050_Read(target->addr,0x0D,tmp,3)!=HAL_OK){
    result[0] = 0x07;
    return MPU6050_TRANS_FAIL;
  }
  tmp[0] &= 0x1F;tmp[1] &= 0x1F;tmp[2] &= 0x1F;

  for(uint8_t i=0;i<3;i++){
    st_shit_cust = (float)labs(bias_regular[i]-bias_st[i])/65536.0f;
    if(tmp[i]){
      st_shift = 3275.0f/(float)test_gyro_sens;
      while(--tmp[i]) st_shift*=1.046f;
      st_shift_var = st_shit_cust/st_shift-1.0f;
      if(fabsf(st_shift_var)>max_gyro_var) result[0] |= 1<<i;
    }
    else if(st_shit_cust<min_dps||st_shit_cust>max_dps) result[0] |= 1<<i;
  }

  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_AccelTest(PAL_MPU6050* target,int32_t* bias_regular,int32_t* bias_st,uint8_t* result){
  uint8_t tmp[3];
  float st_shift[3],st_shift_cust,st_shit_var;

  result[0] = 0;
  if(PAL_MPU6050_GetAccelShift(target,st_shift)!=MPU6050_ACCEPTED){
    result[0] = 0x07;
    return MPU6050_TRANS_FAIL;
  }
  for(uint8_t i=0;i<3;i++){
    st_shift_cust = (float)labs(bias_regular[i]-bias_st[i])/65536.0f;
    if(st_shift[i]){
      st_shit_var = st_shift_cust/st_shift[i]-1.0f;
      if(fabsf(st_shit_var)>max_accel_var) result[0] |= 1<<i;
    }
    else if(st_shift_cust<min_gravity||st_shift_cust>max_gravity) result[0] |= 1<<i;
  }
  return MPU6050_ACCEPTED;
}

PAL_MPU6050_State PAL_MPU6050_GetAccelShift(PAL_MPU6050* target,float* st_shift){
  uint8_t tmp[4],shift_codes[3];
  if(PAL_MPU6050_Read(target->addr,0x0D,tmp,4)!=HAL_OK)
    return MPU6050_TRANS_FAIL;

  shift_codes[0] = (tmp[0]&0xE0)>>3 | (tmp[3]&0x30)>>4;
  shift_codes[1] = (tmp[1]&0xE0)>>3 | (tmp[3]&0x0C)>>2;
  shift_codes[2] = (tmp[2]&0xE0)>>3 | (tmp[3]&0x03);

  for(uint8_t i=0;i<3;i++){
    if(!shift_codes[i]){
      st_shift[i] = 0.0f;
      continue;
    }
    st_shift[i] = 0.34f;
    while(--shift_codes[i]) st_shift[i] *= 1.034f;
  }

  return MPU6050_ACCEPTED;
}

#endif //PAL_FUNC_MPU6050