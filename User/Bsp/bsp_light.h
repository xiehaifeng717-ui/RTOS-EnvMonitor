/**
 * @file    bsp_light.h
 * @author  29283
 * @brief   光照传感器驱动层头文件
 * @created 2026/5/24
 */

#ifndef _BSP_LIGHT_H_
#define _BSP_LIGHT_H_

#include "stdint.h"
#include "stdbool.h"

/*--- 函数声明 ---*/
uint16_t Light_Driver_ReadADC(void);    /* 读取ADC原始值 */
float Light_Driver_GetVoltage(void);    /* 获取电压值（单位：V） */

#endif //_BSP_LIGHT_H_
