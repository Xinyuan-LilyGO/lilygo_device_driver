#pragma once

#include <cstddef>
#include <string>

#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "driver/spi_master.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

namespace lilygo_device_driver {

class SdCard {
 public:
  struct MountConfig {
    bool format_if_mount_failed = false;
    int max_files = 5;
    size_t allocation_unit_size = 16 * 1024;
    bool disk_status_check_enable = false;
    bool use_one_fat = false;
  };

  struct SdmmcConfig {
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot = SDMMC_SLOT_CONFIG_DEFAULT();
    MountConfig mount;
  };

  struct SdspiConfig {
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_device_config_t slot = SDSPI_DEVICE_CONFIG_DEFAULT();
    spi_bus_config_t bus = {
        .mosi_io_num = -1,
        .miso_io_num = -1,
        .sclk_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .data_io_default_level = 0,
        .max_transfer_sz = 0,
        .flags = SPICOMMON_BUSFLAG_MASTER,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
        .intr_flags = 0,
    };
    MountConfig mount;
    // false 使用调用方已初始化的 SPI 总线，本对象不负责释放。
    bool initialize_bus = true;
  };

  SdCard() = default;
  ~SdCard();

  SdCard(const SdCard&) = delete;
  SdCard& operator=(const SdCard&) = delete;

  bool InitSdmmc(const char* base_path, const SdmmcConfig& config);
  bool InitSdspi(const char* base_path, const SdspiConfig& config);
  bool Deinit(bool release_bus = true);

  bool IsReady() const;
  bool IsMounted() const { return card_ != nullptr; }
  const std::string& base_path() const { return base_path_; }

 private:
  bool PrepareMount(const char* base_path, int max_freq_khz);
  bool ReleaseSpiBus();

  sdmmc_card_t* card_ = nullptr;
  std::string base_path_;
  spi_host_device_t spi_host_id_ = SPI2_HOST;
  // 卸载失败或暂不释放时保留状态，避免资源失去归属。
  bool owns_spi_bus_ = false;
};

}  // namespace lilygo_device_driver
