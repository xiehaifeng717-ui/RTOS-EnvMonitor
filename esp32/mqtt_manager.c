/**
 * @file mqtt_manager.c
 * @author  29283
 * @brief   MQTT管理模块源文件
 * @created 2026/5/20
 *
 * 负责MQTT的连接、数据发布、断线重连。
 * 依赖WiFi模块先连接网络。
 */

#include "mqtt_manager.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"

/* 全局变量 */
static esp_mqtt_client_handle_t mqtt_client = NULL;  /* MQTT客户端句柄 */
static mqtt_status_t current_status = MQTT_DISCONNECTED;  /* 当前MQTT状态 */
static char publish_topic[64] = "";  /* 发布数据的主题 */
static char client_id_str[32] = "";  /* 客户端ID */

/* 内部函数声明 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data);

/**
 * @brief MQTT事件处理函数（内部）
 *
 * ESP-IDF在MQTT状态变化时调用此函数。
 * 根据事件类型更新内部状态并打印日志。
 *
 * @param handler_args: 用户参数
 * @param base: 事件基类型
 * @param event_id: 事件ID
 * @param event_data: 事件附带数据
 * @retval None
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            /* MQTT连接成功 */
            current_status = MQTT_CONNECTED;
            printf("[MQTT] 已连接到服务器\r\n");
            break;

        case MQTT_EVENT_DISCONNECTED:
            /* MQTT连接断开，自动重连由ESP-IDF内部处理 */
            current_status = MQTT_DISCONNECTED;
            printf("[MQTT] 连接断开，等待自动重连...\r\n");
            break;

        case MQTT_EVENT_PUBLISHED:
            /* 数据发布成功（mqtt_client库内部已处理确认） */
            break;

        case MQTT_EVENT_ERROR:
            /* 发生错误 */
            printf("[MQTT] 发生错误\r\n");
            break;

        default:
            break;
    }
}

/**
 * @brief 初始化MQTT并连接到服务器
 *
 * 完整流程：
 * 1. 配置MQTT参数（服务器地址、客户端ID等）
 * 2. 创建MQTT客户端
 * 3. 注册事件回调
 * 4. 启动连接
 *
 * @param broker_url: MQTT服务器地址
 * @param client_id: 客户端ID
 * @param pub_topic: 发布主题
 * @retval true  - 初始化成功
 *         false - 初始化失败
 */
bool MQTT_Manager_Init(const char *broker_url, const char *client_id, const char *pub_topic)
{
    /* 参数校验 */
    if (broker_url == NULL || client_id == NULL || pub_topic == NULL) {
        printf("[MQTT] 错误：参数为空\r\n");
        return false;
    }

    /* 保存发布主题和客户端ID */
    strncpy(publish_topic, pub_topic, sizeof(publish_topic) - 1);
    strncpy(client_id_str, client_id, sizeof(client_id_str) - 1);

    /* 配置MQTT参数 */
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = broker_url,
        .credentials.client_id = client_id_str,
        .credentials.username = "5V0v5VXt33",
        .credentials.authentication.password = "version=2018-10-31&res=products%2F5V0v5VXt33%2Fdevices%2Fstm32_esp32_dht11&et=2524579200&method=sha256&sign=h0Wl2h5Gh%2FkUDovs8Q2v%2B7V0p0PHjcbKVApRjfTsq1g%3D",
        .session.last_will.topic = NULL,
    };

    /* 创建MQTT客户端 */
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        printf("[MQTT] 客户端创建失败\r\n");
        return false;
    }

    /* 注册事件回调 */
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);

    /* 启动连接 */
    esp_err_t ret = esp_mqtt_client_start(mqtt_client);
    if (ret != ESP_OK) {
        printf("[MQTT] 启动失败\r\n");
        return false;
    }

    current_status = MQTT_CONNECTING;
    printf("[MQTT] 正在连接到 %s ...\r\n", broker_url);
    return true;
}

/**
 * @brief 发布一条数据到MQTT主题
 *
 * 发布前检查连接状态，未连接则不发布。
 * 数据发布到初始化时指定的主题。
 *
 * @param data: 要发送的数据字符串
 * @retval true  - 发布成功
 *         false - 发布失败
 */
bool MQTT_Manager_Publish(const char *data)
{
    /* 检查是否已连接到MQTT服务器 */
    if (current_status != MQTT_CONNECTED) {
        printf("[MQTT] 未连接，消息已丢弃: %s\r\n", data);
        return false;
    }

    /* 参数校验 */
    if (data == NULL || strlen(data) == 0) {
        return false;
    }

    /* 发布数据到主题 */
    int msg_id = esp_mqtt_client_publish(
        mqtt_client,
        publish_topic,     /* 主题 */
        data,              /* 消息内容 */
        0,                 /* 长度（0表示自动计算） */
        1,                 /* QoS 1（至少一次） */
        0                  /* 不保留 */
    );

    if (msg_id < 0) {
        printf("[MQTT] 发布失败\r\n");
        return false;
    }

    printf("[MQTT] 已发布 -> %s : %s\r\n", publish_topic, data);
    return true;
}

/**
 * @brief 获取当前MQTT连接状态
 * @param None
 * @retval mqtt_status_t 当前状态
 */
mqtt_status_t MQTT_Manager_GetStatus(void)
{
    return current_status;
}

/**
 * @brief 断开MQTT连接
 * @param None
 * @retval None
 */
void MQTT_Manager_Disconnect(void)
{
    if (mqtt_client != NULL) {
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
    }
    current_status = MQTT_DISCONNECTED;
    printf("[MQTT] 已断开连接\r\n");
}
