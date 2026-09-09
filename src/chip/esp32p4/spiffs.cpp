#include "spiffs.h"

#include "core/logger.h"

namespace lilygo_device_driver {

bool InitSpiffs(const SpiffsConfig& config) {
  if (config.base_path == nullptr || config.base_path[0] == '\0' ||
      config.max_files == 0) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "Invalid SPIFFS mount configuration\n");
    return false;
  }
  const esp_vfs_spiffs_conf_t conf = {
      .base_path = config.base_path,
      .partition_label = config.partition_label,
      .max_files = config.max_files,
      .format_if_mount_failed = config.format_if_mount_failed,
  };
  const esp_err_t result = esp_vfs_spiffs_register(&conf);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "esp_vfs_spiffs_register failed (error code: %#X)\n", result);
    return false;
  }

  size_t total = 0;
  size_t used = 0;
  const esp_err_t info_result =
      esp_spiffs_info(config.partition_label, &total, &used);
  if (info_result != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "esp_spiffs_info failed (error code: %#X)\n", info_result);
    DeinitSpiffs(config.partition_label);
    return false;
  }
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "Partition size: total: %zu bytes, used: %zu bytes\n", total, used);

  if (used > total) {
    const esp_err_t check_result = esp_spiffs_check(config.partition_label);
    if (check_result != ESP_OK) {
      LogMessage(LogLevel::kError, __FILE__, __LINE__,
          "esp_spiffs_check failed (error code: %#X)\n", check_result);
      DeinitSpiffs(config.partition_label);
      return false;
    }
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "esp_spiffs_check success\n");
  }
  return true;
}

bool DeinitSpiffs(const char* partition_label) {
  const esp_err_t result = esp_vfs_spiffs_unregister(partition_label);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "esp_vfs_spiffs_unregister failed (error code: %#X)\n", result);
    return false;
  }
  return true;
}

}  // namespace lilygo_device_driver
