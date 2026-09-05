/*
 * @Description: LILYGO 设备驱动库统一入口
 * @Author: LILYGO_L
 * @Date: 2025-12-19 10:32:32
 * @LastEditTime: 2026-06-12 12:00:34
 * @License: GPL 3.0
 */

#pragma once

#include "lilygo_device_driver_config.h"
#include "core/logger.h"

#if defined(CONFIG_LILYGO_DEVICE_DRIVER_T_GLASSES_P4)
#include "t_glasses_p4_driver.h"

#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_T_DISPLAY_P4_AIR)
#include "t_display_p4_air_driver.h"

#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_T_SPE)
#include "t_spe_driver.h"

#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_T_CAN485_C5)
#include "t_can485_c5_driver.h"

#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_T_DISPLAY_P4)
#include "t_display_p4_driver.h"

#else
#error "Missing required macro definition."
#endif
