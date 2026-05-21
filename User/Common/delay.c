/**
 * @file    delay.c
 * @author  29283
 * @brief   延时函数实现文件
 * @created 2026/4/23
 */

#include "delay.h"

extern TIM_HandleTypeDef htim2; /* 定时器2句柄 */

/* 微秒级延时函数 */
void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

/* 毫秒级延时函数 */
void delay_ms(uint16_t ms)
{
    while (ms--)
    {
        delay_us(1000);
    }
}
