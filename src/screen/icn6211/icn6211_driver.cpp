/*
 * @Description: icn6211_driver
 * @Author: LILYGO_L
 * @Date: 2025-09-17 10:24:21
 * @LastEditTime: 2026-01-16 17:46:51
 * @License: GPL 3.0
 */
#include "soc/soc_caps.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "icn6211_driver.h"

#define ICN6211_MDCTL_VALUE_DEFAULT (0x01)

static const icn6211_lcd_init_cmd_t vendor_specific_init_default[] = {
    // {cmd, { data }, data_size, delay_ms}
    {0x7A, (uint8_t[]){0xC1}, 1, 0},
    {0x20, (uint8_t[]){0x80}, 1, 0},
    {0x21, (uint8_t[]){0x90}, 1, 0},
    {0x22, (uint8_t[]){0x12}, 1, 0},
    {0x23, (uint8_t[]){0x24}, 1, 0},
    {0x24, (uint8_t[]){0x02}, 1, 0},
    {0x25, (uint8_t[]){0x24}, 1, 0},
    {0x26, (uint8_t[]){0x00}, 1, 0},
    {0x27, (uint8_t[]){0x14}, 1, 0},
    {0x28, (uint8_t[]){0x02}, 1, 0},
    {0x29, (uint8_t[]){0x14}, 1, 0},
    {0x34, (uint8_t[]){0x80}, 1, 0},
    {0x36, (uint8_t[]){0x24}, 1, 0},
    {0x86, (uint8_t[]){0x29}, 1, 0},
    {0xB5, (uint8_t[]){0xA0}, 1, 0},
    {0x5C, (uint8_t[]){0xFF}, 1, 0},
    {0x2A, (uint8_t[]){0x01}, 1, 0},
    {0x56, (uint8_t[]){0x92}, 1, 0},
    {0x6B, (uint8_t[]){0x73}, 1, 0},
    {0x69, (uint8_t[]){0x18}, 1, 0},
    {0x10, (uint8_t[]){0x40}, 1, 0},
    {0x11, (uint8_t[]){0x88}, 1, 0},
    {0xB6, (uint8_t[]){0x20}, 1, 0},
    {0x51, (uint8_t[]){0x20}, 1, 0},
    {0x09, (uint8_t[]){0x10}, 1, 0},
};

typedef struct
{
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
    const icn6211_lcd_init_cmd_t *init_cmds;
    uint16_t init_cmds_size;
    uint8_t lane_num;
    struct
    {
        unsigned int reset_level : 1;
    } flags;
    // To save the original functions of MIPI DPI panel
    esp_err_t (*del)(esp_lcd_panel_t *panel);
    esp_err_t (*init)(esp_lcd_panel_t *panel);
} icn6211_panel_t;

static const char *TAG = "icn6211";

static esp_err_t panel_icn6211_del(esp_lcd_panel_t *panel);
static esp_err_t panel_icn6211_init(esp_lcd_panel_t *panel);
static esp_err_t panel_icn6211_reset(esp_lcd_panel_t *panel);

