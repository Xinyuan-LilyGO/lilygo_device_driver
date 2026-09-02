/*
 * @Description: T-Glasses-P4 板级设备驱动实现
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:58:49
 * @LastEditTime: 2026-09-02 17:16:16
 * @License: GPL 3.0
 */
#include "t_glasses_p4_driver.h"

#include <cstdio>

#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "driver/spi_master.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdmmc_cmd.h"

namespace lilygo_device_driver {
namespace gpio = t_glasses_p4::gpio;
namespace device = t_glasses_p4::device;
namespace {

using ScreenInfo = device::ScreenInfo;
using ScreenType = device::ScreenType;

constexpr ScreenInfo kS023msafjf10111e1ScreenInfo = {
    .type = ScreenType::kS023msafjf10111e1,
    .name = "s023msafjf10111e1",
    .width = device::s023msafjf10111e1::kScreenWidth,
    .height = device::s023msafjf10111e1::kScreenHeight,
    .bits_per_pixel = device::screen::kBitsPerPixel,
    .pixel_format = device::screen::kPixelFormat,
    .mipi_dsi_dpi_clk_mhz = device::s023msafjf10111e1::kScreenMipiDsiDpiClkMhz,
    .mipi_dsi_hsync = device::s023msafjf10111e1::kScreenMipiDsiHsync,
    .mipi_dsi_hbp = device::s023msafjf10111e1::kScreenMipiDsiHbp,
    .mipi_dsi_hfp = device::s023msafjf10111e1::kScreenMipiDsiHfp,
    .mipi_dsi_vsync = device::s023msafjf10111e1::kScreenMipiDsiVsync,
    .mipi_dsi_vbp = device::s023msafjf10111e1::kScreenMipiDsiVbp,
    .mipi_dsi_vfp = device::s023msafjf10111e1::kScreenMipiDsiVfp,
    .data_lane_num = device::s023msafjf10111e1::kScreenDataLaneNum,
    .lane_bit_rate_mbps = device::s023msafjf10111e1::kScreenLaneBitRateMbps,
};

constexpr const ScreenInfo* kDefaultScreenInfo = &kS023msafjf10111e1ScreenInfo;
constexpr uint32_t kInitializationShutdownTimeoutMs = 5 * 1000;

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

}  // namespace

TGlassesP4Driver& TGlassesP4Driver::GetInstance() {
  static TGlassesP4Driver* instance = new TGlassesP4Driver();
  return *instance;
}

const device::ScreenInfo& TGlassesP4Driver::screen_info() const {
  return *(screen_info_ == nullptr ? kDefaultScreenInfo : screen_info_);
}

device::ScreenType TGlassesP4Driver::screen_type() const {
  return screen_info().type;
}

void TGlassesP4Driver::CreateDrivers() {
  if (tool_ != nullptr) {
    return;
  }
  tool_ = std::make_unique<cpp_bus_driver::Tool>();
  screen_info_ = kDefaultScreenInfo;

  bus_.sy6970_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      gpio::sy6970::kSda, gpio::sy6970::kScl, I2C_NUM_0);
  bus_.sgm38121_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      gpio::sgm38121::kSda, gpio::sgm38121::kScl, I2C_NUM_1);
  bus_.sx1262_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(gpio::sx1262::kMosi,
          gpio::sx1262::kSclk, gpio::sx1262::kMiso, SPI2_HOST, 0);

  bus_.bq27220_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.sy6970_i2c_bus);
  bus_.aw86224_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.sgm38121_i2c_bus);
  bus_.es8311_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.sgm38121_i2c_bus);
  bus_.screen_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c1>(bus_.sgm38121_i2c_bus);

  bus_.es8311_i2s_bus = std::make_shared<cpp_bus_driver::HardwareI2s>(
      gpio::es8311::kAdcData, gpio::es8311::kDacData, gpio::es8311::kWsLrck,
      gpio::es8311::kBclk, gpio::es8311::kMclk, i2s_port_t::I2S_NUM_0,
      cpp_bus_driver::HardwareI2s::DataMode::kInputOutput,
      cpp_bus_driver::HardwareI2s::I2sMode::kStd,
      i2s_clock_src_t::I2S_CLK_SRC_DEFAULT);

  chip_.sy6970 = std::make_unique<cpp_bus_driver::Sy6970>(
      bus_.sy6970_i2c_bus, device::sy6970::kI2cAddress);
  chip_.bq27220 = std::make_unique<cpp_bus_driver::Bq27220>(
      bus_.bq27220_i2c_bus, device::bq27220::kI2cAddress);
  chip_.sgm38121 = std::make_unique<cpp_bus_driver::Sgm38121>(
      bus_.sgm38121_i2c_bus, device::sgm38121::kI2cAddress);
  chip_.aw86224 = std::make_unique<cpp_bus_driver::Aw862xx>(
      bus_.aw86224_i2c_bus, device::aw86224::kI2cAddress);
  chip_.es8311 = std::make_unique<cpp_bus_driver::Es8311>(
      bus_.es8311_i2c_bus, bus_.es8311_i2s_bus, device::es8311::kI2cAddress);
  chip_.sx1262 =
      std::make_unique<usp_cpp_bus_driver::Sx126x>(bus_.sx1262_spi_bus,
          gpio::sx1262::kBusy, gpio::sx1262::kCs, [this](bool level) {
            return tool_->GpioWrite(gpio::sx1262::kRst, level);
          });
  chip_.s023msafjf10111e1 = std::make_unique<cpp_bus_driver::S023msafjf10111e1>(
      bus_.screen_i2c_bus, device::s023msafjf10111e1::kI2cAddress,
      gpio::s023msafjf10111e1::kRst);
}

