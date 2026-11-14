/*
 * @Description: T-Display-P4-Air 板级设备驱动实现
 */
#include "device/t_display_p4_air/driver.h"

#include <cstdio>

#include "../../core/logger.h"
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "driver/spi_master.h"
#include "chip/esp32p4/spiffs.h"
#include "esp_vfs_fat.h"
#include "firmware/bhi260ap/BHI260AP.fw.h"
#include "sdmmc_cmd.h"
#include "es8389_codec.h"

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
constexpr uint32_t kInitializationShutdownTimeoutMs = 5 * 1000;
constexpr uint8_t kLr2021ExpectedVersionMajor = 0x01;
constexpr uint8_t kLr2021ExpectedVersionMinor = 0x18;
constexpr int kEs8390DacMiscControl2Register = 0x45;
constexpr int kEs8390Dac1OutputInvertMask = 1 << 6;

const ScreenInfo* ScreenInfoForType(ScreenType type) {
  for (const auto& info : kScreenInfoRegistry) {
    if (info.type == type) {
      return &info;
    }
  }
  return kDefaultScreenInfo;
}

}  // namespace

TDisplayP4AirDriver& TDisplayP4AirDriver::GetInstance() {
  static TDisplayP4AirDriver* instance = new TDisplayP4AirDriver();
  return *instance;
}

const device::ScreenInfo& TDisplayP4AirDriver::screen_info() const {
  return *(screen_info_ == nullptr ? kDefaultScreenInfo : screen_info_);
}

void TDisplayP4AirDriver::CreateDrivers() {
  if (platform_hal_ != nullptr) {
    return;
  }
  platform_hal_ = std::make_unique<cpp_bus_driver::PlatformHal>();

  bus_.sgm38121_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c>(
      gpio::sgm38121::kSda, gpio::sgm38121::kScl, I2C_NUM_0);
  bus_.xl9535_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c>(
      gpio::xl9535::kSda, gpio::xl9535::kScl, I2C_NUM_1);

  bus_.axp517_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.xl9535_i2c_bus);
  bus_.rx8130ce_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.xl9535_i2c_bus);
  bus_.hi8561_i2c_touch_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.sgm38121_i2c_bus);
  bus_.bhi260ap_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.sgm38121_i2c_bus);
  bus_.qmc6309_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.xl9535_i2c_bus);
  bus_.aw86224_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.xl9535_i2c_bus);
  bus_.st25r3916_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.xl9535_i2c_bus);

  bus_.es8390_i2s_bus = std::make_shared<cpp_bus_driver::HardwareI2s>(
      gpio::es8390::kAdcData, gpio::es8390::kDacData, gpio::es8390::kWsLrck,
      gpio::es8390::kBclk, gpio::es8390::kMclk, i2s_port_t::I2S_NUM_0,
      cpp_bus_driver::HardwareI2s::DataMode::kInputOutput,
      cpp_bus_driver::HardwareI2s::I2sMode::kStd,
      i2s_clock_src_t::I2S_CLK_SRC_DEFAULT);

  bus_.lr2021_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(gpio::lr2021::kMosi,
          gpio::lr2021::kSclk, gpio::lr2021::kMiso, SPI2_HOST, 0);

  bus_.nrf9151_uart_bus = std::make_shared<cpp_bus_driver::HardwareUart>(
      gpio::nrf9151::kUartRx, gpio::nrf9151::kUartTx, UART_NUM_1,
      gpio::nrf9151::kUartCts, gpio::nrf9151::kUartRts);

  chip_.axp517 = std::make_unique<cpp_bus_driver::Axp517>(
      bus_.axp517_i2c_bus, device::axp517::kI2cAddress);
  chip_.rx8130ce = std::make_unique<cpp_bus_driver::Rx8130ce>(
      bus_.rx8130ce_i2c_bus, device::rx8130ce::kI2cAddress);
  chip_.xl9535 = std::make_unique<cpp_bus_driver::Xl95x5>(
      bus_.xl9535_i2c_bus, device::xl9535::kI2cAddress);
  chip_.sgm38121 = std::make_unique<cpp_bus_driver::Sgm38121>(
      bus_.sgm38121_i2c_bus, device::sgm38121::kI2cAddress);
  chip_.aw86224 = std::make_unique<cpp_bus_driver::Aw862xx>(
      bus_.aw86224_i2c_bus, device::aw86224::kI2cAddress);
  chip_.st25r3916 =
      std::make_unique<stsw_st25rfal002_cpp_bus_driver::St25r3916x>(
          bus_.st25r3916_i2c_bus, gpio::st25r3916::kInt,
          device::st25r3916::kI2cAddress);
  chip_.hi8561_touch = std::make_unique<cpp_bus_driver::Hi8561Touch>(
      bus_.hi8561_i2c_touch_bus, device::hi8561::kTouchI2cAddress);
  cpp_bus_driver::TouchCoordinateTransform gt9895_coordinate_transform;
  gt9895_coordinate_transform.source_width =
      device::gt9895::kRawCoordinateWidth;
  gt9895_coordinate_transform.source_height =
      device::gt9895::kRawCoordinateHeight;
  gt9895_coordinate_transform.target_width =
      static_cast<uint16_t>(device::rm69a10::kScreenWidth);
  gt9895_coordinate_transform.target_height =
      static_cast<uint16_t>(device::rm69a10::kScreenHeight);
  chip_.gt9895 = std::make_unique<cpp_bus_driver::Gt9895>(
      bus_.hi8561_i2c_touch_bus, device::gt9895::kI2cAddress, -1, -1,
      gt9895_coordinate_transform);
  chip_.bhi260ap = std::make_unique<bhi2xy_sensorapi_cpp_bus_driver::Bhi2xy>(
      bus_.bhi260ap_i2c_bus, device::bhi260ap::kI2cAddress);
  chip_.qmc6309 = std::make_unique<SensorQMC6309>();
  chip_.sy7200a = std::make_unique<cpp_bus_driver::Pwm>(gpio::sy7200a::kEn);
  chip_.nrf9151 =
      std::make_unique<cpp_bus_driver::Nrf9151>(bus_.nrf9151_uart_bus);
  chip_.lr2021 =
      std::make_unique<usp_cpp_bus_driver::Lr20xx>(bus_.lr2021_spi_bus,
          gpio::lr2021::kBusy, gpio::lr2021::kCs, [this](bool released) {
            return platform_hal_ != nullptr &&
                   platform_hal_->GpioWrite(gpio::lr2021::kRst, released);
          });
}

bool TDisplayP4AirDriver::Init(InitMode mode) {
  CreateDrivers();
  const int64_t start_time_us = platform_hal_->GetSystemTimeUs();
  const bool result = InitDrivers(mode);
  const int64_t elapsed_time_us =
      platform_hal_->GetSystemTimeUs() - start_time_us;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      "TDisplayP4AirDriver init (mode: %s, result: %s, elapsed: "
      "%lld ms)\n",
      mode == InitMode::kAsync ? "async" : "sync",
      result ? (mode == InitMode::kAsync ? "tasks scheduled" : "success")
             : "failed",
      static_cast<long long>(elapsed_time_us / 1000));
  return result;
}

bool TDisplayP4AirDriver::InitMinimal() {
  CreateDrivers();
  // 初始化失败保留公共电源，允许后续重试。
  return InitMinimalDrivers();
}

bool TDisplayP4AirDriver::InitMinimalDrivers() {
  if (minimal_drivers_initialized_) {
    return true;
  }
  if (!InitPower() || !InitAxp517() || !InitXl9535()) {
    return false;
  }
  minimal_drivers_initialized_ = true;
  return true;
}

