/*
 * @Description: t_display_p4_air_config
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-04-16 15:32:03
 * @License: GPL 3.0
 */

#pragma once

//// gpio config ////

// BOOT
#define ESP32P4_BOOT 35

// IIC
#define I2C_1_SDA 9
#define I2C_1_SCL 10
#define I2C_2_SDA 54
#define I2C_2_SCL 53

// SPI
#define SPI_1_SCLK 2
#define SPI_1_MOSI 3
#define SPI_1_MISO 4

// SDIO
#define SDIO_1_CLK 43
#define SDIO_1_CMD 44
#define SDIO_1_D0 39
#define SDIO_1_D1 40
#define SDIO_1_D2 41
#define SDIO_1_D3 42

#define SDIO_2_CLK 18
#define SDIO_2_CMD 19
#define SDIO_2_D0 14
#define SDIO_2_D1 15
#define SDIO_2_D2 16
#define SDIO_2_D3 17

#define XL9535_SDA I2C_1_SDA
#define XL9535_SCL I2C_1_SCL
// XL9535引脚功能
#define XL9535_SD_POWER_EN cpp_bus_driver::Xl95x5::Pin::kIo0
#define XL9535_NRF9151_EN cpp_bus_driver::Xl95x5::Pin::kIo1
#define XL9535_BHI260AP_RST cpp_bus_driver::Xl95x5::Pin::kIo2
#define XL9535_LR1121_POWER_EN cpp_bus_driver::Xl95x5::Pin::kIo5
#define XL9535_USBPHY_POWER_EN cpp_bus_driver::Xl95x5::Pin::kIo10
#define XL9535_ESP32P4_ESP32C5_UART_SWITCH cpp_bus_driver::Xl95x5::Pin::kIo11
#define XL9535_ESP32C5_EN cpp_bus_driver::Xl95x5::Pin::kIo12
#define XL9535_TOUCH_RST cpp_bus_driver::Xl95x5::Pin::kIo13
#define XL9535_SCREEN_RST cpp_bus_driver::Xl95x5::Pin::kIo14
#define XL9535_LED_1 cpp_bus_driver::Xl95x5::Pin::kIo15
#define XL9535_3_3_POWER_EN cpp_bus_driver::Xl95x5::Pin::kIo16
#define XL9535_NS4150_EN cpp_bus_driver::Xl95x5::Pin::kIo17

// HI8561
#define HI8561_SCREEN_BL 50
#define HI8561_TOUCH_SDA I2C_2_SDA
#define HI8561_TOUCH_SCL I2C_2_SCL

// AXP517
#define AXP517_SDA I2C_2_SDA
#define AXP517_SCL I2C_2_SCL

// AW86224
#define AW86224_SDA I2C_1_SDA
#define AW86224_SCL I2C_1_SCL

// ES8388
#define ES8388_SDA I2C_1_SDA
#define ES8388_SCL I2C_1_SCL
#define ES8388_ADC_DATA 33
#define ES8388_DAC_DATA 32
#define ES8388_BCLK 31
#define ES8388_MCLK 30
#define ES8388_WS_LRCK 34

// BHI260AP
#define BHI260AP_SDA I2C_2_SDA
#define BHI260AP_SCL I2C_2_SCL

// QMC6310N
#define QMC6310N_SDA I2C_1_SDA
#define QMC6310N_SCL I2C_1_SCL

// SGM38121
#define SGM38121_SDA I2C_2_SDA
#define SGM38121_SCL I2C_2_SCL

// LR1121
#define LR1121_CS 7
#define LR1121_BUSY 6
#define LR1121_INT 5
#define LR1121_RST 8
#define LR1121_SCLK SPI_1_SCLK
#define LR1121_MOSI SPI_1_MOSI
#define LR1121_MISO SPI_1_MISO

// ST25R3916
#define ST25R3916_SDA I2C_1_SDA
#define ST25R3916_SCL I2C_1_SCL
#define ST25R3916_INT 13

// Infrared
#define INFRARED_RX 29
#define INFRARED_TX 28

// SD
// SDMMC
#define SD_SDIO_CLK SDIO_1_CLK
#define SD_SDIO_CMD SDIO_1_CMD
#define SD_SDIO_D0 SDIO_1_D0
#define SD_SDIO_D1 SDIO_1_D1
#define SD_SDIO_D2 SDIO_1_D2
#define SD_SDIO_D3 SDIO_1_D3
// SDSPI
#define SD_SCLK SDIO_1_CLK
#define SD_MOSI SDIO_1_CMD
#define SD_MISO SDIO_1_D0
#define SD_CS SDIO_1_D3

// ESP32C5 SDIO
#define ESP32C5_SDIO_CLK SDIO_2_CLK
#define ESP32C5_SDIO_CMD SDIO_2_CMD
#define ESP32C5_SDIO_D0 SDIO_2_D0
#define ESP32C5_SDIO_D1 SDIO_2_D1
#define ESP32C5_SDIO_D2 SDIO_2_D2
#define ESP32C5_SDIO_D3 SDIO_2_D3

// NRF9151
#define NRF9151_UART_RX 23
#define NRF9151_UART_TX 22
#define NRF9151_UART_RTS 20
#define NRF9151_UART_CTS 21

//// gpio config ////

//// other define config ////

// XL9535
#define XL9535_I2C_ADDRESS 0x20

// HI8561
#define HI8561_SCREEN_WIDTH 540
#define HI8561_SCREEN_HEIGHT 1168
#define HI8561_SCREEN_MIPI_DSI_DPI_CLK_MHZ 60
// #define HI8561_SCREEN_MIPI_DSI_DPI_CLK_MHZ 45
#define HI8561_SCREEN_MIPI_DSI_HSYNC 28
#define HI8561_SCREEN_MIPI_DSI_HBP 26
#define HI8561_SCREEN_MIPI_DSI_HFP 20
#define HI8561_SCREEN_MIPI_DSI_VSYNC 2
#define HI8561_SCREEN_MIPI_DSI_VBP 22
#define HI8561_SCREEN_MIPI_DSI_VFP 200
#define HI8561_SCREEN_DATA_LANE_NUM 2
#define HI8561_SCREEN_LANE_BIT_RATE_MBPS 1000
#define HI8561_TOUCH_I2C_ADDRESS 0x68

// AXP517
#define AXP517_I2C_ADDRESS 0x34

// AW86224
#define AW86224_I2C_ADDRESS 0x58

// ES8388
#define ES8388_I2C_ADDRESS 0x11

// spiffs
#define SPIFFS_BASE_PATH "/spiffs"

// BHI260AP
#define BHI260AP_I2C_ADDRESS 0x29

// QMC6310N
#define QMC6310N_I2C_ADDRESS 0x3C

// SGM38121
#define SGM38121_I2C_ADDRESS 0x28

#define CAMERA_BUFFER_COUNT 2

// ST25R3916
#define ST25R3916_I2C_ADDRESS 0x50

// Infrared
#define INFRARED_RESOLUTION_HZ 1000000
#define INFRARED_NEC_DECODE_MARGIN 200

// SD
#define SD_BASE_PATH "/sdcard"

//// other define config ////
