/*
 * @Description: T-Display-P4-Air 板级设备驱动实现
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:51:14
 * @LastEditTime: 2026-08-20 16:13:11
 * @License: GPL 3.0
 */
#include "t_display_p4_air_driver.h"

#include "driver/gpio.h"
#include "firmware/bhi260ap/BHI260AP.fw.h"
#include "sdmmc_cmd.h"

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
constexpr uint32_t kInitializationShutdownTimeoutMs = 5 * 1000;
constexpr int kEs8389DacMiscControl2Register = 0x45;
constexpr int kEs8389Dac1OutputInvertMask = 1 << 6;

}  // namespace

TDisplayP4AirDriver& TDisplayP4AirDriver::GetInstance() {
  static TDisplayP4AirDriver* instance = new TDisplayP4AirDriver();
  return *instance;
}

const device::ScreenInfo& TDisplayP4AirDriver::screen_info() const {
  return *(screen_info_ == nullptr ? kDefaultScreenInfo : screen_info_);
}

void TDisplayP4AirDriver::CreateDrivers() {
  if (tool_ != nullptr) {
    return;
  }
  tool_ = std::make_unique<cpp_bus_driver::Tool>();

  bus_.sgm38121_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      gpio::sgm38121::kSda, gpio::sgm38121::kScl, I2C_NUM_0);
  bus_.xl9535_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      gpio::xl9535::kSda, gpio::xl9535::kScl, I2C_NUM_1);

  bus_.axp517_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.xl9535_i2c_bus);
  bus_.hi8561_i2c_touch_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.sgm38121_i2c_bus);
  bus_.bhi260ap_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.sgm38121_i2c_bus);
  bus_.qmc6310n_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.xl9535_i2c_bus);
  bus_.aw86224_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.xl9535_i2c_bus);
  bus_.st25r3916_i2c_bus =
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
  chip_.st25r3916 =
      std::make_unique<stsw_st25rfal002_cpp_bus_driver::St25r3916x>(
          bus_.st25r3916_i2c_bus, gpio::st25r3916::kInt,
          device::st25r3916::kI2cAddress);
  chip_.hi8561_touch = std::make_unique<cpp_bus_driver::Hi8561Touch>(
      bus_.hi8561_i2c_touch_bus, device::hi8561::kTouchI2cAddress);
  chip_.bhi260ap = std::make_unique<bhi2xy_sensorapi_cpp_bus_driver::Bhi2xy>(
      bus_.bhi260ap_i2c_bus, device::bhi260ap::kI2cAddress);
  chip_.qmc6310n = std::make_unique<SensorQMC6310>();
  chip_.sy7200a =
      std::make_unique<cpp_bus_driver::Pwm>(gpio::sy7200a::kEn);
  chip_.nrf9151 =
      std::make_unique<cpp_bus_driver::Nrf9151>(bus_.nrf9151_uart_bus);
  chip_.lr1121 =
      std::make_unique<usp_cpp_bus_driver::Lr11xx>(bus_.lr1121_spi_bus,
          gpio::lr1121::kBusy, gpio::lr1121::kCs, [this](bool released) {
            return tool_ != nullptr &&
                   tool_->GpioWrite(gpio::lr1121::kRst, released);
          });
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

bool TDisplayP4AirDriver::InitMinimal() {
  CreateDrivers();
  if (InitMinimalDrivers()) {
    return true;
  }
  PrepareMinimalDriversForPowerOff();
  return false;
}

bool TDisplayP4AirDriver::InitMinimalDrivers() {
  if (minimal_drivers_initialized_) {
    return true;
  }
  if (!InitPower() || !InitAxp517()) {
    return false;
  }
  minimal_drivers_initialized_ = true;
  return true;
}

bool TDisplayP4AirDriver::InitDrivers(InitMode mode) {
  bool result = InitMinimalDrivers();
  result &= InitXl9535();
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
                   "InitScreenImuTask", 8192, this, 3);

    result &= async_init_manager_.StartTask(
                   [](void* arg) {
                     auto* self = static_cast<TDisplayP4AirDriver*>(arg);
                     if (!self->async_init_manager_.stop_requested()) {
                       self->InitQmc6310n();
                     }
                     self->async_init_manager_.FinishTask();
                   },
                   "InitQmc6310nTask", 4096, this, 3);

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
                       self->InitLr1121();
                     }
                     self->async_init_manager_.FinishTask();
                   },
                   "InitLr1121Task", 4096, this, 3);

    result &= async_init_manager_.StartTask(
                   [](void* arg) {
                     auto* self = static_cast<TDisplayP4AirDriver*>(arg);
                     if (!self->async_init_manager_.stop_requested() &&
                         !self->InitEs8389()) {
                       self->SetNs4150Enabled(false);
                     }
                     self->async_init_manager_.FinishTask();
                   },
                   "InitEs8389Task", 4096, this, 3);

  } else {
    const bool screen_initialized = InitScreen();
    result &= screen_initialized;
    if (screen_initialized) {
      result &= InitTouch();
      result &= InitScreenBacklight();
    }

    result &= InitBhi260ap();
    result &= InitQmc6310n();

    result &= InitAw86224();
    result &= InitLr1121();

    result &= InitEs8389();
  }

  return result;
}

bool TDisplayP4AirDriver::InitAxp517() {
  if (IsAxp517Ready()) {
    return true;
  }
  if (!chip_.axp517->Init()) {
    status_.axp517.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitAxp517 failed\n");
    return false;
  }

  const cpp_bus_driver::Axp517::AdcChannel adc_channel = {
      .battery_discharge_current_measure = true,
      .battery_charge_current_measure = true,
      .chip_temperature_measure = true,
      .ts_value_measure = true,
      .battery_voltage_measure = true,
  };
  bool result = true;
  result &= chip_.axp517->SetAdcChannel(adc_channel);
  result &= chip_.axp517->SetBoostVoltage(5000);
  result &= chip_.axp517->SetForceRbfetEnable(false);
  result &= chip_.axp517->SetBoostEnable(false);
  result &= chip_.axp517->SetTypeCDetectEnable(true);
  result &= chip_.axp517->SetVbusDetectEnable(true);
  result &= chip_.axp517->SetPdRole(false, false);
  status_.axp517.init_flag = result;
  if (!result) {
    chip_.axp517->Deinit(false);
  }
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitAxp517 success\n" : "InitAxp517 failed\n");
  return result;
}