bool TDisplayP4AirDriver::InitDrivers(InitMode mode) {
  bool result = InitMinimalDrivers();
  result &= InitSgm38121();
  async_init_manager_.Reset();

  if (mode == InitMode::kAsync) {
    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TDisplayP4AirDriver*>(arg);
          const bool screen_initialized = self->InitScreen();

          if (screen_initialized) {
            self->InitTouch();
            self->InitScreenBacklight();
          }

          // BHI260AP 与触摸控制器共享 I2C，两个初始化流程
          // 必须依次完成，避免并发占用同一条总线。
          // 屏幕已可用后降低任务优先级，避免上传传感器固件时
          // 抢占界面刷新并造成启动动画停顿。
          if (!self->async_init_manager_.stop_requested()) {
            vTaskPrioritySet(nullptr, tskIDLE_PRIORITY);
            self->InitBhi260ap();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitDisplayBhi260apTask", 8192, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TDisplayP4AirDriver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitQmc6309();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitQmc6309Task", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TDisplayP4AirDriver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitAw86224();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitAw86224Task", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TDisplayP4AirDriver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitLr2021();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitLr2021Task", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TDisplayP4AirDriver*>(arg);
          if (!self->async_init_manager_.stop_requested() &&
              !self->InitEs8390()) {
            self->SetNs4150Enabled(false);
          }
          self->async_init_manager_.FinishTask();
        },
        "InitEs8390Task", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TDisplayP4AirDriver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitRx8130ce();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitRx8130ceTask", 4096, this, 1);

  } else {
    const bool screen_initialized = InitScreen();
    result &= screen_initialized;
    if (screen_initialized) {
      result &= InitTouch();
      result &= InitScreenBacklight();
    }

    result &= InitBhi260ap();
    result &= InitQmc6309();

    result &= InitAw86224();
    result &= InitLr2021();

    result &= InitEs8390();
    result &= InitRx8130ce();
  }

  return result;
}

bool TDisplayP4AirDriver::InitAxp517() {
  if (IsAxp517Ready()) {
    return true;
  }
  if (!chip_.axp517->Init()) {
    chip_status_.axp517.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitAxp517 failed\n");
    return false;
  }

  const uint8_t adc_channels =
      static_cast<uint8_t>(cpp_bus_driver::Axp517::AdcChannel::kBatteryVoltage) |
      static_cast<uint8_t>(cpp_bus_driver::Axp517::AdcChannel::kTs) |
      static_cast<uint8_t>(cpp_bus_driver::Axp517::AdcChannel::kChargeCurrent) |
      static_cast<uint8_t>(cpp_bus_driver::Axp517::AdcChannel::kDischargeCurrent) |
      static_cast<uint8_t>(cpp_bus_driver::Axp517::AdcChannel::kDieTemperature);
  const auto* ntc_config = battery_info().ntc_config;
  cpp_bus_driver::Axp517::Status power_status;
  const bool result =
      chip_.axp517->SetAdcChannels(adc_channels) &&
      ntc_config != nullptr &&
      chip_.axp517->ConfigureNtc(*ntc_config) &&
      chip_.axp517->SetJeitaEnable(false) &&
      chip_.axp517->SetBoostEnable(false) &&
      chip_.axp517->SetRbfetForceEnable(false) &&
      chip_.axp517->SetBoostVoltage(5000) &&
      chip_.axp517->GetStatus(power_status) &&
      chip_.axp517->InitTypeC(false, power_status.battery_present) &&
      chip_.axp517->SetTypeCRole(cpp_bus_driver::Axp517::TypeCRole::kSink);
  chip_status_.axp517.init_flag = result;
  if (!result) {
    chip_.axp517->Deinit(false);
  }
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitAxp517 success\n" : "InitAxp517 failed\n");
  return result;
}

bool TDisplayP4AirDriver::InitRx8130ce() {
  if (IsRx8130ceReady()) {
    return true;
  }
  chip_status_.rx8130ce.init_flag = false;
  if (bus_.rx8130ce_i2c_bus == nullptr || chip_.rx8130ce == nullptr ||
      !bus_.rx8130ce_i2c_bus->Init(device::rx8130ce::kI2cFrequencyHz,
          device::rx8130ce::kI2cAddress) ||
      !bus_.rx8130ce_i2c_bus->Probe(device::rx8130ce::kI2cAddress)) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitRx8130ce I2C probe failed\n");
    return false;
  }
  bool result = chip_.rx8130ce->Init(device::rx8130ce::kI2cFrequencyHz);
  if (result) {
    result = chip_.rx8130ce->SetChargerEnabled(true);
  }
  chip_status_.rx8130ce.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitRx8130ce success\n"
             : "InitRx8130ce failed\n");
  return result;
}

bool TDisplayP4AirDriver::InitXl9535() {
  chip_status_.xl9535.init_flag = false;
  if (chip_.xl9535 == nullptr || !chip_.xl9535->Init()) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitXl9535 failed\n");
    return false;
  }

  constexpr auto kOutput = cpp_bus_driver::Xl95x5::Mode::kOutput;
  // 先预装关闭和复位状态，再切换输出方向，避免默认高电平产生无效脉冲。
  bool result = true;
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kUsbPhyPowerEn, 1);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kUsbPhyPowerEn, kOutput);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAw86224Rst, 0);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kAw86224Rst, kOutput);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAdl161Trig, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAdl161Rst, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32p4Esp32c5UartSwitch, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLed, 1);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kAdl161Trig, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kAdl161Rst, kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kEsp32p4Esp32c5UartSwitch, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kEsp32c5En, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kEsp32c5Boot, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kLed, kOutput);

  if (!result) {
    chip_.xl9535->Deinit(false);
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitXl9535 failed\n");
    return false;
  }

  chip_status_.xl9535.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitXl9535 success\n");
  return true;
}

bool TDisplayP4AirDriver::InitSgm38121() {
  if (chip_.sgm38121 == nullptr || !chip_.sgm38121->Init()) {
    chip_status_.sgm38121.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSgm38121 failed\n");
    return false;
  }

  bool result = true;
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1,
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

bool TDisplayP4AirDriver::ConfigureEs8390OutputPolarity() {
  if (es8390_ctrl_if_ == nullptr || es8390_ctrl_if_->read_reg == nullptr ||
      es8390_ctrl_if_->write_reg == nullptr) {
    return false;
  }

  int register_value = 0;
  if (es8390_ctrl_if_->read_reg(es8390_ctrl_if_,
          kEs8390DacMiscControl2Register, 1, &register_value, 1) !=
      ESP_CODEC_DEV_OK) {
    return false;
  }

  register_value |= kEs8390Dac1OutputInvertMask;
  return es8390_ctrl_if_->write_reg(es8390_ctrl_if_,
             kEs8390DacMiscControl2Register, 1, &register_value,
             1) == ESP_CODEC_DEV_OK;
}

bool TDisplayP4AirDriver::InitBhi260ap() {
  if (IsBhi260apReady()) {
    return true;
  }
  if (chip_.bhi260ap == nullptr || bus_.bhi260ap_i2c_bus == nullptr) {
    chip_status_.bhi260ap.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBhi260ap failed\n");
    return false;
  }

  if (!chip_status_.xl9535.init_flag) {
    chip_status_.bhi260ap.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBhi260ap failed\n");
    return false;
  }
  bool reset_pin_initialized = true;
  reset_pin_initialized &=
      chip_.xl9535->GpioWrite(
        gpio::xl9535::kBhi260apRst, 1);
  reset_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kBhi260apRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_pin_initialized) {
    chip_status_.bhi260ap.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBhi260ap failed\n");
    return false;
  }
  platform_hal_->DelayMs(2);
  if (!chip_.xl9535->GpioWrite(
          gpio::xl9535::kBhi260apRst, 0)) {
    chip_status_.bhi260ap.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBhi260ap failed\n");
    return false;
  }
  platform_hal_->DelayMs(120);

  bool result = chip_.bhi260ap->Init();
  if (result) {
    result = chip_.bhi260ap->BootFromRam(bhy2_firmware_image,
        static_cast<uint32_t>(sizeof(bhy2_firmware_image)));
  }
  if (result) {
    struct bhy2_dev* context = chip_.bhi260ap->context();
    uint8_t host_interface_control = 0;
    result =
        context != nullptr &&
        bhy2_get_host_intf_ctrl(&host_interface_control, context) == BHY2_OK;
    if (result) {
      host_interface_control |= BHY2_HIF_CTRL_AP_SUSPENDED;
      result =
          bhy2_set_host_intf_ctrl(host_interface_control, context) == BHY2_OK;
    }
  }
  chip_status_.bhi260ap.init_flag = result;

  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "InitBhi260ap success (kernel version: %u)\n",
        static_cast<unsigned int>(chip_.bhi260ap->kernel_version()));
  } else {
    const int8_t last_error = chip_.bhi260ap->last_error();
    chip_.bhi260ap->Deinit(false);
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kBhi260apRst, 1);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitBhi260ap failed (error code: %d)\n", static_cast<int>(last_error));
  }
  return result;
}

