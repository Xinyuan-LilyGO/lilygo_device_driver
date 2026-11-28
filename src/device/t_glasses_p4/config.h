/*
 * @Description: T-Glasses-P4 板级硬件配置
 */
#pragma once

#include <array>
#include <cstdint>

#include "chip/i2c/bq2589x/bq2589x.h"
#include "device/common/camera_type.h"

namespace lilygo_device_driver::t_glasses_p4 {
namespace gpio {
namespace button {
inline constexpr int kEsp32p4Boot = 35;
inline constexpr int kKey = 36;
inline constexpr int kPower = 9;
}  // namespace button

namespace power {
inline constexpr int kEnable3v3 = 47;
inline constexpr int kAudioPowerEn = 20;
}  // namespace power

namespace i2c {
inline constexpr int kPort1Sda = 46;
inline constexpr int kPort1Scl = 45;
inline constexpr int kPort2Sda = 6;
inline constexpr int kPort2Scl = 7;
inline constexpr int kPort3Sda = 51;
inline constexpr int kPort3Scl = 50;
}  // namespace i2c

namespace bq25896 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
}  // namespace bq25896

namespace bq27220 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
}  // namespace bq27220

namespace sgm38121 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
}  // namespace sgm38121

namespace aw86224 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
}  // namespace aw86224

namespace aw93307 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
inline constexpr int kInt = 26;
}  // namespace aw93307

namespace rx8130ce {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
inline constexpr int kInt = 22;
}  // namespace rx8130ce

namespace s023msafjf10111e1 {
inline constexpr int kSda = i2c::kPort3Sda;
inline constexpr int kScl = i2c::kPort3Scl;
inline constexpr int kRst = 52;
}  // namespace s023msafjf10111e1

namespace camera {
inline constexpr int kSda = i2c::kPort3Sda;
inline constexpr int kScl = i2c::kPort3Scl;
}  // namespace camera

namespace sdio1 {
inline constexpr int kClk = 43;
inline constexpr int kCmd = 44;
inline constexpr int kD0 = 39;
inline constexpr int kD1 = 40;
inline constexpr int kD2 = 41;
inline constexpr int kD3 = 42;
}  // namespace sdio1

namespace sdio2 {
inline constexpr int kClk = 18;
inline constexpr int kCmd = 19;
inline constexpr int kD0 = 14;
inline constexpr int kD1 = 15;
inline constexpr int kD2 = 16;
inline constexpr int kD3 = 17;
}  // namespace sdio2

namespace spi {
inline constexpr int kPort1Sclk = 33;
inline constexpr int kPort1Mosi = 32;
inline constexpr int kPort1Miso = 31;
}  // namespace spi

namespace esp32c5 {
inline constexpr int kEn = 12;
inline constexpr int kPowerEn = 48;
}  // namespace esp32c5

namespace usb {
inline constexpr int kSelect = 21;
}  // namespace usb

namespace es8389 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
inline constexpr int kAdcData = 4;
inline constexpr int kDacData = 8;
inline constexpr int kBclk = 2;
inline constexpr int kMclk = 3;
inline constexpr int kWsLrck = 5;
}  // namespace es8389

namespace sd {
inline constexpr int kSdioClk = sdio1::kClk;
inline constexpr int kSdioCmd = sdio1::kCmd;
inline constexpr int kSdioD0 = sdio1::kD0;
inline constexpr int kSdioD1 = sdio1::kD1;
inline constexpr int kSdioD2 = sdio1::kD2;
inline constexpr int kSdioD3 = sdio1::kD3;
inline constexpr int kSclk = sdio1::kClk;
inline constexpr int kMosi = sdio1::kCmd;
inline constexpr int kMiso = sdio1::kD0;
inline constexpr int kCs = sdio1::kD3;
}  // namespace sd

namespace bhi260ap {
inline constexpr int kSda = i2c::kPort3Sda;
inline constexpr int kScl = i2c::kPort3Scl;
inline constexpr int kRst = 53;
inline constexpr int kInt = 54;
}  // namespace bhi260ap

namespace lr2021 {
inline constexpr int kCs = 34;
inline constexpr int kBusy = 28;
inline constexpr int kInt = 29;
inline constexpr int kRst = 27;
inline constexpr int kDio3 = 30;
inline constexpr int kSclk = spi::kPort1Sclk;
inline constexpr int kMosi = spi::kPort1Mosi;
inline constexpr int kMiso = spi::kPort1Miso;
}  // namespace lr2021
}  // namespace gpio

namespace device {
namespace bq25896 {
inline constexpr uint8_t kI2cAddress = 0x6B;
inline constexpr auto kPort = I2C_NUM_1;
inline constexpr cpp_bus_driver::Bq2589x::NtcConfig kNtcConfig = {
    .pull_up_ohm = 5230.0f,
    .pull_down_ohm = 30100.0f,
    .resistance_at_25c_ohm = 10000.0f,
    .beta_kelvin = 3950.0f,
};
}  // namespace bq25896

namespace bq27220 {
inline constexpr uint8_t kI2cAddress = 0x55;
inline constexpr uint16_t kBatteryCapacityMah = 800;
}  // namespace bq27220

namespace es8389 {
inline constexpr uint8_t kI2cAddress = 0x10;
inline constexpr auto kPort = LP_I2C_NUM_0;
inline constexpr int kMclkMultiple = 256;
inline constexpr int kSampleRate = 44100;
inline constexpr int kBitsPerSample = 16;
inline constexpr int kChannel = 2;
}  // namespace es8389
namespace camera {
#if defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_SC2336)
inline constexpr CameraType kType = CameraType::kSc2336;
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV2710)
inline constexpr CameraType kType = CameraType::kOv2710;
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV5645)
inline constexpr CameraType kType = CameraType::kOv5645;
#else
inline constexpr CameraType kType = CameraType::kUnknown;
#endif

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_PIXEL_FORMAT_RGB565)
inline constexpr int kBitsPerPixel = 16;
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_PIXEL_FORMAT_RGB888)
inline constexpr int kBitsPerPixel = 24;
#else
inline constexpr int kBitsPerPixel = 0;
#endif

