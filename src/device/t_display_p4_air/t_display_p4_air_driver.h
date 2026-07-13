/*
 * @Description: T-Display-P4-Air 板级设备驱动接口
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-06-18 16:01:52
 * @License: GPL 3.0
 */

#pragma once

#include <cstdint>
#include <memory>

#include "cpp_bus_driver_library.h"
#include "esp32p4_driver.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "radiolib_cpp_bus_driver_library.h"
#include "t_display_p4_air_config.h"

namespace lilygo_device_driver {
namespace t_display_p4_air::device {

// 支持的屏幕类型。
enum class ScreenType {
  kUnknown,  // 未识别屏幕。
  kHi8561,   // HI8561 屏幕。
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

// T-Display-P4-Air 板级总线、芯片和存储设备驱动。
class TDisplayP4AirDriver {
 public:
  // 驱动初始化任务的执行模式。
  enum class InitMode { kAsync, kSync };

  // 板级设备休眠等级。
  enum class SleepLevel {
    kLight,
    kDeep,
  };
  // 外部串口连接目标。
  enum class UartTarget {
    kEsp32p4,
    kEsp32c5,
  };

  // 板级通信总线和 RadioLib 适配对象集合。
  struct Bus {
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> axp517_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> xl9535_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> sgm38121_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> aw86224_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> hi8561_i2c_touch_bus;
    std::shared_ptr<cpp_bus_driver::HardwareMipi> screen_mipi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2s> es8389_i2s_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> lr1121_spi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareUart> nrf9151_uart_bus;

    RadioLibHal* lr1121_radiolib_hal = nullptr;
    Module* lr1121_module = nullptr;
  };

  // 板级芯片驱动对象集合。
  struct Chip {
    std::unique_ptr<cpp_bus_driver::Axp517> axp517;
    std::unique_ptr<cpp_bus_driver::Xl95x5> xl9535;
    std::unique_ptr<cpp_bus_driver::Sgm38121> sgm38121;
    std::unique_ptr<cpp_bus_driver::Aw862xx> aw86224;
    std::unique_ptr<cpp_bus_driver::Hi8561> hi8561;
    std::unique_ptr<cpp_bus_driver::Hi8561Touch> hi8561_touch;
    std::unique_ptr<cpp_bus_driver::Pwm> hi8561_backlight;
    std::unique_ptr<cpp_bus_driver::Nrf9151> nrf9151;

    LR1121* lr1121 = nullptr;
  };

  // 板级芯片和存储设备初始化状态集合。
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
    } hi8561;

    struct {
      bool init_flag = false;
    } hi8561_touch;

    struct {
      bool init_flag = false;
    } hi8561_backlight;

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

  /**
   * @brief 获取 T-Display-P4-Air 驱动单例。
   * @return 驱动单例引用。
   */
  static TDisplayP4AirDriver& GetInstance();

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
  const t_display_p4_air::device::DeviceModelInfo& device_model_info() const {
    return t_display_p4_air::device::kDeviceModelInfo;
  }

  /**
   * @brief 获取当前屏幕的参数信息。
   * @return 当前屏幕参数信息引用。
   */
  const t_display_p4_air::device::ScreenInfo& screen_info() const;

  /**
   * @brief 获取当前屏幕类型。
   * @return 当前屏幕类型。
   */
  t_display_p4_air::device::ScreenType screen_type() const {
    return screen_info().type;
  }

  /**
   * @brief 获取摄像头参数信息。
   * @return 摄像头参数信息引用。
   */
  const t_display_p4_air::device::CameraInfo& camera_info() const {
    return t_display_p4_air::device::kCameraInfo;
  }

  /**
   * @brief 获取 ES8389 输入编解码设备句柄。
   * @return 输入编解码设备句柄，未创建时返回 nullptr。
   */
  esp_codec_dev_handle_t es8389_input_codec_dev() const {
    return es8389_input_codec_dev_;
  }

  /**
   * @brief 获取 ES8389 输出编解码设备句柄。
   * @return 输出编解码设备句柄，未创建时返回 nullptr。
   */
  esp_codec_dev_handle_t es8389_output_codec_dev() const {
    return es8389_output_codec_dev_;
  }

  /**
   * @brief 获取聚合后的设备参数信息。
   * @return 聚合设备参数信息。
   */
  t_display_p4_air::device::DeviceInfo device_info() const {
    return {
        .model = device_model_info(),
        .screen = screen_info(),
        .camera = camera_info(),
    };
  }

