/**
 * @file bsp_esp32c6.c
 * @author  29283
 * @brief   ESP32C6驱动层源文件
 * @created 2026/4/23
 */

#include "bsp_esp32c6.h"
#include "usart.h"

/* 外部句柄声明 */
extern UART_HandleTypeDef huart2;

/* 全局变量 */
static uint8_t rx_data = 0;     /* 接收数据缓冲区 */

/**
 * @brief ESP32C6初始化函数
 * @param None
 * @retval None
 */
void ESP32C6_Driver_Init(void) {
    /* 启动UART接收中断 */
    HAL_UART_Receive_IT(&huart2, &rx_data, 1);
}

/**
 * @brief ESP32C6发送数据函数
 * @param data: 要发送的数据指针
 * @param size: 数据大小
 * @retval None
 */
bool ESP32C6_Driver_Send(uint8_t *data, uint16_t size) {
    /* 参数校验 */
    if(data == NULL || size == 0) {
        return false;
    }

    /* 检验发送数据是否成功 */
    if(HAL_UART_Transmit(&huart2, data, size, HAL_MAX_DELAY) != HAL_OK) {
        return false;
    }

    return true;
}

/**
 * @brief UART接收完成回调函数
 * @param huart: UART句柄指针
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
      /* 重新启动UART接收中断 */
      HAL_UART_Receive_IT(&huart2, &rx_data, 1);
    }
}