bool TDisplayP4AirDriver::InitXl9535() {
  status_.xl9535.init_flag = false;
  if (chip_.xl9535 == nullptr || !chip_.xl9535->Init()) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitXl9535 failed\n");
    return false;
  }

  constexpr auto kOutput = cpp_bus_driver::Xl95x5::Mode::kOutput;
  // 先预装关闭和复位状态，再切换输出方向，避免默认高电平产生无效脉冲。
  bool result = true;
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAdl161Trig, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAdl161Rst, 0);
  // ESP32-P4 只有在 USB PHY 电源保持开启时才能降低功耗；关闭该电源会
  // 产生约 20 mA 功耗，因此初始化后默认保持开启。
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kUsbPhyPowerEn, 1);
  result &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kEsp32p4Esp32c5UartSwitch, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 1);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLed1, 1);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kAdl161Trig, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kAdl161Rst, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kUsbPhyPowerEn, kOutput);
  result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kEsp32p4Esp32c5UartSwitch, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kEsp32c5En, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kEsp32c5Boot, kOutput);
  result &= chip_.xl9535->SetGpioMode(gpio::xl9535::kLed1, kOutput);

  if (!result) {
    chip_.xl9535->Deinit(false);
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitXl9535 failed\n");
    return false;
  }

  status_.xl9535.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitXl9535 success\n");
  return true;
}

bool TDisplayP4AirDriver::InitSgm38121() {
  if (chip_.sgm38121 == nullptr || !chip_.sgm38121->Init()) {
    status_.sgm38121.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSgm38121 failed\n");
    return false;
  }

  bool result = true;
#if defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_SC2336)
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV2710)
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 3000);
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV5645)
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
#endif

  status_.sgm38121.init_flag = result;
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

bool TDisplayP4AirDriver::InitBhi260ap() {
  if (IsBhi260apReady()) {
    return true;
  }
  if (chip_.bhi260ap == nullptr || bus_.bhi260ap_i2c_bus == nullptr) {
    status_.bhi260ap.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBhi260ap failed\n");
    return false;
  }

  if (!status_.xl9535.init_flag) {
    status_.bhi260ap.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBhi260ap failed\n");
    return false;
  }
  bool reset_pin_initialized = true;
  reset_pin_initialized &=
      chip_.xl9535->GpioWrite(gpio::xl9535::kBhi260apRst, 0);
  reset_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kBhi260apRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_pin_initialized) {
    status_.bhi260ap.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBhi260ap failed\n");
    return false;
  }
  tool_->DelayMs(2);
  if (!chip_.xl9535->GpioWrite(gpio::xl9535::kBhi260apRst, 1)) {
    status_.bhi260ap.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBhi260ap failed\n");
    return false;
  }
  tool_->DelayMs(120);

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
  status_.bhi260ap.init_flag = result;

  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
        "InitBhi260ap success (kernel version: %u)\n",
        static_cast<unsigned int>(chip_.bhi260ap->kernel_version()));
  } else {
    const int8_t last_error = chip_.bhi260ap->last_error();
    chip_.bhi260ap->Deinit(false);
    chip_.xl9535->GpioWrite(gpio::xl9535::kBhi260apRst, 0);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitBhi260ap failed (error code: %d)\n", static_cast<int>(last_error));
  }
  return result;
}

bool TDisplayP4AirDriver::InitQmc6310n() {
  if (IsQmc6310nReady()) {
    return true;
  }
  if (chip_.qmc6310n == nullptr || bus_.qmc6310n_i2c_bus == nullptr ||
      !bus_.qmc6310n_i2c_bus->InitBus()) {
    status_.qmc6310n.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitQmc6310n failed\n");
    return false;
  }

  bool result = chip_.qmc6310n->begin(
      bus_.qmc6310n_i2c_bus->bus_handle(), device::qmc6310n::kI2cAddress);
  if (result) {
    chip_.qmc6310n->setOffset(0, 0, 0);
    result &= chip_.qmc6310n->configMagnetometer(
        OperationMode::CONTINUOUS_MEASUREMENT, MagFullScaleRange::FS_8G, 200.0f,
        MagOverSampleRatio::OSR_1, MagDownSampleRatio::DSR_1);
    result &= chip_.qmc6310n->setOperationMode(OperationMode::SUSPEND);
  }
  status_.qmc6310n.init_flag = result;

  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitQmc6310n success\n" : "InitQmc6310n failed\n");
  return result;
}

bool TDisplayP4AirDriver::InitHi8561() {
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

bool TDisplayP4AirDriver::InitHi8561Touch() {
  if (IsHi8561TouchReady()) {
    return true;
  }

  status_.hi8561_touch.init_flag = false;
  if (!status_.xl9535.init_flag || chip_.hi8561_touch == nullptr ||
      bus_.hi8561_i2c_touch_bus == nullptr) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }

  bool reset_result = true;
  // Air 板的触摸复位信号经过反相，高电平为复位状态。
  reset_result &= chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 1);
  reset_result &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kTouchRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_result) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }
  tool_->DelayMs(10);

  if (!chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 0)) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  }
  tool_->DelayMs(100);

  if (chip_.hi8561_touch->Init(device::hi8561::kI2cFrequencyHz)) {
    status_.hi8561_touch.init_flag = true;
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561Touch success\n");
    return true;
  }

  chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 1);
  LogMessage(
      LogLevel::kError, __FILE__, __LINE__, "InitHi8561Touch failed\n");
  return false;
}