  /**
   * @brief 判断 AXP517 电源芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsAxp517Ready() const;

  /**
   * @brief 判断 XL9535 IO 扩展芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsXl9535Ready() const;

  /**
   * @brief 判断 SGM38121 电源芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsSgm38121Ready() const;

  /**
   * @brief 判断 AW86224 振动芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsAw86224Ready() const;

  /**
   * @brief 判断 HI8561 屏幕芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsHi8561Ready() const;

  /**
   * @brief 判断 HI8561 触摸芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsHi8561TouchReady() const;

  /**
   * @brief 判断 HI8561 背光驱动是否已经初始化完成。
   * @return 背光驱动可用返回 true，否则返回 false。
   */
  bool IsHi8561BacklightReady() const;

  /**
   * @brief 判断 ES8389 音频编解码器是否已经初始化完成。
   * @return 音频编解码器可用返回 true，否则返回 false。
   */
  bool IsEs8389Ready() const;

  /**
   * @brief 判断 LR1121 射频芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsLr1121Ready() const;

  /**
   * @brief 判断 NRF9151 通信模块是否已经初始化完成。
   * @return 模块可用返回 true，否则返回 false。
   */
  bool IsNrf9151Ready() const;

  /**
   * @brief 判断当前屏幕及其背光总线是否已经可用。
   * @return 屏幕可用返回 true，否则返回 false。
   */
  bool IsScreenReady() const;

  /**
   * @brief 创建设备驱动指针。
   */
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
   * @brief 初始化 AXP517 电源管理芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitAxp517();

  /**
   * @brief 初始化 XL9535 IO 扩展芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitXl9535();

  /**
   * @brief 配置 XL9535 的板级 IO 功能和默认电平。
   * @return 配置成功时返回 true，否则返回 false。
   */
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

  /**
   * @brief 初始化并配置 SGM38121 摄像头电源芯片。
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
   * @brief 初始化板载触摸驱动。
   * @return 触摸驱动初始化成功时返回 true，否则返回 false。
   */
  bool InitTouch();

  /**
   * @brief 释放触摸驱动并清除触摸状态。
   * @return 触摸驱动释放成功时返回 true，否则返回 false。
   */
  bool DeinitTouch();

  /**
   * @brief 初始化屏幕背光驱动。
   * @return 背光驱动初始化成功时返回 true，否则返回 false。
   */
  bool InitScreenBacklight();

  /**
   * @brief 释放屏幕背光驱动并清除背光状态。
   * @return 背光驱动释放成功时返回 true，否则返回 false。
   */
  bool DeinitScreenBacklight();

  /**
   * @brief 初始化 HI8561 显示芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitHi8561();

  /**
   * @brief 初始化 HI8561 配套触摸芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitHi8561Touch();

  /**
   * @brief 初始化 HI8561 配套背光驱动。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitHi8561Backlight();

  /**
   * @brief 初始化 AW86224 振动芯片并加载波形库。
   * @return 初始化和波形加载成功时返回 true，否则返回 false。
   */
  bool InitAw86224();

  /**
   * @brief 初始化 ES8389 音频编解码设备。
   * @return 输入和输出设备初始化成功时返回 true，否则返回 false。
   */
  bool InitEs8389();

  /**
   * @brief 配置 ES8389 的输入、输出和音量参数。
   * @return 配置成功时返回 true，否则返回 false。
   */
  bool ConfigEs8389();

  /**
   * @brief 初始化 LR1121 射频芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitLr1121();

  /**
   * @brief 初始化 NRF9151 通信模块。
   * @return 初始化成功时返回 true，否则返回 false。
   */
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

/**
 * @brief 使用 T-Display-P4-Air 驱动单例挂载 SPIFFS 文件系统。
 * @param base_path 文件系统挂载路径。
 * @param spiffs_conf 返回挂载时使用的 SPIFFS 配置。
 * @return SPIFFS 挂载成功时返回 true，否则返回 false。
 */
bool InitSpiffs(const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf);

/**
 * @brief 使用 T-Display-P4-Air 驱动单例挂载 SDMMC 存储卡。
 * @param base_path SD 卡挂载路径。
 * @param max_freq_khz SDMMC 总线最大频率，单位为 kHz。
 * @return SD 卡挂载成功时返回 true，否则返回 false。
 */
bool InitSdmmc(const char* base_path, int max_freq_khz = SDMMC_FREQ_DEFAULT);

/**
 * @brief 使用 T-Display-P4-Air 驱动单例挂载 SDSPI 存储卡。
 * @param base_path SD 卡挂载路径。
 * @param host_id SD 卡使用的 SPI 主机。
 * @param max_freq_khz SDSPI 总线最大频率，单位为 kHz。
 * @return SD 卡挂载成功时返回 true，否则返回 false。
 */
bool InitSdspi(const char* base_path, spi_host_device_t host_id,
    int max_freq_khz = SDMMC_FREQ_DEFAULT);

}  // namespace lilygo_device_driver
