/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:51:14
 * @LastEditTime: 2026-05-19 09:43:08
 * @License: GPL 3.0
 */
#include "t_display_p4_driver.h"

#include <cstdint>

namespace lilygo_device_driver {
namespace gpio = t_display_p4::gpio;
namespace device = t_display_p4::device;
namespace keyboard_gpio = t_display_p4::keyboard::gpio;
namespace keyboard_device = t_display_p4::keyboard::device;
namespace {

constexpr uint16_t kBq27220BatteryCapacityMah = 1000;
using ScreenDeviceInfo = device::ScreenDeviceInfo;
using ScreenType = device::ScreenType;

constexpr ScreenDeviceInfo kHi8561ScreenDeviceInfo = {
    .type = ScreenType::kHi8561,
    .name = "hi8561",
    .width = device::hi8561::kScreenWidth,
    .height = device::hi8561::kScreenHeight,
    .bits_per_pixel = device::screen::kBitsPerPixel,
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

constexpr ScreenDeviceInfo kRm69a10ScreenDeviceInfo = {
    .type = ScreenType::kRm69a10,
    .name = "rm69a10",
    .width = device::rm69a10::kScreenWidth,
    .height = device::rm69a10::kScreenHeight,
    .bits_per_pixel = device::screen::kBitsPerPixel,
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

constexpr ScreenDeviceInfo kScreenDeviceInfoRegistry[] = {
    kHi8561ScreenDeviceInfo,
    kRm69a10ScreenDeviceInfo,
};

constexpr const ScreenDeviceInfo* kDefaultScreenDeviceInfo =
    &kHi8561ScreenDeviceInfo;

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
const ScreenDeviceInfo* FindScreenDeviceInfo(ScreenType type) {
  for (const ScreenDeviceInfo& info : kScreenDeviceInfoRegistry) {
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
const ScreenDeviceInfo* ScreenInfoOrDefault(ScreenType type) {
  const auto* info = FindScreenDeviceInfo(type);
  return info == nullptr ? kDefaultScreenDeviceInfo : info;
}

}  // namespace

TDisplayP4Driver& TDisplayP4Driver::GetInstance() {
  static TDisplayP4Driver* instance = new TDisplayP4Driver();
  return *instance;
}

const device::ScreenDeviceInfo& TDisplayP4Driver::screen_info() const {
  return *(screen_info_ == nullptr ? kDefaultScreenDeviceInfo : screen_info_);
}

device::ScreenType TDisplayP4Driver::screen_type() const {
  return screen_info().type;
}

void TDisplayP4Driver::CreateDrivers() {
  tool_ = std::make_unique<cpp_bus_driver::Tool>();

  bus_.xl9535_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      gpio::i2c::kPort1Sda, gpio::i2c::kPort1Scl, I2C_NUM_0);
  bus_.sgm38121_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      gpio::i2c::kPort2Sda, gpio::i2c::kPort2Scl, I2C_NUM_1);
  bus_.sx1262_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(gpio::spi::kPort1Mosi,
          gpio::spi::kPort1Sclk, gpio::spi::kPort1Miso, SPI2_HOST, 0);

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

  bus_.icm20948_i2c_bus = std::make_unique<TwoWire>(1);

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

  chip_.icm20948 = std::make_unique<ICM20948_WE>(
      bus_.icm20948_i2c_bus.get(), device::icm20948::kI2cAddress);

  chip_.l76k = std::make_unique<cpp_bus_driver::L76k>(
      bus_.l76k_uart_bus, [this](bool value) -> bool {
        return chip_.xl9535->GpioWrite(gpio::xl9535::kGpsWakeUp,
            static_cast<cpp_bus_driver::Xl95x5::Value>(value));
      });

  chip_.sx1262 = std::make_unique<cpp_bus_driver::Sx126x>(bus_.sx1262_spi_bus,
      cpp_bus_driver::Sx126x::ChipType::kSx1262, gpio::sx1262::kBusy,
      gpio::sx1262::kCs);

  bus_.xl9555_i2c_bus = std::make_shared<cpp_bus_driver::SoftwareI2c>(
      keyboard_gpio::xl9555::kSda, keyboard_gpio::xl9555::kScl);
  bus_.tca8418_i2c_bus = std::make_shared<cpp_bus_driver::SoftwareI2c>(
      keyboard_gpio::tca8418::kSda, keyboard_gpio::tca8418::kScl);

  bus_.cc1101_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(bus_.sx1262_spi_bus, 0);
  bus_.nrf24l01_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(bus_.sx1262_spi_bus, 0);

  bus_.cc1101_radiolib_hal = new RadiolibCppBusDriverHal(
      bus_.cc1101_spi_bus, 10000000, keyboard_gpio::t_mix_rf::cc1101::kCs);
  bus_.nrf24l01_radiolib_hal = new RadiolibCppBusDriverHal(
      bus_.nrf24l01_spi_bus, 10000000, keyboard_gpio::t_mix_rf::nrf24l01::kCs);

  bus_.cc1101_module = new Module(bus_.cc1101_radiolib_hal,
      static_cast<uint32_t>(RADIOLIB_NC), static_cast<uint32_t>(RADIOLIB_NC),
      static_cast<uint32_t>(RADIOLIB_NC),
      keyboard_gpio::t_mix_rf::cc1101::kBusy);

  bus_.nrf24l01_module =
      new Module(bus_.nrf24l01_radiolib_hal, static_cast<uint32_t>(RADIOLIB_NC),
          static_cast<uint32_t>(keyboard_gpio::t_mix_rf::nrf24l01::kInt),
          static_cast<uint32_t>(keyboard_gpio::t_mix_rf::nrf24l01::kCe),
          static_cast<uint32_t>(RADIOLIB_NC));

  chip_.xl9555 = std::make_unique<cpp_bus_driver::Xl95x5>(
      bus_.xl9555_i2c_bus, keyboard_device::xl9555::kI2cAddress);
  chip_.tca8418 = std::make_unique<cpp_bus_driver::Tca8418>(
      bus_.tca8418_i2c_bus, keyboard_device::tca8418::kI2cAddress);
  chip_.tca8418_backlight =
      std::make_unique<cpp_bus_driver::Pwm>(keyboard_gpio::tca8418::kBl);

  chip_.cc1101 = new CC1101(bus_.cc1101_module);
  chip_.nrf24l01 = new nRF24(bus_.nrf24l01_module);
}

bool TDisplayP4Driver::DetectScreen() {
  status_.gt9895.init_flag = false;

  if (chip_.gt9895 != nullptr && chip_.gt9895->Init()) {
    screen_info_ = ScreenInfoOrDefault(device::ScreenType::kRm69a10);
    status_.gt9895.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "Auto detected T-Display-P4 screen: %s\n", screen_info_->name);
    return true;
  }

  if (bus_.gt9895_i2c_touch_bus != nullptr) {
    bus_.gt9895_i2c_touch_bus->Deinit(false);
  }
  screen_info_ = ScreenInfoOrDefault(device::ScreenType::kHi8561);
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "Auto detected T-Display-P4 screen: %s\n", screen_info_->name);
  return true;
}

void TDisplayP4Driver::CreateSelectedScreenDrivers() {
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
      break;
    case device::ScreenType::kRm69a10:
      chip_.rm69a10 =
          std::make_unique<cpp_bus_driver::Rm69a10>(bus_.screen_mipi_bus);
      break;
    default:
      break;
  }
}

