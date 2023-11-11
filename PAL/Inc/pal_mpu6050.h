#ifndef PIONEER_PAL_MPU6050_H
#define PIONEER_PAL_MPU6050_H

#include "pal_global.h"

#ifdef PAL_FUNC_MPU6050

typedef enum{
  MPU6050_ACCEPTED = 0x00,
  MPU6050_ERROR = 0x01,
  MPU6050_TRANS_FAIL = 0x02,
  MPU6050_INCORRECT = 0x03
}PAL_MPU6050_State;

typedef struct{
  void (*tap_cb)(uint8_t count, uint8_t direction);
  void (*android_orient_cb)(uint8_t orientation);
  uint16_t orient;
  uint16_t feature_mask;
  uint16_t fifo_rate;
  uint16_t packet_length;
}PAL_MPU6050_DMP;

typedef struct{
  uint8_t addr;
  uint8_t sensors;
  uint16_t gyro_fsr;
  uint8_t accel_fsr;
  uint8_t fifo_sensors;
  uint16_t max_fifo;
  bool dmp_en,dmp_loaded;
  uint16_t dmp_sample_rate,sample_rate;
  bool int_en,int_latched,active_low_int;
  bool bypass_en;
  uint16_t lpf;
  bool lp_accel_mode,accel_half;
  uint8_t clock_source;
  PAL_MPU6050_DMP* dmp;//TODO: init
}PAL_MPU6050;

extern const int8_t MPU6050_ORIENTATION_DEFAULT[9];
extern void (*PAL_MPU6050_Log)(const char* message);

//Self Definable Tools
HAL_StatusTypeDef PAL_MPU6050_Write(uint8_t addr,uint8_t reg,uint8_t* data,uint16_t size);
HAL_StatusTypeDef PAL_MPU6050_Read(uint8_t addr,uint8_t reg,uint8_t* data,uint16_t size);
PAL_MPU6050_State PAL_MPU6050_Init(PAL_MPU6050* target,PAL_MPU6050_DMP* dmp);
PAL_MPU6050_State PAL_MPU6050_Calibrate(PAL_MPU6050* target,bool load_gyro,bool load_accel);
void PAL_MPU6050_SetLog(void (*func)(const char* message));

//Configuration APIs
PAL_MPU6050_State PAL_MPU6050_SetSensors(PAL_MPU6050* target,uint8_t sensors);
PAL_MPU6050_State PAL_MPU6050_SetBypassMode(PAL_MPU6050* target,bool enable);
PAL_MPU6050_State PAL_MPU6050_SetIntEnable(PAL_MPU6050* target,bool enable);
PAL_MPU6050_State PAL_MPU6050_SetIntLevel(PAL_MPU6050* target,bool active_low);
PAL_MPU6050_State PAL_MPU6050_SetIntLatched(PAL_MPU6050* target,bool enable);
PAL_MPU6050_State PAL_MPU6050_SetLpAccelMode(PAL_MPU6050* target,uint8_t rate);
PAL_MPU6050_State PAL_MPU6050_SetStateDMP(PAL_MPU6050* target,bool enable);
PAL_MPU6050_State PAL_MPU6050_GetStateDMP(PAL_MPU6050* target,bool* enable);
PAL_MPU6050_State PAL_MPU6050_SetLPF(PAL_MPU6050* target,uint16_t lpf);
PAL_MPU6050_State PAL_MPU6050_GetLPF(PAL_MPU6050* target,uint16_t* lpf);
PAL_MPU6050_State PAL_MPU6050_SetSampleRate(PAL_MPU6050* target,uint16_t rate);
PAL_MPU6050_State PAL_MPU6050_GetSampleRate(PAL_MPU6050* target,uint16_t* rate);
PAL_MPU6050_State PAL_MPU6050_SetConfigFIFO(PAL_MPU6050*target, uint8_t sensor);
PAL_MPU6050_State PAL_MPU6050_GetConfigFIFO(PAL_MPU6050* target,uint8_t* sensor);
PAL_MPU6050_State PAL_MPU6050_SetGyroFSR(PAL_MPU6050* target,uint16_t fsr);
PAL_MPU6050_State PAL_MPU6050_GetGyroFSR(PAL_MPU6050* target,uint16_t* fsr);
PAL_MPU6050_State PAL_MPU6050_SetAccelFSR(PAL_MPU6050* target,uint8_t fsr);
PAL_MPU6050_State PAL_MPU6050_GetAccelFSR(PAL_MPU6050* target,uint8_t* fsr);
PAL_MPU6050_State PAL_MPU6050_CalcGyroSens(PAL_MPU6050* target,float* scale);
PAL_MPU6050_State PAL_MPU6050_CalcAccelSens(PAL_MPU6050* target,float* scale);

