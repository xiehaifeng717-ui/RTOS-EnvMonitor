/**
 * @file    app_data.h
 * @author  29283
 * @brief   应用数据结构定义头文件
 * @created 2026/4/23
 */

#ifndef _APP_DATA_H_
#define _APP_DATA_H_

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os2.h"

/* 传感器原始数据结构体 */
typedef struct {
    uint8_t humi_int;   /* 湿度整数部分 */
    uint8_t humi_dec;   /* 湿度小数部分 */
    uint8_t temp_int;   /* 温度整数部分 */
    uint8_t temp_dec;   /* 温度小数部分 */
} dht11_data_t;

/* 传感器数据全局结构体 */
typedef struct {
    float temperature;  /* 温度数据 */
    float humidity;     /* 湿度数据 */
    uint16_t light_adc; /* 光照ADC原始值 */
    uint8_t blue_led;   /* 蓝灯状态（光照自动控制）0=灭，1=亮 */
    uint8_t green_led;  /* 绿灯状态（按键手动控制）0=灭，1=亮 */
    int time;           /* 时间戳 */
    int key_count;      /* 按键计数 */
    bool status;        /* 状态标志 */
} app_data_t;

/* ESP32C6上报协议帧 */
typedef struct {
    uint8_t header[2];      /* 帧头 */
    int16_t temperature;    /* 温度 */
    int16_t humidity;       /* 湿度 */
    uint8_t checksum;       /* 校验和 */
} __attribute__((packed)) esp_data_t;

/* 全局变量 */
extern osMessageQueueId_t sensor_to_uart_Queue;  /* 传感器向串口发送的数据队列 */
extern osMessageQueueId_t sensor_to_oled_Queue;  /* 传感器向OLED发送的数据队列 */
extern osMessageQueueId_t uart_to_esp32_Queue;   /* 串口向ESP32C6发送的数据队列 */
extern osMutexId_t printMutex;                   /* 打印互斥锁 */

/* 多任务共享数据区：LightTask写入，SensorTask读取后统一发队列 */
extern app_data_t g_sensor_share;

#endif //_APP_DATA_H_
