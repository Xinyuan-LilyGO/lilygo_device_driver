/*
 * @Description: T-CAN485-C5 板级硬件配置
 * @Author: LILYGO_L
 * @Date: 2026-06-12 12:00:00
 * @LastEditTime: 2026-08-26 12:00:00
 * @License: GPL 3.0
 */

#pragma once

namespace lilygo_device_driver::t_can485_c5 {
namespace gpio {
namespace can {
inline constexpr int kTx = 8;
inline constexpr int kRx = 9;
}  // namespace can

namespace spi {
inline constexpr int kMiso = 2;
inline constexpr int kMosi = 7;
inline constexpr int kSclk = 6;
}  // namespace spi

namespace sd {
inline constexpr int kMiso = spi::kMiso;
inline constexpr int kMosi = spi::kMosi;
inline constexpr int kSclk = spi::kSclk;
inline constexpr int kCs = 10;
}  // namespace sd

namespace adc {
inline constexpr int kVoltage = 4;
}  // namespace adc

namespace rs485 {
inline constexpr int kTx = 0;
inline constexpr int kRx = 1;
}  // namespace rs485

namespace button {
inline constexpr int kEsp32c5Boot = 28;
}  // namespace button

namespace ws2812 {
inline constexpr int kData = 3;
}  // namespace ws2812

namespace digital_input {
inline constexpr int kOptocoupler = 24;
}  // namespace digital_input

namespace w5500 {
inline constexpr int kMiso = spi::kMiso;
inline constexpr int kMosi = spi::kMosi;
inline constexpr int kSclk = spi::kSclk;
inline constexpr int kCs = 23;
inline constexpr int kInterrupt = 27;
inline constexpr int kReset = 5;
}  // namespace w5500
}  // namespace gpio

namespace device {
namespace adc {
inline constexpr int kInputScaleNumerator = 526;
inline constexpr int kInputScaleDenominator = 56;
}  // namespace adc

namespace digital_input {
inline constexpr int kActiveLevel = 0;
}  // namespace digital_input

namespace w5500 {
inline constexpr int kPhyAddress = 0;
}  // namespace w5500

namespace ws2812 {
inline constexpr int kLedCount = 1;
}  // namespace ws2812
}  // namespace device
}  // namespace lilygo_device_driver::t_can485_c5
