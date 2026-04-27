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
bool InitLdoPower(uint8_t chan_id, uint32_t voltage_mv);
}
