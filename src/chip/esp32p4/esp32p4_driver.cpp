/*
 * @Description: esp32p4_driver
 * @Author: LILYGO_L
 * @Date: 2025-12-18 17:59:32
 * @LastEditTime: 2025-12-19 17:37:32
 * @License: GPL 3.0
 */
#include "esp_ldo_regulator.h"
#include "esp32p4_driver.h"
#include "t_glasses_p4_driver.h"

namespace Lilygo_Device_Driver
{
    bool Mipi_Dsi_Init(uint8_t num_data_lanes, uint32_t lane_bit_rate_mbps, uint32_t dpi_clock_freq_mhz, lcd_color_rgb_pixel_format_t color_rgb_pixel_format, uint8_t num_fbs, uint32_t width, uint32_t height,
                       uint32_t mipi_dsi_hsync, uint32_t mipi_dsi_hbp, uint32_t mipi_dsi_hfp, uint32_t mipi_dsi_vsync, uint32_t mipi_dsi_vbp, uint32_t mipi_dsi_vfp,
                       uint32_t bits_per_pixel, esp_lcd_panel_handle_t *mipi_dpi_panel)
    {
        esp_lcd_dsi_bus_handle_t mipi_dsi_bus;
        esp_lcd_panel_io_handle_t mipi_dbi_io;

        // create MIPI DSI bus first, it will initialize the DSI PHY as well
        esp_lcd_dsi_bus_config_t bus_config =
            {
                .bus_id = 0,
                .num_data_lanes = num_data_lanes,
                .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
                .lane_bit_rate_mbps = lane_bit_rate_mbps,
            };

        esp_err_t assert = esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus);
        if (assert != ESP_OK)
        {
            assert_log(Log_Level::CHIP, __FILE__, __LINE__, "esp_lcd_new_dsi_bus fail (error code: %#X)\n", assert);
            return false;
        }

        esp_lcd_dbi_io_config_t dbi_io_config =
            {
                .virtual_channel = 0,
                .lcd_cmd_bits = 8,
                .lcd_param_bits = 8,
            };
        assert = esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_io_config, &mipi_dbi_io);
        if (assert != ESP_OK)
        {
            assert_log(Log_Level::CHIP, __FILE__, __LINE__, "esp_lcd_new_panel_io_dbi fail (error code: %#X)\n", assert);
            return false;
        }

        esp_lcd_dpi_panel_config_t dpi_config =
            {
                .virtual_channel = 0,
                .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
                .dpi_clock_freq_mhz = dpi_clock_freq_mhz,
                .pixel_format = color_rgb_pixel_format,
                .in_color_format = LCD_COLOR_FMT_RGB565,
                .out_color_format = LCD_COLOR_FMT_RGB565,
                .num_fbs = num_fbs,
                .video_timing =
                    {
                        .h_size = width,
                        .v_size = height,
                        .hsync_pulse_width = mipi_dsi_hsync,
                        .hsync_back_porch = mipi_dsi_hbp,
                        .hsync_front_porch = mipi_dsi_hfp,
                        .vsync_pulse_width = mipi_dsi_vsync,
                        .vsync_back_porch = mipi_dsi_vbp,
                        .vsync_front_porch = mipi_dsi_vfp,
                    },
                .flags =
                    {
                        .use_dma2d = true, // use DMA2D to copy draw buffer into frame buffer
                        .disable_lp = true,
                    }};

#if defined CONFIG_SCREEN_TYPE_GZ030PCC02
        gz030pcc02_vendor_config_t vendor_config =
            {
                .init_cmds = NULL,
                .init_cmds_size = 0,
                .mipi_config =
                    {
                        .dsi_bus = mipi_dsi_bus,
                        .dpi_config = &dpi_config,
                        .lane_num = 2,
                    },
            };
        esp_lcd_panel_dev_config_t dev_config =
            {
                .reset_gpio_num = -1,
                .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
                .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
                .bits_per_pixel = bits_per_pixel,
                .flags =
                    {
                        .reset_active_high = 1,
                    },
                .vendor_config = &vendor_config,
            };
        assert = esp_lcd_new_panel_gz030pcc02(mipi_dbi_io, &dev_config, mipi_dpi_panel);
        if (assert != ESP_OK)
        {
            assert_log(Log_Level::CHIP, __FILE__, __LINE__, "esp_lcd_new_panel_gz030pcc02 fail (error code: %#X)\n", assert);
            return false;
        }
