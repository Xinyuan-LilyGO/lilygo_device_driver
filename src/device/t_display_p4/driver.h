/*
 * @Description: T-Display-P4 设备驱动接口
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-09-23 11:38:11
 * @License: GPL 3.0
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "cpp_bus_driver.h"
#include "chip/esp32p4/sd_card.h"
#include "device/common/async_init_manager.h"
#include "device/common/pixel_format.h"
#include "driver/spi_common.h"
#include "chip/esp32p4/driver.h"
#include "esp_spiffs.h"
#include "sdmmc_cmd.h"
#include "device/t_display_p4/config.h"
#include "usp_cpp_bus_driver.h"
#include "device/t_display_p4/keyboard_expansion_config.h"
#include "stsw_st25rfal002_cpp_bus_driver.h"

#include "SensorQMC6309.hpp"
#if !defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
#include "SensorQMI8658.hpp"
#endif

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
#include "bhi2xy_sensorapi_cpp_bus_driver.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#endif

namespace lilygo_device_driver {
namespace t_display_p4::device {

enum class ScreenType {
  kUnknown,
  kHi8561,
  kRm69a10,
};

enum class ImuType {
  kUnknown,
  kIcm20948,
  kQmi8658Qmc6309,
};

enum class RadioType {
  kUnknown,
  kSx1262,
  kLr2021,
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
    .name = model::kName,
    .version = model::kVersion,
};

// 相机型号、像素格式和缓冲区信息
struct CameraInfo {
  CameraType type;
  const char* name;
  const char* pixel_format;
  int bits_per_pixel;
  int buffer_count;
};

inline constexpr CameraInfo kCameraInfo = {
    .type = camera::kType,
    .name = GetCameraTypeName(camera::kType),
    .pixel_format = GetRgbPixelFormatName(camera::kBitsPerPixel),
    .bits_per_pixel = camera::kBitsPerPixel,
    .buffer_count = camera::kBufferCount,
};

// 充电芯片、电量计芯片、电池容量、默认 PD 和 NTC 配置信息
struct BatteryInfo {
  const char* charger_chip_name;
  const char* fuel_gauge_chip_name;
  uint16_t capacity_mah;
  // 引用静态板级默认策略，不代表当前 PD 合同；无配置时为 nullptr。
  const cpp_bus_driver::Axp517Sink::Config* default_pd_config = nullptr;
  // 引用静态板级 NTC 参数，不代表实时温度；无配置时为 nullptr。
  const cpp_bus_driver::Axp517::NtcConfig* ntc_config = nullptr;
};

inline constexpr BatteryInfo kBatteryInfo = {
    .charger_chip_name = battery::kChargerChipName,
    .fuel_gauge_chip_name = battery::kFuelGaugeChipName,
    .capacity_mah = battery::kCapacityMah,
#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
    .default_pd_config = &battery::kDefaultPdConfig,
    .ntc_config = &battery::kNtcConfig,
#else
    .default_pd_config = nullptr,
    .ntc_config = nullptr,
#endif
};

// T-Display-P4 聚合设备信息
struct DeviceInfo {
  DeviceModelInfo model;
  ScreenInfo screen;
  CameraInfo camera;
  BatteryInfo battery;
};

}  // namespace t_display_p4::device

/**
 * @brief T-Display-P4 板级设备驱动。
 * @note 自动检测型号的功能以通用名称提供公共接口，例如 Screen、
 * Touch、ScreenBacklight，以及 V1 的 Radio、Imu；对应的具体芯片实现
 * 放在私有区域。无需型号选择的芯片接口使用具体芯片名称，例如
 * V2 的 Lr2021。
 */
class TDisplayP4Driver {
 public:
  enum class InitMode { kAsync, kSync };
  enum class Lr2021OperatingMode {
    kStandby,
    kSleep,
  };

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  enum class Es8389OperatingMode {
    kActive,
    kSleep,
  };
#else
  // SX1262 使用暖启动睡眠，唤醒后保留射频配置。
  enum class Sx1262OperatingMode {
    kStandby,  // 可立即收发。
    kSleep,    // 保留配置的低功耗状态。
  };

