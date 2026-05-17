/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-12-19 10:50:16
 * @LastEditTime: 2026-04-16 15:43:18
 * @License: GPL 3.0
 */
#include "tool.h"

namespace lilygo_device_driver {
namespace {

/**
 * @brief 获取日志等级的可打印名称。
 * @param level 要描述的日志等级。
 * @return 日志等级的可打印名称。
 */
const char* LogLevelName(LogLevel level) {
  switch (level) {
    case LogLevel::kDebug:
      return "Debug";
    case LogLevel::kInfo:
      return "Info";
    case LogLevel::kBus:
      return "Bus";
    case LogLevel::kChip:
      return "Chip";
    default:
      return "Unknown";
  }
}

/**
 * @brief 检查日志等级是否已开启。
 * @param level 要检查的日志等级。
 * @return 日志等级已开启时返回 true，否则返回 false。
 */
bool IsLogLevelEnabled(LogLevel level) {
  switch (level) {
#if defined(LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEBUG)
    case LogLevel::kDebug:
      return true;
#endif
#if defined(LILYGO_DEVICE_DRIVER_LOG_LEVEL_INFO)
    case LogLevel::kInfo:
      return true;
#endif
#if defined(LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEVICE)
    case LogLevel::kBus:
      return true;
#endif
#if defined(LILYGO_DEVICE_DRIVER_LOG_LEVEL_CHIP)
    case LogLevel::kChip:
      return true;
#endif
    default:
      return false;
  }
}

}  // namespace

void LogMessage(LogLevel level, const char* file_name, size_t line_number,
    const char* format, ...) {
  if (!IsLogLevelEnabled(level)) {
    return;
  }

  va_list args;
  va_start(args, format);
  auto buffer = std::make_unique<char[]>(kMaxLogBufferSize);
  snprintf(buffer.get(), kMaxLogBufferSize,
      "[lilygo_device_driver log][%s]->[%s][%u line]: %s", LogLevelName(level),
      file_name, static_cast<unsigned int>(line_number), format);
  vprintf(buffer.get(), args);
  va_end(args);
}

}  // namespace lilygo_device_driver