bool TDisplayP4Driver::InitSelectedScreen() {
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      return InitHi8561();
    case device::ScreenType::kRm69a10:
      return InitRm69a10();
    default:
      return false;
  }
}

bool TDisplayP4Driver::InitSelectedTouchAndBacklight() {
  switch (screen_type()) {
    case device::ScreenType::kHi8561: {
      const bool touch_result = InitHi8561Touch();
      const bool backlight_result = InitHi8561Backlight();
      return touch_result && backlight_result;
    }
    case device::ScreenType::kRm69a10:
      return status_.gt9895.init_flag || InitGt9895();
    default:
      return false;
  }
}

bool TDisplayP4Driver::InitKeyboardDevices(InitMode mode) {
  keyboard_connected_ = false;
  status_.xl9555.init_flag = false;
  status_.tca8418.init_flag = false;
  status_.tca8418_backlight.init_flag = false;
  status_.cc1101.init_flag = false;
  status_.nrf24l01.init_flag = false;

  if (!InitXl9555()) {
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "Keyboard device not connected\n");
    return false;
  }

  keyboard_connected_ = true;
  bool result = ConfigXl9555();
  if (!result) {
    return false;
  }

  tool_->SetGpioMode(keyboard_gpio::t_mix_rf::cc1101::kCs,
      cpp_bus_driver::Tool::GpioMode::kOutput);
  tool_->SetGpioMode(keyboard_gpio::t_mix_rf::nrf24l01::kCs,
      cpp_bus_driver::Tool::GpioMode::kOutput);
  tool_->SetGpioMode(keyboard_gpio::t_mix_rf::st25r3916::kCs,
      cpp_bus_driver::Tool::GpioMode::kOutput);
  tool_->GpioWrite(keyboard_gpio::t_mix_rf::cc1101::kCs, 1);
  tool_->GpioWrite(keyboard_gpio::t_mix_rf::nrf24l01::kCs, 1);
  tool_->GpioWrite(keyboard_gpio::t_mix_rf::st25r3916::kCs, 1);

  tool_->SetGpioMode(keyboard_gpio::t_mix_rf::cc1101::kBusy,
      cpp_bus_driver::Tool::GpioMode::kInput,
      cpp_bus_driver::Tool::GpioStatus::kPulldown);

  switch (mode) {
    case InitMode::kAsync:
      result &= (xTaskCreate(
                     [](void* arg) {
                       auto self = static_cast<TDisplayP4Driver*>(arg);
                       self->InitTca8418();
                       vTaskDelete(NULL);
                     },
                     "InitTca8418Task", 4096, this, 3, NULL) == pdPASS);
      result &= (xTaskCreate(
                     [](void* arg) {
                       auto self = static_cast<TDisplayP4Driver*>(arg);
                       self->InitTca8418Backlight();
                       vTaskDelete(NULL);
                     },
                     "Tca8418BlTask", 4096, this, 3, NULL) == pdPASS);
      result &= (xTaskCreate(
                     [](void* arg) {
                       auto self = static_cast<TDisplayP4Driver*>(arg);
                       self->InitCc1101();
                       vTaskDelete(NULL);
                     },
                     "InitCc1101Task", 4096, this, 3, NULL) == pdPASS);
      result &= (xTaskCreate(
                     [](void* arg) {
                       auto self = static_cast<TDisplayP4Driver*>(arg);
                       self->InitNrf24l01();
                       vTaskDelete(NULL);
                     },
                     "InitNrf24Task", 4096, this, 3, NULL) == pdPASS);
      break;
    case InitMode::kSync:
      result &= InitTca8418();
      result &= InitTca8418Backlight();
      result &= InitCc1101();
      result &= InitNrf24l01();
      break;
    default:
      result = false;
      break;
  }

  return result;
}

