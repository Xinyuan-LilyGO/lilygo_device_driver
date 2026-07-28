/*
 * @Description: T-Display-P4-Air 板级设备驱动接口
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-07-27 14:15:09
 * @License: GPL 3.0
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "cpp_bus_driver_library.h"
#include "esp32p4_driver.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "radiolib_cpp_bus_driver_library.h"
#include "stsw_st25rfal002_cpp_bus_driver_library.h"
#include "t_display_p4_air_config.h"

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

inline constexpr DeviceModelInfo kDeviceModelInfo = {
    .name = "T-Display-P4-Air",
    .version = "v1.0",
};

// 摄像头型号、像素格式和缓冲区信息
struct CameraInfo {
  CameraType type;
  const char* name;
  const char* pixel_format;
  int bits_per_pixel;
  int buffer_count;
};

inline constexpr CameraInfo kCameraInfo = {
    .type = camera::kType,
    .name = camera::kName,
    .pixel_format = camera::kPixelFormat,
    .bits_per_pixel = camera::kBitsPerPixel,
    .buffer_count = camera::kBufferCount,
};

// T-Display-P4-Air 聚合设备信息
struct DeviceInfo {
  DeviceModelInfo model;
  ScreenInfo screen;
  CameraInfo camera;
};

}  // namespace t_display_p4_air::device

class TDisplayP4AirDriver {
 public:
  enum class InitMode { kAsync, kSync };
  enum class PowerState {
    kActive,
    kSleep,
    kOff,
  };
  enum class Lr1121PowerState {
    kStandby,
    kSleep,
  };
  enum class Es8389PowerState {
    kActive,
    kSleep,
  };
  enum class UartTarget {
    kEsp32p4,
    kEsp32c5,
  };

  struct Bus {
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> axp517_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> xl9535_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> sgm38121_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> aw86224_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> st25r3916_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> hi8561_i2c_touch_bus;
    std::shared_ptr<cpp_bus_driver::HardwareMipi> screen_mipi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2s> es8389_i2s_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> lr1121_spi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareUart> nrf9151_uart_bus;

    RadioLibHal* lr1121_radiolib_hal = nullptr;
    Module* lr1121_module = nullptr;
  };

  struct Chip {
    std::unique_ptr<cpp_bus_driver::Axp517> axp517;
    std::unique_ptr<cpp_bus_driver::Xl95x5> xl9535;
    std::unique_ptr<cpp_bus_driver::Sgm38121> sgm38121;
    std::unique_ptr<cpp_bus_driver::Aw862xx> aw86224;
    std::unique_ptr<stsw_st25rfal002_cpp_bus_driver::St25r3916x> st25r3916;
    std::unique_ptr<cpp_bus_driver::Hi8561> hi8561;
    std::unique_ptr<cpp_bus_driver::Hi8561Touch> hi8561_touch;
    std::unique_ptr<cpp_bus_driver::Pwm> hi8561_backlight;
    std::unique_ptr<cpp_bus_driver::Nrf9151> nrf9151;

    LR1121* lr1121 = nullptr;
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
    } hi8561_backlight;

    struct {
      bool init_flag = false;
      // 表示 power_state 是否记录了最后一次成功应用的硬件状态。
      bool power_state_valid = false;
      // 缓存最后一次成功应用的电源状态，避免重复配置硬件。
      Es8389PowerState power_state = Es8389PowerState::kSleep;
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
    };
  }

  bool IsAxp517Ready() const;
  bool IsXl9535Ready() const;
  bool IsSgm38121Ready() const;
  bool IsAw86224Ready() const;
  bool IsSt25r3916Ready() const;
  bool IsHi8561Ready() const;
  bool IsHi8561TouchReady() const;
  bool IsHi8561BacklightReady() const;
  bool IsEs8389Ready() const;
  bool IsLr1121Ready() const;
  bool IsNrf9151Ready() const;
  bool IsScreenReady() const;

  void CreateDrivers();

  /**
   * @brief 初始化已经创建的芯片驱动。
   * @param mode 初始化模式。
   * @return 所有必要驱动初始化成功时返回 true，否则返回 false。
   */
  bool InitDrivers(InitMode mode = InitMode::kSync);

  /**
   * @brief 创建并初始化所有板级驱动。
   * @param mode 初始化模式。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool Init(InitMode mode = InitMode::kSync);

  /**
   * @brief 设置整板运行、休眠或关断状态
   * @param state 目标电源状态
   * @return 状态切换成功返回 true，否则返回 false
   */
  bool SetPowerState(PowerState state);

  bool SetScreenSleep(bool sleep);
  bool SetCameraPowerEnabled(bool enabled);
  bool SetNs4150Enabled(bool enabled);
  bool SetAw86224Standby();
  bool SetEs8389PowerState(Es8389PowerState state);
  bool SetLr1121PowerState(Lr1121PowerState state);
  bool SetNrf9151PowerEnabled(bool enabled);

  bool InitPower();
  bool InitAxp517();
  bool InitXl9535();
  bool ConfigXl9535();

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

  bool InitSgm38121();

  bool InitScreen();
  bool DeinitScreen();
  bool InitTouch();
  bool DeinitTouch();
  bool InitScreenBacklight();
  bool DeinitScreenBacklight();

  bool InitHi8561();
  bool InitHi8561Touch();
  bool InitHi8561Backlight();
  bool InitAw86224();
  bool InitSt25r3916();
  bool InitEs8389();
  bool ConfigEs8389();
  bool InitLr1121();
  bool InitNrf9151();

  /**
   * @brief 挂载 SPIFFS 文件系统。
   * @param base_path 文件系统挂载路径。
   * @param spiffs_conf 返回挂载时使用的 SPIFFS 配置。
   * @return SPIFFS 挂载成功时返回 true，否则返回 false。
   */
  bool InitSpiffs(const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf);

  /**
   * @brief 通过 SDMMC 主机挂载 SD 卡。
   * @param base_path SD 卡挂载路径。
   * @param max_freq_khz SDMMC 总线最大频率，单位为 kHz。
   * @return SD 卡挂载成功时返回 true，否则返回 false。
   */
  bool InitSdmmc(const char* base_path, int max_freq_khz = SDMMC_FREQ_DEFAULT);

  /**
   * @brief 卸载 SD 卡文件系统并释放 SDMMC 主机资源。
   * @return 卸载成功或当前未挂载时返回 true，否则返回 false。
   */
  bool DeinitSdmmc();

  /**
   * @brief 检查已挂载 SD 卡是否仍可响应 SDMMC 命令。
   * @return SD 卡已挂载且可访问时返回 true，否则返回 false。
   */
  bool IsSdmmcReady() const;

  /**
   * @brief 通过 SDSPI 主机挂载 SD 卡。
   * @param base_path SD 卡挂载路径。
   * @param host_id SD 卡使用的 SPI 主机。
   * @param max_freq_khz SDSPI 总线最大频率，单位为 kHz。
   * @return SD 卡挂载成功时返回 true，否则返回 false。
   */
  bool InitSdspi(const char* base_path, spi_host_device_t host_id,
      int max_freq_khz = SDMMC_FREQ_DEFAULT);

 private:
  std::unique_ptr<cpp_bus_driver::Tool> tool_;
  Bus bus_;
  Chip chip_;
  Status status_;
  sdmmc_card_t* sd_card_ = nullptr;
  std::string sd_card_base_path_;
  const t_display_p4_air::device::ScreenInfo* screen_info_ = nullptr;

  const audio_codec_ctrl_if_t* es8389_ctrl_if_ = nullptr;
  const audio_codec_data_if_t* es8389_data_if_ = nullptr;
  const audio_codec_gpio_if_t* es8389_gpio_if_ = nullptr;
  const audio_codec_if_t* es8389_codec_if_ = nullptr;
  esp_codec_dev_handle_t es8389_input_codec_dev_ = nullptr;
  esp_codec_dev_handle_t es8389_output_codec_dev_ = nullptr;

  TDisplayP4AirDriver() = default;
  ~TDisplayP4AirDriver() = default;

  // 禁止拷贝构造和赋值。
  TDisplayP4AirDriver(const TDisplayP4AirDriver&) = delete;
  TDisplayP4AirDriver& operator=(const TDisplayP4AirDriver&) = delete;
};

bool InitSpiffs(const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf);
bool InitSdmmc(const char* base_path, int max_freq_khz = SDMMC_FREQ_DEFAULT);
bool InitSdspi(const char* base_path, spi_host_device_t host_id,
    int max_freq_khz = SDMMC_FREQ_DEFAULT);

}  // namespace lilygo_device_driver