bool TDisplayP4AirDriver::InitSy7200a() {
  if (chip_.sy7200a != nullptr && chip_.sy7200a->IsInitialized()) {
    status_.sy7200a.init_flag = true;
    return true;
  }
  if (chip_.sy7200a == nullptr) {
    status_.sy7200a.init_flag = false;
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitSy7200a failed\n");
    return false;
  }

  cpp_bus_driver::Pwm::Config config;
  config.timer = LEDC_TIMER_0;
  config.channel = LEDC_CHANNEL_0;
  config.frequency_hz = device::sy7200a::kPwmFrequencyHz;
  if (!chip_.sy7200a->Init(config)) {
    status_.sy7200a.init_flag = false;
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitSy7200a failed\n");
    return false;
  }

  status_.sy7200a.init_flag = true;
  LogMessage(
      LogLevel::kInfo, __FILE__, __LINE__, "InitSy7200a success\n");
  return true;
}

bool TDisplayP4AirDriver::InitAw86224() {
  if (IsAw86224Ready()) {
    return true;
  }
  if (!chip_.aw86224->Init(device::aw86224::kI2cFrequencyHz)) {
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

bool TDisplayP4AirDriver::InitSt25r3916() {
  if (IsSt25r3916Ready()) {
    return true;
  }
  if (chip_.st25r3916 == nullptr || bus_.st25r3916_i2c_bus == nullptr) {
    status_.st25r3916.init_flag = false;
    status_.st25r3916.result = RFAL_ERR_INVALID_HANDLE;
    status_.st25r3916.platform_error =
        stsw_st25rfal002_cpp_bus_driver::PlatformError::kInvalidConfiguration;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSt25r3916 failed\n");
    return false;
  }

  status_.st25r3916.result = chip_.st25r3916->Init();
  status_.st25r3916.platform_error = chip_.st25r3916->platform_error();
  status_.st25r3916.init_flag =
      status_.st25r3916.result == RFAL_ERR_NONE &&
      status_.st25r3916.platform_error ==
          stsw_st25rfal002_cpp_bus_driver::PlatformError::kNone &&
      chip_.st25r3916->initialized();

  if (!status_.st25r3916.init_flag) {
    chip_.st25r3916->Deinit(false);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitSt25r3916 failed (RFAL: %u, platform: %u)\n",
        static_cast<unsigned int>(status_.st25r3916.result),
        static_cast<unsigned int>(status_.st25r3916.platform_error));
    return false;
  }

  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSt25r3916 success\n");
  return true;
}

bool TDisplayP4AirDriver::InitEs8389() {
  if (IsEs8389Ready()) {
    return true;
  }
  if ((bus_.xl9535_i2c_bus == nullptr) || (bus_.es8389_i2s_bus == nullptr)) {
    status_.es8389.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  i2c_master_bus_handle_t i2c_bus_handle = bus_.xl9535_i2c_bus->bus_handle();
  if (i2c_bus_handle == nullptr) {
    status_.es8389.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  audio_codec_i2c_cfg_t i2c_cfg = {
      .port = static_cast<uint8_t>(I2C_NUM_1),
      .addr = static_cast<uint8_t>(device::es8389::kI2cAddress << 1),
      .bus_handle = i2c_bus_handle,
  };
  es8389_ctrl_if_ = audio_codec_new_i2c_ctrl(&i2c_cfg);

  if (es8389_ctrl_if_ == nullptr || !bus_.es8389_i2s_bus->Init(
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
                LogLevel::kWarning, __FILE__, __LINE__, "Value out of range\n");
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
                LogLevel::kWarning, __FILE__, __LINE__, "Value out of range\n");
            return i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_16BIT;
        }
      }(device::es8389::kBitsPerSample))) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  audio_codec_i2s_cfg_t i2s_cfg = {
      .port = static_cast<uint8_t>(bus_.es8389_i2s_bus->port()),
      .rx_handle = bus_.es8389_i2s_bus->rx_handle(),
      .tx_handle = bus_.es8389_i2s_bus->tx_handle(),
      .clk_src = static_cast<int>(I2S_CLK_SRC_DEFAULT),
  };
  es8389_data_if_ = audio_codec_new_i2s_data(&i2s_cfg);
  if (es8389_data_if_ == nullptr) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }
  if (!status_.xl9535.init_flag || chip_.xl9535 == nullptr) {
    status_.es8389.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }
  bool amplifier_pin_initialized = true;
  amplifier_pin_initialized &=
      chip_.xl9535->GpioWrite(gpio::xl9535::kNs4150En, 0);
  amplifier_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kNs4150En, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!amplifier_pin_initialized) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  es8389_gpio_if_ = audio_codec_new_gpio();
  if (es8389_gpio_if_ == nullptr) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

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
  if (es8389_codec_if_ == nullptr) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  esp_codec_dev_cfg_t output_dev_cfg = {
      .dev_type = ESP_CODEC_DEV_TYPE_OUT,
      .codec_if = es8389_codec_if_,
      .data_if = es8389_data_if_,
  };
  es8389_output_codec_dev_ = esp_codec_dev_new(&output_dev_cfg);
  if (es8389_output_codec_dev_ == nullptr) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  esp_codec_dev_cfg_t input_dev_cfg = {
      .dev_type = ESP_CODEC_DEV_TYPE_IN,
      .codec_if = es8389_codec_if_,
      .data_if = es8389_data_if_,
  };
  es8389_input_codec_dev_ = esp_codec_dev_new(&input_dev_cfg);
  if (es8389_input_codec_dev_ == nullptr) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
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
  esp_codec_dev_sample_info_t input_sample_info = output_sample_info;
  const bool output_opened =
      esp_codec_dev_open(es8389_output_codec_dev_, &output_sample_info) ==
      ESP_CODEC_DEV_OK;
  const bool input_opened =
      output_opened &&
      esp_codec_dev_open(es8389_input_codec_dev_, &input_sample_info) ==
          ESP_CODEC_DEV_OK;
  bool result = output_opened && input_opened;
  if (result) {
    result &= ConfigureEs8389OutputPolarity();
    result &= (esp_codec_dev_set_out_vol(es8389_output_codec_dev_, 100) ==
               ESP_CODEC_DEV_OK);
    result &= (esp_codec_dev_set_in_gain(es8389_input_codec_dev_, 20.0f) ==
               ESP_CODEC_DEV_OK);
  }

  // 关闭操作属于回滚，必须尽量释放所有已经打开的通道。
  if (input_opened) {
    result &=
        (esp_codec_dev_close(es8389_input_codec_dev_) == ESP_CODEC_DEV_OK);
  }
  if (output_opened) {
    result &=
        (esp_codec_dev_close(es8389_output_codec_dev_) == ESP_CODEC_DEV_OK);
  }
  result &= SetNs4150Enabled(false);
  es8389_operating_mode_ = Es8389OperatingMode::kSleep;
  status_.es8389.init_flag = result;
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitEs8389 success\n");
  } else {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::InitLr1121() {
  if (IsLr1121Ready()) {
    return true;
  }
  status_.lr1121.init_flag = false;
  if (chip_.lr1121 == nullptr || !status_.xl9535.init_flag) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr1121 power enable failed\n");
    return false;
  }
  bool power_enabled = true;
  power_enabled &=
      chip_.xl9535->GpioWrite(gpio::xl9535::kLr1121PowerEn, 0);
  power_enabled &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kLr1121PowerEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  power_enabled &=
      chip_.xl9535->GpioWrite(gpio::xl9535::kLr1121PowerEn, 1);
  if (!power_enabled) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr1121 power enable failed\n");
    return false;
  }
  tool_->DelayMs(10);

  bool transport_initialized = true;
  transport_initialized &= tool_->SetGpioMode(
      gpio::lr1121::kRst, cpp_bus_driver::Tool::GpioMode::kOutput);
  transport_initialized &= tool_->SetGpioMode(
      gpio::lr1121::kInt, cpp_bus_driver::Tool::GpioMode::kInput);
  if (!transport_initialized ||
      !chip_.lr1121->Init(device::lr1121::kSpiFrequencyHz)) {
    tool_->GpioWrite(gpio::lr1121::kRst, 0);
    chip_.xl9535->GpioWrite(gpio::xl9535::kLr1121PowerEn, 0);
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitLr1121 transport failed\n");
    return false;
  }

  lr11xx_system_version_t version = {};
  if (chip_.lr1121->Invoke(lr11xx_system_get_version, &version) !=
          LR11XX_STATUS_OK ||
      version.type != LR11XX_SYSTEM_VERSION_TYPE_LR1121) {
    chip_.lr1121->Deinit();
    tool_->GpioWrite(gpio::lr1121::kRst, 0);
    chip_.xl9535->GpioWrite(gpio::xl9535::kLr1121PowerEn, 0);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr1121 chip detection failed\n");
    return false;
  }

  const lr11xx_system_rfswitch_cfg_t rf_switch_config = {
      .enable = LR11XX_SYSTEM_RFSW0_HIGH | LR11XX_SYSTEM_RFSW1_HIGH,
      .standby = 0x00,
      .rx = LR11XX_SYSTEM_RFSW1_HIGH,
      .tx = 0x00,
      .tx_hp = LR11XX_SYSTEM_RFSW0_HIGH,
      .tx_hf = 0x00,
      .gnss = 0x00,
      .wifi = 0x00,
  };
  usp_cpp_bus_driver::Lr11xx::LoraConfig lora_config = {
      .frequency_hz = 868000000U,
      .modulation =
          {
              .sf = LR11XX_RADIO_LORA_SF12,
              .bw = LR11XX_RADIO_LORA_BW_125,
              .cr = LR11XX_RADIO_LORA_CR_4_7,
              .ldro = 1,
          },
      .packet =
          {
              .preamble_len_in_symb = 8,
              .header_type = LR11XX_RADIO_LORA_PKT_EXPLICIT,
              .pld_len_in_bytes = 255,
              .crc = LR11XX_RADIO_LORA_CRC_ON,
              .iq = LR11XX_RADIO_LORA_IQ_STANDARD,
          },
      .sync_word = 0x12,
      .rx_boosted = true,
      .pa =
          {
              .pa_sel = LR11XX_RADIO_PA_SEL_HP,
              .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT,
              .pa_duty_cycle = 0x04,
              .pa_hp_sel = 0x07,
          },
      .output_power_dbm = 22,
      .ramp_time = LR11XX_RADIO_RAMP_48_US,
  };
  const lr11xx_system_sleep_cfg_t sleep_config = {
      .is_warm_start = true,
      .is_rtc_timeout = false,
  };

  bool result = true;
  result &= (chip_.lr1121->Invoke(lr11xx_system_set_standby,
                 LR11XX_SYSTEM_STANDBY_CFG_RC) == LR11XX_STATUS_OK);
  result &= (chip_.lr1121->Invoke(lr11xx_system_set_tcxo_mode,
                 LR11XX_SYSTEM_TCXO_CTRL_1_6V, 163U) == LR11XX_STATUS_OK);
  result &= (chip_.lr1121->Invoke(lr11xx_radio_set_rx_tx_fallback_mode,
                 LR11XX_RADIO_FALLBACK_STDBY_RC) == LR11XX_STATUS_OK);
  result &= (chip_.lr1121->Invoke(lr11xx_system_clear_irq_status,
                 LR11XX_SYSTEM_IRQ_ALL_MASK) == LR11XX_STATUS_OK);
  result &= (chip_.lr1121->Invoke(lr11xx_system_set_dio_irq_params,
                 LR11XX_SYSTEM_IRQ_NONE, LR11XX_SYSTEM_IRQ_NONE) ==
             LR11XX_STATUS_OK);
  result &= (chip_.lr1121->Invoke(lr11xx_system_calibrate,
                 static_cast<uint8_t>(
                     LR11XX_SYSTEM_CALIB_LF_RC_MASK |
                     LR11XX_SYSTEM_CALIB_HF_RC_MASK |
                     LR11XX_SYSTEM_CALIB_PLL_MASK |
                     LR11XX_SYSTEM_CALIB_ADC_MASK |
                     LR11XX_SYSTEM_CALIB_IMG_MASK |
                     LR11XX_SYSTEM_CALIB_PLL_TX_MASK)) == LR11XX_STATUS_OK);
  result &= (chip_.lr1121->Invoke(
                 lr11xx_system_drive_dio_in_sleep_mode, true) ==
             LR11XX_STATUS_OK);
  result &= (chip_.lr1121->Invoke(lr11xx_system_set_dio_as_rf_switch,
                 &rf_switch_config) == LR11XX_STATUS_OK);
  result &= chip_.lr1121->Configure(lora_config);
  result &= (chip_.lr1121->Invoke(
                 lr11xx_system_set_sleep, sleep_config, 0U) ==
             LR11XX_STATUS_OK);
  if (!result) {
    chip_.lr1121->Deinit();
    tool_->GpioWrite(gpio::lr1121::kRst, 0);
    chip_.xl9535->GpioWrite(gpio::xl9535::kLr1121PowerEn, 0);
    status_.lr1121.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr1121 configuration failed\n");
    return false;
  }

  status_.lr1121.init_flag = result;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "InitLr1121 success (hw: %u, fw: 0x%04x)\n",
      static_cast<unsigned>(version.hw), static_cast<unsigned>(version.fw));
  return result;
}

