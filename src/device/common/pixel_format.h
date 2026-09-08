#pragma once

namespace lilygo_device_driver {

/**
 * @brief 根据RGB像素位深获取格式名称
 * @param bits_per_pixel 每像素位数，支持16和24
 * @return 返回rgb565或rgb888，其他位深返回unknown
 */
constexpr const char* GetRgbPixelFormatName(int bits_per_pixel) {
  switch (bits_per_pixel) {
    case 16:
      return "rgb565";
    case 24:
      return "rgb888";
    default:
      return "unknown";
  }
}

}  // namespace lilygo_device_driver