bool TDisplayP4AirDriver::InitQmc6309() {
  if (IsQmc6309Ready()) {
    return true;
  }
  if (chip_.qmc6309 == nullptr || bus_.qmc6309_i2c_bus == nullptr ||
      !bus_.qmc6309_i2c_bus->InitBus()) {
    chip_status_.qmc6309.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitQmc6309 failed\n");
    return false;
  }

  bool result = chip_.qmc6309->begin(
      bus_.qmc6309_i2c_bus->bus_handle(), device::qmc6309::kI2cAddress);
  if (result) {
    chip_.qmc6309->setOffset(0, 0, 0);
    // SensorLib 0.4.1 的 ODR 接口写入了 0x0A 而非 0x0B。
    // 使用不依赖 ODR 的连续测量模式，并显式覆盖默认 OSR/LPF 配置。
    result = chip_.qmc6309->setOperationMode(OperationMode::SUSPEND) &&
             chip_.qmc6309->setFullScaleRange(MagFullScaleRange::FS_8G) &&
             chip_.qmc6309->setOversamplingRate(MagOverSampleRatio::OSR_8) &&
             chip_.qmc6309->setLowPassFilter(MagLowPassFilter::LPF_8) &&
             chip_.qmc6309->setSetResetMode(
                 SensorQMC6309::MagSetResetMode::SET_AND_RESET_ON);
    if (!result) {
      chip_.qmc6309->setOperationMode(OperationMode::SUSPEND);
    }
  }
  chip_status_.qmc6309.init_flag = result;

  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitQmc6309 success\n" : "InitQmc6309 failed\n");
  return result;
}

bool TDisplayP4AirDriver::InitHi8561() {
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

bool TDisplayP4AirDriver::InitHi8561Touch() {
  if (IsHi8561TouchReady()) {
    return true;
  }

  chip_status_.hi8561_touch.init_flag = false;
  if (!chip_status_.xl9535.init_flag || chip_.hi8561_touch == nullptr ||
      bus_.hi8561_i2c_touch_bus == nullptr) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }

  bool reset_result = true;
  // Air 板的触摸复位信号经过反相，高电平为复位状态。
  reset_result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, 1);
  reset_result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_result) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }
  platform_hal_->DelayMs(10);

  if (!chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, 0)) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }
  platform_hal_->DelayMs(100);

  if (chip_.hi8561_touch->Init(device::hi8561::kI2cFrequencyHz)) {
    chip_status_.hi8561_touch.init_flag = true;
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561Touch success\n");
    return true;
  }

  chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, 1);
  LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
  return false;
}

bool TDisplayP4AirDriver::InitRm69a10() {
  if (chip_.rm69a10 == nullptr) {
    chip_status_.rm69a10.init_flag = false;
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

bool TDisplayP4AirDriver::InitGt9895() {
  chip_status_.gt9895.init_flag = false;
  if (!chip_status_.xl9535.init_flag || chip_.gt9895 == nullptr) {
    return false;
  }
  bool reset_result = chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, 1);
  reset_result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_result) {
    return false;
  }
  platform_hal_->DelayMs(30);
  if (!chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, 0)) {
    return false;
  }
  platform_hal_->DelayMs(100);
  if (!chip_.gt9895->Init(device::gt9895::kI2cFrequencyHz)) {
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kTouchRst, 1);
    return false;
  }
  chip_status_.gt9895.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitGt9895 success\n");
  return true;
}

bool TDisplayP4AirDriver::InitSy7200a() {
  if (chip_.sy7200a != nullptr && chip_.sy7200a->IsInitialized()) {
    chip_status_.sy7200a.init_flag = true;
    return true;
  }
  if (chip_.sy7200a == nullptr) {
    chip_status_.sy7200a.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSy7200a failed\n");
    return false;
  }

  cpp_bus_driver::Pwm::Config config;
  config.timer = LEDC_TIMER_0;
  config.channel = LEDC_CHANNEL_0;
  config.frequency_hz = device::sy7200a::kPwmFrequencyHz;
  if (!chip_.sy7200a->Init(config)) {
    chip_status_.sy7200a.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSy7200a failed\n");
    return false;
  }

  chip_status_.sy7200a.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSy7200a success\n");
  return true;
}

bool TDisplayP4AirDriver::InitAw86224() {
  if (IsAw86224Ready()) {
    return true;
  }
  if (!chip_status_.xl9535.init_flag || chip_.aw86224 == nullptr) {
    return false;
  }
  bool reset_result = chip_.xl9535->GpioWrite(gpio::xl9535::kAw86224Rst, 0);
  reset_result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kAw86224Rst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  platform_hal_->DelayMs(2);
  reset_result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAw86224Rst, 1);
  platform_hal_->DelayMs(5);
  if (!reset_result ||
      !chip_.aw86224->Init(device::aw86224::kI2cFrequencyHz)) {
    chip_.xl9535->GpioWrite(gpio::xl9535::kAw86224Rst, 0);
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

bool TDisplayP4AirDriver::InitSt25r3916() {
  if (IsSt25r3916Ready()) {
    return true;
  }
  if (chip_.st25r3916 == nullptr || bus_.st25r3916_i2c_bus == nullptr) {
    chip_status_.st25r3916.init_flag = false;
    chip_status_.st25r3916.result = RFAL_ERR_INVALID_HANDLE;
    chip_status_.st25r3916.platform_error =
        stsw_st25rfal002_cpp_bus_driver::PlatformError::kInvalidConfiguration;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSt25r3916 failed\n");
    return false;
  }

  chip_status_.st25r3916.result = chip_.st25r3916->Init();
  chip_status_.st25r3916.platform_error = chip_.st25r3916->platform_error();
  chip_status_.st25r3916.init_flag =
      chip_status_.st25r3916.result == RFAL_ERR_NONE &&
      chip_status_.st25r3916.platform_error ==
          stsw_st25rfal002_cpp_bus_driver::PlatformError::kNone &&
      chip_.st25r3916->initialized();

  if (!chip_status_.st25r3916.init_flag) {
    chip_.st25r3916->Deinit(false);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitSt25r3916 failed (RFAL: %u, platform: %u)\n",
        static_cast<unsigned int>(chip_status_.st25r3916.result),
        static_cast<unsigned int>(chip_status_.st25r3916.platform_error));
    return false;
  }

  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSt25r3916 success\n");
  return true;
}

