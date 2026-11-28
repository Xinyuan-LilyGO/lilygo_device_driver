/*
 * @Description: T-Glasses-P4 设备驱动接口
 */
#pragma once

#include <memory>
#include <string>

#include "bhi2xy_sensorapi_cpp_bus_driver.h"
#include "cpp_bus_driver.h"
#include "chip/esp32p4/sd_card.h"
#include "device/common/async_init_manager.h"
#include "device/common/pixel_format.h"
#include "driver/sdmmc_host.h"
#include "driver/spi_common.h"
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

struct DeviceInfo {
  DeviceModelInfo model;
  ScreenInfo screen;
  CameraInfo camera;
};

}  // namespace t_glasses_p4::device

class TGlassesP4Driver {
 public:
  enum class InitMode { kAsync, kSync };

  enum class Es8389OperatingMode {
    kActive,
    kSleep,
  };

  enum class Lr2021OperatingMode {
    kStandby,
    kSleep,
  };
  enum class UartTarget {
    kEsp32p4,
    kEsp32c5,
  };

  using Aw93307TouchEvent = cpp_bus_driver::Aw933xx::SliderEvent;

  struct Aw93307TouchState {
    bool button_touched = false;
    bool slider_touched = false;
    bool slider_position_valid = false;
    int32_t slider_position = 0;
    Aw93307TouchEvent button_event{};
    Aw93307TouchEvent slider_event{};
  };

  static TGlassesP4Driver& GetInstance();

  const auto& bus() const { return bus_; }
  const auto& chip() const { return chip_; }
  const auto& chip_status() const { return chip_status_; }

  const t_glasses_p4::device::DeviceModelInfo& device_model_info() const {
    return t_glasses_p4::device::kDeviceModelInfo;
  }
  t_glasses_p4::device::ScreenType screen_type() const;
  const t_glasses_p4::device::ScreenInfo& screen_info() const;
  const t_glasses_p4::device::CameraInfo& camera_info() const {
    return t_glasses_p4::device::kCameraInfo;
  }
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
        .camera = camera_info(),
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

  bool InitBq27220();
  bool InitSgm38121();
  bool InitRx8130ce();
  bool InitS023msafjf10111e1();
  bool InitAw86224();
  bool InitAw93307();
  bool InitEs8389();
  bool InitBhi260ap();
  bool InitLr2021();
  bool InitPower();
  bool InitSdmmc(const char* base_path,
      int max_freq_khz = SDMMC_FREQ_DEFAULT);
  bool InitSdspi(const char* base_path, spi_host_device_t host_id,
      int max_freq_khz = SDMMC_FREQ_DEFAULT);

  bool DeinitAw86224();
  bool DeinitAw93307();
  bool DeinitBhi260ap();
  bool DeinitBq27220();
  bool DeinitEs8389();
  bool DeinitLr2021();
  bool DeinitRx8130ce();
  bool DeinitS023msafjf10111e1();
  bool DeinitSdmmc();

  bool IsBq25896Ready() const;
  bool IsBq27220Ready() const;
  bool IsSgm38121Ready() const;
  bool IsRx8130ceReady() const;
  bool IsS023msafjf10111e1Ready() const;
  bool IsAw86224Ready() const;
  bool IsAw93307Ready() const;
  bool IsEs8389Ready() const;
  bool IsBhi260apReady() const;
  bool IsLr2021Ready() const;
  bool IsSdmmcReady() const;

  bool SetAw86224Standby();
  bool SetEs8389OperatingMode(Es8389OperatingMode mode);
  bool SetLr2021OperatingMode(Lr2021OperatingMode mode);
  bool SetEsp32c5PowerEnabled(bool enabled);
  bool SetCameraPowerEnabled(bool enabled);

  /**
   * @brief 设置外设 3.3V 电源使能，不自动处理信号 IO 隔离
   * @param enabled true 开启供电，false 关闭供电
   * @return 电平写入成功返回 true，平台未创建或写入失败返回 false
   *
   * 输出模式由 InitPower 配置，此接口只切换电平，不改变 GPIO 模式。
   * 此电源为多个外设共用，板级驱动在初始化时开启，关机准备时不主动关闭。
   * 关闭前须停止外设通信并释放驱动，调用方须配合信号 IO 高阻隔离，
   * 避免反向供电导致芯片断电不完全、下次初始化时 I2C 设备无响应。
   */
  bool Set3v3PowerEnabled(bool enabled);

  /**
   * @brief 设置音频电源使能
   * @param enabled true 开启供电，false 关闭供电
   * @return GPIO 电平写入成功返回 true，否则返回 false
   * @note InitPower 配置输出模式并默认关闭该电源。该电源同时为 ES8389 AVDD
   * 和功放供电，使用音频前开启，停止音频后关闭。
   */
  bool SetAudioPowerEnabled(bool enabled);

