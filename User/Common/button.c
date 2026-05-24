/**
 * @file    button.c
 * @author  29283
 * @brief   按键驱动层源文件（轮询方式）
 * @created 2026/5/24
 */

#include "button.h"
#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "cmsis_os2.h"

/* 按键引脚：PA4，内部上拉，按下为低电平 */
/* CubeMX宏：KEY_Pin / KEY_GPIO_Port */

/* 按键状态机：边缘检测用 */
static bool g_last_state = true;

/**
 * @brief 按键初始化
 * @param None
 * @retval None
 */
void Button_Init(void) {
    g_last_state = true;
}

/**
 * @brief 检测按键是否被按下过（边缘触发，含防抖）
 * @param None
 * @retval true=检测到一次按键按下事件
 * @note 每调用一次只返回一次true，需要释放后才能再次触发
 */
bool Button_WasPressed(void) {
    bool current_state;
    bool ret = false;

    /* 读取按键当前状态 */
    current_state = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);

    /* 检测下降沿：之前为高，现在为低（按下） */
    if (g_last_state == true && current_state == GPIO_PIN_RESET) {
        /* 延时20ms防抖 */
        osDelay(20);

        /* 二次确认 */
        current_state = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
        if (current_state == GPIO_PIN_RESET) {
            ret = true;
        }
    }

    g_last_state = current_state;
    return ret;
}
