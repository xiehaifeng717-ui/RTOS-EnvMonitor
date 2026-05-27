/**
 * @file task_esp32c6.c
 * @brief ESP32C6任务源文件
 * @author  29283
 * @created 2026/5/20
 */

#include "task_esp32c6.h"
#include "service_esp32c6.h"
#include "service_command.h"
#include "bsp_esp32c6.h"
#include <stdio.h>
#include "app_data.h"

/**
 * @brief ESP32C6任务函数
 * @param argument: 任务参数
 * @retval None
 */
void esp32c6_task(void *argument) {
    app_data_t app_data;
    char cmd_buf[64];

    for (;;) {
        /* 等待发送队列（500ms 超时，以便同时轮询下行命令） */
        if (osMessageQueueGet(uart_to_esp32_Queue, &app_data, NULL, 500) == osOK) {
            ESP32C6_Service_SendData(&app_data);
        }

        /* 检查是否有来自 ESP32 的下行命令 */
        if (ESP32C6_Driver_GetCommand(cmd_buf, sizeof(cmd_buf))) {
            Command_Service_Process(cmd_buf);
        }
    }
}
