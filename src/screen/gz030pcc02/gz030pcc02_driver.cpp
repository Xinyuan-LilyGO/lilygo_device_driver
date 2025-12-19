/*
 * @Description: gz030pcc02_driver
 * @Author: LILYGO_L
 * @Date: 2025-09-17 10:24:21
 * @LastEditTime: 2025-12-19 17:14:21
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
#include "gz030pcc02_driver.h"

#define GZ030PCC02_MDCTL_VALUE_DEFAULT (0x01)

typedef struct
{
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
    const gz030pcc02_lcd_init_cmd_t *init_cmds;
    uint16_t init_cmds_size;
    uint8_t lane_num;
    struct
    {
        unsigned int reset_level : 1;
    } flags;
    // To save the original functions of MIPI DPI panel
    esp_err_t (*del)(esp_lcd_panel_t *panel);
    esp_err_t (*init)(esp_lcd_panel_t *panel);
} gz030pcc02_panel_t;

static const char *TAG = "gz030pcc02";

static esp_err_t panel_gz030pcc02_del(esp_lcd_panel_t *panel);
static esp_err_t panel_gz030pcc02_init(esp_lcd_panel_t *panel);
static esp_err_t panel_gz030pcc02_reset(esp_lcd_panel_t *panel);

esp_err_t esp_lcd_new_panel_gz030pcc02(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                       esp_lcd_panel_handle_t *ret_panel)
{
    // ESP_LOGI(TAG, "version: %d.%d.%d", ESP_LCD_gz030pcc02_VER_MAJOR, ESP_LCD_gz030pcc02_VER_MINOR,
    //          ESP_LCD_gz030pcc02_VER_PATCH);
    ESP_RETURN_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "invalid arguments");
    gz030pcc02_vendor_config_t *vendor_config = (gz030pcc02_vendor_config_t *)panel_dev_config->vendor_config;
    ESP_RETURN_ON_FALSE(vendor_config && vendor_config->mipi_config.dpi_config && vendor_config->mipi_config.dsi_bus, ESP_ERR_INVALID_ARG, TAG,
                        "invalid vendor config");

    esp_err_t ret = ESP_OK;
    gz030pcc02_panel_t *gz030pcc02 = (gz030pcc02_panel_t *)calloc(1, sizeof(gz030pcc02_panel_t));
    ESP_RETURN_ON_FALSE(gz030pcc02, ESP_ERR_NO_MEM, TAG, "no mem for gz030pcc02 panel");

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

    gz030pcc02->io = io;
    gz030pcc02->init_cmds = vendor_config->init_cmds;
    gz030pcc02->init_cmds_size = vendor_config->init_cmds_size;
    gz030pcc02->lane_num = vendor_config->mipi_config.lane_num;
    gz030pcc02->reset_gpio_num = panel_dev_config->reset_gpio_num;
    gz030pcc02->flags.reset_level = panel_dev_config->flags.reset_active_high;
    gz030pcc02->madctl_val = GZ030PCC02_MDCTL_VALUE_DEFAULT;

    // Create MIPI DPI panel
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_dpi(vendor_config->mipi_config.dsi_bus, vendor_config->mipi_config.dpi_config, ret_panel), err, TAG,
                      "create MIPI DPI panel failed");
    ESP_LOGD(TAG, "new MIPI DPI panel @%p", *ret_panel);

    // Save the original functions of MIPI DPI panel
    gz030pcc02->del = (*ret_panel)->del;
    gz030pcc02->init = (*ret_panel)->init;
    // Overwrite the functions of MIPI DPI panel
    (*ret_panel)->del = panel_gz030pcc02_del;
    (*ret_panel)->init = panel_gz030pcc02_init;
    (*ret_panel)->reset = panel_gz030pcc02_reset;
    (*ret_panel)->user_data = gz030pcc02;

    ESP_LOGD(TAG, "new gz030pcc02 panel @%p", gz030pcc02);

    return ESP_OK;

err:
    if (gz030pcc02)
    {
        if (panel_dev_config->reset_gpio_num >= 0)
        {
            gpio_reset_pin(static_cast<gpio_num_t>(panel_dev_config->reset_gpio_num));
        }
        free(gz030pcc02);
    }
    return ret;
}

static esp_err_t panel_gz030pcc02_del(esp_lcd_panel_t *panel)
{
    gz030pcc02_panel_t *gz030pcc02 = (gz030pcc02_panel_t *)panel->user_data;

    if (gz030pcc02->reset_gpio_num >= 0)
    {
        gpio_reset_pin(static_cast<gpio_num_t>(gz030pcc02->reset_gpio_num));
    }
    // Delete MIPI DPI panel
    gz030pcc02->del(panel);
    ESP_LOGD(TAG, "del gz030pcc02 panel @%p", gz030pcc02);
    free(gz030pcc02);

    return ESP_OK;
}

static esp_err_t panel_gz030pcc02_init(esp_lcd_panel_t *panel)
{
    gz030pcc02_panel_t *gz030pcc02 = (gz030pcc02_panel_t *)panel->user_data;

    ESP_RETURN_ON_ERROR(gz030pcc02->init(panel), TAG, "init MIPI DPI panel failed");

    return ESP_OK;
}

static esp_err_t panel_gz030pcc02_reset(esp_lcd_panel_t *panel)
{
    gz030pcc02_panel_t *gz030pcc02 = (gz030pcc02_panel_t *)panel->user_data;

    // Perform hardware reset
    if (gz030pcc02->reset_gpio_num >= 0)
    {
        gpio_set_level(static_cast<gpio_num_t>(gz030pcc02->reset_gpio_num), gz030pcc02->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(static_cast<gpio_num_t>(gz030pcc02->reset_gpio_num), !gz030pcc02->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(static_cast<gpio_num_t>(gz030pcc02->reset_gpio_num), gz030pcc02->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    return ESP_OK;
}
