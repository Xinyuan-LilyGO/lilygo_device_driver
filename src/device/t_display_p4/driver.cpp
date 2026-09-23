/*
 * @Description: T-Display-P4 共用设备流程
 * @License: GPL 3.0
 */
#include "device/t_display_p4/driver.h"

#include <cstdio>

#include "core/logger.h"
#include "chip/esp32p4/spiffs.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "driver/spi_master.h"
#include "esp_vfs_fat.h"

namespace lilygo_device_driver {
namespace gpio = t_display_p4::gpio;
namespace device = t_display_p4::device;
namespace keyboard_gpio = t_display_p4::keyboard_expansion::gpio;
namespace keyboard_device = t_display_p4::keyboard_expansion::device;
namespace {

using ScreenInfo = device::ScreenInfo;
using ScreenType = device::ScreenType;

constexpr ScreenInfo kHi8561ScreenInfo = {
    .type = ScreenType::kHi8561,
    .name = "hi8561",
    .width = device::hi8561::kScreenWidth,
    .height = device::hi8561::kScreenHeight,
    .bits_per_pixel = device::screen::kBitsPerPixel,
    .pixel_format = GetRgbPixelFormatName(device::screen::kBitsPerPixel),
    .mipi_dsi_dpi_clk_mhz = device::hi8561::kScreenMipiDsiDpiClkMhz,
    .mipi_dsi_hsync = device::hi8561::kScreenMipiDsiHsync,
    .mipi_dsi_hbp = device::hi8561::kScreenMipiDsiHbp,
    .mipi_dsi_hfp = device::hi8561::kScreenMipiDsiHfp,
    .mipi_dsi_vsync = device::hi8561::kScreenMipiDsiVsync,
    .mipi_dsi_vbp = device::hi8561::kScreenMipiDsiVbp,
    .mipi_dsi_vfp = device::hi8561::kScreenMipiDsiVfp,
    .data_lane_num = device::hi8561::kScreenDataLaneNum,
    .lane_bit_rate_mbps = device::hi8561::kScreenLaneBitRateMbps,
};

constexpr ScreenInfo kRm69a10ScreenInfo = {
    .type = ScreenType::kRm69a10,
    .name = "rm69a10",
    .width = device::rm69a10::kScreenWidth,
    .height = device::rm69a10::kScreenHeight,
    .bits_per_pixel = device::screen::kBitsPerPixel,
    .pixel_format = GetRgbPixelFormatName(device::screen::kBitsPerPixel),
    .mipi_dsi_dpi_clk_mhz = device::rm69a10::kScreenMipiDsiDpiClkMhz,
    .mipi_dsi_hsync = device::rm69a10::kScreenMipiDsiHsync,
    .mipi_dsi_hbp = device::rm69a10::kScreenMipiDsiHbp,
    .mipi_dsi_hfp = device::rm69a10::kScreenMipiDsiHfp,
    .mipi_dsi_vsync = device::rm69a10::kScreenMipiDsiVsync,
    .mipi_dsi_vbp = device::rm69a10::kScreenMipiDsiVbp,
    .mipi_dsi_vfp = device::rm69a10::kScreenMipiDsiVfp,
    .data_lane_num = device::rm69a10::kScreenDataLaneNum,
    .lane_bit_rate_mbps = device::rm69a10::kScreenLaneBitRateMbps,
};

constexpr ScreenInfo kScreenInfoRegistry[] = {
    kHi8561ScreenInfo,
    kRm69a10ScreenInfo,
};

constexpr const ScreenInfo* kDefaultScreenInfo = &kHi8561ScreenInfo;

/**
 * @brief 将屏幕像素位宽转换为 MIPI 颜色格式。
 * @param bits_per_pixel 单个像素的位数。
 * @return 匹配的 MIPI 颜色格式，不支持时返回 RGB565。
 */
cpp_bus_driver::HardwareMipi::ColorFormat ColorFormatFromBitsPerPixel(
    int bits_per_pixel) {
  switch (bits_per_pixel) {
    case 16:
      return cpp_bus_driver::HardwareMipi::ColorFormat::kRgb565;
    case 24:
      return cpp_bus_driver::HardwareMipi::ColorFormat::kRgb888;
    default:
      return cpp_bus_driver::HardwareMipi::ColorFormat::kRgb565;
  }
}

/**
 * @brief 按屏幕类型查找屏幕设备信息。
 * @param type 要查找的屏幕类型。
 * @return 找到时返回屏幕设备信息，否则返回 nullptr。
 */
const ScreenInfo* FindScreenInfo(ScreenType type) {
  for (const ScreenInfo& info : kScreenInfoRegistry) {
    if (info.type == type) {
      return &info;
    }
  }
  return nullptr;
}

/**
 * @brief 按屏幕类型查找设备信息，未找到时返回默认屏幕信息。
 * @param type 要查找的屏幕类型。
 * @return 找到时返回屏幕设备信息，否则返回默认屏幕信息。
 */
const ScreenInfo* ScreenInfoForType(ScreenType type) {
  const auto* info = FindScreenInfo(type);
  return info == nullptr ? kDefaultScreenInfo : info;
}

}  // namespace

TDisplayP4Driver& TDisplayP4Driver::GetInstance() {
  static TDisplayP4Driver* instance = new TDisplayP4Driver();
  return *instance;
}

const device::ScreenInfo& TDisplayP4Driver::screen_info() const {
  return *(screen_info_ == nullptr ? kDefaultScreenInfo : screen_info_);
}

void TDisplayP4Driver::CreateKeyboardExpansionDrivers() {
  if (chip_.xl9555 != nullptr) {
    return;
  }

  bus_.xl9555_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c>(
      keyboard_gpio::xl9555::kSda, keyboard_gpio::xl9555::kScl,
      keyboard_device::i2c::kPort);
  bus_.tca8418_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.xl9555_i2c_bus);
  bus_.cc1101_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(bus_.radio_spi_bus, 0);
  bus_.nrf24l01_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(bus_.radio_spi_bus, 0);
  bus_.st25r3916_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(bus_.radio_spi_bus, 1);

