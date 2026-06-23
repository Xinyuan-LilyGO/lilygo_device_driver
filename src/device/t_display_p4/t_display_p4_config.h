/*
 * @Description: t_display_p4_config
 * @Author: LILYGO_L
 * @Date: 2024-12-06 10:32:28
 * @LastEditTime: 2026-05-17 23:40:51
 */
#pragma once

#include <cstdint>

#include "cpp_bus_driver_library.h"

namespace lilygo_device_driver::t_display_p4 {
namespace gpio {
namespace button {
inline constexpr int kEsp32p4Boot = 35;
inline constexpr int kKey1 = 49;
inline constexpr int kPower = 11;
}  // namespace button

namespace i2c {
inline constexpr int kPort1Sda = 7;
inline constexpr int kPort1Scl = 8;
inline constexpr int kPort2Sda = 20;
inline constexpr int kPort2Scl = 21;
}  // namespace i2c

namespace spi {
inline constexpr int kPort1Sclk = 2;
inline constexpr int kPort1Mosi = 3;
inline constexpr int kPort1Miso = 4;
}  // namespace spi

namespace xl9535 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
inline constexpr int kInt = 5;
inline constexpr auto kPowerEn3v3 = cpp_bus_driver::Xl95x5::Pin::kIo0;
inline constexpr auto kSky13453Vctl = cpp_bus_driver::Xl95x5::Pin::kIo1;
inline constexpr auto kScreenRst = cpp_bus_driver::Xl95x5::Pin::kIo2;
inline constexpr auto kTouchRst = cpp_bus_driver::Xl95x5::Pin::kIo3;
inline constexpr auto kTouchInt = cpp_bus_driver::Xl95x5::Pin::kIo4;
inline constexpr auto kEthernetRst = cpp_bus_driver::Xl95x5::Pin::kIo5;
inline constexpr auto kPowerEn5v0 = cpp_bus_driver::Xl95x5::Pin::kIo6;
inline constexpr auto kIcm20948Int = cpp_bus_driver::Xl95x5::Pin::kIo7;
inline constexpr auto kUsbPhyPowerEn = cpp_bus_driver::Xl95x5::Pin::kIo10;
inline constexpr auto kGpsWakeUp = cpp_bus_driver::Xl95x5::Pin::kIo11;
inline constexpr auto kRtcInt = cpp_bus_driver::Xl95x5::Pin::kIo12;
inline constexpr auto kEsp32c6WakeUp = cpp_bus_driver::Xl95x5::Pin::kIo13;
inline constexpr auto kEsp32c6En = cpp_bus_driver::Xl95x5::Pin::kIo14;
inline constexpr auto kSdPowerEn = cpp_bus_driver::Xl95x5::Pin::kIo15;
inline constexpr auto kSx1262Rst = cpp_bus_driver::Xl95x5::Pin::kIo16;
inline constexpr auto kSx1262Dio1 = cpp_bus_driver::Xl95x5::Pin::kIo17;
}  // namespace xl9535

namespace es8311 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
inline constexpr int kAdcData = 11;
inline constexpr int kDacData = 10;
inline constexpr int kBclk = 12;
inline constexpr int kMclk = 13;
inline constexpr int kWsLrck = 9;
}  // namespace es8311

namespace aw86224 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
}  // namespace aw86224

namespace sgm38121 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
}  // namespace sgm38121

namespace pcf8563 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
}  // namespace pcf8563

namespace bq27220 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
}  // namespace bq27220

namespace sx1262 {
inline constexpr int kCs = 24;
inline constexpr int kBusy = 6;
inline constexpr int kSclk = spi::kPort1Sclk;
inline constexpr int kMosi = spi::kPort1Mosi;
inline constexpr int kMiso = spi::kPort1Miso;
}  // namespace sx1262

namespace l76k {
inline constexpr int kTx = 22;
inline constexpr int kRx = 23;
}  // namespace l76k

namespace icm20948 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
}  // namespace icm20948

namespace hi8561 {
inline constexpr int kScreenBacklight = 51;
inline constexpr int kTouchSda = i2c::kPort1Sda;
inline constexpr int kTouchScl = i2c::kPort1Scl;
}  // namespace hi8561

namespace gt9895 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
}  // namespace gt9895