bool TDisplayP4AirDriver::InitNrf9151() {
  if (IsNrf9151Ready()) {
    return true;
  }
  if (chip_.nrf9151 == nullptr || bus_.nrf9151_uart_bus == nullptr ||
      !status_.xl9535.init_flag) {
    status_.nrf9151.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitNrf9151 failed: driver, UART bus, or modem power is unavailable\n");
    return false;
  }

  bool power_enabled = true;
  power_enabled &=
      chip_.xl9535->GpioWrite(gpio::xl9535::kNrf9151En, 0);
  power_enabled &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kNrf9151En, cpp_bus_driver::Xl95x5::Mode::kOutput);
  power_enabled &=
      chip_.xl9535->GpioWrite(gpio::xl9535::kNrf9151En, 1);
  if (!power_enabled) {
    status_.nrf9151.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitNrf9151 failed: driver, UART bus, or modem power is unavailable\n");
    return false;
  }

  const bool result =
      chip_.nrf9151->Init(device::nrf9151::kDefaultBaudRate);

  if (!result) {
    if (!chip_.xl9535->GpioWrite(gpio::xl9535::kNrf9151En, 0)) {
      LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
          "Disable nRF9151 power after init failure failed\n");
    }
  }

  if (result) {
    cpp_bus_driver::Nrf9151::SerialModemVersion serial_modem_version;
    if (chip_.nrf9151->GetSerialModemVersion(
            &serial_modem_version,
            cpp_bus_driver::Nrf9151::kDefaultCommandTimeoutMs)) {
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
    if (chip_.nrf9151->GetModemFirmwareVersion(&modem_firmware_version,
            cpp_bus_driver::Nrf9151::kDefaultCommandTimeoutMs)) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
          "Nrf9151 modem firmware version: %s\n",
          modem_firmware_version.c_str());
    } else {
      LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
          "Get Nrf9151 modem firmware version failed\n");
    }
  }

  status_.nrf9151.init_flag = result;
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
  bool power_enabled = true;
  power_enabled &= tool_->SetGpioMode(
      gpio::power::kEnable3v3, cpp_bus_driver::Tool::GpioMode::kOutput);
  power_enabled &= tool_->GpioWrite(gpio::power::kEnable3v3, 1);
  if (!power_enabled) {
    return false;
  }
  if (!InitLdoPower(3, 2500)) {
    tool_->GpioWrite(gpio::power::kEnable3v3, 0);
    return false;
  }
  if (!InitLdoPower(4, 3300)) {
    DeinitLdoPower(3);
    tool_->GpioWrite(gpio::power::kEnable3v3, 0);
    return false;
  }
  power_initialized_ = true;
  return true;
}

