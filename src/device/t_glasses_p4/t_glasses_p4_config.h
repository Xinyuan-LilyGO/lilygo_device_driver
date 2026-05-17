/*
 * @Description: t_glasses_p4_config
 * @Author: LILYGO_L
 * @Date: 2024-12-06 10:32:28
 * @LastEditTime: 2026-05-17 23:48:21
 */
#pragma once

#include <cstdint>

namespace lilygo_device_driver::t_glasses_p4 {
namespace gpio {
namespace i2c {
inline constexpr int kPort1Sda = 13;
inline constexpr int kPort1Scl = 12;
inline constexpr int kPort2Sda = 45;
inline constexpr int kPort2Scl = 46;
}  // namespace i2c

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

inline constexpr int kChipBoot = 35;

namespace power {
inline constexpr int kEn3v3 = 7;
inline constexpr int kEn5v0 = 54;
}  // namespace power

namespace esp32c6 {
inline constexpr int kEn = 53;
inline constexpr int kSdioClk = sdio2::kClk;
inline constexpr int kSdioCmd = sdio2::kCmd;
inline constexpr int kSdioD0 = sdio2::kD0;
inline constexpr int kSdioD1 = sdio2::kD1;
inline constexpr int kSdioD2 = sdio2::kD2;
inline constexpr int kSdioD3 = sdio2::kD3;
}  // namespace esp32c6

namespace bq27220 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
}  // namespace bq27220

namespace sgm38121 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
}  // namespace sgm38121

namespace es8311 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
inline constexpr int kAdcData = 50;
inline constexpr int kDacData = 52;
inline constexpr int kBclk = 51;
inline constexpr int kMclk = 48;
inline constexpr int kWsLrck = 47;
}  // namespace es8311

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

namespace gz030pcc02 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
inline constexpr int kRst = 21;
}  // namespace gz030pcc02

namespace bhi260ap {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
}  // namespace bhi260ap

namespace sy6970 {
inline constexpr int kSda = i2c::kPort1Sda;
inline constexpr int kScl = i2c::kPort1Scl;
}  // namespace sy6970

namespace aw86224 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
}  // namespace aw86224

namespace bmm350 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
}  // namespace bmm350

namespace usb {
inline constexpr int kHighSpeedEn = 9;
}  // namespace usb

namespace sx1262 {
inline constexpr int kCs = 34;
inline constexpr int kBusy = 28;
inline constexpr int kInt = 29;
inline constexpr int kRst = 27;
inline constexpr int kSclk = spi::kPort1Sclk;
inline constexpr int kMosi = spi::kPort1Mosi;
inline constexpr int kMiso = spi::kPort1Miso;
}  // namespace sx1262

namespace ecx336cn {
inline constexpr int kSclk = spi::kPort1Sclk;
inline constexpr int kMosi = spi::kPort1Mosi;
inline constexpr int kMiso = spi::kPort1Miso;
inline constexpr int kCs = 34;
}  // namespace ecx336cn

namespace s023msafjf10111e1 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
inline constexpr int kRst = 21;
}  // namespace s023msafjf10111e1
}  // namespace gpio

