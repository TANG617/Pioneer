//
// Created by 李唐 on 2023/11/29.
//

#include "NodeMotor.h"
static float uint2float(int x_int, float x_min, float x_max, int bits){
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int)*span/((float)((1<<bits)-1)) + offset;
}

static int float2uint(float x, float x_min, float x_max, int bits){
    float span = x_max - x_min;
    float offset = x_min;
    return (int) ((x-offset)*((float)((1<<bits)-1))/span);
}


HAL_StatusTypeDef NodeMotorEnable(NodeMotorType *Motor){
    uint32_t TxMailbox;
    uint8_t TxData[8] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfc};
    Motor->TxHeader.StdId = Motor->id;
    Motor->TxHeader.ExtId = 0x01;
    Motor->TxHeader.RTR = CAN_RTR_DATA;
    Motor->TxHeader.IDE = CAN_ID_STD;
    Motor->TxHeader.DLC = 8;
    Motor->TxHeader.TransmitGlobalTime = DISABLE;

    if(HAL_CAN_AddTxMessage(Motor->CanHandler, &Motor->TxHeader, TxData, &TxMailbox)==0){
        return HAL_OK;
    }
}

HAL_StatusTypeDef NodeMotorDisable(NodeMotorType *Motor){
    uint32_t TxMailbox;
    uint8_t TxData[8] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfd};
    Motor->TxHeader.StdId = Motor->id;
    Motor->TxHeader.ExtId = 0x01;
    Motor->TxHeader.RTR = CAN_RTR_DATA;
    Motor->TxHeader.IDE = CAN_ID_STD;
    Motor->TxHeader.DLC = 8;
    Motor->TxHeader.TransmitGlobalTime = DISABLE;

    if(HAL_CAN_AddTxMessage(Motor->CanHandler, &Motor->TxHeader, TxData, &TxMailbox)==0){
        return HAL_OK;
    }
}

HAL_StatusTypeDef NodeMotorSaveZero(NodeMotorType *Motor){
    uint32_t TxMailbox;
    uint8_t TxData[8] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe};
    Motor->TxHeader.StdId = Motor->id;
    Motor->TxHeader.ExtId = 0x01;
    Motor->TxHeader.RTR = CAN_RTR_DATA;
    Motor->TxHeader.IDE = CAN_ID_STD;
    Motor->TxHeader.DLC = 8;
    Motor->TxHeader.TransmitGlobalTime = DISABLE;

    if(HAL_CAN_AddTxMessage(Motor->CanHandler, &Motor->TxHeader, TxData, &TxMailbox)==0){
        return HAL_OK;
    }
}

HAL_StatusTypeDef NodeMotorClearError(NodeMotorType *Motor){
    uint32_t TxMailbox;
    uint8_t TxData[8] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfb};
    Motor->TxHeader.StdId = Motor->id;
    Motor->TxHeader.ExtId = 0x01;
    Motor->TxHeader.RTR = CAN_RTR_DATA;
    Motor->TxHeader.IDE = CAN_ID_STD;
    Motor->TxHeader.DLC = 8;
    Motor->TxHeader.TransmitGlobalTime = DISABLE;

    if(HAL_CAN_AddTxMessage(Motor->CanHandler, &Motor->TxHeader, TxData, &TxMailbox)==0){
        return HAL_OK;
    }
}


HAL_StatusTypeDef NodeMotorMITControl(NodeMotorType *Motor){
    uint32_t TxMailbox;
    uint8_t TxData[8];
    uint16_t pos_tmp,vel_tmp,kp_tmp,kd_tmp,tor_tmp;
    pos_tmp = float2uint(Motor->Position, P_MIN, P_MAX, 16);
    vel_tmp = float2uint(Motor->Velocity, V_MIN, V_MAX, 12);
    tor_tmp = float2uint(Motor->Torque, T_MIN, T_MAX, 12);
    kp_tmp = float2uint(Motor->Kp, KP_MIN, KP_MAX, 12);
    kd_tmp = float2uint(Motor->Kd, KD_MIN, KD_MAX, 12);

    TxData[0] = (pos_tmp >> 8);
    TxData[1] = pos_tmp;
    TxData[2] = (vel_tmp >> 4);
    TxData[3] = ((vel_tmp&0xF)<<4)|(kp_tmp>>8);
    TxData[4] = kp_tmp;
    TxData[5] = (kd_tmp >> 4);
    TxData[6] = ((kd_tmp&0xF)<<4)|(tor_tmp>>8);
    TxData[7] = tor_tmp;

    Motor->TxHeader.StdId = Motor->id;
    Motor->TxHeader.ExtId = 0x01;
    Motor->TxHeader.RTR = CAN_RTR_DATA;
    Motor->TxHeader.IDE = CAN_ID_STD;
    Motor->TxHeader.DLC = 8;
    Motor->TxHeader.TransmitGlobalTime = DISABLE;

    if(HAL_CAN_AddTxMessage(Motor->CanHandler, &Motor->TxHeader, TxData, &TxMailbox)==0){
        return HAL_OK;
    }
}