/**
 * @file    bsp_dht11.c
 * @author  29283
 * @brief   DHT11传感器底层驱动源文件
 * @created 2026/4/23
 */

#include "bsp_dht11.h"
#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "delay.h"
#include <stddef.h>
#include <stdio.h>

/**
 * @brief 设置DHT11数据引脚为输出模式
 * @param None
 * @retval None
 */
void DHT11_Driver_SetPinOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_DATA_Pin;           /* DHT11数据引脚 */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;     /* 推挽输出模式 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;    /* 低速 */

    HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStruct);  /* 初始化引脚为输出模式，准备发送启动信号 */
}

/**
 * @brief 设置DHT11数据引脚为输入模式
 * @param None
 * @retval None
 */
void DHT11_Driver_SetPinInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_DATA_Pin;           /* DHT11数据引脚 */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;         /* 输入模式 */
    GPIO_InitStruct.Pull = GPIO_NOPULL;             /* 无上下拉 */

    HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStruct);  /* 初始化引脚为输入模式，等待DHT11响应 */
}

/**
 * @brief DHT11启动信号
 * @param None
 * @retval None
 */
void DHT11_Driver_Start(void) {
    DHT11_Driver_SetPinOutput();  /* 设置为输出模式 */  

    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_RESET);   
    HAL_Delay(20);          /* 拉低至少18ms */

    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET);
    delay_us(30);           /* 拉高20-40us */

    DHT11_Driver_SetPinInput();   /* 设置为输入模式，等待DHT11响应 */
}

/**
 * @brief 检测DHT11响应
 * @param None
 * @retval None
 */
bool DHT11_Driver_CheckResponse(void) {
    uint8_t timeout = 0;

    while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_SET) {
        delay_us(1);
        if (++timeout > 100) {   /* 等待DHT11拉低响应信号，最长等待100us */
            return false;
        }
    }
    timeout = 0;

    while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_RESET) {
        delay_us(1);
        if (++timeout > 100) {   /* 等待DHT11拉高响应信号，最长等待100us */
            return false;
        }
    }
    timeout = 0;

    while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_SET) {
        delay_us(1);
        if (++timeout > 100) {   /* 等待DHT11拉低准备发送数据，最长等待100us */
            return false;
        }
    }

    return true;
}

/**
 * @brief 读取DHT11一个字节
 * @param data: 指向存储读取数据的指针
 * @retval None
 */
bool DHT11_Driver_ReadByte(uint8_t *data) {
    /* 检查指针是否为空 */
    if (data == NULL) { 
        return false;
    }

    uint8_t i;
    uint8_t result = 0;
    uint16_t timeout;

    for (i = 0; i < 8; i++) {
        timeout = 0;

        while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_RESET) {
            delay_us(1);
            if (++timeout > 100) {    /* 等待数据线拉高，最长等待100us */
                return false;
            }
        }

        /* 延时40us在数据中间采样 */
        delay_us(40);

        /* 读取到1，设置对应位 */
        if (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin)) {
            result |= (1 << (7 - i));
        }

        timeout = 0;
        while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_SET) {
            delay_us(1);
            if (++timeout > 100) {  /* 等待数据线拉低，最长等待100us */
                return false;
            }
        }
    }

    *data = result;
    return true;
}

/**
 * @brief 读取DHT11数据
 * @param raw: 指向存储读取数据的指针
 * @retval None
 */
bool DHT11_Driver_ReadRaw(dht11_raw_data_t *raw)
{
    /* 检查指针是否为空 */
    if(raw == NULL) {
        return false;
    }

    uint8_t buffer[5];
    DHT11_Driver_Start();

    /* 检查DHT11响应 */
    if (!DHT11_Driver_CheckResponse()) {
        return false;
    }

    /* 读取5个字节的数据 */
    for (int i = 0; i < 5; i++)
    {
        if (!DHT11_Driver_ReadByte(&buffer[i])) {
            return false;
        }
    }

    /* 检查校验和 */
    if ((buffer[0] + buffer[1] + buffer[2] + buffer[3]) != buffer[4]) {
        return false;
    }

    raw->humi_int = buffer[0];
    raw->humi_dec = buffer[1];
    raw->temp_int = buffer[2];
    raw->temp_dec = buffer[3];

    return true;
}
