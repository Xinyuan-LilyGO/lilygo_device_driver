/*
 * @Description: t_display_p4_air_driver
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-05-14 22:58:58
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

namespace lilygo_device_driver::t_display_p4::device {

#if defined(CONFIG_BOARD_TYPE_T_DISPLAY_P4)
inline constexpr int kScreenRotationDirection = 0;
#elif defined(CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD)
inline constexpr int kScreenRotationDirection = 90;
#else
#error "Missing required macro definition."
#endif

#if defined(CONFIG_SCREEN_PIXEL_FORMAT_RGB565)
inline constexpr int kScreenBitsPerPixel = 16;
#elif defined(CONFIG_SCREEN_PIXEL_FORMAT_RGB888)
inline constexpr int kScreenBitsPerPixel = 24;
#else
#error "Missing required macro definition."
#endif

#if defined(CONFIG_CAMERA_PIXEL_FORMAT_RGB565)
inline constexpr int kCameraBitsPerPixel = 16;
#elif defined(CONFIG_CAMERA_PIXEL_FORMAT_RGB888)
inline constexpr int kCameraBitsPerPixel = 24;
#else
#error "Missing required macro definition."
#endif

#if defined(CONFIG_SCREEN_TYPE_HI8561)
inline constexpr int kScreenWidth = kHi8561ScreenWidth;
inline constexpr int kScreenHeight = kHi8561ScreenHeight;
inline constexpr int kScreenMipiDsiDpiClkMhz = kHi8561ScreenMipiDsiDpiClkMhz;
inline constexpr int kScreenMipiDsiHsync = kHi8561ScreenMipiDsiHsync;
inline constexpr int kScreenMipiDsiHbp = kHi8561ScreenMipiDsiHbp;
inline constexpr int kScreenMipiDsiHfp = kHi8561ScreenMipiDsiHfp;
inline constexpr int kScreenMipiDsiVsync = kHi8561ScreenMipiDsiVsync;
inline constexpr int kScreenMipiDsiVbp = kHi8561ScreenMipiDsiVbp;
inline constexpr int kScreenMipiDsiVfp = kHi8561ScreenMipiDsiVfp;
inline constexpr int kScreenDataLaneNum = kHi8561ScreenDataLaneNum;
inline constexpr int kScreenLaneBitRateMbps = kHi8561ScreenLaneBitRateMbps;

#elif defined(CONFIG_SCREEN_TYPE_RM69A10)
inline constexpr int kScreenWidth = kRm69a10ScreenWidth;
inline constexpr int kScreenHeight = kRm69a10ScreenHeight;
inline constexpr int kScreenMipiDsiDpiClkMhz = kRm69a10ScreenMipiDsiDpiClkMhz;
inline constexpr int kScreenMipiDsiHsync = kRm69a10ScreenMipiDsiHsync;
inline constexpr int kScreenMipiDsiHbp = kRm69a10ScreenMipiDsiHbp;
inline constexpr int kScreenMipiDsiHfp = kRm69a10ScreenMipiDsiHfp;
inline constexpr int kScreenMipiDsiVsync = kRm69a10ScreenMipiDsiVsync;
inline constexpr int kScreenMipiDsiVbp = kRm69a10ScreenMipiDsiVbp;
inline constexpr int kScreenMipiDsiVfp = kRm69a10ScreenMipiDsiVfp;
inline constexpr int kScreenDataLaneNum = kRm69a10ScreenDataLaneNum;
inline constexpr int kScreenLaneBitRateMbps = kRm69a10ScreenLaneBitRateMbps;

#else
#error "Missing required macro definition."
#endif

}  // namespace lilygo_device_driver::t_display_p4::device

#if defined(CONFIG_CAMERA_TYPE_SC2336)
#elif defined(CONFIG_CAMERA_TYPE_OV2710)
#elif defined(CONFIG_CAMERA_TYPE_OV5645)
#else
#error "Missing required macro definition."
#endif

namespace lilygo_device_driver {

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

#if defined(CONFIG_SCREEN_TYPE_HI8561)
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> hi8561_i2c_touch_bus;
#elif defined(CONFIG_SCREEN_TYPE_RM69A10)
    std::shared_ptr<cpp_bus_driver::HardwareI2c1> gt9895_i2c_touch_bus;
#endif

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

#if defined(CONFIG_SCREEN_TYPE_HI8561)
    std::unique_ptr<cpp_bus_driver::Hi8561> hi8561;
    std::unique_ptr<cpp_bus_driver::Hi8561Touch> hi8561_touch;
    std::unique_ptr<cpp_bus_driver::Pwm> hi8561_backlight;
#elif defined(CONFIG_SCREEN_TYPE_RM69A10)
    std::unique_ptr<cpp_bus_driver::Rm69a10> rm69a10;
    std::unique_ptr<cpp_bus_driver::Gt9895> gt9895;
#endif

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

#if defined(CONFIG_SCREEN_TYPE_HI8561)
    struct {
      bool init_flag = false;
    } hi8561;

    struct {
      bool init_flag = false;
    } hi8561_touch;

    struct {
      bool init_flag = false;
    } hi8561_backlight;
#elif defined(CONFIG_SCREEN_TYPE_RM69A10)
    struct {
      bool init_flag = false;
    } rm69a10;

    struct {
      bool init_flag = false;
    } gt9895;
#endif

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

#if defined(CONFIG_SCREEN_TYPE_HI8561)
  bool InitHi8561();
  bool InitHi8561Touch();
  bool InitHi8561Backlight();
#elif defined(CONFIG_SCREEN_TYPE_RM69A10)
  bool InitRm69a10();
  bool InitGt9895();
#endif

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

  TDisplayP4Driver() = default;
  ~TDisplayP4Driver() = default;

  // 禁止拷贝构造和赋值
  TDisplayP4Driver(const TDisplayP4Driver&) = delete;
  TDisplayP4Driver& operator=(const TDisplayP4Driver&) = delete;
};

}  // namespace lilygo_device_driver
