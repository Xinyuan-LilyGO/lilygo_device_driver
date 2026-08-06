/*
 * @Description: T-Display-P4 板级设备驱动实现
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:51:14
 * @LastEditTime: 2026-08-06 11:52:01
 * @License: GPL 3.0
 */
#include "t_display_p4_driver.h"

#include <array>
#include <cstdint>

namespace lilygo_device_driver {
namespace gpio = t_display_p4::gpio;
namespace device = t_display_p4::device;
namespace keyboard_gpio = t_display_p4::keyboard::gpio;
namespace keyboard_device = t_display_p4::keyboard::device;
namespace {

using ScreenInfo = device::ScreenInfo;
using ScreenType = device::ScreenType;
using RadioType = device::RadioType;

constexpr uint16_t kSx1262VersionStringAddress = 0x0320;
constexpr std::array<uint8_t, 6> kSx1262VersionPrefix = {
    'S', 'X', '1', '2', '6', '1'};
constexpr uint8_t kLr2021ExpectedVersionMajor = 0x01;
constexpr uint8_t kLr2021ExpectedVersionMinor = 0x18;

constexpr ScreenInfo kHi8561ScreenInfo = {
    .type = ScreenType::kHi8561,
    .name = "hi8561",
    .width = device::hi8561::kScreenWidth,
    .height = device::hi8561::kScreenHeight,
    .bits_per_pixel = device::screen::kBitsPerPixel,
    .pixel_format = device::screen::kPixelFormat,
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
    .pixel_format = device::screen::kPixelFormat,
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

device::ScreenType TDisplayP4Driver::screen_type() const {
  return screen_info().type;
}

void TDisplayP4Driver::CreateDrivers() {
  tool_ = std::make_unique<cpp_bus_driver::Tool>();
  radio_type_ = RadioType::kUnknown;
  status_.sx1262.init_flag = false;
  status_.lr2021.init_flag = false;

  bus_.xl9535_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      gpio::i2c::kPort1Sda, gpio::i2c::kPort1Scl, I2C_NUM_0);
  bus_.sgm38121_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      gpio::i2c::kPort2Sda, gpio::i2c::kPort2Scl, I2C_NUM_1);
  bus_.radio_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(gpio::spi::kPort1Mosi,
          gpio::spi::kPort1Sclk, gpio::spi::kPort1Miso, SPI2_HOST, 0);
  bus_.sx1262_spi_bus = bus_.radio_spi_bus;

  bus_.bq27220_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.xl9535_i2c_bus);
  bus_.pcf8563_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.xl9535_i2c_bus);
  bus_.aw86224_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.sgm38121_i2c_bus);
  bus_.es8311_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.sgm38121_i2c_bus);

  bus_.es8311_i2s_bus = std::make_shared<cpp_bus_driver::HardwareI2s>(
      gpio::es8311::kAdcData, gpio::es8311::kDacData, gpio::es8311::kWsLrck,
      gpio::es8311::kBclk, gpio::es8311::kMclk, i2s_port_t::I2S_NUM_0,
      cpp_bus_driver::HardwareI2s::DataMode::kInputOutput,
      cpp_bus_driver::HardwareI2s::I2sMode::kStd,
      i2s_clock_src_t::I2S_CLK_SRC_DEFAULT);

  bus_.l76k_uart_bus = std::make_shared<cpp_bus_driver::HardwareUart>(
      gpio::l76k::kRx, gpio::l76k::kTx, UART_NUM_1);

  bus_.icm20948_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.sgm38121_i2c_bus);

  chip_.bq27220 = std::make_unique<cpp_bus_driver::Bq27220>(
      bus_.bq27220_i2c_bus, device::bq27220::kI2cAddress);

  chip_.xl9535 = std::make_unique<cpp_bus_driver::Xl95x5>(
      bus_.xl9535_i2c_bus, device::xl9535::kI2cAddress);

  chip_.sgm38121 = std::make_unique<cpp_bus_driver::Sgm38121>(
      bus_.sgm38121_i2c_bus, device::sgm38121::kI2cAddress);

  bus_.hi8561_i2c_touch_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.xl9535_i2c_bus);
  chip_.hi8561_touch = std::make_unique<cpp_bus_driver::Hi8561Touch>(
      bus_.hi8561_i2c_touch_bus, device::hi8561::kTouchI2cAddress);
  chip_.hi8561_backlight =
      std::make_unique<cpp_bus_driver::Pwm>(gpio::hi8561::kScreenBacklight);

  bus_.gt9895_i2c_touch_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.xl9535_i2c_bus);
  chip_.gt9895 = std::make_unique<cpp_bus_driver::Gt9895>(
      bus_.gt9895_i2c_touch_bus, device::gt9895::kI2cAddress, -1,
      device::gt9895::kXScaleFactor, device::gt9895::kYScaleFactor);

  chip_.pcf8563 = std::make_unique<cpp_bus_driver::Pcf8563x>(
      bus_.pcf8563_i2c_bus, device::pcf8563::kI2cAddress);

  chip_.aw86224 = std::make_unique<cpp_bus_driver::Aw862xx>(
      bus_.aw86224_i2c_bus, device::aw86224::kI2cAddress);

  chip_.es8311 = std::make_unique<cpp_bus_driver::Es8311>(
      bus_.es8311_i2c_bus, bus_.es8311_i2s_bus, device::es8311::kI2cAddress);

  chip_.icm20948 = std::make_unique<cpp_bus_driver::Icm20948>(
      bus_.icm20948_i2c_bus, device::icm20948::kI2cAddress);

  chip_.l76k = std::make_unique<cpp_bus_driver::L76k>(
      bus_.l76k_uart_bus, [this](bool value) -> bool {
        return chip_.xl9535->GpioWrite(
            gpio::xl9535::kGpsWakeUp, static_cast<uint8_t>(value));
      });

  chip_.sx1262 =
      std::make_unique<usp_cpp_bus_driver::Sx126x>(bus_.radio_spi_bus,
          gpio::radio::kBusy, gpio::radio::kCs, [this](bool level) {
            return chip_.xl9535->GpioWrite(
                gpio::xl9535::kRadioRst, static_cast<uint8_t>(level));
          });

  chip_.lr2021 = std::make_unique<usp_cpp_bus_driver::Lr20xx>(
      bus_.radio_spi_bus, gpio::radio::kBusy, gpio::radio::kCs,
      [this](bool released) {
        if (chip_.xl9535 == nullptr || tool_ == nullptr) {
          return false;
        }
        if (!released) {
          bool result = chip_.xl9535->GpioWrite(gpio::xl9535::kRadioRst, 1);
          tool_->DelayMs(10);
          result &= chip_.xl9535->GpioWrite(gpio::xl9535::kRadioRst, 0);
          tool_->DelayMs(9);
          return result;
        }
        const bool result = chip_.xl9535->GpioWrite(gpio::xl9535::kRadioRst, 1);
        tool_->DelayMs(10);
        return result;
      });

  bus_.xl9555_i2c_bus = std::make_shared<cpp_bus_driver::SoftwareI2c>(
      keyboard_gpio::xl9555::kSda, keyboard_gpio::xl9555::kScl);
  bus_.tca8418_i2c_bus = std::make_shared<cpp_bus_driver::SoftwareI2c>(
      keyboard_gpio::tca8418::kSda, keyboard_gpio::tca8418::kScl);

  bus_.cc1101_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(bus_.radio_spi_bus, 0);
  bus_.nrf24l01_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(bus_.radio_spi_bus, 0);

  chip_.xl9555 = std::make_unique<cpp_bus_driver::Xl95x5>(
      bus_.xl9555_i2c_bus, keyboard_device::xl9555::kI2cAddress);
  chip_.tca8418 = std::make_unique<cpp_bus_driver::Tca8418>(
      bus_.tca8418_i2c_bus, keyboard_device::tca8418::kI2cAddress);
  chip_.tca8418_backlight =
      std::make_unique<cpp_bus_driver::Pwm>(keyboard_gpio::tca8418::kBl);

  chip_.cc1101 = std::make_unique<cpp_bus_driver::Cc1101>(bus_.cc1101_spi_bus,
      keyboard_gpio::t_mix_rf::cc1101::kCs,
      keyboard_gpio::t_mix_rf::cc1101::kMiso,
      keyboard_gpio::t_mix_rf::cc1101::kGdo0,
      keyboard_gpio::t_mix_rf::cc1101::kGdo2);
  chip_.nrf24l01 = std::make_unique<cpp_bus_driver::Nrf24l01x>(
      bus_.nrf24l01_spi_bus, keyboard_gpio::t_mix_rf::nrf24l01::kCs,
      keyboard_gpio::t_mix_rf::nrf24l01::kCe,
      keyboard_gpio::t_mix_rf::nrf24l01::kInt);
}

