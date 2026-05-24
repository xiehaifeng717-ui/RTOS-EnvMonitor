/**
 * @file    service_light.c
 * @author  29283
 * @brief   光照传感器服务层源文件
 * @created 2026/5/24
 */

#include "service_light.h"
#include "bsp_light.h"
#include <stddef.h>

/**
 * @brief 读取光照传感器数据
 * @param sample: 指向存储读取数据的指针
 * @retval true=读取成功，false=读取失败
 */
bool Light_Service_ReadSample(light_sample_t *sample) {
    uint16_t adc_value;

    /* 参数检查 */
    if (sample == NULL) {
        return false;
    }

    /* 读取ADC原始值 */
    adc_value = Light_Driver_ReadADC();

    /* 填充数据 */
    sample->adc_raw = adc_value;
    sample->voltage = (float)adc_value * 3.3f / 4095.0f;
    sample->valid = true;

    return true;
}
