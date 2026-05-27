/**
 * @file main.c
 * @author  29283
 * @brief   ESP32C6主程序文件
 * @created 2026/5/20
 *
 * 温湿度监测终端主程序。
 * 负责：WiFi连接 → MQTT上云 + 下行控制 → UART与STM32双向通信。
 *
 * v2.1 新增：MQTT订阅下行指令，解析后通过UART转发给STM32
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "cJSON.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"

#define UART_NUM        UART_NUM_1
#define BUF_SIZE        256
#define ESP_TX_PIN      7
#define ESP_RX_PIN      6

/* MQTT参数 */
#define MQTT_BROKER     "mqtt://studio-mqtt.heclouds.com:1883"
#define MQTT_CLIENT_ID  "stm32_esp32_dht11"
#define MQTT_TOPIC      "$sys/5V0v5VXt33/stm32_esp32_dht11/thing/property/post"
#define MQTT_SUB_TOPIC  "$sys/5V0v5VXt33/stm32_esp32_dht11/thing/property/set"
#define TOPIC_SET_REPLY "$sys/5V0v5VXt33/stm32_esp32_dht11/thing/property/set_reply"

/**
 * @brief WiFi状态回调函数
 * @param status: 当前WiFi状态
 * @retval None
 */
static void on_wifi_status(wifi_status_t status)
{
    if (status == WIFI_CONNECTED) {
        char ip[16];
        WiFi_Manager_GetIP(ip, sizeof(ip));
        printf("WiFi已连接！IP: %s\r\n", ip);
    } else if (status == WIFI_CONNECTING) {
        printf("WiFi连接中...\r\n");
    } else {
        printf("WiFi已断开\r\n");
    }
}

/*==================================================================*
 * MQTT下行数据解析                                                 *
 *==================================================================*/

/**
 * @brief 尝试从属性节点中提取整数值
 *
 * OneNET property/set 有两种格式：
 *   {"GreenLED": 1}          — 直接值
 *   {"GreenLED": {"value": 1}} — 嵌套对象
 *
 * @param node 属性节点
 * @return 整数值，-1 表示无效
 */
static int extract_param_value(cJSON *node)
{
    if (cJSON_IsNumber(node)) {
        return node->valueint;
    }
    cJSON *val = cJSON_GetObjectItem(node, "value");
    if (val != NULL && cJSON_IsNumber(val)) {
        return val->valueint;
    }
    return -1;
}

/**
 * @brief 尝试从JSON中提取消息ID（兼容字符串和数字两种格式）
 * @param root JSON根节点
 * @param buf 输出缓冲区
 * @param size 缓冲区大小
 */
static void extract_msg_id(cJSON *root, char *buf, int size)
{
    cJSON *id_node = cJSON_GetObjectItem(root, "id");
    if (id_node == NULL) {
        buf[0] = '\0';
    } else if (cJSON_IsString(id_node)) {
        strncpy(buf, cJSON_GetStringValue(id_node), size - 1);
        buf[size - 1] = '\0';
    } else if (cJSON_IsNumber(id_node)) {
        snprintf(buf, size, "%d", id_node->valueint);
    } else {
        buf[0] = '\0';
    }
}

/**
 * @brief MQTT下行数据回调
 *
 * 当 OneNET 平台下发属性设置时，ESP-IDF MQTT 库调用此函数。
 * 解析 OneJSON 格式的 property/set 指令，提取属性名和值，
 * 转换为简写命令后通过 UART 发送给 STM32。
 *
 * OneNET 下行格式：
 *   {"params":{"GreenLED":{"value":1}}}  或  {"params":{"GreenLED":1}}
 *
 * 转换后的命令格式：
 *   GREEN:1\r\n
 *
 * @param topic: MQTT主题
 * @param data:  消息内容（JSON字符串）
 * @retval None
 */
static void on_mqtt_data(const char *topic, const char *data)
{
    printf("[MQTT] 下行数据: %s\r\n", data);

    /* 解析JSON */
    cJSON *root = cJSON_Parse(data);
    if (root == NULL) {
        printf("[MQTT] JSON解析失败\r\n");
        return;
    }

    /* 提取消息ID（回复时需要带回） */
    char msg_id[32];
    extract_msg_id(root, msg_id, sizeof(msg_id));

    cJSON *params = cJSON_GetObjectItem(root, "params");
    if (params == NULL) {
        printf("[MQTT] 未找到 params 字段\r\n");
        cJSON_Delete(root);
        return;
    }

    char cmd_buf[32];
    int val;

    /* === 解析 GreenLED === */
    val = extract_param_value(cJSON_GetObjectItem(params, "GreenLED"));
    if (val >= 0) {
        snprintf(cmd_buf, sizeof(cmd_buf), "GREEN:%d\r\n", val);
        uart_write_bytes(UART_NUM, cmd_buf, strlen(cmd_buf));
        printf("[CMD] -> STM32: %s", cmd_buf);
    }

    /* === 解析 BlueLED === */
    val = extract_param_value(cJSON_GetObjectItem(params, "BlueLED"));
    if (val >= 0) {
        snprintf(cmd_buf, sizeof(cmd_buf), "BLUE:%d\r\n", val);
        uart_write_bytes(UART_NUM, cmd_buf, strlen(cmd_buf));
        printf("[CMD] -> STM32: %s", cmd_buf);
    }

    /* === 解析 Threshold（光照阈值） === */
    val = extract_param_value(cJSON_GetObjectItem(params, "Threshold"));
    if (val >= 0) {
        snprintf(cmd_buf, sizeof(cmd_buf), "THRESH:%d\r\n", val);
        uart_write_bytes(UART_NUM, cmd_buf, strlen(cmd_buf));
        printf("[CMD] -> STM32: %s", cmd_buf);
    }

    cJSON_Delete(root);

    /* 回复OneNET，确认属性设置已收到 */
    char reply[128];
    snprintf(reply, sizeof(reply),
             "{\"id\":\"%s\",\"code\":200,\"msg\":\"success\"}", msg_id);
    MQTT_Manager_PublishToTopic(TOPIC_SET_REPLY, reply);
    printf("[MQTT] 已回复属性设置确认\r\n");
}

