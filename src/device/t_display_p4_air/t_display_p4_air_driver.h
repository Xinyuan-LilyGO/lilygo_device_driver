/*
 * @Description: t_display_p4_air_driver
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-04-16 15:02:38
 * @License: GPL 3.0
 */

#pragma once

#include "esp32p4_driver.h"
#include "t_display_p4_air_config.h"

#if defined(CONFIG_SCREEN_PIXEL_FORMAT_RGB565)
#define SCREEN_BITS_PER_PIXEL 16
#elif defined(CONFIG_SCREEN_PIXEL_FORMAT_RGB888)
#define SCREEN_BITS_PER_PIXEL 24
#else
#error "Missing required macro definition."
#endif

#if defined(CONFIG_CAMERA_PIXEL_FORMAT_RGB565)
#define CAMERA_BITS_PER_PIXEL 16
#elif defined(CONFIG_CAMERA_PIXEL_FORMAT_RGB888)
#define CAMERA_BITS_PER_PIXEL 24
#else
#error "Missing required macro definition."
#endif

// SCREEN
#define SCREEN_WIDTH HI8561_SCREEN_WIDTH
#define SCREEN_HEIGHT HI8561_SCREEN_HEIGHT
#define SCREEN_MIPI_DSI_DPI_CLK_MHZ HI8561_SCREEN_MIPI_DSI_DPI_CLK_MHZ
#define SCREEN_MIPI_DSI_HSYNC HI8561_SCREEN_MIPI_DSI_HSYNC
#define SCREEN_MIPI_DSI_HBP HI8561_SCREEN_MIPI_DSI_HBP
#define SCREEN_MIPI_DSI_HFP HI8561_SCREEN_MIPI_DSI_HFP
#define SCREEN_MIPI_DSI_VSYNC HI8561_SCREEN_MIPI_DSI_VSYNC
#define SCREEN_MIPI_DSI_VBP HI8561_SCREEN_MIPI_DSI_VBP
#define SCREEN_MIPI_DSI_VFP HI8561_SCREEN_MIPI_DSI_VFP
#define SCREEN_DATA_LANE_NUM HI8561_SCREEN_DATA_LANE_NUM
#define SCREEN_LANE_BIT_RATE_MBPS HI8561_SCREEN_LANE_BIT_RATE_MBPS

namespace lilygo_device_driver {
/**
 * @brief 挂载 SPIFFS 文件系统。
 * @param base_path 文件系统挂载路径。
 * @param spiffs_conf 返回挂载时使用的 SPIFFS 配置。
 * @return SPIFFS 挂载成功时返回 true，否则返回 false。
 */
bool InitSpiffs(const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf);

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
