/**
 * @file main.c
 * @author  29283
 * @brief   ESP32C6主程序文件
 * @created 2026/5/20
 *
 * 温湿度监测终端主程序。
 * 负责：WiFi连接 → MQTT上云 → UART接收STM32数据。
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
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

void app_main(void)
{
    printf("========================================\r\n");
    printf("  ESP32C6 温湿度监测终端\r\n");
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

    /* ========== 第二步：连接MQTT服务器 ========== */
    printf("[2/3] 正在连接MQTT服务器...\r\n");

    bool mqtt_ok = MQTT_Manager_Init(MQTT_BROKER, MQTT_CLIENT_ID, MQTT_TOPIC);

    if (mqtt_ok) {
        printf("MQTT初始化完成，等待连接...\r\n");
    } else {
        printf("MQTT初始化失败\r\n");
    }

    /* 给MQTT一点时间连接 */
    vTaskDelay(pdMS_TO_TICKS(2000));

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

                    /* 解析温湿度数据，拼接OneJSON格式 */
                    float temp = 0, humi = 0;
                    sscanf((char*)data, "Temp:%fC,Humi:%f%%", &temp, &humi);

                    char json_buf[256];
                    snprintf(json_buf, sizeof(json_buf),
                             "{\"id\":\"123\",\"version\":\"1.0\","
                             "\"params\":{\"CurrentTemperature\":{\"value\":%.1f},"
                             "\"CurrentHumidity\":{\"value\":%.1f}}}",
                             temp, humi);

                    /* 发布JSON到OneNET */
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
