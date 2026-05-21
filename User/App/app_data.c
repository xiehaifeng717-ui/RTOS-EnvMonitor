/**
 * @file    app_data.c
 * @author  29283
 * @brief   应用数据结构定义源文件
 * @created 2026/4/23
 */

#include "app_data.h"
#include <stdbool.h>
#include <stdint.h>

/* 全局变量定义 */
app_data_t app_data = {
    .temperature = 0.0f,   /* 温度 */
    .humidity = 0.0f,      /* 湿度 */
    .time = 0,             /* 时间 */
    .key_count = 0,        /* 按键次数 */
    .status = false,       /* 状态 */
};

/* 全局变量定义 */
osMessageQueueId_t sensor_to_uart_Queue = NULL;
osMessageQueueId_t sensor_to_oled_Queue = NULL;
osMessageQueueId_t uart_to_esp32_Queue = NULL;
osMutexId_t printMutex = NULL;