#else
#error "unknown macro definition, please select the correct macro definition."
#endif

        return true;
    }

    bool Screen_Init(esp_lcd_panel_handle_t *mipi_dpi_panel)
    {
        if (Mipi_Dsi_Init(SCREEN_DATA_LANE_NUM, SCREEN_LANE_BIT_RATE_MBPS, SCREEN_MIPI_DSI_DPI_CLK_MHZ, SCREEN_COLOR_RGB_PIXEL_FORMAT,
                          0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_MIPI_DSI_HSYNC, SCREEN_MIPI_DSI_HBP,
                          SCREEN_MIPI_DSI_HFP, SCREEN_MIPI_DSI_VSYNC, SCREEN_MIPI_DSI_VBP, SCREEN_MIPI_DSI_VFP,
                          SCREEN_BITS_PER_PIXEL, mipi_dpi_panel) == false)
        {
            assert_log(Log_Level::CHIP, __FILE__, __LINE__, "Mipi_Dsi_Init fail\n");
            return false;
        }

        return true;
    }

    bool Camera_Init(esp_lcd_panel_handle_t *mipi_dpi_panel)
    {
        if (Mipi_Dsi_Init(CAMERA_DATA_LANE_NUM, CAMERA_LANE_BIT_RATE_MBPS, CAMERA_MIPI_DSI_DPI_CLK_MHZ, CAMERA_COLOR_RGB_PIXEL_FORMAT,
                          CONFIG_EXAMPLE_CAM_BUF_COUNT, CAMERA_WIDTH, CAMERA_HEIGHT, 0, 0, 0, 0, 0, 0, CAMERA_BITS_PER_PIXEL, mipi_dpi_panel) == false)
        {
            assert_log(Log_Level::CHIP, __FILE__, __LINE__, "Mipi_Dsi_Init fail\n");
            return false;
        }

        return true;
    }

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

    bool Sdmmc_Init(const char *base_path, int max_freq_khz)
    {
        esp_vfs_fat_sdmmc_mount_config_t mount_config =
            {
                .format_if_mount_failed = false,
                .max_files = 5,
                .allocation_unit_size = 16 * 1024,
                .disk_status_check_enable = false,
                .use_one_fat = false,
            };

        sdmmc_host_t host = SDMMC_HOST_DEFAULT();
        host.slot = SDMMC_HOST_SLOT_0;

        host.max_freq_khz = max_freq_khz;

        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
        slot_config.width = 4;
        slot_config.clk = static_cast<gpio_num_t>(SD_SDIO_CLK);
        slot_config.cmd = static_cast<gpio_num_t>(SD_SDIO_CMD);
        slot_config.d0 = static_cast<gpio_num_t>(SD_SDIO_D0);
        slot_config.d1 = static_cast<gpio_num_t>(SD_SDIO_D1);
        slot_config.d2 = static_cast<gpio_num_t>(SD_SDIO_D2);
        slot_config.d3 = static_cast<gpio_num_t>(SD_SDIO_D3);

        slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

        sdmmc_card_t *card;

        esp_err_t assert = esp_vfs_fat_sdmmc_mount(base_path, &host, &slot_config, &mount_config, &card);
        if (assert != ESP_OK)
        {
            assert_log(Log_Level::CHIP, __FILE__, __LINE__, "esp_vfs_fat_sdmmc_mount fail (error code: %#X)\n", assert);
            return false;
        }

        sdmmc_card_print_info(stdout, card);

        return true;
    }

    bool Sdspi_Init(const char *base_path, spi_host_device_t host_id, int max_freq_khz)
    {
        esp_vfs_fat_sdmmc_mount_config_t mount_config =
            {
                .format_if_mount_failed = false,
                .max_files = 5,
                .allocation_unit_size = 16 * 1024,
                .disk_status_check_enable = false,
                .use_one_fat = false,
            };

        sdmmc_host_t host = SDSPI_HOST_DEFAULT();
        host.slot = host_id;

        host.max_freq_khz = max_freq_khz;

        spi_bus_config_t bus_cfg = {
            .mosi_io_num = SD_MOSI,
            .miso_io_num = SD_MISO,
            .sclk_io_num = SD_SCLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .data4_io_num = -1,
            .data5_io_num = -1,
            .data6_io_num = -1,
            .data7_io_num = -1,
            .data_io_default_level = 0,
            .max_transfer_sz = 0,
            .flags = SPICOMMON_BUSFLAG_MASTER,
            .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
            .intr_flags = static_cast<uint32_t>(NULL),
        };

        esp_err_t assert = spi_bus_initialize(host_id, &bus_cfg, SDSPI_DEFAULT_DMA);
        if (assert != ESP_OK)
        {
            assert_log(Log_Level::CHIP, __FILE__, __LINE__, "spi_bus_initialize fail (error code: %#X)\n", assert);
            return false;
        }

        sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
        slot_config.host_id = host_id;
        slot_config.gpio_cs = static_cast<gpio_num_t>(SD_CS);

        sdmmc_card_t *card;

        assert = esp_vfs_fat_sdspi_mount(base_path, &host, &slot_config, &mount_config, &card);
        if (assert != ESP_OK)
        {
            assert_log(Log_Level::CHIP, __FILE__, __LINE__, "esp_vfs_fat_sdspi_mount fail (error code: %#X)\n", assert);
            return false;
        }

        sdmmc_card_print_info(stdout, card);

        return true;
    }
}