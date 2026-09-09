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

bool TDisplayP4Driver::InitSgm38121() {
  if (chip_.sgm38121 == nullptr || !chip_.sgm38121->Init()) {
    status_.sgm38121.init_flag = false;
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
  status_.hi8561_touch.init_flag = false;
  if (!status_.xl9535.init_flag) {
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

  status_.hi8561_touch.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561Touch success\n");
  return true;
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
  status_.gt9895.init_flag = false;
  if (!status_.xl9535.init_flag) {
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

  status_.gt9895.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitGt9895 success\n");
  return true;
}

bool TDisplayP4Driver::InitAw86224() {
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

bool TDisplayP4Driver::InitScreen() {
  if (!status_.xl9535.init_flag) {
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
  status_.hi8561.init_flag = false;
  status_.hi8561_touch.init_flag = false;
  ResetScreenBacklightStatus();
  status_.rm69a10.init_flag = false;

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
      return status_.gt9895.init_flag || InitGt9895();
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
  if (!status_.xl9535.init_flag || chip_.xl9535 == nullptr) {
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
  status_.sd_card.init_flag = sd_card_.IsMounted();
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
  if (!status_.xl9535.init_flag || chip_.xl9535 == nullptr) {
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
  status_.sd_card.init_flag = sd_card_.IsMounted();
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

  if (bus_.screen_mipi_bus != nullptr) {
    result &= bus_.screen_mipi_bus->Deinit();
    bus_.screen_mipi_bus.reset();
  }
  if (status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kScreenRst, device::xl9535::kResetAsserted);
  }

  return result;
}

bool TDisplayP4Driver::DeinitTouch() {
  bool result = true;

  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      if (status_.hi8561_touch.init_flag) {
        result &= chip_.hi8561_touch->Deinit(false);
        status_.hi8561_touch.init_flag = false;
      }
      break;
    case device::ScreenType::kRm69a10:
      if (status_.gt9895.init_flag) {
        result &= chip_.gt9895->Deinit(false);
        status_.gt9895.init_flag = false;
      }
      break;
    default:
      break;
  }

  if (status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kTouchRst, device::xl9535::kResetAsserted);
  }

  return result;
}

bool TDisplayP4Driver::DeinitAw86224() {
  bool result = true;
  if (status_.aw86224.init_flag && chip_.aw86224 != nullptr) {
    result &= chip_.aw86224->StopRamPlaybackWaveform();
    result &= chip_.aw86224->Deinit(false);
  }
  status_.aw86224.init_flag = false;
  status_.aw86224.ram_waveform_info = {};
  return result;
}

bool TDisplayP4Driver::DeinitL76k() {
  bool result = true;
  if (status_.l76k.init_flag && chip_.l76k != nullptr) {
    result &= chip_.l76k->Sleep(true);
    result &= chip_.l76k->Deinit();
  }
  status_.l76k.init_flag = false;
  return result;
}

bool TDisplayP4Driver::DeinitSdmmc(bool release_bus) {
  bool result = sd_card_.Deinit(release_bus);
  status_.sd_card.init_flag = sd_card_.IsMounted();
  // 卸载失败时保留供电，允许后续重试。
  if (sd_card_.IsMounted()) {
    return false;
  }
  if (status_.xl9535.init_flag && chip_.xl9535 != nullptr) {
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kSdPowerEn, device::xl9535::kSdPowerDisabled);
  }
  return result;
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

bool TDisplayP4Driver::IsRm69a10Ready() const {
  return status_.rm69a10.init_flag && chip_.rm69a10 != nullptr;
}

bool TDisplayP4Driver::IsGt9895Ready() const {
  return status_.gt9895.init_flag && chip_.gt9895 != nullptr;
}

bool TDisplayP4Driver::IsAw86224Ready() const {
  return status_.aw86224.init_flag && chip_.aw86224 != nullptr;
}

bool TDisplayP4Driver::IsL76kReady() const {
  return status_.l76k.init_flag && chip_.l76k != nullptr;
}

bool TDisplayP4Driver::IsSy7200aReady() const {
  return status_.sy7200a.init_flag && chip_.sy7200a != nullptr &&
         chip_.sy7200a->IsInitialized();
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
  return status_.sd_card.init_flag && sd_card_.IsReady();
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

bool TDisplayP4Driver::SetUsbHostPowerEnabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  return chip_.xl9535->GpioWrite(gpio::xl9535::kUsbPhyPowerEn, enabled ? 1 : 0);
}

bool TDisplayP4Driver::DetectScreenType() {
  status_.gt9895.init_flag = false;

  if (!status_.xl9535.init_flag) {
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
