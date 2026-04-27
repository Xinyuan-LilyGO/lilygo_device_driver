/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-12-19 10:50:07
 * @LastEditTime: 2026-04-16 15:42:32
 * @License: GPL 3.0
 */
#pragma once

#include "config.h"

namespace lilygo_device_driver {
static constexpr uint16_t kMaxLogBufferSize = 1024;

enum class LogLevel {
  kDebug,  // debug信息
  kInfo,   // 普通信息

  kBus,   // 总线错误
  kChip,  // 芯片错误
};

void LogMessage(LogLevel level, const char* file_name, size_t line_number,
    const char* format, ...);
}  // namespace lilygo_device_driver