  enum class RadioOperatingMode {
    kStandby,
    kSleep,
  };
#endif

  enum class Cc1101OperatingMode {
    kStandby,
    kSleep,
  };

  enum class Nrf24l01OperatingMode {
    kStandby,
    kSleep,
  };

  enum class St25r3916OperatingMode {
    kActive,
    kSleep,
  };

  enum class KeyboardExpansionOperatingMode {
    kActive,
    kSleep,
  };

  enum class KeyboardExpansionDeinitMode {
    kNormal,
    kForced,
  };

  enum class KeyboardExpansionLed {
    kLed1,
    kLed2,
    kLed3,
  };

#if !defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  // ES8311 按实际音频路径区分工作模式。
  enum class Es8311OperatingMode {
    kSleep,     // 关闭 ADC、DAC 和模拟偏置。
    kPlayback,  // 仅打开 DAC 和耳机驱动。
    kCapture,   // 仅打开 PGA 和 ADC。
    kDuplex,    // 同时打开采集与播放路径。
  };

  enum class Sky13453RfSwitch {
    kInternalAntenna,
    kExternalAntenna,
  };
#endif

  enum class Cc1101RfSwitch {
    k315Mhz,
    k434Mhz,
    k868_915Mhz,
  };

  static TDisplayP4Driver& GetInstance();

  const auto& bus() const { return bus_; }
  const auto& chip() const { return chip_; }
  const auto& chip_status() const { return chip_status_; }

  // 自动检测的硬件类型查询。
  t_display_p4::device::ScreenType screen_type() const {
    return screen_info().type;
  }
#if !defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  t_display_p4::device::RadioType radio_type() const { return radio_type_; }
  t_display_p4::device::ImuType imu_type() const {
    return imu_type_;
  }
#endif

  // 设备及外设信息查询。
  const t_display_p4::device::DeviceModelInfo& device_model_info() const {
    return t_display_p4::device::kDeviceModelInfo;
  }
  const t_display_p4::device::ScreenInfo& screen_info() const;
  const t_display_p4::device::CameraInfo& camera_info() const {
    return t_display_p4::device::kCameraInfo;
  }
  const t_display_p4::device::BatteryInfo& battery_info() const {
    return t_display_p4::device::kBatteryInfo;
  }
  t_display_p4::device::DeviceInfo device_info() const {
    return {
        .model = device_model_info(),
        .screen = screen_info(),
        .camera = camera_info(),
        .battery = battery_info(),
    };
  }

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  esp_codec_dev_handle_t es8389_input_codec_dev() const {
    return es8389_input_codec_dev_;
  }
  esp_codec_dev_handle_t es8389_output_codec_dev() const {
    return es8389_output_codec_dev_;
  }
#endif

  bool Init(InitMode mode = InitMode::kSync);
  bool InitMinimal();
  bool InitXl9535();
  bool InitSgm38121();
  bool InitAw86224();
  bool InitL76k();
  bool InitPower();
  bool InitScreen();
  bool InitTouch();
  bool InitScreenBacklight();
  bool InitSpiffs(const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf);
  bool InitSdmmc(const char* base_path, int max_freq_khz = SDMMC_FREQ_DEFAULT);
  bool InitSdspi(const char* base_path, spi_host_device_t host_id,
      int max_freq_khz = SDMMC_FREQ_DEFAULT);

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  bool InitAxp517();
  bool InitBhi260ap();
  bool InitQmc6309();
  bool InitEs8389();
  bool InitLr2021();
  bool InitUsbHostPower();
#else
  bool InitImu();
  bool InitBq27220();
  bool InitPcf8563();
  bool InitEs8311();
  bool InitRadio();
#endif
  bool InitXl9555();
  bool InitTca8418();
  bool InitCc1101();
  bool InitNrf24l01();
  bool InitSt25r3916();
  bool InitKeyboardExpansion();
  bool InitKeyboardBacklight();

