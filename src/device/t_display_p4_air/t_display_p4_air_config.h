/*
 * @Description: t_display_p4_air_config
 * @Author: LILYGO_L
 * @Date: 2026-01-22 09:15:30
 * @LastEditTime: 2026-02-04 11:09:31
 * @License: GPL 3.0
 */

#pragma once

////////////////////////////////////////////////// gpio config //////////////////////////////////////////////////

// IIC
#define IIC_1_SDA 9
#define IIC_1_SCL 10
#define IIC_2_SDA 54
#define IIC_2_SCL 53

#define XL9555_SDA IIC_1_SDA
#define XL9555_SCL IIC_1_SCL
// XL9555引脚功能
#define XL9555_TOUCH_RST Cpp_Bus_Driver::Xl95x5::Pin::IO13
#define XL9555_SCREEN_RST Cpp_Bus_Driver::Xl95x5::Pin::IO14
#define XL9555_LED_1 Cpp_Bus_Driver::Xl95x5::Pin::IO15
#define XL9555_POWER_EN Cpp_Bus_Driver::Xl95x5::Pin::IO16

// HI8561
#define HI8561_SCREEN_BL 50
#define HI8561_TOUCH_SDA IIC_1_SDA
#define HI8561_TOUCH_SCL IIC_1_SCL

// AXP517
#define AXP517_SDA IIC_2_SDA
#define AXP517_SCL IIC_2_SCL

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

////////////////////////////////////////////////// other define config //////////////////////////////////////////////////
