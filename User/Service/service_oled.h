/**
 * @file service_oled.h
 * @author  29283
 * @brief   OLED服务层头文件
 * @created 2026/4/23
 */

 #ifndef _SERVICE_OLED_H_
 #define _SERVICE_OLED_H_

 #include "app_data.h"

/* 函数声明 */
void OLED_Service_ShowWelcomePage(void);                    /* 显示欢迎页面 */
void OLED_Service_ShowMainPage(const app_data_t *data);     /* 显示主页面 */
void OLED_Service_ShowErrorPage(const char *error_msg);     /* 显示错误页面 */
void OLED_Service_ShowSensorData(const app_data_t *data);   /* 显示传感器数据 */

#endif //_SERVICE_OLED_H_