  bool PrepareMinimalDriversForPowerOff();
  bool PrepareDriversForPowerOff();

  /**
   * @brief 开启 ESP32-C5 电源并将 USB 切换至板载 CH343P
   * @return 最小板级驱动已就绪且 GPIO 操作成功时返回 true
   * @note 释放 P4 对 C5 EN 的控制，由主机工具通过 CH343P 的 DTR/RTS 进入下载模式。
   */
  bool EnterEsp32c5DownloadMode();

  /**
   * @brief 切换 USB-C 连接的串口设备，不复位处理器
   * @param target ESP32-P4 原生 USB 串口或 ESP32-C5 的板载 CH343P
   * @return USB 选择脚配置成功返回 true，否则返回 false
   * @note 切换后主机重新枚举 USB 设备，需重新连接对应串口。
   */
  bool SetUartTarget(UartTarget target);

  /**
   * @brief 相对设备默认显示方向设置镜像并核对读回状态
   * @param horizontal 是否左右镜像
   * @param vertical 是否上下镜像
   * @return 屏幕芯片已就绪且写入、读回一致返回 true，否则返回 false
   * @note 两个参数均为 false 时恢复默认方向，重复调用不会叠加翻转。
   */
  bool SetScreenMirror(bool horizontal, bool vertical);

  /**
   * @brief 空载校准触摸按键和滑条，等待校准完成并清除旧手势。
   * @return 触摸驱动已就绪且校准操作完成返回 true。
   */
  bool CalibrateAw93307();

  /**
   * @brief 轮询触摸按键和滑条，并读清本次硬件手势事件。
   * @param state 输出触摸状态、手势和从后往前的滑条位置（0~100）。
   * 仅在 slider_position_valid 为 true 时使用位置；读取失败时输出清零。
   * @return 读取成功返回 true，无触摸时也返回 true。
   * @note 由单个任务轮询，不与校准或反初始化并行调用。
   */
  bool ReadAw93307Touch(Aw93307TouchState& state);

 private:
  struct Bus {
    // BQ25896 与 BQ27220 共用 I2C1 主总线，使用独立设备地址。
    std::shared_ptr<cpp_bus_driver::HardwareI2c> bq25896_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> bq27220_i2c_bus;
    // SGM38121 使用 LP I2C0。
    std::shared_ptr<cpp_bus_driver::HardwareI2c> sgm38121_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> rx8130ce_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> aw86224_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> aw93307_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> screen_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c> bhi260ap_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareMipi> screen_mipi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2s> es8389_i2s_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> lr2021_spi_bus;
  };

  struct Chip {
    std::unique_ptr<cpp_bus_driver::Bq2589x> bq25896;
    std::unique_ptr<cpp_bus_driver::Bq27220> bq27220;
    std::unique_ptr<cpp_bus_driver::Sgm38121> sgm38121;
    std::unique_ptr<cpp_bus_driver::Rx8130ce> rx8130ce;
    std::unique_ptr<cpp_bus_driver::Aw862xx> aw86224;
    std::unique_ptr<cpp_bus_driver::Aw933xx> aw93307;
    std::unique_ptr<bhi2xy_sensorapi_cpp_bus_driver::Bhi2xy> bhi260ap;
    std::unique_ptr<usp_cpp_bus_driver::Lr20xx> lr2021;
    std::unique_ptr<cpp_bus_driver::S023msafjf10111e1> s023msafjf10111e1;
  };

  struct ChipStatus {
    struct {
      bool init_flag = false;
    } bq25896;

    struct {
      bool init_flag = false;
    } bq27220;

    struct {
      bool init_flag = false;
    } sgm38121;

    struct {
      bool init_flag = false;
    } rx8130ce;

    struct {
      bool init_flag = false;
    } s023msafjf10111e1;

    struct {
      bool init_flag = false;
      cpp_bus_driver::Aw862xx::RamWaveformInfo ram_waveform_info;
    } aw86224;

    struct {
      bool init_flag = false;
    } aw93307;

    struct {
      bool init_flag = false;
    } es8389;

    struct {
      bool init_flag = false;
    } lr2021;

    struct {
      bool init_flag = false;
    } sd_card;
  };

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
  ChipStatus chip_status_;
  SdCard sd_card_;
  bool minimal_drivers_initialized_ = false;
  bool power_initialized_ = false;
  bool aw93307_slider_touched_ = false;
  uint8_t aw93307_slider_transition_samples_ = 0;

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