bool TGlassesP4Driver::Init(InitMode mode) {
  CreateDrivers();
  const int64_t start_time_us = tool_->GetSystemTimeUs();
  const bool result = InitDrivers(mode);
  const int64_t elapsed_time_us = tool_->GetSystemTimeUs() - start_time_us;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "TGlassesP4Driver init finished (mode: %s, result: %s, elapsed: "
      "%lld ms)\n",
      mode == InitMode::kAsync ? "async" : "sync",
      result ? "success" : "failed",
      static_cast<long long>(elapsed_time_us / 1000));
  return result;
}

bool TGlassesP4Driver::InitMinimal() {
  CreateDrivers();
  return InitMinimalDrivers();
}

bool TGlassesP4Driver::InitMinimalDrivers() {
  if (minimal_drivers_initialized_) {
    return true;
  }

  bool result = true;
  result &= InitSy6970();
  result &= InitPower();
  result &= InitSgm38121();
  minimal_drivers_initialized_ = result;
  return result;
}

bool TGlassesP4Driver::InitDrivers(InitMode mode) {
  bool result = InitMinimalDrivers();
  async_init_manager_.Reset();

  if (mode == InitMode::kAsync) {
    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TGlassesP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitScreen();
          }
          self->async_init_manager_.FinishTask();
        },
        "ScreenTask", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TGlassesP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitBq27220();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitBq27220Task", 2048, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TGlassesP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitAw86224();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitAw86224Task", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TGlassesP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested() &&
              self->InitEs8311()) {
            self->SetEs8311OperatingMode(Es8311OperatingMode::kSleep);
          }
          self->async_init_manager_.FinishTask();
        },
        "InitEs8311Task", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TGlassesP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitSx1262();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitSx1262Task", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TGlassesP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitSdmmc(device::sd::kBasePath, SDMMC_FREQ_52M);
          }
          self->async_init_manager_.FinishTask();
        },
        "InitSdmmcTask", 4096, this, 3);
  } else {
    result &= InitScreen();
    InitBq27220();
    result &= InitAw86224();
    bool es8311_initialized = InitEs8311();
    if (es8311_initialized) {
      es8311_initialized = SetEs8311OperatingMode(Es8311OperatingMode::kSleep);
    }
    result &= es8311_initialized;
    result &= InitSx1262();

    InitSdmmc(device::sd::kBasePath, SDMMC_FREQ_52M);

    result &= status_.sy6970.init_flag;
    result &= status_.sgm38121.init_flag;
    result &= status_.bq27220.init_flag;
    result &= status_.aw86224.init_flag;
    result &= status_.es8311.init_flag;
    result &= status_.sx1262.init_flag;
  }

  return result;
}

