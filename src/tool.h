/*
 * @Description: 设备驱动库日志工具接口
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
  kDebug,    // 调试信息
  kInfo,     // 普通信息
  kWarning,  // 警告信息
  kError,    // 错误信息
};

/**
 * @brief 在日志等级开启时输出格式化日志。
 * @param level 要检查并输出的日志等级。
 * @param file_name 日志对应的源文件名。
 * @param line_number 日志对应的源代码行号。
 * @param format printf 兼容的格式化字符串。
 */
void LogMessage(LogLevel level, const char* file_name, size_t line_number,
    const char* format, ...);
}  // namespace lilygo_device_driver