bool TDisplayP4Driver::Init(InitMode mode) {
  CreateDrivers();
  const int64_t start_time_us = tool_->GetSystemTimeUs();
  const bool result = InitDrivers(mode);
  const int64_t elapsed_time_us = tool_->GetSystemTimeUs() - start_time_us;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "TDisplayP4Driver init finished (mode: %s, result: %s, elapsed: %lld "
      "ms)\n",
      mode == InitMode::kAsync ? "async" : "sync",
      result ? "success" : "failed",
      static_cast<long long>(elapsed_time_us / 1000));
  return result;
}

bool TDisplayP4Driver::InitDrivers(InitMode mode) {
  bool result = true;

  result &= InitXl9535();
  result &= InitPower();
  result &= ConfigXl9535();

  result &= InitSgm38121();

  result &= InitBq27220();

  if (mode == InitMode::kAsync) {
    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4Driver*>(arg);
                     if (self->InitScreen()) {
                       self->InitTouch();
                       self->InitScreenBacklight();
                     }
                     vTaskDelete(NULL);
                   },
                   "ScreenTask", 4096, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4Driver*>(arg);
                     self->InitKeyboard();
                     vTaskDelete(NULL);
                   },
                   "InitKeyboardTask", 4096, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4Driver*>(arg);
                     self->InitPcf8563();
                     vTaskDelete(NULL);
                   },
                   "InitPcf8563Task", 2048, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4Driver*>(arg);
                     self->InitAw86224();
                     vTaskDelete(NULL);
                   },
                   "InitAw86224Task", 4096, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4Driver*>(arg);
                     if (self->InitEs8311() && self->ConfigEs8311()) {
                       self->SetEs8311PowerState(Es8311PowerState::kSleep);
                     }
                     vTaskDelete(NULL);
                   },
                   "InitConfigEs8311Task", 4096, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4Driver*>(arg);
                     self->InitL76k();
                     vTaskDelete(NULL);
                   },
                   "InitL76kTask", 2048, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4Driver*>(arg);
                     self->InitIcm20948();
                     vTaskDelete(NULL);
                   },
                   "InitIcm20948Task", 4096, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4Driver*>(arg);
                     self->InitRadio();
                     vTaskDelete(NULL);
                   },
                   "InitRadioTask", 4096, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4Driver*>(arg);
                     self->InitSdmmc(device::sd::kBasePath, SDMMC_FREQ_52M);
                     vTaskDelete(NULL);
                   },
                   "InitSdmmcTask", 4096, this, 3, NULL) == pdPASS);
  } else {
    const bool screen_result = InitScreen();
    result &= screen_result;
    if (screen_result) {
      result &= InitTouch();
      result &= InitScreenBacklight();
    }

    InitKeyboard();

    result &= InitPcf8563();
    result &= InitAw86224();
    result &= InitEs8311() && ConfigEs8311() &&
              SetEs8311PowerState(Es8311PowerState::kSleep);
    result &= InitL76k();
    result &= InitIcm20948();
    result &= InitRadio();

    InitSdmmc(device::sd::kBasePath, SDMMC_FREQ_52M);
  }

  return result;
}

bool TDisplayP4Driver::InitBq27220() {
  if (!chip_.bq27220->Init()) {
    status_.bq27220.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBq27220 failed\n");
    return false;
  } else {
    cpp_bus_driver::Bq27220::CedvProfile battery_profile;
    battery_profile.design_capacity = battery_info().capacity_mah;
    battery_profile.full_charge_capacity = battery_info().capacity_mah;
    cpp_bus_driver::Bq27220::GaugingConfig gauging_config;

    bool result = true;
    result &= chip_.bq27220->ApplyBatteryProfileIfNeeded(
        battery_profile, gauging_config);
    result &= chip_.bq27220->SetTemperatureMode(
        cpp_bus_driver::Bq27220::TemperatureMode::kExternalNtc);

    status_.bq27220.init_flag = result;
    if (result) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitBq27220 success\n");
    } else {
      LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBq27220 failed\n");
    }
    return result;
  }
}

bool TDisplayP4Driver::InitXl9535() {
  if (!chip_.xl9535->Init()) {
    status_.xl9535.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitXl9535 failed\n");
    return false;
  } else {
    status_.xl9535.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitXl9535 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitSgm38121() {
  if (chip_.sgm38121 == nullptr || !chip_.sgm38121->Init()) {
    status_.sgm38121.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSgm38121 failed\n");
    return false;
  }

  bool result = true;
#if defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_SC2336)
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2,
      cpp_bus_driver::Sgm38121::Status::kOff);
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV2710)
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 3000);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2,
      cpp_bus_driver::Sgm38121::Status::kOff);
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV5645)
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2,
      cpp_bus_driver::Sgm38121::Status::kOff);
#endif

  status_.sgm38121.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      "InitSgm38121 %s\n", result ? "success" : "failed");
  return result;
}