bool TGlassesP4Driver::InitSy6970() {
  if (!chip_.sy6970->Init()) {
    status_.sy6970.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSy6970 failed\n");
    return false;
  }

  status_.sy6970.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSy6970 success\n");
  return true;
}

bool TGlassesP4Driver::InitBq27220() {
  if (!chip_.bq27220->Init()) {
    status_.bq27220.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBq27220 failed\n");
    return false;
  }

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
    chip_.bq27220->Deinit(false);
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBq27220 failed\n");
  }
  return result;
}

bool TGlassesP4Driver::InitSgm38121() {
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

bool TGlassesP4Driver::InitS023msafjf10111e1() {
  if (!chip_.s023msafjf10111e1->Init()) {
    status_.s023msafjf10111e1.init_flag = false;
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitS023msafjf10111e1 failed\n");
    return false;
  }

  bool result = true;
  result &= chip_.s023msafjf10111e1->SetDataFormat(
      cpp_bus_driver::S023msafjf10111e1::DataFormat::kRgb888);
  result &= chip_.s023msafjf10111e1->SetBrightness(0);

  status_.s023msafjf10111e1.init_flag = result;
  if (result) {
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "InitS023msafjf10111e1 success\n");
  } else {
    chip_.s023msafjf10111e1->Deinit(false);
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitS023msafjf10111e1 failed\n");
  }
  return result;
}

bool TGlassesP4Driver::InitAw86224() {
  if (IsAw86224Ready()) {
    return true;
  }
  if (!chip_.aw86224->Init(device::aw86224::kI2cFrequencyHz)) {
    status_.aw86224.init_flag = false;
    status_.aw86224.ram_waveform_selection =
        cpp_bus_driver::Aw862xx::RamWaveformSelection();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitAw86224 failed\n");
    return false;
  }

  cpp_bus_driver::Aw862xx::RamWaveformSelection selection;
  bool result = chip_.aw86224->InitRamModeByF0(selection);
  if (result) {
    result = chip_.aw86224->StopRamPlaybackWaveform();
  }
  if (!result) {
    chip_.aw86224->Deinit(false);
  }

  status_.aw86224.init_flag = result;
  status_.aw86224.ram_waveform_selection =
      result ? selection : cpp_bus_driver::Aw862xx::RamWaveformSelection();
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitAw86224 success\n");
  } else {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitAw86224 failed\n");
  }
  return result;
}

bool TGlassesP4Driver::InitEs8311() {
  if (IsEs8311Ready()) {
    return true;
  }

  status_.es8311.init_flag = false;
  if (chip_.es8311 == nullptr || !chip_.es8311->Init() ||
      !chip_.es8311->Init(device::es8311::kMclkMultiple,
          device::es8311::kSampleRate, device::es8311::kBitsPerSample)) {
    if (chip_.es8311 != nullptr) {
      chip_.es8311->Deinit(false);
    }
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8311 failed\n");
    return false;
  }

  const cpp_bus_driver::Es8311::PowerStatus power_status = {
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
  result &= chip_.es8311->SetPowerStatus(power_status);
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
    chip_.es8311->Deinit(false);
  }
  status_.es8311.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitEs8311 success\n" : "InitEs8311 failed\n");
  return result;
}

bool TGlassesP4Driver::InitSx1262() {
  if (IsSx1262Ready()) {
    return true;
  }
  if (!tool_->SetGpioMode(gpio::sx1262::kRst,
          cpp_bus_driver::Tool::GpioMode::kOutput,
          cpp_bus_driver::Tool::GpioStatus::kPullup) ||
      !chip_.sx1262->Init(device::sx1262::kSpiFrequencyHz)) {
    status_.sx1262.init_flag = false;
    tool_->GpioWrite(gpio::sx1262::kRst, 0);
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSx1262 failed\n");
    return false;
  }

  const bool result = chip_.sx1262->SetSleep();
  if (!result) {
    chip_.sx1262->Deinit(false);
    tool_->GpioWrite(gpio::sx1262::kRst, 0);
  }
  status_.sx1262.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitSx1262 success\n" : "InitSx1262 sleep failed\n");
  return result;
}

