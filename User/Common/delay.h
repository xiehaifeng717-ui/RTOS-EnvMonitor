/**
 * @file delay.h
 * @author  29283
 * @brief   延时头文件
 * @created 2026/4/23
 */

#ifndef __DELAY_H
#define __DELAY_H

#include "tim.h"

/* 函数声明 */
void delay_us(uint16_t us);     /* 微秒级延时函数 */
void delay_ms(uint16_t ms);     /* 毫秒级延时函数 */

#endif
