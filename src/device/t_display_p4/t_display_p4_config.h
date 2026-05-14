/*
 * @Description: t_display_p4_config
 * @Author: LILYGO_L
 * @Date: 2024-12-06 10:32:28
 * @LastEditTime: 2026-05-14 21:51:08
 */
#pragma once

#include <cstdint>

#include "cpp_bus_driver_library.h"

namespace lilygo_device_driver::t_display_p4 {

namespace gpio {

inline constexpr int kI2c1Sda = 7;
inline constexpr int kI2c1Scl = 8;
inline constexpr int kI2c2Sda = 20;
inline constexpr int kI2c2Scl = 21;

inline constexpr int kEsp32p4Boot = 35;

inline constexpr int kXl9535Sda = kI2c1Sda;
inline constexpr int kXl9535Scl = kI2c1Scl;
inline constexpr int kXl9535Int = 5;
inline constexpr auto kXl9535PowerEn3v3 =
    cpp_bus_driver::Xl95x5::Pin::kIo0;
inline constexpr auto kXl9535Sky13453Vctl =
    cpp_bus_driver::Xl95x5::Pin::kIo1;
inline constexpr auto kXl9535ScreenRst =
    cpp_bus_driver::Xl95x5::Pin::kIo2;
inline constexpr auto kXl9535TouchRst =
    cpp_bus_driver::Xl95x5::Pin::kIo3;
inline constexpr auto kXl9535TouchInt =
    cpp_bus_driver::Xl95x5::Pin::kIo4;
inline constexpr auto kXl9535EthernetRst =
    cpp_bus_driver::Xl95x5::Pin::kIo5;
inline constexpr auto kXl9535PowerEn5v0 =
    cpp_bus_driver::Xl95x5::Pin::kIo6;
inline constexpr auto kXl9535Icm20948Int =
    cpp_bus_driver::Xl95x5::Pin::kIo7;
inline constexpr auto kXl9535Esp32p4VccaPowerEn =
    cpp_bus_driver::Xl95x5::Pin::kIo10;
inline constexpr auto kXl9535GpsWakeUp =
    cpp_bus_driver::Xl95x5::Pin::kIo11;
inline constexpr auto kXl9535RtcInt =
    cpp_bus_driver::Xl95x5::Pin::kIo12;
inline constexpr auto kXl9535Esp32c6WakeUp =
    cpp_bus_driver::Xl95x5::Pin::kIo13;
inline constexpr auto kXl9535Esp32c6En =
    cpp_bus_driver::Xl95x5::Pin::kIo14;
inline constexpr auto kXl9535SdEn =
    cpp_bus_driver::Xl95x5::Pin::kIo15;
inline constexpr auto kXl9535Sx1262Rst =
    cpp_bus_driver::Xl95x5::Pin::kIo16;
inline constexpr auto kXl9535Sx1262Dio1 =
    cpp_bus_driver::Xl95x5::Pin::kIo17;

inline constexpr int kEs8311Sda = kI2c2Sda;
inline constexpr int kEs8311Scl = kI2c2Scl;
inline constexpr int kEs8311AdcData = 11;
inline constexpr int kEs8311DacData = 10;
inline constexpr int kEs8311Bclk = 12;
inline constexpr int kEs8311Mclk = 13;
inline constexpr int kEs8311WsLrck = 9;

inline constexpr int kAw86224Sda = kI2c2Sda;
inline constexpr int kAw86224Scl = kI2c2Scl;

inline constexpr int kSgm38121Sda = kI2c2Sda;
inline constexpr int kSgm38121Scl = kI2c2Scl;

inline constexpr int kPcf8563Sda = kI2c1Sda;
inline constexpr int kPcf8563Scl = kI2c1Scl;

inline constexpr int kBq27220Sda = kI2c1Sda;
inline constexpr int kBq27220Scl = kI2c1Scl;

inline constexpr int kSpi1Sclk = 2;
inline constexpr int kSpi1Mosi = 3;
inline constexpr int kSpi1Miso = 4;

inline constexpr int kSx1262Cs = 24;
inline constexpr int kSx1262Busy = 6;
inline constexpr int kSx1262Sclk = kSpi1Sclk;
inline constexpr int kSx1262Mosi = kSpi1Mosi;
inline constexpr int kSx1262Miso = kSpi1Miso;

inline constexpr int kGpsTx = 22;
inline constexpr int kGpsRx = 23;

inline constexpr int kIcm20948Sda = kI2c2Sda;
inline constexpr int kIcm20948Scl = kI2c2Scl;

inline constexpr int kHi8561ScreenBl = 51;
inline constexpr int kHi8561TouchSda = kI2c1Sda;
inline constexpr int kHi8561TouchScl = kI2c1Scl;

inline constexpr int kGt9895Sda = kI2c1Sda;
inline constexpr int kGt9895Scl = kI2c1Scl;

inline constexpr int kCameraSda = kI2c2Sda;
inline constexpr int kCameraScl = kI2c2Scl;

inline constexpr int kSdio1Clk = 43;
inline constexpr int kSdio1Cmd = 44;
inline constexpr int kSdio1D0 = 39;
inline constexpr int kSdio1D1 = 40;
inline constexpr int kSdio1D2 = 41;
inline constexpr int kSdio1D3 = 42;

inline constexpr int kSdio2Clk = 18;
inline constexpr int kSdio2Cmd = 19;
inline constexpr int kSdio2D0 = 14;
inline constexpr int kSdio2D1 = 15;
inline constexpr int kSdio2D2 = 16;
inline constexpr int kSdio2D3 = 17;

inline constexpr int kSdSdioClk = kSdio1Clk;
inline constexpr int kSdSdioCmd = kSdio1Cmd;
inline constexpr int kSdSdioD0 = kSdio1D0;
inline constexpr int kSdSdioD1 = kSdio1D1;
inline constexpr int kSdSdioD2 = kSdio1D2;
inline constexpr int kSdSdioD3 = kSdio1D3;

inline constexpr int kSdSclk = kSdio1Clk;
inline constexpr int kSdMosi = kSdio1Cmd;
inline constexpr int kSdMiso = kSdio1D0;
inline constexpr int kSdCs = kSdio1D3;

inline constexpr int kEsp32c6SdioClk = kSdio2Clk;
inline constexpr int kEsp32c6SdioCmd = kSdio2Cmd;
inline constexpr int kEsp32c6SdioD0 = kSdio2D0;
inline constexpr int kEsp32c6SdioD1 = kSdio2D1;
inline constexpr int kEsp32c6SdioD2 = kSdio2D2;
inline constexpr int kEsp32c6SdioD3 = kSdio2D3;

inline constexpr int kExt2x8pSpiSclk = kSpi1Sclk;
inline constexpr int kExt2x8pSpiMosi = kSpi1Mosi;
inline constexpr int kExt2x8pSpiMiso = kSpi1Miso;

inline constexpr int kExt2x8pIo26 = 26;
inline constexpr int kExt2x8pIo27 = 27;
inline constexpr int kExt2x8pIo33 = 33;
inline constexpr int kExt2x8pIo32 = 32;
inline constexpr int kExt2x8pIo25 = 25;
inline constexpr int kExt2x8pIo36 = 36;
inline constexpr int kExt2x8pIo53 = 53;
inline constexpr int kExt2x8pIo54 = 54;
inline constexpr int kExt1x4p1Io47 = 47;
inline constexpr int kExt1x4p1Io48 = 48;
inline constexpr int kExt1x4p2Io45 = 45;
inline constexpr int kExt1x4p2Io46 = 46;

namespace ip101 {

inline constexpr int kPhyRst = -1;
inline constexpr int kRmiiRefClk = 50;
inline constexpr int kRmiiClkOut = -1;
inline constexpr int kRmiiMdc = 31;
inline constexpr int kRmiiMdio = 52;
inline constexpr int kRmiiTxEn = 49;
inline constexpr int kRmiiTxd0 = 34;
inline constexpr int kRmiiTxd1 = 35;
inline constexpr int kRmiiCrsDv = 28;
inline constexpr int kRmiiRxd0 = 29;
inline constexpr int kRmiiRxd1 = 30;

}  // namespace ip101

inline constexpr int kEthernetPhyRst = ip101::kPhyRst;
inline constexpr int kEthernetRmiiRefClk = ip101::kRmiiRefClk;
inline constexpr int kEthernetRmiiClkOut = ip101::kRmiiClkOut;
inline constexpr int kEthernetMdc = ip101::kRmiiMdc;
inline constexpr int kEthernetMdio = ip101::kRmiiMdio;
inline constexpr int kEthernetRmiiTxEn = ip101::kRmiiTxEn;
inline constexpr int kEthernetRmiiTxd0 = ip101::kRmiiTxd0;
inline constexpr int kEthernetRmiiTxd1 = ip101::kRmiiTxd1;
inline constexpr int kEthernetRmiiCrsDv = ip101::kRmiiCrsDv;
inline constexpr int kEthernetRmiiRxd0 = ip101::kRmiiRxd0;
inline constexpr int kEthernetRmiiRxd1 = ip101::kRmiiRxd1;

#if defined(CONFIG_BOARD_VERSION_T_DISPLAY_P4_V2_0)
inline constexpr int kBq25896Sda = kI2c1Sda;
inline constexpr int kBq25896Scl = kI2c1Scl;
#endif

}  // namespace gpio

namespace device {

namespace ip101 {

inline constexpr int kPhyAddress = 1;

}  // namespace ip101

inline constexpr int kEthernetPhyAddress = ip101::kPhyAddress;

inline constexpr uint8_t kXl9535I2cAddress = 0x20;

inline constexpr uint8_t kEs8311I2cAddress = 0x18;
inline constexpr int kEs8311MclkMultiple = 256;
inline constexpr int kEs8311SampleRate = 44100;
inline constexpr int kEs8311BitsPerSample = 16;
inline constexpr int kEs8311Channel = 2;

inline constexpr uint8_t kAw86224I2cAddress = 0x58;
inline constexpr uint8_t kSgm38121I2cAddress = 0x28;
inline constexpr uint8_t kPcf8563I2cAddress = 0x51;
inline constexpr uint8_t kBq27220I2cAddress = 0x55;
inline constexpr uint8_t kIcm20948I2cAddress = 0x68;

inline constexpr int kHi8561ScreenWidth = 540;
inline constexpr int kHi8561ScreenHeight = 1168;
inline constexpr int kHi8561ScreenMipiDsiDpiClkMhz = 60;
inline constexpr int kHi8561ScreenMipiDsiHsync = 28;
inline constexpr int kHi8561ScreenMipiDsiHbp = 26;
inline constexpr int kHi8561ScreenMipiDsiHfp = 20;
inline constexpr int kHi8561ScreenMipiDsiVsync = 2;
inline constexpr int kHi8561ScreenMipiDsiVbp = 22;
inline constexpr int kHi8561ScreenMipiDsiVfp = 200;
inline constexpr int kHi8561ScreenDataLaneNum = 2;
inline constexpr int kHi8561ScreenLaneBitRateMbps = 1000;
inline constexpr uint8_t kHi8561TouchI2cAddress = 0x68;

inline constexpr int kRm69a10ScreenWidth = 568;
inline constexpr int kRm69a10ScreenHeight = 1232;
inline constexpr int kRm69a10ScreenMipiDsiDpiClkMhz = 60;
inline constexpr int kRm69a10ScreenMipiDsiHsync = 50;
inline constexpr int kRm69a10ScreenMipiDsiHbp = 150;
inline constexpr int kRm69a10ScreenMipiDsiHfp = 50;
inline constexpr int kRm69a10ScreenMipiDsiVsync = 40;
inline constexpr int kRm69a10ScreenMipiDsiVbp = 120;
inline constexpr int kRm69a10ScreenMipiDsiVfp = 80;
inline constexpr int kRm69a10ScreenDataLaneNum = 2;
inline constexpr int kRm69a10ScreenLaneBitRateMbps = 1000;

inline constexpr uint8_t kGt9895I2cAddress = 0x5D;
inline constexpr int kGt9895MaxXSize = 1060;
inline constexpr int kGt9895MaxYSize = 2400;
inline constexpr float kGt9895XScaleFactor =
    static_cast<float>(kRm69a10ScreenWidth) /
    static_cast<float>(kGt9895MaxXSize);
inline constexpr float kGt9895YScaleFactor =
    static_cast<float>(kRm69a10ScreenHeight) /
    static_cast<float>(kGt9895MaxYSize);

inline constexpr int kCameraBufferCount = 2;
inline constexpr const char* kSdBasePath = "/sdcard";

#if defined(CONFIG_BOARD_VERSION_T_DISPLAY_P4_V2_0)
inline constexpr uint8_t kBq25896I2cAddress = 0x6B;
#endif

}  // namespace device

}  // namespace lilygo_device_driver::t_display_p4
