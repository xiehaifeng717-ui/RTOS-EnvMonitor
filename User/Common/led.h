/**
 * @file    led.h
 * @author  29283
 * @brief   LED驱动层头文件
 * @created 2026/5/24
 */

#ifndef _LED_H_
#define _LED_H_

#include "stdint.h"
#include "stdbool.h"

/*--- 函数声明 ---*/
/* 蓝灯（PB1，光照自动控制） */
void LED_Blue_Init(void);       /* 蓝灯初始化 */
void LED_Blue_On(void);         /* 点亮蓝灯 */
void LED_Blue_Off(void);        /* 熄灭蓝灯 */
void LED_Blue_Toggle(void);     /* 翻转蓝灯 */

/* 绿灯（PB0，按键手动控制） */
void LED_Green_Init(void);      /* 绿灯初始化 */
void LED_Green_On(void);        /* 点亮绿灯 */
void LED_Green_Off(void);       /* 熄灭绿灯 */
void LED_Green_Toggle(void);    /* 翻转绿灯 */

#endif //_BSP_LED_H_
