/**
 * @file mqtt_manager.c
 * @author  29283
 * @brief   MQTT管理模块源文件
 * @created 2026/5/20
 *
 * 负责MQTT的连接、数据发布、断线重连。
 * 依赖WiFi模块先连接网络。
 * v2.1 新增：订阅下行主题 + 数据回调机制
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
static char publish_topic[64] = "";   /* 发布数据的主题 */
static char subscribe_topic[64] = ""; /* 订阅主题 */
static char client_id_str[32] = "";   /* 客户端ID */

/* 下行数据回调 */
static mqtt_data_callback_t data_callback = NULL;

/*==================================================================*
 * 离线数据缓存                                                     *
 *==================================================================*/
#define CACHE_MAX_ENTRIES   30          /* 最多缓存30条 */
#define CACHE_DATA_LEN      520         /* 单条数据最大长度 */

static char cache_buf[CACHE_MAX_ENTRIES][CACHE_DATA_LEN]; /* 环形缓冲区 */
static int  cache_write_idx = 0;        /* 下一个写入位置 */
static int  cache_count = 0;            /* 当前缓存条目数 */

/* 内部函数声明 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data);
static void flush_cache(void);

/**
 * @brief MQTT事件处理函数（内部）
 *
 * ESP-IDF在MQTT状态变化时调用此函数。
 * 根据事件类型更新内部状态并打印日志。
 * v2.1 新增：连接成功后自动订阅 + 下行数据接收
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
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            current_status = MQTT_CONNECTED;
            printf("[MQTT] 已连接到服务器\r\n");

            /* 连接成功后自动订阅下行主题 */
            if (subscribe_topic[0] != '\0') {
                esp_mqtt_client_subscribe(mqtt_client, subscribe_topic, 1);
                printf("[MQTT] 已订阅: %s\r\n", subscribe_topic);
            }

            /* 补发离线缓存 */
            if (cache_count > 0) {
                printf("[MQTT] 正在补发缓存数据... (%d条)\r\n", cache_count);
                flush_cache();
            }
            break;

        case MQTT_EVENT_DISCONNECTED:
            current_status = MQTT_DISCONNECTED;
            printf("[MQTT] 连接断开，等待自动重连...\r\n");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            printf("[MQTT] 订阅成功\r\n");
            break;

        case MQTT_EVENT_DATA:
            /* 收到下行数据 */
            if (event->topic && event->data) {
                /* 构造以 \0 结尾的字符串 */
                char topic_buf[128];
                char data_buf[512];
                int topic_len = event->topic_len < (int)sizeof(topic_buf) - 1
                                ? event->topic_len : (int)sizeof(topic_buf) - 1;
                int data_len = event->data_len < (int)sizeof(data_buf) - 1
                               ? event->data_len : (int)sizeof(data_buf) - 1;

                strncpy(topic_buf, event->topic, topic_len);
                topic_buf[topic_len] = '\0';
                strncpy(data_buf, event->data, data_len);
                data_buf[data_len] = '\0';

                printf("[MQTT] 收到下行数据\r\n");

                /* 调用回调（由 main.c 实现具体解析逻辑） */
                if (data_callback) {
                    data_callback(topic_buf, data_buf);
                }
            }
            break;

        case MQTT_EVENT_PUBLISHED:
            /* 数据发布成功（mqtt_client库内部已处理确认） */
            break;

        case MQTT_EVENT_ERROR:
            printf("[MQTT] 发生错误, error_type=%d\r\n", event->error_handle->error_type);
            break;

        default:
            break;
    }
}

/**
 * @brief 初始化MQTT并连接到服务器
 *
 * @param broker_url: MQTT服务器地址
 * @param client_id: 客户端ID
 * @param pub_topic: 发布主题
 * @retval true  - 初始化成功
 *         false - 初始化失败
 */
bool MQTT_Manager_Init(const char *broker_url, const char *client_id, const char *pub_topic)
{
    if (broker_url == NULL || client_id == NULL || pub_topic == NULL) {
        printf("[MQTT] 错误：参数为空\r\n");
        return false;
    }

    strncpy(publish_topic, pub_topic, sizeof(publish_topic) - 1);
    strncpy(client_id_str, client_id, sizeof(client_id_str) - 1);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = broker_url,
        .credentials.client_id = client_id_str,
        .credentials.username = "5V0v5VXt33",
        .credentials.authentication.password = "version=2018-10-31&res=products%2F5V0v5VXt33%2Fdevices%2Fstm32_esp32_dht11&et=4102416000&method=sha256&sign=mqgY1ujL3y19A84PlN7fWYRMpjjeS1HYKNnFux75IL4%3D",
        .session.keepalive = 10,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        printf("[MQTT] 客户端创建失败\r\n");
        return false;
    }

    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);

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
 * @brief 设置订阅主题
 *
 * 在 MQTT 连接（或重连）成功后，会自动订阅此主题。
 *
 * @param topic: 要订阅的MQTT主题
 * @retval None
 */
