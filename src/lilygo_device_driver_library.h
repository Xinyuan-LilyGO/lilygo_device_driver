/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-12-19 10:32:32
 * @LastEditTime: 2026-03-05 14:18:11
 * @License: GPL 3.0
 */

#pragma once

#include "sdkconfig.h"

#if defined CONFIG_DEVICE_T_GLASSES_P4
#include "t_glasses_p4_driver.h"

#elif defined CONFIG_DEVICE_T_DISPLAY_P4_AIR
#include "t_display_p4_air_driver.h"

#elif defined CONFIG_DEVICE_T_SPE
#include "t_spe_driver.h"

#elif defined CONFIG_DEVICE_T_DISPLAY_P4
#include "t_display_p4_driver.h"

#else
#error "no macro definition is set"
#endif
