/**
 * @file    service_dht11.c
 * @author  29283
 * @brief   串口服务层源文件
 * @created 2026/4/23
 */

#include "service_uart.h"
#include <stdio.h>

/**
 * @brief 通过串口打印传感器数据
 * @param data: 指向应用数据的指针
 * @retval None
 */
void UART_Service_PrintSensorData(const app_data_t *data) {
    /* 参数检验 */
    if(data == NULL) {
        printf("[UART Service] Invalid data pointer\r\n");
    }

    /* 打印传感器数据 */
    if(data->status) {
        /* 将浮点数转换为整数和小数部分 */
        int temperature_int = (int)(data->temperature);
        int temperature_dec = (int)((data->temperature - temperature_int) * 10);
        int humidity_int = (int)(data->humidity);
        int humidity_dec = (int)((data->humidity - humidity_int) * 10);

        printf("[UART Service] Temp=%d.%d C, Humi=%d.%d %%, Time=%lu s\r\n", 
            temperature_int, temperature_dec, humidity_int, humidity_dec, (unsigned long)data->time);
    }else {
        printf("[UART Service] Invalid sensor data\r\n");
    }
}
