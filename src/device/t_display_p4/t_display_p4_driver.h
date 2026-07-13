/*
 * @Description: T-Display-P4 板级设备驱动接口
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-05-21 18:05:18
 * @License: GPL 3.0
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "ICM20948_WE.h"
#include "cpp_bus_driver_library.h"
#include "esp32p4_driver.h"
#include "radiolib_cpp_bus_driver_library.h"
#include "t_display_p4_keyboard_config.h"

namespace lilygo_device_driver {
namespace t_display_p4::device {

// 支持的屏幕类型。
enum class ScreenType {
  kUnknown,   // 未识别屏幕。
  kHi8561,    // HI8561 屏幕。
  kRm69a10,   // RM69A10 屏幕。
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

// T-Display-P4 板级总线、芯片和存储设备驱动。
class TDisplayP4Driver {
 public:
  // 驱动初始化任务的执行模式。
  enum class InitMode { kAsync, kSync };

  // 板级设备休眠等级。
  enum class SleepLevel {
    kLight,
    kDeep,
  };

  // CC1101 外部射频开关通路。
  enum class Cc1101RfSwitch {
    k315Mhz,
    k434Mhz,
    k868_915Mhz,
  };

  // 板级通信总线和 RadioLib 适配对象集合。
  struct Bus {
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> bq27220_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> xl9535_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> sgm38121_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> pcf8563_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> aw86224_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> es8311_i2c_bus;
    std::unique_ptr<TwoWire> icm20948_i2c_bus;
    std::shared_ptr<cpp_bus_driver::HardwareMipi> screen_mipi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2s> es8311_i2s_bus;
    std::shared_ptr<cpp_bus_driver::HardwareUart> l76k_uart_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> sx1262_spi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> hi8561_i2c_touch_bus;
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> gt9895_i2c_touch_bus;

    std::shared_ptr<cpp_bus_driver::SoftwareI2c> xl9555_i2c_bus;
    std::shared_ptr<cpp_bus_driver::SoftwareI2c> tca8418_i2c_bus;

    std::shared_ptr<cpp_bus_driver::HardwareSpi> cc1101_spi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> nrf24l01_spi_bus;

    RadioLibHal* cc1101_radiolib_hal = nullptr;
    RadioLibHal* nrf24l01_radiolib_hal = nullptr;

    Module* cc1101_module = nullptr;
    Module* nrf24l01_module = nullptr;
  };

  // 板级芯片驱动对象集合。
  struct Chip {
    std::unique_ptr<cpp_bus_driver::Xl95x5> xl9535;
    std::unique_ptr<cpp_bus_driver::Bq27220> bq27220;
    std::unique_ptr<cpp_bus_driver::Sgm38121> sgm38121;
    std::unique_ptr<cpp_bus_driver::Pcf8563x> pcf8563;
    std::unique_ptr<cpp_bus_driver::Aw862xx> aw86224;
    std::unique_ptr<cpp_bus_driver::Es8311> es8311;
    std::unique_ptr<cpp_bus_driver::L76k> l76k;
    std::unique_ptr<ICM20948_WE> icm20948;
    std::unique_ptr<cpp_bus_driver::Sx126x> sx1262;
    std::unique_ptr<cpp_bus_driver::Hi8561> hi8561;
    std::unique_ptr<cpp_bus_driver::Hi8561Touch> hi8561_touch;
    std::unique_ptr<cpp_bus_driver::Pwm> hi8561_backlight;
    std::unique_ptr<cpp_bus_driver::Rm69a10> rm69a10;
    std::unique_ptr<cpp_bus_driver::Gt9895> gt9895;

    std::unique_ptr<cpp_bus_driver::Xl95x5> xl9555;
    std::unique_ptr<cpp_bus_driver::Tca8418> tca8418;
    std::unique_ptr<cpp_bus_driver::Pwm> tca8418_backlight;

    CC1101* cc1101 = nullptr;
    nRF24* nrf24l01 = nullptr;
  };

  // 板级芯片和存储设备初始化状态集合。
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

  /**
   * @brief 获取 T-Display-P4 驱动单例。
   * @return 驱动单例引用。
   */
  static TDisplayP4Driver& GetInstance();

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
  const t_display_p4::device::DeviceModelInfo& device_model_info() const {
    return t_display_p4::device::kDeviceModelInfo;
  }

  /**
   * @brief 获取当前识别到的屏幕类型。
   * @return 当前屏幕类型。
   */
  t_display_p4::device::ScreenType screen_type() const;

  /**
   * @brief 获取当前屏幕的参数信息。
   * @return 当前屏幕参数信息引用。
   */
  const t_display_p4::device::ScreenInfo& screen_info() const;

  /**
   * @brief 获取摄像头参数信息。
   * @return 摄像头参数信息引用。
   */
  const t_display_p4::device::CameraInfo& camera_info() const {
    return t_display_p4::device::kCameraInfo;
  }

  /**
   * @brief 获取电池参数信息。
   * @return 电池参数信息引用。
   */
  const t_display_p4::device::BatteryInfo& battery_info() const {
    return t_display_p4::device::kBatteryInfo;
  }

  /**
   * @brief 获取聚合后的设备参数信息。
   * @return 聚合设备参数信息。
   */
  t_display_p4::device::DeviceInfo device_info() const {
    return {
        .model = device_model_info(),
        .screen = screen_info(),
        .camera = camera_info(),
        .battery = battery_info(),
    };
  }

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
   * @brief 判断 RM69A10 屏幕芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsRm69a10Ready() const;

  /**
   * @brief 判断 GT9895 触摸芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsGt9895Ready() const;

  /**
   * @brief 判断 BQ27220 电量计是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsBq27220Ready() const;

  /**
   * @brief 判断 PCF8563 RTC 芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsPcf8563Ready() const;

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
   * @brief 判断 L76K GPS 模块是否已经初始化完成。
   * @return 模块可用返回 true，否则返回 false。
   */
  bool IsL76kReady() const;

  /**
   * @brief 判断 ICM20948 IMU 芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsIcm20948Ready() const;

  /**
   * @brief 判断 SX1262 射频芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsSx1262Ready() const;

  /**
   * @brief 判断 XL9555 键盘 IO 扩展芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsXl9555Ready() const;

  /**
   * @brief 判断 TCA8418 键盘芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsTca8418Ready() const;

  /**
   * @brief 判断 TCA8418 键盘背光驱动是否已经初始化完成。
   * @return 背光驱动可用返回 true，否则返回 false。
   */
  bool IsTca8418BacklightReady() const;

  /**
   * @brief 判断 CC1101 射频芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsCc1101Ready() const;

  /**
   * @brief 判断 NRF24L01 射频芯片是否已经初始化完成。
   * @return 芯片可用返回 true，否则返回 false。
   */
  bool IsNrf24l01Ready() const;

  /**
   * @brief 判断当前屏幕及其背光总线是否已经可用。
   * @return 屏幕可用返回 true，否则返回 false。
   */
  bool IsScreenReady() const;

  /**
   * @brief 判断当前屏幕对应的触摸芯片是否已经可用。
   * @return 触摸芯片可用返回 true，否则返回 false。
   */
  bool IsTouchReady() const;

  /**
   * @brief 判断外接键盘设备是否已经探测成功。
   * @return 已检测到键盘返回 true，否则返回 false。
   */
  bool keyboard_connected() const { return keyboard_connected_; }

  /**
   * @brief 创建设备驱动指针。
   */
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
   * @brief 初始化 BQ27220 电量计并应用电池参数。
   * @return 初始化和配置成功时返回 true，否则返回 false。
   */
  bool InitBq27220();

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
   * @brief 初始化并配置 SGM38121 摄像头电源芯片。
   * @return 初始化和配置成功时返回 true，否则返回 false。
   */
  bool InitSgm38121();

  /**
   * @brief 根据当前屏幕类型初始化显示芯片。
   * @return 屏幕初始化成功时返回 true，否则返回 false。
   */
  bool InitScreen();

  /**
   * @brief 释放当前屏幕驱动并清除屏幕状态。
   * @return 屏幕释放成功时返回 true，否则返回 false。
   */
  bool DeinitScreen();

  /**
   * @brief 根据当前屏幕类型初始化触摸芯片。
   * @return 触摸芯片初始化成功时返回 true，否则返回 false。
   */
  bool InitTouch();

  /**
   * @brief 释放当前触摸驱动并清除触摸状态。
   * @return 触摸驱动释放成功时返回 true，否则返回 false。
   */
  bool DeinitTouch();

  /**
   * @brief 初始化当前屏幕使用的背光驱动。
   * @return 背光驱动初始化成功时返回 true，否则返回 false。
   */
  bool InitScreenBacklight();

  /**
   * @brief 释放当前屏幕背光驱动并清除背光状态。
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
   * @brief 初始化 RM69A10 显示芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitRm69a10();

  /**
   * @brief 初始化 GT9895 触摸芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitGt9895();

  /**
   * @brief 初始化 PCF8563 RTC 芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitPcf8563();

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
   * @brief 初始化 L76K GPS 模块及其串口通信。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitL76k();

  /**
   * @brief 初始化 ICM20948 IMU 和磁力计。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitIcm20948();

  /**
   * @brief 初始化 SX1262 射频芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitSx1262();

  /**
   * @brief 初始化外接键盘上的 XL9555 IO 扩展芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitXl9555();

  /**
   * @brief 配置 XL9555 的键盘、射频和指示灯 IO。
   * @return 配置成功时返回 true，否则返回 false。
   */
  bool ConfigXl9555();

  /**
   * @brief 初始化 TCA8418 键盘扫描芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitTca8418();

  /**
   * @brief 初始化 TCA8418 键盘背光驱动。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitTca8418Backlight();

  /**
   * @brief 初始化 CC1101 射频芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitCc1101();

  /**
   * @brief 初始化 NRF24L01 射频芯片。
   * @return 初始化成功时返回 true，否则返回 false。
   */
  bool InitNrf24l01();

  /**
   * @brief 探测并初始化外接键盘及其附属芯片。
   * @return 键盘初始化成功时返回 true，否则返回 false。
   */
  bool InitKeyboard();

  /**
   * @brief 释放外接键盘及其附属芯片驱动。
   * @return 键盘释放成功时返回 true，否则返回 false。
   */
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
  bool keyboard_connected_ = false;

  /**
   * @brief 通过 GT9895 触摸 ID 检测屏幕类型。
   * @return 检测流程完成时返回 true，否则返回 false。
   */
  bool DetectScreenType();

  /**
   * @brief 设置键盘扩展设备睡眠状态。
   * @param level 睡眠等级。
   * @param enable 是否开启该睡眠等级。
   * @return 设置成功返回 true，否则返回 false。
   */
  bool SetKeyboardSleep(SleepLevel level, bool enable);

  TDisplayP4Driver() = default;
  ~TDisplayP4Driver() = default;

  // 禁止拷贝构造和赋值。
  TDisplayP4Driver(const TDisplayP4Driver&) = delete;
  TDisplayP4Driver& operator=(const TDisplayP4Driver&) = delete;
};

}  // namespace lilygo_device_driver