  chip_.xl9555 = std::make_unique<cpp_bus_driver::Xl95x5>(
      bus_.xl9555_i2c_bus, keyboard_device::xl9555::kI2cAddress);
  chip_.tca8418 = std::make_unique<cpp_bus_driver::Tca8418>(
      bus_.tca8418_i2c_bus, keyboard_device::tca8418::kI2cAddress);
  chip_.keyboard_sy7200a =
      std::make_unique<cpp_bus_driver::Pwm>(keyboard_gpio::sy7200a::kEn);
  chip_.cc1101 = std::make_unique<cpp_bus_driver::Cc1101>(bus_.cc1101_spi_bus,
      keyboard_gpio::t_mix_rf::cc1101::kCs,
      keyboard_gpio::t_mix_rf::cc1101::kMiso,
      keyboard_gpio::t_mix_rf::cc1101::kGdo0,
      keyboard_gpio::t_mix_rf::cc1101::kGdo2);
  chip_.nrf24l01 = std::make_unique<cpp_bus_driver::Nrf24l01x>(
      bus_.nrf24l01_spi_bus, keyboard_gpio::t_mix_rf::nrf24l01::kCs,
      keyboard_gpio::t_mix_rf::nrf24l01::kCe,
      keyboard_gpio::t_mix_rf::nrf24l01::kInt);
  chip_.st25r3916 =
      std::make_unique<stsw_st25rfal002_cpp_bus_driver::St25r3916x>(
          bus_.st25r3916_spi_bus, keyboard_gpio::t_mix_rf::st25r3916::kInt,
          keyboard_gpio::t_mix_rf::st25r3916::kCs);
}

void TDisplayP4Driver::DestroyKeyboardExpansionDrivers() {
  chip_.st25r3916.reset();
  chip_.nrf24l01.reset();
  chip_.cc1101.reset();
  chip_.keyboard_sy7200a.reset();
  chip_.tca8418.reset();
  chip_.xl9555.reset();
  bus_.st25r3916_spi_bus.reset();
  bus_.nrf24l01_spi_bus.reset();
  bus_.cc1101_spi_bus.reset();
  bus_.tca8418_i2c_bus.reset();
  bus_.xl9555_i2c_bus.reset();
}

bool TDisplayP4Driver::InitSgm38121() {
  if (chip_.sgm38121 == nullptr || !chip_.sgm38121->Init()) {
    chip_status_.sgm38121.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSgm38121 failed\n");
    return false;
  }

  bool result = true;
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2,
      cpp_bus_driver::Sgm38121::Status::kOff);

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_SC2336)
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV2710)
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 3000);
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV5645)
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
#endif

  chip_status_.sgm38121.init_flag = result;
  if (!result) {
    chip_.sgm38121->Deinit(false);
  }
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSgm38121 success\n");
  } else {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSgm38121 failed\n");
  }
  return result;
}

bool TDisplayP4Driver::InitHi8561() {
  if (chip_.hi8561 == nullptr) {
    chip_status_.hi8561.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitHi8561 failed\n");
    return false;
  }

  const auto& screen = screen_info();
  bool result = chip_.hi8561->Init(
      screen.mipi_dsi_dpi_clk_mhz, screen.lane_bit_rate_mbps);
  if (result) {
    result &= chip_.hi8561->SetScreenOff(true);
    result &= chip_.hi8561->SetSleep(true);
  }
  if (!result) {
    chip_.hi8561->Deinit();
  }
  chip_status_.hi8561.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitHi8561 success\n" : "InitHi8561 failed\n");
  return result;
}

bool TDisplayP4Driver::InitHi8561Touch() {
  chip_status_.hi8561_touch.init_flag = false;
  if (!chip_status_.xl9535.init_flag) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }
  bool reset_pin_initialized = true;
  reset_pin_initialized &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, device::xl9535::kResetAsserted);
  reset_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_pin_initialized) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }
  platform_hal_->DelayMs(10);
  if (!chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, device::xl9535::kResetReleased)) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }
  platform_hal_->DelayMs(100);

  if (chip_.hi8561_touch == nullptr) {
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kTouchRst, device::xl9535::kResetAsserted);
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }

  if (!chip_.hi8561_touch->Init(device::hi8561::kI2cFrequencyHz)) {
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kTouchRst, device::xl9535::kResetAsserted);
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }

  chip_status_.hi8561_touch.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561Touch success\n");
  return true;
}

bool TDisplayP4Driver::InitRm69a10() {
  if (chip_.rm69a10 == nullptr) {
    chip_status_.rm69a10.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitRm69a10 failed\n");
    return false;
  }

  const auto& screen = screen_info();
  bool result = chip_.rm69a10->Init(
      screen.mipi_dsi_dpi_clk_mhz, screen.lane_bit_rate_mbps);
  if (result) {
    result &= chip_.rm69a10->SetBrightness(0);
    result &= chip_.rm69a10->SetScreenOff(true);
    result &= chip_.rm69a10->SetSleep(true);
  }
  if (!result) {
    chip_.rm69a10->Deinit();
  }
  chip_status_.rm69a10.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitRm69a10 success\n" : "InitRm69a10 failed\n");
  return result;
}

bool TDisplayP4Driver::InitGt9895() {
  chip_status_.gt9895.init_flag = false;
  if (!chip_status_.xl9535.init_flag) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitGt9895 failed\n");
    return false;
  }
  bool reset_pin_initialized = true;
  reset_pin_initialized &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, device::xl9535::kResetAsserted);
  reset_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_pin_initialized) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitGt9895 failed\n");
    return false;
  }
  platform_hal_->DelayMs(30);
  if (!chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, device::xl9535::kResetReleased)) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitGt9895 failed\n");
    return false;
  }
  platform_hal_->DelayMs(100);

  if (chip_.gt9895 == nullptr) {
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kTouchRst, device::xl9535::kResetAsserted);
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitGt9895 failed\n");
    return false;
  }

  if (!chip_.gt9895->Init(device::gt9895::kI2cFrequencyHz)) {
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kTouchRst, device::xl9535::kResetAsserted);
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitGt9895 failed\n");
    return false;
  }

  chip_status_.gt9895.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitGt9895 success\n");
  return true;
}

