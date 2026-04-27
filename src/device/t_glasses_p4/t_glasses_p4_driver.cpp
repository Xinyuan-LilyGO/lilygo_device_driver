/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:58:49
 * @LastEditTime: 2026-04-16 15:02:48
 * @License: GPL 3.0
 */
#include "t_glasses_p4_driver.h"

namespace lilygo_device_driver {
bool InitSdmmc(const char* base_path, int max_freq_khz) {
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024,
      .disk_status_check_enable = false,
      .use_one_fat = false,
  };

  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.slot = SDMMC_HOST_SLOT_0;

  host.max_freq_khz = max_freq_khz;

  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.width = 4;
  slot_config.clk = static_cast<gpio_num_t>(SD_SDIO_CLK);
  slot_config.cmd = static_cast<gpio_num_t>(SD_SDIO_CMD);
  slot_config.d0 = static_cast<gpio_num_t>(SD_SDIO_D0);
  slot_config.d1 = static_cast<gpio_num_t>(SD_SDIO_D1);
  slot_config.d2 = static_cast<gpio_num_t>(SD_SDIO_D2);
  slot_config.d3 = static_cast<gpio_num_t>(SD_SDIO_D3);

  slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

  sdmmc_card_t* card;

  esp_err_t ret = esp_vfs_fat_sdmmc_mount(
      base_path, &host, &slot_config, &mount_config, &card);
  if (ret != ESP_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "esp_vfs_fat_sdmmc_mount failed (error code: %#X)\n", ret);
    return false;
  }

  sdmmc_card_print_info(stdout, card);

  return true;
}

bool InitSdspi(
    const char* base_path, spi_host_device_t host_id, int max_freq_khz) {
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024,
      .disk_status_check_enable = false,
      .use_one_fat = false,
  };

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  host.slot = host_id;

  host.max_freq_khz = max_freq_khz;

  spi_bus_config_t bus_config = {
      .mosi_io_num = SD_MOSI,
      .miso_io_num = SD_MISO,
      .sclk_io_num = SD_SCLK,
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

  esp_err_t ret = spi_bus_initialize(host_id, &bus_config, SDSPI_DEFAULT_DMA);
  if (ret != ESP_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "spi_bus_initialize failed (error code: %#X)\n", ret);
    return false;
  }

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.host_id = host_id;
  slot_config.gpio_cs = static_cast<gpio_num_t>(SD_CS);

  sdmmc_card_t* card;

  ret = esp_vfs_fat_sdspi_mount(
      base_path, &host, &slot_config, &mount_config, &card);
  if (ret != ESP_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "esp_vfs_fat_sdspi_mount failed (error code: %#X)\n", ret);
    return false;
  }

  sdmmc_card_print_info(stdout, card);

  return true;
}
}  // namespace lilygo_device_driver