bool TDisplayP4AirDriver::InitScreen() {
  if (!status_.xl9535.init_flag) {
    return false;
  }
  bool reset_pin_initialized = true;
  reset_pin_initialized &=
      chip_.xl9535->GpioWrite(gpio::xl9535::kScreenRst, 1);
  reset_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kScreenRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_pin_initialized) {
    return false;
  }
  tool_->DelayMs(10);
  if (!chip_.xl9535->GpioWrite(gpio::xl9535::kScreenRst, 0)) {
    return false;
  }
  tool_->DelayMs(120);

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
                LogLevel::kWarning, __FILE__, __LINE__, "Value out of range\n");
            return cpp_bus_driver::HardwareMipi::ColorFormat::kRgb565;
        }
      }(screen.bits_per_pixel));
  chip_.hi8561 = std::make_unique<cpp_bus_driver::Hi8561>(bus_.screen_mipi_bus);

  const bool result = InitHi8561();
  if (!result) {
    DeinitScreen();
  }
  return result;
}

bool TDisplayP4AirDriver::InitTouch() { return InitHi8561Touch(); }

bool TDisplayP4AirDriver::InitScreenBacklight() {
  return InitSy7200a();
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

  size_t total = 0;
  size_t used = 0;
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
      "Partition size: total: %zu bytes, used: %zu bytes\n", total, used);

  if (used > total) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "Number of used bytes cannot be larger than total performing "
        "esp_spiffs_check\n");
    result = esp_spiffs_check(conf.partition_label);
    if (result != ESP_OK) {
      LogMessage(LogLevel::kError, __FILE__, __LINE__,
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
  if (base_path == nullptr || base_path[0] == '\0') {
    return false;
  }
  if (sd_card_ != nullptr && !DeinitSdmmc()) {
    return false;
  }
  if (!status_.xl9535.init_flag || chip_.xl9535 == nullptr) {
    return false;
  }
  bool power_enabled = true;
  power_enabled &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
  power_enabled &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kSdPowerEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  power_enabled &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 1);
  if (!power_enabled) {
    return false;
  }

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
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "esp_vfs_fat_sdmmc_mount failed (error code: %#X)\n", result);
    status_.sd_card.init_flag = false;
    sd_card_ = nullptr;
    sd_card_base_path_.clear();
    sd_card_uses_spi_ = false;
    chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
    return false;
  }

  sdmmc_card_print_info(stdout, card);
  sd_card_ = card;
  sd_card_base_path_ = base_path;
  sd_card_uses_spi_ = false;
  status_.sd_card.init_flag = true;
  return true;
}

