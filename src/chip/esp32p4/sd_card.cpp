#include "sd_card.h"

#include <cstdio>

#include "core/logger.h"

namespace lilygo_device_driver {
namespace {

esp_vfs_fat_sdmmc_mount_config_t MakeMountConfig(
    const SdCard::MountConfig& config) {
  return {
      .format_if_mount_failed = config.format_if_mount_failed,
      .max_files = config.max_files,
      .allocation_unit_size = config.allocation_unit_size,
      .disk_status_check_enable = config.disk_status_check_enable,
      .use_one_fat = config.use_one_fat,
  };
}

}  // namespace

SdCard::~SdCard() {
  Deinit();
}

bool SdCard::InitSdmmc(const char* base_path, const SdmmcConfig& config) {
  if (!PrepareMount(base_path, config.host.max_freq_khz)) {
    return false;
  }
  const auto mount_config = MakeMountConfig(config.mount);
  const esp_err_t result = esp_vfs_fat_sdmmc_mount(
      base_path_.c_str(), &config.host, &config.slot, &mount_config, &card_);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "esp_vfs_fat_sdmmc_mount failed (error code: %#X)\n", result);
    card_ = nullptr;
    base_path_.clear();
    return false;
  }
  sdmmc_card_print_info(stdout, card_);
  return true;
}

bool SdCard::InitSdspi(const char* base_path, const SdspiConfig& config) {
  if (config.host.slot != config.slot.host_id) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "SDSPI host and slot do not match\n");
    return false;
  }
  if (!PrepareMount(base_path, config.host.max_freq_khz)) {
    return false;
  }
  if (config.initialize_bus) {
    const esp_err_t result = spi_bus_initialize(
        config.slot.host_id, &config.bus, SDSPI_DEFAULT_DMA);
    if (result != ESP_OK) {
      LogMessage(LogLevel::kError, __FILE__, __LINE__,
          "spi_bus_initialize failed (error code: %#X)\n", result);
      base_path_.clear();
      return false;
    }
    spi_host_id_ = config.slot.host_id;
    owns_spi_bus_ = true;
  }

  const auto mount_config = MakeMountConfig(config.mount);
  const esp_err_t result = esp_vfs_fat_sdspi_mount(
      base_path_.c_str(), &config.host, &config.slot, &mount_config, &card_);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "esp_vfs_fat_sdspi_mount failed (error code: %#X)\n", result);
    card_ = nullptr;
    base_path_.clear();
    ReleaseSpiBus();
    return false;
  }
  sdmmc_card_print_info(stdout, card_);
  return true;
}

bool SdCard::Deinit(bool release_bus) {
  if (card_ != nullptr) {
    const esp_err_t result =
        esp_vfs_fat_sdcard_unmount(base_path_.c_str(), card_);
    if (result != ESP_OK) {
      LogMessage(LogLevel::kError, __FILE__, __LINE__,
          "esp_vfs_fat_sdcard_unmount failed (error code: %#X)\n", result);
      return false;
    }
    card_ = nullptr;
  }
  base_path_.clear();
  return !release_bus || ReleaseSpiBus();
}

bool SdCard::IsReady() const {
  return card_ != nullptr && sdmmc_get_status(card_) == ESP_OK;
}

bool SdCard::PrepareMount(const char* base_path, int max_freq_khz) {
  if (base_path == nullptr || base_path[0] == '\0' || max_freq_khz <= 0) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "Invalid SD card mount configuration\n");
    return false;
  }
  if (IsMounted()) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "SD card is already mounted (path: %s)\n", base_path_.c_str());
    return false;
  }
  if (!ReleaseSpiBus()) {
    return false;
  }
  base_path_ = base_path;
  return true;
}

bool SdCard::ReleaseSpiBus() {
  if (!owns_spi_bus_) {
    return true;
  }
  const esp_err_t result = spi_bus_free(spi_host_id_);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "spi_bus_free failed (error code: %#X)\n", result);
    return false;
  }
  owns_spi_bus_ = false;
  return true;
}

}  // namespace lilygo_device_driver
