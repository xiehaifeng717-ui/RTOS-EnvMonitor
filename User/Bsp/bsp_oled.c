/**
 * @file    bsp_oled.c
 * @author  29283
 * @brief   OLED驱动层实现文件
 * @created 2026/4/23
 */

#include "bsp_oled.h"
#include "oledfont.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

extern I2C_HandleTypeDef hi2c1;
static uint8_t OLED_GRAM[8][128];   /* OLED数据缓冲区 */

/**
 * @brief OLED写命令
 * @param cmd: 命令数据
 * @retval None
 */
void OLED_Driver_WriteCommand(uint8_t cmd) {
    uint8_t buf[2];
    buf[0] = 0x00;  /* 命令模式 */
    buf[1] = cmd;   /* 命令数据 */
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buf, 2, HAL_MAX_DELAY);
}

/**
 * @brief 清空OLED缓冲区
 * @param None
 * @retval None
 */
void OLED_Driver_Clear(void) {
    memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
}

/**
 * @brief 刷新屏幕状态
 * @param None
 * @retval None
 */
void OLED_Driver_Refresh(void) {
    for (uint8_t page = 0; page < 8; page++) {
        OLED_Driver_WriteCommand(0xB0 + page); /* 设置页地址 */
        OLED_Driver_WriteCommand(0x00);        /* 设置列地址低8位 */
        OLED_Driver_WriteCommand(0x10);        /* 设置列地址高8位 */

        for (uint8_t i = 0; i < 128; i += 32) {
            uint8_t buf[33];            /* I2C传输缓冲区 */
            buf[0] = 0x40;              /* 数据模式 */
            memcpy(&buf[1], &OLED_GRAM[page][i], 32);   /* 复制数据到传输缓冲区 */
            HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buf, 33, HAL_MAX_DELAY);
        }
    }
}

/**
 * @brief 初始化OLED
 * @param None
 * @retval None
 */
void OLED_Driver_Init(void)
{
    HAL_Delay(100);

    OLED_Driver_WriteCommand(0xAE);    /* 关闭显示 */
    OLED_Driver_WriteCommand(0xD5);    /* 设置时钟分频因子 */
    OLED_Driver_WriteCommand(0x80);    /* 推荐设置 */
    OLED_Driver_WriteCommand(0xA8);    /* 设置驱动路数 */
    OLED_Driver_WriteCommand(0x3F);    /* 1/64占空比 */
    OLED_Driver_WriteCommand(0xD3);    /* 设置显示偏移 */
    OLED_Driver_WriteCommand(0x00);    /* 不偏移 */
    OLED_Driver_WriteCommand(0x40);    /* 设置起始行 */
    OLED_Driver_WriteCommand(0x8D);    /* 电荷泵设置 */
    OLED_Driver_WriteCommand(0x14);    /* 使能电荷泵 */
    OLED_Driver_WriteCommand(0x20);    /* 设置内存地址模式 */
    OLED_Driver_WriteCommand(0x02);    /* 水平地址模式 */
    OLED_Driver_WriteCommand(0xA1);    /* 设置段重定义 */
    OLED_Driver_WriteCommand(0xC8);    /* 设置COM扫描方向 */
    OLED_Driver_WriteCommand(0xDA);    /* 设置COM引脚硬件配置 */
    OLED_Driver_WriteCommand(0x12);    /* 推荐设置 */
    OLED_Driver_WriteCommand(0x81);    /* 设置对比度 */
    OLED_Driver_WriteCommand(0xCF);    /* 推荐设置 */
    OLED_Driver_WriteCommand(0xD9);    /* 设置预充电周期 */
    OLED_Driver_WriteCommand(0xF1);    /* 推荐设置 */
    OLED_Driver_WriteCommand(0xDB);    /* 设置VCOMH电压倍率 */
    OLED_Driver_WriteCommand(0x40);    /* 推荐设置 */
    OLED_Driver_WriteCommand(0xA4);    /* 全局显示开启 */
    OLED_Driver_WriteCommand(0xA6);    /* 设置正常显示 */
    OLED_Driver_WriteCommand(0xAF);    /* 打开显示 */

    OLED_Driver_Clear();               /* 清空缓冲区 */
    OLED_Driver_Refresh();             /* 刷新显示 */
}