bool TDisplayP4AirDriver::InitSdspi(
    const char* base_path, spi_host_device_t host_id, int max_freq_khz) {
  if (base_path == nullptr || base_path[0] == '\0') {
    return false;
  }
  if (sd_card_ != nullptr && !DeinitSdmmc()) {
    return false;
  }
  if (!status_.xl9535.init_flag || chip_.xl9535 == nullptr) {
    return false;
  }
  bool power_enabled = true;
  power_enabled &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
  power_enabled &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kSdPowerEn, cpp_bus_driver::Xl95x5::Mode::kOutput);
  power_enabled &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 1);
  if (!power_enabled) {
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
    sd_card_uses_spi_ = false;
    chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
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
    sd_card_uses_spi_ = false;
    spi_bus_free(host_id);
    chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
    return false;
  }

  sdmmc_card_print_info(stdout, card);
  sd_card_ = card;
  sd_card_base_path_ = base_path;
  sd_card_uses_spi_ = true;
  sd_card_spi_host_id_ = host_id;
  status_.sd_card.init_flag = true;
  return true;
}

bool TDisplayP4AirDriver::DeinitBhi260ap() {
  bool result = true;
  if (status_.bhi260ap.init_flag && chip_.bhi260ap != nullptr) {
    result &= SetBhi260apSleep(true);
    result &= chip_.bhi260ap->Deinit(false);
  }
  if (status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kBhi260apRst, 0);
  }
  status_.bhi260ap.init_flag = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitQmc6310n() {
  bool result = true;
  if (status_.qmc6310n.init_flag && chip_.qmc6310n != nullptr) {
    result = chip_.qmc6310n->setOperationMode(OperationMode::SUSPEND);
  }
  status_.qmc6310n.init_flag = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitAw86224() {
  bool result = true;
  if (status_.aw86224.init_flag && chip_.aw86224 != nullptr) {
    result &= chip_.aw86224->StopRamPlaybackWaveform();
    result &= chip_.aw86224->Deinit(false);
  }
  status_.aw86224.init_flag = false;
  status_.aw86224.ram_waveform_info = {};
  return result;
}

bool TDisplayP4AirDriver::DeinitSt25r3916() {
  if (!status_.st25r3916.init_flag) {
    return true;
  }

  status_.st25r3916.result = chip_.st25r3916->Deinit(false);
  status_.st25r3916.platform_error = chip_.st25r3916->platform_error();
  status_.st25r3916.init_flag = false;
  return status_.st25r3916.result == RFAL_ERR_NONE &&
         status_.st25r3916.platform_error ==
             stsw_st25rfal002_cpp_bus_driver::PlatformError::kNone;
}

bool TDisplayP4AirDriver::DeinitLr1121() {
  bool result = true;
  if (status_.lr1121.init_flag && chip_.lr1121 != nullptr) {
    result &= SetLr1121OperatingMode(Lr1121OperatingMode::kSleep);
    result &= chip_.lr1121->Deinit(false);
    result &= tool_->GpioWrite(gpio::lr1121::kRst, 0);
  }
  if (status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLr1121PowerEn, 0);
  }
  status_.lr1121.init_flag = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitNrf9151() {
  bool result = true;
  if (status_.nrf9151.init_flag && chip_.nrf9151 != nullptr) {
    result &= chip_.nrf9151->Deinit();
  }
  if (status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kNrf9151En, 0);
  }
  status_.nrf9151.init_flag = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitEs8389() {
  bool result = true;
  result &= SetEs8389OperatingMode(Es8389OperatingMode::kSleep);

  if (es8389_input_codec_dev_ != nullptr) {
    esp_codec_dev_delete(es8389_input_codec_dev_);
    es8389_input_codec_dev_ = nullptr;
  }
  if (es8389_output_codec_dev_ != nullptr) {
    esp_codec_dev_delete(es8389_output_codec_dev_);
    es8389_output_codec_dev_ = nullptr;
  }
  if (es8389_codec_if_ != nullptr) {
    result &=
        (audio_codec_delete_codec_if(es8389_codec_if_) == ESP_CODEC_DEV_OK);
    es8389_codec_if_ = nullptr;
  }
  if (es8389_ctrl_if_ != nullptr) {
    result &= (audio_codec_delete_ctrl_if(es8389_ctrl_if_) == ESP_CODEC_DEV_OK);
    es8389_ctrl_if_ = nullptr;
  }
  if (es8389_data_if_ != nullptr) {
    result &= (audio_codec_delete_data_if(es8389_data_if_) == ESP_CODEC_DEV_OK);
    es8389_data_if_ = nullptr;
  }
  if (es8389_gpio_if_ != nullptr) {
    result &= (audio_codec_delete_gpio_if(es8389_gpio_if_) == ESP_CODEC_DEV_OK);
    es8389_gpio_if_ = nullptr;
  }
  if (bus_.es8389_i2s_bus != nullptr) {
    result &= bus_.es8389_i2s_bus->Deinit();
  }

  status_.es8389.init_flag = false;
  es8389_operating_mode_ = Es8389OperatingMode::kSleep;
  return result;
}

bool TDisplayP4AirDriver::DeinitPower() {
  bool result = true;
  result &= DeinitLdoPower(3);
  result &= DeinitLdoPower(4);
  if (tool_ != nullptr) {
    result &= tool_->GpioWrite(gpio::power::kEnable3v3, 0);
  }
  power_initialized_ = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitScreen() {
  bool result = true;
  if (status_.hi8561.init_flag) {
    result &= chip_.hi8561->Deinit();
  }
  if (bus_.screen_mipi_bus != nullptr) {
    result &= bus_.screen_mipi_bus->Deinit();
    bus_.screen_mipi_bus.reset();
  }
  if (status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kScreenRst, 1);
  }
  status_.hi8561.init_flag = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitTouch() {
  bool result = true;
  if (status_.hi8561_touch.init_flag) {
    result &= chip_.hi8561_touch->Deinit(false);
  }
  if (status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 1);
  }
  status_.hi8561_touch.init_flag = false;
  return result;
}

bool TDisplayP4AirDriver::DeinitScreenBacklight() {
  if (chip_.sy7200a == nullptr || !chip_.sy7200a->IsInitialized()) {
    status_.sy7200a.init_flag = false;
    return true;
  }

  const bool result = chip_.sy7200a->Deinit();
  status_.sy7200a.init_flag = chip_.sy7200a->IsInitialized();
  return result;
}

bool TDisplayP4AirDriver::DeinitSdmmc(bool release_bus) {
  if (sd_card_ == nullptr) {
    status_.sd_card.init_flag = false;
    sd_card_base_path_.clear();
    sd_card_uses_spi_ = false;
    return !status_.xl9535.init_flag || chip_.xl9535 == nullptr ||
           chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
  }

  const bool uses_spi = sd_card_uses_spi_;
  const spi_host_device_t spi_host_id = sd_card_spi_host_id_;
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
  sd_card_uses_spi_ = false;
  status_.sd_card.init_flag = false;
  bool deinit_result = true;
  if (uses_spi && release_bus) {
    const esp_err_t spi_result = spi_bus_free(spi_host_id);
    if (spi_result != ESP_OK) {
      LogMessage(LogLevel::kError, __FILE__, __LINE__,
          "spi_bus_free failed (error code: %#X)\n", spi_result);
      deinit_result = false;
    }
  }
  if (status_.xl9535.init_flag && chip_.xl9535 != nullptr) {
    deinit_result &=
        chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
  }
  return deinit_result;
}

bool TDisplayP4AirDriver::IsAxp517Ready() const {
  return status_.axp517.init_flag && chip_.axp517 != nullptr;
}

bool TDisplayP4AirDriver::IsXl9535Ready() const {
  return status_.xl9535.init_flag && chip_.xl9535 != nullptr;
}

bool TDisplayP4AirDriver::IsSgm38121Ready() const {
  return status_.sgm38121.init_flag && chip_.sgm38121 != nullptr;
}

bool TDisplayP4AirDriver::IsBhi260apReady() const {
  return status_.bhi260ap.init_flag && chip_.bhi260ap != nullptr &&
         chip_.bhi260ap->initialized() && chip_.bhi260ap->firmware_running();
}

bool TDisplayP4AirDriver::IsQmc6310nReady() const {
  return status_.qmc6310n.init_flag && chip_.qmc6310n != nullptr;
}

bool TDisplayP4AirDriver::IsHi8561Ready() const {
  return status_.hi8561.init_flag && chip_.hi8561 != nullptr;
}

bool TDisplayP4AirDriver::IsHi8561TouchReady() const {
  return status_.hi8561_touch.init_flag && chip_.hi8561_touch != nullptr;
}

bool TDisplayP4AirDriver::IsSy7200aReady() const {
  return status_.sy7200a.init_flag && chip_.sy7200a != nullptr &&
         chip_.sy7200a->IsInitialized();
}

bool TDisplayP4AirDriver::IsAw86224Ready() const {
  return status_.aw86224.init_flag && chip_.aw86224 != nullptr;
}

bool TDisplayP4AirDriver::IsSt25r3916Ready() const {
  return status_.st25r3916.init_flag && chip_.st25r3916 != nullptr &&
         chip_.st25r3916->initialized();
}

bool TDisplayP4AirDriver::IsEs8389Ready() const {
  return status_.es8389.init_flag && es8389_input_codec_dev_ != nullptr &&
         es8389_output_codec_dev_ != nullptr;
}

bool TDisplayP4AirDriver::IsLr1121Ready() const {
  return status_.lr1121.init_flag && chip_.lr1121 != nullptr &&
         chip_.lr1121->initialized();
}

bool TDisplayP4AirDriver::IsNrf9151Ready() const {
  return status_.nrf9151.init_flag && chip_.nrf9151 != nullptr;
}

bool TDisplayP4AirDriver::IsScreenReady() const {
  return bus_.screen_mipi_bus != nullptr &&
         bus_.screen_mipi_bus->device_handle() != nullptr && IsHi8561Ready() &&
         IsSy7200aReady();
}

bool TDisplayP4AirDriver::IsSdmmcReady() const {
  return status_.sd_card.init_flag && sd_card_ != nullptr &&
         sdmmc_get_status(sd_card_) == ESP_OK;
}

bool TDisplayP4AirDriver::SetNs4150Enabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  return chip_.xl9535->GpioWrite(
      gpio::xl9535::kNs4150En, enabled ? 1 : 0);
}

