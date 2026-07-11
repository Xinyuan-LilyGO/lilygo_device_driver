/*
 * @Description: t_display_p4_air_config
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-07-11 09:53:30
 * @License: GPL 3.0
 */

#pragma once

#include <cstdint>

#include "cpp_bus_driver_library.h"

namespace lilygo_device_driver::t_display_p4_air {
namespace gpio {
namespace button {
inline constexpr int kEsp32p4Boot = 35;
inline constexpr int kKey1 = 49;
inline constexpr int kPower = 11;
}  // namespace button

namespace power {
inline constexpr int kEnable3v3 = 12;
}  // namespace power

namespace i2c {
inline constexpr int kPort1Sda = 9;
inline constexpr int kPort1Scl = 10;
inline constexpr int kPort2Sda = 54;
inline constexpr int kPort2Scl = 53;
}  // namespace i2c

namespace spi {
inline constexpr int kPort1Sclk = 2;
inline constexpr int kPort1Mosi = 3;
inline constexpr int kPort1Miso = 4;
}  // namespace spi

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

namespace xl9535 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
inline constexpr auto kSdPowerEn = cpp_bus_driver::Xl95x5::Pin::kIo0;
inline constexpr auto kNrf9151En = cpp_bus_driver::Xl95x5::Pin::kIo1;
inline constexpr auto kBhi260apRst = cpp_bus_driver::Xl95x5::Pin::kIo2;
inline constexpr auto kAdl161Trig = cpp_bus_driver::Xl95x5::Pin::kIo3;
inline constexpr auto kAdl161Rst = cpp_bus_driver::Xl95x5::Pin::kIo4;
inline constexpr auto kLr1121PowerEn = cpp_bus_driver::Xl95x5::Pin::kIo5;
inline constexpr auto kUsbPhyPowerEn = cpp_bus_driver::Xl95x5::Pin::kIo10;
inline constexpr auto kEsp32p4Esp32c5UartSwitch =
    cpp_bus_driver::Xl95x5::Pin::kIo11;
inline constexpr auto kEsp32c5En = cpp_bus_driver::Xl95x5::Pin::kIo12;
inline constexpr auto kTouchRst = cpp_bus_driver::Xl95x5::Pin::kIo13;
inline constexpr auto kScreenRst = cpp_bus_driver::Xl95x5::Pin::kIo14;
inline constexpr auto kEsp32c5Boot = cpp_bus_driver::Xl95x5::Pin::kIo15;
inline constexpr auto kLed1 = cpp_bus_driver::Xl95x5::Pin::kIo16;
inline constexpr auto kNs4150En = cpp_bus_driver::Xl95x5::Pin::kIo17;
}  // namespace xl9535

namespace hi8561 {
inline constexpr int kScreenBacklight = 50;
inline constexpr int kTouchSda = i2c::kPort2Sda;
inline constexpr int kTouchScl = i2c::kPort2Scl;
inline constexpr int kTouchInt = 52;
}  // namespace hi8561

namespace axp517 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
}  // namespace axp517

namespace aw86224 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
}  // namespace aw86224

namespace es8389 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
inline constexpr int kAdcData = 33;
inline constexpr int kDacData = 32;
inline constexpr int kBclk = 31;
inline constexpr int kMclk = 30;
inline constexpr int kWsLrck = 34;
}  // namespace es8389

namespace bhi260ap {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
inline constexpr int kInt = 51;
}  // namespace bhi260ap

namespace qmc6310n {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
}  // namespace qmc6310n

namespace sgm38121 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
}  // namespace sgm38121

namespace lr1121 {
inline constexpr int kCs = 7;
inline constexpr int kBusy = 6;
inline constexpr int kInt = 5;
inline constexpr int kRst = 8;
inline constexpr int kSclk = spi::kPort1Sclk;
inline constexpr int kMosi = spi::kPort1Mosi;
inline constexpr int kMiso = spi::kPort1Miso;
}  // namespace lr1121

namespace st25r3916 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
inline constexpr int kInt = 13;
}  // namespace st25r3916

namespace infrared {
inline constexpr int kRx = 29;
inline constexpr int kTx = 28;
}  // namespace infrared

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

namespace esp32c5 {
inline constexpr int kSdioClk = sdio2::kClk;
inline constexpr int kSdioCmd = sdio2::kCmd;
inline constexpr int kSdioD0 = sdio2::kD0;
inline constexpr int kSdioD1 = sdio2::kD1;
inline constexpr int kSdioD2 = sdio2::kD2;
inline constexpr int kSdioD3 = sdio2::kD3;
}  // namespace esp32c5

namespace nrf9151 {
inline constexpr int kUartRx = 22;
inline constexpr int kUartTx = 23;
inline constexpr int kUartRts = 21;
inline constexpr int kUartCts = 20;
}  // namespace nrf9151
}  // namespace gpio

