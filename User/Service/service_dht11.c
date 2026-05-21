/**
 * @file    service_dht11.c
 * @author  29283
 * @brief   DHT11传感器服务层源文件
 * @created 2026/4/23
 */

#include "service_dht11.h"
#include "bsp_dht11.h"
#include <stddef.h>
#include <stdio.h>

/**
 * @brief 读取DHT11传感器数据
 * @param sample: 指向存储读取数据的指针
 * @retval None
 */
bool DHT11_Service_ReadSample(dht11_sample_t *sample) {
    dht11_raw_data_t raw;   /* 传感器原始数据 */

    /* 参数检查 */
    if (sample == NULL) {
        return false;
    }

    /* 读取原始数据 */
    if (!DHT11_Driver_ReadRaw(&raw)) {
        sample->temperature = 0.0f;
        sample->humidity = 0.0f;
        sample->valid = false;
        return false;
    }

    /* 转换数据 */
    sample->temperature = (float)raw.temp_int + (float)raw.temp_dec / 10.0f;
    sample->humidity = (float)raw.humi_int + (float)raw.humi_dec / 10.0f;
    sample->valid = true;

    return true;
}