bool TDisplayP4Driver::InitHi8561() {
  if (chip_.hi8561 == nullptr) {
    status_.hi8561.init_flag = false;
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
  status_.hi8561.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitHi8561 success\n" : "InitHi8561 failed\n");
  return result;
}

bool TDisplayP4Driver::InitHi8561Touch() {
  if (!status_.xl9535.init_flag ||
      !chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 0)) {
    status_.hi8561_touch.init_flag = false;
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }
  tool_->DelayMs(2);
  if (!chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 1)) {
    status_.hi8561_touch.init_flag = false;
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }
  tool_->DelayMs(120);

  if (chip_.hi8561_touch == nullptr) {
    status_.hi8561_touch.init_flag = false;
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }

  if (!chip_.hi8561_touch->Init()) {
    status_.hi8561_touch.init_flag = false;
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  } else {
    status_.hi8561_touch.init_flag = true;
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561Touch success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitHi8561Backlight() {
  if (chip_.hi8561_backlight == nullptr) {
    status_.hi8561_backlight.init_flag = false;
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Backlight failed\n");
    return false;
  }

  if (!chip_.hi8561_backlight->Init(
          ledc_timer_t::LEDC_TIMER_0, ledc_channel_t::LEDC_CHANNEL_0, 2000)) {
    status_.hi8561_backlight.init_flag = false;
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Backlight failed\n");
    return false;
  } else {
    status_.hi8561_backlight.init_flag = true;
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561Backlight success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitRm69a10() {
  if (chip_.rm69a10 == nullptr) {
    status_.rm69a10.init_flag = false;
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
  status_.rm69a10.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitRm69a10 success\n" : "InitRm69a10 failed\n");
  return result;
}

bool TDisplayP4Driver::InitGt9895() {
  if (chip_.gt9895 == nullptr) {
    status_.gt9895.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitGt9895 failed\n");
    return false;
  }

  if (!chip_.gt9895->Init()) {
    status_.gt9895.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitGt9895 failed\n");
    return false;
  } else {
    status_.gt9895.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitGt9895 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitPcf8563() {
  if (!chip_.pcf8563->Init()) {
    status_.pcf8563.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitPcf8563 failed\n");
    return false;
  } else {
    status_.pcf8563.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitPcf8563 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitAw86224() {
  if (!chip_.aw86224->Init(500000)) {
    status_.aw86224.init_flag = false;
    status_.aw86224.ram_waveform_info =
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
  status_.aw86224.init_flag = result;
  status_.aw86224.ram_waveform_info =
      cpp_bus_driver::Aw862xx::GetRamWaveformInfo(
          cpp_bus_driver::Aw862xx::RamWaveformLibrary::kRam12k041230_235);
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "InitAw86224 success (RAM library: %s)\n",
        status_.aw86224.ram_waveform_info.name);
  } else {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitAw86224 failed\n");
  }
  return result;
}

bool TDisplayP4Driver::InitEs8311() {
  if (!chip_.es8311->Init()) {
    status_.es8311.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8311 failed\n");
    return false;
  } else {
    if (!chip_.es8311->Init(device::es8311::kMclkMultiple,
            device::es8311::kSampleRate, device::es8311::kBitsPerSample)) {
      status_.es8311.init_flag = false;
      LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8311 failed\n");
      return false;
    } else {
      status_.es8311.init_flag = true;
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitEs8311 success\n");
      return true;
    }
  }
}

bool TDisplayP4Driver::InitL76k() {
  if (!chip_.l76k->Init()) {
    if (!bus_.l76k_uart_bus->SetBaudRate(115200)) {
      status_.l76k.init_flag = false;
      LogMessage(LogLevel::kError, __FILE__, __LINE__, "SetBaudRate failed\n");
      return false;
    }
    if (!chip_.l76k->Init()) {
      status_.l76k.init_flag = false;
      LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitL76k failed\n");
      return false;
    } else {
      bool result = true;
      result &=
          chip_.l76k->SetBaudRate(cpp_bus_driver::L76k::BaudRate::kBr115200Bps);
      result &= chip_.l76k->SetUpdateFrequency(
          cpp_bus_driver::L76k::UpdateFreq::kFreq5Hz);
      result &= chip_.l76k->ClearRxBufferData();
      result &= chip_.l76k->Sleep(true);
      if (!result) {
        chip_.l76k->Deinit();
      }

      status_.l76k.init_flag = result;
      if (result) {
        LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitL76k success\n");
      } else {
        LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitL76k failed\n");
      }
      return result;
    }

  } else {
    bool result = true;
    result &=
        chip_.l76k->SetBaudRate(cpp_bus_driver::L76k::BaudRate::kBr115200Bps);

    result &= chip_.l76k->SetUpdateFrequency(
        cpp_bus_driver::L76k::UpdateFreq::kFreq5Hz);
    result &= chip_.l76k->ClearRxBufferData();
    result &= chip_.l76k->Sleep(true);
    if (!result) {
      chip_.l76k->Deinit();
    }

    status_.l76k.init_flag = result;
    if (result) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitL76k success\n");
    } else {
      LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitL76k failed\n");
    }
    return result;
  }
}

bool TDisplayP4Driver::InitIcm20948() {
  cpp_bus_driver::Icm20948::Config config;
  config.accel_range = cpp_bus_driver::Icm20948::AccelRange::k2g;
  config.gyro_range = cpp_bus_driver::Icm20948::GyroRange::k250Dps;
  config.accel_dlpf = cpp_bus_driver::Icm20948::Dlpf::k6;
  config.gyro_dlpf = cpp_bus_driver::Icm20948::Dlpf::k6;
  config.accel_sample_rate_divider = 9;
  config.gyro_sample_rate_divider = 9;
  config.magnetometer_mode =
      cpp_bus_driver::Icm20948::MagnetometerMode::kContinuous20Hz;

  bool result = chip_.icm20948->Init(config);
  if (result) {
    result = chip_.icm20948->SetSleep(true);
    if (!result) {
      chip_.icm20948->Deinit(false);
    }
  }
  status_.icm20948.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitIcm20948 success\n" : "InitIcm20948 failed\n");
  return result;
}

bool TDisplayP4Driver::InitSx1262() {
  if (IsSx1262Ready()) {
    radio_type_ = RadioType::kSx1262;
    return true;
  }
  if (IsLr2021Ready() || chip_.sx1262 == nullptr) {
    return false;
  }

  status_.sx1262.init_flag = false;
  if (!chip_.sx1262->Init(10000000)) {
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "SX1262 transport probe failed\n");
    return false;
  }

  // SX1261 与 SX1262 的内部版本字符串均以 "SX1261" 开头。
  std::array<uint8_t, 6> version = {};
  const bool detected =
      sx126x_read_register(chip_.sx1262->context(), kSx1262VersionStringAddress,
          version.data(),
          static_cast<uint8_t>(version.size())) == SX126X_STATUS_OK &&
      version == kSx1262VersionPrefix;
  if (!detected) {
    chip_.sx1262->Deinit(false);
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "SX1262 chip detection failed, trying LR2021\n");
    return false;
  }

  if (!chip_.sx1262->SetSleep()) {
    chip_.sx1262->Deinit(false);
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitSx1262 sleep failed\n");
    return false;
  }

  status_.sx1262.init_flag = true;
  radio_type_ = RadioType::kSx1262;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "Auto detected T-Display-P4 radio: SX1262\n");
  return true;
}

bool TDisplayP4Driver::InitLr2021() {
  if (IsLr2021Ready()) {
    radio_type_ = RadioType::kLr2021;
    return true;
  }
  if (IsSx1262Ready() || chip_.lr2021 == nullptr) {
    return false;
  }

  status_.lr2021.init_flag = false;
  if (!chip_.lr2021->Init(10000000)) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitLr2021 transport failed\n");
    return false;
  }

  constexpr uint8_t kDetectionAttempts = 10;
  lr20xx_system_version_t version = {};
  bool detected = false;
  for (uint8_t attempt = 0; attempt < kDetectionAttempts; ++attempt) {
    version = {};
    detected = chip_.lr2021->Invoke(lr20xx_system_get_version, &version) ==
                   LR20XX_STATUS_OK &&
               version.major == kLr2021ExpectedVersionMajor &&
               version.minor == kLr2021ExpectedVersionMinor;
    if (detected) {
      break;
    }
    if (attempt + 1U < kDetectionAttempts) {
      tool_->DelayMs(10);
      if (!chip_.lr2021->Reset()) {
        break;
      }
    }
  }

  if (!detected) {
    chip_.lr2021->Deinit(false);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr2021 chip detection failed (fw: %u.%u)\n",
        static_cast<unsigned>(version.major),
        static_cast<unsigned>(version.minor));
    return false;
  }

  const auto configure_rf_switch =
      [this](
          lr20xx_system_dio_t dio, lr20xx_system_dio_rf_switch_cfg_t config) {
        return chip_.lr2021->Invoke(lr20xx_system_set_dio_function, dio,
                   LR20XX_SYSTEM_DIO_FUNC_RF_SWITCH,
                   LR20XX_SYSTEM_DIO_DRIVE_NONE) == LR20XX_STATUS_OK &&
               chip_.lr2021->Invoke(lr20xx_system_set_dio_rf_switch_cfg, dio,
                   config) == LR20XX_STATUS_OK;
      };
  constexpr auto kCalibrationMask =
      static_cast<lr20xx_system_calibration_mask_t>(
          LR20XX_SYSTEM_CALIB_LF_RC_MASK | LR20XX_SYSTEM_CALIB_HF_RC_MASK |
          LR20XX_SYSTEM_CALIB_PLL_MASK | LR20XX_SYSTEM_CALIB_AAF_MASK |
          LR20XX_SYSTEM_CALIB_MU_MASK | LR20XX_SYSTEM_CALIB_PA_OFF_MASK);

  const lr20xx_system_sleep_cfg_t sleep_config = {
      .is_clk_32k_enabled = false,
      .is_ram_retention_enabled = true,
  };
  const bool result =
      chip_.lr2021->Invoke(lr20xx_system_set_standby_mode,
          LR20XX_SYSTEM_STANDBY_MODE_RC) == LR20XX_STATUS_OK &&
      chip_.lr2021->Invoke(lr20xx_system_set_tcxo_mode,
          LR20XX_SYSTEM_TCXO_CTRL_3_3V, 32768U) == LR20XX_STATUS_OK &&
      chip_.lr2021->Invoke(lr20xx_system_set_reg_mode,
          LR20XX_SYSTEM_REG_MODE_DCDC) == LR20XX_STATUS_OK &&
      chip_.lr2021->Invoke(lr20xx_radio_common_set_rx_tx_fallback_mode,
          LR20XX_RADIO_FALLBACK_STDBY_RC) == LR20XX_STATUS_OK &&
      chip_.lr2021->Invoke(lr20xx_system_clear_irq_status,
          LR20XX_SYSTEM_IRQ_ALL_MASK) == LR20XX_STATUS_OK &&
      chip_.lr2021->Invoke(lr20xx_system_calibrate, kCalibrationMask) ==
          LR20XX_STATUS_OK &&
      configure_rf_switch(
          LR20XX_SYSTEM_DIO_6, LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_RX_HF) &&
      configure_rf_switch(
          LR20XX_SYSTEM_DIO_7, LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_TX_HF) &&
      configure_rf_switch(
          LR20XX_SYSTEM_DIO_8, LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_RX_LF |
                                   LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_TX_LF) &&
      configure_rf_switch(
          LR20XX_SYSTEM_DIO_10, LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_RX_HF |
                                    LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_TX_HF) &&
      chip_.lr2021->Invoke(lr20xx_system_set_dio_function, LR20XX_SYSTEM_DIO_11,
          LR20XX_SYSTEM_DIO_FUNC_IRQ,
          LR20XX_SYSTEM_DIO_DRIVE_NONE) == LR20XX_STATUS_OK &&
      chip_.lr2021->Invoke(lr20xx_system_set_dio_irq_cfg, LR20XX_SYSTEM_DIO_11,
          static_cast<lr20xx_system_irq_mask_t>(0)) == LR20XX_STATUS_OK &&
      chip_.lr2021->SetSleep(sleep_config);
  if (!result) {
    chip_.lr2021->Deinit(false);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr2021 hardware configuration failed\n");
    return false;
  }

  status_.lr2021.init_flag = true;
  radio_type_ = RadioType::kLr2021;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "Auto detected T-Display-P4 radio: LR2021 (fw: %u.%u)\n",
      static_cast<unsigned>(version.major),
      static_cast<unsigned>(version.minor));
  return true;
}