namespace device {
namespace sy6970 {
inline constexpr uint8_t kI2cAddress = 0x6A;
}  // namespace sy6970

namespace bq27220 {
inline constexpr uint8_t kI2cAddress = 0x55;
}  // namespace bq27220

namespace sgm38121 {
inline constexpr uint8_t kI2cAddress = 0x28;
}  // namespace sgm38121

namespace es8311 {
inline constexpr uint8_t kI2cAddress = 0x18;
}  // namespace es8311

namespace gz030pcc02 {
inline constexpr uint8_t kI2cAddress = 0x54;
inline constexpr int kScreenWidth = 1280;
inline constexpr int kScreenHeight = 720;
inline constexpr int kScreenMipiDsiDpiClkMhz = 60;
inline constexpr int kScreenMipiDsiHsync = 40;
inline constexpr int kScreenMipiDsiHbp = 220;
inline constexpr int kScreenMipiDsiHfp = 110;
inline constexpr int kScreenMipiDsiVsync = 5;
inline constexpr int kScreenMipiDsiVbp = 20;
inline constexpr int kScreenMipiDsiVfp = 5;
inline constexpr int kScreenDataLaneNum = 2;
inline constexpr int kScreenLaneBitRateMbps = 1000;
}  // namespace gz030pcc02

namespace camera {
#if defined(CONFIG_CAMERA_PIXEL_FORMAT_RGB565)
inline constexpr int kBitsPerPixel = 16;
#elif defined(CONFIG_CAMERA_PIXEL_FORMAT_RGB888)
inline constexpr int kBitsPerPixel = 24;
#else
#error "Missing required macro definition."
#endif

inline constexpr int kWidth = 1280;
inline constexpr int kHeight = 720;
inline constexpr int kDataLaneNum = 2;
inline constexpr int kLaneBitRateMbps = 1000;
inline constexpr int kMipiDsiDpiClkMhz = 60;
}  // namespace camera

namespace bhi260ap {
inline constexpr uint8_t kI2cAddress = 0x28;
}  // namespace bhi260ap

namespace aw86224 {
inline constexpr uint8_t kI2cAddress = 0x58;
}  // namespace aw86224

namespace bmm350 {
inline constexpr uint8_t kI2cAddress = 0x14;
}  // namespace bmm350

namespace sd {
inline constexpr const char* kBasePath = "/sdcard";
}  // namespace sd

namespace ecx336cn {
inline constexpr int kHeight = 400;
inline constexpr int kWidth = 640;
inline constexpr double kClockMhz = 27.027;
inline constexpr int kScreenRgbDsiHsync = 64;
inline constexpr int kScreenRgbDsiHbp = 58;
inline constexpr int kScreenRgbDsiHfp = 96;
inline constexpr int kScreenRgbDsiVsync = 6;
inline constexpr int kScreenRgbDsiVbp = 32;
inline constexpr int kScreenRgbDsiVfp = 87;
}  // namespace ecx336cn

namespace icn6211 {
inline constexpr uint8_t kI2cAddress = 0x2C;
inline constexpr double kExternalReferenceClockMhz = 26.0;
inline constexpr double kScreenMipiDsiDpiClkMhz = 24.0;
inline constexpr int kScreenMipiDsiHsync = ecx336cn::kScreenRgbDsiHsync;
inline constexpr int kScreenMipiDsiHbp = ecx336cn::kScreenRgbDsiHbp;
inline constexpr int kScreenMipiDsiHfp = ecx336cn::kScreenRgbDsiHfp;
inline constexpr int kScreenMipiDsiVsync = ecx336cn::kScreenRgbDsiVsync;
inline constexpr int kScreenMipiDsiVbp = ecx336cn::kScreenRgbDsiVbp;
inline constexpr int kScreenMipiDsiVfp = ecx336cn::kScreenRgbDsiVfp;
inline constexpr int kScreenDataLaneNum = 2;
inline constexpr int kScreenLaneBitRateMbps = 1000;
}  // namespace icn6211

namespace s023msafjf10111e1 {
inline constexpr uint8_t kI2cAddress = 0x54;
inline constexpr int kScreenWidth = 640;
inline constexpr int kScreenHeight = 400;
inline constexpr int kScreenMipiDsiDpiClkMhz = 24;
inline constexpr int kScreenMipiDsiHsync = 64;
inline constexpr int kScreenMipiDsiHbp = 58;
inline constexpr int kScreenMipiDsiHfp = 96;
inline constexpr int kScreenMipiDsiVsync = 6;
inline constexpr int kScreenMipiDsiVbp = 56;
inline constexpr int kScreenMipiDsiVfp = 63;
inline constexpr int kScreenDataLaneNum = 1;
inline constexpr int kScreenLaneBitRateMbps = 1000;
}  // namespace s023msafjf10111e1

namespace screen {
#if defined(CONFIG_SCREEN_PIXEL_FORMAT_RGB888)
inline constexpr int kBitsPerPixel = 24;
#else
#error "Missing required macro definition."
#endif

#if defined(CONFIG_SCREEN_TYPE_GZ030PCC02)
inline constexpr int kWidth = gz030pcc02::kScreenWidth;
inline constexpr int kHeight = gz030pcc02::kScreenHeight;
inline constexpr auto kMipiDsiDpiClkMhz = gz030pcc02::kScreenMipiDsiDpiClkMhz;
inline constexpr int kMipiDsiHsync = gz030pcc02::kScreenMipiDsiHsync;
inline constexpr int kMipiDsiHbp = gz030pcc02::kScreenMipiDsiHbp;
inline constexpr int kMipiDsiHfp = gz030pcc02::kScreenMipiDsiHfp;
inline constexpr int kMipiDsiVsync = gz030pcc02::kScreenMipiDsiVsync;
inline constexpr int kMipiDsiVbp = gz030pcc02::kScreenMipiDsiVbp;
inline constexpr int kMipiDsiVfp = gz030pcc02::kScreenMipiDsiVfp;
inline constexpr int kDataLaneNum = gz030pcc02::kScreenDataLaneNum;
inline constexpr int kLaneBitRateMbps = gz030pcc02::kScreenLaneBitRateMbps;
#elif defined(CONFIG_SCREEN_TYPE_ICN6211_ECX336CN)
inline constexpr int kWidth = ecx336cn::kWidth;
inline constexpr int kHeight = ecx336cn::kHeight;
inline constexpr auto kMipiDsiDpiClkMhz = icn6211::kScreenMipiDsiDpiClkMhz;
inline constexpr int kMipiDsiHsync = icn6211::kScreenMipiDsiHsync;
inline constexpr int kMipiDsiHbp = icn6211::kScreenMipiDsiHbp;
inline constexpr int kMipiDsiHfp = icn6211::kScreenMipiDsiHfp;
inline constexpr int kMipiDsiVsync = icn6211::kScreenMipiDsiVsync;
inline constexpr int kMipiDsiVbp = icn6211::kScreenMipiDsiVbp;
inline constexpr int kMipiDsiVfp = icn6211::kScreenMipiDsiVfp;
inline constexpr int kDataLaneNum = icn6211::kScreenDataLaneNum;
inline constexpr int kLaneBitRateMbps = icn6211::kScreenLaneBitRateMbps;
#elif defined(CONFIG_SCREEN_TYPE_S023MSAFJF10111E1)
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
#else
#error "Missing required macro definition."
#endif
}  // namespace screen
}  // namespace device
}  // namespace lilygo_device_driver::t_glasses_p4
