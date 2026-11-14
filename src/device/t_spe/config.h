/*
 * @Description: T-SPE 板级硬件配置
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
