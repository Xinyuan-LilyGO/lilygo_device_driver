/*
 * @Description: T-Glasses-P4 设备驱动接口
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:58:49
 * @License: GPL 3.0
 */
#pragma once

// #include <cstdint>
#include <memory>
#include <string>

#include "cpp_bus_driver.h"
#include "chip/esp32p4/sd_card.h"
#include "device/common/async_init_manager.h"
#include "device/common/pixel_format.h"
#include "driver/sdmmc_host.h"
// #include "driver/spi_common.h"
#include "chip/esp32p4/driver.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "lr20xx/lr20xx_driver.h"
#include "device/t_glasses_p4/config.h"

namespace lilygo_device_driver {
namespace t_glasses_p4::device {

enum class ScreenType {
  kS023msafjf10111e1,
};

struct ScreenInfo {
  ScreenType type;
  const char* name;
  int width;
  int height;
  int bits_per_pixel;
  const char* pixel_format;
  double mipi_dsi_dpi_clk_mhz;
  int mipi_dsi_hsync;
  int mipi_dsi_hbp;
  int mipi_dsi_hfp;
  int mipi_dsi_vsync;
  int mipi_dsi_vbp;
  int mipi_dsi_vfp;
  int data_lane_num;
  double lane_bit_rate_mbps;
};

struct DeviceModelInfo {
  const char* name;
  const char* version;
};

inline constexpr DeviceModelInfo kDeviceModelInfo = {
    .name = "T-Glasses-P4",
    .version = "v1.0",
};

// 非屏幕设备信息暂不启用，保留定义供后续接入。
// struct CameraInfo {
//   CameraType type;
//   const char* name;
//   const char* pixel_format;
//   int bits_per_pixel;
//   int buffer_count;
// };
//
// inline constexpr CameraInfo kCameraInfo = {
//     .type = camera::kType,
//     .name = GetCameraTypeName(camera::kType),
//     .pixel_format = GetRgbPixelFormatName(camera::kBitsPerPixel),
//     .bits_per_pixel = camera::kBitsPerPixel,
//     .buffer_count = camera::kBufferCount,
// };
//
// struct BatteryInfo {
//   const char* charger_chip_name;
//   const char* fuel_gauge_chip_name;
//   uint16_t capacity_mah;
// };
//
// 旧版电池信息；新板充电芯片为 BQ25896，容量需随实际电池核对。
// inline constexpr BatteryInfo kBatteryInfo = {
//     .charger_chip_name = "sy6970",
//     .fuel_gauge_chip_name = "bq27220",
//     .capacity_mah = 650,
// };

struct DeviceInfo {
  DeviceModelInfo model;
  ScreenInfo screen;
  // CameraInfo camera;
  // BatteryInfo battery;
};

}  // namespace t_glasses_p4::device

class TGlassesP4Driver {
 public:
  enum class InitMode { kAsync, kSync };

  // 旧版音频接口参考；新板为 ES8389，接入时需核对对应驱动 API。
  // enum class Es8311OperatingMode {
  //   kSleep,     // 关闭 ADC、DAC 和模拟偏置。
  //   kPlayback,  // 仅开启播放路径。
  //   kCapture,   // 仅开启采集路径。
  //   kDuplex,    // 同时开启采集和播放路径。
  // };
  //
  enum class Es8389OperatingMode {
    kActive,
    kSleep,
  };
  //
  enum class Lr2021OperatingMode {
    kStandby,
    kSleep,
  };

