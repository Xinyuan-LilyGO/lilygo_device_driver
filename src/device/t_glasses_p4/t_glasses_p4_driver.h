/*
 * @Description: T-Glasses-P4 板级设备驱动接口
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:58:49
 * @LastEditTime: 2026-08-03 16:14:12
 * @License: GPL 3.0
 */

#pragma once

#include <cstdint>
#include <memory>

#include "cpp_bus_driver_library.h"
#include "driver/sdmmc_host.h"
#include "driver/spi_common.h"
#include "esp32p4_driver.h"
#include "sx126x/sx126x_driver.h"
#include "t_glasses_p4_config.h"

namespace lilygo_device_driver {
namespace t_glasses_p4::device {

enum class ScreenType {
  kS023msafjf10111e1,
};

// 屏幕型号、分辨率和 MIPI 参数信息
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

// 设备型号名称和版本信息
struct DeviceModelInfo {
  const char* name;
  const char* version;
};

inline constexpr DeviceModelInfo kDeviceModelInfo = {
    .name = "T-Glasses-P4",
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

// 充电芯片、电量计和电池容量信息
struct BatteryInfo {
  const char* charger_name;
  const char* fuel_gauge_name;
  uint16_t capacity_mah;
};

inline constexpr BatteryInfo kBatteryInfo = {
    .charger_name = "sy6970",
    .fuel_gauge_name = "bq27220",
    .capacity_mah = 650,
};

// T-Glasses-P4 聚合设备信息
struct DeviceInfo {
  DeviceModelInfo model;
  ScreenInfo screen;
  CameraInfo camera;
  BatteryInfo battery;
};

}  // namespace t_glasses_p4::device

class TGlassesP4Driver {
 public:
  enum class InitMode { kAsync, kSync };
  enum class PowerState {
    kActive,
    kSleep,
    kOff,
  };

  enum class Es8311PowerState {
    kSleep,     // 关闭 ADC、DAC 和模拟偏置。
    kPlayback,  // 仅开启播放路径。
    kCapture,   // 仅开启采集路径。
    kDuplex,    // 同时开启采集和播放路径。
  };

  enum class Sx1262PowerState {
    kStandby,
    kSleep,
  };

  struct Bus {
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> sy6970_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> bq27220_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> sgm38121_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> aw86224_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> es8311_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> screen_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareMipi> screen_mipi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2s> es8311_i2s_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> sx1262_spi_bus;
  };

  struct Chip {
    std::unique_ptr<cpp_bus_driver::Sy6970> sy6970;
    std::unique_ptr<cpp_bus_driver::Bq27220> bq27220;
    std::unique_ptr<cpp_bus_driver::Sgm38121> sgm38121;
    std::unique_ptr<cpp_bus_driver::Aw862xx> aw86224;
    std::unique_ptr<cpp_bus_driver::Es8311> es8311;
    std::unique_ptr<usp_cpp_bus_driver::Sx126x> sx1262;
    std::unique_ptr<cpp_bus_driver::S023msafjf10111e1> s023msafjf10111e1;
  };

  struct Status {
    struct {
      bool init_flag = false;
    } sy6970;

    struct {
      bool init_flag = false;
    } bq27220;

    struct {
      bool init_flag = false;
    } sgm38121;

    struct {
      bool init_flag = false;
    } s023msafjf10111e1;

    struct {
      bool init_flag = false;
      cpp_bus_driver::Aw862xx::RamWaveformSelection ram_waveform_selection;
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
    } sx1262;

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
  const t_glasses_p4::device::CameraInfo& camera_info() const {
    return t_glasses_p4::device::kCameraInfo;
  }
  const t_glasses_p4::device::BatteryInfo& battery_info() const {
    return t_glasses_p4::device::kBatteryInfo;
  }
  t_glasses_p4::device::DeviceInfo device_info() const {
    return {
        .model = device_model_info(),
        .screen = screen_info(),
        .camera = camera_info(),
        .battery = battery_info(),
    };
  }

  bool IsSy6970Ready() const;
  bool IsBq27220Ready() const;
  bool IsSgm38121Ready() const;
  bool IsS023msafjf10111e1Ready() const;
  bool IsAw86224Ready() const;
  bool IsEs8311Ready() const;
  bool IsSx1262Ready() const;
  bool IsScreenReady() const;

  void CreateDrivers();

  /**
   * @brief 初始化已经创建的板级驱动。
   * @param mode 初始化模式。
   * @return 必需驱动初始化成功时返回 true，否则返回 false。
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
  bool SetAw86224Standby();
  bool SetEs8311PowerState(Es8311PowerState state);
  bool SetSx1262PowerState(Sx1262PowerState state);

  bool InitPower();
  bool InitSy6970();
  bool InitBq27220();
  bool InitSgm38121();

  bool InitScreen();
  bool DeinitScreen();
  bool InitS023msafjf10111e1();

  bool InitAw86224();
  bool InitEs8311();
  bool ConfigEs8311();
  bool InitSx1262();

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
  const t_glasses_p4::device::ScreenInfo* screen_info_ = nullptr;

  TGlassesP4Driver() = default;
  ~TGlassesP4Driver() = default;

  // 禁止拷贝构造和赋值。
  TGlassesP4Driver(const TGlassesP4Driver&) = delete;
  TGlassesP4Driver& operator=(const TGlassesP4Driver&) = delete;
};

/**
 * @brief 通过 SDMMC 主机挂载 SD 卡。
 * @param base_path SD 卡挂载路径。
 * @param max_freq_khz SDMMC 总线最大频率，单位为 kHz。
 * @return SD 卡挂载成功时返回 true，否则返回 false。
 */
bool InitSdmmc(const char* base_path, int max_freq_khz = SDMMC_FREQ_DEFAULT);

/**
 * @brief 通过 SDSPI 主机挂载 SD 卡。
 * @param base_path SD 卡挂载路径。
 * @param host_id SD 卡使用的 SPI 主机。
 * @param max_freq_khz SDSPI 总线最大频率，单位为 kHz。
 * @return SD 卡挂载成功时返回 true，否则返回 false。
 */
bool InitSdspi(const char* base_path, spi_host_device_t host_id,
    int max_freq_khz = SDMMC_FREQ_DEFAULT);

}  // namespace lilygo_device_driver