bool TDisplayP4AirDriver::InitEs8390() {
  if (IsEs8390Ready()) {
    return true;
  }
  if ((bus_.xl9535_i2c_bus == nullptr) || (bus_.es8390_i2s_bus == nullptr)) {
    chip_status_.es8390.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8390 failed\n");
    return false;
  }

  i2c_master_bus_handle_t i2c_bus_handle = bus_.xl9535_i2c_bus->bus_handle();
  if (i2c_bus_handle == nullptr) {
    chip_status_.es8390.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8390 failed\n");
    return false;
  }

  audio_codec_i2c_cfg_t i2c_cfg = {
      .port = static_cast<uint8_t>(I2C_NUM_1),
      .addr = static_cast<uint8_t>(device::es8390::kI2cAddress << 1),
      .bus_handle = i2c_bus_handle,
  };
  es8390_ctrl_if_ = audio_codec_new_i2c_ctrl(&i2c_cfg);

  if (es8390_ctrl_if_ == nullptr ||
      !bus_.es8390_i2s_bus->Init(
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
                LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
                    "Value out of range\n");
                return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_256;
            }
          }(device::es8390::kMclkMultiple),
          device::es8390::kSampleRate,
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
                LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
                    "Value out of range\n");
                return i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_16BIT;
            }
          }(device::es8390::kBitsPerSample))) {
    DeinitEs8390();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8390 failed\n");
    return false;
  }

  audio_codec_i2s_cfg_t i2s_cfg = {
      .port = static_cast<uint8_t>(bus_.es8390_i2s_bus->port()),
      .rx_handle = bus_.es8390_i2s_bus->rx_handle(),
      .tx_handle = bus_.es8390_i2s_bus->tx_handle(),
      .clk_src = static_cast<int>(I2S_CLK_SRC_DEFAULT),
  };
  es8390_data_if_ = audio_codec_new_i2s_data(&i2s_cfg);
  if (es8390_data_if_ == nullptr) {
    DeinitEs8390();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8390 failed\n");
    return false;
  }
  if (!chip_status_.xl9535.init_flag || chip_.xl9535 == nullptr) {
    chip_status_.es8390.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8390 failed\n");
    return false;
  }
  bool amplifier_pin_initialized = true;
  amplifier_pin_initialized &=
      chip_.xl9535->GpioWrite(gpio::xl9535::kNs4150En, 0);
  amplifier_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kNs4150En, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!amplifier_pin_initialized) {
    DeinitEs8390();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8390 failed\n");
    return false;
  }

  es8390_gpio_if_ = audio_codec_new_gpio();
  if (es8390_gpio_if_ == nullptr) {
    DeinitEs8390();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8390 failed\n");
    return false;
  }

  es8389_codec_cfg_t codec_cfg = {
      .ctrl_if = es8390_ctrl_if_,
      .gpio_if = es8390_gpio_if_,
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
      .mclk_div = device::es8390::kMclkMultiple,
  };
  es8390_codec_if_ = es8389_codec_new(&codec_cfg);
  if (es8390_codec_if_ == nullptr) {
    DeinitEs8390();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8390 failed\n");
    return false;
  }

  esp_codec_dev_cfg_t output_dev_cfg = {
      .dev_type = ESP_CODEC_DEV_TYPE_OUT,
      .codec_if = es8390_codec_if_,
      .data_if = es8390_data_if_,
  };
  es8390_output_codec_dev_ = esp_codec_dev_new(&output_dev_cfg);
  if (es8390_output_codec_dev_ == nullptr) {
    DeinitEs8390();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8390 failed\n");
    return false;
  }

  esp_codec_dev_cfg_t input_dev_cfg = {
      .dev_type = ESP_CODEC_DEV_TYPE_IN,
      .codec_if = es8390_codec_if_,
      .data_if = es8390_data_if_,
  };
  es8390_input_codec_dev_ = esp_codec_dev_new(&input_dev_cfg);
  if (es8390_input_codec_dev_ == nullptr) {
    DeinitEs8390();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8390 failed\n");
    return false;
  }

  esp_codec_dev_sample_info_t output_sample_info = {
      .bits_per_sample = device::es8390::kBitsPerSample,
      .channel = device::es8390::kChannel,
      .channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0) |
                      ESP_CODEC_DEV_MAKE_CHANNEL_MASK(1),
      .sample_rate = device::es8390::kSampleRate,
      .mclk_multiple = device::es8390::kMclkMultiple,
  };
  esp_codec_dev_sample_info_t input_sample_info = output_sample_info;
  const bool output_opened = esp_codec_dev_open(es8390_output_codec_dev_,
                                 &output_sample_info) == ESP_CODEC_DEV_OK;
  const bool input_opened =
      output_opened && esp_codec_dev_open(es8390_input_codec_dev_,
                           &input_sample_info) == ESP_CODEC_DEV_OK;
  bool result = output_opened && input_opened;
  if (result) {
    result &= ConfigureEs8390OutputPolarity();
    result &= (esp_codec_dev_set_out_vol(es8390_output_codec_dev_, 100) ==
               ESP_CODEC_DEV_OK);
    result &= (esp_codec_dev_set_in_gain(es8390_input_codec_dev_, 20.0f) ==
               ESP_CODEC_DEV_OK);
  }

  // 关闭操作属于回滚，必须尽量释放所有已经打开的通道。
  if (input_opened) {
    result &=
        (esp_codec_dev_close(es8390_input_codec_dev_) == ESP_CODEC_DEV_OK);
  }
  if (output_opened) {
    result &=
        (esp_codec_dev_close(es8390_output_codec_dev_) == ESP_CODEC_DEV_OK);
  }
  result &= SetNs4150Enabled(false);
  es8390_operating_mode_ = Es8390OperatingMode::kSleep;
  chip_status_.es8390.init_flag = result;
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitEs8390 success\n");
  } else {
    DeinitEs8390();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8390 failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::InitLr2021() {
  if (IsLr2021Ready()) {
    return true;
  }
  chip_status_.lr2021.init_flag = false;
  if (chip_.lr2021 == nullptr || !chip_status_.xl9535.init_flag) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr2021 power enable failed\n");
    return false;
  }
  bool power_enabled = true;
  power_enabled &= chip_.xl9535->GpioWrite(gpio::xl9535::kLr2021PowerEn, 0);
  power_enabled &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kLr2021PowerEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  power_enabled &= chip_.xl9535->GpioWrite(gpio::xl9535::kLr2021PowerEn, 1);
  if (!power_enabled) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr2021 power enable failed\n");
    return false;
  }
  platform_hal_->DelayMs(10);

  bool transport_initialized = true;
  transport_initialized &= platform_hal_->SetGpioMode(
      gpio::lr2021::kRst, cpp_bus_driver::PlatformHal::GpioMode::kOutput);
  transport_initialized &= platform_hal_->SetGpioMode(
      gpio::lr2021::kInt, cpp_bus_driver::PlatformHal::GpioMode::kInput);
  if (!transport_initialized ||
      !chip_.lr2021->Init(device::lr2021::kSpiFrequencyHz)) {
    platform_hal_->GpioWrite(gpio::lr2021::kRst, 0);
    chip_.xl9535->GpioWrite(gpio::xl9535::kLr2021PowerEn, 0);
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitLr2021 transport failed\n");
    return false;
  }

  lr20xx_system_version_t version = {};
  bool detected = false;
  for (uint8_t attempt = 0; attempt < 10; ++attempt) {
    detected = chip_.lr2021->Invoke(lr20xx_system_get_version, &version) ==
                   LR20XX_STATUS_OK &&
               version.major == kLr2021ExpectedVersionMajor &&
               version.minor == kLr2021ExpectedVersionMinor;
    if (detected || !chip_.lr2021->Reset()) {
      break;
    }
    platform_hal_->DelayMs(10);
  }
  if (!detected) {
    chip_.lr2021->Deinit(false);
    platform_hal_->GpioWrite(gpio::lr2021::kRst, 0);
    chip_.xl9535->GpioWrite(gpio::xl9535::kLr2021PowerEn, 0);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr2021 chip detection failed (fw: %u.%u)\n",
        static_cast<unsigned>(version.major),
        static_cast<unsigned>(version.minor));
    return false;
  }

  const auto configure_rf_switch =
      [this](
          lr20xx_system_dio_t dio, lr20xx_system_dio_rf_switch_cfg_t config) {
        if (chip_.lr2021->Invoke(lr20xx_system_set_dio_function, dio,
                LR20XX_SYSTEM_DIO_FUNC_RF_SWITCH,
                LR20XX_SYSTEM_DIO_DRIVE_NONE) != LR20XX_STATUS_OK) {
          return false;
        }
        return chip_.lr2021->Invoke(lr20xx_system_set_dio_rf_switch_cfg, dio,
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
  bool result = true;
  result &= (chip_.lr2021->Invoke(lr20xx_system_set_standby_mode,
                 LR20XX_SYSTEM_STANDBY_MODE_RC) == LR20XX_STATUS_OK);
  result &= (chip_.lr2021->Invoke(lr20xx_system_set_tcxo_mode,
                 LR20XX_SYSTEM_TCXO_CTRL_3_0V, 32768U) == LR20XX_STATUS_OK);
  result &= (chip_.lr2021->Invoke(lr20xx_system_set_reg_mode,
                 LR20XX_SYSTEM_REG_MODE_DCDC) == LR20XX_STATUS_OK);
  result &= (chip_.lr2021->Invoke(lr20xx_radio_common_set_rx_tx_fallback_mode,
                 LR20XX_RADIO_FALLBACK_STDBY_RC) == LR20XX_STATUS_OK);
  result &= (chip_.lr2021->Invoke(lr20xx_system_clear_irq_status,
                 LR20XX_SYSTEM_IRQ_ALL_MASK) == LR20XX_STATUS_OK);
  result &= (chip_.lr2021->Invoke(lr20xx_system_calibrate, kCalibrationMask) ==
             LR20XX_STATUS_OK);
  result &= configure_rf_switch(
      LR20XX_SYSTEM_DIO_6, LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_RX_HF);
  result &= configure_rf_switch(
      LR20XX_SYSTEM_DIO_7, LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_TX_HF);
  result &= configure_rf_switch(
      LR20XX_SYSTEM_DIO_8, LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_RX_LF |
                               LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_TX_LF);
  result &= configure_rf_switch(
      LR20XX_SYSTEM_DIO_10, LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_RX_HF |
                                LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_TX_HF);
  result &= (chip_.lr2021->Invoke(lr20xx_system_set_dio_function,
                 LR20XX_SYSTEM_DIO_11, LR20XX_SYSTEM_DIO_FUNC_IRQ,
                 LR20XX_SYSTEM_DIO_DRIVE_NONE) == LR20XX_STATUS_OK);
  result &=
      (chip_.lr2021->Invoke(lr20xx_system_set_dio_irq_cfg, LR20XX_SYSTEM_DIO_11,
           LR20XX_SYSTEM_IRQ_NONE) == LR20XX_STATUS_OK);
  result &= chip_.lr2021->SetSleep(sleep_config);
  if (!result) {
    chip_.lr2021->Deinit(false);
    platform_hal_->GpioWrite(gpio::lr2021::kRst, 0);
    chip_.xl9535->GpioWrite(gpio::xl9535::kLr2021PowerEn, 0);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr2021 hardware configuration failed\n");
    return false;
  }

  chip_status_.lr2021.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "InitLr2021 success (fw: %u.%u)\n",
      static_cast<unsigned>(version.major),
      static_cast<unsigned>(version.minor));
  return true;
}