bool TDisplayP4Driver::InitXl9555() {
  if (!chip_.xl9555->Init()) {
    status_.xl9555.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitXl9555 failed\n");
    return false;
  } else {
    status_.xl9555.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitXl9555 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitTca8418() {
  if (!chip_.tca8418->Init()) {
    status_.tca8418.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitTca8418 failed\n");
    return false;
  } else {
    bool result = true;
    result &= chip_.tca8418->SetKeypadScanWindow(0, 0,
        keyboard_device::tca8418::kKeypadScanWidth,
        keyboard_device::tca8418::kKeypadScanHeight);
    result &= chip_.tca8418->SetIrqGpioMode(
        cpp_bus_driver::Tca8418::IrqMask::kKeyEvents);
    result &= chip_.tca8418->ClearIrqFlag(
        cpp_bus_driver::Tca8418::IrqFlag::kKeyEvents);

    status_.tca8418.init_flag = result;
    if (result) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitTca8418 success\n");
    } else {
      LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitTca8418 failed\n");
    }
    return result;
  }
}

bool TDisplayP4Driver::InitTca8418Backlight() {
  if (!chip_.tca8418_backlight->Init(ledc_timer_t::LEDC_TIMER_1,
          ledc_channel_t::LEDC_CHANNEL_1, 1000000, 0,
          ledc_mode_t::LEDC_LOW_SPEED_MODE,
          ledc_timer_bit_t ::LEDC_TIMER_5_BIT)) {
    status_.tca8418_backlight.init_flag = false;
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitTca8418Backlight failed\n");
    return false;
  } else {
    status_.tca8418_backlight.init_flag = true;
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "InitTca8418Backlight success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitCc1101() {
  if (chip_.cc1101 == nullptr || !chip_.cc1101->Init()) {
    status_.cc1101.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitCc1101 failed\n");
    return false;
  }

  if (!chip_.cc1101->Sleep()) {
    chip_.cc1101->Deinit(false);
    status_.cc1101.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitCc1101 failed\n");
    return false;
  }

  status_.cc1101.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitCc1101 success\n");
  return true;
}

bool TDisplayP4Driver::InitNrf24l01() {
  if (chip_.nrf24l01 == nullptr || !chip_.nrf24l01->Init()) {
    status_.nrf24l01.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitNrf24l01 failed\n");
    return false;
  }

  if (!chip_.nrf24l01->PowerDown()) {
    chip_.nrf24l01->Deinit(false);
    status_.nrf24l01.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitNrf24l01 failed\n");
    return false;
  }

  status_.nrf24l01.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitNrf24l01 success\n");
  return true;
}

bool TDisplayP4Driver::InitPower() {
  bool result = true;
  result &= InitLdoPower(3, 2500);
  result &= InitLdoPower(4, 3300);
  return result;
}

bool TDisplayP4Driver::InitScreen() {
  if (!status_.xl9535.init_flag ||
      !chip_.xl9535->GpioWrite(gpio::xl9535::kScreenRst, 0)) {
    return false;
  }
  tool_->DelayMs(2);
  if (!chip_.xl9535->GpioWrite(gpio::xl9535::kScreenRst, 1)) {
    return false;
  }
  tool_->DelayMs(120);

  if (!DetectScreenType()) {
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
  status_.hi8561.init_flag = false;
  status_.hi8561_touch.init_flag = false;
  status_.hi8561_backlight.init_flag = false;
  status_.rm69a10.init_flag = false;

  switch (screen.type) {
    case device::ScreenType::kHi8561:
      chip_.hi8561 =
          std::make_unique<cpp_bus_driver::Hi8561>(bus_.screen_mipi_bus);
      return InitHi8561();
    case device::ScreenType::kRm69a10:
      chip_.rm69a10 =
          std::make_unique<cpp_bus_driver::Rm69a10>(bus_.screen_mipi_bus);
      return InitRm69a10();
    default:
      return false;
  }
}

bool TDisplayP4Driver::InitTouch() {
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      return InitHi8561Touch();
    case device::ScreenType::kRm69a10:
      return status_.gt9895.init_flag || InitGt9895();
    default:
      return false;
  }
}

bool TDisplayP4Driver::InitScreenBacklight() {
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      return InitHi8561Backlight();
    case device::ScreenType::kRm69a10:
      return true;
    default:
      return false;
  }
}

bool TDisplayP4Driver::InitRadio() {
  if (IsRadioReady()) {
    return true;
  }

  radio_type_ = RadioType::kUnknown;
  if (InitSx1262()) {
    return true;
  }
  if (InitLr2021()) {
    return true;
  }

  LogMessage(LogLevel::kError, __FILE__, __LINE__,
      "No supported radio detected on shared SX1262/LR2021 pins\n");
  return false;
}

bool TDisplayP4Driver::InitKeyboard() {
  keyboard_connected_ = false;
  status_.xl9555.init_flag = false;
  status_.tca8418.init_flag = false;
  status_.tca8418_backlight.init_flag = false;
  status_.cc1101.init_flag = false;
  status_.nrf24l01.init_flag = false;

  if (!InitXl9555()) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "Keyboard device not connected\n");
    return false;
  }

  bool result = ConfigXl9555();
  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "ConfigXl9555 failed\n");
    return false;
  }

  result &= tool_->SetGpioMode(keyboard_gpio::t_mix_rf::cc1101::kCs,
      cpp_bus_driver::Tool::GpioMode::kOutput);
  result &= tool_->SetGpioMode(keyboard_gpio::t_mix_rf::nrf24l01::kCs,
      cpp_bus_driver::Tool::GpioMode::kOutput);
  result &= tool_->SetGpioMode(keyboard_gpio::t_mix_rf::st25r3916::kCs,
      cpp_bus_driver::Tool::GpioMode::kOutput);
  result &= tool_->GpioWrite(keyboard_gpio::t_mix_rf::cc1101::kCs, 1);
  result &= tool_->GpioWrite(keyboard_gpio::t_mix_rf::nrf24l01::kCs, 1);
  result &= tool_->GpioWrite(keyboard_gpio::t_mix_rf::st25r3916::kCs, 1);

  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "Keyboard gpio failed\n");
    return false;
  }

  result &= InitTca8418();
  result &= InitTca8418Backlight();
  result &= InitCc1101();
  result &= InitNrf24l01();

  keyboard_connected_ = result;

  return result;
}

