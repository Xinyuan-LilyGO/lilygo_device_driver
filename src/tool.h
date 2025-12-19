/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-12-19 10:50:07
 * @LastEditTime: 2025-12-19 11:12:32
 * @License: GPL 3.0
 */
#pragma once

namespace Lilygo_Device_Driver
{
    static constexpr uint16_t ASSERT_LOG_MAX_LOG_BUFFER_SIZE = 1024;
    
    enum class Log_Level
    {
        DEBUG, // debug信息
        INFO,  // 普通信息

        DEVICE,  // 设备错误
        CHIP, // 芯片错误
    };

    void assert_log(Log_Level level, const char *file_name, size_t line_number, const char *format, ...);
}