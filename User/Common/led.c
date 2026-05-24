/**
 * @file    led.c
 * @author  29283
 * @brief   LED驱动层源文件
 * @created 2026/5/24
 */

#include "led.h"
#include "stm32f1xx_hal.h"
#include "gpio.h"

/*==================================================================*
 * 蓝灯（PB1）：光照自动控制                                        *
 * CubeMX宏：LED_BLUE_Pin / LED_BLUE_GPIO_Port                     *
 *==================================================================*/

/**
 * @brief 蓝灯初始化
 * @param None
 * @retval None
 */
void LED_Blue_Init(void) {
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
}

/**
 * @brief 点亮蓝灯
 * @param None
 * @retval None
 */
void LED_Blue_On(void) {
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
}

/**
 * @brief 熄灭蓝灯
 * @param None
 * @retval None
 */
void LED_Blue_Off(void) {
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
}

/**
 * @brief 翻转蓝灯
 * @param None
 * @retval None
 */
void LED_Blue_Toggle(void) {
    HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
}

/*==================================================================*
 * 绿灯（PB0）：按键手动控制                                        *
 * CubeMX宏：LED_GREEN_Pin / LED_GREEN_GPIO_Port                   *
 *==================================================================*/

/**
 * @brief 绿灯初始化
 * @param None
 * @retval None
 */
void LED_Green_Init(void) {
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
}

/**
 * @brief 点亮绿灯
 * @param None
 * @retval None
 */
void LED_Green_On(void) {
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
}

/**
 * @brief 熄灭绿灯
 * @param None
 * @retval None
 */
void LED_Green_Off(void) {
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
}

/**
 * @brief 翻转绿灯
 * @param None
 * @retval None
 */
void LED_Green_Toggle(void) {
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}
