/*
 * @Description: T-Display-P4 设备驱动接口
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-09-02 17:16:01
 * @License: GPL 3.0
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "cpp_bus_driver_library.h"
#include "device/common/async_init_manager.h"
#include "esp32p4_driver.h"
#include "stsw_st25rfal002_cpp_bus_driver_library.h"
#include "t_display_p4_keyboard_expansion_config.h"
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

// 充电芯片、电量计芯片和电池容量信息
struct BatteryInfo {
  const char* charger_chip_name;
  const char* fuel_gauge_chip_name;
  uint16_t capacity_mah;
};

inline constexpr BatteryInfo kBatteryInfo = {
    .charger_chip_name = "lgs4056hda",
    .fuel_gauge_chip_name = "bq27220",
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

  // SX1262 使用暖启动睡眠，唤醒后保留射频配置。
  enum class Sx1262OperatingMode {
    kStandby,  // 可立即收发。
    kSleep,    // 保留配置的低功耗状态。
  };

  enum class Lr2021OperatingMode {
    kStandby,
    kSleep,
  };

  enum class RadioOperatingMode {
    kStandby,
    kSleep,
  };

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
    std::shared_ptr<cpp_bus_driver::HardwareSpi> st25r3916_spi_bus;
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
    std::unique_ptr<cpp_bus_driver::Pwm> pt4103;
    std::unique_ptr<cpp_bus_driver::Rm69a10> rm69a10;
    std::unique_ptr<cpp_bus_driver::Gt9895> gt9895;

    std::unique_ptr<cpp_bus_driver::Xl95x5> xl9555;
    std::unique_ptr<cpp_bus_driver::Tca8418> tca8418;
    std::unique_ptr<cpp_bus_driver::Pwm> sy7200a;

    std::unique_ptr<cpp_bus_driver::Cc1101> cc1101;
    std::unique_ptr<cpp_bus_driver::Nrf24l01x> nrf24l01;
    std::unique_ptr<stsw_st25rfal002_cpp_bus_driver::St25r3916x> st25r3916;
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
    } pt4103;

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
    } sy7200a;

    struct {
      bool init_flag = false;
    } cc1101;

    struct {
      bool init_flag = false;
    } nrf24l01;

    struct {
      bool init_flag = false;
    } st25r3916;

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

  bool Init(InitMode mode = InitMode::kSync);
  bool InitMinimal();
  bool InitBq27220();
  bool InitXl9535();
  bool InitSgm38121();
  bool InitHi8561();
  bool InitHi8561Touch();
  bool InitPt4103();
  bool InitRm69a10();
  bool InitGt9895();
  bool InitPcf8563();
  bool InitAw86224();
  bool InitEs8311();
  bool InitL76k();
  bool InitIcm20948();
  bool InitSx1262();
  bool InitLr2021();
  bool InitXl9555();
  bool InitTca8418();
  bool InitSy7200a();
  bool InitCc1101();
  bool InitNrf24l01();
  bool InitSt25r3916();
  bool InitPower();
  bool InitScreen();
  bool InitTouch();
  bool InitScreenBacklight();
  bool InitRadio();
  bool InitKeyboardExpansion();
  bool InitSpiffs(const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf);
  bool InitSdmmc(const char* base_path, int max_freq_khz = SDMMC_FREQ_DEFAULT);
  bool InitSdspi(const char* base_path, spi_host_device_t host_id,
      int max_freq_khz = SDMMC_FREQ_DEFAULT);

  bool DeinitScreen();
  bool DeinitTouch();
  bool DeinitScreenBacklight();
  bool DeinitAw86224();
  bool DeinitEs8311();
  bool DeinitL76k();
  bool DeinitIcm20948();
  bool DeinitSx1262();
  bool DeinitLr2021();
  bool DeinitRadio();
  bool DeinitSt25r3916();
  bool DeinitKeyboardExpansion(
      KeyboardExpansionDeinitMode mode = KeyboardExpansionDeinitMode::kNormal);
  bool DeinitSdmmc();

  bool IsBq27220Ready() const;
  bool IsXl9535Ready() const;
  bool IsSgm38121Ready() const;
  bool IsHi8561Ready() const;
  bool IsHi8561TouchReady() const;
  bool IsPt4103Ready() const;
  bool IsRm69a10Ready() const;
  bool IsGt9895Ready() const;
  bool IsPcf8563Ready() const;
  bool IsAw86224Ready() const;
  bool IsEs8311Ready() const;
  bool IsL76kReady() const;
  bool IsIcm20948Ready() const;
  bool IsSx1262Ready() const;
  bool IsLr2021Ready() const;
  bool IsXl9555Ready() const;
  bool IsTca8418Ready() const;
  bool IsSy7200aReady() const;
  bool IsCc1101Ready() const;
  bool IsNrf24l01Ready() const;
  bool IsSt25r3916Ready() const;
  bool IsScreenReady() const;
  bool IsTouchReady() const;
  bool IsRadioReady() const;
  bool IsSdmmcReady() const;

  bool SetAw86224Standby();
  bool SetL76kSleep(bool sleep);
  bool SetIcm20948Sleep(bool sleep);
  bool SetScreenSleep(bool sleep);
  bool SetEs8311OperatingMode(Es8311OperatingMode mode);
  bool SetSx1262OperatingMode(Sx1262OperatingMode mode);
  bool SetLr2021OperatingMode(Lr2021OperatingMode mode);
  bool SetCc1101OperatingMode(Cc1101OperatingMode mode);
  bool SetNrf24l01OperatingMode(Nrf24l01OperatingMode mode);
  bool SetSt25r3916OperatingMode(St25r3916OperatingMode mode);
  bool SetKeyboardExpansionOperatingMode(KeyboardExpansionOperatingMode mode);
  bool SetRadioOperatingMode(RadioOperatingMode mode);
  bool SetEsp32c6PowerEnabled(bool enabled);
  bool SetCameraPowerEnabled(bool enabled);
  bool SetEthernetPowerEnabled(bool enabled);
  bool SetUsbHostPowerEnabled(bool enabled);
  bool PrepareDriversForPowerOff();

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

  /**
   * @brief 选择 SKY13453 RF 开关连接的天线。
   * @param rf_switch 内置或外置天线开关位置。
   * @return RF 开关引脚配置成功时返回 true，否则返回 false。
   */
  bool SetSky13453RfSwitch(Sky13453RfSwitch rf_switch);

 private:
  void CreateDrivers();
  void CreateKeyboardExpansionDrivers();
  void DestroyKeyboardExpansionDrivers();
  bool InitDrivers(InitMode mode);
  bool InitMinimalDrivers();

  AsyncInitManager async_init_manager_;
  std::unique_ptr<cpp_bus_driver::Tool> tool_;
  Bus bus_;
  Chip chip_;
  Status status_;
  sdmmc_card_t* sd_card_ = nullptr;
  std::string sd_card_base_path_;
  const t_display_p4::device::ScreenInfo* screen_info_ = nullptr;
  t_display_p4::device::RadioType radio_type_ =
      t_display_p4::device::RadioType::kUnknown;
  bool minimal_drivers_initialized_ = false;
  /**
   * @brief 通过 GT9895 触摸 ID 检测屏幕类型。
   * @return 检测流程完成时返回 true，否则返回 false。
   */
  bool DetectScreenType();

  TDisplayP4Driver() = default;
  ~TDisplayP4Driver() = default;

  // 禁止拷贝构造和赋值。
  TDisplayP4Driver(const TDisplayP4Driver&) = delete;
  TDisplayP4Driver& operator=(const TDisplayP4Driver&) = delete;
};

}  // namespace lilygo_device_driver