bool TDisplayP4Driver::InitDrivers(InitMode mode) {
  bool result = true;

  InitXl9535();
  InitPower();
  result &= ConfigXl9535();

  InitSgm38121();

  result &= DetectScreen();
  CreateSelectedScreenDrivers();

  result &= bus_.icm20948_i2c_bus->set_bus_handle(
      bus_.sgm38121_i2c_bus->bus_handle());
  result &=
      bus_.icm20948_i2c_bus->begin(gpio::icm20948::kSda, gpio::icm20948::kScl);

  if (mode == InitMode::kAsync) {
    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4Driver*>(arg);
                     if (self->InitSelectedScreen()) {
                       self->InitSelectedTouchAndBacklight();
                     }
                     vTaskDelete(NULL);
                   },
                   "ScreenTask", 4096, this, 3, NULL) == pdPASS);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4Driver*>(arg);
                     self->InitBq27220();
                     vTaskDelete(NULL);
                   },
                   "InitBq27220Task", 2048, this, 3, NULL) == pdPASS);

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
                     self->InitEs8311();
                     self->ConfigEs8311();
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
                     self->InitSx1262();
                     vTaskDelete(NULL);
                   },
                   "InitSx1262Task", 4096, this, 3, NULL) == pdPASS);

    result &= InitKeyboardDevices(InitMode::kAsync);

    result &= (xTaskCreate(
                   [](void* arg) {
                     auto self = static_cast<TDisplayP4Driver*>(arg);
                     self->InitSdmmc(device::sd::kBasePath, SDMMC_FREQ_52M);
                     vTaskDelete(NULL);
                   },
                   "InitSdmmcTask", 4096, this, 3, NULL) == pdPASS);
  } else {
    if (InitSelectedScreen()) {
      InitSelectedTouchAndBacklight();
    }

    InitBq27220();
    InitPcf8563();
    InitAw86224();
    InitEs8311();
    result &= ConfigEs8311();
    InitL76k();
    InitIcm20948();
    InitSx1262();

    InitKeyboardDevices();

    InitSdmmc(device::sd::kBasePath, SDMMC_FREQ_52M);

    result &= status_.xl9535.init_flag;
    result &= status_.sgm38121.init_flag;

    switch (screen_type()) {
      case device::ScreenType::kHi8561:
        result &= status_.hi8561.init_flag;
        result &= status_.hi8561_touch.init_flag;
        break;
      case device::ScreenType::kRm69a10:
        result &= status_.rm69a10.init_flag;
        result &= status_.gt9895.init_flag;
        break;
      default:
        result = false;
        break;
    }

    result &= status_.bq27220.init_flag;
    result &= status_.pcf8563.init_flag;
    result &= status_.aw86224.init_flag;
    result &= status_.es8311.init_flag;
    result &= status_.l76k.init_flag;
    result &= status_.icm20948.init_flag;
    result &= status_.sx1262.init_flag;

    result &= status_.sd_card.init_flag;
  }

  return result;
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

