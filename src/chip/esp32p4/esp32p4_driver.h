/*
 * @Description: esp32p4_driver
 * @Author: LILYGO_L
 * @Date: 2025-12-18 17:59:41
 * @LastEditTime: 2026-02-24 16:28:23
 * @License: GPL 3.0
 */
#pragma once

#include "../../config.h"

namespace Lilygo_Device_Driver
{
    bool Init_Ldo_Channel_Power(uint8_t chan_id, uint32_t voltage_mv);
}