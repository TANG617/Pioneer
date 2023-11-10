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

///////////////////////PCA9685
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

///////////////////////ENCODER
static void EncoderInit(MotionNode *Car, TIM_HandleTypeDef* LF, TIM_HandleTypeDef* LR, TIM_HandleTypeDef* RF, TIM_HandleTypeDef* RR){
    Car->LeftFrontEncoder.Timer = LF;
    Car->LeftRearEncoder.Timer = LR;
    Car->RightFrontEncoder.Timer = RF;
    Car->RightRearEncoder.Timer = RR;
    
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

static void EncoderUpdate(EncoderNode *Encoder){
    uint16_t Pulse = __HAL_TIM_GetCounter(Encoder->Timer);
    Encoder->Position += Pulse/(REDUCE*PULSES)*0.17;
    Encoder->Speed = Pulse/(INTERVAL)*0.17;
    __HAL_TIM_SetCounter(Encoder->Timer,0);
}

///////////////////////MOTOR
static void MotorInit(MotorNode *_Motor, uint8_t setForwardChannel, uint8_t setBackwardChannel){
    _Motor->Forward = setForwardChannel;
    _Motor->Backward = setBackwardChannel;
    _Motor->Parameter.Angel.Kp = KP;
    _Motor->Parameter.Angel.Ki = KI;
    _Motor->Parameter.Velocity.Kp = KP;
    _Motor->Parameter.Velocity.Ki = KI;
    _Motor->Position = 0.0f;
    _Motor->Speed = 0.0f;
}

void MotionInit(MotionNode *_Car, TIM_HandleTypeDef* LF, TIM_HandleTypeDef* LR, TIM_HandleTypeDef* RF, TIM_HandleTypeDef* RR){
    //Define the Pin
    MotorInit(&_Car->LeftFrontMotor,MotorLeftFrontForward,MotorLeftFrontBackward);
    MotorInit(&_Car->LeftRearMotor,MotorLeftRearForward,MotorLeftRearBackward);
    MotorInit(&_Car->RightFrontMotor,MotorRightFrontForward,MotorRightFrontBackward);
    MotorInit(&_Car->RightRearMotor,MotorRightRearForward,MotorRightRearBackward);

    _Car->Speed = 0.0f;
    _Car->Position = 0.0f;

    //Init PCA9685
    PCA_Init(1500);

    //Init Encoder
    EncoderInit(_Car,LF,LR,RF,RR);

}

static void MotorSetSpeed(MotorNode *_Motor, float targetSpeed){
    _Motor->Speed = targetSpeed;
}

static void MotorOutputSpeed(MotorNode *_Motor){
    if(_Motor->Speed > 0){
        PCA_Output(_Motor->Backward,0.0f);
        PCA_Output(_Motor->Forward,fabs(_Motor->Speed));
    }
    else{
        PCA_Output(_Motor->Forward,0.0f);
        PCA_Output(_Motor->Backward,fabs(_Motor->Speed));
    }
    
}

static void MotionUpdateSpeed(MotionNode *_Car){
    MotorOutputSpeed(&_Car->LeftFrontMotor);
    MotorOutputSpeed(&_Car->LeftRearMotor);
    MotorOutputSpeed(&_Car->RightFrontMotor);
    MotorOutputSpeed(&_Car->RightRearMotor);

    EncoderUpdate(&_Car->LeftFrontEncoder);
    EncoderUpdate(&_Car->LeftRearEncoder);
    EncoderUpdate(&_Car->RightFrontEncoder);
    EncoderUpdate(&_Car->RightRearEncoder);
}

// static void MotorVolocityLoop(MotionNode *_Motor){
    
// }

///////////////////////APP
void AdvanceIV(MotionNode *_Car){
    MotorSetSpeed(&_Car->LeftFrontMotor,1.0f);
    MotorSetSpeed(&_Car->LeftRearMotor,1.0f);
    MotorSetSpeed(&_Car->RightFrontMotor,1.0f);
    MotorSetSpeed(&_Car->RightRearMotor,1.0f);
    MotionUpdateSpeed(_Car);
}


void MotionTest(MotionNode *_Car){
    MotorSetSpeed(&_Car->LeftFrontMotor,1.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotorSetSpeed(&_Car->LeftFrontMotor,0.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotorSetSpeed(&_Car->LeftFrontMotor,-1.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotorSetSpeed(&_Car->LeftFrontMotor,0.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);

    MotorSetSpeed(&_Car->LeftRearMotor,1.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotorSetSpeed(&_Car->LeftRearMotor,0.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotorSetSpeed(&_Car->LeftRearMotor,-1.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotorSetSpeed(&_Car->LeftRearMotor,0.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);

    MotorSetSpeed(&_Car->RightFrontMotor,1.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotorSetSpeed(&_Car->RightFrontMotor,0.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotorSetSpeed(&_Car->RightFrontMotor,-1.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotorSetSpeed(&_Car->RightFrontMotor,0.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);

    MotorSetSpeed(&_Car->RightRearMotor,1.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotorSetSpeed(&_Car->RightRearMotor,0.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotorSetSpeed(&_Car->RightRearMotor,-1.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotorSetSpeed(&_Car->RightRearMotor,0.0f);
    MotionUpdateSpeed(_Car);
    HAL_Delay(1000);
    MotionUpdateSpeed(_Car);
}

//////////////////


void MotionMoveRad(MotionNode *_Car, float DirectionRad, float Speed){
    float Xaxis, Yaxis;
    Xaxis = Speed * cosf(DirectionRad);
    Yaxis = Speed * sinf(DirectionRad);

    MotorSetSpeed(&_Car->LeftFrontMotor,Yaxis-Xaxis);
    MotorSetSpeed(&_Car->LeftRearMotor,Yaxis+Xaxis);
    MotorSetSpeed(&_Car->RightFrontMotor,Yaxis+Xaxis);
    MotorSetSpeed(&_Car->RightRearMotor,Yaxis-Xaxis);

    MotionUpdateSpeed(_Car);
}

void MotionMoveAng(MotionNode *_Car, float DirectionAngel, float Speed){
    float Xaxis, Yaxis;
    Xaxis = Speed * cosf(DirectionAngel / 360.0 * 2 * 3.14);
    Yaxis = Speed * sinf(DirectionAngel / 360.0 * 2 * 3.14);

    MotorSetSpeed(&_Car->LeftFrontMotor,Yaxis-Xaxis);
    MotorSetSpeed(&_Car->LeftRearMotor,Yaxis+Xaxis);
    MotorSetSpeed(&_Car->RightFrontMotor,Yaxis+Xaxis);
    MotorSetSpeed(&_Car->RightRearMotor,Yaxis-Xaxis);

    MotionUpdateSpeed(_Car);
}

void MotionMoveInt(MotionNode *_Car, int X_intensity, int Y_intensity){
    float Xaxis, Yaxis;
    float Speed;
    float DirectionAngel;
    DirectionAngel = atanf(Y_intensity/X_intensity);
    Speed = sqrtf(2) * sqrtf(X_intensity * X_intensity + Y_intensity * Y_intensity) / 2 ;
    Xaxis = Speed * cosf(DirectionAngel / 360.0 * 2 * 3.14);
    Yaxis = Speed * sinf(DirectionAngel / 360.0 * 2 * 3.14);

    MotorSetSpeed(&_Car->LeftFrontMotor,Yaxis-Xaxis);
    MotorSetSpeed(&_Car->LeftRearMotor,Yaxis+Xaxis);
    MotorSetSpeed(&_Car->RightFrontMotor,Yaxis+Xaxis);
    MotorSetSpeed(&_Car->RightRearMotor,Yaxis-Xaxis);

    MotionUpdateSpeed(_Car);

}