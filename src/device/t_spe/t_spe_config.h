/*
 * @Description: T-SPE 板级硬件配置
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-04-16 15:35:27
 * @License: GPL 3.0
 */

#pragma once

namespace lilygo_device_driver::t_spe {
namespace gpio {
inline constexpr int kGpio0_50mhzSwitch = 33;

namespace lan8671 {
inline constexpr int kWakeUp = 13;
inline constexpr int kInt = 4;
inline constexpr int kReceiveError = 18;
}  // namespace lan8671

namespace td301d485h_a {
inline constexpr int kTx = 15;
inline constexpr int kRx = 5;
}  // namespace td301d485h_a
}  // namespace gpio
}  // namespace lilygo_device_driver::t_spe
