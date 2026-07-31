/*
 * @Description: T-Display-P4 板级设备驱动接口
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-07-31 13:41:31
 * @License: GPL 3.0
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "cpp_bus_driver_library.h"
#include "esp32p4_driver.h"
#include "t_display_p4_keyboard_config.h"
#include "usp_cpp_bus_driver_library.h"

namespace lilygo_device_driver {
namespace t_display_p4::device {

enum class ScreenType {
  kUnknown,
  kHi8561,
  kRm69a10,
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
    .name = "T-Display-P4",
    .version = "v1.0",
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
    .name = camera::kName,
    .pixel_format = camera::kPixelFormat,
    .bits_per_pixel = camera::kBitsPerPixel,
    .buffer_count = camera::kBufferCount,
};

// 电量计型号和电池容量信息
struct BatteryInfo {
  const char* fuel_gauge_name;
  uint16_t capacity_mah;
};

inline constexpr BatteryInfo kBatteryInfo = {
    .fuel_gauge_name = "bq27220",
    .capacity_mah = 1000,
};

// T-Display-P4 聚合设备信息
struct DeviceInfo {
  DeviceModelInfo model;
  ScreenInfo screen;
  CameraInfo camera;
  BatteryInfo battery;
};

}  // namespace t_display_p4::device

class TDisplayP4Driver {
 public:
  enum class InitMode { kAsync, kSync };

  enum class PowerState {
    kActive,
    kSleep,
    kOff,
  };

  // SX1262 使用暖启动睡眠，唤醒后保留射频配置。
  enum class Sx1262PowerState {
    kStandby,  // 可立即收发。
    kSleep,    // 保留配置的低功耗状态。
  };

  enum class Lr2021PowerState {
    kStandby,
    kSleep,
  };

  enum class RadioPowerState {
    kStandby,
    kSleep,
  };

  enum class Cc1101PowerState {
    kStandby,
    kSleep,
  };

  enum class Nrf24l01PowerState {
    kStandby,
    kSleep,
  };

  // ES8311 按实际音频路径分别供电，避免仅用启用/禁用表达不清。
  enum class Es8311PowerState {
    kSleep,     // 关闭 ADC、DAC 和模拟偏置。
    kPlayback,  // 仅打开 DAC 和耳机驱动。
    kCapture,   // 仅打开 PGA 和 ADC。
    kDuplex,    // 同时打开采集与播放路径。
  };

  enum class Cc1101RfSwitch {
    k315Mhz,
    k434Mhz,
    k868_915Mhz,
  };

  struct Bus {
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> bq27220_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> xl9535_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> sgm38121_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> pcf8563_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> aw86224_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> es8311_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> icm20948_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareMipi> screen_mipi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2s> es8311_i2s_bus;
    std::shared_ptr<cpp_bus_driver::HardwareUart> l76k_uart_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> radio_spi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> sx1262_spi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> hi8561_i2c_touch_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> gt9895_i2c_touch_bus;

    std::shared_ptr<cpp_bus_driver::SoftwareI2c> xl9555_i2c_bus;
    std::shared_ptr<cpp_bus_driver::SoftwareI2c> tca8418_i2c_bus;

    std::shared_ptr<cpp_bus_driver::HardwareSpi> cc1101_spi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> nrf24l01_spi_bus;
  };

  struct Chip {
    std::unique_ptr<cpp_bus_driver::Xl95x5> xl9535;
    std::unique_ptr<cpp_bus_driver::Bq27220> bq27220;
    std::unique_ptr<cpp_bus_driver::Sgm38121> sgm38121;
    std::unique_ptr<cpp_bus_driver::Pcf8563x> pcf8563;
    std::unique_ptr<cpp_bus_driver::Aw862xx> aw86224;
    std::unique_ptr<cpp_bus_driver::Es8311> es8311;
    std::unique_ptr<cpp_bus_driver::L76k> l76k;
    std::unique_ptr<cpp_bus_driver::Icm20948> icm20948;
    std::unique_ptr<usp_cpp_bus_driver::Sx126x> sx1262;
    std::unique_ptr<usp_cpp_bus_driver::Lr20xx> lr2021;
    std::unique_ptr<cpp_bus_driver::Hi8561> hi8561;
    std::unique_ptr<cpp_bus_driver::Hi8561Touch> hi8561_touch;
    std::unique_ptr<cpp_bus_driver::Pwm> hi8561_backlight;
    std::unique_ptr<cpp_bus_driver::Rm69a10> rm69a10;
    std::unique_ptr<cpp_bus_driver::Gt9895> gt9895;

    std::unique_ptr<cpp_bus_driver::Xl95x5> xl9555;
    std::unique_ptr<cpp_bus_driver::Tca8418> tca8418;
    std::unique_ptr<cpp_bus_driver::Pwm> tca8418_backlight;

    std::unique_ptr<cpp_bus_driver::Cc1101> cc1101;
    std::unique_ptr<cpp_bus_driver::Nrf24l01x> nrf24l01;
  };

  struct Status {
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
    } hi8561_backlight;

    struct {
      bool init_flag = false;
    } rm69a10;

    struct {
      bool init_flag = false;
    } gt9895;

    struct {
      bool init_flag = false;
    } bq27220;

    struct {
      bool init_flag = false;
    } pcf8563;

    struct {
      bool init_flag = false;
      cpp_bus_driver::Aw862xx::RamWaveformInfo ram_waveform_info;
    } aw86224;

    struct {
      bool init_flag = false;
      // 表示 power_state 是否记录了最后一次成功应用的硬件状态。
      bool power_state_valid = false;
      // 缓存最后一次成功应用的电源状态，避免重复配置硬件。
      Es8311PowerState power_state = Es8311PowerState::kSleep;
    } es8311;

    struct {
      bool init_flag = false;
    } l76k;

    struct {
      bool init_flag = false;
    } icm20948;

    struct {
      bool init_flag = false;
    } sx1262;

    struct {
      bool init_flag = false;
    } lr2021;

    struct {
      bool init_flag = false;
    } xl9555;

    struct {
      bool init_flag = false;
    } tca8418;

    struct {
      bool init_flag = false;
    } tca8418_backlight;

    struct {
      bool init_flag = false;
    } cc1101;

    struct {
      bool init_flag = false;
    } nrf24l01;

    struct {
      bool init_flag = false;
    } sd_card;
  };

  static TDisplayP4Driver& GetInstance();

  const Bus& bus() const { return bus_; }
  const Chip& chip() const { return chip_; }
  const Status& status() const { return status_; }

  const t_display_p4::device::DeviceModelInfo& device_model_info() const {
    return t_display_p4::device::kDeviceModelInfo;
  }
  t_display_p4::device::ScreenType screen_type() const;
  const t_display_p4::device::ScreenInfo& screen_info() const;
  t_display_p4::device::RadioType radio_type() const { return radio_type_; }
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

  bool IsXl9535Ready() const;
  bool IsSgm38121Ready() const;
  bool IsHi8561Ready() const;
  bool IsHi8561TouchReady() const;
  bool IsHi8561BacklightReady() const;
  bool IsRm69a10Ready() const;
  bool IsGt9895Ready() const;
  bool IsBq27220Ready() const;
  bool IsPcf8563Ready() const;
  bool IsAw86224Ready() const;
  bool IsEs8311Ready() const;
  bool IsL76kReady() const;
  bool IsIcm20948Ready() const;
  bool IsRadioReady() const;
  bool IsSx1262Ready() const;
  bool IsLr2021Ready() const;
  bool IsXl9555Ready() const;
  bool IsTca8418Ready() const;
  bool IsTca8418BacklightReady() const;
  bool IsCc1101Ready() const;
  bool IsNrf24l01Ready() const;
  bool IsScreenReady() const;
  bool IsTouchReady() const;
  bool keyboard_connected() const { return keyboard_connected_; }

  void CreateDrivers();

  /**
   * @brief 初始化已创建的芯片驱动。
   * @param mode 初始化模式。
   * @return 所有必需驱动初始化成功时返回 true，否则返回 false。
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
  bool SetEsp32c6PowerEnabled(bool enabled);
  bool SetEthernetPowerEnabled(bool enabled);
  bool SetAudioPowerEnabled(bool enabled);
  bool SetUsbHostPowerEnabled(bool enabled);
  bool SetAw86224Standby();
  bool SetEs8311PowerState(Es8311PowerState state);
  bool SetL76kSleep(bool sleep);
  bool SetIcm20948Sleep(bool sleep);
  bool SetRadioPowerState(RadioPowerState state);
  bool SetSx1262PowerState(Sx1262PowerState state);
  bool SetLr2021PowerState(Lr2021PowerState state);
  bool SetCc1101PowerState(Cc1101PowerState state);
  bool SetNrf24l01PowerState(Nrf24l01PowerState state);

  bool InitPower();

  bool InitBq27220();
  bool InitXl9535();
  bool ConfigXl9535();
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
  bool InitRm69a10();
  bool InitGt9895();

  bool InitPcf8563();
  bool InitAw86224();
  bool InitEs8311();
  bool ConfigEs8311();
  bool InitL76k();
  bool InitIcm20948();
  bool InitRadio();
  bool InitSx1262();
  bool InitLr2021();

  bool InitXl9555();
  bool ConfigXl9555();
  bool InitTca8418();
  bool InitTca8418Backlight();
  bool InitCc1101();
  bool InitNrf24l01();

  bool InitKeyboard();
  bool DeinitKeyboard();

  /**
   * @brief 选择 CC1101 RF 开关通路。
   * @param rf_switch RF 频段开关位置。
   * @return RF 开关引脚配置成功时返回 true，否则返回 false。
   */
  bool SetCc1101RfSwitch(Cc1101RfSwitch rf_switch);

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
   * @brief 检查已挂载 SD 卡是否仍可响应 SDMMC 命令。
   * @return SD 卡已挂载且可访问时返回 true，否则返回 false。
   */
  bool IsSdmmcReady() const;

  /**
   * @brief 卸载 SD 卡文件系统并释放 SDMMC 主机资源。
   * @return 卸载成功或当前未挂载时返回 true，否则返回 false。
   */
  bool DeinitSdmmc();

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
  const t_display_p4::device::ScreenInfo* screen_info_ = nullptr;
  t_display_p4::device::RadioType radio_type_ =
      t_display_p4::device::RadioType::kUnknown;
  bool keyboard_connected_ = false;

  /**
   * @brief 通过 GT9895 触摸 ID 检测屏幕类型。
   * @return 检测流程完成时返回 true，否则返回 false。
   */
  bool DetectScreenType();

  bool SetKeyboardPowerState(PowerState state);

  TDisplayP4Driver() = default;
  ~TDisplayP4Driver() = default;

  // 禁止拷贝构造和赋值。
  TDisplayP4Driver(const TDisplayP4Driver&) = delete;
  TDisplayP4Driver& operator=(const TDisplayP4Driver&) = delete;
};

}  // namespace lilygo_device_driver