bool TDisplayP4AirDriver::InitNrf9151() {
  if (IsNrf9151Ready()) {
    return true;
  }
  if (chip_.nrf9151 == nullptr || bus_.nrf9151_uart_bus == nullptr ||
      !chip_status_.xl9535.init_flag) {
    chip_status_.nrf9151.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitNrf9151 failed: driver, UART bus, or modem power is "
        "unavailable\n");
    return false;
  }

  bool power_enabled = true;
  power_enabled &= chip_.xl9535->GpioWrite(gpio::xl9535::kNrf9151En, 0);
  power_enabled &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kNrf9151En, cpp_bus_driver::Xl95x5::Mode::kOutput);
  power_enabled &= chip_.xl9535->GpioWrite(gpio::xl9535::kNrf9151En, 1);
  if (!power_enabled) {
    chip_status_.nrf9151.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitNrf9151 failed: driver, UART bus, or modem power is "
        "unavailable\n");
    return false;
  }

  const bool result = chip_.nrf9151->Init(device::nrf9151::kDefaultBaudRate);

  if (!result) {
    if (!chip_.xl9535->GpioWrite(gpio::xl9535::kNrf9151En, 0)) {
      LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
          "Disable nRF9151 power after init failure failed\n");
    }
  }

  if (result) {
    cpp_bus_driver::Nrf9151::SerialModemVersion serial_modem_version;
    if (chip_.nrf9151->GetSerialModemVersion(&serial_modem_version)) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
          "Nrf9151 Serial Modem version: %s, NCS version: %s\n",
          serial_modem_version.application.c_str(),
          serial_modem_version.ncs.c_str());
      if (!serial_modem_version.customer.empty()) {
        LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
            "Nrf9151 customer version: %s\n",
            serial_modem_version.customer.c_str());
      }
    } else {
      LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
          "Get Nrf9151 Serial Modem version failed\n");
    }

    std::string modem_firmware_version;
    if (chip_.nrf9151->GetModemFirmwareVersion(&modem_firmware_version)) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
          "Nrf9151 modem firmware version: %s\n",
          modem_firmware_version.c_str());
    } else {
      LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
          "Get Nrf9151 modem firmware version failed\n");
    }
  }

  chip_status_.nrf9151.init_flag = result;
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitNrf9151 success\n");
  } else {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitNrf9151 failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::InitPower() {
  if (power_initialized_) {
    return true;
  }
  // 先预置使能电平，再配置输出方向；后续开关电源只需切换电平。
  if (!Set3v3PowerEnabled(true) ||
      !platform_hal_->SetGpioMode(gpio::power::kEnable3v3,
          cpp_bus_driver::PlatformHal::GpioMode::kOutput,
          cpp_bus_driver::PlatformHal::GpioStatus::kDisable)) {
    return false;
  }
  if (!InitLdoPower(3, 2500)) {
    return false;
  }
  if (!InitLdoPower(4, 3300)) {
    DeinitLdoPower(3);
    return false;
  }
  power_initialized_ = true;
  return true;
}

bool TDisplayP4AirDriver::InitScreen() {
  if (!chip_status_.xl9535.init_flag) {
    return false;
  }
  bool reset_pin_initialized = true;
  reset_pin_initialized &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kScreenRst, 1);
  reset_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kScreenRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_pin_initialized) {
    return false;
  }
  platform_hal_->DelayMs(10);
  if (!chip_.xl9535->GpioWrite(
      gpio::xl9535::kScreenRst, 0)) {
    return false;
  }
  platform_hal_->DelayMs(120);

  if (!DetectScreenType()) {
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kScreenRst, 1);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "DetectScreenType failed\n");
    return false;
  }
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
                LogLevel::kWarning, __FILE__, __LINE__, "Value out of range\n");
            return cpp_bus_driver::HardwareMipi::ColorFormat::kRgb565;
        }
      }(screen.bits_per_pixel));

  chip_.hi8561.reset();
  chip_.rm69a10.reset();
  chip_status_.hi8561.init_flag = false;
  chip_status_.hi8561_touch.init_flag = false;
  chip_status_.sy7200a.init_flag = false;
  chip_status_.rm69a10.init_flag = false;

  bool result = false;
  switch (screen.type) {
    case ScreenType::kHi8561:
      chip_.hi8561 =
          std::make_unique<cpp_bus_driver::Hi8561>(bus_.screen_mipi_bus);
      result = InitHi8561();
      break;
    case ScreenType::kRm69a10:
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

bool TDisplayP4AirDriver::InitTouch() {
  switch (screen_type()) {
    case ScreenType::kHi8561:
      return InitHi8561Touch();
    case ScreenType::kRm69a10:
      return chip_status_.gt9895.init_flag || InitGt9895();
    default:
      return false;
  }
}

bool TDisplayP4AirDriver::InitScreenBacklight() {
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      return InitSy7200a();
    case device::ScreenType::kRm69a10:
      return true;
    default:
      return false;
  }
}