//Data Operate Functions
PAL_MPU6050_State PAL_MPU6050_GetRawGyro(PAL_MPU6050* target,int16_t* data);
PAL_MPU6050_State PAL_MPU6050_GetRawAccel(PAL_MPU6050* target,int16_t* data);
PAL_MPU6050_State PAL_MPU6050_GetRawTemp(PAL_MPU6050* target,int16_t* data);
PAL_MPU6050_State PAL_MPU6050_CalculateTemp(PAL_MPU6050* target,float* temp);
PAL_MPU6050_State PAL_MPU6050_ReadFIFO(PAL_MPU6050* target,int16_t* gyro,int16_t* accel,uint8_t* sensors,uint8_t* more);
PAL_MPU6050_State PAL_MPU6050_StreamFIFO(PAL_MPU6050* target, uint16_t length,uint8_t* data,uint8_t* more);
PAL_MPU6050_State PAL_MPU6050_ResetFIFO(PAL_MPU6050* target);
PAL_MPU6050_State PAL_MPU6050_ReadWriteMem(PAL_MPU6050* target, uint16_t mem_addr,uint16_t length,uint8_t* data,bool read);
PAL_MPU6050_State PAL_MPU6050_ProgramDMP(PAL_MPU6050* target,uint8_t* firmware,uint16_t length,uint16_t start_addr,uint16_t sample_rate);
PAL_MPU6050_State PAL_MPU6050_ReadRegister(PAL_MPU6050* target,uint8_t reg,uint8_t* data);
bool PAL_MPU6050_RegHealthyCheck(PAL_MPU6050* target);
PAL_MPU6050_State PAL_MPU6050_GetIntState(PAL_MPU6050* target, uint16_t* status);
uint16_t PAL_MPU6050_Row2Scale(const int8_t* row);
uint16_t PAL_MPU6050_Matrix2Scalar(const int8_t* mtx);

//Self Test Functions
PAL_MPU6050_State PAL_MPU6050_SelfTest(PAL_MPU6050* target,int32_t* gyro,int32_t* accel,uint8_t* result);
PAL_MPU6050_State PAL_MPU6050_GetChipBiases(PAL_MPU6050* target,int32_t* gyro,int32_t* accel,bool hw_test);
PAL_MPU6050_State PAL_MPU6050_GetAccelShift(PAL_MPU6050* target,float* st_shift);
PAL_MPU6050_State PAL_MPU6050_GyroTest(PAL_MPU6050* target,int32_t* bias_regular,int32_t* bias_st,uint8_t* result);
PAL_MPU6050_State PAL_MPU6050_AccelTest(PAL_MPU6050* target,int32_t* bias_regular,int32_t* bias_st,uint8_t* result);

#define MPU6050_I2C_ADDR        (0x68)
#define MPU6050_REG_NUMBER      (118)
#define MPU6050_TEMP_OFFSET     (36.53f)
#define MPU6050_TEMP_SENS       (340.0f)
#define MPU6050_DEFAULT_HZ      (100)
#define MPU6050_BANK_SIZE       (256)
//#define MPU6050_DISABLE_Q14

//MPU6050-Register Table
#define MPU6050_REG_WHO_AM_I    (0x75)
#define MPU6050_REG_RATE_DIV    (0x19)
#define MPU6050_REG_LPF         (0x1A)
#define MPU6050_REG_PRODUCT_ID  (0x0C)
#define MPU6050_REG_USER_CTRL   (0x6A)
#define MPU6050_REG_FIFO_EN     (0x23)
#define MPU6050_REG_GYRO_CFG    (0x1B)
#define MPU6050_REG_ACCEL_CFG   (0x1C)
#define MPU6050_REG_MOTION_THR  (0x1F)
#define MPU6050_REG_MOTION_DUR  (0x20)
#define MPU6050_REG_FIFO_COUNT  (0x72)
#define MPU6050_REG_FIFO_RW     (0x74)
#define MPU6050_REG_RAW_GYRO    (0x43)
#define MPU6050_REG_RAW_ACCEL   (0x3B)
#define MPU6050_REG_RAW_TEMP    (0x41)
#define MPU6050_REG_INT_EN      (0x38)
#define MPU6050_REG_DMP_INT_ST  (0x39)
#define MPU6050_REG_INT_ST      (0x3A)
#define MPU6050_REG_PWR_MGMT1   (0x6B)
#define MPU6050_REG_PWR_MGMT2   (0x6C)
#define MPU6050_REG_INT_PIN_CFG (0x37)
#define MPU6050_REG_MEM_RW      (0x6F)
#define MPU6050_REG_ACCEL_OFFS  (0x06)
#define MPU6050_REG_I2C_MASTER  (0x24)
#define MPU6050_REG_BANK_SEL    (0x6D)
#define MPU6050_REG_MEM_START   (0x6E)
#define MPU6050_REG_PRGM_START  (0x70)

#define MPU6050_SENS_GYRO_X     (0x40)
#define MPU6050_SENS_GYRO_Y     (0x20)
#define MPU6050_SENS_GYRO_Z     (0x10)
#define MPU6050_SENS_GYRO_ALL   (MPU6050_SENS_GYRO_X|MPU6050_SENS_GYRO_Y|MPU6050_SENS_GYRO_Z)
#define MPU6050_SENS_ACCEL      (0x08)
#define MPU6050_SENS_COMPASS    (0x01)

