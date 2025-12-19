/*
 * @Description: esp32p4_driver
 * @Author: LILYGO_L
 * @Date: 2025-12-18 17:59:41
 * @LastEditTime: 2025-12-19 09:24:23
 * @License: GPL 3.0
 */
#pragma once

namespace Lilygo_Device_Driver
{
    bool Mipi_Dsi_Init(uint8_t num_data_lanes, uint32_t lane_bit_rate_mbps, uint32_t dpi_clock_freq_mhz, lcd_color_rgb_pixel_format_t color_rgb_pixel_format, uint8_t num_fbs, uint32_t width, uint32_t height,
                       uint32_t mipi_dsi_hsync, uint32_t mipi_dsi_hbp, uint32_t mipi_dsi_hfp, uint32_t mipi_dsi_vsync, uint32_t mipi_dsi_vbp, uint32_t mipi_dsi_vfp,
                       uint32_t bits_per_pixel, esp_lcd_panel_handle_t *mipi_dpi_panel);

    bool Screen_Init(esp_lcd_panel_handle_t *mipi_dpi_panel);

    bool Camera_Init(esp_lcd_panel_handle_t *mipi_dpi_panel);

    bool Init_Ldo_Channel_Power(uint8_t chan_id, uint32_t voltage_mv);
}