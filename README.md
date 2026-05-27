# RTOS-EnvMonitor：基于 FreeRTOS 的物联网多任务环境监测终端

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![MCU](https://img.shields.io/badge/MCU-STM32F103C8T6-blue)](https://www.st.com/)
[![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS-green)](https://www.freertos.org/)
[![IoT](https://img.shields.io/badge/IoT-OneNET-orange)](https://open.iot.10086.cn/)

基于 **STM32F103C8T6 + FreeRTOS** 的物联网多任务环境监测终端。采集温湿度、光照等多维环境参数，通过 ESP32C6 WiFi MQTT 上报至 OneNET 云平台，**并支持云端远程下发指令控制设备**（LED、光照阈值等），实现完整的 MQTT 双向通信。

---

## 项目特色

- **全链路闭环**：`传感器 → STM32(FreeRTOS) → UART → ESP32 → WiFi → MQTT → OneNET 云端`
- **双向数据通信**：支持 MQTT 属性上报（设备→云端）和属性设置（云端→设备）全双工通信
- **工业级分层架构**：BSP 驱动层 → Service 服务层 → Task 任务层 → App 数据中心，硬件与业务高度解耦
- **双芯片通信**：STM32(数据采集控制) + ESP32(WiFi/MQTT)，真实产品常见设计
- **6参数实时上云**：温度、湿度、光照、蓝灯状态、绿灯状态、光照阈值，~1s 刷新至 OneNET 物模型
- **远程可调阈值**：光照阈值支持远程调整，断电不丢失（Flash 持久化）
- **离线缓存**：MQTT 断连时数据自动缓存，重连后补发，不丢点

---

## 数据流总览

### 上行（采集上报）
```
DHT11(温湿度) ──┐
                ├──→ SensorTask → 队列分发 ──→ ESP32Task ──→ USART2 ──→ ESP32C6
光照传感器(ADC) ─┘                                      ↑                    │
                                          LightTask(50ms写入共享变量)   WiFi → MQTT
                                                                              │
                                                                     OneNET 云端 ✅
```

### 下行（远程控制）
```
OneNET 应用模拟器 ──→ MQTT property/set ──→ ESP32C6
                                                │
                                              UART1
                                                │
                                          USART2(PA2/PA3)
                                                │
                                        ESP32C6_Driver_GetCommand()
                                                │
                                        Command_Service_Process()
                                           ├── GREEN:1/0 → 绿灯控制
                                           ├── BLUE:1/0  → 蓝灯控制
                                           └── THRESH:xxx → 光照阈值调整
```

---

## 技术栈

| 维度 | 技术选型 |
|:---|:---|
| **MCU** | STM32F103C8T6 (ARM Cortex-M3, 72MHz) |
| **RTOS** | FreeRTOS (CMSIS-RTOS V2 API) |
| **通信芯片** | ESP32C6 (WiFi + MQTT) |
| **云平台** | OneNET（中国移动物联网平台，物模型+OneJSON） |
| **通信协议** | UART (STM32↔ESP32) / MQTT (ESP32↔OneNET) |
| **外设** | DHT11(温湿度) / OLED 128×64(I2C) / 光敏电阻(ADC) / 按键(GPIO) / LED(GPIO) |
| **工具链** | VS Code + CMake + Ninja + arm-none-eabi-gcc + OpenOCD + DAP 烧录器 |
| **ESP32 工具链** | PlatformIO + ESP-IDF |

---

## 软件架构

```
┌─────────────────────────────────────────────────────────┐
│                    App 数据中心                          │
│     (app_data.h/c) 全局数据结构 + 队列/互斥锁声明         │
├─────────────────────────────────────────────────────────┤
│               Task 任务层 (task_*.c/h)                   │
│   SensorTask / UartTask / OledTask / ESP32Task / Light  │
├─────────────────────────────────────────────────────────┤
│             Service 服务层 (service_*.c/h)               │
│          业务逻辑、数据格式化、页面调度                    │
├─────────────────────────────────────────────────────────┤
│             BSP 驱动层 (bsp_*.c/h)                      │
│     DHT11单总线 / OLED_I2C / USART / ADC / LED / 按键   │
└─────────────────────────────────────────────────────────┘
```

## 任务设计

| 任务 | 优先级 | 栈大小 | 周期 | 功能 |
|:---|:---:|:---:|:---:|:---|
| **SensorTask** | AboveNormal(28) | 1KB | ~1s | DHT11 温湿度采集 + 数据分发到各队列 |
| **LightSensorTask** | Normal(24) | 1KB | 50ms | ADC 光照采样 + 写入共享变量 + 蓝灯自动控制 |
| **UartTask** | Normal(24) | 1KB | — | 串口数据打印 |
| **oledTask** | Low(8) | 2KB | — | OLED 屏幕刷新 |
| **esp32Task** | Low(8) | 1KB | 500ms轮询 | 从队列取数据→USART2→ESP32 发送；同时轮询下行命令 |

## IPC 通信机制

- **3个消息队列**：`sensor_to_uart_Queue` / `sensor_to_oled_Queue` / `uart_to_esp32_Queue`
- **1个互斥锁**：`printMutex` 保护多任务并发串口打印
- **1个全局共享变量**：`g_sensor_share`（LightTask 50ms写入 → SensorTask 1s读取后统一发出）
- **1个命令缓冲区**：ESP32 下行命令通过 `ESP32C6_Driver_GetCommand()` 非阻塞读取，`Command_Service_Process()` 解析执行

---

## 外设引脚分配

| 外设 | 引脚 | 说明 |
|:---|:---|:---|
| DHT11 | PB12 | 单总线温湿度传感器 |
| OLED | PB6(SCL) / PB7(SDA) | I2C1, SSD1306 128×64 |
| 光敏电阻 | PA1 | ADC1_IN1, 12位精度 |
| 蓝灯 | PB1 | 光照自动控制（>3500亮） |
| 绿灯 | PB0 | 按键手动切换 |
| 按键 | PA4 | 轮询 + 20ms 防抖 |
| USART1 | PA9(TX) / PA10(RX) | 调试日志 |
| USART2 | PA2(TX) / PA3(RX) | ↔ ESP32C6 UART1 |

### STM32 ↔ ESP32 接线

```
STM32 PA2(TX)  ────→  ESP32 GPIO6 (UART1_RX)
STM32 PA3(RX)  ←────  ESP32 GPIO7 (UART1_TX)
GND            ────→  GND
```

> 采用 ESP32 UART1(GPIO6/GPIO7) 而非 UART0，避免与烧录口冲突，烧录和通信互不干扰。

---

## 项目结构

```
├── Core/                    STM32CubeMX 生成代码 (HAL库)
│   ├── Inc/                头文件
│   └── Src/                初始化代码
├── Drivers/                 CMSIS / HAL 驱动库
├── User/
│   ├── App/                应用层 (app_data.h/c)
│   ├── Bsp/                硬件驱动层 (bsp_dht11/bsp_oled/bsp_light/bsp_uart)
│   ├── Service/            服务层 (service_esp32c6/service_light/service_command)
│   ├── Tasks/              任务层 (task_dht11/task_uart/task_oled/task_esp32c6/task_light)
│   └── Common/             通用工具 (led/button/delay)
├── ESP32_Src/              ESP32C6 源码 (PlatformIO 独立工程)
│   ├── main.c              WiFi → MQTT → UART 三步初始化
│   ├── wifi_manager.c/h    WiFi 管理模块
│   └── mqtt_manager.c/h    MQTT 管理模块
├── CMakeLists.txt
├── CMakePresets.json
└── README.md
```

---

## 踩坑与解决（面试常问）

| 问题 | 根因 | 解决方案 |
|:---|:---|:---|
| DHT11 频繁 errorData | LightTask 同级优先级打断微秒级单总线时序 | SensorTask 优先级 Normal → AboveNormal |
| ESP32 烧录需拔线 | RX/TX 接在 UART0，与烧录口冲突 | 改用 UART1(GPIO6/GPIO7)，烧录通信互不干扰 |
| OneNET 连不上 code=4 | VPN 劫持 DNS 到假 IP | 关梯子，用官方 Token 工具重新计算 |
| PA0 不能配 EXTI 中断 | PA0 是 WKUP 唤醒引脚，特殊行为 | 换 PA4，改为轮询 + 20ms 防抖 |
| CubeMX 重生丢变量 | CubeMX 重写 freertos.c 删除了手写代码 | 变量定义移到 app_data.c 中 |
| ESP32 数据粘包 | uart_read_bytes 一次性读 256 字节 | 改为逐字节读取 + `\n` 换行判定 |
| **OneNET 上线即掉线** | MQTT 用户名格式错误 + 持久会话冲突 | 用户名用纯产品ID；`disable_clean_session` 必须关掉 |
| **属性设置超时 10411** | ESP32 处理指令后未回复 set_reply | 添加 `property/set_reply` 回复，id 必须与原请求一致 |

## 版本历史

- **v2.8** (2026-05-27) — 新增离线缓存 + 参数持久化，补全 ESP32 源码到仓库
- **v2.5** (2026-05-27) — 新增 MQTT 下行控制（OneNET 属性设置→STM32 命令转发，支持远程 LED 控制 + 光照阈值调整），修复 OneNET 连接配置问题
- **v2.0** (2026-05-24) — 新增光照传感器(ADC)、按键、LED 控制、MQTT 上云扩展（5参数）
- **v1.0** (2026-04-23) — 初始搭建：DHT11 + OLED + 3任务 + 消息队列 IPC

---

## 本地开发

### 前置条件

- VS Code + CMake + arm-none-eabi-gcc + Ninja
- OpenOCD + DAP 烧录器
- PlatformIO（ESP32 开发）

### 一键烧录

按 `Ctrl+Shift+B` 默认执行一键烧录（编译 → 烧录 → 校验 → 复位）。

### ESP32 烧录

```bash
cd ESP32_Src/
py -m platformio run --target upload
```

---

## 作者

- **谢海峰** — 南昌航空大学 物联网工程
- GitHub: [@xiehaifeng717-ui](https://github.com/xiehaifeng717-ui)
- Email: xiehaifeng717@outlook.com

> 如果这个项目对你有帮助，欢迎 ⭐ 一下！
