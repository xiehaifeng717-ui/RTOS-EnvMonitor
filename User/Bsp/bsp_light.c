/**
 * @file    bsp_light.c
 * @author  29283
 * @brief   光照传感器驱动层源文件
 * @created 2026/5/24
 */

#include "bsp_light.h"
#include "stm32f1xx_hal.h"

/* ADC句柄 */
extern ADC_HandleTypeDef hadc1;

/**
 * @brief 读取ADC原始值
 * @param None
 * @retval ADC转换结果（0~4095）
 */
uint16_t Light_Driver_ReadADC(void) {
    uint16_t adc_value = 0;

    /* 启动ADC转换 */
    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return 0;
    }

    /* 等待转换完成，超时100ms */
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
        adc_value = HAL_ADC_GetValue(&hadc1);
    }

    /* 停止ADC */
    HAL_ADC_Stop(&hadc1);

    return adc_value;
}

/**
 * @brief 获取ADC电压值
 * @param None
 * @retval 电压值（单位：V）
 * @note STM32F103参考电压3.3V，ADC为12位（0~4095）
 *       计算公式：电压 = ADC值 × 3.3 / 4095
 */
float Light_Driver_GetVoltage(void) {
    uint16_t adc_value = Light_Driver_ReadADC();
    float voltage = (float)adc_value * 3.3f / 4095.0f;
    return voltage;
}
