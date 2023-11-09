#include "motion.h"

extern I2C_HandleTypeDef hi2c2;

HAL_StatusTypeDef i2c_tx(uint8_t reg,uint8_t data){
    return HAL_I2C_Mem_Write(&hi2c2,PCA9685_IIC_ADDR,reg,
                             I2C_MEMADD_SIZE_8BIT,&data,1,1000);
}

HAL_StatusTypeDef i2c_rx(uint8_t reg,uint8_t* data){
    return HAL_I2C_Mem_Read(&hi2c2,PCA9685_IIC_ADDR,reg,
                            I2C_MEMADD_SIZE_8BIT,data,1,1000);
}


static void PCA_Sleep(bool sleep){
    uint8_t mode;
    i2c_rx(PCA9685_REG_MODE1,&mode);

    if(sleep) mode |= 0x01<<PCA9685_BIT_SLEEP;
    else mode &= ~(0x01<<PCA9685_BIT_SLEEP);

    i2c_tx(PCA9685_REG_MODE1,mode);
}

static uint8_t PCA_SetFreq(float freq){

    float preScale = (float)(PCA9685_INTERNAL_OSC)/(4096.0f*freq)-1;
    if(preScale<3||preScale>255) return 1;

    PCA_Sleep(true);

    i2c_tx(PCA9685_REG_FREQ,(uint8_t)(preScale+0.5f));

    PCA_Sleep(false);

    return 0;
}

static uint8_t PCA_Output(uint8_t Channel,float DutyCycle) {
    if (DutyCycle < 0 || DutyCycle > 1) return 1;

    uint16_t High, Low;
    if (DutyCycle == 1) {
        High = 4096;
        Low = 0;
    }
    else if (DutyCycle == 0) {
        High = 0;
        Low = 4096;
    }
    else {
        High = (uint16_t) (4096.0f * 0 + 0.5f);//延时输出了0.5个pwm周期
        Low = (uint16_t) (4096.0f * DutyCycle + 0.5f);
    }

    i2c_tx(PCA9685_REG_BASE_ON_L  + Channel * 4, (uint8_t) High);
    i2c_tx(PCA9685_REG_BASE_ON_H  + Channel * 4, (uint8_t) (High >> 8));
    i2c_tx(PCA9685_REG_BASE_OFF_L + Channel * 4, (uint8_t) Low);
    i2c_tx(PCA9685_REG_BASE_OFF_H + Channel * 4, (uint8_t) (Low >> 8));

    return 0;
}

static void PCA_Init(float freq){
    i2c_tx(PCA9685_REG_MODE1,0x00);
    PCA_SetFreq(freq);
    PCA_Output(0,1.0f);//限流A4950
}

static void EncoderInit(MotionNode *Car){
    // eA1.timer = EA1;
    HAL_TIM_Encoder_Start(Car->LeftFrontEncoder.Timer,TIM_CHANNEL_1);
    HAL_TIM_Encoder_Start(Car->LeftFrontEncoder.Timer,TIM_CHANNEL_2);
    Car->LeftFrontEncoder.Round=0;
    Car->LeftFrontEncoder.Position=0;
    Car->LeftFrontEncoder.Speed=0;
   __HAL_TIM_SetCounter(Car->LeftFrontEncoder.Timer,0);
   __HAL_TIM_ENABLE_IT(Car->LeftFrontEncoder.Timer,TIM_IT_UPDATE);
    // FS_Debug("EA1 init\r\n");

    HAL_TIM_Encoder_Start(Car->LeftRearEncoder.Timer, TIM_CHANNEL_1);
    HAL_TIM_Encoder_Start(Car->LeftRearEncoder.Timer, TIM_CHANNEL_2);
    Car->LeftRearEncoder.Round = 0;
    Car->LeftRearEncoder.Position = 0;
    Car->LeftRearEncoder.Speed = 0;
    __HAL_TIM_SetCounter(Car->LeftRearEncoder.Timer, 0);
    __HAL_TIM_ENABLE_IT(Car->LeftRearEncoder.Timer, TIM_IT_UPDATE);


    HAL_TIM_Encoder_Start(Car->RightFrontEncoder.Timer, TIM_CHANNEL_1);
    HAL_TIM_Encoder_Start(Car->RightFrontEncoder.Timer, TIM_CHANNEL_2);
    Car->RightFrontEncoder.Round = 0;
    Car->RightFrontEncoder.Position = 0;
    Car->RightFrontEncoder.Speed = 0;
    __HAL_TIM_SetCounter(Car->RightFrontEncoder.Timer, 0);
    __HAL_TIM_ENABLE_IT(Car->RightFrontEncoder.Timer, TIM_IT_UPDATE);

    HAL_TIM_Encoder_Start(Car->RightRearEncoder.Timer, TIM_CHANNEL_1);
    HAL_TIM_Encoder_Start(Car->RightRearEncoder.Timer, TIM_CHANNEL_2);
    Car->RightRearEncoder.Round = 0;
    Car->RightRearEncoder.Position = 0;
    Car->RightRearEncoder.Speed = 0;
    __HAL_TIM_SetCounter(Car->RightRearEncoder.Timer, 0);
    __HAL_TIM_ENABLE_IT(Car->RightRearEncoder.Timer, TIM_IT_UPDATE);

}

