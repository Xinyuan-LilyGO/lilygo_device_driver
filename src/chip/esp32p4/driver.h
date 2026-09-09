/*
 * @Description: ESP32-P4 片上外设驱动接口
 * @Author: LILYGO_L
 * @Date: 2025-12-18 17:59:41
 * @LastEditTime: 2026-08-03 16:14:01
 * @License: GPL 3.0
 */
#pragma once

#include <cstdint>

namespace lilygo_device_driver {
bool InitLdoPower(uint8_t chan_id, uint32_t voltage_mv);
bool DeinitLdoPower(uint8_t chan_id);
}  // namespace lilygo_device_driver
