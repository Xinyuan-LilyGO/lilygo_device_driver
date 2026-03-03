/*
 * @Description: t_display_p4_air_config
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-03-03 16:25:33
 * @License: GPL 3.0
 */

#pragma once

////////////////////////////////////////////////// gpio config //////////////////////////////////////////////////

// BOOT
#define ESP32P4_BOOT 35

// IIC
#define IIC_1_SDA 9
#define IIC_1_SCL 10
#define IIC_2_SDA 54
#define IIC_2_SCL 53

// SPI
#define SPI_1_SCLK 2
#define SPI_1_MOSI 3
#define SPI_1_MISO 4

#define XL9555_SDA IIC_1_SDA
#define XL9555_SCL IIC_1_SCL
// XL9555引脚功能
#define XL9555_BHI260AP_RST Cpp_Bus_Driver::Xl95x5::Pin::IO2
#define XL9555_LR1121_POWER_EN Cpp_Bus_Driver::Xl95x5::Pin::IO5
#define XL9555_ESP32P4_ESP32C5_UART_SWITCH Cpp_Bus_Driver::Xl95x5::Pin::IO11
#define XL9555_ESP32C5_EN Cpp_Bus_Driver::Xl95x5::Pin::IO12
#define XL9555_TOUCH_RST Cpp_Bus_Driver::Xl95x5::Pin::IO13
#define XL9555_SCREEN_RST Cpp_Bus_Driver::Xl95x5::Pin::IO14
#define XL9555_LED_1 Cpp_Bus_Driver::Xl95x5::Pin::IO15
#define XL9555_3_3_POWER_EN Cpp_Bus_Driver::Xl95x5::Pin::IO16
#define XL9555_NS4150_EN Cpp_Bus_Driver::Xl95x5::Pin::IO17

// HI8561
#define HI8561_SCREEN_BL 50
#define HI8561_TOUCH_SDA IIC_2_SDA
#define HI8561_TOUCH_SCL IIC_2_SCL

// AXP517
#define AXP517_SDA IIC_2_SDA
#define AXP517_SCL IIC_2_SCL

// AW86224
#define AW86224_SDA IIC_1_SDA
#define AW86224_SCL IIC_1_SCL

// ES8388
#define ES8388_SDA IIC_1_SDA
#define ES8388_SCL IIC_1_SCL
#define ES8388_ADC_DATA 33
#define ES8388_DAC_DATA 32
#define ES8388_BCLK 31
#define ES8388_MCLK 30
#define ES8388_WS_LRCK 34

// BHI260AP
#define BHI260AP_SDA IIC_2_SDA
#define BHI260AP_SCL IIC_2_SCL

// QMC6310N
#define QMC6310N_SDA IIC_1_SDA
#define QMC6310N_SCL IIC_1_SCL

// SGM38121
#define SGM38121_SDA IIC_2_SDA
#define SGM38121_SCL IIC_2_SCL

// LR1121
#define LR1121_CS 7
#define LR1121_BUSY 6
#define LR1121_INT 5
#define LR1121_RST 8
#define LR1121_SCLK SPI_1_SCLK
#define LR1121_MOSI SPI_1_MOSI
#define LR1121_MISO SPI_1_MISO

// ST25R3916
#define ST25R3916_SDA IIC_1_SDA
#define ST25R3916_SCL IIC_1_SCL
#define ST25R3916_INT 13

// Infrared
#define INFRARED_RX 29
#define INFRARED_TX 28

////////////////////////////////////////////////// gpio config //////////////////////////////////////////////////

////////////////////////////////////////////////// other define config //////////////////////////////////////////////////

// XL9555
#define XL9555_IIC_ADDRESS 0x20

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
#define HI8561_TOUCH_IIC_ADDRESS 0x68

// AXP517
#define AXP517_IIC_ADDRESS 0x34

// AW86224
#define AW86224_IIC_ADDRESS 0x58

// ES8388
#define ES8388_IIC_ADDRESS 0x11

// spiffs
#define SPIFFS_BASE_PATH "/spiffs"

// BHI260AP
#define BHI260AP_IIC_ADDRESS 0x29

// QMC6310N
#define QMC6310N_IIC_ADDRESS 0x3C

// SGM38121
#define SGM38121_IIC_ADDRESS 0x28

#define CAMERA_BUFFER_COUNT 2

// ST25R3916
#define ST25R3916_IIC_ADDRESS 0x50

// Infrared
#define INFRARED_RESOLUTION_HZ 1000000
#define INFRARED_NEC_DECODE_MARGIN 200

////////////////////////////////////////////////// other define config //////////////////////////////////////////////////