bool TDisplayP4Driver::SetSleep(SleepLevel level, bool enable) {
  bool result = true;

  switch (level) {
    case SleepLevel::kChipSleep:
      if (enable) {
        switch (screen_type()) {
          case device::ScreenType::kHi8561:
            if (status_.hi8561_backlight.init_flag) {
              result &= chip_.hi8561_backlight->Stop(0);
            }
            if (status_.hi8561.init_flag) {
              result &= chip_.hi8561->SetScreenOff(true);
              result &= chip_.hi8561->SetSleep(true);
            }
            break;
          case device::ScreenType::kRm69a10:
            if (status_.rm69a10.init_flag) {
              result &= chip_.rm69a10->SetBrightness(0);
              result &= chip_.rm69a10->SetScreenOff(true);
              result &= chip_.rm69a10->SetSleep(true);
            }
            if (status_.gt9895.init_flag) {
              result &= chip_.gt9895->SetSleep();
            }
            break;
          default:
            break;
        }

        if (status_.es8311.init_flag) {
          cpp_bus_driver::Es8311::PowerStatus ps = {
              .contorl =
                  {
                      .analog_circuits = false,
                      .analog_bias_circuits = false,
                      .analog_adc_bias_circuits = false,
                      .analog_adc_reference_circuits = false,
                      .analog_dac_reference_circuit = false,
                      .internal_reference_circuits = false,
                  },
              .vmid = cpp_bus_driver::Es8311::Vmid::kPowerDown,
          };
          result &= chip_.es8311->SetOutputToHpDrive(false);
          result &= chip_.es8311->SetPgaPower(false);
          result &= chip_.es8311->SetAdcPower(false);
          result &= chip_.es8311->SetDacPower(false);
          result &= chip_.es8311->SetPowerStatus(ps);
        }
        if (status_.icm20948.init_flag) {
          chip_.icm20948->sleep(true);
        }
        if (status_.l76k.init_flag) {
          result &= chip_.l76k->Sleep(true);
        }
        if (status_.sx1262.init_flag) {
          result &= chip_.sx1262->SetStandby(
              cpp_bus_driver::Sx126x::StdbyConfig::kStdbyRc);
          result &= chip_.sx1262->SetSleep(
              cpp_bus_driver::Sx126x::SleepMode::kWarmStart);
        }
        if (status_.sgm38121.init_flag) {
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kDvdd1,
              cpp_bus_driver::Sgm38121::Status::kOff);
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kDvdd2,
              cpp_bus_driver::Sgm38121::Status::kOff);
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kAvdd1,
              cpp_bus_driver::Sgm38121::Status::kOff);
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kAvdd2,
              cpp_bus_driver::Sgm38121::Status::kOff);
        }
        if (status_.cc1101.init_flag) {
          int16_t ret = chip_.cc1101->sleep();
          if (ret != RADIOLIB_ERR_NONE) {
            LogMessage(LogLevel::kChip, __FILE__, __LINE__,
                "cc1101 sleep failed (error code: %d)\n", ret);
            result = false;
          }
        }
        if (status_.nrf24l01.init_flag) {
          int16_t ret = chip_.nrf24l01->sleep();
          if (ret != RADIOLIB_ERR_NONE) {
            LogMessage(LogLevel::kChip, __FILE__, __LINE__,
                "nrf24l01 sleep failed (error code: %d)\n", ret);
            result = false;
          }
        }
        if (status_.tca8418_backlight.init_flag) {
          result &= chip_.tca8418_backlight->Stop(0);
        }

        if (status_.xl9535.init_flag) {
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kEsp32c6En, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kSdEn, cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kPowerEn5v0, cpp_bus_driver::Xl95x5::Value::kLow);
        }
        if (status_.xl9555.init_flag) {
          result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed1,
              cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed2,
              cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed3,
              cpp_bus_driver::Xl95x5::Value::kHigh);
        }
      } else {
        if (status_.xl9535.init_flag) {
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kEsp32c6En, cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kSdEn, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kPowerEn5v0, cpp_bus_driver::Xl95x5::Value::kHigh);
        }

        switch (screen_type()) {
          case device::ScreenType::kHi8561:
            if (status_.hi8561.init_flag) {
              result &= chip_.hi8561->SetSleep(false);
              result &= chip_.hi8561->SetScreenOff(false);
            }
            break;
          case device::ScreenType::kRm69a10:
            if (status_.rm69a10.init_flag) {
              result &= chip_.rm69a10->SetSleep(false);
              result &= chip_.rm69a10->SetScreenOff(false);
            }
            if (status_.gt9895.init_flag) {
              result &= InitGt9895();
            }
            break;
          default:
            break;
        }

        if (status_.es8311.init_flag) {
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
              .vmid =
                  cpp_bus_driver::Es8311::Vmid::kStartUpVmidNormalSpeedCharge,
          };
          result &= chip_.es8311->SetOutputToHpDrive(true);
          result &= chip_.es8311->SetPgaPower(true);
          result &= chip_.es8311->SetAdcPower(true);
          result &= chip_.es8311->SetDacPower(true);
          result &= chip_.es8311->SetPowerStatus(ps);
        }
        if (status_.icm20948.init_flag) {
          chip_.icm20948->sleep(false);
        }
        if (status_.l76k.init_flag) {
          result &= chip_.l76k->Sleep(false);
        }
        if (status_.sx1262.init_flag) {
          // 唤醒
          result &= chip_.sx1262->Wakeup();
        }
        if (status_.sgm38121.init_flag) {
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kDvdd1,
              cpp_bus_driver::Sgm38121::Status::kOn);
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kDvdd2,
              cpp_bus_driver::Sgm38121::Status::kOn);
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kAvdd1,
              cpp_bus_driver::Sgm38121::Status::kOn);
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kAvdd2,
              cpp_bus_driver::Sgm38121::Status::kOn);
        }
        if (status_.cc1101.init_flag) {
          int16_t ret = chip_.cc1101->standby();
          if (ret != RADIOLIB_ERR_NONE) {
            LogMessage(LogLevel::kChip, __FILE__, __LINE__,
                "cc1101 standby failed (error code: %d)\n", ret);
            result = false;
          }
        }
        if (status_.nrf24l01.init_flag) {
          int16_t ret = chip_.nrf24l01->standby();
          if (ret != RADIOLIB_ERR_NONE) {
            LogMessage(LogLevel::kChip, __FILE__, __LINE__,
                "nrf24l01 standby failed (error code: %d)\n", ret);
            result = false;
          }
        }
      }
      break;
    case SleepLevel::kPowerOff:
      if (enable) {
        if (status_.icm20948.init_flag) {
          chip_.icm20948->sleep(true);
        }

        if (status_.l76k.init_flag) {
          result &= chip_.l76k->Sleep(true);
        }

        if (status_.sx1262.init_flag) {
          result &= chip_.sx1262->SetStandby(
              cpp_bus_driver::Sx126x::StdbyConfig::kStdbyRc);
          result &= chip_.sx1262->SetSleep(
              cpp_bus_driver::Sx126x::SleepMode::kWarmStart);
        }

        if (status_.aw86224.init_flag) {
          result &= chip_.aw86224->Deinit();
          status_.aw86224.init_flag = false;
        }
        if (status_.es8311.init_flag) {
          result &= chip_.es8311->Deinit();
          status_.es8311.init_flag = false;
        }
        if (status_.icm20948.init_flag) {
          result &= bus_.icm20948_i2c_bus->end(false);
          status_.icm20948.init_flag = false;
        }

        switch (screen_type()) {
          case device::ScreenType::kHi8561:
            if (status_.hi8561_touch.init_flag) {
              result &= chip_.hi8561_touch->Deinit();
              status_.hi8561_touch.init_flag = false;
            }
            if (status_.hi8561_backlight.init_flag) {
              result &= chip_.hi8561_backlight->Stop(0);
              status_.hi8561_backlight.init_flag = false;
            }
            if (status_.hi8561.init_flag) {
              result &= chip_.hi8561->Deinit();
              status_.hi8561.init_flag = false;
            }
            break;
          case device::ScreenType::kRm69a10:
            if (status_.gt9895.init_flag) {
              result &= chip_.gt9895->Deinit();
              status_.gt9895.init_flag = false;
            }
            if (status_.rm69a10.init_flag) {
              result &= chip_.rm69a10->Deinit();
              status_.rm69a10.init_flag = false;
            }
            break;
          default:
            break;
        }

        if (status_.bq27220.init_flag) {
          result &= chip_.bq27220->Deinit();
          status_.bq27220.init_flag = false;
        }
        if (status_.pcf8563.init_flag) {
          result &= chip_.pcf8563->Deinit();
          status_.pcf8563.init_flag = false;
        }

        if (status_.tca8418_backlight.init_flag) {
          result &= chip_.tca8418_backlight->Stop(0);
          status_.tca8418_backlight.init_flag = false;
        }
        if (status_.tca8418.init_flag) {
          result &= chip_.tca8418->Deinit();
          status_.tca8418.init_flag = false;
        }
        if (status_.xl9555.init_flag) {
          result &= chip_.xl9555->Deinit();
          status_.xl9555.init_flag = false;
          keyboard_connected_ = false;
        }
        if (status_.cc1101.init_flag) {
          result &= bus_.cc1101_spi_bus->Deinit();
          status_.cc1101.init_flag = false;
        }
        if (status_.nrf24l01.init_flag) {
          result &= bus_.nrf24l01_spi_bus->Deinit();
          status_.nrf24l01.init_flag = false;
        }
        if (status_.sx1262.init_flag) {
          result &= chip_.sx1262->Deinit();
          status_.sx1262.init_flag = false;
        }
        if (status_.l76k.init_flag) {
          result &= chip_.l76k->Deinit();
          status_.l76k.init_flag = false;
        }
        if (status_.sgm38121.init_flag) {
          result &= chip_.sgm38121->Deinit();
          status_.sgm38121.init_flag = false;
        }
        if (status_.xl9535.init_flag) {
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kScreenRst, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kEsp32c6En, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kEthernetRst, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kSdEn, cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kPowerEn5v0, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              gpio::xl9535::kPowerEn3v3, cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9535->Deinit();
          status_.xl9535.init_flag = false;
        }

        if (status_.xl9555.init_flag) {
          result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed1,
              cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed2,
              cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kLed3,
              cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfEn,
              cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTca8418Rst,
              cpp_bus_driver::Xl95x5::Value::kLow);
        }
      } else {
        result &= InitDrivers(InitMode::kAsync);
      }
      break;

    default:
      break;
  }

  return result;
}