bool TDisplayP4AirDriver::SetLedEnabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  return chip_.xl9535->GpioWrite(gpio::xl9535::kLed1, enabled ? 1 : 0);
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

bool TDisplayP4AirDriver::SetQmc6310nSleep(bool sleep) {
  if (!IsQmc6310nReady()) {
    return sleep;
  }
  return chip_.qmc6310n->setOperationMode(
      sleep ? OperationMode::SUSPEND : OperationMode::CONTINUOUS_MEASUREMENT);
}

bool TDisplayP4AirDriver::SetScreenSleep(bool sleep) {
  if (!status_.hi8561.init_flag) {
    return sleep;
  }
  bool result = true;
  if (sleep) {
    result &= chip_.hi8561->SetScreenOff(true);
    result &= chip_.hi8561->SetSleep(true);
  } else {
    result &= chip_.hi8561->SetSleep(false);
    result &= chip_.hi8561->SetScreenOff(false);
  }
  return result;
}

bool TDisplayP4AirDriver::SetEs8389OperatingMode(Es8389OperatingMode mode) {
  if (!status_.es8389.init_flag) {
    return mode == Es8389OperatingMode::kSleep && SetNs4150Enabled(false);
  }
  if (mode == es8389_operating_mode_) {
    return mode != Es8389OperatingMode::kSleep || SetNs4150Enabled(false);
  }

  bool result = true;
  if (mode == Es8389OperatingMode::kSleep) {
    result &=
        (esp_codec_dev_close(es8389_input_codec_dev_) == ESP_CODEC_DEV_OK);
    result &=
        (esp_codec_dev_close(es8389_output_codec_dev_) == ESP_CODEC_DEV_OK);
    result &= SetNs4150Enabled(false);
    es8389_operating_mode_ = Es8389OperatingMode::kSleep;
  } else {
    esp_codec_dev_sample_info_t output_sample_info = {
        .bits_per_sample = device::es8389::kBitsPerSample,
        .channel = device::es8389::kChannel,
        .channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0) |
                        ESP_CODEC_DEV_MAKE_CHANNEL_MASK(1),
        .sample_rate = device::es8389::kSampleRate,
        .mclk_multiple = device::es8389::kMclkMultiple,
    };
    esp_codec_dev_sample_info_t input_sample_info = output_sample_info;
    const bool output_opened =
        esp_codec_dev_open(es8389_output_codec_dev_, &output_sample_info) ==
        ESP_CODEC_DEV_OK;
    const bool input_opened =
        output_opened &&
        esp_codec_dev_open(es8389_input_codec_dev_, &input_sample_info) ==
            ESP_CODEC_DEV_OK;
    result = output_opened && input_opened &&
             ConfigureEs8389OutputPolarity() && SetNs4150Enabled(true);
    if (!result) {
      if (input_opened) {
        esp_codec_dev_close(es8389_input_codec_dev_);
      }
      if (output_opened) {
        esp_codec_dev_close(es8389_output_codec_dev_);
      }
      SetNs4150Enabled(false);
    }
  }
  if (result && mode == Es8389OperatingMode::kActive) {
    es8389_operating_mode_ = mode;
  }
  return result;
}

