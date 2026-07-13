/*
 * @Description: T-Glasses-P4 板级设备驱动接口
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:58:49
 * @LastEditTime: 2026-05-24 17:00:00
 * @License: GPL 3.0
 */

#pragma once

#include <cstdint>
#include <memory>

#include "cpp_bus_driver_library.h"
#include "driver/sdmmc_host.h"
#include "driver/spi_common.h"
#include "esp32p4_driver.h"
#include "t_glasses_p4_config.h"

namespace lilygo_device_driver {
namespace t_glasses_p4::device {

// 支持的屏幕类型。
enum class ScreenType {
  kS023msafjf10111e1,  // S023MSAFJF10111E1 屏幕。
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

// T-Glasses-P4 板级总线、芯片和存储设备驱动。
class TGlassesP4Driver {
 public:
  // 驱动初始化任务的执行模式。
  enum class InitMode { kAsync, kSync };

  // 板级设备休眠等级。
  enum class SleepLevel {
    kLight,
    kDeep,
  };

  // 板级通信总线对象集合。
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

  // 板级芯片驱动对象集合。
  struct Chip {
    std::unique_ptr<cpp_bus_driver::Sy6970> sy6970;
    std::unique_ptr<cpp_bus_driver::Bq27220> bq27220;
    std::unique_ptr<cpp_bus_driver::Sgm38121> sgm38121;
    std::unique_ptr<cpp_bus_driver::Aw862xx> aw86224;
    std::unique_ptr<cpp_bus_driver::Es8311> es8311;
    std::unique_ptr<cpp_bus_driver::Sx126x> sx1262;
    std::unique_ptr<cpp_bus_driver::S023msafjf10111e1> s023msafjf10111e1;
  };

  // 板级芯片和存储设备初始化状态集合。
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
    } es8311;

    struct {
      bool init_flag = false;
    } sx1262;

    struct {
      bool init_flag = false;
    } sd_card;
  };

  /**
   * @brief 获取 T-Glasses-P4 驱动单例。
   * @return 驱动单例引用。
   */
  static TGlassesP4Driver& GetInstance();

  /**
   * @brief 获取板级总线对象集合。
   * @return 只读总线对象集合。
   */
  const Bus& bus() const { return bus_; }

  /**
   * @brief 获取板级芯片驱动对象集合。
   * @return 只读芯片驱动对象集合。
   */
  const Chip& chip() const { return chip_; }

  /**
   * @brief 获取板级驱动初始化状态集合。
   * @return 只读初始化状态集合。
   */
  const Status& status() const { return status_; }

  /**
   * @brief 获取设备型号信息。
   * @return 设备型号信息引用。
   */
  const t_glasses_p4::device::DeviceModelInfo& device_model_info() const {
    return t_glasses_p4::device::kDeviceModelInfo;
  }

  /**
   * @brief 获取当前屏幕类型。
   * @return 当前屏幕类型。
   */
  t_glasses_p4::device::ScreenType screen_type() const;

  /**
   * @brief 获取当前屏幕的参数信息。
   * @return 当前屏幕参数信息引用。
   */
  const t_glasses_p4::device::ScreenInfo& screen_info() const;

  /**
   * @brief 获取摄像头参数信息。
   * @return 摄像头参数信息引用。
   */
  const t_glasses_p4::device::CameraInfo& camera_info() const {
    return t_glasses_p4::device::kCameraInfo;
  }

  /**
   * @brief 获取电池参数信息。
   * @return 电池参数信息引用。
   */
  const t_glasses_p4::device::BatteryInfo& battery_info() const {
    return t_glasses_p4::device::kBatteryInfo;
  }

  /**
   * @brief 获取聚合后的设备参数信息。
   * @return 聚合设备参数信息。
   */
  t_glasses_p4::device::DeviceInfo device_info() const {
    return {
        .model = device_model_info(),
        .screen = screen_info(),
        .camera = camera_info(),
        .battery = battery_info(),
    };
  }

  /**
   * @brief 判断 SY6970 充电芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsSy6970Ready() const;

  /**
   * @brief 判断 BQ27220 电量计是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsBq27220Ready() const;

  /**
   * @brief 判断 SGM38121 电源芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsSgm38121Ready() const;

  /**
   * @brief 判断 S023MSAFJF10111E1 屏幕芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsS023msafjf10111e1Ready() const;

  /**
   * @brief 判断 AW86224 振动芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsAw86224Ready() const;

  /**
   * @brief 判断 ES8311 音频芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsEs8311Ready() const;

  /**
   * @brief 判断 SX1262 射频芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsSx1262Ready() const;

  /**
   * @brief 判断当前屏幕及其显示总线是否已经可用。
   * @return 屏幕可用返回 true，否则返回 false。
   */
  bool IsScreenReady() const;

  /**
   * @brief 创建设备驱动指针。
   */
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
   * @brief 开启或关闭指定板级休眠等级。
   * @param level 要控制的休眠等级。
   * @param enable 是否开启该休眠等级。
   * @return 休眠状态设置成功时返回 true，否则返回 false。
   */
  bool SetSleep(SleepLevel level, bool enable);

  /**
   * @brief 初始化板级 LDO 电源通道。
   * @return 所有电源通道初始化成功时返回 true，否则返回 false。
   */
  bool InitPower();

  /**
   * @brief 初始化 SY6970 充电管理芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitSy6970();

  /**
   * @brief 初始化 BQ27220 电量计并应用电池参数。
   * @return 初始化和配置成功时返回 true，否则返回 false。
   */
  bool InitBq27220();

  /**
   * @brief 初始化并配置 SGM38121 板级电源芯片。
   * @return 初始化和配置成功时返回 true，否则返回 false。
   */
  bool InitSgm38121();

  /**
   * @brief 初始化板载屏幕驱动。
   * @return 屏幕初始化成功时返回 true，否则返回 false。
   */
  bool InitScreen();

  /**
   * @brief 释放屏幕驱动并清除屏幕状态。
   * @return 屏幕释放成功时返回 true，否则返回 false。
   */
  bool DeinitScreen();

  /**
   * @brief 初始化 S023MSAFJF10111E1 显示芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitS023msafjf10111e1();

  /**
   * @brief 初始化 AW86224 振动芯片并加载波形库。
   * @return 初始化和波形加载成功时返回 true，否则返回 false。
   */
  bool InitAw86224();

  /**
   * @brief 初始化 ES8311 音频编解码芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitEs8311();

  /**
   * @brief 配置 ES8311 的电源、输入和音量参数。
   * @return 配置成功时返回 true，否则返回 false。
   */
  bool ConfigEs8311();

  /**
   * @brief 初始化 SX1262 射频芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
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
 * @brief 使用 T-Glasses-P4 驱动单例挂载 SDMMC 存储卡。
 * @param base_path SD 卡挂载路径。
 * @param max_freq_khz SDMMC 总线最大频率，单位为 kHz。
 * @return SD 卡挂载成功时返回 true，否则返回 false。
 */
bool InitSdmmc(const char* base_path, int max_freq_khz = SDMMC_FREQ_DEFAULT);

/**
 * @brief 使用 T-Glasses-P4 驱动单例挂载 SDSPI 存储卡。
 * @param base_path SD 卡挂载路径。
 * @param host_id SD 卡使用的 SPI 主机。
 * @param max_freq_khz SDSPI 总线最大频率，单位为 kHz。
 * @return SD 卡挂载成功时返回 true，否则返回 false。
 */
bool InitSdspi(const char* base_path, spi_host_device_t host_id,
    int max_freq_khz = SDMMC_FREQ_DEFAULT);

}  // namespace lilygo_device_driver