esp_err_t esp_lcd_new_panel_icn6211(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                    esp_lcd_panel_handle_t *ret_panel)
{
    // ESP_LOGI(TAG, "version: %d.%d.%d", ESP_LCD_icn6211_VER_MAJOR, ESP_LCD_icn6211_VER_MINOR,
    //          ESP_LCD_icn6211_VER_PATCH);
    ESP_RETURN_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "invalid arguments");
    icn6211_vendor_config_t *vendor_config = (icn6211_vendor_config_t *)panel_dev_config->vendor_config;
    ESP_RETURN_ON_FALSE(vendor_config && vendor_config->mipi_config.dpi_config && vendor_config->mipi_config.dsi_bus, ESP_ERR_INVALID_ARG, TAG,
                        "invalid vendor config");

    esp_err_t ret = ESP_OK;
    icn6211_panel_t *icn6211 = (icn6211_panel_t *)calloc(1, sizeof(icn6211_panel_t));
    ESP_RETURN_ON_FALSE(icn6211, ESP_ERR_NO_MEM, TAG, "no mem for icn6211 panel");

    if (panel_dev_config->reset_gpio_num >= 0)
    {
        gpio_config_t io_conf = {
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
#if SOC_GPIO_SUPPORT_PIN_HYS_FILTER
            .hys_ctrl_mode = GPIO_HYS_SOFT_ENABLE,
#endif
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    icn6211->io = io;
    icn6211->init_cmds = vendor_config->init_cmds;
    icn6211->init_cmds_size = vendor_config->init_cmds_size;
    icn6211->lane_num = vendor_config->mipi_config.lane_num;
    icn6211->reset_gpio_num = panel_dev_config->reset_gpio_num;
    icn6211->flags.reset_level = panel_dev_config->flags.reset_active_high;
    icn6211->madctl_val = ICN6211_MDCTL_VALUE_DEFAULT;

    // Create MIPI DPI panel
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_dpi(vendor_config->mipi_config.dsi_bus, vendor_config->mipi_config.dpi_config, ret_panel), err, TAG,
                      "create MIPI DPI panel failed");
    ESP_LOGD(TAG, "new MIPI DPI panel @%p", *ret_panel);

    // Save the original functions of MIPI DPI panel
    icn6211->del = (*ret_panel)->del;
    icn6211->init = (*ret_panel)->init;
    // Overwrite the functions of MIPI DPI panel
    (*ret_panel)->del = panel_icn6211_del;
    (*ret_panel)->init = panel_icn6211_init;
    (*ret_panel)->reset = panel_icn6211_reset;
    (*ret_panel)->user_data = icn6211;

    ESP_LOGD(TAG, "new icn6211 panel @%p", icn6211);

    return ESP_OK;

err:
    if (icn6211)
    {
        if (panel_dev_config->reset_gpio_num >= 0)
        {
            gpio_reset_pin(static_cast<gpio_num_t>(panel_dev_config->reset_gpio_num));
        }
        free(icn6211);
    }
    return ret;
}

static esp_err_t panel_icn6211_send_init_cmds(icn6211_panel_t *icn6211)
{
    esp_lcd_panel_io_handle_t io = icn6211->io;
    const icn6211_lcd_init_cmd_t *init_cmds = NULL;
    uint16_t init_cmds_size = 0;

    init_cmds = vendor_specific_init_default;
    init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(icn6211_lcd_init_cmd_t);

    for (int i = 0; i < init_cmds_size; i++)
    {
        // Send command
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, init_cmds[i].cmd, init_cmds[i].data, init_cmds[i].data_bytes), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(init_cmds[i].delay_ms));
        // printf("Ciallo\n");
    }

    ESP_LOGD(TAG, "send init commands success");

    return ESP_OK;
}

static esp_err_t panel_icn6211_del(esp_lcd_panel_t *panel)
{
    icn6211_panel_t *icn6211 = (icn6211_panel_t *)panel->user_data;

    if (icn6211->reset_gpio_num >= 0)
    {
        gpio_reset_pin(static_cast<gpio_num_t>(icn6211->reset_gpio_num));
    }
    // Delete MIPI DPI panel
    icn6211->del(panel);
    ESP_LOGD(TAG, "del icn6211 panel @%p", icn6211);
    free(icn6211);

    return ESP_OK;
}

static esp_err_t panel_icn6211_init(esp_lcd_panel_t *panel)
{
    icn6211_panel_t *icn6211 = (icn6211_panel_t *)panel->user_data;

    // ESP_RETURN_ON_ERROR(panel_icn6211_send_init_cmds(icn6211), TAG, "send init commands failed");
    ESP_RETURN_ON_ERROR(icn6211->init(panel), TAG, "init MIPI DPI panel failed");

    return ESP_OK;
}

static esp_err_t panel_icn6211_reset(esp_lcd_panel_t *panel)
{
    icn6211_panel_t *icn6211 = (icn6211_panel_t *)panel->user_data;

    // Perform hardware reset
    if (icn6211->reset_gpio_num >= 0)
    {
        gpio_set_level(static_cast<gpio_num_t>(icn6211->reset_gpio_num), icn6211->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(static_cast<gpio_num_t>(icn6211->reset_gpio_num), !icn6211->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(static_cast<gpio_num_t>(icn6211->reset_gpio_num), icn6211->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    return ESP_OK;
}