#define MPU6050_BIT_I2C_MST_VDDIO (0x80)
#define MPU6050_BIT_FIFO_EN       (0x40)
#define MPU6050_BIT_DMP_EN        (0x80)
#define MPU6050_BIT_FIFO_RST      (0x04)
#define MPU6050_BIT_DMP_RST       (0x08)
#define MPU6050_BIT_FIFO_OVERFLOW (0x10)
#define MPU6050_BIT_DATA_RDY_EN   (0x01)
#define MPU6050_BIT_DMP_INT_EN    (0x02)
#define MPU6050_BIT_MOT_INT_EN    (0x40)
#define MPU6050_BIT_FSR           (0x18)
#define MPU6050_BIT_LPF           (0x07)
#define MPU6050_BIT_HPF           (0x07)
#define MPU6050_BIT_CLOCK         (0x07)
#define MPU6050_BIT_FIFO_SIZE1024 (0x40)
#define MPU6050_BIT_FIFO_SIZE2048 (0x80)
#define MPU6050_BIT_FIFO_SIZE4096 (0xC0)
#define MPU6050_BIT_RESET         (0x80)
#define MPU6050_BIT_SLEEP         (0x40)
#define MPU6050_BIT_S0_DELAY_EN   (0x01)
#define MPU6050_BIT_S1_DELAY_EN   (0x02)
#define MPU6050_BIT_S2_DELAY_EN   (0x04)
#define MPU6050_BIT_SLAVE_LENGTH  (0x0F)
#define MPU6050_BIT_SLAVE_BYTE_SW (0x40)
#define MPU6050_BIT_SLAVE_GROUP   (0x10)
#define MPU6050_BIT_SLAVE_EN      (0x80)
#define MPU6050_BIT_I2C_READ      (0x80)
#define MPU6050_BIT_I2C_MST_DLY   (0x1F)
#define MPU6050_BIT_AUX_IF_EN     (0x20)
#define MPU6050_BIT_ACTL          (0x80)
#define MPU6050_BIT_LATCH_EN      (0x20)
#define MPU6050_BIT_ANY_RD_CLR    (0x10)
#define MPU6050_BIT_BYPASS_EN     (0x02)
#define MPU6050_BITS_WOM_EN       (0xC0)
#define MPU6050_BIT_LPA_CYCLE     (0x20)
#define MPU6050_BIT_STBY_XA       (0x20)
#define MPU6050_BIT_STBY_YA       (0x10)
#define MPU6050_BIT_STBY_ZA       (0x08)
#define MPU6050_BIT_STBY_XG       (0x04)
#define MPU6050_BIT_STBY_YG       (0x02)
#define MPU6050_BIT_STBY_ZG       (0x01)
#define MPU6050_BIT_STBY_XYZA     (MPU6050_BIT_STBY_XA | MPU6050_BIT_STBY_YA | MPU6050_BIT_STBY_ZA)
#define MPU6050_BIT_STBY_XYZG     (MPU6050_BIT_STBY_XG | MPU6050_BIT_STBY_YG | MPU6050_BIT_STBY_ZG)

typedef enum {
  MPU6050_FILTER_256HZ_NOLPF2 = 0,
  MPU6050_FILTER_188HZ,
  MPU6050_FILTER_98HZ,
  MPU6050_FILTER_42HZ,
  MPU6050_FILTER_20HZ,
  MPU6050_FILTER_10HZ,
  MPU6050_FILTER_5HZ,
  MPU6050_FILTER_2100HZ_NOLPF,
  NUM_FILTER
}MPU6050_LPF;

typedef enum {
  MPU6050_FSR_250DPS = 0,
  MPU6050_FSR_500DPS,
  MPU6050_FSR_1000DPS,
  MPU6050_FSR_2000DPS,
  NUM_GYRO_FSR
}MPU6050_GYRO_FSR;

typedef enum {
  MPU6050_FSR_2G = 0,
  MPU6050_FSR_4G,
  MPU6050_FSR_8G,
  MPU6050_FSR_16G,
  NUM_ACCEL_FSR
}MPU6050_ACCEL_FSR;

typedef enum {
  MPU6050_CLK_INTERNAL = 0,
  MPU6050_CLK_PLL,
  NUM_CLK
}MPU6050_CLOCK;

typedef enum {
  MPU6050_LPA_1_25HZ,
  MPU6050_LPA_5HZ,
  MPU6050_LPA_20HZ,
  MPU6050_LPA_40HZ,
  NUM_ACCEL_RATE
}MPU6050_LP_ACCEL_RATE;

#endif //PAL_FUNC_MPU6050

#endif //PIONEER_PAL_MPU6050_H
//TODO: Compass Function
//TODO: Interrupt Processing