/*
 * @Description: t_display_p4_air_driver
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-02-24 16:27:17
 * @License: GPL 3.0
 */

#pragma once

#include "t_display_p4_air_config.h"
#include "esp32p4_driver.h"

#if defined CONFIG_SCREEN_PIXEL_FORMAT_RGB565
#define SCREEN_BITS_PER_PIXEL 16
#elif defined CONFIG_SCREEN_PIXEL_FORMAT_RGB888
#define SCREEN_BITS_PER_PIXEL 24
#else
#error "no macro definition is set"
#endif

#if defined CONFIG_CAMERA_PIXEL_FORMAT_RGB565
#define CAMERA_BITS_PER_PIXEL 16
#elif defined CONFIG_CAMERA_PIXEL_FORMAT_RGB888
#define CAMERA_BITS_PER_PIXEL 24
#else
#error "no macro definition is set"
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

namespace Lilygo_Device_Driver
{
    bool Spiffs_Init(const char *base_path, esp_vfs_spiffs_conf_t &spiffs_conf);
}
