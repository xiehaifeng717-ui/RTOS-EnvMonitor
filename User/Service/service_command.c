/**
 * @file    service_command.c
 * @author  29283
 * @brief   命令解析服务层源文件
 * @created 2026/5/27
 */

#include "service_command.h"
#include "led.h"
#include "app_data.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief 解析并执行下行命令
 * @param cmd: 命令字符串，如 "GREEN:1"
 * @retval None
 */
void Command_Service_Process(const char *cmd) {
    if (cmd == NULL || cmd[0] == '\0') {
        return;
    }

    /* === 绿灯控制 === */
    if (strncmp(cmd, "GREEN:", 6) == 0) {
        int val = atoi(cmd + 6);
        if (val) {
            LED_Green_On();
            g_sensor_share.green_led = 1;
            printf("[CMD] Green LED ON\r\n");
        } else {
            LED_Green_Off();
            g_sensor_share.green_led = 0;
            printf("[CMD] Green LED OFF\r\n");
        }
        return;
    }

    /* === 蓝灯控制 === */
    if (strncmp(cmd, "BLUE:", 5) == 0) {
        int val = atoi(cmd + 5);
        if (val) {
            LED_Blue_On();
            printf("[CMD] Blue LED ON\r\n");
        } else {
            LED_Blue_Off();
            printf("[CMD] Blue LED OFF\r\n");
        }
        /* 蓝灯由光照自动控制，下行控制时只临时操作，不覆盖自动逻辑 */
        return;
    }

    /* === 光照阈值设置 === */
    if (strncmp(cmd, "THRESH:", 7) == 0) {
        int val = atoi(cmd + 7);
        if (val >= 500 && val <= 4000) {
            g_light_threshold = (uint16_t)val;
            printf("[CMD] Light threshold set to %d\r\n", g_light_threshold);
        } else {
            printf("[CMD] Invalid threshold: %d (500~4000)\r\n", val);
        }
        return;
    }

    /* 未知命令 */
    printf("[CMD] Unknown: %s\r\n", cmd);
}
