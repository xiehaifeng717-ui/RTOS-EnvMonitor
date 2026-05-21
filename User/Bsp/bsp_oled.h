/**
 * @file bsp_oled.h
 * @author  29283
 * @brief   OLED驱动层头文件
 * @created 2026/4/23
 */

#ifndef _BSP_OLED_H_
#define _BSP_OLED_H_

#include "main.h"
#include <stdint.h>

#define OLED_I2C_ADDR_7BIT   0x3C
#define OLED_ADDR            (OLED_I2C_ADDR_7BIT << 1) // OLED 7位地址0x3C左移1位，供HAL_I2C_Master_Transmit使用

/* 函数声明 */
void OLED_Driver_WriteCommand(uint8_t cmd);  /* OLED写命令 */

void OLED_Driver_Init(void);       /* OLED初始化 */
void OLED_Driver_Clear(void);      /* OLED清屏 */
void OLED_Driver_Refresh(void);    /* OLED刷新 */

/* OLED显示字符 */
void OLED_Driver_ShowChar(uint8_t x, uint8_t y, uint8_t chr);  
/* OLED显示字符串 */             
void OLED_Driver_ShowString(uint8_t x, uint8_t y, const char *str);
/* OLED显示数字 */         
void OLED_Driver_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len);  
/* OLED显示浮点数 */
void OLED_Driver_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t int_len, uint8_t frac_len);

#endif //_BSP_OLED_H_
