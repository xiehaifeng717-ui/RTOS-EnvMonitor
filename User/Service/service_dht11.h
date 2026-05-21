/**
 * @file    service_dht11.h
 * @author  29283
 * @brief   DHT11传感器服务层头文件
 * @created 2026/4/23
 */

#ifndef _SERVICE_DHT11_H_
#define _SERVICE_DHT11_H_

#include "stdbool.h"

/* 服务层传感器数据结构体 */
typedef struct
{
    float temperature;  /* 温度数据 */
    float humidity;     /* 湿度数据 */
    bool valid;         /* 数据有效标志 */
} dht11_sample_t;

/* 函数声明 */
bool DHT11_Service_ReadSample(dht11_sample_t *sample);  /* 服务层读取传感器数据 */

#endif //_SERVICE_DHT11_H_
