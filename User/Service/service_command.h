/**
 * @file    service_command.h
 * @author  29283
 * @brief   命令解析服务层头文件
 * @created 2026/5/27
 *
 * 解析从 ESP32 下行的控制命令，执行对应操作。
 * 下行命令格式（一行一个）：
 *   GREEN:1    — 开绿灯
 *   GREEN:0    — 关绿灯
 *   BLUE:1     — 开蓝灯
 *   BLUE:0     — 关蓝灯
 *   THRESH:xxx — 设置光照阈值
 */

#ifndef _SERVICE_COMMAND_H_
#define _SERVICE_COMMAND_H_

/* 函数声明 */
void Command_Service_Process(const char *cmd);  /* 解析并执行下行命令 */

#endif //_SERVICE_COMMAND_H_
