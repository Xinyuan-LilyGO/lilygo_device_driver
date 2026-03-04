/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-12-19 10:52:38
 * @LastEditTime: 2026-03-04 09:27:04
 * @License: GPL 3.0
 */
#pragma once

#include <iostream>
#include <memory>
#include <string.h>
#include <stdarg.h>

#if defined CONFIG_IDF_INIT_VERSION
#define LILYGO_DEVICE_DRIVER_DEVELOPMENT_FRAMEWORK_ESPIDF

#include "driver/sdmmc_host.h"
#include "driver/spi_master.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "sdmmc_cmd.h"

#elif defined ARDUINO

#else
#error "no macro definition is set"
#endif

#include "tool.h"

#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEBUG
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_INFO
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_CHIP
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEVICE
