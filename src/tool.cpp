/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-12-19 10:50:16
 * @LastEditTime: 2025-12-19 11:28:45
 * @License: GPL 3.0
 */
#include "tool.h"

namespace Lilygo_Device_Driver
{
    void assert_log(Log_Level level, const char *file_name, size_t line_number, const char *format, ...)
    {
#if defined LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEVICE || defined LILYGO_DEVICE_DRIVER_LOG_LEVEL_CHIP || \
    defined LILYGO_DEVICE_DRIVER_LOG_LEVEL_INFO || defined LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEBUG

        switch (level)
        {
#if defined LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEBUG
        case Log_Level::DEBUG:
        {
            va_list args;
            va_start(args, format);
            auto buffer = std::make_unique<char[]>(ASSERT_LOG_MAX_LOG_BUFFER_SIZE);
            snprintf(buffer.get(), ASSERT_LOG_MAX_LOG_BUFFER_SIZE, "[lilygo_device_driver][log debug]->[%s][%u line]: %s", file_name, line_number, format);
            vprintf(buffer.get(), args);
            va_end(args);

            break;
        }
#endif
#if defined LILYGO_DEVICE_DRIVER_LOG_LEVEL_INFO
        case Log_Level::INFO:
        {
            va_list args;
            va_start(args, format);
            auto buffer = std::make_unique<char[]>(ASSERT_LOG_MAX_LOG_BUFFER_SIZE);
            snprintf(buffer.get(), ASSERT_LOG_MAX_LOG_BUFFER_SIZE, "[lilygo_device_driver][log info]->[%s][%u line]: %s", file_name, line_number, format);
            vprintf(buffer.get(), args);
            va_end(args);

            break;
        }
#endif
#if defined LILYGO_DEVICE_DRIVER_LOG_LEVEL_DEVICE
        case Log_Level::DEVICE:
        {
            va_list args;
            va_start(args, format);
            auto buffer = std::make_unique<char[]>(ASSERT_LOG_MAX_LOG_BUFFER_SIZE);
            snprintf(buffer.get(), ASSERT_LOG_MAX_LOG_BUFFER_SIZE, "[lilygo_device_driver][log device]->[%s][%u line]: %s", file_name, line_number, format);
            vprintf(buffer.get(), args);
            va_end(args);

            break;
        }
#endif
#if defined LILYGO_DEVICE_DRIVER_LOG_LEVEL_CHIP
        case Log_Level::CHIP:
        {
            va_list args;
            va_start(args, format);
            auto buffer = std::make_unique<char[]>(ASSERT_LOG_MAX_LOG_BUFFER_SIZE);
            snprintf(buffer.get(), ASSERT_LOG_MAX_LOG_BUFFER_SIZE, "[lilygo_device_driver][log chip]->[%s][%u line]: %s", file_name, line_number, format);
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

}
