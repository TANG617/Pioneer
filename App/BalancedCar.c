//
// Created by 李唐 on 2023/11/30.
//

#include "BalancedCar.h"
static float uint2float(int x_int, float x_min, float x_max, int bits){
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int)*span/((float)((1<<bits)-1)) + offset;
}


BalancedCarType Car;
HAL_StatusTypeDef BalancedCarInit(){
    Car.LMotor.CanHandler = &hcan;
    Car.LMotor.id = 0x01;
    Car.LMotor.Mode = Velocity;
    Car.LMotor.Velocity = 0;

    Car.RMotor.CanHandler = &hcan;
    Car.RMotor.id = 0x02;
    Car.RMotor.Mode = Velocity;
    Car.RMotor.Velocity = 0;

    if(NodeMotorEnable(&Car.LMotor) != HAL_OK ) return HAL_ERROR;
    if(NodeMotorEnable(&Car.RMotor) != HAL_OK ) return HAL_ERROR;
    return HAL_OK;
}

HAL_StatusTypeDef BalancedCarUpdate(){
    if(NodeMotorVelocityControl(&Car.LMotor) != HAL_OK) return HAL_ERROR ;
    if(NodeMotorVelocityControl(&Car.RMotor) != HAL_OK) return HAL_ERROR ;
    return HAL_OK;
}

HAL_StatusTypeDef BalancedCarFetchStatus(uint16_t id,uint8_t *StatusMessage){

    if(id == Car.RMotor.id - Car.RMotor.Mode)
    {
        int pBuf=(StatusMessage[1]<<8)|StatusMessage[2];
        int vBuf=(StatusMessage[3]<<4)|(StatusMessage[4]>>4);
        int tBuf=((StatusMessage[4]&0xF)<<8)|StatusMessage[5];
        Car.RMotor.Status.Position = uint2float(pBuf, P_MIN, P_MAX, 16);
        Car.RMotor.Status.Velocity = uint2float(vBuf, V_MIN, V_MAX, 12);
        Car.RMotor.Status.Torque = uint2float(tBuf, T_MIN, T_MAX, 12);

    }
    else if(id == Car.LMotor.id - Car.LMotor.Mode)
    {
        int pBuf=(StatusMessage[1]<<8)|StatusMessage[2];
        int vBuf=(StatusMessage[3]<<4)|(StatusMessage[4]>>4);
        int tBuf=((StatusMessage[4]&0xF)<<8)|StatusMessage[5];
        Car.LMotor.Status.Position = uint2float(pBuf, P_MIN, P_MAX, 16);
        Car.LMotor.Status.Velocity = uint2float(vBuf, V_MIN, V_MAX, 12);
        Car.LMotor.Status.Torque = uint2float(tBuf, T_MIN, T_MAX, 12);

    }
    else return HAL_ERROR;
    return HAL_OK;

}