/**
 * @file wifi_manager.c
 * @author  29283
 * @brief   WiFi管理模块源文件
 * @created 2026/5/20
 *
 * 负责WiFi的初始化、连接、断线重连、状态通知。
 * 上层模块通过回调函数感知网络状态变化。
 */

#include "wifi_manager.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif.h"

/* 事件组bit定义 */
#define WIFI_CONNECTED_BIT  BIT0  /* 连接成功标志 */
#define WIFI_FAIL_BIT       BIT1  /* 连接失败标志 */

/* 全局变量 */
static EventGroupHandle_t wifi_event_group;   /* 事件组句柄 */
static int retry_count = 0;                    /* 当前重试次数 */
static int max_retry = 5;                      /* 最大重试次数 */
static wifi_status_t current_status = WIFI_DISCONNECTED;  /* 当前WiFi状态 */
static wifi_callback_t user_callback = NULL;   /* 用户注册的回调函数 */
static char local_ip[16] = "0.0.0.0";           /* 本机IP地址 */

/* 内部函数声明 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data);

/**
 * @brief WiFi事件处理函数（内部）
 *
 * ESP-IDF在WiFi状态发生变化时调用此函数。
 * 根据事件类型更新内部状态，并通过回调通知上层。
 *
 * @param arg: 用户参数（未使用）
 * @param event_base: 事件基类型（WIFI_EVENT或IP_EVENT）
 * @param event_id: 具体事件ID
 * @param event_data: 事件附带的数据
 * @retval None
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    /* ---- STA模式事件（WiFi本身状态变化） ---- */
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        /* WiFi硬件已准备就绪，开始连接路由器 */
        esp_wifi_connect();
        current_status = WIFI_CONNECTING;
        if (user_callback) user_callback(WIFI_CONNECTING);
        printf("[WiFi] 硬件就绪，正在连接...\r\n");

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        /* 连接断开（可能是连不上、断线、或被踢） */
        current_status = WIFI_DISCONNECTED;
        if (user_callback) user_callback(WIFI_DISCONNECTED);

        if (retry_count < max_retry) {
            /* 尚未超过最大重试次数，自动重连 */
            retry_count++;
            printf("[WiFi] 断开，第%d次重试...\r\n", retry_count);
            esp_wifi_connect();
        } else {
            /* 超过最大重试次数，通知上层连接失败 */
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
            printf("[WiFi] 连接失败：已达最大重试次数%d\r\n", max_retry);
        }

    /* ---- IP事件（成功获取IP才算真正连上网） ---- */
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        esp_ip4addr_ntoa(&event->ip_info.ip, local_ip, sizeof(local_ip));
        printf("[WiFi] 连接成功！IP地址: %s\r\n", local_ip);

        retry_count = 0;              /* 重置重试计数 */
        current_status = WIFI_CONNECTED;
        if (user_callback) user_callback(WIFI_CONNECTED);

        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief 初始化WiFi并开始连接
 *
 * 完整流程：
 * 1. 初始化NVS存储
 * 2. 初始化网络接口
 * 3. 注册事件回调
 * 4. 配置WiFi参数（SSID/密码）
 * 5. 启动WiFi
 * 6. 等待连接结果
 *
 * @param config: WiFi配置指针
 * @param callback: 状态变化回调函数指针（可传NULL）
 * @retval true  - 连接成功
 *         false - 连接失败或超时
 */
bool WiFi_Manager_Init(wifi_config_data_t *config, wifi_callback_t callback)
{
    /* 参数校验 */
    if (config == NULL || strlen(config->ssid) == 0) {
        printf("[WiFi] 错误：WiFi配置为空\r\n");
        return false;
    }

    user_callback = callback;
    max_retry = (config->max_retry > 0) ? config->max_retry : 5;

    /* 第一步：初始化NVS存储（WiFi固件需要） */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS分区被破坏或版本更新，擦除重建 */
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        printf("[WiFi] NVS初始化失败\r\n");
        return false;
    }

    /* 第二步：初始化网络接口 */
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    /* 第三步：初始化WiFi驱动 */
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_cfg);

    /* 第四步：注册事件回调 */
    esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    /* 第五步：配置WiFi参数 */
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    strncpy((char *)wifi_config.sta.ssid, config->ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, config->password, sizeof(wifi_config.sta.password));

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

    /* 第六步：创建事件组，启动WiFi，等待连接结果 */
    wifi_event_group = xEventGroupCreate();
    esp_wifi_start();

    /* 等待连接结果（最长等待10秒） */
    EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdTRUE, pdFALSE, pdMS_TO_TICKS(10000));

    if (bits & WIFI_CONNECTED_BIT) {
        printf("[WiFi] 初始化完成，已连接\r\n");
        return true;
    } else if (bits & WIFI_FAIL_BIT) {
        printf("[WiFi] 连接失败：请检查SSID和密码\r\n");
        return false;
    } else {
        printf("[WiFi] 连接超时（10秒），后台继续重试中...\r\n");
        return false;
    }
}

/**
 * @brief 获取当前WiFi状态
 * @param None
 * @retval wifi_status_t 当前状态
 */
wifi_status_t WiFi_Manager_GetStatus(void)
{
    return current_status;
}

/**
 * @brief 获取本机IP地址
 * @param ip: 输出缓冲区
 * @param size: 缓冲区大小
 * @retval true  - 获取成功
 *         false - 未连接
 */
bool WiFi_Manager_GetIP(char *ip, int size)
{
    if (current_status != WIFI_CONNECTED) {
        return false;
    }
    if (ip != NULL && size >= 16) {
        strncpy(ip, local_ip, size);
        return true;
    }
    return false;
}

/**
 * @brief 断开WiFi连接
 * @param None
 * @retval None
 */
void WiFi_Manager_Disconnect(void)
{
    esp_wifi_disconnect();
    esp_wifi_stop();
    current_status = WIFI_DISCONNECTED;
    if (user_callback) user_callback(WIFI_DISCONNECTED);
    printf("[WiFi] 已断开连接\r\n");
}