bool TDisplayP4AirDriver::InitSpiffs(
    const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf) {
  if (base_path == nullptr || base_path[0] == '\0') {
    return false;
  }
  SpiffsConfig config;
  config.base_path = base_path;
  if (!lilygo_device_driver::InitSpiffs(config)) {
    return false;
  }
  spiffs_conf.base_path = base_path;
  spiffs_conf.partition_label = config.partition_label;
  spiffs_conf.max_files = config.max_files;
  spiffs_conf.format_if_mount_failed = config.format_if_mount_failed;
  return true;
}

bool TDisplayP4AirDriver::InitSdmmc(const char* base_path, int max_freq_khz) {
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
      gpio::xl9535::kSdPowerEn, 0);
  power_enabled &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kSdPowerEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  power_enabled &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSdPowerEn, 1);
  if (!power_enabled) {
    chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
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

  const bool result = sd_card_.InitSdmmc(base_path, config);
  chip_status_.sd_card.init_flag = sd_card_.IsMounted();
  if (!result) {
    chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
  }
  return result;
}

bool TDisplayP4AirDriver::InitSdspi(
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
      gpio::xl9535::kSdPowerEn, 0);
  power_enabled &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kSdPowerEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  power_enabled &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kSdPowerEn, 1);
  if (!power_enabled) {
    chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
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
    chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
  }
  return result;
}

bool TDisplayP4AirDriver::DeinitBhi260ap() {
  bool result = true;
  if (chip_status_.bhi260ap.init_flag && chip_.bhi260ap != nullptr) {
    result &= SetBhi260apSleep(true);
    result &= chip_.bhi260ap->Deinit(false);
  }
  if (chip_status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kBhi260apRst, 1);
  }
  chip_status_.bhi260ap.init_flag = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitQmc6309() {
  bool result = true;
  if (chip_status_.qmc6309.init_flag && chip_.qmc6309 != nullptr) {
    result = chip_.qmc6309->setOperationMode(OperationMode::SUSPEND);
  }
  chip_status_.qmc6309.init_flag = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitAw86224() {
  bool result = true;
  if (chip_status_.aw86224.init_flag && chip_.aw86224 != nullptr) {
    result &= chip_.aw86224->StopRamPlaybackWaveform();
    result &= chip_.aw86224->Deinit(false);
  }
  if (chip_status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAw86224Rst, 0);
  }
  chip_status_.aw86224.init_flag = false;
  chip_status_.aw86224.ram_waveform_info = {};
  return result;
}

bool TDisplayP4AirDriver::DeinitSt25r3916() {
  if (!chip_status_.st25r3916.init_flag) {
    return true;
  }

  chip_status_.st25r3916.result = chip_.st25r3916->Deinit(false);
  chip_status_.st25r3916.platform_error = chip_.st25r3916->platform_error();
  chip_status_.st25r3916.init_flag = false;
  return chip_status_.st25r3916.result == RFAL_ERR_NONE &&
         chip_status_.st25r3916.platform_error ==
             stsw_st25rfal002_cpp_bus_driver::PlatformError::kNone;
}

bool TDisplayP4AirDriver::DeinitLr2021() {
  bool result = true;
  if (chip_status_.lr2021.init_flag && chip_.lr2021 != nullptr) {
    result &= SetLr2021OperatingMode(Lr2021OperatingMode::kSleep);
    result &= chip_.lr2021->Deinit(false);
    result &= platform_hal_->GpioWrite(gpio::lr2021::kRst, 0);
  }
  if (chip_status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLr2021PowerEn, 0);
  }
  chip_status_.lr2021.init_flag = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitNrf9151() {
  bool result = true;
  if (chip_status_.nrf9151.init_flag && chip_.nrf9151 != nullptr) {
    result &= chip_.nrf9151->Deinit();
  }
  if (chip_status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kNrf9151En, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kUsbPhyPowerEn, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAw86224Rst, 0);
  }
  chip_status_.nrf9151.init_flag = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitEs8390() {
  bool result = true;
  result &= SetEs8390OperatingMode(Es8390OperatingMode::kSleep);

  if (es8390_input_codec_dev_ != nullptr) {
    esp_codec_dev_delete(es8390_input_codec_dev_);
    es8390_input_codec_dev_ = nullptr;
  }
  if (es8390_output_codec_dev_ != nullptr) {
    esp_codec_dev_delete(es8390_output_codec_dev_);
    es8390_output_codec_dev_ = nullptr;
  }
  if (es8390_codec_if_ != nullptr) {
    result &=
        (audio_codec_delete_codec_if(es8390_codec_if_) == ESP_CODEC_DEV_OK);
    es8390_codec_if_ = nullptr;
  }
  if (es8390_ctrl_if_ != nullptr) {
    result &= (audio_codec_delete_ctrl_if(es8390_ctrl_if_) == ESP_CODEC_DEV_OK);
    es8390_ctrl_if_ = nullptr;
  }
  if (es8390_data_if_ != nullptr) {
    result &= (audio_codec_delete_data_if(es8390_data_if_) == ESP_CODEC_DEV_OK);
    es8390_data_if_ = nullptr;
  }
  if (es8390_gpio_if_ != nullptr) {
    result &= (audio_codec_delete_gpio_if(es8390_gpio_if_) == ESP_CODEC_DEV_OK);
    es8390_gpio_if_ = nullptr;
  }
  if (bus_.es8390_i2s_bus != nullptr) {
    result &= bus_.es8390_i2s_bus->Deinit();
  }

  chip_status_.es8390.init_flag = false;
  es8390_operating_mode_ = Es8390OperatingMode::kSleep;
  return result;
}

bool TDisplayP4AirDriver::Set3v3PowerEnabled(bool enabled) {
  if (platform_hal_ == nullptr) {
    return false;
  }
  return platform_hal_->GpioWrite(gpio::power::kEnable3v3, enabled);
}

