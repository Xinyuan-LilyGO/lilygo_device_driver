/*
 * @Description: lilygo_device_driver 编译配置与平台选择
 */
#pragma once

// 加载设备选择等构建配置，避免依赖其他头文件的包含顺序。
#if defined(ESP_PLATFORM)
#include "sdkconfig.h"
#endif

#define LILYGO_DEVICE_DRIVER_PLATFORM_ESP_IDF 1

// 当前仅支持 ESP-IDF 平台。
#if defined(ESP_PLATFORM)
#define LILYGO_DEVICE_DRIVER_PLATFORM LILYGO_DEVICE_DRIVER_PLATFORM_ESP_IDF

#else
#error "Unsupported build platform."
#endif
