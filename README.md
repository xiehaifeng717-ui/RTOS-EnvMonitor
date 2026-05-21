# RTOS-EnvMonitor

> 基于 STM32F103C8T6 + FreeRTOS 的温湿度实时监测系统，支持 WiFi 上云

## 项目概览

| 项目 | 说明 |
|---|---|
| **硬件** | STM32F103C8T6 + DHT11 + OLED(0.96'') + ESP32C6 |
| **RTOS** | FreeRTOS (CMSIS-RTOS V2) |
| **通信** | STM32 ↔ ESP32C6 (USART2 ↔ UART1, GPIO6/GPIO7) |
| **联网** | ESP32C6 WiFi + MQTT |
| **云端** | OneNET 物联网平台 |
| **工具链** | VS Code + CMake + arm-none-eabi-gcc + OpenOCD / PlatformIO + ESP-IDF |

## 完整数据流

```
DHT11 → STM32 (FreeRTOS)
         ├→ OLED 显示温湿度
         ├→ USART1 串口打印
         └→ USART2 → ESP32C6 GPIO6(RX)
                       ↓
              WiFi → MQTT → OneNET 云端
```

## 软件架构

```
分层设计（自下而上）：
┌──────────────────────────────────┐
│  App 数据中心（app_data.h/c）    │
│  全局数据结构 + 消息队列/互斥锁   │
├──────────────────────────────────┤
│  Task 任务层（task_*.c/h）        │
│  FreeRTOS 任务函数入口            │
├──────────────────────────────────┤
│  Service 服务层（service_*.c/h）   │
│  业务逻辑、数据格式转换            │
├──────────────────────────────────┤
│  BSP 驱动层（bsp_*.c/h）          │
│  外设底层操作（DHT11/OLED/UART）  │
└──────────────────────────────────┘
```

## FreeRTOS 任务

| 任务 | 优先级 | 栈大小 | 功能 |
|---|---|---|---|
| SensorTask | Normal | 1024B | 每秒采集 DHT11 数据，分发到消息队列 |
| UartTask | Normal | 1024B | 从队列接收数据，USART1 打印调试日志 |
| OledTask | Low | 2048B | 从队列接收数据，OLED 显示温湿度 |
| ESP32Task | Low | 1024B | 从队列接收数据，通过 USART2 转发给 ESP32C6 |

## 硬件接线

### STM32 外设

| 外设 | 引脚 | 协议 |
|---|---|---|
| DHT11 | PB12 | 单总线 |
| OLED | PB6(SCL) / PB7(SDA) | I2C1 |
| USART1(调试) | PA9(TX) / PA10(RX) | 115200 |
| USART2(→ESP32) | PA2(TX) / PA3(RX) | 115200 |

### STM32 ↔ ESP32C6 接线

```
STM32 PA2 (TX) ────→ ESP32C6 GPIO6 (UART1_RX)
STM32 PA3 (RX) ────→ ESP32C6 GPIO7 (UART1_TX)
GND ────→ GND
```

> **注意：** 不要接板上标 RX/TX 的引脚（那是 UART0，与烧录冲突）

## 快速开始

### STM32 端

```bash
cd Project_1_FreeRTOS/project_1
cmake --preset Debug
cmake --build --preset Debug
# 烧录：连接 DAP 烧录器，运行 openocd
```

VS Code 快捷键：`Ctrl+Shift+B` 编译 + 一键烧录

### ESP32C6 端

```bash
cd ESP_project/test_project/test
py -m platformio run --target upload
```

VS Code 快捷键：`Ctrl+Shift+B` 编译 + 烧录

> **注意：** 烧录前把 `main.c` 里的 WiFi 名和密码改成你自己的

### MQTT 配置

代码默认连接 EMQX 公共 Broker。如需切换 OneNET：

```c
#define MQTT_BROKER     "mqtt://studio-mqtt.heclouds.com:1883"
#define MQTT_CLIENT_ID  "stm32_esp32_dht11"
#define MQTT_TOPIC      "$sys/{产品ID}/{设备名}/thing/property/post"
```

OneNET 需要 Token 认证，使用官方 Token 工具计算。

## 技术栈

- **MCU**: STM32F103C8T6 (ARM Cortex-M3)
- **RTOS**: FreeRTOS (CMSIS-RTOS V2)
- **WiFi**: ESP32C6 (ESP-IDF)
- **协议**: I2C / USART / MQTT
- **工具**: VS Code / CMake / Ninja / OpenOCD / PlatformIO

## 相关项目

- [Smart_Gateway](https://github.com/xiehaifeng717-ui/Smart_Gateway) — Linux C 多线程网关学习项目

## License

MIT