  struct Bus {
    // 屏幕与 BQ25896 分别使用 I2C0、I2C1，SGM38121 使用 LP I2C0。
    std::shared_ptr<cpp_bus_driver::HardwareI2c> bq25896_i2c_bus;
    // std::shared_ptr<cpp_bus_driver::HardwareI2c> sy6970_i2c_bus;
    // 旧版 BQ27220 与充电芯片共享硬件 I2C 的字段参考。
    // std::shared_ptr<cpp_bus_driver::HardwareI2c> bq27220_i2c_bus;
    // BQ27220 与 BQ25896 共用引脚，恢复后共享 BQ25896 的硬件总线。
    // 恢复新板时使用下方字段，两个同名字段不可同时启用。
    // std::shared_ptr<cpp_bus_driver::HardwareI2c> bq27220_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> sgm38121_i2c_bus;
    // std::shared_ptr<cpp_bus_driver::HardwareI2c> aw86224_i2c_bus;
    // std::shared_ptr<cpp_bus_driver::HardwareI2c> es8311_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> screen_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareMipi> screen_mipi_bus;
    // std::shared_ptr<cpp_bus_driver::HardwareI2s> es8311_i2s_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2s> es8389_i2s_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> lr2021_spi_bus;
  };

  struct Chip {
    std::unique_ptr<cpp_bus_driver::Bq2589x> bq25896;
    // std::unique_ptr<cpp_bus_driver::Sy6970> sy6970;
    // std::unique_ptr<cpp_bus_driver::Bq27220> bq27220;
    std::unique_ptr<cpp_bus_driver::Sgm38121> sgm38121;
    // std::unique_ptr<cpp_bus_driver::Aw862xx> aw86224;
    // std::unique_ptr<cpp_bus_driver::Es8311> es8311;
    std::unique_ptr<usp_cpp_bus_driver::Lr20xx> lr2021;
    std::unique_ptr<cpp_bus_driver::S023msafjf10111e1> s023msafjf10111e1;
  };

  struct Status {
    struct {
      bool init_flag = false;
    } bq25896;

    // struct {
    //   bool init_flag = false;
    // } sy6970;
    //
    // struct {
    //   bool init_flag = false;
    // } bq27220;
    //
    struct {
      bool init_flag = false;
    } sgm38121;

    struct {
      bool init_flag = false;
    } s023msafjf10111e1;

    // struct {
    //   bool init_flag = false;
    //   cpp_bus_driver::Aw862xx::RamWaveformSelection ram_waveform_selection;
    // } aw86224;
    //
    // struct {
    //   bool init_flag = false;
    // } es8311;
    //
    struct {
      bool init_flag = false;
    } es8389;
    //
    struct {
      bool init_flag = false;
    } lr2021;
    //
    struct {
      bool init_flag = false;
    } sd_card;
  };

  static TGlassesP4Driver& GetInstance();

  const Bus& bus() const { return bus_; }
  const Chip& chip() const { return chip_; }
  const Status& status() const { return status_; }

  const t_glasses_p4::device::DeviceModelInfo& device_model_info() const {
    return t_glasses_p4::device::kDeviceModelInfo;
  }
  t_glasses_p4::device::ScreenType screen_type() const;
  const t_glasses_p4::device::ScreenInfo& screen_info() const;
  // const t_glasses_p4::device::CameraInfo& camera_info() const {
  //   return t_glasses_p4::device::kCameraInfo;
  // }
  // const t_glasses_p4::device::BatteryInfo& battery_info() const {
  //   return t_glasses_p4::device::kBatteryInfo;
  // }
  esp_codec_dev_handle_t es8389_input_codec_dev() const {
    return es8389_input_codec_dev_;
  }
  esp_codec_dev_handle_t es8389_output_codec_dev() const {
    return es8389_output_codec_dev_;
  }
  t_glasses_p4::device::DeviceInfo device_info() const {
    return {
        .model = device_model_info(),
        .screen = screen_info(),
        // .camera = camera_info(),
        // .battery = battery_info(),
    };
  }

  bool Init(InitMode mode = InitMode::kSync);
  bool InitMinimal();

  /**
   * @brief 初始化 BQ25896 并更新设备状态。
   * @return 芯片识别和配置成功返回 true；供电、总线或配置失败返回 false。
   * @note 驱动对象创建和整板上电由上层初始化流程负责。
   * @note 已就绪时直接返回，不覆盖应用后续设置的充电参数。
   */
  bool InitBq25896();

