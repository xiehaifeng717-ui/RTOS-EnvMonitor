/**
 * @file mqtt_manager.h
 * @author  29283
 * @brief   MQTT管理模块头文件
 * @created 2026/5/20
 */

#ifndef _MQTT_MANAGER_H_
#define _MQTT_MANAGER_H_

#include <stdbool.h>

/* MQTT连接状态枚举 */
typedef enum {
    MQTT_DISCONNECTED = 0,   /* 未连接 */
    MQTT_CONNECTING,          /* 正在连接 */
    MQTT_CONNECTED            /* 已连接 */
} mqtt_status_t;

/* 函数声明 */

/**
 * @brief 初始化MQTT并连接到服务器
 * @param broker_url: MQTT服务器地址，如"mqtt://broker.emqx.io:1883"
 * @param client_id: 客户端ID，必须唯一
 * @param pub_topic: 发布数据的主题，如"envmonitor/data"
 * @retval true  - 初始化成功
 *         false - 初始化失败
 */
bool MQTT_Manager_Init(const char *broker_url, const char *client_id, const char *pub_topic);

/**
 * @brief 发布一条数据到MQTT主题
 * @param data: 要发送的数据字符串
 * @retval true  - 发布成功
 *         false - 发布失败（未连接等）
 */
bool MQTT_Manager_Publish(const char *data);

/**
 * @brief 获取当前MQTT连接状态
 * @retval mqtt_status_t 当前状态
 */
mqtt_status_t MQTT_Manager_GetStatus(void);

/**
 * @brief 断开MQTT连接
 * @param None
 * @retval None
 */
void MQTT_Manager_Disconnect(void);

#endif /* _MQTT_MANAGER_H_ */