bool TDisplayP4AirDriver::ConfigureEs8389OutputPolarity() {
  if (es8389_ctrl_if_ == nullptr || es8389_ctrl_if_->read_reg == nullptr ||
      es8389_ctrl_if_->write_reg == nullptr) {
    return false;
  }

  int register_value = 0;
  if (es8389_ctrl_if_->read_reg(es8389_ctrl_if_,
          kEs8389DacMiscControl2Register, 1, &register_value, 1) !=
      ESP_CODEC_DEV_OK) {
    return false;
  }

  // T-Display-P4-Air V1.0 的 DAC 输出声道采用不同极性：左声道为
  // 负极性，右声道为正极性。因此必须反转左声道的输出极性，
  // 否则两个扬声器的声学相位不一致，输出声音会失真。
  register_value |= kEs8389Dac1OutputInvertMask;
  return es8389_ctrl_if_->write_reg(es8389_ctrl_if_,
             kEs8389DacMiscControl2Register, 1, &register_value, 1) ==
         ESP_CODEC_DEV_OK;
}

bool TDisplayP4AirDriver::SetLr1121OperatingMode(Lr1121OperatingMode mode) {
  if (!status_.lr1121.init_flag || chip_.lr1121 == nullptr) {
    return mode == Lr1121OperatingMode::kSleep;
  }
  lr11xx_status_t result = LR11XX_STATUS_ERROR;
  if (mode == Lr1121OperatingMode::kSleep) {
    const lr11xx_system_sleep_cfg_t sleep_config = {
        .is_warm_start = true,
        .is_rtc_timeout = false,
    };
    result = chip_.lr1121->Invoke(lr11xx_system_set_sleep, sleep_config, 0U);
  } else if (chip_.lr1121->Wakeup()) {
    result = chip_.lr1121->Invoke(
        lr11xx_system_set_standby, LR11XX_SYSTEM_STANDBY_CFG_RC);
  }
  if (result != LR11XX_STATUS_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "LR1121 operating mode change failed (error code: %d)\n",
        static_cast<int>(result));
    return false;
  }
  return true;
}

bool TDisplayP4AirDriver::SetEsp32c5PowerEnabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  return chip_.xl9535->GpioWrite(
      gpio::xl9535::kEsp32c5En, enabled ? 1 : 0);
}

bool TDisplayP4AirDriver::SetCameraPowerEnabled(bool enabled) {
  if (!status_.sgm38121.init_flag) {
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

bool TDisplayP4AirDriver::SetUsbHostPowerEnabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  return chip_.xl9535->GpioWrite(gpio::xl9535::kUsbPhyPowerEn, enabled ? 1 : 0);
}

bool TDisplayP4AirDriver::PrepareMinimalDriversForPowerOff() {
  bool result = true;
  if (status_.axp517.init_flag && chip_.axp517 != nullptr) {
    result &= chip_.axp517->Deinit(false);
    status_.axp517.init_flag = false;
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
  result &= DeinitQmc6310n();
  result &= DeinitAw86224();
  result &= DeinitEs8389();
  result &= DeinitLr1121();
  result &= DeinitSt25r3916();
  result &= DeinitNrf9151();
  result &= SetCameraPowerEnabled(false);
  result &= SetEsp32c5PowerEnabled(false);
  result &= DeinitSdmmc(false);

  // 将外设复位、电源使能及控制引脚设置为关机安全电平。
  if (status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kNrf9151En, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kBhi260apRst, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAdl161Trig, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kAdl161Rst, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLr1121PowerEn, 0);
    result &=
        chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32p4Esp32c5UartSwitch, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kTouchRst, 1);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kScreenRst, 1);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 1);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLed1, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kNs4150En, 0);
  }

  result &= DeinitPower();
  return result;
}

bool TDisplayP4AirDriver::EnterEsp32c5DownloadMode() {
  if (!status_.xl9535.init_flag) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "EnterEsp32c5DownloadMode failed\n");
    return false;
  }

  bool result = true;

  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 0);
  tool_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 1);
  tool_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 1);

  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "EnterEsp32c5DownloadMode failed\n");
  }
  return result;
}

bool TDisplayP4AirDriver::SetUartTarget(UartTarget target) {
  if (!status_.xl9535.init_flag) {
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
