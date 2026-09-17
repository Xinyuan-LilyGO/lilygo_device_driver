/*
 * @Description: T-Display-P4 V2 键盘扩展硬件配置
 * @Author: LILYGO_L
 * @Date: 2024-12-06 10:32:28
 * @LastEditTime: 2026-08-21 18:02:24
 */
#pragma once

#include <cstdint>

#include "device/t_display_p4/v2/config.h"

namespace lilygo_device_driver::t_display_p4 {
namespace keyboard_expansion {
namespace base_gpio = ::lilygo_device_driver::t_display_p4::gpio;

namespace gpio {
namespace i2c {
inline constexpr int kPort3Sda = 45;
inline constexpr int kPort3Scl = 46;
}  // namespace i2c

namespace xl9555 {
inline constexpr int kSda = i2c::kPort3Sda;
inline constexpr int kScl = i2c::kPort3Scl;
inline constexpr auto kTMixRfEn = cpp_bus_driver::Xl95x5::Pin::kIo0;
inline constexpr auto kTMixRfCc1101RfSwitch0 =
    cpp_bus_driver::Xl95x5::Pin::kIo1;
inline constexpr auto kTMixRfCc1101RfSwitch1 =
    cpp_bus_driver::Xl95x5::Pin::kIo2;
inline constexpr auto kLed1 = cpp_bus_driver::Xl95x5::Pin::kIo3;
inline constexpr auto kLed2 = cpp_bus_driver::Xl95x5::Pin::kIo4;
inline constexpr auto kLed3 = cpp_bus_driver::Xl95x5::Pin::kIo5;
inline constexpr auto kTca8418Rst = cpp_bus_driver::Xl95x5::Pin::kIo6;
}  // namespace xl9555

namespace sy7200a {
inline constexpr int kEn = 48;
}  // namespace sy7200a

namespace tca8418 {
inline constexpr int kSda = i2c::kPort3Sda;
inline constexpr int kScl = i2c::kPort3Scl;
inline constexpr int kInt = 47;
}  // namespace tca8418

namespace t_mix_rf {
namespace cc1101 {
inline constexpr int kCs = 31;
inline constexpr int kSclk = base_gpio::spi::kPort1Sclk;
inline constexpr int kMosi = base_gpio::spi::kPort1Mosi;
inline constexpr int kMiso = base_gpio::spi::kPort1Miso;
inline constexpr int kGdo0 = 30;
inline constexpr int kGdo2 = 29;
inline constexpr int kInt = kGdo0;
inline constexpr int kBusy = kGdo2;
}  // namespace cc1101

namespace nrf24l01 {
inline constexpr int kCs = 34;
inline constexpr int kSclk = base_gpio::spi::kPort1Sclk;
inline constexpr int kMosi = base_gpio::spi::kPort1Mosi;
inline constexpr int kMiso = base_gpio::spi::kPort1Miso;
inline constexpr int kCe = 33;
inline constexpr int kInt = 32;
}  // namespace nrf24l01

namespace st25r3916 {
inline constexpr int kCs = 28;
inline constexpr int kSclk = base_gpio::spi::kPort1Sclk;
inline constexpr int kMosi = base_gpio::spi::kPort1Mosi;
inline constexpr int kMiso = base_gpio::spi::kPort1Miso;
inline constexpr int kInt = 27;
}  // namespace st25r3916

namespace lr1121 {
inline constexpr int kSclk = base_gpio::spi::kPort1Sclk;
inline constexpr int kMosi = base_gpio::spi::kPort1Mosi;
inline constexpr int kMiso = base_gpio::spi::kPort1Miso;
}  // namespace lr1121
}  // namespace t_mix_rf
}  // namespace gpio

namespace device {
namespace i2c {
inline constexpr i2c_port_t kPort = I2C_NUM_1;
}  // namespace i2c

namespace xl9555 {
inline constexpr uint8_t kI2cAddress = 0x20;
}  // namespace xl9555

namespace cc1101 {
inline constexpr int32_t kSpiFrequencyHz = 4000000;
}  // namespace cc1101

namespace tca8418 {
inline constexpr uint8_t kI2cAddress = 0x34;
inline constexpr int kKeypadScanWidth = 10;
inline constexpr int kKeypadScanHeight = 7;
}  // namespace tca8418

namespace sy7200a {
inline constexpr uint32_t kPwmFrequencyHz = 20000;
}  // namespace sy7200a
}  // namespace device
}  // namespace keyboard_expansion
}  // namespace lilygo_device_driver::t_display_p4