bool TDisplayP4Driver::InitAw86224() {
  if (IsAw86224Ready()) {
    return true;
  }
  if (!chip_.aw86224->Init(device::aw86224::kI2cFrequencyHz)) {
    chip_status_.aw86224.init_flag = false;
    chip_status_.aw86224.ram_waveform_info =
        cpp_bus_driver::Aw862xx::RamWaveformInfo();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitAw86224 failed\n");
    return false;
  }

  const uint32_t detected_f0 = chip_.aw86224->GetF0Detection();
  if (detected_f0 == 0 || detected_f0 == static_cast<uint32_t>(-1)) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "Aw86224 F0 reference read failed\n");
  } else {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "Aw86224 F0 reference: %u.%uHz\n",
        static_cast<unsigned int>(detected_f0 / 10),
        static_cast<unsigned int>(detected_f0 % 10));
  }

  bool result = chip_.aw86224->InitRamMode(
      cpp_bus_driver::Aw862xx::RamWaveformLibrary::kRam12k041230_235);
  if (result) {
    result = chip_.aw86224->StopRamPlaybackWaveform();
  }
  if (!result) {
    chip_.aw86224->Deinit(false);
  }
  chip_status_.aw86224.init_flag = result;
  chip_status_.aw86224.ram_waveform_info =
      cpp_bus_driver::Aw862xx::GetRamWaveformInfo(
          cpp_bus_driver::Aw862xx::RamWaveformLibrary::kRam12k041230_235);
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "InitAw86224 success (RAM library: %s)\n",
        chip_status_.aw86224.ram_waveform_info.name);
  } else {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitAw86224 failed\n");
  }
  return result;
}

bool TDisplayP4Driver::InitXl9555() {
  chip_status_.xl9555.init_flag = false;
  if (chip_.xl9555 == nullptr || !chip_.xl9555->Init()) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitXl9555 failed\n");
    return false;
  }

  chip_status_.xl9555.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitXl9555 success\n");
  return true;
}

bool TDisplayP4Driver::InitTca8418() {
  if (!chip_status_.xl9555.init_flag) {
    chip_status_.tca8418.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitTca8418 failed\n");
    return false;
  }
  bool reset_pin_initialized = true;
  reset_pin_initialized &=
      chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTca8418Rst, 0);
  reset_pin_initialized &=
      chip_.xl9555->SetGpioMode(keyboard_gpio::xl9555::kTca8418Rst,
          cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_pin_initialized) {
    chip_status_.tca8418.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitTca8418 failed\n");
    return false;
  }
  platform_hal_->DelayMs(10);
  if (!chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTca8418Rst, 1)) {
    chip_status_.tca8418.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitTca8418 failed\n");
    return false;
  }
  platform_hal_->DelayMs(10);

  if (!chip_.tca8418->Init()) {
    chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTca8418Rst, 0);
    chip_status_.tca8418.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitTca8418 failed\n");
    return false;
  } else {
    bool result = true;
    result &= chip_.tca8418->SetKeypadScanWindow(0, 0,
        keyboard_device::tca8418::kKeypadScanWidth,
        keyboard_device::tca8418::kKeypadScanHeight);
    result &= chip_.tca8418->SetInterruptEnable(
        cpp_bus_driver::Tca8418::IrqMask::kKeyEvents);
    result &= chip_.tca8418->ClearIrqFlag(
        cpp_bus_driver::Tca8418::IrqFlag::kKeyEvents);

    chip_status_.tca8418.init_flag = result;
    if (result) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitTca8418 success\n");
    } else {
      chip_.tca8418->Deinit();
      chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTca8418Rst, 0);
      LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitTca8418 failed\n");
    }
    return result;
  }
}

bool TDisplayP4Driver::InitCc1101() {
  if (!chip_status_.xl9555.init_flag || chip_.cc1101 == nullptr) {
    chip_status_.cc1101.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitCc1101 failed\n");
    return false;
  }

  bool rf_switch_initialized = true;
  rf_switch_initialized &=
      chip_.xl9555->SetGpioMode(keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch0,
          cpp_bus_driver::Xl95x5::Mode::kOutput);
  rf_switch_initialized &=
      chip_.xl9555->SetGpioMode(keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch1,
          cpp_bus_driver::Xl95x5::Mode::kOutput);
  // 初始化默认选择 868/915 MHz 通路
  rf_switch_initialized &= SetCc1101RfSwitch(Cc1101RfSwitch::k868_915Mhz);
  if (!rf_switch_initialized ||
      !chip_.cc1101->Init(keyboard_device::cc1101::kSpiFrequencyHz)) {
    chip_status_.cc1101.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitCc1101 failed\n");
    return false;
  }

  if (!chip_.cc1101->Sleep()) {
    chip_.cc1101->Deinit(false);
    chip_status_.cc1101.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitCc1101 failed\n");
    return false;
  }

  chip_status_.cc1101.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitCc1101 success\n");
  return true;
}

bool TDisplayP4Driver::InitNrf24l01() {
  if (chip_.nrf24l01 == nullptr || !chip_.nrf24l01->Init()) {
    chip_status_.nrf24l01.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitNrf24l01 failed\n");
    return false;
  }

  if (!chip_.nrf24l01->PowerDown()) {
    chip_.nrf24l01->Deinit(false);
    chip_status_.nrf24l01.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitNrf24l01 failed\n");
    return false;
  }

  chip_status_.nrf24l01.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitNrf24l01 success\n");
  return true;
}

