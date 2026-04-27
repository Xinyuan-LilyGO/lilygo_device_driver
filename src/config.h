/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-12-19 10:52:38
 * @LastEditTime: 2026-04-16 15:36:43
 * @License: GPL 3.0
 */
#pragma once

#include <stdarg.h>
#include <string.h>

#include <iostream>
#include <memory>

#if defined CONFIG_IDF_INIT_VERSION
#define LILYGO_DEVICE_DRIVER_DEVELOPMENT_FRAMEWORK_ESPIDF

#include "driver/sdmmc_host.h"
#include "driver/spi_master.h"
#include "esp_spiffs.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#elif defined ARDUINO

#else
#error "Missing required macro definition."
#endif

#include "tool.h"

#if defined CONFIG_LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEBUG
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEBUG
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_INFO
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_CHIP
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEVICE
#elif defined CONFIG_LILYGO_DEVICE_DRIVER_LOG_LEVEL_INFO
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_INFO
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_CHIP
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEVICE
#elif defined CONFIG_LILYGO_DEVICE_DRIVER_LOG_LEVEL_ERROR
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_CHIP
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEVICE
#elif defined CONFIG_LILYGO_DEVICE_DRIVER_LOG_LEVEL_NONE
#else
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_INFO
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_CHIP
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEVICE
#endif