void MQTT_Manager_SetSubscribeTopic(const char *topic)
{
    if (topic == NULL) {
        subscribe_topic[0] = '\0';
        return;
    }
    strncpy(subscribe_topic, topic, sizeof(subscribe_topic) - 1);
    subscribe_topic[sizeof(subscribe_topic) - 1] = '\0';
}

/**
 * @brief 注册下行数据回调函数
 *
 * 当收到已订阅主题的消息时，回调函数会被调用。
 * 建议在 main.c 中实现具体解析逻辑。
 *
 * @param callback: 回调函数指针
 * @retval None
 */
void MQTT_Manager_SetDataCallback(mqtt_data_callback_t callback)
{
    data_callback = callback;
}

/**
 * @brief 发布一条数据到MQTT主题
 *
 * @param data: 要发送的数据字符串
 * @retval true  - 发布成功
 *         false - 发布失败
 */
bool MQTT_Manager_Publish(const char *data)
{
    if (data == NULL || strlen(data) == 0) {
        return false;
    }

    /* 未连接：缓存数据 */
    if (current_status != MQTT_CONNECTED) {
        if (cache_count < CACHE_MAX_ENTRIES) {
            strncpy(cache_buf[cache_write_idx], data, CACHE_DATA_LEN - 1);
            cache_buf[cache_write_idx][CACHE_DATA_LEN - 1] = '\0';
            cache_write_idx = (cache_write_idx + 1) % CACHE_MAX_ENTRIES;
            cache_count++;
        } else {
            /* 缓存已满，覆盖最旧的一条 */
            strncpy(cache_buf[cache_write_idx], data, CACHE_DATA_LEN - 1);
            cache_buf[cache_write_idx][CACHE_DATA_LEN - 1] = '\0';
            cache_write_idx = (cache_write_idx + 1) % CACHE_MAX_ENTRIES;
        }
        printf("[MQTT] 未连接，数据已缓存 (共%d条)\r\n", cache_count);
        return false;
    }

    /* 已连接：直接发布 */
    int msg_id = esp_mqtt_client_publish(
        mqtt_client,
        publish_topic,
        data,
        0,
        1,
        0
    );

    if (msg_id < 0) {
        printf("[MQTT] 发布失败\r\n");
        return false;
    }

    printf("[MQTT] 已发布\r\n");
    return true;
}

/**
 * @brief 补发所有离线缓存的数据
 * @retval None
 */
static void flush_cache(void)
{
    /* 计算最旧数据的位置 */
    int start = (cache_write_idx - cache_count + CACHE_MAX_ENTRIES) % CACHE_MAX_ENTRIES;
    int success = 0;

    for (int i = 0; i < cache_count; i++) {
        int pos = (start + i) % CACHE_MAX_ENTRIES;
        int msg_id = esp_mqtt_client_publish(
            mqtt_client,
            publish_topic,
            cache_buf[pos],
            0,
            1,
            0
        );
        if (msg_id >= 0) {
            success++;
            printf("[MQTT] 已补发 [%d/%d]\r\n", i + 1, cache_count);
        } else {
            printf("[MQTT] 补发失败 [%d/%d]\r\n", i + 1, cache_count);
        }
    }

    cache_count = 0;
    printf("[MQTT] 缓存补发完成 (%d条成功)\r\n", success);
}

/**
 * @brief 发布一条数据到指定MQTT主题
 * @param topic: 目标主题
 * @param data: 要发送的数据字符串
 * @retval true  - 发布成功
 *         false - 发布失败（未连接等）
 */
bool MQTT_Manager_PublishToTopic(const char *topic, const char *data)
{
    if (current_status != MQTT_CONNECTED) {
        printf("[MQTT] 未连接，消息已丢弃: %s\r\n", data);
        return false;
    }

    if (topic == NULL || data == NULL || strlen(data) == 0) {
        return false;
    }

    int msg_id = esp_mqtt_client_publish(
        mqtt_client,
        topic,
        data,
        0,
        1,
        0
    );

    if (msg_id < 0) {
        printf("[MQTT] 发布到 %s 失败\r\n", topic);
        return false;
    }

    printf("[MQTT] 已发布到 %s\r\n", topic);
    return true;
}

/**
 * @brief 获取当前MQTT连接状态
 * @retval mqtt_status_t 当前状态
 */
mqtt_status_t MQTT_Manager_GetStatus(void)
{
    return current_status;
}

/**
 * @brief 断开MQTT连接
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
