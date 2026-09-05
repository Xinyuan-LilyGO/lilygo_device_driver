/*
 * @Description: T-Display-P4-Air 设备驱动接口
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-09-02 17:16:14
 * @License: GPL 3.0
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "SensorQMC6310.hpp"
#include "bhi2xy_sensorapi_cpp_bus_driver.h"
#include "cpp_bus_driver.h"
#include "device/common/async_init_manager.h"
#include "driver/spi_common.h"
#include "esp32p4_driver.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "esp_spiffs.h"
#include "sdmmc_cmd.h"
#include "stsw_st25rfal002_cpp_bus_driver.h"
#include "t_display_p4_air_config.h"
#include "usp_cpp_bus_driver.h"

namespace lilygo_device_driver {
namespace t_display_p4_air::device {

enum class ScreenType {
  kUnknown,
  kHi8561,
};

// 屏幕型号、分辨率和 MIPI 参数信息
struct ScreenInfo {
  ScreenType type;
  const char* name;
  int width;
  int height;
  int bits_per_pixel;
  const char* pixel_format;
  int mipi_dsi_dpi_clk_mhz;
  int mipi_dsi_hsync;
  int mipi_dsi_hbp;
  int mipi_dsi_hfp;
  int mipi_dsi_vsync;
  int mipi_dsi_vbp;
  int mipi_dsi_vfp;
  int data_lane_num;
  int lane_bit_rate_mbps;
};

// 设备型号名称和版本信息
struct DeviceModelInfo {
  const char* name;
  const char* version;
};

// 摄像头型号、像素格式和缓冲区信息
struct CameraInfo {
  CameraType type;
  const char* name;
  const char* pixel_format;
  int bits_per_pixel;
  int buffer_count;
};

// 充电芯片、电量计芯片和主机内置电池容量信息
struct BatteryInfo {
  const char* charger_chip_name;
  const char* fuel_gauge_chip_name;
  uint16_t capacity_mah;
};

// T-Display-P4-Air 聚合设备信息
struct DeviceInfo {
  DeviceModelInfo model;
  ScreenInfo screen;
  CameraInfo camera;
  BatteryInfo battery;
};

inline constexpr DeviceModelInfo kDeviceModelInfo = {
    .name = "T-Display-P4-Air",
    .version = "v1.0",
};

inline constexpr CameraInfo kCameraInfo = {
    .type = camera::kType,
    .name = camera::kName,
    .pixel_format = camera::kPixelFormat,
    .bits_per_pixel = camera::kBitsPerPixel,
    .buffer_count = camera::kBufferCount,
};

inline constexpr BatteryInfo kBatteryInfo = {
    .charger_chip_name = "axp517",
    .fuel_gauge_chip_name = "axp517",
    .capacity_mah = 1000,
};

}  // namespace t_display_p4_air::device

class TDisplayP4AirDriver {
 public:
  enum class InitMode { kAsync, kSync };
  enum class Lr1121OperatingMode {
    kStandby,
    kSleep,
  };
  enum class Es8389OperatingMode {
    kActive,
    kSleep,
  };
  enum class UartTarget {
    kEsp32p4,
    kEsp32c5,
  };

  struct Bus {
    std::shared_ptr<cpp_bus_driver::HardwareI2c> axp517_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> xl9535_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> sgm38121_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> aw86224_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> st25r3916_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> hi8561_i2c_touch_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> bhi260ap_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> qmc6310n_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareMipi> screen_mipi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2s> es8389_i2s_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> lr1121_spi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareUart> nrf9151_uart_bus;
  };

  struct Chip {
    std::unique_ptr<cpp_bus_driver::Axp517> axp517;
    std::unique_ptr<cpp_bus_driver::Xl95x5> xl9535;
    std::unique_ptr<cpp_bus_driver::Sgm38121> sgm38121;
    std::unique_ptr<cpp_bus_driver::Aw862xx> aw86224;
    std::unique_ptr<stsw_st25rfal002_cpp_bus_driver::St25r3916x> st25r3916;
    std::unique_ptr<cpp_bus_driver::Hi8561> hi8561;
    std::unique_ptr<cpp_bus_driver::Hi8561Touch> hi8561_touch;
    std::unique_ptr<cpp_bus_driver::Pwm> sy7200a;
    std::unique_ptr<cpp_bus_driver::Nrf9151> nrf9151;
    std::unique_ptr<bhi2xy_sensorapi_cpp_bus_driver::Bhi2xy> bhi260ap;
    std::unique_ptr<SensorQMC6310> qmc6310n;
    std::unique_ptr<usp_cpp_bus_driver::Lr11xx> lr1121;
  };

  struct Status {
    struct {
      bool init_flag = false;
    } axp517;

    struct {
      bool init_flag = false;
    } xl9535;

    struct {
      bool init_flag = false;
    } sgm38121;

    struct {
      bool init_flag = false;
    } bhi260ap;

    struct {
      bool init_flag = false;
    } qmc6310n;

    struct {
      bool init_flag = false;
      cpp_bus_driver::Aw862xx::RamWaveformInfo ram_waveform_info;
    } aw86224;

    struct {
      bool init_flag = false;
      ReturnCode result = RFAL_ERR_NONE;
      stsw_st25rfal002_cpp_bus_driver::PlatformError platform_error =
          stsw_st25rfal002_cpp_bus_driver::PlatformError::kNone;
    } st25r3916;

    struct {
      bool init_flag = false;
    } hi8561;

    struct {
      bool init_flag = false;
    } hi8561_touch;

    struct {
      bool init_flag = false;
    } sy7200a;

    struct {
      bool init_flag = false;
    } es8389;

    struct {
      bool init_flag = false;
    } lr1121;

    struct {
      bool init_flag = false;
    } nrf9151;

    struct {
      bool init_flag = false;
    } sd_card;
  };

  static TDisplayP4AirDriver& GetInstance();

  const Bus& bus() const { return bus_; }
  const Chip& chip() const { return chip_; }
  const Status& status() const { return status_; }

  const t_display_p4_air::device::DeviceModelInfo& device_model_info() const {
    return t_display_p4_air::device::kDeviceModelInfo;
  }
  const t_display_p4_air::device::ScreenInfo& screen_info() const;
  t_display_p4_air::device::ScreenType screen_type() const {
    return screen_info().type;
  }
  const t_display_p4_air::device::CameraInfo& camera_info() const {
    return t_display_p4_air::device::kCameraInfo;
  }
  /**
   * @brief 获取主机固定电池硬件信息
   * @return 充电芯片、电量计芯片和主机内置电池额定容量
   */
  const t_display_p4_air::device::BatteryInfo& battery_info() const {
    return t_display_p4_air::device::kBatteryInfo;
  }
  esp_codec_dev_handle_t es8389_input_codec_dev() const {
    return es8389_input_codec_dev_;
  }
  esp_codec_dev_handle_t es8389_output_codec_dev() const {
    return es8389_output_codec_dev_;
  }
  t_display_p4_air::device::DeviceInfo device_info() const {
    return {
        .model = device_model_info(),
        .screen = screen_info(),
        .camera = camera_info(),
        .battery = battery_info(),
    };
  }

  bool Init(InitMode mode = InitMode::kSync);
  bool InitMinimal();
  bool InitAxp517();
  bool InitXl9535();
  bool InitSgm38121();
  bool InitBhi260ap();
  bool InitQmc6310n();
  bool InitHi8561();
  bool InitHi8561Touch();
  bool InitSy7200a();
  bool InitAw86224();
  bool InitSt25r3916();
  bool InitEs8389();
  bool InitLr1121();
  bool InitNrf9151();
  bool InitPower();
  bool InitScreen();
  bool InitTouch();
  bool InitScreenBacklight();
  bool InitSpiffs(const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf);
  bool InitSdmmc(const char* base_path, int max_freq_khz = SDMMC_FREQ_DEFAULT);
  bool InitSdspi(const char* base_path, spi_host_device_t host_id,
      int max_freq_khz = SDMMC_FREQ_DEFAULT);

  bool DeinitEs8389();
  bool DeinitBhi260ap();
  bool DeinitQmc6310n();
  bool DeinitAw86224();
  bool DeinitSt25r3916();
  bool DeinitLr1121();
  bool DeinitNrf9151();
  bool DeinitPower();
  bool DeinitScreen();
  bool DeinitTouch();
  bool DeinitScreenBacklight();
  bool DeinitSdmmc(bool release_bus = true);

  bool IsAxp517Ready() const;
  bool IsXl9535Ready() const;
  bool IsSgm38121Ready() const;
  bool IsBhi260apReady() const;
  bool IsQmc6310nReady() const;
  bool IsHi8561Ready() const;
  bool IsHi8561TouchReady() const;
  bool IsSy7200aReady() const;
  bool IsAw86224Ready() const;
  bool IsSt25r3916Ready() const;
  bool IsEs8389Ready() const;
  bool IsLr1121Ready() const;
  bool IsNrf9151Ready() const;
  bool IsScreenReady() const;
  bool IsSdmmcReady() const;

  bool SetLedEnabled(bool enabled);

  bool SetAw86224Standby();
  bool SetBhi260apSleep(bool sleep);
  bool SetQmc6310nSleep(bool sleep);
  bool SetScreenSleep(bool sleep);
  bool SetEs8389OperatingMode(Es8389OperatingMode mode);
  bool SetLr1121OperatingMode(Lr1121OperatingMode mode);
  bool SetEsp32c5PowerEnabled(bool enabled);
  bool SetCameraPowerEnabled(bool enabled);
  bool SetUsbHostPowerEnabled(bool enabled);
  bool PrepareMinimalDriversForPowerOff();
  bool PrepareDriversForPowerOff();

  /**
   * @brief 使 ESP32-C5 进入下载模式。
   * @return 时序控制成功时返回 true，否则返回 false。
   */
  bool EnterEsp32c5DownloadMode();

  /**
   * @brief 切换外部串口连接目标。
   * @param target 串口连接到 ESP32-P4 或 ESP32-C5。
   * @return 串口切换成功时返回 true，否则返回 false。
   */
  bool SetUartTarget(UartTarget target);

 private:
  void CreateDrivers();
  bool InitDrivers(InitMode mode);
  bool InitMinimalDrivers();
  bool SetNs4150Enabled(bool enabled);
  bool ConfigureEs8389OutputPolarity();

  AsyncInitManager async_init_manager_;
  std::unique_ptr<cpp_bus_driver::PlatformHal> platform_hal_;
  Bus bus_;
  Chip chip_;
  Status status_;
  sdmmc_card_t* sd_card_ = nullptr;
  std::string sd_card_base_path_;
  // 当前挂载是否使用 SDSPI 主机
  bool sd_card_uses_spi_ = false;
  bool minimal_drivers_initialized_ = false;
  bool power_initialized_ = false;
  // 当前 SDSPI 主机编号
  spi_host_device_t sd_card_spi_host_id_ = SPI2_HOST;
  const t_display_p4_air::device::ScreenInfo* screen_info_ = nullptr;

  const audio_codec_ctrl_if_t* es8389_ctrl_if_ = nullptr;
  const audio_codec_data_if_t* es8389_data_if_ = nullptr;
  const audio_codec_gpio_if_t* es8389_gpio_if_ = nullptr;
  const audio_codec_if_t* es8389_codec_if_ = nullptr;
  esp_codec_dev_handle_t es8389_input_codec_dev_ = nullptr;
  esp_codec_dev_handle_t es8389_output_codec_dev_ = nullptr;
  Es8389OperatingMode es8389_operating_mode_ = Es8389OperatingMode::kSleep;

  TDisplayP4AirDriver() = default;
  ~TDisplayP4AirDriver() = default;

  // 禁止拷贝构造和赋值。
  TDisplayP4AirDriver(const TDisplayP4AirDriver&) = delete;
  TDisplayP4AirDriver& operator=(const TDisplayP4AirDriver&) = delete;
};

}  // namespace lilygo_device_driver
