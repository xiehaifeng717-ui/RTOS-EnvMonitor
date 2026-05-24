/**
 * @file service_uart.h
 * @author  29283
 * @brief   串口服务层头文件
 * @created 2026/4/23
 */

#ifndef _SERVICE_UART_H_
#define _SERVICE_UART_H_

#include "app_data.h"

/* 函数声明 */
void UART_Service_PrintSensorData(const app_data_t *data);  /* 服务层串口打印传感器数据函数 */

#endif //_SERVICE_UART_H_