bool TDisplayP4AirDriver::DeinitPower() {
  bool result = true;
  result &= DeinitLdoPower(3);
  result &= DeinitLdoPower(4);
  power_initialized_ = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitScreen() {
  bool result = true;
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      if (chip_status_.hi8561.init_flag) {
        result &= chip_.hi8561->Deinit();
      }
      break;
    case device::ScreenType::kRm69a10:
      if (chip_status_.rm69a10.init_flag) {
        result &= chip_.rm69a10->Deinit();
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
        gpio::xl9535::kScreenRst, 1);
  }
  chip_status_.hi8561.init_flag = false;
  chip_status_.rm69a10.init_flag = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitTouch() {
  bool result = true;
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      if (chip_status_.hi8561_touch.init_flag) {
        result &= chip_.hi8561_touch->Deinit(false);
      }
      break;
    case device::ScreenType::kRm69a10:
      if (chip_status_.gt9895.init_flag) {
        result &= chip_.gt9895->Deinit(false);
      }
      break;
    default:
      break;
  }
  if (chip_status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kTouchRst, 1);
  }
  chip_status_.hi8561_touch.init_flag = false;
  chip_status_.gt9895.init_flag = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitScreenBacklight() {
  if (chip_.sy7200a == nullptr || !chip_.sy7200a->IsInitialized()) {
    chip_status_.sy7200a.init_flag = false;
    return true;
  }

  const bool result = chip_.sy7200a->Deinit();
  chip_status_.sy7200a.init_flag = chip_.sy7200a->IsInitialized();
  return result;
}

bool TDisplayP4AirDriver::DeinitSdmmc(bool release_bus) {
  bool result = sd_card_.Deinit(release_bus);
  chip_status_.sd_card.init_flag = sd_card_.IsMounted();
  // 卸载失败时保留供电，允许后续重试。
  if (sd_card_.IsMounted()) {
    return false;
  }
  if (chip_status_.xl9535.init_flag && chip_.xl9535 != nullptr) {
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kSdPowerEn, 0);
  }
  return result;
}

bool TDisplayP4AirDriver::IsAxp517Ready() const {
  return chip_status_.axp517.init_flag && chip_.axp517 != nullptr;
}

bool TDisplayP4AirDriver::IsRx8130ceReady() const {
  return chip_status_.rx8130ce.init_flag && chip_.rx8130ce != nullptr;
}

bool TDisplayP4AirDriver::IsXl9535Ready() const {
  return chip_status_.xl9535.init_flag && chip_.xl9535 != nullptr;
}

bool TDisplayP4AirDriver::IsSgm38121Ready() const {
  return chip_status_.sgm38121.init_flag && chip_.sgm38121 != nullptr;
}

bool TDisplayP4AirDriver::IsBhi260apReady() const {
  return chip_status_.bhi260ap.init_flag && chip_.bhi260ap != nullptr &&
         chip_.bhi260ap->initialized() && chip_.bhi260ap->firmware_running();
}

bool TDisplayP4AirDriver::IsQmc6309Ready() const {
  return chip_status_.qmc6309.init_flag && chip_.qmc6309 != nullptr;
}

bool TDisplayP4AirDriver::IsHi8561Ready() const {
  return chip_status_.hi8561.init_flag && chip_.hi8561 != nullptr;
}

bool TDisplayP4AirDriver::IsHi8561TouchReady() const {
  return chip_status_.hi8561_touch.init_flag && chip_.hi8561_touch != nullptr;
}

bool TDisplayP4AirDriver::IsRm69a10Ready() const {
  return chip_status_.rm69a10.init_flag && chip_.rm69a10 != nullptr;
}

bool TDisplayP4AirDriver::IsGt9895Ready() const {
  return chip_status_.gt9895.init_flag && chip_.gt9895 != nullptr;
}

bool TDisplayP4AirDriver::IsSy7200aReady() const {
  return chip_status_.sy7200a.init_flag && chip_.sy7200a != nullptr &&
         chip_.sy7200a->IsInitialized();
}

bool TDisplayP4AirDriver::IsAw86224Ready() const {
  return chip_status_.aw86224.init_flag && chip_.aw86224 != nullptr;
}

bool TDisplayP4AirDriver::IsSt25r3916Ready() const {
  return chip_status_.st25r3916.init_flag && chip_.st25r3916 != nullptr &&
         chip_.st25r3916->initialized();
}

bool TDisplayP4AirDriver::IsEs8390Ready() const {
  return chip_status_.es8390.init_flag && es8390_input_codec_dev_ != nullptr &&
         es8390_output_codec_dev_ != nullptr;
}

bool TDisplayP4AirDriver::IsLr2021Ready() const {
  return chip_status_.lr2021.init_flag && chip_.lr2021 != nullptr &&
         chip_.lr2021->initialized();
}

bool TDisplayP4AirDriver::IsNrf9151Ready() const {
  return chip_status_.nrf9151.init_flag && chip_.nrf9151 != nullptr;
}

bool TDisplayP4AirDriver::IsScreenReady() const {
  if (bus_.screen_mipi_bus == nullptr ||
      bus_.screen_mipi_bus->device_handle() == nullptr) {
    return false;
  }
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      return IsHi8561Ready() && IsSy7200aReady();
    case device::ScreenType::kRm69a10:
      return IsRm69a10Ready();
    default:
      return false;
  }
}

bool TDisplayP4AirDriver::IsTouchReady() const {
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      return IsHi8561TouchReady();
    case device::ScreenType::kRm69a10:
      return IsGt9895Ready();
    default:
      return false;
  }
}

bool TDisplayP4AirDriver::IsScreenBacklightReady() const {
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      return IsSy7200aReady();
    case device::ScreenType::kRm69a10:
      return IsRm69a10Ready();
    default:
      return false;
  }
}

bool TDisplayP4AirDriver::IsSdmmcReady() const {
  return chip_status_.sd_card.init_flag && sd_card_.IsReady();
}

bool TDisplayP4AirDriver::SetNs4150Enabled(bool enabled) {
  if (!chip_status_.xl9535.init_flag) {
    return !enabled;
  }
  return chip_.xl9535->GpioWrite(gpio::xl9535::kNs4150En, enabled ? 1 : 0);
}

bool TDisplayP4AirDriver::SetLedEnabled(bool enabled) {
  if (!chip_status_.xl9535.init_flag) {
    return !enabled;
  }
  return chip_.xl9535->GpioWrite(gpio::xl9535::kLed, enabled ? 1 : 0);
}

bool TDisplayP4AirDriver::SetAw86224Standby() {
  return !IsAw86224Ready() || chip_.aw86224->StopRamPlaybackWaveform();
}

bool TDisplayP4AirDriver::SetBhi260apSleep(bool sleep) {
  if (!IsBhi260apReady()) {
    return sleep;
  }

  struct bhy2_dev* context = chip_.bhi260ap->context();
  uint8_t host_interface_control = 0;
  if (context == nullptr ||
      bhy2_get_host_intf_ctrl(&host_interface_control, context) != BHY2_OK) {
    return false;
  }

  if (sleep) {
    host_interface_control |= BHY2_HIF_CTRL_AP_SUSPENDED;
  } else {
    host_interface_control &= static_cast<uint8_t>(~BHY2_HIF_CTRL_AP_SUSPENDED);
  }
  return bhy2_set_host_intf_ctrl(host_interface_control, context) == BHY2_OK;
}

bool TDisplayP4AirDriver::SetQmc6309Sleep(bool sleep) {
  if (!IsQmc6309Ready()) {
    return sleep;
  }
  return chip_.qmc6309->setOperationMode(
      sleep ? OperationMode::SUSPEND : OperationMode::CONTINUOUS_MEASUREMENT);
}

bool TDisplayP4AirDriver::SetScreenSleep(bool sleep) {
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      if (!chip_status_.hi8561.init_flag) {
        return sleep;
      }
      if (sleep) {
        return chip_.hi8561->SetScreenOff(true) &&
               chip_.hi8561->SetSleep(true);
      }
      return chip_.hi8561->SetSleep(false) &&
             chip_.hi8561->SetScreenOff(false);
    case device::ScreenType::kRm69a10:
      if (!chip_status_.rm69a10.init_flag) {
        return sleep;
      }
      if (sleep) {
        return chip_.rm69a10->SetScreenOff(true) &&
               chip_.rm69a10->SetSleep(true);
      }
      return chip_.rm69a10->SetSleep(false) &&
             chip_.rm69a10->SetScreenOff(false);
    default:
      return false;
  }
}

