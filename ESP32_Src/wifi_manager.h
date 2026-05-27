/**
 * @file wifi_manager.h
 * @author  29283
 * @brief   WiFi管理模块头文件
 * @created 2026/5/20
 */

#ifndef _WIFI_MANAGER_H_
#define _WIFI_MANAGER_H_

#include <stdbool.h>

/* WiFi连接状态枚举 */
typedef enum {
    WIFI_DISCONNECTED = 0,   /* 未连接 */
    WIFI_CONNECTING,          /* 正在连接 */
    WIFI_CONNECTED            /* 已连接 */
} wifi_status_t;

/* WiFi配置结构体 */
typedef struct {
    char ssid[32];        /* WiFi名称 */
    char password[64];    /* WiFi密码 */
    int max_retry;        /* 最大重试次数 */
} wifi_config_data_t;

/* 回调函数类型 */
typedef void (*wifi_callback_t)(wifi_status_t status);

/* 函数声明 */

/**
 * @brief 初始化WiFi并开始连接
 * @param config: WiFi配置（SSID, 密码, 重试次数）
 * @param callback: 可选的回调函数，状态变化时调用
 * @retval true  - 初始化成功
 *         false - 初始化失败
 */
bool WiFi_Manager_Init(wifi_config_data_t *config, wifi_callback_t callback);

/**
 * @brief 获取当前WiFi连接状态
 * @retval wifi_status_t 当前状态
 */
wifi_status_t WiFi_Manager_GetStatus(void);

/**
 * @brief 获取本机IP地址字符串
 * @param ip: 输出缓冲区，至少16字节
 * @param size: 缓冲区大小
 * @retval true  - 已获取到IP
 *         false - 未连接或无IP
 */
bool WiFi_Manager_GetIP(char *ip, int size);

/**
 * @brief 断开WiFi连接
 * @param None
 * @retval None
 */
void WiFi_Manager_Disconnect(void);

#endif /* _WIFI_MANAGER_H_ */