bool TDisplayP4Driver::InitPower() {
  bool result = true;
  result &= InitLdoPower(3, 2500);
  result &= InitLdoPower(4, 3300);
  return result;
}

bool TDisplayP4Driver::InitBq27220() {
  if (!chip_.bq27220->Init()) {
    status_.bq27220.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitBq27220 failed\n");
    return false;
  } else {
    cpp_bus_driver::Bq27220::CedvProfile battery_profile;
    battery_profile.design_capacity = kBq27220BatteryCapacityMah;
    battery_profile.full_charge_capacity = kBq27220BatteryCapacityMah;
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
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitBq27220 failed\n");
    }
    return result;
  }
}

bool TDisplayP4Driver::InitXl9535() {
  if (!chip_.xl9535->Init()) {
    status_.xl9535.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitXl9535 failed\n");
    return false;
  } else {
    status_.xl9535.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitXl9535 success\n");
    return true;
  }
}

bool TDisplayP4Driver::ConfigXl9535() {
  if (!status_.xl9535.init_flag) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigXl9535 failed\n");
    return false;
  }

  bool result = true;
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kScreenRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kEsp32p4VccaPowerEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kPowerEn5v0, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kPowerEn3v3, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kGpsWakeUp, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kEsp32c6En, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kEthernetRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kSdEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kSx1262Rst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kSky13453Vctl, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kIcm20948Int, cpp_bus_driver::Xl95x5::Mode::kInput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kSx1262Dio1, cpp_bus_driver::Xl95x5::Mode::kInput);

  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kEsp32p4VccaPowerEn, cpp_bus_driver::Xl95x5::Value::kLow);
  // 默认使用RF1天线
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSky13453Vctl, cpp_bus_driver::Xl95x5::Value::kHigh);

  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kPowerEn3v3, cpp_bus_driver::Xl95x5::Value::kLow);
  tool_->DelayMs(100);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kPowerEn3v3, cpp_bus_driver::Xl95x5::Value::kHigh);
  tool_->DelayMs(100);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kPowerEn3v3, cpp_bus_driver::Xl95x5::Value::kLow);
  tool_->DelayMs(200);

  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kScreenRst, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kEsp32c6En, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kEthernetRst, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kGpsWakeUp, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSx1262Rst, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSdEn, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kPowerEn5v0, cpp_bus_driver::Xl95x5::Value::kHigh);
  tool_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kScreenRst, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kEsp32c6En, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kEthernetRst, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kGpsWakeUp, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSx1262Rst, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSdEn, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kPowerEn5v0, cpp_bus_driver::Xl95x5::Value::kLow);
  tool_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kScreenRst, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kEsp32c6En, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kEthernetRst, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kGpsWakeUp, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSx1262Rst, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSdEn, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kPowerEn5v0, cpp_bus_driver::Xl95x5::Value::kHigh);
  tool_->DelayMs(120);

  if (!result) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigXl9535 failed\n");
  }
  return result;
}

