#pragma once

namespace lilygo_device_driver {

// 摄像头型号。
enum class CameraType {
  kUnknown,
  kSc2336,
  kOv2710,
  kOv5645,
};

/**
 * @brief 根据摄像头型号获取名称
 * @param type 摄像头型号
 * @return 返回型号名称，未知型号返回unknown
 */
constexpr const char* GetCameraTypeName(CameraType type) {
  switch (type) {
    case CameraType::kSc2336:
      return "sc2336";
    case CameraType::kOv2710:
      return "ov2710";
    case CameraType::kOv5645:
      return "ov5645";
    default:
      return "unknown";
  }
}

}  // namespace lilygo_device_driver
