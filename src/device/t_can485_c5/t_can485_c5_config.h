/*
 * @Description: t_can485_c5_config
 * @Author: LILYGO_L
 * @Date: 2026-06-12 12:00:00
 * @LastEditTime: 2026-06-12 12:00:00
 * @License: GPL 3.0
 */

#pragma once

#include <cstdint>

namespace lilygo_device_driver::t_can485_c5 {
namespace gpio {
namespace can {
inline constexpr int kTx = 0;
inline constexpr int kRx = 1;
}  // namespace can

namespace sd {
inline constexpr int kMiso = 2;
inline constexpr int kMosi = 7;
inline constexpr int kSclk = 6;
inline constexpr int kCs = 10;
}  // namespace sd

namespace adc {
inline constexpr int kChannel1 = 3;
inline constexpr int kChannel2 = 4;
inline constexpr int kChannel3 = 5;
}  // namespace adc

namespace rs485 {
inline constexpr int kTx = 8;
inline constexpr int kRx = 9;
}  // namespace rs485

namespace button {
inline constexpr int kEsp32c5Boot = 28;
}  // namespace button

namespace led {
inline constexpr int kRed = 24;
inline constexpr int kGreen = 25;
inline constexpr int kBlue = 26;
}  // namespace led

namespace ws2812 {
inline constexpr int kData = 23;
}  // namespace ws2812
}  // namespace gpio

namespace device {
namespace ws2812 {
inline constexpr int kLedCount = 1;
inline constexpr uint32_t kResolutionHz = 10000000;
}  // namespace ws2812
}  // namespace device
}  // namespace lilygo_device_driver::t_can485_c5