bool TDisplayP4Driver::InitSpiffs(
    const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf) {
  esp_vfs_spiffs_conf_t conf = {
      .base_path = base_path,
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = false,
  };

  esp_err_t result = esp_vfs_spiffs_register(&conf);
  if (result != ESP_OK) {
    if (result == ESP_FAIL) {
      LogMessage(LogLevel::kError, __FILE__, __LINE__,
          "Failed to mount or format filesystem (error code: %#X)\n", result);
    } else if (result == ESP_ERR_NOT_FOUND) {
      LogMessage(LogLevel::kError, __FILE__, __LINE__,
          "Failed to find spiffs partition (error code: %#X)\n", result);
    } else {
      LogMessage(LogLevel::kError, __FILE__, __LINE__,
          "Failed to initialize spiffs (error code: %#X)\n", result);
    }
    return false;
  }

  size_t total = 0, used = 0;
  result = esp_spiffs_info(conf.partition_label, &total, &used);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "Failed to get spiffs partition information (error code: %#X). "
        "formatting...\n",
        result);
    esp_spiffs_format(conf.partition_label);
    return false;
  }

  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "Partition size: total: %d bytes, used: %d bytes\n", total, used);

  if (used > total) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "Number of used bytes cannot be larger than total performing "
        "esp_spiffs_check\n");
    result = esp_spiffs_check(conf.partition_label);
    if (result != ESP_OK) {
      LogMessage(LogLevel::kError, __FILE__, __LINE__,
          "esp_spiffs_check failed (error code: %#X)\n", result);
      return false;
    } else {
      LogMessage(
          LogLevel::kError, __FILE__, __LINE__, "esp_spiffs_check success\n");
    }
  }

  spiffs_conf = conf;
  return true;
}

bool TDisplayP4Driver::InitSdmmc(const char* base_path, int max_freq_khz) {
  if (base_path == nullptr || base_path[0] == '\0') {
    return false;
  }
  if (sd_card_ != nullptr && !DeinitSdmmc()) {
    return false;
  }
  if (!SetSdPowerEnabled(true)) {
    return false;
  }

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024,
      .disk_status_check_enable = true,
      .use_one_fat = false,
  };

  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.slot = SDMMC_HOST_SLOT_0;
  host.max_freq_khz = max_freq_khz;

  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.width = 4;
  slot_config.clk = static_cast<gpio_num_t>(gpio::sd::kSdioClk);
  slot_config.cmd = static_cast<gpio_num_t>(gpio::sd::kSdioCmd);
  slot_config.d0 = static_cast<gpio_num_t>(gpio::sd::kSdioD0);
  slot_config.d1 = static_cast<gpio_num_t>(gpio::sd::kSdioD1);
  slot_config.d2 = static_cast<gpio_num_t>(gpio::sd::kSdioD2);
  slot_config.d3 = static_cast<gpio_num_t>(gpio::sd::kSdioD3);
  slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

  sdmmc_card_t* card = nullptr;
  esp_err_t result = esp_vfs_fat_sdmmc_mount(
      base_path, &host, &slot_config, &mount_config, &card);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "esp_vfs_fat_sdmmc_mount failed (error code: %#X)\n", result);
    status_.sd_card.init_flag = false;
    sd_card_ = nullptr;
    sd_card_base_path_.clear();
    SetSdPowerEnabled(false);
    return false;
  }

  sdmmc_card_print_info(stdout, card);

  sd_card_ = card;
  sd_card_base_path_ = base_path;
  status_.sd_card.init_flag = true;
  return true;
}

bool TDisplayP4Driver::InitSdspi(
    const char* base_path, spi_host_device_t host_id, int max_freq_khz) {
  if (base_path == nullptr || base_path[0] == '\0') {
    return false;
  }
  if (sd_card_ != nullptr && !DeinitSdmmc()) {
    return false;
  }
  if (!SetSdPowerEnabled(true)) {
    return false;
  }

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024,
      .disk_status_check_enable = false,
      .use_one_fat = false,
  };

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  host.slot = host_id;
  host.max_freq_khz = max_freq_khz;

  spi_bus_config_t bus_config = {
      .mosi_io_num = gpio::sd::kMosi,
      .miso_io_num = gpio::sd::kMiso,
      .sclk_io_num = gpio::sd::kSclk,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .data4_io_num = -1,
      .data5_io_num = -1,
      .data6_io_num = -1,
      .data7_io_num = -1,
      .data_io_default_level = 0,
      .max_transfer_sz = 0,
      .flags = SPICOMMON_BUSFLAG_MASTER,
      .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
      .intr_flags = 0,
  };

  esp_err_t result =
      spi_bus_initialize(host_id, &bus_config, SDSPI_DEFAULT_DMA);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "spi_bus_initialize failed (error code: %#X)\n", result);
    status_.sd_card.init_flag = false;
    sd_card_ = nullptr;
    sd_card_base_path_.clear();
    SetSdPowerEnabled(false);
    return false;
  }

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.host_id = host_id;
  slot_config.gpio_cs = static_cast<gpio_num_t>(gpio::sd::kCs);

  sdmmc_card_t* card = nullptr;
  result = esp_vfs_fat_sdspi_mount(
      base_path, &host, &slot_config, &mount_config, &card);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "esp_vfs_fat_sdspi_mount failed (error code: %#X)\n", result);
    status_.sd_card.init_flag = false;
    sd_card_ = nullptr;
    sd_card_base_path_.clear();
    SetSdPowerEnabled(false);
    return false;
  }

  sdmmc_card_print_info(stdout, card);
  sd_card_ = card;
  sd_card_base_path_ = base_path;
  status_.sd_card.init_flag = true;
  return true;
}

