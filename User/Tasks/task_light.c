/**
 * @file    task_light.c
 * @author  29283
 * @brief   光照传感器任务层源文件
 * @created 2026/5/24
 */

#include "task_light.h"
#include "service_light.h"
#include "led.h"
#include "button.h"
#include "app_data.h"
#include "cmsis_os2.h"
#include <stdio.h>

/* 光照阈值变量：ADC高于此值判定为天黑，自动开灯 */
/* 初始值3500（老梅调好的），可通过 MQTT 下行远程调整 */
uint16_t g_light_threshold = 3500;

/**
 * @brief 光照传感器任务函数
 * @param argument: 任务参数
 * @retval None
 */
void light_sensor_task(void *argument) {
    /* 初始化硬件 */
    LED_Blue_Init();
    LED_Green_Init();
    Button_Init();

    /* 打印任务启动信息 */
    osMutexAcquire(printMutex, osWaitForever);
    printf("Light Sensor Task Started\r\n");
    osMutexRelease(printMutex);

    light_sample_t light_sample;
    uint32_t print_counter = 0;
    bool green_led_state = false;   /* 绿灯状态跟踪 */

    for (;;) {
        /* 清空数据结构 */
        light_sample = (light_sample_t){0};

        /* 检测按键按下（轮询+防抖，按一次切换一次绿灯） */
        if (Button_WasPressed()) {
            LED_Green_Toggle();
            green_led_state = !green_led_state;
            g_sensor_share.green_led = green_led_state ? 1 : 0;
            osMutexAcquire(printMutex, osWaitForever);
            printf("Button Pressed, Green LED Toggled\r\n");
            osMutexRelease(printMutex);
        }

        /* 读取光照传感器数据 */
        if (Light_Service_ReadSample(&light_sample)) {
            /* 蓝灯自动控制：天黑开灯，天亮关灯 */
            if (light_sample.adc_raw > g_light_threshold) {
                LED_Blue_On();
            } else {
                LED_Blue_Off();
            }

            /* 写入共享数据区（SensorTask每秒读取后统一发送） */
            g_sensor_share.light_adc = light_sample.adc_raw;
            g_sensor_share.blue_led = (light_sample.adc_raw > g_light_threshold) ? 1 : 0;

            /* 每20次（约1秒）串口打印一次光照值，与DHT11频率对齐 */
            print_counter++;
            if (print_counter >= 20) {
                print_counter = 0;
                osMutexAcquire(printMutex, osWaitForever);
                printf("Light ADC: %d (threshold: %d)\r\n",
                       light_sample.adc_raw, g_light_threshold);
                osMutexRelease(printMutex);
            }
        }

        /* 每50ms循环一次，保证按键响应速度 */
        osDelay(50);
    }
}