/**
 * @brief 在指定位置显示一个字符
 * @param x: X坐标
 * @param y: Y坐标
 * @param chr: 要显示的字符
 * @retval None
 */
void OLED_Driver_ShowChar(uint8_t x, uint8_t y, uint8_t chr) {
    if (chr < ' ' || chr > '~' || x > 120 || y > 6) {
        return; /* 字符不在可显示范围内，或坐标超出屏幕范围 */
    }

    uint8_t c = chr - ' ';

    for (uint8_t i = 0; i < 8; i++){
        OLED_GRAM[y][x + i]     = F8x16[c][i];      /* 字符上半部分 */
        OLED_GRAM[y + 1][x + i] = F8x16[c][i + 8];  /* 字符下半部分 */
    }
}

/**
 * @brief 在指定位置显示字符串
 * @param x: X坐标
 * @param y: Y坐标
 * @param str: 要显示的字符串
 * @retval None
 */
void OLED_Driver_ShowString(uint8_t x, uint8_t y, const char *str) {
    while (*str) {
        OLED_Driver_ShowChar(x, y, (uint8_t)(*str));
        str++;  
        x += 8; /* 每个字符占8像素宽 */

        if (x > 120) {      /* 如果超出屏幕宽度 */
            x = 0;          /* 回到行首 */
            y += 2;         /* 换行显示 */
            if (y > 6) {    /* 如果超出屏幕高度 */
                break;
            }
        }
    }
}

/**
 * @brief 在指定位置显示数字
 * @param x: X坐标
 * @param y: Y坐标
 * @param num: 要显示的数字
 * @param len: 数字长度
 * @retval None
 */
void OLED_Driver_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len) {
    char buf[12];       /* 缓冲区 */
    char format[10];    /* 格式字符串 */

    if (len > 10) {     /* 如果长度超过10 */
        len = 10;
    }

    snprintf(format, sizeof(format), "%%0%dlu", len);       /* 构造格式字符串 */
    snprintf(buf, sizeof(buf), format, (unsigned long)num); /* 格式化数字 */

    OLED_Driver_ShowString(x, y, buf); /* 指定位置显示数字 */
}

/**
 * @brief 在指定位置显示浮点数
 * @param x: X坐标
 * @param y: Y坐标
 * @param num: 要显示的浮点数
 * @param int_len: 整数部分长度
 * @param frac_len: 小数部分长度
 * @retval None
 */
void OLED_Driver_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t int_len, uint8_t frac_len) {
    uint32_t int_part = 0;  /* 整数部分 */
    uint32_t frac_part = 0; /* 小数部分 */
    uint32_t scale = 1;     /* 小数部分放大倍数 */

    /* 计算小数部分放大倍数 */
    for (uint8_t i = 0; i < frac_len; i++) {
        scale *= 10;        
    }

    /* 处理负数 */
    if (num < 0) {
        OLED_Driver_ShowChar(x, y, '-');
        x += 8;
        num = -num;
    }

    /* 计算整数部分和小数部分 */
    int_part = (uint32_t)num;
    frac_part = (uint32_t)((num - int_part) * scale + 0.5f);

    /* 处理小数部分四舍五入 */
    if (frac_part >= scale) {
        frac_part = 0;
        int_part += 1;
    }

    /* 显示整数部分 */
    OLED_Driver_ShowNum(x, y, int_part, int_len);
    x += int_len * 8;

    /* 显示小数点 */
    OLED_Driver_ShowChar(x, y, '.');
    x += 8;

    /* 显示小数部分 */
    OLED_Driver_ShowNum(x, y, frac_part, frac_len);
}