bool TDisplayP4Driver::DeinitScreen() {
  bool result = true;

  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      if (status_.hi8561.init_flag) {
        result &= chip_.hi8561->Deinit();
        status_.hi8561.init_flag = false;
      }
      break;
    case device::ScreenType::kRm69a10:
      if (status_.rm69a10.init_flag) {
        result &= chip_.rm69a10->Deinit();
        status_.rm69a10.init_flag = false;
      }
      break;
    default:
      break;
  }

  return result;
}

bool TDisplayP4Driver::DeinitTouch() {
  bool result = true;

  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      if (status_.hi8561_touch.init_flag) {
        result &= chip_.hi8561_touch->Deinit();
        status_.hi8561_touch.init_flag = false;
      }
      break;
    case device::ScreenType::kRm69a10:
      if (status_.gt9895.init_flag) {
        result &= chip_.gt9895->Deinit();
        status_.gt9895.init_flag = false;
      }
      break;
    default:
      break;
  }

  return result;
}

bool TDisplayP4Driver::DeinitScreenBacklight() {
  bool result = true;

  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      if (status_.hi8561_backlight.init_flag) {
        result &= chip_.hi8561_backlight->Stop(0);
        status_.hi8561_backlight.init_flag = false;
      }
      break;
    case device::ScreenType::kRm69a10:
      break;
    default:
      break;
  }

  return result;
}

bool TDisplayP4Driver::DeinitKeyboard() {
  bool result = true;

  if (status_.tca8418_backlight.init_flag) {
    result &= chip_.tca8418_backlight->Stop(0);
    status_.tca8418_backlight.init_flag = false;
  }
  if (status_.tca8418.init_flag) {
    result &= chip_.tca8418->Deinit();
    status_.tca8418.init_flag = false;
  }
  if (status_.cc1101.init_flag) {
    result &= chip_.cc1101->Deinit(false);
    status_.cc1101.init_flag = false;
  }
  if (status_.nrf24l01.init_flag) {
    result &= chip_.nrf24l01->Deinit(false);
    status_.nrf24l01.init_flag = false;
  }
  if (status_.xl9555.init_flag) {
    result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed1, 1);
    result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed2, 1);
    result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed3, 1);
    result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfEn, 0);
    result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTca8418Rst, 0);

    result &= chip_.xl9555->Deinit();
    status_.xl9555.init_flag = false;
  }

  result &= tool_->ResetGpio(keyboard_gpio::t_mix_rf::cc1101::kCs);
  result &= tool_->ResetGpio(keyboard_gpio::t_mix_rf::cc1101::kGdo0);
  result &= tool_->ResetGpio(keyboard_gpio::t_mix_rf::cc1101::kGdo2);
  result &= tool_->ResetGpio(keyboard_gpio::t_mix_rf::nrf24l01::kCs);
  result &= tool_->ResetGpio(keyboard_gpio::t_mix_rf::st25r3916::kCs);

  keyboard_connected_ = false;

  return result;
}

bool TDisplayP4Driver::DeinitSdmmc() {
  if (sd_card_ == nullptr) {
    status_.sd_card.init_flag = false;
    sd_card_base_path_.clear();
    return SetSdPowerEnabled(false);
  }

  const esp_err_t result =
      esp_vfs_fat_sdcard_unmount(sd_card_base_path_.c_str(), sd_card_);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "esp_vfs_fat_sdcard_unmount failed (error code: %#X)\n", result);
    status_.sd_card.init_flag = false;
    return false;
  }

  sd_card_ = nullptr;
  sd_card_base_path_.clear();
  status_.sd_card.init_flag = false;
  return SetSdPowerEnabled(false);
}

bool TDisplayP4Driver::ConfigXl9535() {
  if (!status_.xl9535.init_flag) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "ConfigXl9535 failed\n");
    return false;
  }

  // XL9535 上电后输出寄存器默认为高电平。先预装安全状态，再切换输出方向，
  // 避免电源、复位和唤醒信号在配置过程中被短暂拉高。
  bool result = true;
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kPowerEn3v3, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kSky13453Vctl, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kScreenRst, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEthernetRst, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAudioPowerEn, 0);
  // ESP32-P4 只有在 USB PHY 电源保持开启时才能降低功耗；关闭该电源会
  // 产生约 20 mA 功耗，因此这里只在初始化时拉高，后续内部流程不再主动
  // 控制此引脚，预留接口仅供需要时显式调整。
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kUsbPhyPowerEn, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kGpsWakeUp, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c6En, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kRadioRst, 0);
  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "ConfigXl9535 failed\n");
    return false;
  }

  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kScreenRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kTouchInt, cpp_bus_driver::Xl95x5::Mode::kInput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kUsbPhyPowerEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kAudioPowerEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kPowerEn3v3, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kGpsWakeUp, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kRtcInt, cpp_bus_driver::Xl95x5::Mode::kInput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kEsp32c6WakeUp, cpp_bus_driver::Xl95x5::Mode::kInput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kEsp32c6En, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kEthernetRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kSdPowerEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kRadioRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kSky13453Vctl, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kIcm20948Int, cpp_bus_driver::Xl95x5::Mode::kInput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kRadioDio1, cpp_bus_driver::Xl95x5::Mode::kInput);
  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "ConfigXl9535 failed\n");
    return false;
  }

  // 先关闭 kPowerEn3v3 并等待负载电容放电，再只执行一次上电。
  tool_->DelayMs(500);
  if (!chip_.xl9535->GpioWrite(gpio::xl9535::kPowerEn3v3, 0)) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "ConfigXl9535 failed\n");
    return false;
  }
  tool_->DelayMs(10);
  return true;
}

bool TDisplayP4Driver::ConfigEs8311() {
  if (!status_.es8311.init_flag) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "ConfigEs8311 failed\n");
    return false;
  }

  cpp_bus_driver::Es8311::PowerStatus ps = {
      .contorl =
          {
              .analog_circuits = true,
              .analog_bias_circuits = true,
              .analog_adc_bias_circuits = true,
              .analog_adc_reference_circuits = true,
              .analog_dac_reference_circuit = true,
              .internal_reference_circuits = false,
          },
      .vmid = cpp_bus_driver::Es8311::Vmid::kStartUpVmidNormalSpeedCharge,
  };
  bool result = true;
  result &= chip_.es8311->SetPowerStatus(ps);
  result &= chip_.es8311->SetPgaPower(true);
  result &= chip_.es8311->SetAdcPower(true);
  result &= chip_.es8311->SetDacPower(true);
  result &= chip_.es8311->SetOutputToHpDrive(true);
  result &= chip_.es8311->SetAdcOffsetFreeze(
      cpp_bus_driver::Es8311::AdcOffsetFreeze::kDynamicHpf);
  result &= chip_.es8311->SetAdcHpfStage2Coeff(10);
  result &= chip_.es8311->SetDacEqualizer(false);
  result &= chip_.es8311->SetMic(cpp_bus_driver::Es8311::MicType::kAnalogMic,
      cpp_bus_driver::Es8311::MicInput::kMic1p1n);
  result &= chip_.es8311->SetAdcAutoVolumeControl(false);
  result &=
      chip_.es8311->SetAdcGain(cpp_bus_driver::Es8311::AdcGain::kGain18db);
  result &= chip_.es8311->SetAdcPgaGain(
      cpp_bus_driver::Es8311::AdcPgaGain::kGain30db);
  result &= chip_.es8311->SetAdcVolume(191);
  result &= chip_.es8311->SetDacVolume(191);

  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "ConfigEs8311 failed\n");
  }
  return result;
}

bool TDisplayP4Driver::ConfigXl9555() {
  if (!status_.xl9555.init_flag) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "ConfigXl9555 failed\n");
    return false;
  }

  bool result = true;
  result &= chip_.xl9555->SetGpioMode(
      keyboard_gpio::xl9555::kLed1, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9555->SetGpioMode(
      keyboard_gpio::xl9555::kLed2, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9555->SetGpioMode(
      keyboard_gpio::xl9555::kLed3, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9555->SetGpioMode(keyboard_gpio::xl9555::kTca8418Rst,
      cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9555->SetGpioMode(
      keyboard_gpio::xl9555::kTMixRfEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &=
      chip_.xl9555->SetGpioMode(keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch0,
          cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &=
      chip_.xl9555->SetGpioMode(keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch1,
          cpp_bus_driver::Xl95x5::Mode::kOutput);

  result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed1,
      1);  // 关闭指示灯
  result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed2, 1);
  result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed3, 1);
  result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfEn, 1);

  result &= SetCc1101RfSwitch(Cc1101RfSwitch::k868_915Mhz);

  result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTca8418Rst, 1);
  tool_->DelayMs(10);
  result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTca8418Rst, 0);
  tool_->DelayMs(10);
  result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTca8418Rst, 1);
  tool_->DelayMs(10);

  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "ConfigXl9555 failed\n");
  }
  return result;
}

bool TDisplayP4Driver::IsBq27220Ready() const {
  return status_.bq27220.init_flag && chip_.bq27220 != nullptr;
}

bool TDisplayP4Driver::IsXl9535Ready() const {
  return status_.xl9535.init_flag && chip_.xl9535 != nullptr;
}

bool TDisplayP4Driver::IsSgm38121Ready() const {
  return status_.sgm38121.init_flag && chip_.sgm38121 != nullptr;
}

bool TDisplayP4Driver::IsHi8561Ready() const {
  return status_.hi8561.init_flag && chip_.hi8561 != nullptr;
}

bool TDisplayP4Driver::IsHi8561TouchReady() const {
  return status_.hi8561_touch.init_flag && chip_.hi8561_touch != nullptr;
}

bool TDisplayP4Driver::IsHi8561BacklightReady() const {
  return status_.hi8561_backlight.init_flag &&
         chip_.hi8561_backlight != nullptr;
}

bool TDisplayP4Driver::IsRm69a10Ready() const {
  return status_.rm69a10.init_flag && chip_.rm69a10 != nullptr;
}

bool TDisplayP4Driver::IsGt9895Ready() const {
  return status_.gt9895.init_flag && chip_.gt9895 != nullptr;
}

bool TDisplayP4Driver::IsPcf8563Ready() const {
  return status_.pcf8563.init_flag && chip_.pcf8563 != nullptr;
}

bool TDisplayP4Driver::IsAw86224Ready() const {
  return status_.aw86224.init_flag && chip_.aw86224 != nullptr;
}

bool TDisplayP4Driver::IsEs8311Ready() const {
  return status_.es8311.init_flag && chip_.es8311 != nullptr;
}

bool TDisplayP4Driver::IsL76kReady() const {
  return status_.l76k.init_flag && chip_.l76k != nullptr;
}

bool TDisplayP4Driver::IsIcm20948Ready() const {
  return status_.icm20948.init_flag && chip_.icm20948 != nullptr;
}

bool TDisplayP4Driver::IsSx1262Ready() const {
  return status_.sx1262.init_flag && chip_.sx1262 != nullptr;
}

bool TDisplayP4Driver::IsLr2021Ready() const {
  return status_.lr2021.init_flag && chip_.lr2021 != nullptr;
}

bool TDisplayP4Driver::IsXl9555Ready() const {
  return status_.xl9555.init_flag && chip_.xl9555 != nullptr;
}

bool TDisplayP4Driver::IsTca8418Ready() const {
  return status_.tca8418.init_flag && chip_.tca8418 != nullptr;
}

bool TDisplayP4Driver::IsTca8418BacklightReady() const {
  return status_.tca8418_backlight.init_flag &&
         chip_.tca8418_backlight != nullptr;
}

bool TDisplayP4Driver::IsCc1101Ready() const {
  return status_.cc1101.init_flag && chip_.cc1101 != nullptr;
}

bool TDisplayP4Driver::IsNrf24l01Ready() const {
  return status_.nrf24l01.init_flag && chip_.nrf24l01 != nullptr;
}

bool TDisplayP4Driver::IsScreenReady() const {
  if (bus_.screen_mipi_bus == nullptr ||
      bus_.screen_mipi_bus->device_handle() == nullptr) {
    return false;
  }

  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      return IsHi8561Ready() && IsHi8561BacklightReady();
    case device::ScreenType::kRm69a10:
      return IsRm69a10Ready();
    default:
      return false;
  }
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

bool TDisplayP4Driver::IsRadioReady() const {
  switch (radio_type_) {
    case RadioType::kSx1262:
      return IsSx1262Ready();
    case RadioType::kLr2021:
      return IsLr2021Ready();
    default:
      return false;
  }
}

bool TDisplayP4Driver::IsSdmmcReady() const {
  return status_.sd_card.init_flag && sd_card_ != nullptr &&
         sdmmc_get_status(sd_card_) == ESP_OK;
}

bool TDisplayP4Driver::SetAw86224Standby() {
  return !IsAw86224Ready() || chip_.aw86224->StopRamPlaybackWaveform();
}

bool TDisplayP4Driver::SetEs8311PowerState(Es8311PowerState state) {
  if (!IsEs8311Ready()) {
    if (state == Es8311PowerState::kSleep) {
      return true;
    }
    if (!InitEs8311() || !ConfigEs8311()) {
      return false;
    }
  }
  const bool playback_enabled = state == Es8311PowerState::kPlayback ||
                                state == Es8311PowerState::kDuplex;
  const bool capture_enabled =
      state == Es8311PowerState::kCapture || state == Es8311PowerState::kDuplex;
  const bool sleep = state == Es8311PowerState::kSleep;
  // 该开关控制共享的 OUT_5V 音频电源域。除 NS4150 外，RT9080 也从
  // OUT_5V 生成 ES8311 模拟 ADC 使用的 AD_3V3，因此仅采集时同样要开启。
  if (!sleep && !SetAudioPowerEnabled(true)) {
    return false;
  }
  cpp_bus_driver::Es8311::PowerStatus power_status = {
      .contorl =
          {
              .analog_circuits = !sleep,
              .analog_bias_circuits = !sleep,
              .analog_adc_bias_circuits = capture_enabled,
              .analog_adc_reference_circuits = capture_enabled,
              .analog_dac_reference_circuit = playback_enabled,
              .internal_reference_circuits = false,
          },
      .vmid = sleep
                  ? cpp_bus_driver::Es8311::Vmid::kPowerDown
                  : cpp_bus_driver::Es8311::Vmid::kStartUpVmidNormalSpeedCharge,
  };

  bool result = true;
  if (sleep) {
    result &= chip_.es8311->SetOutputToHpDrive(false);
    result &= chip_.es8311->SetPgaPower(false);
    result &= chip_.es8311->SetAdcPower(false);
    result &= chip_.es8311->SetDacPower(false);
    result &= chip_.es8311->SetPowerStatus(power_status);
  } else {
    result &= chip_.es8311->SetPowerStatus(power_status);
    result &= chip_.es8311->SetPgaPower(capture_enabled);
    result &= chip_.es8311->SetAdcPower(capture_enabled);
    result &= chip_.es8311->SetDacPower(playback_enabled);
    result &= chip_.es8311->SetOutputToHpDrive(playback_enabled);
  }
  if (sleep) {
    result &= SetAudioPowerEnabled(false);
  } else if (!result) {
    SetAudioPowerEnabled(false);
  }
  return result;
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

bool TDisplayP4Driver::SetIcm20948Sleep(bool sleep) {
  if (!IsIcm20948Ready()) {
    if (sleep) {
      return true;
    }
    if (!InitIcm20948()) {
      return false;
    }
  }
  return chip_.icm20948->SetSleep(sleep);
}

bool TDisplayP4Driver::SetSx1262PowerState(Sx1262PowerState state) {
  if (radio_type_ == RadioType::kLr2021) {
    return false;
  }
  if (!IsSx1262Ready()) {
    if (state == Sx1262PowerState::kSleep) {
      return true;
    }
    if (!InitSx1262()) {
      return false;
    }
  }

  switch (state) {
    case Sx1262PowerState::kStandby:
      return chip_.sx1262->Wakeup();
    case Sx1262PowerState::kSleep:
      return chip_.sx1262->SetSleep();
    default:
      return false;
  }
}

bool TDisplayP4Driver::SetLr2021PowerState(Lr2021PowerState state) {
  if (radio_type_ == RadioType::kSx1262) {
    return false;
  }
  if (!IsLr2021Ready()) {
    if (state == Lr2021PowerState::kSleep) {
      return true;
    }
    if (!InitLr2021()) {
      return false;
    }
  }

  lr20xx_status_t result = LR20XX_STATUS_ERROR;
  if (state == Lr2021PowerState::kSleep) {
    const lr20xx_system_sleep_cfg_t sleep_config = {
        .is_clk_32k_enabled = false,
        .is_ram_retention_enabled = true,
    };
    result = chip_.lr2021->SetSleep(sleep_config) ? LR20XX_STATUS_OK
                                                  : LR20XX_STATUS_ERROR;
  } else if (chip_.lr2021->Wakeup()) {
    result = chip_.lr2021->Invoke(
        lr20xx_system_set_standby_mode, LR20XX_SYSTEM_STANDBY_MODE_RC);
  }

  if (result != LR20XX_STATUS_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "LR2021 power state change failed (error code: %d)\n",
        static_cast<int>(result));
    return false;
  }
  return true;
}

bool TDisplayP4Driver::SetCc1101PowerState(Cc1101PowerState state) {
  if (!IsCc1101Ready()) {
    return state == Cc1101PowerState::kSleep;
  }
  const bool result = state == Cc1101PowerState::kSleep
                          ? chip_.cc1101->Sleep()
                          : chip_.cc1101->Wakeup();
  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "CC1101 power state change failed\n");
    return false;
  }
  return true;
}

bool TDisplayP4Driver::SetNrf24l01PowerState(Nrf24l01PowerState state) {
  if (!IsNrf24l01Ready()) {
    return state == Nrf24l01PowerState::kSleep;
  }
  const bool result = state == Nrf24l01PowerState::kSleep
                          ? chip_.nrf24l01->PowerDown()
                          : chip_.nrf24l01->Standby();
  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "NRF24L01 power state change failed\n");
    return false;
  }
  return true;
}

bool TDisplayP4Driver::SetEsp32c6PowerEnabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  const bool result =
      chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c6En, enabled ? 1 : 0);
  if (result && enabled) {
    tool_->DelayMs(20);
  }
  return result;
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

bool TDisplayP4Driver::SetTouchEnabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  if (enabled && IsTouchReady()) {
    return true;
  }

  bool result = true;
  if (!enabled) {
    result &= DeinitTouch();
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 0);
    return result;
  }

  return InitTouch();
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
  if (result && enabled) {
    tool_->DelayMs(10);
  }
  return result;
}

bool TDisplayP4Driver::SetEthernetPowerEnabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  const bool result =
      chip_.xl9535->GpioWrite(gpio::xl9535::kEthernetRst, enabled ? 1 : 0);
  if (result && enabled) {
    tool_->DelayMs(20);
  }
  return result;
}

bool TDisplayP4Driver::SetAudioPowerEnabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  const bool result =
      chip_.xl9535->GpioWrite(gpio::xl9535::kAudioPowerEn, enabled ? 1 : 0);
  if (result && enabled) {
    tool_->DelayMs(10);
  }
  return result;
}

bool TDisplayP4Driver::SetUsbHostPowerEnabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  const bool result =
      chip_.xl9535->GpioWrite(gpio::xl9535::kUsbPhyPowerEn, enabled ? 1 : 0);
  if (result && enabled) {
    tool_->DelayMs(20);
  }
  return result;
}

bool TDisplayP4Driver::SetSdPowerEnabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  const bool result =
      chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, enabled ? 0 : 1);
  if (result && enabled) {
    tool_->DelayMs(20);
  }
  return result;
}

bool TDisplayP4Driver::SetRadioPowerState(RadioPowerState state) {
  if (!IsRadioReady()) {
    if (state == RadioPowerState::kSleep) {
      return true;
    }
    if (!InitRadio()) {
      return false;
    }
  }

  switch (radio_type_) {
    case RadioType::kSx1262:
      return SetSx1262PowerState(state == RadioPowerState::kSleep
                                     ? Sx1262PowerState::kSleep
                                     : Sx1262PowerState::kStandby);
    case RadioType::kLr2021:
      return SetLr2021PowerState(state == RadioPowerState::kSleep
                                     ? Lr2021PowerState::kSleep
                                     : Lr2021PowerState::kStandby);
    default:
      return state == RadioPowerState::kSleep;
  }
}

bool TDisplayP4Driver::PrepareForPowerOff() {
  bool result = true;
  if (IsScreenReady()) {
    result &= SetScreenSleep(true);
  }
  result &= SetAw86224Standby();
  result &= SetEs8311PowerState(Es8311PowerState::kSleep);
  result &= SetIcm20948Sleep(true);
  result &= SetL76kSleep(true);
  result &= SetRadioPowerState(RadioPowerState::kSleep);

  result &= SetTouchEnabled(false);
  result &= SetCameraPowerEnabled(false);
  result &= SetEsp32c6PowerEnabled(false);
  result &= SetEthernetPowerEnabled(false);
  result &= SetAudioPowerEnabled(false);
  result &= DeinitKeyboard();
  result &= DeinitSdmmc();

  // 将外设复位、电源使能及控制引脚设置为关机安全电平。
  if (status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kScreenRst, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c6En, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEthernetRst, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 1);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAudioPowerEn, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kPowerEn3v3, 1);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kRadioRst, 0);
  }

  result &= DeinitLdoPower(3);
  result &= DeinitLdoPower(4);
  return result;
}

bool TDisplayP4Driver::SetCc1101RfSwitch(Cc1101RfSwitch rf_switch) {
  if (!status_.xl9555.init_flag) {
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

bool TDisplayP4Driver::DetectScreenType() {
  status_.gt9895.init_flag = false;

  if (!status_.xl9535.init_flag ||
      !chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 0)) {
    return false;
  }
  tool_->DelayMs(2);
  if (!chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 1)) {
    return false;
  }
  tool_->DelayMs(120);

  if (chip_.gt9895 != nullptr && chip_.gt9895->Init()) {
    screen_info_ = ScreenInfoForType(device::ScreenType::kRm69a10);
    status_.gt9895.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "Auto detected T-Display-P4 screen: %s\n", screen_info_->name);
    return true;
  }

  if (bus_.gt9895_i2c_touch_bus != nullptr) {
    bus_.gt9895_i2c_touch_bus->Deinit(false);
  }
  screen_info_ = ScreenInfoForType(device::ScreenType::kHi8561);
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "Auto detected T-Display-P4 screen: %s\n", screen_info_->name);
  return true;
}

}  // namespace lilygo_device_driver