namespace device {
namespace xl9535 {
inline constexpr uint8_t kI2cAddress = 0x20;
}  // namespace xl9535

namespace hi8561 {
inline constexpr int kScreenWidth = 540;
inline constexpr int kScreenHeight = 1168;
inline constexpr int kScreenMipiDsiDpiClkMhz = 60;
inline constexpr int kScreenMipiDsiHsync = 28;
inline constexpr int kScreenMipiDsiHbp = 26;
inline constexpr int kScreenMipiDsiHfp = 20;
inline constexpr int kScreenMipiDsiVsync = 2;
inline constexpr int kScreenMipiDsiVbp = 22;
inline constexpr int kScreenMipiDsiVfp = 200;
inline constexpr int kScreenDataLaneNum = 2;
inline constexpr int kScreenLaneBitRateMbps = 1000;
inline constexpr uint8_t kTouchI2cAddress = 0x68;

inline constexpr int kWidth = kScreenWidth;
inline constexpr int kHeight = kScreenHeight;
inline constexpr int kMipiDsiDpiClkMhz = kScreenMipiDsiDpiClkMhz;
inline constexpr int kMipiDsiHsync = kScreenMipiDsiHsync;
inline constexpr int kMipiDsiHbp = kScreenMipiDsiHbp;
inline constexpr int kMipiDsiHfp = kScreenMipiDsiHfp;
inline constexpr int kMipiDsiVsync = kScreenMipiDsiVsync;
inline constexpr int kMipiDsiVbp = kScreenMipiDsiVbp;
inline constexpr int kMipiDsiVfp = kScreenMipiDsiVfp;
inline constexpr int kDataLaneNum = kScreenDataLaneNum;
inline constexpr int kLaneBitRateMbps = kScreenLaneBitRateMbps;
}  // namespace hi8561

namespace screen {
inline constexpr int kRotationDirection = 0;

#if defined(CONFIG_SCREEN_PIXEL_FORMAT_RGB565)
inline constexpr int kBitsPerPixel = 16;
inline constexpr const char* kPixelFormat = "rgb565";
#elif defined(CONFIG_SCREEN_PIXEL_FORMAT_RGB888)
inline constexpr int kBitsPerPixel = 24;
inline constexpr const char* kPixelFormat = "rgb888";
#else
#error "Missing required macro definition."
#endif
}  // namespace screen

namespace axp517 {
inline constexpr uint8_t kI2cAddress = 0x34;
}  // namespace axp517

namespace aw86224 {
inline constexpr uint8_t kI2cAddress = 0x58;
}  // namespace aw86224

namespace es8389 {
inline constexpr uint8_t kI2cAddress = 0x10;
inline constexpr int kMclkMultiple = 256;
inline constexpr int kSampleRate = 44100;
inline constexpr int kBitsPerSample = 16;
inline constexpr int kChannel = 2;
}  // namespace es8389

namespace spiffs {
inline constexpr const char* kBasePath = "/spiffs";
}  // namespace spiffs

namespace bhi260ap {
inline constexpr uint8_t kI2cAddress = 0x29;
}  // namespace bhi260ap

namespace qmc6310n {
inline constexpr uint8_t kI2cAddress = 0x3C;
}  // namespace qmc6310n

namespace sgm38121 {
inline constexpr uint8_t kI2cAddress = 0x28;
}  // namespace sgm38121

enum class CameraType {
  kUnknown,
  kSc2336,
  kOv2710,
  kOv5645,
};

namespace camera {
#if defined(CONFIG_CAMERA_TYPE_SC2336)
inline constexpr CameraType kType = CameraType::kSc2336;
inline constexpr const char* kName = "sc2336";
#elif defined(CONFIG_CAMERA_TYPE_OV2710)
inline constexpr CameraType kType = CameraType::kOv2710;
inline constexpr const char* kName = "ov2710";
#elif defined(CONFIG_CAMERA_TYPE_OV5645)
inline constexpr CameraType kType = CameraType::kOv5645;
inline constexpr const char* kName = "ov5645";
#else
inline constexpr CameraType kType = CameraType::kUnknown;
inline constexpr const char* kName = "unknown";
#endif

#if defined(CONFIG_CAMERA_PIXEL_FORMAT_RGB565)
inline constexpr int kBitsPerPixel = 16;
inline constexpr const char* kPixelFormat = "rgb565";
#elif defined(CONFIG_CAMERA_PIXEL_FORMAT_RGB888)
inline constexpr int kBitsPerPixel = 24;
inline constexpr const char* kPixelFormat = "rgb888";
#else
inline constexpr int kBitsPerPixel = 0;
inline constexpr const char* kPixelFormat = "unknown";
#endif

inline constexpr int kBufferCount = 2;
}  // namespace camera

namespace st25r3916 {
inline constexpr uint8_t kI2cAddress = 0x50;
}  // namespace st25r3916

namespace infrared {
inline constexpr int kResolutionHz = 1000000;
inline constexpr int kNecDecodeMargin = 200;
}  // namespace infrared

namespace sd {
inline constexpr const char* kBasePath = "/sdcard";
}  // namespace sd

namespace nrf9151 {
inline constexpr int kDefaultBaudRate = 115200;
}  // namespace nrf9151
}  // namespace device
}  // namespace lilygo_device_driver::t_display_p4_air
