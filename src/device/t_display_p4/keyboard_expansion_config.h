#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "sdkconfig.h"

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
#include "device/t_display_p4/v2/keyboard_expansion_config.h"
#else
#include "device/t_display_p4/v1/keyboard_expansion_config.h"
#endif

namespace lilygo_device_driver::t_display_p4::keyboard_expansion::device::tca8418 {

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
}  // namespace lilygo_device_driver::t_display_p4::keyboard_expansion::device::tca8418
