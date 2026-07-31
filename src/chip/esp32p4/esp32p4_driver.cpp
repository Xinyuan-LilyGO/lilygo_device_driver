/*
 * @Description: ESP32-P4 片上外设驱动实现
 * @Author: LILYGO_L
 * @Date: 2025-12-18 17:59:32
 * @LastEditTime: 2026-04-23 17:52:12
 * @License: GPL 3.0
 */
#include "esp32p4_driver.h"

#include <array>

#include "esp_ldo_regulator.h"

namespace lilygo_device_driver {
namespace {

// ESP32-P4 共包含 4 路片上 LDO，数组下标与通道 ID 相差 1。
std::array<esp_ldo_channel_handle_t, 4> g_ldo_channel_handles = {};

}  // namespace

bool InitLdoPower(uint8_t chan_id, uint32_t voltage_mv) {
  if (chan_id == 0 || chan_id > g_ldo_channel_handles.size()) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "Invalid LDO channel: %u\n", static_cast<unsigned int>(chan_id));
    return false;
  }

  auto& ldo_channel_handle = g_ldo_channel_handles[chan_id - 1];
  if (ldo_channel_handle != nullptr) {
    const esp_err_t ret = esp_ldo_channel_adjust_voltage(
        ldo_channel_handle, static_cast<int>(voltage_mv));
    if (ret != ESP_OK) {
      LogMessage(LogLevel::kError, __FILE__, __LINE__,
          "esp_ldo_channel_adjust_voltage %d failed (error code: %#X)\n",
          chan_id, ret);
      return false;
    }
    return true;
  }

  esp_ldo_channel_config_t ldo_channel_config = {
      .chan_id = static_cast<int>(chan_id),
      .voltage_mv = static_cast<int>(voltage_mv),
      .flags =
          {
              .adjustable = 1,
              .owned_by_hw = 1,
              .bypass = 1,
          },
  };
  esp_err_t ret =
      esp_ldo_acquire_channel(&ldo_channel_config, &ldo_channel_handle);
  if (ret != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "esp_ldo_acquire_channel %d failed (error code: %#X)\n", chan_id, ret);
    return false;
  }

  return true;
}

bool DeinitLdoPower(uint8_t chan_id) {
  if (chan_id == 0 || chan_id > g_ldo_channel_handles.size()) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "Invalid LDO channel: %u\n", static_cast<unsigned int>(chan_id));
    return false;
  }

  auto& ldo_channel_handle = g_ldo_channel_handles[chan_id - 1];
  if (ldo_channel_handle == nullptr) {
    return true;
  }

  const esp_err_t ret = esp_ldo_release_channel(ldo_channel_handle);
  if (ret != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "esp_ldo_release_channel %d failed (error code: %#X)\n", chan_id, ret);
    return false;
  }

  ldo_channel_handle = nullptr;
  return true;
}
}  // namespace lilygo_device_driver