  bool DeinitScreen();
  bool DeinitTouch();
  bool DeinitScreenBacklight();
  bool DeinitAw86224();
  bool DeinitL76k();
  bool DeinitSdmmc(bool release_bus = true);

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  bool DeinitEs8389();
  bool DeinitBhi260ap();
  bool DeinitQmc6309();
  bool DeinitLr2021();
  bool DeinitPower();
#else
  bool DeinitImu();
  bool DeinitEs8311();
  bool DeinitRadio();
#endif
  bool DeinitSt25r3916();
  bool DeinitKeyboardExpansion(
      KeyboardExpansionDeinitMode mode = KeyboardExpansionDeinitMode::kNormal);

  bool IsXl9535Ready() const;
  bool IsSgm38121Ready() const;
  bool IsAw86224Ready() const;
  bool IsL76kReady() const;
  bool IsScreenBacklightReady() const;
  bool IsScreenReady() const;
  bool IsTouchReady() const;
  bool IsSdmmcReady() const;

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  bool IsAxp517Ready() const;
  bool IsBhi260apReady() const;
  bool IsQmc6309Ready() const;
  bool IsEs8389Ready() const;
  bool IsLr2021Ready() const;
#else
  bool IsImuReady() const;
  bool IsBq27220Ready() const;
  bool IsPcf8563Ready() const;
  bool IsEs8311Ready() const;
  bool IsRadioReady() const;
#endif
  bool IsXl9555Ready() const;
  bool IsTca8418Ready() const;
  bool IsCc1101Ready() const;
  bool IsNrf24l01Ready() const;
  bool IsSt25r3916Ready() const;
  bool IsKeyboardBacklightReady() const;

  bool SetAw86224Standby();
  bool SetL76kSleep(bool sleep);
  bool SetScreenSleep(bool sleep);
  bool SetCameraPowerEnabled(bool enabled);
#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  bool SetUsbHostPowerEnabled(bool enabled);
#endif
  bool PrepareDriversForPowerOff();

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  bool SetEs8389OperatingMode(Es8389OperatingMode mode);
  bool SetLr2021OperatingMode(Lr2021OperatingMode mode);
  bool SetBhi260apSleep(bool sleep);
  bool SetQmc6309Sleep(bool sleep);
  bool SetEsp32c5PowerEnabled(bool enabled);
  bool PrepareMinimalDriversForPowerOff();
#else
  bool SetImuSleep(bool sleep);
  bool SetEs8311OperatingMode(Es8311OperatingMode mode);
  bool SetRadioOperatingMode(RadioOperatingMode mode);
  bool SetEsp32c6PowerEnabled(bool enabled);
  bool SetEthernetPowerEnabled(bool enabled);
#endif
  bool SetCc1101OperatingMode(Cc1101OperatingMode mode);
  bool SetNrf24l01OperatingMode(Nrf24l01OperatingMode mode);
  bool SetSt25r3916OperatingMode(St25r3916OperatingMode mode);
  bool SetKeyboardExpansionOperatingMode(KeyboardExpansionOperatingMode mode);

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  /**
   * @brief 使 ESP32-C5 进入下载模式。
   * @return 时序控制成功时返回 true，否则返回 false。
   */
  bool EnterEsp32c5DownloadMode();

  bool SetLedEnabled(bool enabled);
#endif
  /**
   * @brief 选择 CC1101 RF 开关通路。
   * @param rf_switch RF 频段开关位置。
   * @return RF 开关引脚配置成功时返回 true，否则返回 false。
   */
  bool SetCc1101RfSwitch(Cc1101RfSwitch rf_switch);

  /**
   * @brief 设置键盘扩展指示灯状态
   * @param led 键盘扩展指示灯
   * @param enabled true 点亮，false 熄灭
   * @return 指示灯状态设置成功返回 true，否则返回 false
   */
  bool SetKeyboardExpansionLed(KeyboardExpansionLed led, bool enabled);

#if !defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  /**
   * @brief 选择 SKY13453 RF 开关连接的天线。
   * @param rf_switch 内置或外置天线开关位置。
   * @return RF 开关引脚配置成功时返回 true，否则返回 false。
   */
  bool SetSky13453RfSwitch(Sky13453RfSwitch rf_switch);
#endif

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  /**
   * @brief 检查电池选择开关是否位于外部电池位置
   * @return 选择 EX_VBAT 返回true，选择 P4_VBAT 或芯片未初始化返回false
   */
  bool IsExternalBatterySelected() const;
#endif

