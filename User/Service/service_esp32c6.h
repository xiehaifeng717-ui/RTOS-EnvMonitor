/**
 * @file service_esp32c6.h
 * @author  29283
 * @brief   ESP32C6服务层头文件
 * @created 2026/5/20
 */

#ifndef _SERVICE_ESP32C6_H_
#define _SERVICE_ESP32C6_H_

#include "bsp_esp32c6.h"
#include "app_data.h"

/* 函数声明 */
void ESP32C6_Service_SendData(const app_data_t *data);  /* 服务层ESP32C6发送数据函数 */

#endif //_SERVICE_ESP32C6_H_