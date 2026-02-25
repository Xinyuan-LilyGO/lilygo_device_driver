/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:51:14
 * @LastEditTime: 2026-02-24 17:34:07
 * @License: GPL 3.0
 */
#include "t_display_p4_air_driver.h"

namespace Lilygo_Device_Driver
{
    bool Spiffs_Init(const char *base_path, esp_vfs_spiffs_conf_t &spiffs_conf)
    {
        esp_vfs_spiffs_conf_t conf =
            {
                .base_path = base_path,
                .partition_label = NULL,
                .max_files = 5,
                .format_if_mount_failed = false,
            };

        // Use settings defined above to initialize and mount SPIFFS filesystem.
        // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
        esp_err_t assert = esp_vfs_spiffs_register(&conf);

        if (assert != ESP_OK)
        {
            if (assert == ESP_FAIL)
            {
                assert_log(Log_Level::CHIP, __FILE__, __LINE__, "fail to mount or format filesystem (error code: %#X)\n", assert);
            }
            else if (assert == ESP_ERR_NOT_FOUND)
            {
                assert_log(Log_Level::CHIP, __FILE__, __LINE__, "fail to find spiffs partition (error code: %#X)\n", assert);
            }
            else
            {
                assert_log(Log_Level::CHIP, __FILE__, __LINE__, "fail to initialize spiffs (error code: %#X)\n", assert);
            }
            return false;
        }

        size_t total = 0, used = 0;
        assert = esp_spiffs_info(conf.partition_label, &total, &used);
        if (assert != ESP_OK)
        {
            assert_log(Log_Level::CHIP, __FILE__, __LINE__, "fail to get spiffs partition information (error code: %#X). formatting...\n", assert);
            esp_spiffs_format(conf.partition_label);
            return false;
        }

        assert_log(Log_Level::CHIP, __FILE__, __LINE__, "partition size: total: %d bytes, used: %d bytes\n", total, used);

        // Check consistency of reported partition size info.
        if (used > total)
        {
            assert_log(Log_Level::CHIP, __FILE__, __LINE__, "number of used bytes cannot be larger than total performing esp_spiffs_check\n");
            assert = esp_spiffs_check(conf.partition_label);
            // Could be also used to mend broken files, to clean unreferenced pages, etc.
            // More info at https://github.com/pellepl/spiffs/wiki/FAQ#powerlosses-contd-when-should-i-run-spiffs_check
            if (assert != ESP_OK)
            {
                assert_log(Log_Level::CHIP, __FILE__, __LINE__, "esp_spiffs_check fail (error code: %#X)\n", assert);
                return false;
            }
            else
            {
                assert_log(Log_Level::CHIP, __FILE__, __LINE__, "esp_spiffs_check successful\n");
            }
        }

        spiffs_conf = conf;

        return true;
    }

}