/**
 * @file    task_oled.c
 * @author  29283
 * @brief   OLED任务层源文件
 * @created 2026/4/23
 */

#include "task_oled.h"
#include "service_oled.h"

/**
 * @brief OLED任务函数
 * @param argument: 任务参数
 * @retval None
 */
void oled_task(void *argument) {
    app_data_t app_data;
    /* 显示欢迎页面 */
    OLED_Service_ShowWelcomePage();
    osDelay(2000);  /* 显示2秒后进入主页面 */

    /* 无限循环，保持OLED显示 */
    for(;;) {
        if(osMessageQueueGet(sensor_to_oled_Queue, &app_data, NULL, osWaitForever) == osOK) {
            if (app_data.status) {
                OLED_Service_ShowSensorData(&app_data);
            } else {
                OLED_Service_ShowErrorPage("Data");
            }
        } else {
            OLED_Service_ShowErrorPage("Queue");
        }
    }
}