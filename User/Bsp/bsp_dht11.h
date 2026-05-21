/**
 * @file    bsp_dht11.h
 * @author  29283
 * @brief   DHT11传感器底层驱动头文件
 * @created 2026/4/23
 */

#ifndef _BSP_DHT11_H_
#define _BSP_DHT11_H_

#include "stdbool.h"
#include "stdint.h"

/* 传感器原始数据结构 */
typedef struct
{
    uint8_t humi_int;   /* 湿度整数部分 */
    uint8_t humi_dec;   /* 湿度小数部分 */
    uint8_t temp_int;   /* 温度整数部分 */
    uint8_t temp_dec;   /* 温度小数部分 */
} dht11_raw_data_t;

/* 函数声明 */
void DHT11_Driver_SetPinOutput(void);             /* 设置DHT11数据引脚为输出模式 */
void DHT11_Driver_SetPinInput(void);              /* 设置DHT11数据引脚为输入模式 */
void DHT11_Driver_Start(void);                    /* 发送DHT11启动信号 */
bool DHT11_Driver_CheckResponse(void);            /* 检测DHT11响应 */
bool DHT11_Driver_ReadByte(uint8_t *data);        /* 读取DHT11一个字节 */
bool DHT11_Driver_ReadRaw(dht11_raw_data_t *raw); /* 读取DHT11数据 */

#endif //_BSP_DHT11_H_