bool TDisplayP4Driver::InitSgm38121() {
  if (!chip_.sgm38121->Init()) {
    status_.sgm38121.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitSgm38121 failed\n");
    return false;
  } else {
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
}

bool TDisplayP4Driver::InitHi8561() {
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
  } else {
    status_.hi8561.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitHi8561Touch() {
  if (chip_.hi8561_touch == nullptr) {
    status_.hi8561_touch.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }

  if (!chip_.hi8561_touch->Init()) {
    status_.hi8561_touch.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitHi8561Touch failed\n");
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
        LogLevel::kChip, __FILE__, __LINE__, "InitHi8561Backlight failed\n");
    return false;
  }

  if (!chip_.hi8561_backlight->Init(
          ledc_timer_t::LEDC_TIMER_0, ledc_channel_t::LEDC_CHANNEL_0, 2000)) {
    status_.hi8561_backlight.init_flag = false;
    LogMessage(
        LogLevel::kChip, __FILE__, __LINE__, "InitHi8561Backlight failed\n");
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
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitRm69a10 failed\n");
    return false;
  }

  const auto& screen = screen_info();
  if (!chip_.rm69a10->Init(
          screen.mipi_dsi_dpi_clk_mhz, screen.lane_bit_rate_mbps)) {
    status_.rm69a10.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitRm69a10 failed\n");
    return false;
  } else {
    status_.rm69a10.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitRm69a10 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitGt9895() {
  if (chip_.gt9895 == nullptr) {
    status_.gt9895.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitGt9895 failed\n");
    return false;
  }

  if (!chip_.gt9895->Init()) {
    status_.gt9895.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitGt9895 failed\n");
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
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitPcf8563 failed\n");
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
    status_.aw86224.ram_waveform_selection =
        cpp_bus_driver::Aw862xx::RamWaveformSelection();
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitAw86224 failed\n");
    return false;
  } else {
    cpp_bus_driver::Aw862xx::RamWaveformSelection selection;
    const bool result = chip_.aw86224->InitRamModeByF0(selection);

    status_.aw86224.init_flag = result;
    status_.aw86224.ram_waveform_selection =
        result ? selection : cpp_bus_driver::Aw862xx::RamWaveformSelection();
    if (result) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitAw86224 success\n");
    } else {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitAw86224 failed\n");
    }
    return result;
  }
}

bool TDisplayP4Driver::InitEs8311() {
  if (!chip_.es8311->Init()) {
    status_.es8311.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitEs8311 failed\n");
    return false;
  } else {
    if (!chip_.es8311->Init(device::es8311::kMclkMultiple,
            device::es8311::kSampleRate, device::es8311::kBitsPerSample)) {
      status_.es8311.init_flag = false;
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitEs8311 failed\n");
      return false;
    } else {
      status_.es8311.init_flag = true;
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitEs8311 success\n");
      return true;
    }
  }
}

bool TDisplayP4Driver::ConfigEs8311() {
  if (!status_.es8311.init_flag) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigEs8311 failed\n");
    return false;
  }

  cpp_bus_driver::Es8311::PowerStatus ps = {
      .contorl =
          {
              .analog_circuits = true,                // 开启模拟电�?
              .analog_bias_circuits = true,           // 开启模拟偏置电�?
              .analog_adc_bias_circuits = true,       // 开启模拟ADC偏置电路
              .analog_adc_reference_circuits = true,  // 开启模拟ADC参考电�?
              .analog_dac_reference_circuit = true,   // 开启模拟DAC参考电�?
              .internal_reference_circuits = false,   // 关闭内部参考电�?
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
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigEs8311 failed\n");
  }
  return result;
}

bool TDisplayP4Driver::InitL76k() {
  if (!chip_.l76k->Init()) {
    if (!bus_.l76k_uart_bus->SetBaudRate(115200)) {
      status_.l76k.init_flag = false;
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "SetBaudRate failed\n");
      return false;
    }
    if (!chip_.l76k->Init()) {
      status_.l76k.init_flag = false;
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitL76k failed\n");
      return false;
    } else {
      bool result = true;
      result &=
          chip_.l76k->SetBaudRate(cpp_bus_driver::L76k::BaudRate::kBr115200Bps);
      result &= chip_.l76k->SetUpdateFrequency(
          cpp_bus_driver::L76k::UpdateFreq::kFreq5Hz);
      result &= chip_.l76k->ClearRxBufferData();

      status_.l76k.init_flag = result;
      if (result) {
        LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitL76k success\n");
      } else {
        LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitL76k failed\n");
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

    status_.l76k.init_flag = result;
    if (result) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitL76k success\n");
    } else {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitL76k failed\n");
    }
    return result;
  }
}

bool TDisplayP4Driver::InitIcm20948() {
  if (!chip_.icm20948->init()) {
    if (!chip_.icm20948->initMagnetometer()) {
      status_.icm20948.init_flag = false;
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitIcm20948 failed\n");
      return false;
    } else {
      chip_.icm20948->setAccRange(ICM20948_ACC_RANGE_2G);
      chip_.icm20948->setAccDLPF(ICM20948_DLPF_6);
      chip_.icm20948->setMagOpMode(AK09916_CONT_MODE_20HZ);

      status_.icm20948.init_flag = true;
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitIcm20948 success\n");
      return true;
    }
  } else {
    chip_.icm20948->setAccRange(ICM20948_ACC_RANGE_2G);
    chip_.icm20948->setAccDLPF(ICM20948_DLPF_6);
    chip_.icm20948->setMagOpMode(AK09916_CONT_MODE_20HZ);

    status_.icm20948.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitIcm20948 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitSx1262() {
  if (!chip_.sx1262->Init(10000000)) {
    status_.sx1262.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitSx1262 failed\n");
    return false;
  } else {
    status_.sx1262.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSx1262 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitXl9555() {
  if (!chip_.xl9555->Init()) {
    status_.xl9555.init_flag = false;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitXl9555 failed\n");
    return false;
  } else {
    status_.xl9555.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitXl9555 success\n");
    return true;
  }
}

bool TDisplayP4Driver::ConfigXl9555() {
  if (!status_.xl9555.init_flag) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigXl9555 failed\n");
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
      cpp_bus_driver::Xl95x5::Value::kHigh);  // 关闭led
  result &= chip_.xl9555->GpioWrite(
      keyboard_gpio::xl9555::kLed2, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9555->GpioWrite(
      keyboard_gpio::xl9555::kLed3, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9555->GpioWrite(
      keyboard_gpio::xl9555::kTMixRfEn, cpp_bus_driver::Xl95x5::Value::kHigh);

  result &= SetCc1101RfSwitch(Cc1101RfSwitch::k868_915Mhz);

  result &= chip_.xl9555->GpioWrite(
      keyboard_gpio::xl9555::kTca8418Rst, cpp_bus_driver::Xl95x5::Value::kHigh);
  tool_->DelayMs(10);
  result &= chip_.xl9555->GpioWrite(
      keyboard_gpio::xl9555::kTca8418Rst, cpp_bus_driver::Xl95x5::Value::kLow);
  tool_->DelayMs(10);
  result &= chip_.xl9555->GpioWrite(
      keyboard_gpio::xl9555::kTca8418Rst, cpp_bus_driver::Xl95x5::Value::kHigh);
  tool_->DelayMs(10);

  if (!result) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigXl9555 failed\n");
  }
  return result;
}

bool TDisplayP4Driver::InitTca8418() {
  if (!chip_.tca8418->Init()) {
    status_.tca8418.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitTca8418 failed\n");
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
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitTca8418 failed\n");
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
        LogLevel::kChip, __FILE__, __LINE__, "InitTca8418Backlight failed\n");
    return false;
  } else {
    status_.tca8418_backlight.init_flag = true;
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "InitTca8418Backlight success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitCc1101() {
  int16_t ret = chip_.cc1101->begin();
  if (ret != RADIOLIB_ERR_NONE) {
    status_.cc1101.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "InitCc1101 failed (error code: %d)\n", ret);
    return false;
  } else {
    status_.cc1101.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitCc1101 success\n");
    return true;
  }
}

bool TDisplayP4Driver::SetCc1101RfSwitch(Cc1101RfSwitch rf_switch) {
  if (!status_.xl9555.init_flag) {
    LogMessage(
        LogLevel::kChip, __FILE__, __LINE__, "SetCc1101RfSwitch failed\n");
    return false;
  }

  bool result = true;
  switch (rf_switch) {
    case Cc1101RfSwitch::k315Mhz:
      result &=
          chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch0,
              cpp_bus_driver::Xl95x5::Value::kLow);
      result &=
          chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch1,
              cpp_bus_driver::Xl95x5::Value::kHigh);
      break;
    case Cc1101RfSwitch::k434Mhz:
      result &=
          chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch0,
              cpp_bus_driver::Xl95x5::Value::kHigh);
      result &=
          chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch1,
              cpp_bus_driver::Xl95x5::Value::kHigh);
      break;
    case Cc1101RfSwitch::k868_915Mhz:
      result &=
          chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch0,
              cpp_bus_driver::Xl95x5::Value::kHigh);
      result &=
          chip_.xl9555->GpioWrite(keyboard_gpio::xl9555::kTMixRfCc1101RfSwitch1,
              cpp_bus_driver::Xl95x5::Value::kLow);
      break;

    default:
      result = false;
      break;
  }

  if (!result) {
    LogMessage(
        LogLevel::kChip, __FILE__, __LINE__, "SetCc1101RfSwitch failed\n");
  }
  return result;
}

bool TDisplayP4Driver::InitNrf24l01() {
  int16_t ret = chip_.nrf24l01->begin();
  if (ret != RADIOLIB_ERR_NONE) {
    status_.nrf24l01.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "InitNrf24l01 failed (error code: %d)\n", ret);
    return false;
  } else {
    status_.nrf24l01.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitNrf24l01 success\n");
    return true;
  }
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

  size_t total = 0, used = 0;
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
      "Partition size: total: %d bytes, used: %d bytes\n", total, used);

  if (used > total) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "Number of used bytes cannot be larger than total performing "
        "esp_spiffs_check\n");
    result = esp_spiffs_check(conf.partition_label);
    if (result != ESP_OK) {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__,
          "esp_spiffs_check failed (error code: %#X)\n", result);
      return false;
    } else {
      LogMessage(
          LogLevel::kChip, __FILE__, __LINE__, "esp_spiffs_check success\n");
    }
  }

  spiffs_conf = conf;
  return true;
}

bool TDisplayP4Driver::InitSdmmc(const char* base_path, int max_freq_khz) {
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

bool TDisplayP4Driver::InitSdspi(
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

}  // namespace lilygo_device_driver
