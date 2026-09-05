/*
 * @Description: 设备驱动库日志工具实现
 * @Author: LILYGO_L
 * @Date: 2025-12-19 10:50:16
 * @LastEditTime: 2026-04-16 15:43:18
 * @License: GPL 3.0
 */
#include "logger.h"

#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <new>

#include "lilygo_device_driver_config.h"

namespace lilygo_device_driver {
namespace {

// 日志正文缓冲区上限，包含结尾空字符；过长正文会被截断。
constexpr std::size_t kMaxLogBufferSize = 1024;

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEBUG)
constexpr LogLevel kDefaultMinimumLogLevel = LogLevel::kDebug;
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_LOG_LEVEL_INFO)
constexpr LogLevel kDefaultMinimumLogLevel = LogLevel::kInfo;
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_LOG_LEVEL_WARNING)
constexpr LogLevel kDefaultMinimumLogLevel = LogLevel::kWarning;
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_LOG_LEVEL_ERROR)
constexpr LogLevel kDefaultMinimumLogLevel = LogLevel::kError;
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_LOG_LEVEL_NONE)
constexpr LogLevel kDefaultMinimumLogLevel = LogLevel::kNone;
#else
constexpr LogLevel kDefaultMinimumLogLevel = LogLevel::kInfo;
#endif

// 各任务共享最低日志等级，使用原子访问避免并发读写冲突。
std::atomic<LogLevel> g_minimum_log_level{kDefaultMinimumLogLevel};

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
    case LogLevel::kWarning:
      return "Warning";
    case LogLevel::kError:
      return "Error";
    case LogLevel::kNone:
      return "None";
    default:
      return "Unknown";
  }
}

}  // namespace

void SetMinimumLogLevel(LogLevel level) {
  if (level > LogLevel::kNone) {
    level = LogLevel::kNone;
  }
  g_minimum_log_level.store(level, std::memory_order_relaxed);
}

LogLevel GetMinimumLogLevel() {
  return g_minimum_log_level.load(std::memory_order_relaxed);
}

bool ShouldLog(LogLevel level) {
  if (level > LogLevel::kError) {
    return false;
  }
  const LogLevel minimum_level = GetMinimumLogLevel();
  return minimum_level != LogLevel::kNone && level >= minimum_level;
}

void LogMessage(LogLevel level, const char* file_name, std::size_t line_number,
    const char* format, ...) {
  if (!ShouldLog(level) || format == nullptr) {
    return;
  }

  // 不占用任务栈上的大缓冲区，分配失败时跳过日志而不抛出异常。
  std::unique_ptr<char[]> buffer(new (std::nothrow) char[kMaxLogBufferSize]);
  if (!buffer) {
    return;
  }

  va_list args;
  va_start(args, format);
  const int length = std::vsnprintf(buffer.get(), kMaxLogBufferSize, format, args);
  va_end(args);
  if (length < 0) {
    return;
  }

  // 正文先完成格式化，避免截断格式占位符或将文件名当作格式串解析。
  std::printf("[lilygo_device_driver log][%s]->[%s][%zu line]: %s",
      LogLevelName(level), file_name != nullptr ? file_name : "Unknown",
      line_number, buffer.get());
}

}  // namespace lilygo_device_driver
