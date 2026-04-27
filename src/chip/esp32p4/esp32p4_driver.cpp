/*
 * @Description: esp32p4_driver
 * @Author: LILYGO_L
 * @Date: 2025-12-18 17:59:32
 * @LastEditTime: 2026-04-23 17:52:12
 * @License: GPL 3.0
 */
#include "esp32p4_driver.h"

#include "esp_ldo_regulator.h"

namespace lilygo_device_driver {
bool InitLdoPower(uint8_t chan_id, uint32_t voltage_mv) {
  esp_ldo_channel_handle_t ldo_channel_handle = nullptr;
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
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "esp_ldo_acquire_channel %d failed (error code: %#X)\n", chan_id, ret);
    return false;
  }

  return true;
}
}  // namespace lilygo_device_driver