 private:
  struct Bus {
    std::shared_ptr<cpp_bus_driver::HardwareI2c> xl9535_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> sgm38121_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> aw86224_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareMipi> screen_mipi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareUart> l76k_uart_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> touch_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> radio_spi_bus;

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
    std::shared_ptr<cpp_bus_driver::HardwareI2c> axp517_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> bhi260ap_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> qmc6309_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2s> es8389_i2s_bus;
#else
    std::shared_ptr<cpp_bus_driver::HardwareI2c> imu_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> bq27220_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> pcf8563_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> es8311_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2s> es8311_i2s_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> sx1262_spi_bus;
#endif
    std::shared_ptr<cpp_bus_driver::HardwareI2c> xl9555_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> tca8418_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> cc1101_spi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> nrf24l01_spi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> st25r3916_spi_bus;
  };

  struct Chip {
    std::unique_ptr<cpp_bus_driver::Xl95x5> xl9535;
    std::unique_ptr<cpp_bus_driver::Sgm38121> sgm38121;
    std::unique_ptr<cpp_bus_driver::Aw862xx> aw86224;
    std::unique_ptr<cpp_bus_driver::L76k> l76k;
    std::unique_ptr<usp_cpp_bus_driver::Lr20xx> lr2021;
    std::unique_ptr<cpp_bus_driver::Hi8561> hi8561;
    std::unique_ptr<cpp_bus_driver::Hi8561Touch> hi8561_touch;
    std::unique_ptr<cpp_bus_driver::Rm69a10> rm69a10;
    std::unique_ptr<cpp_bus_driver::Gt9895> gt9895;
    std::unique_ptr<cpp_bus_driver::Pwm> keyboard_sy7200a;

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
    std::unique_ptr<cpp_bus_driver::Pwm> sy7200a;
    std::unique_ptr<cpp_bus_driver::Axp517> axp517;
    std::unique_ptr<bhi2xy_sensorapi_cpp_bus_driver::Bhi2xy> bhi260ap;
    std::unique_ptr<SensorQMC6309> qmc6309;
#else
    std::unique_ptr<cpp_bus_driver::Icm20948> icm20948;
    std::unique_ptr<SensorQMI8658> qmi8658;
    std::unique_ptr<SensorQMC6309> qmc6309;
    std::unique_ptr<cpp_bus_driver::Bq27220> bq27220;
    std::unique_ptr<cpp_bus_driver::Pcf8563x> pcf8563;
    std::unique_ptr<cpp_bus_driver::Es8311> es8311;
    std::unique_ptr<usp_cpp_bus_driver::Sx126x> sx1262;
    std::unique_ptr<cpp_bus_driver::Pwm> pt4103;
#endif
    std::unique_ptr<cpp_bus_driver::Xl95x5> xl9555;
    std::unique_ptr<cpp_bus_driver::Tca8418> tca8418;
    std::unique_ptr<cpp_bus_driver::Cc1101> cc1101;
    std::unique_ptr<cpp_bus_driver::Nrf24l01x> nrf24l01;
    std::unique_ptr<stsw_st25rfal002_cpp_bus_driver::St25r3916x> st25r3916;
  };

  struct ChipStatus {
    struct {
      bool init_flag = false;
    } xl9535;

    struct {
      bool init_flag = false;
    } sgm38121;

    struct {
      bool init_flag = false;
    } hi8561;

    struct {
      bool init_flag = false;
    } hi8561_touch;

    struct {
      bool init_flag = false;
    } rm69a10;

    struct {
      bool init_flag = false;
    } gt9895;

    struct {
      bool init_flag = false;
      cpp_bus_driver::Aw862xx::RamWaveformInfo ram_waveform_info;
    } aw86224;

    struct {
      bool init_flag = false;
    } l76k;

    struct {
      bool init_flag = false;
    } lr2021;

    struct {
      bool init_flag = false;
    } keyboard_sy7200a;

    struct {
      bool init_flag = false;
    } sd_card;

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
    struct {
      bool init_flag = false;
    } sy7200a;

