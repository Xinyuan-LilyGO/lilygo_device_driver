/*
 * @Description: t_glasses_p4_driver
 * @Author: LILYGO_L
 * @Date: 2025-08-15 14:29:40
 * @LastEditTime: 2026-04-16 15:02:51
 * @License: GPL 3.0
 */

#pragma once

#include "esp32p4_driver.h"
#include "t_glasses_p4_config.h"

namespace lilygo_device_driver {
/**
 * @brief 通过 SDMMC 主机挂载 SD 卡。
 * @param base_path SD 卡挂载路径。
 * @param max_freq_khz SDMMC 总线最大频率，单位为 kHz。
 * @return SD 卡挂载成功时返回 true，否则返回 false。
 */
bool InitSdmmc(const char* base_path, int max_freq_khz = SDMMC_FREQ_DEFAULT);

/**
 * @brief 通过 SDSPI 主机挂载 SD 卡。
 * @param base_path SD 卡挂载路径。
 * @param host_id SD 卡使用的 SPI 主机。
 * @param max_freq_khz SDSPI 总线最大频率，单位为 kHz。
 * @return SD 卡挂载成功时返回 true，否则返回 false。
 */
bool InitSdspi(const char* base_path, spi_host_device_t host_id,
    int max_freq_khz = SDMMC_FREQ_DEFAULT);
}  // namespace lilygo_device_driver