/*==================================================================*
 * 主函数                                                            *
 *==================================================================*/

void app_main(void)
{
    printf("========================================\r\n");
    printf("  ESP32C6 温湿度监测终端 v2.1\r\n");
    printf("  支持 MQTT 双向通信\r\n");
    printf("========================================\r\n");

    /* ========== 第一步：连接WiFi ========== */
    printf("[1/3] 正在连接WiFi...\r\n");

    wifi_config_data_t wifi_cfg = {
        .ssid     = "199",
        .password = "00000000",
        .max_retry = 5,
    };

    bool wifi_ok = WiFi_Manager_Init(&wifi_cfg, on_wifi_status);

    if (wifi_ok) {
        char ip[16];
        WiFi_Manager_GetIP(ip, sizeof(ip));
        printf("网络就绪：%s\r\n", ip);
    } else {
        printf("WiFi连接失败，后台持续重试中...\r\n");
    }

    /* ========== 第二步：初始化MQTT ========== */
    printf("[2/3] 正在连接MQTT服务器...\r\n");

    /* 先注册下行数据回调（MQTT连接后自动订阅） */
    MQTT_Manager_SetDataCallback(on_mqtt_data);

    /* 设置订阅主题（用于接收云端下发的控制指令） */
    MQTT_Manager_SetSubscribeTopic(MQTT_SUB_TOPIC);

    bool mqtt_ok = MQTT_Manager_Init(MQTT_BROKER, MQTT_CLIENT_ID, MQTT_TOPIC);

    if (mqtt_ok) {
        printf("MQTT初始化完成，等待连接...\r\n");
        /* 等待MQTT连接成功（最多等15秒） */
        for (int i = 0; i < 15; i++) {
            if (MQTT_Manager_GetStatus() == MQTT_CONNECTED) {
                printf("MQTT已连接！\r\n");
                break;
            }
            printf(".");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        printf("\r\n");
    } else {
        printf("MQTT初始化失败\r\n");
    }

    /* 再等一下确保订阅完成 */
    vTaskDelay(pdMS_TO_TICKS(500));

    /* ========== 第三步：启动UART1（与STM32通信） ========== */
    printf("[3/3] 启动UART1（GPIO6/GPIO7）...\r\n");

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, ESP_TX_PIN, ESP_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

    printf("UART1就绪，等待STM32数据...\r\n");
    printf("========================================\r\n\r\n");

    /* ========== 主循环：接收STM32数据 → MQTT发布 ========== */
    uint8_t data[BUF_SIZE];
    int pos = 0;

    while(1) {
        uint8_t byte;
        int len = uart_read_bytes(UART_NUM, &byte, 1, pdMS_TO_TICKS(2000));

        if (len == 1) {
            if (byte == '\n') {
                data[pos] = '\0';
                if (pos > 0) {
                    printf("[STM32] %s\r\n", (char*)data);

                    /* 解析温湿度+光照+LED状态 */
                    float temp = 0, humi = 0;
                    int light = 0, blue = 0, green = 0;
                    sscanf((char*)data, "Temp:%fC,Humi:%f%%,Light:%d,Blue:%d,Green:%d",
                           &temp, &humi, &light, &blue, &green);

                    char json_buf[512];
                    snprintf(json_buf, sizeof(json_buf),
                             "{\"id\":\"123\",\"version\":\"1.0\","
                             "\"params\":{"
                             "\"CurrentTemperature\":{\"value\":%.1f},"
                             "\"CurrentHumidity\":{\"value\":%.1f},"
                             "\"CurrentLight\":{\"value\":%d},"
                             "\"BlueLED\":{\"value\":%d},"
                             "\"GreenLED\":{\"value\":%d}}}",
                             temp, humi, light, blue, green);

                    MQTT_Manager_Publish(json_buf);
                }
                pos = 0;
            } else if (byte != '\r') {
                if (pos < BUF_SIZE - 1) {
                    data[pos++] = byte;
                }
            }
        }
    }
}