bool TDisplayP4AirDriver::DetectScreenType() {
  if (!DeinitTouch()) {
    return false;
  }
  screen_info_ = nullptr;

  if (!chip_status_.xl9535.init_flag || chip_.xl9535 == nullptr ||
      chip_.gt9895 == nullptr) {
    return false;
  }
  bool reset_result = chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, 1);
  reset_result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_result) {
    return false;
  }
  platform_hal_->DelayMs(30);
  if (!chip_.xl9535->GpioWrite(
      gpio::xl9535::kTouchRst, 0)) {
    return false;
  }
  platform_hal_->DelayMs(100);

  if (chip_.gt9895->Init(device::gt9895::kI2cFrequencyHz)) {
    screen_info_ = ScreenInfoForType(device::ScreenType::kRm69a10);
    chip_status_.gt9895.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "Auto detected T-Display-P4-Air screen: %s\n", screen_info_->name);
  } else {
    if (!chip_.gt9895->Deinit(false)) {
      return false;
    }
    chip_status_.gt9895.init_flag = false;
    screen_info_ = ScreenInfoForType(device::ScreenType::kHi8561);
    LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
        "GT9895 detection failed; trying HI8561 (screen type not confirmed)\n");
  }
  return true;
}

bool TDisplayP4AirDriver::SetEs8390OperatingMode(Es8390OperatingMode mode) {
  if (!chip_status_.es8390.init_flag) {
    return mode == Es8390OperatingMode::kSleep && SetNs4150Enabled(false);
  }
  if (mode == es8390_operating_mode_) {
    return mode != Es8390OperatingMode::kSleep || SetNs4150Enabled(false);
  }

  bool result = true;
  if (mode == Es8390OperatingMode::kSleep) {
    result &=
        (esp_codec_dev_close(es8390_input_codec_dev_) == ESP_CODEC_DEV_OK);
    result &=
        (esp_codec_dev_close(es8390_output_codec_dev_) == ESP_CODEC_DEV_OK);
    result &= SetNs4150Enabled(false);
    es8390_operating_mode_ = Es8390OperatingMode::kSleep;
  } else {
    esp_codec_dev_sample_info_t output_sample_info = {
        .bits_per_sample = device::es8390::kBitsPerSample,
        .channel = device::es8390::kChannel,
        .channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0) |
                        ESP_CODEC_DEV_MAKE_CHANNEL_MASK(1),
        .sample_rate = device::es8390::kSampleRate,
        .mclk_multiple = device::es8390::kMclkMultiple,
    };
    esp_codec_dev_sample_info_t input_sample_info = output_sample_info;
    const bool output_opened = esp_codec_dev_open(es8390_output_codec_dev_,
                                   &output_sample_info) == ESP_CODEC_DEV_OK;
    const bool input_opened =
        output_opened && esp_codec_dev_open(es8390_input_codec_dev_,
                             &input_sample_info) == ESP_CODEC_DEV_OK;
    result = output_opened && input_opened &&
             ConfigureEs8390OutputPolarity() && SetNs4150Enabled(true);
    if (!result) {
      if (input_opened) {
        esp_codec_dev_close(es8390_input_codec_dev_);
      }
      if (output_opened) {
        esp_codec_dev_close(es8390_output_codec_dev_);
      }
      SetNs4150Enabled(false);
    }
  }
  if (result && mode == Es8390OperatingMode::kActive) {
    es8390_operating_mode_ = mode;
  }
  return result;
}

bool TDisplayP4AirDriver::SetLr2021OperatingMode(Lr2021OperatingMode mode) {
  if (!chip_status_.lr2021.init_flag || chip_.lr2021 == nullptr) {
    return mode == Lr2021OperatingMode::kSleep;
  }
  if (mode == Lr2021OperatingMode::kSleep) {
    const lr20xx_system_sleep_cfg_t sleep_config = {
        .is_clk_32k_enabled = false,
        .is_ram_retention_enabled = true,
    };
    return chip_.lr2021->SetSleep(sleep_config);
  }
  return chip_.lr2021->Wakeup() &&
         chip_.lr2021->Invoke(lr20xx_system_set_standby_mode,
             LR20XX_SYSTEM_STANDBY_MODE_RC) == LR20XX_STATUS_OK;
}

bool TDisplayP4AirDriver::SetEsp32c5PowerEnabled(bool enabled) {
  if (!chip_status_.xl9535.init_flag) {
    return !enabled;
  }
  return chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, enabled ? 1 : 0);
}

bool TDisplayP4AirDriver::SetCameraPowerEnabled(bool enabled) {
  if (!chip_status_.sgm38121.init_flag) {
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
  // 特殊硬件限制：AVDD1 开启时必须同时开启 DVDD1，否则摄像头会卡住 I2C。
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, status);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, status);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, status);
#endif
  return result;
}

bool TDisplayP4AirDriver::PrepareMinimalDriversForPowerOff() {
  bool result = true;
  if (IsXl9535Ready()) {
    result &= SetLedEnabled(false);
    result &= chip_.xl9535->Deinit(false);
    chip_status_.xl9535.init_flag = false;
  }
  if (chip_status_.axp517.init_flag && chip_.axp517 != nullptr) {
    result &= chip_.axp517->Deinit(false);
    chip_status_.axp517.init_flag = false;
  }

  result &= DeinitPower();

  minimal_drivers_initialized_ = false;
  return result;
}

bool TDisplayP4AirDriver::PrepareDriversForPowerOff() {
  if (!async_init_manager_.StopAndWait(kInitializationShutdownTimeoutMs)) {
    LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
        "Wait for asynchronous initialization before power off timed out\n");
    return false;
  }

  bool result = true;
  result &= DeinitScreenBacklight();
  result &= DeinitTouch();
  result &= DeinitScreen();
  result &= DeinitBhi260ap();
  result &= DeinitQmc6309();
  result &= DeinitAw86224();
  result &= DeinitEs8390();
  result &= DeinitLr2021();
  result &= DeinitSt25r3916();
  result &= DeinitNrf9151();
  result &= SetCameraPowerEnabled(false);
  result &= SetEsp32c5PowerEnabled(false);
  result &= DeinitSdmmc(false);

  // 将外设复位、电源使能及控制引脚设置为关机安全电平。
  if (chip_status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kNrf9151En, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kUsbPhyPowerEn, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAw86224Rst, 0);
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kBhi260apRst, 1);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAdl161Rst, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLr2021PowerEn, 0);
    result &=
        chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32p4Esp32c5UartSwitch, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 0);
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kTouchRst, 1);
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kScreenRst, 1);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 1);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kNs4150En, 0);
  }

  result &= PrepareMinimalDriversForPowerOff();
  return result;
}

bool TDisplayP4AirDriver::EnterEsp32c5DownloadMode() {
  if (!chip_status_.xl9535.init_flag) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "EnterEsp32c5DownloadMode failed\n");
    return false;
  }

  bool result = true;

  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 0);
  platform_hal_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 1);
  platform_hal_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 1);

  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "EnterEsp32c5DownloadMode failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::SetUartTarget(UartTarget target) {
  if (!chip_status_.xl9535.init_flag) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "SetUartTarget failed\n");
    return false;
  }

  bool result = true;
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32p4Esp32c5UartSwitch,
      target == UartTarget::kEsp32c5 ? 1 : 0);

  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "SetUartTarget failed\n");
  }
  return result;
}

}  // namespace lilygo_device_driver