bool TDisplayP4Driver::InitSt25r3916() {
  if (chip_.st25r3916 == nullptr || bus_.st25r3916_spi_bus == nullptr) {
    chip_status_.st25r3916.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSt25r3916 failed\n");
    return false;
  }

  const ReturnCode result = chip_.st25r3916->Init();
  const auto platform_error = chip_.st25r3916->platform_error();
  chip_status_.st25r3916.init_flag =
      result == RFAL_ERR_NONE &&
      platform_error == stsw_st25rfal002_cpp_bus_driver::PlatformError::kNone &&
      chip_.st25r3916->initialized();
  if (!chip_status_.st25r3916.init_flag) {
    chip_.st25r3916->Deinit(false);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitSt25r3916 failed (RFAL: %u, platform: %u)\n",
        static_cast<unsigned int>(result),
        static_cast<unsigned int>(platform_error));
    return false;
  }

  if (!SetSt25r3916OperatingMode(St25r3916OperatingMode::kSleep)) {
    chip_.st25r3916->Deinit(false);
    chip_status_.st25r3916.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSt25r3916 failed\n");
    return false;
  }

  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSt25r3916 success\n");
  return true;
}

bool TDisplayP4Driver::InitKeyboardBacklight() {
  if (chip_.keyboard_sy7200a != nullptr &&
      chip_.keyboard_sy7200a->IsInitialized()) {
    chip_status_.keyboard_sy7200a.init_flag = true;
    return true;
  }
  if (chip_.keyboard_sy7200a == nullptr) {
    chip_status_.keyboard_sy7200a.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitKeyboardBacklight failed\n");
    return false;
  }

  cpp_bus_driver::Pwm::Config config;
  config.timer = LEDC_TIMER_1;
  config.channel = LEDC_CHANNEL_1;
  config.frequency_hz = keyboard_device::sy7200a::kPwmFrequencyHz;
  config.resolution = LEDC_TIMER_5_BIT;
  config.initial_duty = {.value = 0, .scale = 1};
  config.idle_level_on_deinit = cpp_bus_driver::Pwm::IdleLevel::kLow;
  if (!chip_.keyboard_sy7200a->Init(config) ||
      !chip_.keyboard_sy7200a->DisableOutput(
          cpp_bus_driver::Pwm::IdleLevel::kLow)) {
    chip_.keyboard_sy7200a->Deinit();
    chip_status_.keyboard_sy7200a.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitKeyboardBacklight failed\n");
    return false;
  }

  chip_status_.keyboard_sy7200a.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "InitKeyboardBacklight success\n");
  return true;
}

bool TDisplayP4Driver::InitKeyboardExpansion() {
  const bool base_ready = minimal_drivers_initialized_ &&
                          platform_hal_ != nullptr &&
                          bus_.radio_spi_bus != nullptr;
  if (!base_ready) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "Initialize the base device driver before the keyboard expansion\n");
    return false;
  }
  if (!DeinitKeyboardExpansion()) {
    return false;
  }
  CreateKeyboardExpansionDrivers();
  chip_status_.xl9555.init_flag = false;
  chip_status_.tca8418.init_flag = false;
  chip_status_.keyboard_sy7200a.init_flag = false;
  chip_status_.cc1101.init_flag = false;
  chip_status_.nrf24l01.init_flag = false;
  chip_status_.st25r3916.init_flag = false;

  if (!InitXl9555()) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "Keyboard expansion not connected\n");
    return false;
  }

  bool expander_outputs_initialized = true;
  expander_outputs_initialized &=
      chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed1, 1);
  expander_outputs_initialized &=
      chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed2, 1);
  expander_outputs_initialized &=
      chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed3, 1);
  expander_outputs_initialized &=
      chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfEn, 0);
  expander_outputs_initialized &= chip_.xl9555->SetGpioMode(
      keyboard_gpio::xl9555::kLed1, cpp_bus_driver::Xl95x5::Mode::kOutput);
  expander_outputs_initialized &= chip_.xl9555->SetGpioMode(
      keyboard_gpio::xl9555::kLed2, cpp_bus_driver::Xl95x5::Mode::kOutput);
  expander_outputs_initialized &= chip_.xl9555->SetGpioMode(
      keyboard_gpio::xl9555::kLed3, cpp_bus_driver::Xl95x5::Mode::kOutput);
  expander_outputs_initialized &= chip_.xl9555->SetGpioMode(
      keyboard_gpio::xl9555::kTMixRfEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  expander_outputs_initialized &=
      chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfEn, 1);
  if (!expander_outputs_initialized) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "Keyboard expansion GPIO initialization failed\n");
    return false;
  }

  // 扩展板通过外部电阻上拉 TCA8418 INT。启用主板内部下拉后，
  // 扩展板断开时 INT 会自动变为低电平，供应用层确认连接状态。
  if (!platform_hal_->SetGpioMode(keyboard_gpio::tca8418::kInt,
          cpp_bus_driver::PlatformHal::GpioMode::kInput,
          cpp_bus_driver::PlatformHal::GpioStatus::kPulldown)) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "Keyboard expansion GPIO initialization failed\n");
    return false;
  }

  bool result = true;
  result &= InitTca8418();
  result &= InitKeyboardBacklight();
  result &= InitCc1101();
  result &= InitNrf24l01();
  result &= InitSt25r3916();
  result &=
      SetKeyboardExpansionOperatingMode(KeyboardExpansionOperatingMode::kSleep);
  return result;
}

