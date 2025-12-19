/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-12-19 10:52:38
 * @LastEditTime: 2025-12-19 11:28:56
 * @License: GPL 3.0
 */
#pragma once

#include <iostream>
#include <memory>
#include <string.h>
#include <stdarg.h>

#if defined CONFIG_IDF_INIT_VERSION


#elif defined ARDUINO

#else
#error "development framework not selected"
#endif

#include "tool.h"

#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEBUG
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_INFO
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_CHIP
#define LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEVICE
