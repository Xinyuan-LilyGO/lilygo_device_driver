/*
 * @Description: t_display_p4_air_driver
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:51:14

 * @LastEditTime: 2026-06-18 16:20:45
 * @License: GPL 3.0
 */
#include "t_display_p4_air_driver.h"

namespace lilygo_device_driver {
namespace gpio = t_display_p4_air::gpio;
namespace device = t_display_p4_air::device;
namespace {

using ScreenInfo = device::ScreenInfo;
using ScreenType = device::ScreenType;

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

constexpr const ScreenInfo* kDefaultScreenInfo = &kHi8561ScreenInfo;

}  // namespace

TDisplayP4AirDriver& TDisplayP4AirDriver::GetInstance() {
  static TDisplayP4AirDriver* instance = new TDisplayP4AirDriver();
  return *instance;
}

const device::ScreenInfo& TDisplayP4AirDriver::screen_info() const {
  return *(screen_info_ == nullptr ? kDefaultScreenInfo : screen_info_);
}

void TDisplayP4AirDriver::CreateDrivers() {
  tool_ = std::make_unique<cpp_bus_driver::Tool>();

  bus_.axp517_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      gpio::i2c::kPort2Sda, gpio::i2c::kPort2Scl, I2C_NUM_0);
  bus_.xl9535_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      gpio::i2c::kPort1Sda, gpio::i2c::kPort1Scl, I2C_NUM_1);

  bus_.sgm38121_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.axp517_i2c_bus);
  bus_.hi8561_i2c_touch_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.axp517_i2c_bus);
  bus_.aw86224_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.xl9535_i2c_bus);

  bus_.es8389_i2s_bus = std::make_shared<cpp_bus_driver::HardwareI2s>(
      gpio::es8389::kAdcData, gpio::es8389::kDacData, gpio::es8389::kWsLrck,
      gpio::es8389::kBclk, gpio::es8389::kMclk, i2s_port_t::I2S_NUM_0,
      cpp_bus_driver::HardwareI2s::DataMode::kInputOutput,
      cpp_bus_driver::HardwareI2s::I2sMode::kStd,
      i2s_clock_src_t::I2S_CLK_SRC_DEFAULT);

  bus_.lr1121_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(gpio::lr1121::kMosi,
          gpio::lr1121::kSclk, gpio::lr1121::kMiso, SPI2_HOST, 0);

  bus_.nrf9151_uart_bus = std::make_shared<cpp_bus_driver::HardwareUart>(
      gpio::nrf9151::kUartTx, gpio::nrf9151::kUartRx, UART_NUM_1,
      gpio::nrf9151::kUartRts, gpio::nrf9151::kUartCts);

  chip_.axp517 = std::make_unique<cpp_bus_driver::Axp517>(
      bus_.axp517_i2c_bus, device::axp517::kI2cAddress);
  chip_.xl9535 = std::make_unique<cpp_bus_driver::Xl95x5>(
      bus_.xl9535_i2c_bus, device::xl9535::kI2cAddress);
  chip_.sgm38121 = std::make_unique<cpp_bus_driver::Sgm38121>(
      bus_.sgm38121_i2c_bus, device::sgm38121::kI2cAddress);
  chip_.aw86224 = std::make_unique<cpp_bus_driver::Aw862xx>(
      bus_.aw86224_i2c_bus, device::aw86224::kI2cAddress);
  chip_.hi8561_touch = std::make_unique<cpp_bus_driver::Hi8561Touch>(
      bus_.hi8561_i2c_touch_bus, device::hi8561::kTouchI2cAddress);
  chip_.hi8561_backlight =
      std::make_unique<cpp_bus_driver::Pwm>(gpio::hi8561::kScreenBl);

  bus_.lr1121_radiolib_hal = new RadiolibCppBusDriverHal(
      bus_.lr1121_spi_bus, 10000000, gpio::lr1121::kCs);
  bus_.lr1121_module =
      new Module(bus_.lr1121_radiolib_hal, static_cast<uint32_t>(RADIOLIB_NC),
          static_cast<uint32_t>(gpio::lr1121::kInt),
          static_cast<uint32_t>(gpio::lr1121::kRst),
          static_cast<uint32_t>(gpio::lr1121::kBusy));
  chip_.lr1121 = new LR1121(bus_.lr1121_module);
}

bool TDisplayP4AirDriver::Init(InitMode mode) {
  CreateDrivers();
  const int64_t start_time_us = tool_->GetSystemTimeUs();
  const bool result = InitDrivers(mode);
  const int64_t elapsed_time_us = tool_->GetSystemTimeUs() - start_time_us;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "TDisplayP4AirDriver init finished (mode: %s, result: %s, elapsed: "
      "%lld ms)\n",
      mode == InitMode::kAsync ? "async" : "sync",
      result ? "success" : "failed",
      static_cast<long long>(elapsed_time_us / 1000));
  return result;
}

bool TDisplayP4AirDriver::InitDrivers(InitMode mode) {
  bool result = true;

  result &= InitPower();
  result &= InitAxp517();
  result &= InitXl9535();
  result &= ConfigXl9535();
  result &= InitSgm38121();

  if (mode == InitMode::kAsync) {
    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4AirDriver*>(arg);
                     if (self->InitScreen()) {
                       self->InitTouch();
                       self->InitScreenBacklight();
                     }
                     vTaskDelete(NULL);
                   },
                   "ScreenTask", 4096, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4AirDriver*>(arg);
                     self->InitAw86224();
                     vTaskDelete(NULL);
                   },
                   "InitAw86224Task", 4096, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4AirDriver*>(arg);
                     self->InitLr1121();
                     vTaskDelete(NULL);
                   },
                   "InitLr1121Task", 4096, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4AirDriver*>(arg);
                     self->InitEs8389();
                     self->ConfigEs8389();
                     vTaskDelete(NULL);
                   },
                   "InitConfigEs8389Task", 4096, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4AirDriver*>(arg);
                     self->InitNrf9151();
                     vTaskDelete(NULL);
                   },
                   "InitNrf9151Task", 4096, this, 3, NULL) == pdPASS);

  } else {
    result &= InitScreen();
    result &= InitTouch();
    result &= InitScreenBacklight();
    result &= InitAw86224();
    result &= InitLr1121();
    result &= InitEs8389();
    result &= ConfigEs8389();
    result &= InitNrf9151();
  }

  return result;
}

bool TDisplayP4AirDriver::SetSleep(SleepLevel level, bool enable) {
  bool result = true;

  switch (level) {
    case SleepLevel::kChipSleep:
      if (enable) {
        if (status_.hi8561.init_flag) {
          result &= chip_.hi8561->SetScreenOff(true);
          result &= chip_.hi8561->SetSleep(true);
        }
      } else if (status_.hi8561.init_flag) {
        result &= chip_.hi8561->SetSleep(false);
        result &= chip_.hi8561->SetScreenOff(false);
      }
      break;

    case SleepLevel::kPowerOff:
      if (enable) {
        result &= DeinitScreenBacklight();
        result &= DeinitTouch();
        result &= DeinitScreen();

        if (status_.xl9535.init_flag) {
          result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLr1121PowerEn, 0);
          result &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
          result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 0);
          result &= chip_.xl9535->GpioWrite(gpio::xl9535::kNrf9151En, 0);
          result &= chip_.xl9535->GpioWrite(gpio::xl9535::kNs4150En, 0);
        }
        result &= tool_->GpioWrite(gpio::power::kEnable3v3, 0);
      } else {
        result &= tool_->GpioWrite(gpio::power::kEnable3v3, 1);
        result &= ConfigXl9535();
        result &= InitScreen();
        result &= InitTouch();
        result &= InitScreenBacklight();
      }
      break;

    default:
      break;
  }

  return result;
}

bool TDisplayP4AirDriver::InitPower() {
  bool result = true;

  result &= tool_->SetGpioMode(
      gpio::power::kEnable3v3, cpp_bus_driver::Tool::GpioMode::kOutput);
  result &= tool_->GpioWrite(gpio::power::kEnable3v3, 1);

  result &= InitLdoPower(3, 2500);
  result &= InitLdoPower(4, 3300);
  return result;
}

bool TDisplayP4AirDriver::InitAxp517() {
  if (!chip_.axp517->Init()) {
    status_.axp517.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitAxp517 failed\n");
    return false;
  }

  status_.axp517.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitAxp517 success\n");
  return true;
}

bool TDisplayP4AirDriver::InitXl9535() {
  if (!chip_.xl9535->Init()) {
    status_.xl9535.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitXl9535 failed\n");
    return false;
  }

  status_.xl9535.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitXl9535 success\n");
  return true;
}

bool TDisplayP4AirDriver::ConfigXl9535() {
  if (!status_.xl9535.init_flag) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigXl9535 failed\n");
    return false;
  }

  bool result = true;
  constexpr auto kOutput = cpp_bus_driver::Xl95x5::Mode::kOutput;

  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kSdPowerEn, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kNrf9151En, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kBhi260apRst, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kAdl161Rst, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kLr1121PowerEn, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kUsbPhyPowerEn, kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kEsp32p4Esp32c5UartSwitch, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kEsp32c5En, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kTouchRst, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kScreenRst, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kEsp32c5Boot, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kLed1, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kNs4150En, kOutput);

  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kNrf9151En, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kUsbPhyPowerEn, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32p4Esp32c5UartSwitch, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLed1, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kNs4150En, 1);

  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kScreenRst, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kBhi260apRst, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLr1121PowerEn, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAdl161Rst, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 1);
  tool_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kScreenRst, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kBhi260apRst, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLr1121PowerEn, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAdl161Rst, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
  tool_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kScreenRst, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kBhi260apRst, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLr1121PowerEn, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAdl161Rst, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 1);
  tool_->DelayMs(120);

  if (!result) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigXl9535 failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::EnterEsp32c5DownloadMode() {
  if (!status_.xl9535.init_flag) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "EnterEsp32c5DownloadMode failed\n");
    return false;
  }

  bool result = true;

  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 1);
  tool_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 0);
  tool_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 1);
  tool_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 1);

  if (!result) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "EnterEsp32c5DownloadMode failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::SetUartTarget(UartTarget target) {
  if (!status_.xl9535.init_flag) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "SetUartTarget failed\n");
    return false;
  }

  bool result = true;
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32p4Esp32c5UartSwitch,
      target == UartTarget::kEsp32c5 ? 1 : 0);

  if (!result) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "SetUartTarget failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::InitSgm38121() {
  if (!chip_.sgm38121->Init()) {
    status_.sgm38121.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitSgm38121 failed\n");
    return false;
  }

  bool result = true;
#if defined(CONFIG_CAMERA_TYPE_SC2336)
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1,
      cpp_bus_driver::Sgm38121::Status::kOn);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2,
      cpp_bus_driver::Sgm38121::Status::kOn);
#elif defined(CONFIG_CAMERA_TYPE_OV2710)
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 3000);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1,
      cpp_bus_driver::Sgm38121::Status::kOn);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1,
      cpp_bus_driver::Sgm38121::Status::kOn);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2,
      cpp_bus_driver::Sgm38121::Status::kOn);
#elif defined(CONFIG_CAMERA_TYPE_OV5645)
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1,
      cpp_bus_driver::Sgm38121::Status::kOn);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1,
      cpp_bus_driver::Sgm38121::Status::kOn);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2,
      cpp_bus_driver::Sgm38121::Status::kOn);
#endif

  status_.sgm38121.init_flag = result;
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSgm38121 success\n");
  } else {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitSgm38121 failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::InitScreen() {
  screen_info_ = kDefaultScreenInfo;
  const auto& screen = screen_info();

  bus_.screen_mipi_bus = std::make_shared<cpp_bus_driver::HardwareMipi>(
      screen.width, screen.height, screen.mipi_dsi_hsync, screen.mipi_dsi_hbp,
      screen.mipi_dsi_hfp, screen.mipi_dsi_vsync, screen.mipi_dsi_vbp,
      screen.mipi_dsi_vfp, screen.data_lane_num,
      [](int bits_per_pixel) -> cpp_bus_driver::HardwareMipi::ColorFormat {
        switch (bits_per_pixel) {
          case 16:
            return cpp_bus_driver::HardwareMipi::ColorFormat::kRgb565;
          case 24:
            return cpp_bus_driver::HardwareMipi::ColorFormat::kRgb888;
          default:
            LogMessage(
                LogLevel::kInfo, __FILE__, __LINE__, "Value out of range\n");
            return cpp_bus_driver::HardwareMipi::ColorFormat::kRgb565;
        }
      }(screen.bits_per_pixel));
  chip_.hi8561 = std::make_unique<cpp_bus_driver::Hi8561>(bus_.screen_mipi_bus);

  return InitHi8561();
}

bool TDisplayP4AirDriver::DeinitScreen() {
  if (!status_.hi8561.init_flag) {
    return true;
  }

  const bool result = chip_.hi8561->Deinit();
  status_.hi8561.init_flag = !result;
  return result;
}

bool TDisplayP4AirDriver::InitTouch() { return InitHi8561Touch(); }

bool TDisplayP4AirDriver::DeinitTouch() {
  if (!status_.hi8561_touch.init_flag) {
    return true;
  }

  const bool result = chip_.hi8561_touch->Deinit();
  status_.hi8561_touch.init_flag = !result;
  return result;
}

bool TDisplayP4AirDriver::InitScreenBacklight() {
  return InitHi8561Backlight();
}

bool TDisplayP4AirDriver::DeinitScreenBacklight() {
  if (!status_.hi8561_backlight.init_flag) {
    return true;
  }

  const bool result = chip_.hi8561_backlight->Stop(0);
  status_.hi8561_backlight.init_flag = !result;
  return result;
}

bool TDisplayP4AirDriver::InitHi8561() {
  if (chip_.hi8561 == nullptr) {
    status_.hi8561.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitHi8561 failed\n");
    return false;
  }

  const auto& screen = screen_info();
  if (!chip_.hi8561->Init(
          screen.mipi_dsi_dpi_clk_mhz, screen.lane_bit_rate_mbps)) {
    status_.hi8561.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitHi8561 failed\n");
    return false;
  }

  status_.hi8561.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561 success\n");
  return true;
}

bool TDisplayP4AirDriver::InitHi8561Touch() {
  if (!chip_.hi8561_touch->Init()) {
    status_.hi8561_touch.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }

  status_.hi8561_touch.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561Touch success\n");
  return true;
}

bool TDisplayP4AirDriver::InitHi8561Backlight() {
  if (!chip_.hi8561_backlight->Init(ledc_timer_t::LEDC_TIMER_0,
          ledc_channel_t::LEDC_CHANNEL_0, 2000, 0,
          ledc_mode_t::LEDC_LOW_SPEED_MODE)) {
    status_.hi8561_backlight.init_flag = false;
    LogMessage(
        LogLevel::kChip, __FILE__, __LINE__, "InitHi8561Backlight failed\n");
    return false;
  }

  status_.hi8561_backlight.init_flag = true;
  LogMessage(
      LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561Backlight success\n");
  return true;
}

bool TDisplayP4AirDriver::InitAw86224() {
  if (!chip_.aw86224->Init(500000)) {
    status_.aw86224.init_flag = false;
    status_.aw86224.ram_waveform_info =
        cpp_bus_driver::Aw862xx::RamWaveformInfo();
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitAw86224 failed\n");
    return false;
  }

  const uint32_t detected_f0 = chip_.aw86224->GetF0Detection();
  if (detected_f0 == 0 || detected_f0 == static_cast<uint32_t>(-1)) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "Aw86224 F0 reference read failed\n");
  } else {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "Aw86224 F0 reference: %u.%uHz\n",
        static_cast<unsigned int>(detected_f0 / 10),
        static_cast<unsigned int>(detected_f0 % 10));
  }

  const bool result = chip_.aw86224->InitRamMode(
      cpp_bus_driver::Aw862xx::RamWaveformLibrary::kRam12k041230_235);
  status_.aw86224.init_flag = result;
  status_.aw86224.ram_waveform_info =
      cpp_bus_driver::Aw862xx::GetRamWaveformInfo(
          cpp_bus_driver::Aw862xx::RamWaveformLibrary::kRam12k041230_235);
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "InitAw86224 success (RAM library: %s)\n",
        status_.aw86224.ram_waveform_info.name);
  } else {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitAw86224 failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::InitEs8389() {
  if ((bus_.xl9535_i2c_bus == nullptr) || (bus_.es8389_i2s_bus == nullptr)) {
    status_.es8389.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  i2c_master_bus_handle_t i2c_bus_handle = bus_.xl9535_i2c_bus->bus_handle();
  if (i2c_bus_handle == nullptr) {
    status_.es8389.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  audio_codec_i2c_cfg_t i2c_cfg = {
      .port = static_cast<uint8_t>(I2C_NUM_1),
      .addr = static_cast<uint8_t>(device::es8389::kI2cAddress << 1),
      .bus_handle = i2c_bus_handle,
  };
  es8389_ctrl_if_ = audio_codec_new_i2c_ctrl(&i2c_cfg);

  bool result = (es8389_ctrl_if_ != nullptr);
  result &= bus_.es8389_i2s_bus->Init(
      [](int value) -> i2s_mclk_multiple_t {
        switch (value) {
          case 128:
            return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_128;
          case 192:
            return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_192;
          case 256:
            return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_256;
          case 384:
            return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_384;
          case 512:
            return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_512;
          case 576:
            return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_576;
          case 768:
            return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_768;
          case 1024:
            return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_1024;
          case 1152:
            return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_1152;
          default:
            LogMessage(
                LogLevel::kInfo, __FILE__, __LINE__, "Value out of range\n");
            return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_256;
        }
      }(device::es8389::kMclkMultiple),
      device::es8389::kSampleRate,
      [](int value) -> i2s_data_bit_width_t {
        switch (value) {
          case 8:
            return i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_8BIT;
          case 16:
            return i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_16BIT;
          case 24:
            return i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_24BIT;
          case 32:
            return i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_32BIT;
          default:
            LogMessage(
                LogLevel::kInfo, __FILE__, __LINE__, "Value out of range\n");
            return i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_16BIT;
        }
      }(device::es8389::kBitsPerSample));
  if (!result) {
    status_.es8389.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  audio_codec_i2s_cfg_t i2s_cfg = {
      .port = static_cast<uint8_t>(bus_.es8389_i2s_bus->port()),
      .rx_handle = bus_.es8389_i2s_bus->rx_handle(),
      .tx_handle = bus_.es8389_i2s_bus->tx_handle(),
      .clk_src = static_cast<int>(I2S_CLK_SRC_DEFAULT),
  };
  es8389_data_if_ = audio_codec_new_i2s_data(&i2s_cfg);
  es8389_gpio_if_ = audio_codec_new_gpio();

  es8389_codec_cfg_t codec_cfg = {
      .ctrl_if = es8389_ctrl_if_,
      .gpio_if = es8389_gpio_if_,
      .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH,
      .pa_pin = -1,
      .pa_reverted = false,
      .master_mode = false,
      .use_mclk = true,
      .digital_mic = false,
      .invert_mclk = false,
      .invert_sclk = false,
      .hw_gain =
          {
              .pa_voltage = 3.3f,
              .codec_dac_voltage = 3.3f,
              .pa_gain = 0.0f,
          },
      .no_dac_ref = false,
      .mclk_div = device::es8389::kMclkMultiple,
  };
  es8389_codec_if_ = es8389_codec_new(&codec_cfg);

  esp_codec_dev_cfg_t output_dev_cfg = {
      .dev_type = ESP_CODEC_DEV_TYPE_OUT,
      .codec_if = es8389_codec_if_,
      .data_if = es8389_data_if_,
  };
  es8389_output_codec_dev_ = esp_codec_dev_new(&output_dev_cfg);

  esp_codec_dev_cfg_t input_dev_cfg = {
      .dev_type = ESP_CODEC_DEV_TYPE_IN,
      .codec_if = es8389_codec_if_,
      .data_if = es8389_data_if_,
  };
  es8389_input_codec_dev_ = esp_codec_dev_new(&input_dev_cfg);

  result &= (es8389_data_if_ != nullptr) && (es8389_gpio_if_ != nullptr) &&
            (es8389_codec_if_ != nullptr) &&
            (es8389_input_codec_dev_ != nullptr) &&
            (es8389_output_codec_dev_ != nullptr);
  status_.es8389.init_flag = result;
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitEs8389 success\n");
  } else {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitEs8389 failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::ConfigEs8389() {
  if (!status_.es8389.init_flag || (es8389_input_codec_dev_ == nullptr) ||
      (es8389_output_codec_dev_ == nullptr)) {
    status_.es8389.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigEs8389 failed\n");
    return false;
  }

  esp_codec_dev_sample_info_t output_sample_info = {
      .bits_per_sample = device::es8389::kBitsPerSample,
      .channel = device::es8389::kChannel,
      .channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0) |
                      ESP_CODEC_DEV_MAKE_CHANNEL_MASK(1),
      .sample_rate = device::es8389::kSampleRate,
      .mclk_multiple = device::es8389::kMclkMultiple,
  };

  esp_codec_dev_sample_info_t input_sample_info = {
      .bits_per_sample = device::es8389::kBitsPerSample,
      .channel = device::es8389::kChannel,
      .channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0) |
                      ESP_CODEC_DEV_MAKE_CHANNEL_MASK(1),
      .sample_rate = device::es8389::kSampleRate,
      .mclk_multiple = device::es8389::kMclkMultiple,
  };

  bool result = true;
  int ret = esp_codec_dev_open(es8389_output_codec_dev_, &output_sample_info);
  if (ret != ESP_CODEC_DEV_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "esp_codec_dev_open output failed (error code: %#X)\n", ret);
    result = false;
  }

  ret = esp_codec_dev_open(es8389_input_codec_dev_, &input_sample_info);
  if (ret != ESP_CODEC_DEV_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "esp_codec_dev_open input failed (error code: %#X)\n", ret);
    result = false;
  }

  ret = esp_codec_dev_set_out_vol(es8389_output_codec_dev_, 100);
  if (ret != ESP_CODEC_DEV_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "esp_codec_dev_set_out_vol failed (error code: %#X)\n", ret);
    result = false;
  }

  ret = esp_codec_dev_set_in_gain(es8389_input_codec_dev_, 20.0f);
  if (ret != ESP_CODEC_DEV_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "esp_codec_dev_set_in_gain failed (error code: %#X)\n", ret);
    result = false;
  }

  status_.es8389.init_flag = result;
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "ConfigEs8389 success\n");
  } else {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigEs8389 failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::InitLr1121() {
  bool result = true;

  int16_t ret = chip_.lr1121->begin(
      2450.0, 125.0, 12, 7, RADIOLIB_LR11X0_LORA_SYNC_WORD_PRIVATE, 13, 8);
  if (ret != RADIOLIB_ERR_NONE) {
    status_.lr1121.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "InitLr1121 failed (error code: %d)\n", ret);
    return false;
  }

  const uint32_t rf_switch_dio_pins[] = {
      RADIOLIB_LR11X0_DIO5, RADIOLIB_LR11X0_DIO6, RADIOLIB_NC, RADIOLIB_NC,
      RADIOLIB_NC,
  };
  const Module::RfSwitchMode_t rf_switch_table[] = {
      {LR11x0::MODE_STBY, {0, 0}},
      {LR11x0::MODE_RX, {0, 1}},
      {LR11x0::MODE_TX, {0, 0}},
      {LR11x0::MODE_TX_HP, {1, 0}},
      {LR11x0::MODE_TX_HF, {0, 0}},
      {LR11x0::MODE_GNSS, {0, 0}},
      {LR11x0::MODE_WIFI, {0, 0}},
      END_OF_MODE_TABLE,
  };
  chip_.lr1121->setRfSwitchTable(rf_switch_dio_pins, rf_switch_table);

  status_.lr1121.init_flag = result;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitLr1121 success\n");
  return result;
}

bool TDisplayP4AirDriver::InitNrf9151() {
  bool result = true;
  result &= bus_.nrf9151_uart_bus->Init(device::nrf9151::kDefaultBaudRate);

  status_.nrf9151.init_flag = result;
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitNrf9151 success\n");
  } else {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitNrf9151 failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::InitSpiffs(
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
      LogMessage(LogLevel::kChip, __FILE__, __LINE__,
          "Failed to mount or format filesystem (error code: %#X)\n", result);
    } else if (result == ESP_ERR_NOT_FOUND) {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__,
          "Failed to find spiffs partition (error code: %#X)\n", result);
    } else {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__,
          "Failed to initialize spiffs (error code: %#X)\n", result);
    }
    return false;
  }

  size_t total = 0;
  size_t used = 0;
  result = esp_spiffs_info(conf.partition_label, &total, &used);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "Failed to get spiffs partition information (error code: %#X). "
        "formatting...\n",
        result);
    esp_spiffs_format(conf.partition_label);
    return false;
  }

  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "Partition size: total: %zu bytes, used: %zu bytes\n", total, used);

  if (used > total) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "Number of used bytes cannot be larger than total performing "
        "esp_spiffs_check\n");
    result = esp_spiffs_check(conf.partition_label);
    if (result != ESP_OK) {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__,
          "esp_spiffs_check failed (error code: %#X)\n", result);
      return false;
    }

    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "esp_spiffs_check success\n");
  }

  spiffs_conf = conf;
  return true;
}

bool TDisplayP4AirDriver::InitSdmmc(const char* base_path, int max_freq_khz) {
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024,
      .disk_status_check_enable = false,
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
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "esp_vfs_fat_sdmmc_mount failed (error code: %#X)\n", result);
    status_.sd_card.init_flag = false;
    return false;
  }

  sdmmc_card_print_info(stdout, card);
  status_.sd_card.init_flag = true;
  return true;
}

bool TDisplayP4AirDriver::InitSdspi(
    const char* base_path, spi_host_device_t host_id, int max_freq_khz) {
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
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "spi_bus_initialize failed (error code: %#X)\n", result);
    status_.sd_card.init_flag = false;
    return false;
  }

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.host_id = host_id;
  slot_config.gpio_cs = static_cast<gpio_num_t>(gpio::sd::kCs);

  sdmmc_card_t* card = nullptr;
  result = esp_vfs_fat_sdspi_mount(
      base_path, &host, &slot_config, &mount_config, &card);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "esp_vfs_fat_sdspi_mount failed (error code: %#X)\n", result);
    status_.sd_card.init_flag = false;
    return false;
  }

  sdmmc_card_print_info(stdout, card);
  status_.sd_card.init_flag = true;
  return true;
}

bool InitSpiffs(const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf) {
  return TDisplayP4AirDriver::GetInstance().InitSpiffs(base_path, spiffs_conf);
}

bool InitSdmmc(const char* base_path, int max_freq_khz) {
  return TDisplayP4AirDriver::GetInstance().InitSdmmc(base_path, max_freq_khz);
}

bool InitSdspi(
    const char* base_path, spi_host_device_t host_id, int max_freq_khz) {
  return TDisplayP4AirDriver::GetInstance().InitSdspi(
      base_path, host_id, max_freq_khz);
}

}  // namespace lilygo_device_driver
