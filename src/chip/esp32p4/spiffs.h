#pragma once

#include <cstddef>

#include "esp_spiffs.h"

namespace lilygo_device_driver {

struct SpiffsConfig {
  const char* base_path = nullptr;
  const char* partition_label = nullptr;
  size_t max_files = 5;
  // 仅在明确允许时，才由 ESP-IDF 在挂载失败后尝试格式化。
  bool format_if_mount_failed = false;
};

bool InitSpiffs(const SpiffsConfig& config);
bool DeinitSpiffs(const char* partition_label = nullptr);

}  // namespace lilygo_device_driver
