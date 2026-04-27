/*
 * @Description: t_display_p4_keyboard_config
 * @Author: LILYGO_L
 * @Date: 2024-12-06 10:32:28
 * @LastEditTime: 2026-04-27 09:42:44
 */
#pragma once
#include <string>

#include "t_display_p4_config.h"

////////////////////////////////////////////////// gpio config
/////////////////////////////////////////////////////

#define IIC_3_SDA EXT_1X4P_2_IO_46
#define IIC_3_SCL EXT_1X4P_2_IO_45

// XL9555
#define XL9555_SDA IIC_3_SDA
#define XL9555_SCL IIC_3_SCL
// XL9555引脚功能
#define XL9555_T_MIXRF_EN cpp_bus_driver::Xl95x5::Pin::kIo0
#define XL9555_T_MIXRF_CC1101_RF_SWITCH_0 cpp_bus_driver::Xl95x5::Pin::kIo1
#define XL9555_T_MIXRF_CC1101_RF_SWITCH_1 cpp_bus_driver::Xl95x5::Pin::kIo2
#define XL9555_LED_1 cpp_bus_driver::Xl95x5::Pin::kIo3
#define XL9555_LED_2 cpp_bus_driver::Xl95x5::Pin::kIo4
#define XL9555_LED_3 cpp_bus_driver::Xl95x5::Pin::kIo5
#define XL9555_TCA8418_RST cpp_bus_driver::Xl95x5::Pin::kIo6
#define XL9555_T_MIXRF_LR1121_INT cpp_bus_driver::Xl95x5::Pin::kIo7
#define XL9555_T_MIXRF_LR1121_RST cpp_bus_driver::Xl95x5::Pin::kIo10
#define XL9555_T_MIXRF_LR1121_CS cpp_bus_driver::Xl95x5::Pin::kIo11
#define XL9555_T_MIXRF_LR1121_BUSY cpp_bus_driver::Xl95x5::Pin::kIo12

// SY7200A
#define SY7200A_EN_PWM EXT_1X4P_1_IO_47

// Keyboard
#define KEYBOARD_BL SY7200A_EN_PWM

// TCA8418
#define TCA8418_SDA IIC_3_SDA
#define TCA8418_SCL IIC_3_SCL
#define TCA8418_INT EXT_1X4P_1_IO_48
#define TCA8418_BL KEYBOARD_BL

// T-MixRF
#define T_MIXRF_CC1101_CS EXT_2X8P_IO_36
#define T_MIXRF_CC1101_SCLK EXT_2X8P_SPI_SCLK
#define T_MIXRF_CC1101_MOSI EXT_2X8P_SPI_MOSI
#define T_MIXRF_CC1101_MISO EXT_2X8P_SPI_MISO
#define T_MIXRF_CC1101_GDO0 EXT_2X8P_IO_25
#define T_MIXRF_CC1101_GDO2 EXT_2X8P_IO_33
#define T_MIXRF_CC1101_INT T_MIXRF_CC1101_GDO0
#define T_MIXRF_CC1101_BUSY T_MIXRF_CC1101_GDO2

#define T_MIXRF_NRF24L01_CS EXT_2X8P_IO_54
#define T_MIXRF_NRF24L01_SCLK EXT_2X8P_SPI_SCLK
#define T_MIXRF_NRF24L01_MOSI EXT_2X8P_SPI_MOSI
#define T_MIXRF_NRF24L01_MISO EXT_2X8P_SPI_MISO
#define T_MIXRF_NRF24L01_CE EXT_2X8P_IO_53
#define T_MIXRF_NRF24L01_INT EXT_2X8P_IO_32

#define T_MIXRF_ST25R3916_CS EXT_2X8P_IO_27
#define T_MIXRF_ST25R3916_SCLK EXT_2X8P_SPI_SCLK
#define T_MIXRF_ST25R3916_MOSI EXT_2X8P_SPI_MOSI
#define T_MIXRF_ST25R3916_MISO EXT_2X8P_SPI_MISO
#define T_MIXRF_ST25R3916_INT EXT_2X8P_IO_26

#define T_MIXRF_LR1121_SCLK EXT_2X8P_SPI_SCLK
#define T_MIXRF_LR1121_MOSI EXT_2X8P_SPI_MOSI
#define T_MIXRF_LR1121_MISO EXT_2X8P_SPI_MISO

////////////////////////////////////////////////// gpio config
/////////////////////////////////////////////////////

////////////////////////////////////////////////// other define config
/////////////////////////////////////////////////////

// XL9555
#define XL9555_IIC_ADDRESS 0x20

// TCA8418
#define TCA8418_IIC_ADDRESS 0x34
#define TCA8418_KEYPAD_SCAN_WIDTH 10
#define TCA8418_KEYPAD_SCAN_HEIGHT 7
// TCA8418键盘按键映射
constexpr const std::string Tca8418_Map[] = {"F1", "F2", "F3", "F4", "F5", "F6",
    "F7", "F8", "F9", "F10", "Esc", "Esc", "1", "2", "3", "4", "5", "6", "7",
    "8", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "Caps", "a", "s",
    "d", "f", "g", "h", "j", "k", "l", "Alt", "z", "x", "c", "v", "b", "n", "m",
    "Ctrl", "Up", "Fn", "Win", "Shift", "Tab", "Space", "Space", "Space", "Fn",
    "Left", "Down", "F11", "9", "Del", "Enter", "Record", "Enter", "0",
    "Right"};

////////////////////////////////////////////////// other define config
/////////////////////////////////////////////////////
