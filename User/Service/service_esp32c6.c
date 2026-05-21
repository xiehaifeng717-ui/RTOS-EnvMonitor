/**
 * @file service_esp32c6.c
 * @author  29283
 * @brief   ESP32C6服务层源文件
 * @created 2026/5/20
 */

#include "service_esp32c6.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief 服务层ESP32C6发送数据函数
 * @param data: 指向应用数据的指针
 * @retval None
 */
void ESP32C6_Service_SendData(const app_data_t *data) {
    /* 参数检验 */
    if(data == NULL) {
        printf("[ESP32C6 Service] Invalid data pointer\r\n");
        return;
    }

    /* 发送缓冲区 */
    char buffer[64];  

    /* 构建发送数据字符串 */
    snprintf(buffer, sizeof(buffer), "Temp:%.1fC,Humi:%.1f%%\r\n", data->temperature, data->humidity);

    /* 发送数据到ESP32C6 */
    if(ESP32C6_Driver_Send((uint8_t *)buffer, strlen(buffer)) == false) {
        printf("[ESP32C6 Service] Failed to send data\r\n");
    }
}