  bool InitSgm38121();
  bool InitS023msafjf10111e1();
  // 以下外围暂未实现，不参与 Init/InitMinimal，调用均返回 false。
  // bool InitSy6970();  // 旧版接口参考，新板为 BQ25896。
  bool InitBq27220();
  // bool InitEs8311();  // 旧版接口参考，新板为 ES8389。
  bool InitEs8389();
  bool InitAw86224();
  bool InitBhi260ap();
  bool InitBmm350();
  bool InitLr2021();
  bool InitPower();
  bool InitScreen();
  bool InitSdmmc(const char* base_path,
      int max_freq_khz = SDMMC_FREQ_DEFAULT);
  // bool InitSdspi(const char* base_path, spi_host_device_t host_id,
  //     int max_freq_khz = SDMMC_FREQ_DEFAULT);

  // bool DeinitAw86224();
  // bool DeinitEs8311();
  bool DeinitEs8389();
  bool DeinitLr2021();
  bool DeinitScreen();
  bool DeinitSdmmc();

  // bool IsSy6970Ready() const;
  // bool IsBq27220Ready() const;
  bool IsBq25896Ready() const;
  bool IsSgm38121Ready() const;
  bool IsS023msafjf10111e1Ready() const;
  // bool IsAw86224Ready() const;
  // bool IsEs8311Ready() const;
  bool IsEs8389Ready() const;
  bool IsLr2021Ready() const;
  bool IsScreenReady() const;
  bool IsSdmmcReady() const;

  // bool SetAw86224Standby();
  // bool SetEs8311OperatingMode(Es8311OperatingMode mode);
  bool SetEs8389OperatingMode(Es8389OperatingMode mode);
  bool SetLr2021OperatingMode(Lr2021OperatingMode mode);
  bool SetEsp32c5PowerEnabled(bool enabled);
  bool SetCameraPowerEnabled(bool enabled);
  bool PrepareMinimalDriversForPowerOff();
  bool PrepareDriversForPowerOff();

  /**
   * @brief 相对设备默认显示方向设置镜像并核对读回状态
   * @param horizontal 是否左右镜像
   * @param vertical 是否上下镜像
   * @return 屏幕芯片已就绪且写入、读回一致返回 true，否则返回 false
   * @note 两个参数均为 false 时恢复默认方向，重复调用不会叠加翻转。
   */
  bool SetScreenMirror(bool horizontal, bool vertical);

 private:
  void CreateDrivers();

  /**
   * @brief 按指定模式执行设备初始化流程。
   * @param mode 初始化模式。
   * @return 同步初始化或异步任务启动成功返回 true，失败返回 false。
   */
  bool InitDrivers(InitMode mode);

  /**
   * @brief 执行最小驱动集合的初始化流程。
   * @return 初始化成功返回 true，失败返回 false。
   */
  bool InitMinimalDrivers();

  bool DeinitPower();

  AsyncInitManager async_init_manager_;
  std::unique_ptr<cpp_bus_driver::PlatformHal> platform_hal_;
  Bus bus_;
  Chip chip_;
  Status status_;
  SdCard sd_card_;
  // const t_glasses_p4::device::ScreenInfo* screen_info_ = nullptr;
  bool minimal_drivers_initialized_ = false;
  bool power_initialized_ = false;

  const audio_codec_ctrl_if_t* es8389_ctrl_if_ = nullptr;
  const audio_codec_data_if_t* es8389_data_if_ = nullptr;
  const audio_codec_gpio_if_t* es8389_gpio_if_ = nullptr;
  const audio_codec_if_t* es8389_codec_if_ = nullptr;
  esp_codec_dev_handle_t es8389_input_codec_dev_ = nullptr;
  esp_codec_dev_handle_t es8389_output_codec_dev_ = nullptr;
  Es8389OperatingMode es8389_operating_mode_ = Es8389OperatingMode::kSleep;

  TGlassesP4Driver() = default;
  ~TGlassesP4Driver() = default;

  TGlassesP4Driver(const TGlassesP4Driver&) = delete;
  TGlassesP4Driver& operator=(const TGlassesP4Driver&) = delete;
};

}  // namespace lilygo_device_driver