bool TDisplayP4Driver::InitScreen() {
  if (!chip_status_.xl9535.init_flag) {
    return false;
  }
  bool reset_pin_initialized = true;
  reset_pin_initialized &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kScreenRst, device::xl9535::kResetAsserted);
  reset_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kScreenRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_pin_initialized) {
    return false;
  }
  platform_hal_->DelayMs(10);
  if (!chip_.xl9535->GpioWrite(
      gpio::xl9535::kScreenRst, device::xl9535::kResetReleased)) {
    return false;
  }
  platform_hal_->DelayMs(120);

  if (!DetectScreenType()) {
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kScreenRst, device::xl9535::kResetAsserted);
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "DetectScreenType failed\n");
    return false;
  }

  const auto& screen = screen_info();
  bus_.screen_mipi_bus = std::make_shared<cpp_bus_driver::HardwareMipi>(
      screen.width, screen.height, screen.mipi_dsi_hsync, screen.mipi_dsi_hbp,
      screen.mipi_dsi_hfp, screen.mipi_dsi_vsync, screen.mipi_dsi_vbp,
      screen.mipi_dsi_vfp, screen.data_lane_num,
      ColorFormatFromBitsPerPixel(screen.bits_per_pixel));

  chip_.hi8561.reset();
  chip_.rm69a10.reset();
  chip_status_.hi8561.init_flag = false;
  chip_status_.hi8561_touch.init_flag = false;
  ResetScreenBacklightStatus();
  chip_status_.rm69a10.init_flag = false;

  bool result = false;
  switch (screen.type) {
    case device::ScreenType::kHi8561:
      chip_.hi8561 =
          std::make_unique<cpp_bus_driver::Hi8561>(bus_.screen_mipi_bus);
      result = InitHi8561();
      break;
    case device::ScreenType::kRm69a10:
      chip_.rm69a10 =
          std::make_unique<cpp_bus_driver::Rm69a10>(bus_.screen_mipi_bus);
      result = InitRm69a10();
      break;
    default:
      break;
  }
  if (!result) {
    DeinitTouch();
    DeinitScreen();
  }
  return result;
}

bool TDisplayP4Driver::InitTouch() {
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      return InitHi8561Touch();
    case device::ScreenType::kRm69a10:
      return chip_status_.gt9895.init_flag || InitGt9895();
    default:
      return false;
  }
}

bool TDisplayP4Driver::InitSpiffs(
    const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf) {
  SpiffsConfig config;
  config.base_path = base_path;
  if (!lilygo_device_driver::InitSpiffs(config)) {
    return false;
  }
  spiffs_conf = {
      .base_path = base_path,
      .partition_label = config.partition_label,
      .max_files = config.max_files,
      .format_if_mount_failed = config.format_if_mount_failed,
  };
  return true;
}

bool TDisplayP4Driver::InitSdmmc(const char* base_path, int max_freq_khz) {
  if (base_path == nullptr || base_path[0] == '\0' || max_freq_khz <= 0) {
    return false;
  }
  if (sd_card_.IsMounted() && !DeinitSdmmc()) {
    return false;
  }
  if (!chip_status_.xl9535.init_flag || chip_.xl9535 == nullptr) {
    return false;
  }
  bool power_enabled = true;
  power_enabled &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSdPowerEn, device::xl9535::kSdPowerDisabled);
  power_enabled &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kSdPowerEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  power_enabled &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSdPowerEn, device::xl9535::kSdPowerEnabled);
  if (!power_enabled) {
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kSdPowerEn, device::xl9535::kSdPowerDisabled);
    return false;
  }

  SdCard::SdmmcConfig config;
  config.host.slot = SDMMC_HOST_SLOT_0;
  config.host.max_freq_khz = max_freq_khz;
  config.slot.width = 4;
  config.slot.clk = static_cast<gpio_num_t>(gpio::sd::kSdioClk);
  config.slot.cmd = static_cast<gpio_num_t>(gpio::sd::kSdioCmd);
  config.slot.d0 = static_cast<gpio_num_t>(gpio::sd::kSdioD0);
  config.slot.d1 = static_cast<gpio_num_t>(gpio::sd::kSdioD1);
  config.slot.d2 = static_cast<gpio_num_t>(gpio::sd::kSdioD2);
  config.slot.d3 = static_cast<gpio_num_t>(gpio::sd::kSdioD3);
  config.slot.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
  config.mount.disk_status_check_enable = device::sd::kDiskStatusCheckEnabled;

  const bool result = sd_card_.InitSdmmc(base_path, config);
  chip_status_.sd_card.init_flag = sd_card_.IsMounted();
  if (!result) {
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kSdPowerEn, device::xl9535::kSdPowerDisabled);
  }
  return result;
}

bool TDisplayP4Driver::InitSdspi(
    const char* base_path, spi_host_device_t host_id, int max_freq_khz) {
  if (base_path == nullptr || base_path[0] == '\0' || max_freq_khz <= 0) {
    return false;
  }
  if (sd_card_.IsMounted() && !DeinitSdmmc()) {
    return false;
  }
  if (!chip_status_.xl9535.init_flag || chip_.xl9535 == nullptr) {
    return false;
  }
  bool power_enabled = true;
  power_enabled &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSdPowerEn, device::xl9535::kSdPowerDisabled);
  power_enabled &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kSdPowerEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  power_enabled &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSdPowerEn, device::xl9535::kSdPowerEnabled);
  if (!power_enabled) {
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kSdPowerEn, device::xl9535::kSdPowerDisabled);
    return false;
  }

  SdCard::SdspiConfig config;
  config.host.slot = host_id;
  config.host.max_freq_khz = max_freq_khz;
  config.slot.host_id = host_id;
  config.slot.gpio_cs = static_cast<gpio_num_t>(gpio::sd::kCs);
  config.bus.mosi_io_num = gpio::sd::kMosi;
  config.bus.miso_io_num = gpio::sd::kMiso;
  config.bus.sclk_io_num = gpio::sd::kSclk;

  const bool result = sd_card_.InitSdspi(base_path, config);
  chip_status_.sd_card.init_flag = sd_card_.IsMounted();
  if (!result) {
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kSdPowerEn, device::xl9535::kSdPowerDisabled);
  }
  return result;
}

