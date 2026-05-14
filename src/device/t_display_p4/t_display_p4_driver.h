/*
 * @Description: t_display_p4_air_driver
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-05-15 00:57:17
 * @License: GPL 3.0
 */

#pragma once

#include "esp32p4_driver.h"

#if defined(CONFIG_BOARD_TYPE_T_DISPLAY_P4)
#include "t_display_p4_config.h"
#elif defined(CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD)
#include "t_display_p4_keyboard_config.h"
#else
#error "Missing required macro definition."
#endif

#include "cpp_bus_driver_library.h"

#if defined(CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD)
#include "radiolib_cpp_bus_driver_library.h"
#endif

#include "ICM20948_WE.h"

#if defined(CONFIG_BOARD_VERSION_T_DISPLAY_P4_V2_0)
#include "kode_bq25896.h"
#endif

#if defined(CONFIG_CAMERA_TYPE_SC2336)
#elif defined(CONFIG_CAMERA_TYPE_OV2710)
#elif defined(CONFIG_CAMERA_TYPE_OV5645)
#else
#error "Missing required macro definition."
#endif

namespace lilygo_device_driver {

namespace t_display_p4::device {

enum class ScreenType {
  kUnknown,
  kHi8561,
  kRm69a10,
};

struct ScreenDeviceInfo {
  ScreenType type;
  const char* name;
  int width;
  int height;
  int bits_per_pixel;
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

}  // namespace t_display_p4::device

class TDisplayP4Driver {
 public:
  enum class InitMode { kAsync, kSync };
  enum class SleepLevel {
    kChipSleep,
    kPowerOff,
  };

#if defined(CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD)
  enum class Cc1101RfSwitch {
    k315Mhz,
    k434Mhz,
    k868_915Mhz,
  };
#endif

  struct Bus {
#if defined(CONFIG_BOARD_VERSION_T_DISPLAY_P4_V2_0)
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> bq25896_i2c_bus;
#endif

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

#if defined(CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD)
    std::shared_ptr<cpp_bus_driver::SoftwareI2c> xl9555_i2c_bus;
    std::shared_ptr<cpp_bus_driver::SoftwareI2c> tca8418_i2c_bus;

    std::shared_ptr<cpp_bus_driver::HardwareSpi> cc1101_spi_bus;
    std::shared_ptr<cpp_bus_driver::HardwareSpi> nrf24l01_spi_bus;

    RadioLibHal* cc1101_radiolib_hal = nullptr;
    RadioLibHal* nrf24l01_radiolib_hal = nullptr;

    Module* cc1101_module = nullptr;
    Module* nrf24l01_module = nullptr;
#endif
  };

  struct Chip {
#if defined(CONFIG_BOARD_VERSION_T_DISPLAY_P4_V2_0)
    std::shared_ptr<kode_bq25896::bq25896_dev_t> bq25896_dev;
    kode_bq25896::bq25896_handle_t bq25896_handle = nullptr;
#endif

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

#if defined(CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD)

    std::unique_ptr<cpp_bus_driver::Xl95x5> xl9555;
    std::unique_ptr<cpp_bus_driver::Tca8418> tca8418;
    std::unique_ptr<cpp_bus_driver::Pwm> tca8418_backlight;

    CC1101* cc1101 = nullptr;
    nRF24* nrf24l01 = nullptr;
#endif
  };

  struct Status {
#if defined(CONFIG_BOARD_VERSION_T_DISPLAY_P4_V2_0)
    struct {
      bool init_flag = false;
    } bq25896;
#endif

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
      cpp_bus_driver::Aw862xx::RamWaveformSelection ram_waveform_selection;
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

#if defined(CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD)
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
#endif

    struct {
      bool init_flag = false;
    } sd_card;
  };

  static TDisplayP4Driver& GetInstance();

  const Bus& bus() const { return bus_; }
  const Chip& chip() const { return chip_; }
  const Status& status() const { return status_; }
  const t_display_p4::device::ScreenDeviceInfo& screen_info() const;
  t_display_p4::device::ScreenType screen_type() const;

  void CreateDrivers();
  bool InitDrivers(InitMode mode = InitMode::kSync);
  bool Init(InitMode mode = InitMode::kSync);
  bool SetSleep(SleepLevel level, bool enable);

  bool InitEsp32p4();
  bool InitPower();

#if defined(CONFIG_BOARD_VERSION_T_DISPLAY_P4_V2_0)
  bool InitBq25896();
#endif

  bool InitBq27220();
  bool InitXl9535();
  bool ConfigXl9535();
  bool InitSgm38121();
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
  bool InitSx1262();

#if defined(CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD)
  bool InitXl9555();
  bool ConfigXl9555();
  bool InitTca8418();
  bool InitTca8418Backlight();
  bool InitCc1101();
  bool InitNrf24l01();

  bool SetCc1101RfSwitch(Cc1101RfSwitch rf_switch);
#endif

  bool InitSpiffs(const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf);
  bool InitSdmmc(const char* base_path, int max_freq_khz = SDMMC_FREQ_DEFAULT);
  bool InitSdspi(const char* base_path, spi_host_device_t host_id,
      int max_freq_khz = SDMMC_FREQ_DEFAULT);

 private:
  std::unique_ptr<cpp_bus_driver::Tool> tool_;
  Bus bus_;
  Chip chip_;
  Status status_;
  const t_display_p4::device::ScreenDeviceInfo* screen_info_ = nullptr;

  /**
   * @brief 通过 GT9895 触摸 ID 自动识别屏幕类型
   * @return 识别流程完成返回 true，否则返回 false
   * @Date 2026-05-15 00:00:00
   */
  bool DetectScreen();

  /**
   * @brief 按当前屏幕信息创建对应的 MIPI 屏幕驱动
   * @Date 2026-05-15 00:00:00
   */
  void CreateSelectedScreenDrivers();

  /**
   * @brief 初始化当前自动识别到的屏幕
   * @return 初始化成功返回 true，否则返回 false
   * @Date 2026-05-15 00:00:00
   */
  bool InitSelectedScreen();

  /**
   * @brief 初始化当前屏幕对应的触摸和背光设备
   * @return 初始化成功返回 true，否则返回 false
   * @Date 2026-05-15 00:00:00
   */
  bool InitSelectedTouchAndBacklight();

  TDisplayP4Driver() = default;
  ~TDisplayP4Driver() = default;

  // 禁止拷贝构造和赋值
  TDisplayP4Driver(const TDisplayP4Driver&) = delete;
  TDisplayP4Driver& operator=(const TDisplayP4Driver&) = delete;
};

}  // namespace lilygo_device_driver
