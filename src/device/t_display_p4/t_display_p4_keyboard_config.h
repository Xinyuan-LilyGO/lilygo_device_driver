/*
 * @Description: t_display_p4_keyboard_config
 * @Author: LILYGO_L
 * @Date: 2024-12-06 10:32:28
 * @LastEditTime: 2026-05-17 23:48:01
 */
#pragma once

#include <cstdint>
#include <string>

#include "t_display_p4_config.h"

namespace lilygo_device_driver::t_display_p4 {
namespace gpio {
namespace i2c {
inline constexpr int kPort3Sda = ext::k1x4p2Io46;
inline constexpr int kPort3Scl = ext::k1x4p2Io45;
}  // namespace i2c

namespace xl9555 {
inline constexpr int kSda = i2c::kPort3Sda;
inline constexpr int kScl = i2c::kPort3Scl;
inline constexpr auto kTMixrfEn = cpp_bus_driver::Xl95x5::Pin::kIo0;
inline constexpr auto kTMixrfCc1101RfSwitch0 =
    cpp_bus_driver::Xl95x5::Pin::kIo1;
inline constexpr auto kTMixrfCc1101RfSwitch1 =
    cpp_bus_driver::Xl95x5::Pin::kIo2;
inline constexpr auto kLed1 = cpp_bus_driver::Xl95x5::Pin::kIo3;
inline constexpr auto kLed2 = cpp_bus_driver::Xl95x5::Pin::kIo4;
inline constexpr auto kLed3 = cpp_bus_driver::Xl95x5::Pin::kIo5;
inline constexpr auto kTca8418Rst = cpp_bus_driver::Xl95x5::Pin::kIo6;
inline constexpr auto kTMixrfLr1121Int = cpp_bus_driver::Xl95x5::Pin::kIo7;
inline constexpr auto kTMixrfLr1121Rst = cpp_bus_driver::Xl95x5::Pin::kIo10;
inline constexpr auto kTMixrfLr1121Cs = cpp_bus_driver::Xl95x5::Pin::kIo11;
inline constexpr auto kTMixrfLr1121Busy = cpp_bus_driver::Xl95x5::Pin::kIo12;
}  // namespace xl9555

namespace sy7200a {
inline constexpr int kEnPwm = ext::k1x4p1Io47;
}  // namespace sy7200a

namespace keyboard {
inline constexpr int kBl = sy7200a::kEnPwm;
}  // namespace keyboard

namespace tca8418 {
inline constexpr int kSda = i2c::kPort3Sda;
inline constexpr int kScl = i2c::kPort3Scl;
inline constexpr int kInt = ext::k1x4p1Io48;
inline constexpr int kBl = keyboard::kBl;
}  // namespace tca8418

namespace tmixrf {
namespace cc1101 {
inline constexpr int kCs = ext::k2x8pIo36;
inline constexpr int kSclk = ext::k2x8pSpiSclk;
inline constexpr int kMosi = ext::k2x8pSpiMosi;
inline constexpr int kMiso = ext::k2x8pSpiMiso;
inline constexpr int kGdo0 = ext::k2x8pIo25;
inline constexpr int kGdo2 = ext::k2x8pIo33;
inline constexpr int kInt = kGdo0;
inline constexpr int kBusy = kGdo2;
}  // namespace cc1101

namespace nrf24l01 {
inline constexpr int kCs = ext::k2x8pIo54;
inline constexpr int kSclk = ext::k2x8pSpiSclk;
inline constexpr int kMosi = ext::k2x8pSpiMosi;
inline constexpr int kMiso = ext::k2x8pSpiMiso;
inline constexpr int kCe = ext::k2x8pIo53;
inline constexpr int kInt = ext::k2x8pIo32;
}  // namespace nrf24l01

namespace st25r3916 {
inline constexpr int kCs = ext::k2x8pIo27;
inline constexpr int kSclk = ext::k2x8pSpiSclk;
inline constexpr int kMosi = ext::k2x8pSpiMosi;
inline constexpr int kMiso = ext::k2x8pSpiMiso;
inline constexpr int kInt = ext::k2x8pIo26;
}  // namespace st25r3916

namespace lr1121 {
inline constexpr int kSclk = ext::k2x8pSpiSclk;
inline constexpr int kMosi = ext::k2x8pSpiMosi;
inline constexpr int kMiso = ext::k2x8pSpiMiso;
}  // namespace lr1121
}  // namespace tmixrf
}  // namespace gpio

namespace device {
namespace xl9555 {
inline constexpr uint8_t kI2cAddress = 0x20;
}  // namespace xl9555

namespace tca8418 {
inline constexpr uint8_t kI2cAddress = 0x34;
inline constexpr int kKeypadScanWidth = 10;
inline constexpr int kKeypadScanHeight = 7;

// TCA8418 key map.
constexpr const std::string kMap[] = {"F1", "F2", "F3", "F4", "F5", "F6", "F7",
    "F8", "F9", "F10", "Esc", "Esc", "1", "2", "3", "4", "5", "6", "7", "8",
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "Caps", "a", "s", "d",
    "f", "g", "h", "j", "k", "l", "Alt", "z", "x", "c", "v", "b", "n", "m",
    "Ctrl", "Up", "Fn", "Win", "Shift", "Tab", "Space", "Space", "Space", "Fn",
    "Left", "Down", "F11", "9", "Del", "Enter", "Record", "Enter", "0",
    "Right"};
}  // namespace tca8418
}  // namespace device
}  // namespace lilygo_device_driver::t_display_p4
