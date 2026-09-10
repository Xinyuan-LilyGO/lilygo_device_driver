/*
 * @Description: T-Display-P4 V2 板级硬件配置
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-09-10 11:54:28
 * @License: GPL 3.0
 */

#pragma once

#include <cstdint>

#include "cpp_bus_driver.h"

namespace lilygo_device_driver::t_display_p4 {
namespace gpio {
namespace button {
inline constexpr int kEsp32p4Boot = 35;
inline constexpr int kKey = 26;
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
inline constexpr auto kGpsRst = cpp_bus_driver::Xl95x5::Pin::kIo0;
inline constexpr auto kGpsWakeUp = cpp_bus_driver::Xl95x5::Pin::kIo1;
inline constexpr auto kEsp32c5Boot = cpp_bus_driver::Xl95x5::Pin::kIo4;
inline constexpr auto kEsp32c5En = cpp_bus_driver::Xl95x5::Pin::kIo5;
inline constexpr auto kScreenRst = cpp_bus_driver::Xl95x5::Pin::kIo6;
inline constexpr auto kNs4150En = cpp_bus_driver::Xl95x5::Pin::kIo7;
inline constexpr auto kTouchRst = cpp_bus_driver::Xl95x5::Pin::kIo11;
inline constexpr auto kLed = cpp_bus_driver::Xl95x5::Pin::kIo12;
inline constexpr auto kUsbPhyPowerEn = cpp_bus_driver::Xl95x5::Pin::kIo13;
inline constexpr auto kLr2021Rst = cpp_bus_driver::Xl95x5::Pin::kIo15;
inline constexpr auto kLr2021PowerEn = cpp_bus_driver::Xl95x5::Pin::kIo16;
inline constexpr auto kSdPowerEn = cpp_bus_driver::Xl95x5::Pin::kIo17;
}  // namespace xl9535

namespace hi8561 {
inline constexpr int kTouchSda = i2c::kPort2Sda;
inline constexpr int kTouchScl = i2c::kPort2Scl;
inline constexpr int kTouchInt = 52;
}  // namespace hi8561

namespace sy7200a {
inline constexpr int kEn = 50;
}  // namespace sy7200a

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
inline constexpr int kAdcData = 21;
inline constexpr int kDacData = 22;
inline constexpr int kBclk = 23;
inline constexpr int kMclk = 13;
inline constexpr int kWsLrck = 20;
}  // namespace es8389

namespace sgm38121 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
}  // namespace sgm38121

namespace lr2021 {
inline constexpr int kCs = 7;
inline constexpr int kBusy = 6;
inline constexpr int kInt = 5;
inline constexpr int kSclk = spi::kPort1Sclk;
inline constexpr int kMosi = spi::kPort1Mosi;
inline constexpr int kMiso = spi::kPort1Miso;
}  // namespace lr2021

namespace sd {
inline constexpr int kDetect = 49;
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

namespace gt9895 {
inline constexpr int kSda = i2c::kPort2Sda;
inline constexpr int kScl = i2c::kPort2Scl;
inline constexpr int kInt = 52;
}  // namespace gt9895

namespace l76k {
inline constexpr int kTx = 37;
inline constexpr int kRx = 38;
inline constexpr int kPps = 36;
}  // namespace l76k

}  // namespace gpio

namespace device {
namespace model {
inline constexpr const char* kVersion = "v2.0";
}  // namespace model

namespace battery {
inline constexpr const char* kChargerChipName = "axp517";
inline constexpr const char* kFuelGaugeChipName = "axp517";
inline constexpr uint16_t kCapacityMah = 1000;
}  // namespace battery

namespace xl9535 {
inline constexpr uint8_t kI2cAddress = 0x20;
inline constexpr int kSdPowerEnabled = 1;
inline constexpr int kSdPowerDisabled = 0;
// 屏幕、触摸和 LR2021 的复位控制经 NMOS 反相。
inline constexpr int kResetAsserted = 1;
inline constexpr int kResetReleased = 0;
}  // namespace xl9535

namespace sy7200a {
// EN/PWM 引脚推荐使用 20 kHz～1 MHz 调光频率。
inline constexpr uint32_t kPwmFrequencyHz = 20000;
}  // namespace sy7200a

namespace axp517 {
inline constexpr uint8_t kI2cAddress = 0x34;
}  // namespace axp517

namespace aw86224 {
inline constexpr uint8_t kI2cAddress = 0x58;
inline constexpr int32_t kI2cFrequencyHz = 500000;
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

namespace lr2021 {
inline constexpr int32_t kSpiFrequencyHz = 10000000;
}  // namespace lr2021

namespace sgm38121 {
inline constexpr uint8_t kI2cAddress = 0x28;
}  // namespace sgm38121

namespace sd {
inline constexpr const char* kBasePath = "/sdcard";
inline constexpr bool kDiskStatusCheckEnabled = false;
}  // namespace sd

}  // namespace device
}  // namespace lilygo_device_driver::t_display_p4
