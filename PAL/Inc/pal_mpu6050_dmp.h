#ifndef PIONEER_PAL_MPU6050_DMP_H
#define PIONEER_PAL_MPU6050_DMP_H

#include "pal_global.h"
#ifdef PAL_FUNC_MPU6050

#define MPU6050_DMP_TAP_X                   (0x01)
#define MPU6050_DMP_TAP_Y                   (0x02)
#define MPU6050_DMP_TAP_Z                   (0x04)
#define MPU6050_DMP_TAP_XYZ                 (0x07)

#define MPU6050_DMP_INT_GESTURE             (0x01)
#define MPU6050_DMP_INT_CONTINUOUS          (0x02)

#define MPU6050_DMP_FEATURE_TAP             (0x001)
#define MPU6050_DMP_FEATURE_ANDROID_ORIENT  (0x002)
#define MPU6050_DMP_FEATURE_LP_QUAT         (0x004)
#define MPU6050_DMP_FEATURE_PEDOMETER       (0x008)
#define MPU6050_DMP_FEATURE_6X_LP_QUAT      (0x010)
#define MPU6050_DMP_FEATURE_GYRO_CAL        (0x020)
#define MPU6050_DMP_FEATURE_SEND_RAW_ACCEL  (0x040)
#define MPU6050_DMP_FEATURE_SEND_RAW_GYRO   (0x080)
#define MPU6050_DMP_FEATURE_SEND_CAL_GYRO   (0x100)

#define MPU6050_DMP_SENSE_WXYZ_QUAT         (0x100)
#define Quaternion30 1073741824.0f

//DMP Main Functions
PAL_MPU6050_State PAL_MPU6050_DMP_Init(PAL_MPU6050* target,PAL_MPU6050_DMP* dmp,uint8_t* error_step);
PAL_MPU6050_State PAL_MPU6050_DMP_ReadFIFO(PAL_MPU6050* target,int16_t* gyro,int16_t* accel,int32_t* quaternion,uint16_t* sensors,uint8_t* more);
PAL_MPU6050_State PAL_MPU6050_DMP_DecodeGesture(PAL_MPU6050* target,const uint8_t* gesture);
PAL_MPU6050_State PAL_MPU6050_DMP_Calculate(PAL_MPU6050* target,float* pitch,float* roll,float* yaw,int16_t* gyro_raw,int16_t* accel_raw);

//Setup Functions
PAL_MPU6050_State PAL_MPU6050_DMP_LoadFirmware(PAL_MPU6050* target);
PAL_MPU6050_State PAL_MPU6050_DMP_SetRateFIFO(PAL_MPU6050* target,uint16_t rate);
PAL_MPU6050_State PAL_MPU6050_DMP_GetRateFIFO(PAL_MPU6050* target,uint16_t* rate);
PAL_MPU6050_State PAL_MPU6050_DMP_SetFeature(PAL_MPU6050* target,uint16_t mask);
PAL_MPU6050_State PAL_MPU6050_DMP_GetFeature(PAL_MPU6050* target,uint16_t* mask);
PAL_MPU6050_State PAL_MPU6050_DMP_SetIntMode(PAL_MPU6050* target,uint8_t mode);
PAL_MPU6050_State PAL_MPU6050_DMP_SetOrientation(PAL_MPU6050* target,uint16_t orient);
PAL_MPU6050_State PAL_MPU6050_DMP_GyroCalibrate(PAL_MPU6050* target,bool enable);
PAL_MPU6050_State PAL_MPU6050_DMP_SetGyroBias(PAL_MPU6050* target,const int32_t* bias);
PAL_MPU6050_State PAL_MPU6050_DMP_SetAccelBias(PAL_MPU6050* target,const int32_t* bias);

//Tap Functions
PAL_MPU6050_State PAL_MPU6050_DMP_SetTapThresh(PAL_MPU6050* target,uint8_t axis,uint16_t thresh);
PAL_MPU6050_State PAL_MPU6050_DMP_SetTapAxes(PAL_MPU6050* target,uint8_t axis);
PAL_MPU6050_State PAL_MPU6050_DMP_SetTapCount(PAL_MPU6050* target,uint8_t min_taps);
PAL_MPU6050_State PAL_MPU6050_DMP_SetTapTime(PAL_MPU6050* target,uint16_t time);
PAL_MPU6050_State PAL_MPU6050_DMP_SetTapTimeMulti(PAL_MPU6050* target,uint16_t time);
PAL_MPU6050_State PAL_MPU6050_DMP_SetShakeRejectThresh(PAL_MPU6050* target,int32_t sf,uint16_t thresh);
PAL_MPU6050_State PAL_MPU6050_DMP_SetShakeRejectTime(PAL_MPU6050* target,uint16_t time);
PAL_MPU6050_State PAL_MPU6050_DMP_SetShakeRejectTimeout(PAL_MPU6050* target,uint16_t time);

//Quaternions Functions
PAL_MPU6050_State PAL_MPU6050_DMP_EnableQuaternions3(PAL_MPU6050* target,bool enable);
PAL_MPU6050_State PAL_MPU6050_DMP_EnableQuaternions6(PAL_MPU6050* target,bool enable);

//Pedometer Functions
PAL_MPU6050_State PAL_MPU6050_DMP_GetStep(PAL_MPU6050* target,uint32_t* count);
PAL_MPU6050_State PAL_MPU6050_DMP_SetStep(PAL_MPU6050* target,uint32_t count);
PAL_MPU6050_State PAL_MPU6050_DMP_GetWalkTime(PAL_MPU6050* target,uint32_t* time);
PAL_MPU6050_State PAL_MPU6050_DMP_SetWalkTime(PAL_MPU6050* target,uint32_t time);

//Callback Setters
PAL_MPU6050_State PAL_MPU6050_DMP_SetOrientCallback(PAL_MPU6050* target,void (*func)(uint8_t));
PAL_MPU6050_State PAL_MPU6050_DMP_SetTapCallback(PAL_MPU6050* target,void (*func)(uint8_t,uint8_t));

#endif //PAL_FUNC_MPU6050

#endif //PIONEER_PAL_MPU6050_DMP_H