namespace camera {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
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

namespace esp32c6 {
inline constexpr int kSdioClk = sdio2::kClk;
inline constexpr int kSdioCmd = sdio2::kCmd;
inline constexpr int kSdioD0 = sdio2::kD0;
inline constexpr int kSdioD1 = sdio2::kD1;
inline constexpr int kSdioD2 = sdio2::kD2;
inline constexpr int kSdioD3 = sdio2::kD3;
}  // namespace esp32c6

namespace ext {
inline constexpr int k2x8PSpiSclk = spi::kPort1Sclk;
inline constexpr int k2x8PSpiMosi = spi::kPort1Mosi;
inline constexpr int k2x8PSpiMiso = spi::kPort1Miso;
inline constexpr int k2x8PIo26 = 26;
inline constexpr int k2x8PIo27 = 27;
inline constexpr int k2x8PIo33 = 33;
inline constexpr int k2x8PIo32 = 32;
inline constexpr int k2x8PIo25 = 25;
inline constexpr int k2x8PIo36 = 36;
inline constexpr int k2x8PIo53 = 53;
inline constexpr int k2x8PIo54 = 54;
inline constexpr int k1x4P1Io47 = 47;
inline constexpr int k1x4P1Io48 = 48;
inline constexpr int k1x4P2Io45 = 45;
inline constexpr int k1x4P2Io46 = 46;
}  // namespace ext

namespace ip101 {
inline constexpr int kPhyRst = -1;
inline constexpr int kRmiiRefClk = 50;
inline constexpr int kRmiiClkOut = -1;
inline constexpr int kRmiiMdc = 31;
inline constexpr int kRmiiMdio = 52;
inline constexpr int kRmiiTxEn = 49;
inline constexpr int kRmiiTxd0 = 34;
inline constexpr int kRmiiTxd1 = 35;
inline constexpr int kRmiiCrsDv = 28;
inline constexpr int kRmiiRxd0 = 29;
inline constexpr int kRmiiRxd1 = 30;
}  // namespace ip101

}  // namespace gpio

namespace device {
namespace ip101 {
inline constexpr int kPhyAddress = 1;
}  // namespace ip101

namespace xl9535 {
inline constexpr uint8_t kI2cAddress = 0x20;
}  // namespace xl9535

namespace es8311 {
inline constexpr uint8_t kI2cAddress = 0x18;
inline constexpr int kMclkMultiple = 256;
inline constexpr int kSampleRate = 44100;
inline constexpr int kBitsPerSample = 16;
inline constexpr int kChannel = 2;
}  // namespace es8311

namespace aw86224 {
inline constexpr uint8_t kI2cAddress = 0x58;
}  // namespace aw86224

namespace sgm38121 {
inline constexpr uint8_t kI2cAddress = 0x28;
}  // namespace sgm38121

namespace pcf8563 {
inline constexpr uint8_t kI2cAddress = 0x51;
}  // namespace pcf8563

namespace bq27220 {
inline constexpr uint8_t kI2cAddress = 0x55;
}  // namespace bq27220

namespace icm20948 {
inline constexpr uint8_t kI2cAddress = 0x68;
}  // namespace icm20948

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
}  // namespace hi8561

namespace rm69a10 {
inline constexpr int kScreenWidth = 568;
inline constexpr int kScreenHeight = 1232;
inline constexpr int kScreenMipiDsiDpiClkMhz = 60;
inline constexpr int kScreenMipiDsiHsync = 50;
inline constexpr int kScreenMipiDsiHbp = 150;
inline constexpr int kScreenMipiDsiHfp = 50;
inline constexpr int kScreenMipiDsiVsync = 40;
inline constexpr int kScreenMipiDsiVbp = 120;
inline constexpr int kScreenMipiDsiVfp = 80;
inline constexpr int kScreenDataLaneNum = 2;
inline constexpr int kScreenLaneBitRateMbps = 1000;
}  // namespace rm69a10

namespace gt9895 {
inline constexpr uint8_t kI2cAddress = 0x5D;
inline constexpr int kMaxXSize = 1060;
inline constexpr int kMaxYSize = 2400;
inline constexpr float kXScaleFactor =
    static_cast<float>(rm69a10::kScreenWidth) / static_cast<float>(kMaxXSize);
inline constexpr float kYScaleFactor =
    static_cast<float>(rm69a10::kScreenHeight) / static_cast<float>(kMaxYSize);
}  // namespace gt9895

namespace sd {
inline constexpr const char* kBasePath = "/sdcard";
}  // namespace sd

}  // namespace device
}  // namespace lilygo_device_driver::t_display_p4