bool TDisplayP4Driver::DeinitScreen() {
  bool result = true;

  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      if (chip_status_.hi8561.init_flag) {
        result &= chip_.hi8561->Deinit();
        chip_status_.hi8561.init_flag = false;
      }
      break;
    case device::ScreenType::kRm69a10:
      if (chip_status_.rm69a10.init_flag) {
        result &= chip_.rm69a10->Deinit();
        chip_status_.rm69a10.init_flag = false;
      }
      break;
    default:
      break;
  }

  if (bus_.screen_mipi_bus != nullptr) {
    result &= bus_.screen_mipi_bus->Deinit();
    bus_.screen_mipi_bus.reset();
  }
  if (chip_status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kScreenRst, device::xl9535::kResetAsserted);
  }

  return result;
}

bool TDisplayP4Driver::DeinitTouch() {
  bool result = true;

  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      if (chip_status_.hi8561_touch.init_flag) {
        result &= chip_.hi8561_touch->Deinit(false);
        chip_status_.hi8561_touch.init_flag = false;
      }
      break;
    case device::ScreenType::kRm69a10:
      if (chip_status_.gt9895.init_flag) {
        result &= chip_.gt9895->Deinit(false);
        chip_status_.gt9895.init_flag = false;
      }
      break;
    default:
      break;
  }

  if (chip_status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kTouchRst, device::xl9535::kResetAsserted);
  }

  return result;
}

bool TDisplayP4Driver::DeinitAw86224() {
  bool result = true;
  if (chip_status_.aw86224.init_flag && chip_.aw86224 != nullptr) {
    result &= chip_.aw86224->StopRamPlaybackWaveform();
    result &= chip_.aw86224->Deinit(false);
  }
  chip_status_.aw86224.init_flag = false;
  chip_status_.aw86224.ram_waveform_info = {};
  return result;
}

bool TDisplayP4Driver::DeinitSt25r3916() {
  if (chip_.st25r3916 == nullptr) {
    chip_status_.st25r3916.init_flag = false;
    return true;
  }

  const bool was_ready = chip_status_.st25r3916.init_flag;
  const ReturnCode deinit_result = chip_.st25r3916->Deinit(false);
  const auto platform_error = chip_.st25r3916->platform_error();
  bool result = deinit_result == RFAL_ERR_NONE;
  if (was_ready) {
    result &=
        platform_error == stsw_st25rfal002_cpp_bus_driver::PlatformError::kNone;
  }
  chip_status_.st25r3916.init_flag = false;
  return result;
}

bool TDisplayP4Driver::DeinitKeyboardExpansion(
    KeyboardExpansionDeinitMode mode) {
  if (chip_.xl9555 == nullptr) {
    return true;
  }
  bool result = true;

  if (mode == KeyboardExpansionDeinitMode::kNormal) {
    result &= DeinitSt25r3916();

    if (chip_.nrf24l01 != nullptr) {
      result &= chip_.nrf24l01->Deinit(false);
    }
    if (chip_.cc1101 != nullptr) {
      result &= chip_.cc1101->Deinit(false);
    }
    if (chip_.keyboard_sy7200a != nullptr &&
        chip_.keyboard_sy7200a->IsInitialized()) {
      result &= chip_.keyboard_sy7200a->DisableOutput(
          cpp_bus_driver::Pwm::IdleLevel::kLow);
      result &= chip_.keyboard_sy7200a->Deinit();
    }
    if (chip_.tca8418 != nullptr) {
      result &= chip_.tca8418->Deinit(false);
    }
    if (chip_.xl9555 != nullptr) {
      if (chip_status_.xl9555.init_flag) {
        result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed1, 1);
        result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed2, 1);
        result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed3, 1);
        result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfEn, 0);
        result &=
            chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTca8418Rst, 0);
      }
      result &= chip_.xl9555->Deinit(false);
    }
  } else {
    // 扩展芯片无法通信时不再发送芯片命令，但仍必须注销主控侧的
    // SPI/I2C device handle，否则反复连接会耗尽 SPI 设备槽。
    if (chip_.st25r3916 != nullptr) {
      result &= chip_.st25r3916->DeinitLocalResources(false) == RFAL_ERR_NONE;
    }
    if (chip_.nrf24l01 != nullptr) {
      result &= chip_.nrf24l01->DeinitLocalResources(false);
    }
    if (chip_.cc1101 != nullptr) {
      result &= chip_.cc1101->DeinitLocalResources(false);
    }
    if (chip_.keyboard_sy7200a != nullptr &&
        chip_.keyboard_sy7200a->IsInitialized()) {
      result &= chip_.keyboard_sy7200a->DisableOutput(
          cpp_bus_driver::Pwm::IdleLevel::kLow);
      result &= chip_.keyboard_sy7200a->Deinit();
    }
    if (chip_.tca8418 != nullptr) {
      result &= chip_.tca8418->Deinit(false);
    }
    if (chip_.xl9555 != nullptr) {
      result &= chip_.xl9555->Deinit(false);
    }
  }

  chip_status_.st25r3916.init_flag = false;
  chip_status_.nrf24l01.init_flag = false;
  chip_status_.cc1101.init_flag = false;
  chip_status_.keyboard_sy7200a.init_flag = false;
  chip_status_.tca8418.init_flag = false;
  chip_status_.xl9555.init_flag = false;

  if (platform_hal_ != nullptr) {
    result &= platform_hal_->ResetGpio(keyboard_gpio::tca8418::kInt);
  }

  // 先注销两个设备，再释放键盘独占的硬件 I2C 总线。
  if (bus_.xl9555_i2c_bus != nullptr && !bus_.xl9555_i2c_bus->Deinit()) {
    return false;
  }

  DestroyKeyboardExpansionDrivers();

  return result;
}

bool TDisplayP4Driver::DeinitL76k() {
  bool result = true;
  if (chip_status_.l76k.init_flag && chip_.l76k != nullptr) {
    result &= chip_.l76k->Sleep(true);
    result &= chip_.l76k->Deinit();
  }
  chip_status_.l76k.init_flag = false;
  return result;
}

