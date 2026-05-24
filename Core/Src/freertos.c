/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "usart.h"
#include "stdio.h"
#include "app_data.h"
#include "task_dht11.h"
#include "task_uart.h"
#include "task_oled.h"
#include "task_esp32c6.h"
#include "task_light.h"
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

/* USER CODE END Variables */
/* Definitions for SensorTask */
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
  .name = "SensorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for UartTask */
osThreadId_t UartTaskHandle;
const osThreadAttr_t UartTask_attributes = {
  .name = "UartTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for oledTask */
osThreadId_t oledTaskHandle;
const osThreadAttr_t oledTask_attributes = {
  .name = "oledTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for esp32Task */
osThreadId_t esp32TaskHandle;
const osThreadAttr_t esp32Task_attributes = {
  .name = "esp32Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for LightSensorTask */
osThreadId_t LightSensorTaskHandle;
const osThreadAttr_t LightSensorTask_attributes = {
  .name = "LightSensorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartSensorTask(void *argument);
void StartUartTask(void *argument);
void StartOledTask(void *argument);
void StartEsp32Task(void *argument);
void StartLightSensorTask(void *argument);

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
  printMutex = osMutexNew(NULL);  /* 创建打印互斥锁 */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* 传感器向串口发送的数据队列，长度10，元素大小为app_data_t */
  sensor_to_uart_Queue = osMessageQueueNew(10, sizeof(app_data_t), NULL);  
  /* 传感器向OLED发送的数据队列，长度10，元素大小为app_data_t */
  sensor_to_oled_Queue = osMessageQueueNew(10, sizeof(app_data_t), NULL);
  /* 串口向ESP32C6发送的数据队列，长度10，元素大小为app_data_t */
  uart_to_esp32_Queue = osMessageQueueNew(10, sizeof(app_data_t), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of SensorTask */
  SensorTaskHandle = osThreadNew(StartSensorTask, NULL, &SensorTask_attributes);

  /* creation of UartTask */
  UartTaskHandle = osThreadNew(StartUartTask, NULL, &UartTask_attributes);

  /* creation of oledTask */
  oledTaskHandle = osThreadNew(StartOledTask, NULL, &oledTask_attributes);

  /* creation of esp32Task */
  esp32TaskHandle = osThreadNew(StartEsp32Task, NULL, &esp32Task_attributes);

  /* creation of LightSensorTask */
  LightSensorTaskHandle = osThreadNew(StartLightSensorTask, NULL, &LightSensorTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartSensorTask */
/**
  * @brief  Function implementing the SensorTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartSensorTask */
void StartSensorTask(void *argument)
{
  /* USER CODE BEGIN StartSensorTask */
  /* Infinite loop */
  sensor_task(argument);
  /* USER CODE END StartSensorTask */
}

/* USER CODE BEGIN Header_StartUartTask */
/**
* @brief Function implementing the UartTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUartTask */
void StartUartTask(void *argument)
{
  /* USER CODE BEGIN StartUartTask */
  /* Infinite loop */
  uart_task(argument);
  /* USER CODE END StartUartTask */
}

/* USER CODE BEGIN Header_StartOledTask */
/**
* @brief Function implementing the oledTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartOledTask */
void StartOledTask(void *argument)
{
  /* USER CODE BEGIN StartOledTask */
  /* Infinite loop */
  oled_task(argument);
  /* USER CODE END StartOledTask */
}

/* USER CODE BEGIN Header_StartEsp32Task */
/**
* @brief Function implementing the esp32Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartEsp32Task */
void StartEsp32Task(void *argument)
{
  /* USER CODE BEGIN StartEsp32Task */
  /* Infinite loop */
  esp32c6_task(argument);
  /* USER CODE END StartEsp32Task */
}

/* USER CODE BEGIN Header_StartLightSensorTask */
/**
* @brief Function implementing the LightSensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLightSensorTask */
void StartLightSensorTask(void *argument)
{
  /* USER CODE BEGIN StartLightSensorTask */
  /* Infinite loop */
  light_sensor_task(argument);
  /* USER CODE END StartLightSensorTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