inline constexpr int kBufferCount = 2;
}  // namespace camera

namespace bhi260ap {
inline constexpr uint8_t kI2cAddress = 0x28;
inline constexpr uint32_t kI2cFrequencyHz = 400000;
}  // namespace bhi260ap

namespace aw86224 {
inline constexpr uint8_t kI2cAddress = 0x58;
inline constexpr int32_t kI2cFrequencyHz = 100000;
}  // namespace aw86224

namespace aw93307 {
inline constexpr uint8_t kI2cAddress = 0x12;
inline constexpr int32_t kI2cFrequencyHz = 400000;
inline constexpr uint16_t kEnabledChannels = 0x001F;
inline constexpr uint8_t kButtonChannel = 0;
inline constexpr uint8_t kSliderFirstChannel = 1;
inline constexpr uint8_t kSliderChannelCount = 4;
inline constexpr uint8_t kSliderWidgetIndex = 0;
inline constexpr uint8_t kButtonWidgetIndex = 1;
inline constexpr std::array<uint8_t, 5> kElectrodes = {4, 0, 1, 3, 5};
inline constexpr uint32_t kSliderTouchThreshold = 900000;
inline constexpr uint32_t kSliderPositionThreshold = 50000;
inline constexpr uint32_t kButtonTouchThreshold = 360000;
inline constexpr int32_t kSliderSignalTouchThreshold = 360000000;
inline constexpr int32_t kSliderSignalReleaseThreshold =
    kSliderSignalTouchThreshold / 2;
inline constexpr uint8_t kSliderSignalDebounceSamples = 3;
inline constexpr uint32_t kCalibrationDelayMs = 500;
}  // namespace aw93307

namespace lr2021 {
inline constexpr int32_t kSpiFrequencyHz = 10000000;
}  // namespace lr2021

namespace sd {
inline constexpr const char* kBasePath = "/sdcard";
}  // namespace sd

namespace sgm38121 {
inline constexpr uint8_t kI2cAddress = 0x28;
inline constexpr auto kPort = LP_I2C_NUM_0;
}  // namespace sgm38121

namespace rx8130ce {
inline constexpr uint8_t kI2cAddress = 0x32;
inline constexpr int32_t kI2cFrequencyHz = 100000;
}  // namespace rx8130ce

namespace s023msafjf10111e1 {
inline constexpr uint8_t kI2cAddress = 0x54;
inline constexpr auto kPort = I2C_NUM_0;
inline constexpr int kScreenWidth = 640;
inline constexpr int kScreenHeight = 400;
inline constexpr int kScreenMipiDsiHsync = 64;
inline constexpr int kScreenMipiDsiHbp = 58;
inline constexpr int kScreenMipiDsiHfp = 96;
inline constexpr int kScreenMipiDsiVsync = 6;
inline constexpr int kScreenMipiDsiVbp = 56;
inline constexpr int kScreenMipiDsiVfp = 63;
inline constexpr int kScreenRefreshRateHz = 60;
inline constexpr double kScreenMipiDsiDpiClkMhz =
    (kScreenWidth + kScreenMipiDsiHsync + kScreenMipiDsiHbp +
        kScreenMipiDsiHfp) *
    (kScreenHeight + kScreenMipiDsiVsync + kScreenMipiDsiVbp +
        kScreenMipiDsiVfp) * kScreenRefreshRateHz / 1000000.0;
inline constexpr int kScreenDataLaneNum = 1;
inline constexpr int kScreenLaneBitRateMbps = 1000;
}  // namespace s023msafjf10111e1

namespace screen {
inline constexpr int kRotationDirection = 0;
#if defined(CONFIG_LILYGO_DEVICE_DRIVER_SCREEN_PIXEL_FORMAT_RGB888)
inline constexpr int kBitsPerPixel = 24;
#else
#error "T-Glasses-P4 screen requires RGB888."
#endif

inline constexpr int kWidth = s023msafjf10111e1::kScreenWidth;
inline constexpr int kHeight = s023msafjf10111e1::kScreenHeight;
inline constexpr auto kMipiDsiDpiClkMhz =
    s023msafjf10111e1::kScreenMipiDsiDpiClkMhz;
inline constexpr int kMipiDsiHsync = s023msafjf10111e1::kScreenMipiDsiHsync;
inline constexpr int kMipiDsiHbp = s023msafjf10111e1::kScreenMipiDsiHbp;
inline constexpr int kMipiDsiHfp = s023msafjf10111e1::kScreenMipiDsiHfp;
inline constexpr int kMipiDsiVsync = s023msafjf10111e1::kScreenMipiDsiVsync;
inline constexpr int kMipiDsiVbp = s023msafjf10111e1::kScreenMipiDsiVbp;
inline constexpr int kMipiDsiVfp = s023msafjf10111e1::kScreenMipiDsiVfp;
inline constexpr int kDataLaneNum = s023msafjf10111e1::kScreenDataLaneNum;
inline constexpr int kLaneBitRateMbps =
    s023msafjf10111e1::kScreenLaneBitRateMbps;
}  // namespace screen
}  // namespace device
}  // namespace lilygo_device_driver::t_glasses_p4