bool TGlassesP4Driver::InitPower() {
  if (!InitLdoPower(4, 3300)) {
    return false;
  }
  if (!InitLdoPower(3, 2500)) {
    DeinitLdoPower(4);
    return false;
  }

  bool gpio_initialized = true;
  gpio_initialized &= tool_->SetGpioMode(
      gpio::power::kEn5v0, cpp_bus_driver::Tool::GpioMode::kOutput);
  gpio_initialized &= tool_->SetGpioMode(
      gpio::power::kEn3v3, cpp_bus_driver::Tool::GpioMode::kOutput);
  gpio_initialized &= tool_->GpioWrite(gpio::power::kEn5v0, true);
  gpio_initialized &= tool_->GpioWrite(gpio::power::kEn3v3, true);
  if (!gpio_initialized) {
    tool_->GpioWrite(gpio::power::kEn5v0, false);
    tool_->GpioWrite(gpio::power::kEn3v3, false);
    DeinitLdoPower(3);
    DeinitLdoPower(4);
    return false;
  }
  tool_->DelayMs(200);
  return true;
}

bool TGlassesP4Driver::InitScreen() {
  const auto& screen = screen_info();
  bus_.screen_mipi_bus = std::make_shared<cpp_bus_driver::HardwareMipi>(
      screen.width, screen.height, screen.mipi_dsi_hsync, screen.mipi_dsi_hbp,
      screen.mipi_dsi_hfp, screen.mipi_dsi_vsync, screen.mipi_dsi_vbp,
      screen.mipi_dsi_vfp, screen.data_lane_num,
      ColorFormatFromBitsPerPixel(screen.bits_per_pixel));

  status_.s023msafjf10111e1.init_flag = false;

  bool result = InitS023msafjf10111e1();
  if (result) {
    result = bus_.screen_mipi_bus->Init(
        screen.mipi_dsi_dpi_clk_mhz, screen.lane_bit_rate_mbps);
  }
  if (result) {
    result = bus_.screen_mipi_bus->StartTransmit();
  }

  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitScreen success\n");
  } else {
    DeinitScreen();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitScreen failed\n");
  }
  return result;
}

bool TGlassesP4Driver::InitSdmmc(const char* base_path, int max_freq_khz) {
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
    return false;
  }

  sdmmc_card_print_info(stdout, card);
  sd_card_ = card;
  status_.sd_card.init_flag = true;
  return true;
}

bool TGlassesP4Driver::InitSdspi(
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
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "spi_bus_initialize failed (error code: %#X)\n", result);
    status_.sd_card.init_flag = false;
    sd_card_ = nullptr;
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
    return false;
  }

  sdmmc_card_print_info(stdout, card);
  sd_card_ = card;
  status_.sd_card.init_flag = true;
  return true;
}

bool TGlassesP4Driver::DeinitAw86224() {
  bool result = true;
  if (status_.aw86224.init_flag && chip_.aw86224 != nullptr) {
    result &= chip_.aw86224->StopRamPlaybackWaveform();
    result &= chip_.aw86224->Deinit(false);
  }
  status_.aw86224.init_flag = false;
  status_.aw86224.ram_waveform_selection = {};
  return result;
}

bool TGlassesP4Driver::DeinitEs8311() {
  bool result = true;
  if (status_.es8311.init_flag && chip_.es8311 != nullptr) {
    result &= SetEs8311OperatingMode(Es8311OperatingMode::kSleep);
    result &= chip_.es8311->Deinit(false);
  }
  status_.es8311.init_flag = false;
  return result;
}

bool TGlassesP4Driver::DeinitSx1262() {
  bool result = true;
  if (status_.sx1262.init_flag && chip_.sx1262 != nullptr) {
    result &= chip_.sx1262->SetSleep();
    result &= chip_.sx1262->Deinit(false);
    result &= tool_->GpioWrite(gpio::sx1262::kRst, 0);
  }
  status_.sx1262.init_flag = false;
  return result;
}

