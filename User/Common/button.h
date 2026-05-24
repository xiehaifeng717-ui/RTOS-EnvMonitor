/**
 * @file    button.h
 * @author  29283
 * @brief   按键驱动层头文件（中断方式）
 * @created 2026/5/24
 */

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "stdbool.h"

/*--- 函数声明 ---*/
void Button_Init(void);             /* 按键初始化 */
bool Button_WasPressed(void);       /* 检测是否有按键按下事件（含防抖确认） */

#endif //_BUTTON_H_