bool TDisplayP4Driver::DeinitSdmmc(bool release_bus) {
  bool result = sd_card_.Deinit(release_bus);
  chip_status_.sd_card.init_flag = sd_card_.IsMounted();
  // 卸载失败时保留供电，允许后续重试。
  if (sd_card_.IsMounted()) {
    return false;
  }
  if (chip_status_.xl9535.init_flag && chip_.xl9535 != nullptr) {
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kSdPowerEn, device::xl9535::kSdPowerDisabled);
  }
  return result;
}

bool TDisplayP4Driver::IsXl9535Ready() const {
  return chip_status_.xl9535.init_flag && chip_.xl9535 != nullptr;
}

bool TDisplayP4Driver::IsSgm38121Ready() const {
  return chip_status_.sgm38121.init_flag && chip_.sgm38121 != nullptr;
}

bool TDisplayP4Driver::IsHi8561Ready() const {
  return chip_status_.hi8561.init_flag && chip_.hi8561 != nullptr;
}

bool TDisplayP4Driver::IsHi8561TouchReady() const {
  return chip_status_.hi8561_touch.init_flag && chip_.hi8561_touch != nullptr;
}

bool TDisplayP4Driver::IsRm69a10Ready() const {
  return chip_status_.rm69a10.init_flag && chip_.rm69a10 != nullptr;
}

bool TDisplayP4Driver::IsGt9895Ready() const {
  return chip_status_.gt9895.init_flag && chip_.gt9895 != nullptr;
}

bool TDisplayP4Driver::IsAw86224Ready() const {
  return chip_status_.aw86224.init_flag && chip_.aw86224 != nullptr;
}

bool TDisplayP4Driver::IsL76kReady() const {
  return chip_status_.l76k.init_flag && chip_.l76k != nullptr;
}

bool TDisplayP4Driver::IsXl9555Ready() const {
  return chip_status_.xl9555.init_flag && chip_.xl9555 != nullptr;
}

bool TDisplayP4Driver::IsTca8418Ready() const {
  return chip_status_.tca8418.init_flag && chip_.tca8418 != nullptr;
}

bool TDisplayP4Driver::IsCc1101Ready() const {
  return chip_status_.cc1101.init_flag && chip_.cc1101 != nullptr;
}

bool TDisplayP4Driver::IsNrf24l01Ready() const {
  return chip_status_.nrf24l01.init_flag && chip_.nrf24l01 != nullptr;
}

bool TDisplayP4Driver::IsSt25r3916Ready() const {
  return chip_status_.st25r3916.init_flag && chip_.st25r3916 != nullptr &&
         chip_.st25r3916->initialized();
}

bool TDisplayP4Driver::IsKeyboardBacklightReady() const {
  return chip_status_.keyboard_sy7200a.init_flag &&
         chip_.keyboard_sy7200a != nullptr &&
         chip_.keyboard_sy7200a->IsInitialized();
}

bool TDisplayP4Driver::IsTouchReady() const {
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      return IsHi8561TouchReady();
    case device::ScreenType::kRm69a10:
      return IsGt9895Ready();
    default:
      return false;
  }
}

bool TDisplayP4Driver::IsSdmmcReady() const {
  return chip_status_.sd_card.init_flag && sd_card_.IsReady();
}

bool TDisplayP4Driver::SetAw86224Standby() {
  return !IsAw86224Ready() || chip_.aw86224->StopRamPlaybackWaveform();
}

bool TDisplayP4Driver::SetL76kSleep(bool sleep) {
  if (!IsL76kReady()) {
    if (sleep) {
      return true;
    }
    if (!InitL76k()) {
      return false;
    }
  }
  return chip_.l76k->Sleep(sleep);
}

bool TDisplayP4Driver::SetScreenSleep(bool sleep) {
  if (!IsScreenReady()) {
    return false;
  }

  bool result = true;
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      if (sleep) {
        result &= chip_.hi8561->SetScreenOff(true);
        result &= chip_.hi8561->SetSleep(true);
      } else {
        result &= chip_.hi8561->SetSleep(false);
        result &= chip_.hi8561->SetScreenOff(false);
      }
      break;
    case device::ScreenType::kRm69a10:
      if (sleep) {
        result &= chip_.rm69a10->SetScreenOff(true);
        result &= chip_.rm69a10->SetSleep(true);
      } else {
        result &= chip_.rm69a10->SetSleep(false);
        result &= chip_.rm69a10->SetScreenOff(false);
      }
      break;
    default:
      return false;
  }
  return result;
}

bool TDisplayP4Driver::SetCameraPowerEnabled(bool enabled) {
  if (!IsSgm38121Ready()) {
    return !enabled;
  }

  const auto status = enabled ? cpp_bus_driver::Sgm38121::Status::kOn
                              : cpp_bus_driver::Sgm38121::Status::kOff;
  bool result = true;
#if defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_SC2336)
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, status);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, status);
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV2710) || \
    defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV5645)
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, status);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, status);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, status);
#endif
  return result;
}


bool TDisplayP4Driver::SetCc1101OperatingMode(Cc1101OperatingMode mode) {
  if (!IsCc1101Ready()) {
    return mode == Cc1101OperatingMode::kSleep;
  }
  const bool result = mode == Cc1101OperatingMode::kSleep
                          ? chip_.cc1101->Sleep()
                          : chip_.cc1101->Wakeup();
  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "CC1101 operating mode change failed\n");
    return false;
  }
  return true;
}

bool TDisplayP4Driver::SetNrf24l01OperatingMode(Nrf24l01OperatingMode mode) {
  if (!IsNrf24l01Ready()) {
    return mode == Nrf24l01OperatingMode::kSleep;
  }
  const bool result = mode == Nrf24l01OperatingMode::kSleep
                          ? chip_.nrf24l01->PowerDown()
                          : chip_.nrf24l01->Standby();
  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "NRF24L01 operating mode change failed\n");
    return false;
  }
  return true;
}

