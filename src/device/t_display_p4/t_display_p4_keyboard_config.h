/*
 * @Description: t_display_p4_keyboard_config
 * @Author: LILYGO_L
 * @Date: 2024-12-06 10:32:28
 * @LastEditTime: 2026-05-14 22:56:02
 */
#pragma once

#include <cstdint>
#include <string>

#include "t_display_p4_config.h"

namespace lilygo_device_driver::t_display_p4 {

namespace gpio {

inline constexpr int kI2c3Sda = kExt1x4p2Io46;
inline constexpr int kI2c3Scl = kExt1x4p2Io45;

inline constexpr int kXl9555Sda = kI2c3Sda;
inline constexpr int kXl9555Scl = kI2c3Scl;
inline constexpr auto kXl9555TMixrfEn =
    cpp_bus_driver::Xl95x5::Pin::kIo0;
inline constexpr auto kXl9555TMixrfCc1101RfSwitch0 =
    cpp_bus_driver::Xl95x5::Pin::kIo1;
inline constexpr auto kXl9555TMixrfCc1101RfSwitch1 =
    cpp_bus_driver::Xl95x5::Pin::kIo2;
inline constexpr auto kXl9555Led1 =
    cpp_bus_driver::Xl95x5::Pin::kIo3;
inline constexpr auto kXl9555Led2 =
    cpp_bus_driver::Xl95x5::Pin::kIo4;
inline constexpr auto kXl9555Led3 =
    cpp_bus_driver::Xl95x5::Pin::kIo5;
inline constexpr auto kXl9555Tca8418Rst =
    cpp_bus_driver::Xl95x5::Pin::kIo6;
inline constexpr auto kXl9555TMixrfLr1121Int =
    cpp_bus_driver::Xl95x5::Pin::kIo7;
inline constexpr auto kXl9555TMixrfLr1121Rst =
    cpp_bus_driver::Xl95x5::Pin::kIo10;
inline constexpr auto kXl9555TMixrfLr1121Cs =
    cpp_bus_driver::Xl95x5::Pin::kIo11;
inline constexpr auto kXl9555TMixrfLr1121Busy =
    cpp_bus_driver::Xl95x5::Pin::kIo12;

inline constexpr int kSy7200aEnPwm = kExt1x4p1Io47;
inline constexpr int kKeyboardBl = kSy7200aEnPwm;

inline constexpr int kTca8418Sda = kIic3Sda;
inline constexpr int kTca8418Scl = kIic3Scl;
inline constexpr int kTca8418Int = kExt1x4p1Io48;
inline constexpr int kTca8418Bl = kKeyboardBl;

inline constexpr int kTMixrfCc1101Cs = kExt2x8pIo36;
inline constexpr int kTMixrfCc1101Sclk = kExt2x8pSpiSclk;
inline constexpr int kTMixrfCc1101Mosi = kExt2x8pSpiMosi;
inline constexpr int kTMixrfCc1101Miso = kExt2x8pSpiMiso;
inline constexpr int kTMixrfCc1101Gdo0 = kExt2x8pIo25;
inline constexpr int kTMixrfCc1101Gdo2 = kExt2x8pIo33;
inline constexpr int kTMixrfCc1101Int = kTMixrfCc1101Gdo0;
inline constexpr int kTMixrfCc1101Busy = kTMixrfCc1101Gdo2;

inline constexpr int kTMixrfNrf24l01Cs = kExt2x8pIo54;
inline constexpr int kTMixrfNrf24l01Sclk = kExt2x8pSpiSclk;
inline constexpr int kTMixrfNrf24l01Mosi = kExt2x8pSpiMosi;
inline constexpr int kTMixrfNrf24l01Miso = kExt2x8pSpiMiso;
inline constexpr int kTMixrfNrf24l01Ce = kExt2x8pIo53;
inline constexpr int kTMixrfNrf24l01Int = kExt2x8pIo32;

inline constexpr int kTMixrfSt25r3916Cs = kExt2x8pIo27;
inline constexpr int kTMixrfSt25r3916Sclk = kExt2x8pSpiSclk;
inline constexpr int kTMixrfSt25r3916Mosi = kExt2x8pSpiMosi;
inline constexpr int kTMixrfSt25r3916Miso = kExt2x8pSpiMiso;
inline constexpr int kTMixrfSt25r3916Int = kExt2x8pIo26;

inline constexpr int kTMixrfLr1121Sclk = kExt2x8pSpiSclk;
inline constexpr int kTMixrfLr1121Mosi = kExt2x8pSpiMosi;
inline constexpr int kTMixrfLr1121Miso = kExt2x8pSpiMiso;

}  // namespace gpio

namespace device {

inline constexpr uint8_t kXl9555I2cAddress = 0x20;
inline constexpr uint8_t kTca8418I2cAddress = 0x34;
inline constexpr int kTca8418KeypadScanWidth = 10;
inline constexpr int kTca8418KeypadScanHeight = 7;

// TCA8418 key map.
constexpr const std::string Tca8418_Map[] = {
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
    "Esc", "Esc", "1", "2", "3", "4", "5", "6", "7", "8",
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
    "Caps", "a", "s", "d", "f", "g", "h", "j", "k", "l",
    "Alt", "z", "x", "c", "v", "b", "n", "m", "Ctrl", "Up",
    "Fn", "Win", "Shift", "Tab", "Space", "Space", "Space", "Fn",
    "Left", "Down", "F11", "9", "Del", "Enter", "Record", "Enter",
    "0", "Right"};

}  // namespace device

}  // namespace lilygo_device_driver::t_display_p4
