/*
 * @Description: esp32p4_driver
 * @Author: LILYGO_L
 * @Date: 2025-12-18 17:59:32
 * @LastEditTime: 2026-01-22 11:46:36
 * @License: GPL 3.0
 */
#include "esp_ldo_regulator.h"
#include "esp32p4_driver.h"

namespace Lilygo_Device_Driver
{
    bool Init_Ldo_Channel_Power(uint8_t chan_id, uint32_t voltage_mv)
    {
        esp_ldo_channel_handle_t ldo_channel_handle = NULL;
        esp_ldo_channel_config_t ldo_channel_config =
            {
                .chan_id = static_cast<int>(chan_id),
                .voltage_mv = static_cast<int>(voltage_mv),
                .flags =
                    {
                        .adjustable = 1,
                        .owned_by_hw = 1,
                        .bypass = 1,
                    },
            };
        esp_err_t assert = esp_ldo_acquire_channel(&ldo_channel_config, &ldo_channel_handle);
        if (assert != ESP_OK)
        {
            assert_log(Log_Level::CHIP, __FILE__, __LINE__, "esp_ldo_acquire_channel %d fail (error code: %#X)\n", chan_id, assert);
            return false;
        }

        return true;
    }
}