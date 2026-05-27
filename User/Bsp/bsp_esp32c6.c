/**
 * @file bsp_esp32c6.c
 * @author  29283
 * @brief   ESP32C6驱动层源文件
 * @created 2026/4/23
 */

#include "bsp_esp32c6.h"
#include "usart.h"
#include <string.h>

/* 外部句柄声明 */
extern UART_HandleTypeDef huart2;

/*==================================================================*
 * 接收缓冲（USART2 中断方式，逐字节接收）                           *
 *==================================================================*/

static uint8_t rx_byte = 0;                     /* 接收中断单字节缓冲区 */
static char rx_line_buf[RX_CMD_BUF_SIZE];       /* 行缓冲：逐字节累积 */
static int  rx_line_pos = 0;                    /* 行缓冲当前位置 */
static char rx_cmd_buf[RX_CMD_BUF_SIZE];        /* 完整命令存放处 */
static volatile bool rx_cmd_pending = false;    /* 有命令待处理标志 */

/*==================================================================*
 * 接口函数                                                          *
 *==================================================================*/

/**
 * @brief ESP32C6初始化函数
 * @param None
 * @retval None
 */
void ESP32C6_Driver_Init(void) {
    /* 清空接收缓冲 */
    rx_line_pos = 0;
    rx_cmd_pending = false;
    memset(rx_line_buf, 0, sizeof(rx_line_buf));
    memset(rx_cmd_buf, 0, sizeof(rx_cmd_buf));

    /* 启动UART接收中断 */
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
}

/**
 * @brief ESP32C6发送数据函数
 * @param data: 要发送的数据指针
 * @param size: 数据大小
 * @retval true  - 发送成功
 *         false - 发送失败
 */
bool ESP32C6_Driver_Send(uint8_t *data, uint16_t size) {
    if(data == NULL || size == 0) {
        return false;
    }

    if(HAL_UART_Transmit(&huart2, data, size, HAL_MAX_DELAY) != HAL_OK) {
        return false;
    }

    return true;
}

/**
 * @brief 获取接收到的命令（非阻塞）
 * @param buf:  输出缓冲区
 * @param size: 缓冲区大小
 * @retval true  - 有命令已取出
 *         false - 无待处理命令
 */
bool ESP32C6_Driver_GetCommand(char *buf, int size) {
    if (!rx_cmd_pending) {
        return false;
    }

    /* 关中断取数据，防止与 ISR 竞争 */
    __disable_irq();
    strncpy(buf, rx_cmd_buf, size - 1);
    buf[size - 1] = '\0';
    rx_cmd_pending = false;
    __enable_irq();

    return true;
}

/*==================================================================*
 * 中断回调                                                          *
 *==================================================================*/

/**
 * @brief UART接收完成回调函数
 * @param huart: UART句柄指针
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        char ch = (char)rx_byte;

        if (ch == '\n') {
            /* 遇到换行：一行结束，存为命令 */
            rx_line_buf[rx_line_pos] = '\0';
            strncpy(rx_cmd_buf, rx_line_buf, RX_CMD_BUF_SIZE - 1);
            rx_cmd_buf[RX_CMD_BUF_SIZE - 1] = '\0';
            rx_cmd_pending = true;
            rx_line_pos = 0;
        } else if (ch != '\r') {
            /* 普通字符，存入行缓冲 */
            if (rx_line_pos < RX_CMD_BUF_SIZE - 1) {
                rx_line_buf[rx_line_pos++] = ch;
            }
        }
        /* '\r' 直接丢弃 */

        /* 重新启动接收中断 */
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}
