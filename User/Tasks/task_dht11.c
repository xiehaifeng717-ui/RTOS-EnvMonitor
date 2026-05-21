/**
 * @file    task_dht11.c
 * @author  29283
 * @brief   DHT11传感器任务层源文件
 * @created 2026/4/23
 */

#include "task_dht11.h"
#include "service_dht11.h"
#include "app_data.h"
#include "cmsis_os2.h"
#include <stdio.h>


/**
 * @brief DHT11传感器任务函数
 * @param argument: 任务参数
 * @retval None
 */
void sensor_task(void *argument) {
    /* 添加互斥锁, 打印调试任务启动信息 */
    osMutexAcquire(printMutex, osWaitForever);  
    printf("Sensor Task Started\r\n");
    osMutexRelease(printMutex);  

    /* 定义传感器数据结构 */
    dht11_sample_t dht11_sample;    /* 服务层 */
    app_data_t app_data;            /* 应用层 */

    for(;;) {
        /* 清空应用层数据结构 */
        app_data = (app_data_t){0};  

        /* 读取传感器数据 */
        if (DHT11_Service_ReadSample(&dht11_sample)) {
            /* 数据有效，更新应用层数据结构 */
            app_data.temperature = dht11_sample.temperature;
            app_data.humidity = dht11_sample.humidity;
            /* 数据有效标志 */
            app_data.time = osKernelGetTickCount() / 1000;  
            app_data.status = true; 
        } else {
            app_data.temperature = 0.0f;
            app_data.humidity = 0.0f;
            /* 数据无效，设置状态标志 */
            app_data.status = false;
        }

        /* 将数据发送到串口任务 */
        osMessageQueuePut(sensor_to_uart_Queue, &app_data, 0, 0);
        /* 将数据发送到OLED任务 */
        osMessageQueuePut(sensor_to_oled_Queue, &app_data, 0, 0);
        /* 将数据发送到ESP32C6任务 */
        osMessageQueuePut(uart_to_esp32_Queue, &app_data, 0, 0);

        /* 延时1秒后再次读取 */
        osDelay(1000);
    }
}
