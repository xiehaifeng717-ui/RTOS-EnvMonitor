/**
 * @file bsp_esp32c6.h
 * @author  29283
 * @brief   ESP32C6驱动层头文件
 * @created 2026/5/3
 */

#ifndef _BSP_ESP32C6_H_
#define _BSP_ESP32C6_H_

#include "stdbool.h"
#include "stdint.h"

/* 函数声明 */
void ESP32C6_Driver_Init(void);                         /* ESP32C6初始化函数 */
bool ESP32C6_Driver_Send(uint8_t *data, uint16_t size); /* ESP32C6发送数据函数 */

#endif //_BSP_ESP32C6_H_