void EncoderUpdate(EncoderNode *Encoder){
    uint16_t Pulse = __HAL_TIM_GetCounter(Encoder->Timer);
    Encoder->Position += Pulse/(REDUCE*PULSES)*0.17;
    Encoder->Speed = Pulse/(INTERVAL)*0.17;
    __HAL_TIM_SetCounter(Encoder->Timer,0);
}

MotionNode* MotionInit(){
    //Define the Pin
    MotionNode PioneerCar;
    PioneerCar.LeftFrontMotor.Forward = MotorLeftFrontForward;
    PioneerCar.LeftFrontMotor.Backward = MotorLeftFrontBackward;
    PioneerCar.LeftRearMotor.Forward = MotorLeftRearForward;
    PioneerCar.LeftRearMotor.Backward = MotorLeftRearBackward;
    PioneerCar.RightFrontMotor.Forward = MotorRightFrontForward;
    PioneerCar.RightFrontMotor.Backward = MotorRightFrontBackward;
    PioneerCar.RightRearMotor.Forward = MotorRightRearForward;
    PioneerCar.RightRearMotor.Backward = MotorRightRearBackward;

    PioneerCar.LeftFrontMotor.Speed = 0.0f;
    PioneerCar.LeftFrontMotor.Position = 0.0f;
    PioneerCar.LeftRearMotor.Speed = 0.0f;
    PioneerCar.LeftRearMotor.Position = 0.0f;
    PioneerCar.RightFrontMotor.Speed = 0.0f;
    PioneerCar.RightFrontMotor.Position = 0.0f;
    PioneerCar.RightRearMotor.Speed = 0.0f;
    PioneerCar.RightRearMotor.Position = 0.0f;

    PioneerCar.Speed = 0.0f;
    PioneerCar.Position = 0.0f;

    //Init PCA9685
    PCA_Init(1500);

    //Init Encoder
    EncoderInit(&PioneerCar);

    return &PioneerCar;

}

void MotorSetSpeed(MotorNode *Motor, float targetSpeed){
    Motor->Speed = fabsf(targetSpeed)>1.0f ? 1.0f:targetSpeed;
}

void AdvanceIV(MotionNode *Car){
    MotorSetSpeed(&Car->LeftFrontMotor,1.0f);
    MotorSetSpeed(&Car->LeftRearMotor,1.0f);
    MotorSetSpeed(&Car->RightFrontMotor,1.0f);
    MotorSetSpeed(&Car->RightRearMotor,1.0f);
    MotionUpdateEncoder(&Car);
}

void MotionUpdateEncoder(MotionNode *Car){
    EncoderUpdate(&Car->LeftFrontEncoder);
    EncoderUpdate(&Car->LeftRearEncoder);
    EncoderUpdate(&Car->RightFrontEncoder);
    EncoderUpdate(&Car->RightRearEncoder);
}

void MotionUpdateSpeed(MotionNode *Car){
    PCA_Output(Car->LeftFrontMotor.Speed > 0.0f ? Car->LeftFrontMotor.Forward : Car->LeftFrontMotor.Backward, fabsf(Car->LeftFrontMotor.Speed));
    PCA_Output(Car->LeftRearMotor.Speed > 0.0f ? Car->LeftRearMotor.Forward : Car->LeftRearMotor.Backward, fabsf(Car->LeftRearMotor.Speed));
    PCA_Output(Car->RightFrontMotor.Speed > 0.0f ? Car->RightFrontMotor.Forward : Car->RightFrontMotor.Backward, fabsf(Car->RightFrontMotor.Speed));
    PCA_Output(Car->RightRearMotor.Speed > 0.0f ? Car->RightRearMotor.Forward : Car->RightRearMotor.Backward, fabsf(Car->RightRearMotor.Speed));
}


