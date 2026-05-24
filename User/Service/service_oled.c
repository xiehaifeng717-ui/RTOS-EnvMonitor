/**
 * @file    service_oled.c
 * @author  29283
 * @brief   OLED服务层实现文件
 * @created 2026/4/23
 */

#include "service_oled.h"
#include "bsp_oled.h"

/**
 * @brief 显示欢迎页面
 * @param None
 * @retval None
 */
void OLED_Service_ShowWelcomePage(void) {
    OLED_Driver_Init(); 
    OLED_Driver_Clear();
    OLED_Driver_ShowString(24, 1, "Welcome to");
    OLED_Driver_ShowString(32, 4, "FreeRTOS");
    OLED_Driver_Refresh();
}

/**
 * @brief 显示主页面
 * @param data: 指向应用数据的指针
 * @retval None
 */
void OLED_Service_ShowMainPage(const app_data_t *data) {
    OLED_Driver_Clear();
    OLED_Driver_ShowString(40, 3, "Main Page");
    OLED_Driver_Refresh();
}

/**
 * @brief 显示传感器数据
 * @param data: 指向应用数据的指针
 * @retval None
 */
void OLED_Service_ShowSensorData(const app_data_t *data){
    OLED_Driver_Clear();

    OLED_Driver_ShowString(16, 0, "Temp: ");
    OLED_Driver_ShowChar(104, 0, 'C');
    OLED_Driver_ShowFloat(64, 0, data->temperature, 2, 1);

    OLED_Driver_ShowString(16, 2, "Humi: ");
    OLED_Driver_ShowChar(104, 2, '%');
    OLED_Driver_ShowFloat(64, 2, data->humidity, 2, 1);

    OLED_Driver_ShowString(16, 4, "Time: ");
    OLED_Driver_ShowChar(104, 4, 's');
    OLED_Driver_ShowNum(64, 4, data->time, 4);

    OLED_Driver_ShowString(0, 6, "Status: ");
    OLED_Driver_ShowString(64, 6, data->status ? "True" : "False");

    OLED_Driver_Refresh();
}

/**
 * @brief 显示错误页面
 * @param error_msg: 错误信息字符串
 * @retval None
 */
void OLED_Service_ShowErrorPage(const char *error_msg) {
    OLED_Driver_Clear();
    OLED_Driver_ShowString(16, 3, "Error:");
    OLED_Driver_ShowString(72, 3, error_msg);
    OLED_Driver_Refresh();
}