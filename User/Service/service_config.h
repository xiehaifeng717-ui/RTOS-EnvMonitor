/**
 * @file    service_config.h
 * @author  29283
 * @brief   配置持久化服务层头文件
 * @created 2026/5/27
 *
 * 将系统参数（光照阈值等）存入 STM32 内部 Flash 最后一页，
 * 实现掉电不丢失。
 */

#ifndef _SERVICE_CONFIG_H_
#define _SERVICE_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

/* Flash 存储地址：最后一页（0x0800FC00，1KB） */
#define CONFIG_FLASH_ADDR   0x0800FC00

/* 配置数据结构体 */
typedef struct {
    uint32_t magic;              /* 魔数 0x5A5AA5A5 */
    uint16_t light_threshold;    /* 光照阈值 */
    uint16_t reserved;           /* 预留 */
    uint32_t checksum;           /* 校验和 */
} config_t;

/* 函数声明 */
bool Config_Service_Load(config_t *cfg);     /* 从 Flash 加载配置 */
bool Config_Service_Save(config_t *cfg);     /* 保存配置到 Flash */

#endif /* _SERVICE_CONFIG_H_ */
