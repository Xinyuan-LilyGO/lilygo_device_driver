/*
 * @Description: esp32p4_driver
 * @Author: LILYGO_L
 * @Date: 2025-12-18 17:59:41
 * @LastEditTime: 2026-04-23 17:52:20
 * @License: GPL 3.0
 */
#pragma once

#include "../../config.h"

namespace lilygo_device_driver {
/**
 * @brief 获取并配置 ESP32-P4 LDO 电源通道。
 * @param chan_id LDO 通道 ID。
 * @param voltage_mv 输出电压，单位为毫伏。
 * @return LDO 通道获取成功时返回 true，否则返回 false。
 */
bool InitLdoPower(uint8_t chan_id, uint32_t voltage_mv);
}
