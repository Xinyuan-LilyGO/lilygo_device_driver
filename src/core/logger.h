/*
 * @Description: 设备驱动库日志工具接口
 * @Author: LILYGO_L
 * @Date: 2025-12-19 10:50:07
 * @LastEditTime: 2026-04-16 15:42:32
 * @License: GPL 3.0
 */
#pragma once

#include <cstddef>
#include <cstdint>

namespace lilygo_device_driver {
enum class LogLevel : uint8_t {
  kDebug,    // 调试信息
  kInfo,     // 普通信息
  kWarning,  // 警告信息
  kError,    // 错误信息
  kNone,     // 禁止日志输出
};

/**
 * @brief 设置库级最低日志输出等级
 * @param level 最低日志等级，kNone 表示禁止全部日志
 */
void SetMinimumLogLevel(LogLevel level);

/**
 * @brief 获取库级最低日志输出等级
 * @return 当前最低日志等级
 */
LogLevel GetMinimumLogLevel();

/**
 * @brief 判断指定等级的日志当前是否允许输出
 * @param level 待判断的日志等级
 * @return 允许输出返回 true，否则返回 false
 */
bool ShouldLog(LogLevel level);

/**
 * @brief 按当前最低日志等级输出格式化日志
 * @param level 日志等级
 * @param file_name 源文件名
 * @param line_number 源代码行号
 * @param format printf 风格格式字符串
 */
void LogMessage(LogLevel level, const char* file_name, std::size_t line_number,
    const char* format, ...);
}  // namespace lilygo_device_driver
