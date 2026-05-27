/**
 * @file    service_config.c
 * @author  29283
 * @brief   配置持久化服务层源文件
 * @created 2026/5/27
 *
 * 使用 STM32 内部 Flash 最后一页（0x0800FC00）存储系统参数。
 * 擦写前会关闭中断，防止 Flash 操作期间指令预取失败。
 */

#include "service_config.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

#define CONFIG_MAGIC    0x5A5AA5A5  /* 魔数，标记配置有效 */

/**
 * @brief 计算配置结构的校验和（XOR 累加）
 * @param cfg: 配置结构体指针
 * @return 校验和
 */
static uint32_t calc_checksum(const config_t *cfg)
{
    const uint8_t *p = (const uint8_t *)cfg;
    uint32_t sum = 0;
    /* magic + light_threshold + reserved，不包含 checksum 本身 */
    for (int i = 0; i < (int)sizeof(config_t) - (int)sizeof(uint32_t); i++) {
        sum ^= p[i];
    }
    return sum;
}

/**
 * @brief 从 Flash 加载配置
 * @param cfg: 输出缓冲区
 * @retval true  - 加载成功（有效配置）
 *         false - Flash 中无有效配置
 */
bool Config_Service_Load(config_t *cfg)
{
    /* 直接从 Flash 地址读取 */
    const config_t *flash_cfg = (const config_t *)CONFIG_FLASH_ADDR;

    /* 检查魔数 */
    if (flash_cfg->magic != CONFIG_MAGIC) {
        printf("[CONFIG] Flash 中无有效配置，使用默认值\r\n");
        return false;
    }

    /* 校验 checksum */
    if (flash_cfg->checksum != calc_checksum(flash_cfg)) {
        printf("[CONFIG] 配置校验失败，使用默认值\r\n");
        return false;
    }

    /* 复制到输出缓冲区 */
    memcpy(cfg, flash_cfg, sizeof(config_t));
    printf("[CONFIG] 加载配置: threshold=%d\r\n", cfg->light_threshold);
    return true;
}

/**
 * @brief 保存配置到 Flash
 *
 * 先擦除整页，再逐字写入。擦写期间关中断。
 *
 * @param cfg: 要保存的配置
 * @retval true  - 保存成功
 *         false - 保存失败
 */
bool Config_Service_Save(config_t *cfg)
{
    HAL_StatusTypeDef hal_status;

    /* 填充校验字段 */
    cfg->magic = CONFIG_MAGIC;
    cfg->checksum = calc_checksum(cfg);

    /* Flash 解锁 */
    hal_status = HAL_FLASH_Unlock();
    if (hal_status != HAL_OK) {
        printf("[CONFIG] Flash 解锁失败\r\n");
        return false;
    }

    /* 擦除最后一页 */
    FLASH_EraseInitTypeDef erase = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .PageAddress = CONFIG_FLASH_ADDR,
        .NbPages = 1,
    };
    uint32_t page_error = 0;

    /* 擦除期间关中断（Flash 总线不能被中断打断） */
    __disable_irq();
    hal_status = HAL_FLASHEx_Erase(&erase, &page_error);
    if (hal_status != HAL_OK || page_error != 0xFFFFFFFFU) {
        __enable_irq();
        HAL_FLASH_Lock();
        printf("[CONFIG] 页擦除失败\r\n");
        return false;
    }

    /* 逐字写入配置 */
    const uint32_t *data = (const uint32_t *)cfg;
    for (int i = 0; i < (int)sizeof(config_t) / 4; i++) {
        hal_status = HAL_FLASH_Program(
            FLASH_TYPEPROGRAM_WORD,
            CONFIG_FLASH_ADDR + i * 4,
            data[i]
        );
        if (hal_status != HAL_OK) {
            __enable_irq();
            HAL_FLASH_Lock();
            printf("[CONFIG] 写入失败 (offset=%d)\r\n", i * 4);
            return false;
        }
    }

    __enable_irq();

    /* Flash 上锁 */
    HAL_FLASH_Lock();

    printf("[CONFIG] 已保存: threshold=%d\r\n", cfg->light_threshold);
    return true;
}
