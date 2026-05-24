/**
 * @file    service_light.h
 * @author  29283
 * @brief   光照传感器服务层头文件
 * @created 2026/5/24
 */

#ifndef _SERVICE_LIGHT_H_
#define _SERVICE_LIGHT_H_

#include "stdint.h"
#include "stdbool.h"

/* 服务层传感器数据结构体 */
typedef struct {
    uint16_t adc_raw;   /* ADC原始值（0~4095） */
    float voltage;      /* 电压值（单位：V） */
    bool valid;         /* 数据有效标志 */
} light_sample_t;

/* 函数声明 */
bool Light_Service_ReadSample(light_sample_t *sample);  /* 服务层读取光照数据 */

#endif //_SERVICE_LIGHT_H_
