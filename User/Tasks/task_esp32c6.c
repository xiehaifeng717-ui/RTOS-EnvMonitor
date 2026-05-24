/**
 * @file task_esp32c6.h
 * @brief ESP32C6任务源文件
 * @author  29283
 * @created 2026/5/20
 */

#include "task_esp32c6.h"
#include "service_esp32c6.h"
#include <stdio.h>
#include "app_data.h"

/**
 * @brief ESP32C6任务函数
 * @param argument: 任务参数
 * @retval None
 */
void esp32c6_task(void *argument) {
    app_data_t app_data;

    for(;;) {
        if(osMessageQueueGet(uart_to_esp32_Queue, &app_data, NULL, osWaitForever) == osOK) {
            ESP32C6_Service_SendData(&app_data);
        } else {
            printf("[ESP32C6 Task] Failed to receive data from queue\r\n");
        }
    }
}