/**
 * @file    task_uart.c
 * @author  29283
 * @brief   串口任务层源文件
 * @created 2026/4/23
 */

#include "task_uart.h"
#include "service_uart.h"
#include <stdio.h>

/**
 * @brief 串口任务函数
 * @param argument: 任务参数
 * @retval None
 */
void uart_task(void *argument) {
    /* 添加互斥锁, 打印任务启动调试信息 */
    osMutexAcquire(printMutex, osWaitForever);  
    printf("UART Task Started\r\n");
    osMutexRelease(printMutex);  

    app_data_t app_data;

    for(;;) {
        if(osMessageQueueGet(sensor_to_uart_Queue, &app_data, NULL, osWaitForever) == osOK) {
            UART_Service_PrintSensorData(&app_data);
        } else {
            printf("[UART Task] Failed to receive data from queue\r\n");
        }
    }
}