bool TDisplayP4Driver::SetSt25r3916OperatingMode(St25r3916OperatingMode mode) {
  if (!IsSt25r3916Ready()) {
    return mode == St25r3916OperatingMode::kSleep;
  }
  const ReturnCode result = mode == St25r3916OperatingMode::kSleep
                                ? chip_.st25r3916->StartLowPowerMode()
                                : chip_.st25r3916->StopLowPowerMode();
  if (result != RFAL_ERR_NONE) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "ST25R3916 operating mode change failed (error code: %u)\n",
        static_cast<unsigned int>(result));
    return false;
  }
  return true;
}

bool TDisplayP4Driver::SetKeyboardExpansionOperatingMode(
    KeyboardExpansionOperatingMode mode) {
  if (!IsXl9555Ready()) {
    return mode == KeyboardExpansionOperatingMode::kSleep;
  }

  if (mode == KeyboardExpansionOperatingMode::kSleep) {
    bool result = true;
    // 低功耗状态下关闭全部指示灯，避免屏幕熄灭后继续耗电和发光。
    result &= SetKeyboardExpansionLed(KeyboardExpansionLed::kLed1, false);
    result &= SetKeyboardExpansionLed(KeyboardExpansionLed::kLed2, false);
    result &= SetKeyboardExpansionLed(KeyboardExpansionLed::kLed3, false);
    if (IsKeyboardBacklightReady()) {
      result &=
          chip_.keyboard_sy7200a->DisableOutput(
              cpp_bus_driver::Pwm::IdleLevel::kLow);
    }
    result &= SetCc1101OperatingMode(Cc1101OperatingMode::kSleep);
    result &= SetNrf24l01OperatingMode(Nrf24l01OperatingMode::kSleep);
    result &= SetSt25r3916OperatingMode(St25r3916OperatingMode::kSleep);
    // TCA8418 没有独立睡眠命令，保持矩阵扫描才能继续响应按键。
    return result;
  }

  bool result = chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfEn, 1);
  result &= SetCc1101OperatingMode(Cc1101OperatingMode::kStandby);
  result &= SetNrf24l01OperatingMode(Nrf24l01OperatingMode::kStandby);
  result &= SetSt25r3916OperatingMode(St25r3916OperatingMode::kActive);
  if (!result) {
    SetKeyboardExpansionOperatingMode(KeyboardExpansionOperatingMode::kSleep);
  }
  return result;
}

bool TDisplayP4Driver::SetCc1101RfSwitch(Cc1101RfSwitch rf_switch) {
  if (!chip_status_.xl9555.init_flag) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "SetCc1101RfSwitch failed\n");
    return false;
  }

  bool result = true;
  switch (rf_switch) {
    case Cc1101RfSwitch::k315Mhz:
      result &= chip_.xl9555->GpioWrite(
          keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch0, 0);
      result &= chip_.xl9555->GpioWrite(
          keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch1, 1);
      break;
    case Cc1101RfSwitch::k434Mhz:
      result &= chip_.xl9555->GpioWrite(
          keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch0, 1);
      result &= chip_.xl9555->GpioWrite(
          keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch1, 1);
      break;
    case Cc1101RfSwitch::k868_915Mhz:
      result &= chip_.xl9555->GpioWrite(
          keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch0, 1);
      result &= chip_.xl9555->GpioWrite(
          keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch1, 0);
      break;

    default:
      result = false;
      break;
  }

  if (!result) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "SetCc1101RfSwitch failed\n");
  }
  return result;
}

bool TDisplayP4Driver::SetKeyboardExpansionLed(
    KeyboardExpansionLed led, bool enabled) {
  if (!IsXl9555Ready()) {
    return false;
  }

  cpp_bus_driver::Xl95x5::Pin pin = keyboard_gpio::xl9555::kLed1;
  switch (led) {
    case KeyboardExpansionLed::kLed1:
      pin = keyboard_gpio::xl9555::kLed1;
      break;
    case KeyboardExpansionLed::kLed2:
      pin = keyboard_gpio::xl9555::kLed2;
      break;
    case KeyboardExpansionLed::kLed3:
      pin = keyboard_gpio::xl9555::kLed3;
      break;
    default:
      return false;
  }

  // 键盘扩展指示灯为低电平点亮。
  return chip_.xl9555->GpioWrite(pin, enabled ? 0 : 1);
}

bool TDisplayP4Driver::DetectScreenType() {
  // HardwareI2c::Init 不会替换已存在的设备句柄，重新探测前必须释放。
  if (!DeinitTouch()) {
    return false;
  }
  screen_info_ = nullptr;

  if (!chip_status_.xl9535.init_flag) {
    return false;
  }
  bool reset_pin_initialized = true;
  reset_pin_initialized &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, device::xl9535::kResetAsserted);
  reset_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_pin_initialized) {
    return false;
  }
  platform_hal_->DelayMs(30);
  if (!chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, device::xl9535::kResetReleased)) {
    return false;
  }
  platform_hal_->DelayMs(100);

  if (chip_.gt9895 != nullptr &&
      chip_.gt9895->Init(device::gt9895::kI2cFrequencyHz)) {
    screen_info_ = ScreenInfoForType(device::ScreenType::kRm69a10);
    chip_status_.gt9895.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "Auto detected T-Display-P4 screen: %s\n", screen_info_->name);
    return true;
  }

  // 探测失败后清理 GT9895 的句柄和缓存，再允许 HI8561 初始化。
  if (chip_.gt9895 != nullptr && !chip_.gt9895->Deinit(false)) {
    return false;
  }
  chip_status_.gt9895.init_flag = false;
  screen_info_ = ScreenInfoForType(device::ScreenType::kHi8561);
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "Auto detected T-Display-P4 screen: %s\n", screen_info_->name);
  return true;
}

}  // namespace lilygo_device_driver
