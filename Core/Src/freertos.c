/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Lib/LCD.h"
#include "Lib/UART.h"
#include "Lib/MPU6050/MPU6050.h"
#include "Lib/NodeMotor.h"

#include "App/DualSenseController.h"
#include "App/Display.h"
#include "App/AGVMotion.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
int16_t DSC_DATA[6];
//NodeMotorType NodeMotor1, NodeMotor2;
AGV_MotionType AGV_Car;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GetMessageTask */
osThreadId_t GetMessageTaskHandle;
const osThreadAttr_t GetMessageTask_attributes = {
  .name = "GetMessageTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for ControlTask */
osThreadId_t ControlTaskHandle;
const osThreadAttr_t ControlTask_attributes = {
  .name = "ControlTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DisplayTask */
osThreadId_t DisplayTaskHandle;
const osThreadAttr_t DisplayTask_attributes = {
  .name = "DisplayTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void startGetMessageTask(void *argument);
void startControlTask(void *argument);
void startDisplayTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of GetMessageTask */
  GetMessageTaskHandle = osThreadNew(startGetMessageTask, NULL, &GetMessageTask_attributes);

  /* creation of ControlTask */
  ControlTaskHandle = osThreadNew(startControlTask, NULL, &ControlTask_attributes);

  /* creation of DisplayTask */
  DisplayTaskHandle = osThreadNew(startDisplayTask, NULL, &DisplayTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
//    osDelay(1);
      osDelay(1000);
      HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_startGetMessageTask */
/**
* @brief Function implementing the GetMessageTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startGetMessageTask */
void startGetMessageTask(void *argument)
{
  /* USER CODE BEGIN startGetMessageTask */

  /* Infinite loop */
  for(;;)
  {
      DSC_GET(DSC_DATA);
      float DSC_Velocity = DSC_DATA[LStickY]/127.0*5.0;
      float DSC_Rotate = DSC_DATA[LStickX]/127.0*5;
      if(DSC_Velocity > 1 || DSC_Velocity < - 1)
      {
          AGV_MotionUpdate(&AGV_Car,DSC_Velocity,DSC_Rotate);
      }
      else{
          AGV_MotionUpdate(&AGV_Car,0,DSC_Rotate);
      }
      osDelay(100);

  }
  /* USER CODE END startGetMessageTask */
}

/* USER CODE BEGIN Header_startControlTask */
/**
* @brief Function implementing the ControlTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startControlTask */
void startControlTask(void *argument)
{
  /* USER CODE BEGIN startControlTask */

//    NodeMotor1.CanHandler = &hcan;
//    NodeMotor1.id = 0x01;
//    NodeMotor1.Mode = Velocity;
//    NodeMotor1.Velocity = 0;//nagative
//
//    NodeMotor2.CanHandler = &hcan;
//    NodeMotor2.id = 0x02;
//    NodeMotor2.Mode = Velocity;
//    NodeMotor2.Velocity = 0;//positive


//    NodeMotorEnable(&NodeMotor1);
//    osDelay(100);
//    NodeMotorEnable(&NodeMotor2);
    osDelay(1000);
    AGV_MotionInit(&AGV_Car);
  /* Infinite loop */
  for(;;)
  {
    osDelay(100);
//      AGV_MotionInit(&AGV_Car);
      AGV_MotionMove(&AGV_Car);
//      NodeMotorVelocityControl(&NodeMotor1);
//      NodeMotorVelocityControl(&NodeMotor2);
  }
  /* USER CODE END startControlTask */
}

/* USER CODE BEGIN Header_startDisplayTask */
/**
* @brief Function implementing the DisplayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startDisplayTask */
void startDisplayTask(void *argument)
{
  /* USER CODE BEGIN startDisplayTask */
    LCD_Init();
    LCD_Clear(BLACK);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);

  }
  /* USER CODE END startDisplayTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

