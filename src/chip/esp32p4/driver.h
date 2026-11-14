/*
 * @Description: ESP32-P4 片上外设驱动接口
 */
#pragma once

#include <cstdint>

namespace lilygo_device_driver {
bool InitLdoPower(uint8_t chan_id, uint32_t voltage_mv);
bool DeinitLdoPower(uint8_t chan_id);
}  // namespace lilygo_device_driver
