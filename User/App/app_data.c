/**
 * @file    app_data.c
 * @author  29283
 * @brief   应用数据结构定义源文件
 * @created 2026/4/23
 */

#include "app_data.h"

/* 多任务共享数据区：LightTask写入传感器/LED状态，SensorTask读取后统一发送 */
app_data_t g_sensor_share = {0};

/* 消息队列和互斥锁定义（CubeMX生成的freertos.c中未包含，在此处定义） */
osMessageQueueId_t sensor_to_uart_Queue;   /* 传感器向串口发送的数据队列 */
osMessageQueueId_t sensor_to_oled_Queue;   /* 传感器向OLED发送的数据队列 */
osMessageQueueId_t uart_to_esp32_Queue;    /* 串口向ESP32C6发送的数据队列 */
osMutexId_t printMutex;                    /* 打印互斥锁 */
