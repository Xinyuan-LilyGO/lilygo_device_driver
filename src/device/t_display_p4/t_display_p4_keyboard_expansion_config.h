/*
 * @Description: T-Display-P4 键盘扩展硬件配置
 * @Author: LILYGO_L
 * @Date: 2024-12-06 10:32:28
 * @LastEditTime: 2026-05-18 18:29:34
 */
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "t_display_p4_config.h"

namespace lilygo_device_driver::t_display_p4 {
namespace keyboard_expansion {
namespace base_gpio = ::lilygo_device_driver::t_display_p4::gpio;

namespace gpio {
namespace i2c {
inline constexpr int kPort3Sda = base_gpio::ext::k1x4P2Io46;
inline constexpr int kPort3Scl = base_gpio::ext::k1x4P2Io45;
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
inline constexpr int kEn = base_gpio::ext::k1x4P1Io47;
}  // namespace sy7200a

namespace tca8418 {
inline constexpr int kSda = i2c::kPort3Sda;
inline constexpr int kScl = i2c::kPort3Scl;
inline constexpr int kInt = base_gpio::ext::k1x4P1Io48;
}  // namespace tca8418

namespace t_mix_rf {
namespace cc1101 {
inline constexpr int kCs = base_gpio::ext::k2x8PIo36;
inline constexpr int kSclk = base_gpio::ext::k2x8PSpiSclk;
inline constexpr int kMosi = base_gpio::ext::k2x8PSpiMosi;
inline constexpr int kMiso = base_gpio::ext::k2x8PSpiMiso;
inline constexpr int kGdo0 = base_gpio::ext::k2x8PIo25;
inline constexpr int kGdo2 = base_gpio::ext::k2x8PIo33;
inline constexpr int kInt = kGdo0;
inline constexpr int kBusy = kGdo2;
}  // namespace cc1101

namespace nrf24l01 {
inline constexpr int kCs = base_gpio::ext::k2x8PIo54;
inline constexpr int kSclk = base_gpio::ext::k2x8PSpiSclk;
inline constexpr int kMosi = base_gpio::ext::k2x8PSpiMosi;
inline constexpr int kMiso = base_gpio::ext::k2x8PSpiMiso;
inline constexpr int kCe = base_gpio::ext::k2x8PIo53;
inline constexpr int kInt = base_gpio::ext::k2x8PIo32;
}  // namespace nrf24l01

namespace st25r3916 {
inline constexpr int kCs = base_gpio::ext::k2x8PIo27;
inline constexpr int kSclk = base_gpio::ext::k2x8PSpiSclk;
inline constexpr int kMosi = base_gpio::ext::k2x8PSpiMosi;
inline constexpr int kMiso = base_gpio::ext::k2x8PSpiMiso;
inline constexpr int kInt = base_gpio::ext::k2x8PIo26;
}  // namespace st25r3916

namespace lr1121 {
inline constexpr int kSclk = base_gpio::ext::k2x8PSpiSclk;
inline constexpr int kMosi = base_gpio::ext::k2x8PSpiMosi;
inline constexpr int kMiso = base_gpio::ext::k2x8PSpiMiso;
}  // namespace lr1121
}  // namespace t_mix_rf
}  // namespace gpio

namespace device {
namespace xl9555 {
inline constexpr uint8_t kI2cAddress = 0x20;
}  // namespace xl9555

namespace tca8418 {
inline constexpr uint8_t kI2cAddress = 0x34;
inline constexpr int kKeypadScanWidth = 10;
inline constexpr int kKeypadScanHeight = 7;
inline constexpr std::size_t kKeyCount = 68;

enum class KeyCode : uint8_t {
  kUnknown,
  kCharacter,
  kEscape,
  kBackspace,
  kEnter,
  kTab,
  kUp,
  kDown,
  kLeft,
  kRight,
  kCapsLock,
  kShift,
  kControl,
  kAlt,
  kMeta,
  kFunction,
  kRecord,
  kF1,
  kF2,
  kF3,
  kF4,
  kF5,
  kF6,
  kF7,
  kF8,
  kF9,
  kF10,
  kF11,
};

struct KeyMapping {
  KeyCode key = KeyCode::kUnknown;
  char character = '\0';
  char function_character = '\0';
};

// 按 TCA8418 事件编号排列的实体键盘主键值与 Fn 二级键值。
inline constexpr std::array<KeyMapping, kKeyCount> kMap = {{
    {KeyCode::kF1},
    {KeyCode::kF2},
    {KeyCode::kF3},
    {KeyCode::kF4},
    {KeyCode::kF5},
    {KeyCode::kF6},
    {KeyCode::kF7},
    {KeyCode::kF8},
    {KeyCode::kF9},
    {KeyCode::kF10},
    {KeyCode::kEscape},
    {KeyCode::kEscape},
    {KeyCode::kCharacter, '1', '!'},
    {KeyCode::kCharacter, '2', '@'},
    {KeyCode::kCharacter, '3', '#'},
    {KeyCode::kCharacter, '4', '$'},
    {KeyCode::kCharacter, '5', '%'},
    {KeyCode::kCharacter, '6', '^'},
    {KeyCode::kCharacter, '7', '&'},
    {KeyCode::kCharacter, '8', '*'},
    {KeyCode::kCharacter, 'q', '\''},
    {KeyCode::kCharacter, 'w', '_'},
    {KeyCode::kCharacter, 'e', '-'},
    {KeyCode::kCharacter, 'r', '+'},
    {KeyCode::kCharacter, 't', '='},
    {KeyCode::kCharacter, 'y', '\\'},
    {KeyCode::kCharacter, 'u', '|'},
    {KeyCode::kCharacter, 'i', ';'},
    {KeyCode::kCharacter, 'o', ':'},
    {KeyCode::kCharacter, 'p', '"'},
    {KeyCode::kCapsLock},
    {KeyCode::kCharacter, 'a', '~'},
    {KeyCode::kCharacter, 's', '['},
    {KeyCode::kCharacter, 'd', ']'},
    {KeyCode::kCharacter, 'f', '{'},
    {KeyCode::kCharacter, 'g', '}'},
    {KeyCode::kCharacter, 'h', ','},
    {KeyCode::kCharacter, 'j', '`'},
    {KeyCode::kCharacter, 'k', '/'},
    {KeyCode::kCharacter, 'l', '?'},
    {KeyCode::kAlt},
    {KeyCode::kCharacter, 'z'},
    {KeyCode::kCharacter, 'x'},
    {KeyCode::kCharacter, 'c'},
    {KeyCode::kCharacter, 'v'},
    {KeyCode::kCharacter, 'b', '.'},
    {KeyCode::kCharacter, 'n', '<'},
    {KeyCode::kCharacter, 'm', '>'},
    {KeyCode::kControl},
    {KeyCode::kUp},
    {KeyCode::kFunction},
    {KeyCode::kMeta},
    {KeyCode::kShift},
    {KeyCode::kTab},
    {KeyCode::kCharacter, ' ', ' '},
    {KeyCode::kCharacter, ' ', ' '},
    {KeyCode::kCharacter, ' ', ' '},
    {KeyCode::kFunction},
    {KeyCode::kLeft},
    {KeyCode::kDown},
    {KeyCode::kF11},
    {KeyCode::kCharacter, '9', '('},
    {KeyCode::kBackspace},
    {KeyCode::kEnter},
    {KeyCode::kRecord},
    {KeyCode::kEnter},
    {KeyCode::kCharacter, '0', ')'},
    {KeyCode::kRight},
}};
}  // namespace tca8418

namespace sy7200a {
inline constexpr uint32_t kPwmFrequencyHz = 20000;
}  // namespace sy7200a
}  // namespace device
}  // namespace keyboard_expansion
}  // namespace lilygo_device_driver::t_display_p4