    struct {
      bool init_flag = false;
    } axp517;

    struct {
      bool init_flag = false;
    } bhi260ap;

    struct {
      bool init_flag = false;
    } qmc6309;

    struct {
      bool init_flag = false;
    } es8389;

#else
    struct {
      bool init_flag = false;
    } pt4103;

    struct {
      bool init_flag = false;
    } bq27220;

    struct {
      bool init_flag = false;
    } pcf8563;

    struct {
      bool init_flag = false;
    } es8311;

    struct {
      bool init_flag = false;
    } icm20948;

    struct {
      bool init_flag = false;
    } qmi8658;

    struct {
      bool init_flag = false;
    } qmc6309;

    struct {
      bool init_flag = false;
    } sx1262;
#endif

    struct {
      bool init_flag = false;
    } xl9555;

    struct {
      bool init_flag = false;
    } tca8418;

    struct {
      bool init_flag = false;
    } cc1101;

    struct {
      bool init_flag = false;
    } nrf24l01;

    struct {
      bool init_flag = false;
    } st25r3916;
  };

  TDisplayP4Driver() = default;
  ~TDisplayP4Driver() = default;

  // 禁止拷贝构造和赋值。
  TDisplayP4Driver(const TDisplayP4Driver&) = delete;
  TDisplayP4Driver& operator=(const TDisplayP4Driver&) = delete;

  void CreateDrivers();

  bool InitDrivers(InitMode mode);
  bool InitMinimalDrivers();
  bool InitHi8561();
  bool InitHi8561Touch();
  bool InitRm69a10();
  bool InitGt9895();
  bool InitSy7200a();

#if !defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  bool InitIcm20948();
  bool InitQmi8658();
  bool InitQmc6309();
  bool InitPt4103();
  bool InitSx1262();
  bool InitLr2021();

  bool DeinitSx1262();
  bool DeinitLr2021();

  bool IsPt4103Ready() const;
  bool IsSx1262Ready() const;
  bool IsLr2021Ready() const;

  bool SetQmi8658Sleep(bool sleep);
  bool SetQmc6309Sleep(bool sleep);
  bool SetSx1262OperatingMode(Sx1262OperatingMode mode);
  bool SetLr2021OperatingMode(Lr2021OperatingMode mode);
#endif

  bool IsHi8561Ready() const;
  bool IsHi8561TouchReady() const;
  bool IsRm69a10Ready() const;
  bool IsGt9895Ready() const;

  /**
   * @brief 通过 GT9895 触摸 ID 检测屏幕类型。
   * @return 检测流程完成时返回 true，否则返回 false。
   */
  bool DetectScreenType();

  void ResetScreenBacklightStatus();

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  bool SetNs4150Enabled(bool enabled);
#endif

  void CreateKeyboardExpansionDrivers();
  void DestroyKeyboardExpansionDrivers();

  AsyncInitManager async_init_manager_;
  std::unique_ptr<cpp_bus_driver::PlatformHal> platform_hal_;
  Bus bus_;
  Chip chip_;
  ChipStatus chip_status_;
  SdCard sd_card_;
  const t_display_p4::device::ScreenInfo* screen_info_ = nullptr;
  bool minimal_drivers_initialized_ = false;

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
  bool power_initialized_ = false;
  const audio_codec_ctrl_if_t* es8389_ctrl_if_ = nullptr;
  const audio_codec_data_if_t* es8389_data_if_ = nullptr;
  const audio_codec_gpio_if_t* es8389_gpio_if_ = nullptr;
  const audio_codec_if_t* es8389_codec_if_ = nullptr;
  esp_codec_dev_handle_t es8389_input_codec_dev_ = nullptr;
  esp_codec_dev_handle_t es8389_output_codec_dev_ = nullptr;
  Es8389OperatingMode es8389_operating_mode_ = Es8389OperatingMode::kSleep;
#else
  t_display_p4::device::ImuType imu_type_ =
      t_display_p4::device::ImuType::kUnknown;
  t_display_p4::device::RadioType radio_type_ =
      t_display_p4::device::RadioType::kUnknown;
#endif
};

}  // namespace lilygo_device_driver