bool TGlassesP4Driver::DeinitScreen() {
  bool result = true;

  if (bus_.screen_mipi_bus != nullptr) {
    result &= bus_.screen_mipi_bus->Deinit();
    bus_.screen_mipi_bus.reset();
  }

  if (status_.s023msafjf10111e1.init_flag) {
    result &= chip_.s023msafjf10111e1->Deinit(false);
    status_.s023msafjf10111e1.init_flag = false;
  }

  return result;
}

bool TGlassesP4Driver::IsSy6970Ready() const {
  return status_.sy6970.init_flag && chip_.sy6970 != nullptr;
}

bool TGlassesP4Driver::IsBq27220Ready() const {
  return status_.bq27220.init_flag && chip_.bq27220 != nullptr;
}

bool TGlassesP4Driver::IsSgm38121Ready() const {
  return status_.sgm38121.init_flag && chip_.sgm38121 != nullptr;
}

bool TGlassesP4Driver::IsS023msafjf10111e1Ready() const {
  return status_.s023msafjf10111e1.init_flag &&
         chip_.s023msafjf10111e1 != nullptr;
}

bool TGlassesP4Driver::IsAw86224Ready() const {
  return status_.aw86224.init_flag && chip_.aw86224 != nullptr;
}

bool TGlassesP4Driver::IsEs8311Ready() const {
  return status_.es8311.init_flag && chip_.es8311 != nullptr;
}

bool TGlassesP4Driver::IsSx1262Ready() const {
  return status_.sx1262.init_flag && chip_.sx1262 != nullptr;
}

bool TGlassesP4Driver::IsScreenReady() const {
  return bus_.screen_mipi_bus != nullptr &&
         bus_.screen_mipi_bus->device_handle() != nullptr &&
         IsS023msafjf10111e1Ready();
}

bool TGlassesP4Driver::IsSdmmcReady() const {
  return status_.sd_card.init_flag && sd_card_ != nullptr &&
         sdmmc_get_status(sd_card_) == ESP_OK;
}

bool TGlassesP4Driver::SetAw86224Standby() {
  return !IsAw86224Ready() || chip_.aw86224->StopRamPlaybackWaveform();
}

bool TGlassesP4Driver::SetEs8311OperatingMode(Es8311OperatingMode mode) {
  if (!status_.es8311.init_flag) {
    return mode == Es8311OperatingMode::kSleep;
  }
  const bool playback_enabled = mode == Es8311OperatingMode::kPlayback ||
                                mode == Es8311OperatingMode::kDuplex;
  const bool capture_enabled = mode == Es8311OperatingMode::kCapture ||
                               mode == Es8311OperatingMode::kDuplex;
  const bool sleep = mode == Es8311OperatingMode::kSleep;
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
  return result;
}

bool TGlassesP4Driver::SetSx1262OperatingMode(Sx1262OperatingMode mode) {
  if (!status_.sx1262.init_flag) {
    return mode == Sx1262OperatingMode::kSleep;
  }
  return mode == Sx1262OperatingMode::kSleep ? chip_.sx1262->SetSleep()
                                             : chip_.sx1262->Wakeup();
}

bool TGlassesP4Driver::SetCameraPowerEnabled(bool enabled) {
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

bool TGlassesP4Driver::PrepareDriversForPowerOff() {
  if (!async_init_manager_.StopAndWait(kInitializationShutdownTimeoutMs)) {
    LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
        "Wait for asynchronous initialization before power off timed out\n");
    return false;
  }

  bool result = true;
  result &= DeinitScreen();
  result &= DeinitAw86224();
  result &= DeinitEs8311();
  result &= DeinitSx1262();
  result &= SetCameraPowerEnabled(false);
  if (IsSdmmcReady()) {
    result &= DeinitSdmmc();
  }
  result &= tool_->GpioWrite(gpio::power::kEn5v0, 0);
  result &= tool_->GpioWrite(gpio::power::kEn3v3, 0);
  minimal_drivers_initialized_ = false;
  return result;
}

}  // namespace lilygo_device_driver
