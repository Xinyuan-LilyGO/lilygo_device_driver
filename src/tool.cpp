/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-12-19 10:50:16
 * @LastEditTime: 2026-04-16 15:43:18
 * @License: GPL 3.0
 */
#include "tool.h"

namespace lilygo_device_driver {
void LogMessage(LogLevel level, const char* file_name, size_t line_number,
    const char* format, ...) {
#if defined(LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEVICE) || \
    defined(LILYGO_DEVICE_DRIVER_LOG_LEVEL_CHIP) ||   \
    defined(LILYGO_DEVICE_DRIVER_LOG_LEVEL_INFO) ||   \
    defined(LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEBUG)

  switch (level) {
#if defined(LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEBUG)
    case LogLevel::kDebug: {
      va_list args;
      va_start(args, format);
      auto buffer = std::make_unique<char[]>(kMaxLogBufferSize);
      snprintf(buffer.get(), kMaxLogBufferSize,
          "[lilygo_device_driver log][Debug]->[%s][%u line]: %s", file_name,
          line_number, format);
      vprintf(buffer.get(), args);
      va_end(args);

      break;
    }
#endif
#if defined(LILYGO_DEVICE_DRIVER_LOG_LEVEL_INFO)
    case LogLevel::kInfo: {
      va_list args;
      va_start(args, format);
      auto buffer = std::make_unique<char[]>(kMaxLogBufferSize);
      snprintf(buffer.get(), kMaxLogBufferSize,
          "[lilygo_device_driver log][Info]->[%s][%u line]: %s", file_name,
          line_number, format);
      vprintf(buffer.get(), args);
      va_end(args);

      break;
    }
#endif
#if defined(LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEVICE)
    case LogLevel::kDebug: {
      va_list args;
      va_start(args, format);
      auto buffer = std::make_unique<char[]>(kMaxLogBufferSize);
      snprintf(buffer.get(), kMaxLogBufferSize,
          "[lilygo_device_driver log][Device]->[%s][%u line]: %s", file_name,
          line_number, format);
      vprintf(buffer.get(), args);
      va_end(args);

      break;
    }
#endif
#if defined(LILYGO_DEVICE_DRIVER_LOG_LEVEL_CHIP)
    case LogLevel::kChip: {
      va_list args;
      va_start(args, format);
      auto buffer = std::make_unique<char[]>(kMaxLogBufferSize);
      snprintf(buffer.get(), kMaxLogBufferSize,
          "[lilygo_device_driver log][Chip]->[%s][%u line]: %s", file_name,
          line_number, format);
      vprintf(buffer.get(), args);
      va_end(args);

      break;
    }
#endif
    default:
      break;
  }

#endif
}

}  // namespace lilygo_device_driver
