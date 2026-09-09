#pragma once

#include <cstdint>

#include "device/common/camera_type.h"
#include "sdkconfig.h"

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
#include "device/t_display_p4/v2/config.h"
#else
#include "device/t_display_p4/v1/config.h"
#endif

namespace lilygo_device_driver::t_display_p4::device {
namespace model {
inline constexpr const char* kName = "T-Display-P4";
}  // namespace model

namespace screen {
inline constexpr int kRotationDirection = 0;

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_SCREEN_PIXEL_FORMAT_RGB565)
inline constexpr int kBitsPerPixel = 16;
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_SCREEN_PIXEL_FORMAT_RGB888)
inline constexpr int kBitsPerPixel = 24;
#else
#error "Missing required macro definition."
#endif
}  // namespace screen

namespace hi8561 {
inline constexpr int kScreenWidth = 540;
inline constexpr int kScreenHeight = 1168;
inline constexpr int kScreenMipiDsiDpiClkMhz = 60;
inline constexpr int kScreenMipiDsiHsync = 28;
inline constexpr int kScreenMipiDsiHbp = 26;
inline constexpr int kScreenMipiDsiHfp = 20;
inline constexpr int kScreenMipiDsiVsync = 2;
inline constexpr int kScreenMipiDsiVbp = 22;
inline constexpr int kScreenMipiDsiVfp = 200;
inline constexpr int kScreenDataLaneNum = 2;
inline constexpr int kScreenLaneBitRateMbps = 1000;
inline constexpr uint8_t kTouchI2cAddress = 0x68;
inline constexpr int32_t kI2cFrequencyHz = 400000;
}  // namespace hi8561

namespace rm69a10 {
inline constexpr int kScreenWidth = 568;
inline constexpr int kScreenHeight = 1232;
inline constexpr int kScreenMipiDsiDpiClkMhz = 60;
inline constexpr int kScreenMipiDsiHsync = 50;
inline constexpr int kScreenMipiDsiHbp = 150;
inline constexpr int kScreenMipiDsiHfp = 50;
inline constexpr int kScreenMipiDsiVsync = 40;
inline constexpr int kScreenMipiDsiVbp = 120;
inline constexpr int kScreenMipiDsiVfp = 80;
inline constexpr int kScreenDataLaneNum = 2;
inline constexpr int kScreenLaneBitRateMbps = 1000;
}  // namespace rm69a10

namespace gt9895 {
inline constexpr uint8_t kI2cAddress = 0x5D;
inline constexpr int32_t kI2cFrequencyHz = 400000;
inline constexpr uint16_t kRawCoordinateWidth = 1060;
inline constexpr uint16_t kRawCoordinateHeight = 2400;
}  // namespace gt9895

namespace camera {
#if defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_SC2336)
inline constexpr CameraType kType = CameraType::kSc2336;
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV2710)
inline constexpr CameraType kType = CameraType::kOv2710;
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV5645)
inline constexpr CameraType kType = CameraType::kOv5645;
#else
inline constexpr CameraType kType = CameraType::kUnknown;
#endif

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_PIXEL_FORMAT_RGB565)
inline constexpr int kBitsPerPixel = 16;
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_PIXEL_FORMAT_RGB888)
inline constexpr int kBitsPerPixel = 24;
#else
inline constexpr int kBitsPerPixel = 0;
#endif

inline constexpr int kBufferCount = 2;
}  // namespace camera

}  // namespace lilygo_device_driver::t_display_p4::device
