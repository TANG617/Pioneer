//
// Created by 李唐 on 2023/11/13.
//

#include "App/Display.h"
#include "Lib/LCD_Image.h"
HAL_StatusTypeDef DisplayNum(float num)
{
    LCD_ShowNum(0, 0, num*10000, 6, 32);
    return HAL_OK;
}

HAL_StatusTypeDef ShowHelloWorld()
{
    LCD_Show_Image(0,0,240,240,gImage_homer);
}

HAL_StatusTypeDef ShowWheelStatus(MotionType *_Car)
{
    LCD_ShowNum(10,10,_Car->LeftFrot.Encoder.RadVelocity*100,6,24);
    LCD_ShowNum(10,40,_Car->LeftFrot.Motor.RadVelocity*100,6,24);
    LCD_ShowNum(10,70,_Car->LeftFrot.Parameter.PID*100,6,24);

    LCD_ShowNum(140,10,_Car->RightFrot.Encoder.RadVelocity*100,6,24);
    LCD_ShowNum(140,40,_Car->RightFrot.Motor.RadVelocity*100,6,24);
    LCD_ShowNum(140,70,_Car->RightFrot.Parameter.PID*100,6,24);

    LCD_ShowNum(10,150,_Car->LeftRear.Encoder.RadVelocity*100,6,24);
    LCD_ShowNum(10,180,_Car->LeftRear.Motor.RadVelocity*100,6,24);
    LCD_ShowNum(10,210,_Car->LeftRear.Parameter.PID*100,6,24);

    LCD_ShowNum(140,150,_Car->RightRear.Encoder.RadVelocity*100,6,24);
    LCD_ShowNum(140,180,_Car->RightRear.Motor.RadVelocity*100,6,24);
    LCD_ShowNum(140,210,_Car->RightRear.Parameter.PID*100,6,24);
}

HAL_StatusTypeDef ShowDSCStatus(int16_t *Data)
{
    LCD_ShowNum(10,10,Data[LStickX],6,24);
    LCD_ShowNum(10,40,Data[LStickY],6,24);
    LCD_ShowNum(10,70,Data[RStickX],6,24);
    LCD_ShowNum(10,100,Data[RStickY],6,24);
    LCD_ShowNum(10,130,Data[L2Value],6,24);
    LCD_ShowNum(10,160,Data[R2Value],6,24);

    // LCD_ShowNum(10,70,_Car->LeftFrot.Parameter.PID*100,6,24);

}

HAL_StatusTypeDef ShowIMU()
{

    LCD_ShowNum(10,10,mpu.accX*100,6,24);
    LCD_ShowNum(10,50,mpu.accY*100,6,24);
    LCD_ShowNum(10,90,mpu.accZ*100,6,24);

    LCD_ShowNum(100,10,mpu.gyrX*100,6,24);
    LCD_ShowNum(100,50,mpu.gyrY*100,6,24);
    LCD_ShowNum(100,90,mpu.gyrZ*100,6,24);

    LCD_ShowNum(190,10,mpu.yaw*100,6,24);
    LCD_ShowNum(190,50,mpu.pitch*100,6,24);
    LCD_ShowNum(190,90,mpu.roll*100,6,24);
}