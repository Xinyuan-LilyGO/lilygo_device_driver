<h1 align="center">lilygo_device_driver</h1>

## **English** | [中文](./README_CN.md)

[![Release](https://img.shields.io/github/v/release/Xinyuan-LilyGO/lilygo_device_driver?style=flat-square)](https://github.com/Xinyuan-LilyGO/lilygo_device_driver/releases)
[![License](https://img.shields.io/github/license/Xinyuan-LilyGO/lilygo_device_driver?style=flat-square)](./LICENSE)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.5.3%2B-ff6f00?style=flat-square)](https://github.com/espressif/esp-idf)
[![C++](https://img.shields.io/badge/C%2B%2B-11%2B-00599c?style=flat-square)](https://isocpp.org/)

**lilygo_device_driver** is a C++ device-level driver library for LILYGO boards. It collects board pin definitions, power initialization, storage mounting helpers, and board peripheral drivers behind one device-oriented entry point, so application code can initialize a board and then access its buses and chips through a consistent interface.

The library is designed to work together with [`cpp_bus_driver`](https://github.com/Llgok/cpp_bus_driver), which provides the lower-level bus and chip drivers used by supported LILYGO devices.

## Table of Contents

- [Features](#features)
- [Supported Frameworks](#supported-frameworks)
- [Supported Devices](#supported-devices)
- [Quick Start](#quick-start)
- [Driver Access](#driver-access)
- [Log Configuration](#log-configuration)
- [Development Notes](#development-notes)

## Features

### Device-Oriented Initialization

Select the target LILYGO device in `idf.py menuconfig`, include the unified header, and initialize the selected board. The library chooses the correct device driver through `sdkconfig`:

```cpp
#include "lilygo_device_driver_library.h"
```

### Unified Access to Board Resources

For boards with a device driver class, application code can use the singleton driver to create and initialize board peripherals, then access initialized bus, chip, and status objects:

```cpp
auto& driver = lilygo_device_driver::TDisplayP4Driver::GetInstance();
driver.Init();

auto& es8311 = driver.chip().es8311;
auto& es8311_i2c = driver.bus().es8311_i2c_bus;
bool es8311_ready = driver.status().es8311.init_flag;
```

### Board Configuration Files

Each supported device has its own configuration header under `src/device/`. These headers define the GPIO map, screen parameters, storage pins, chip addresses, and other board-level constants used by the driver.

## Supported Frameworks

| Framework | Status | Notes |
| --- | --- | --- |
| ESP-IDF | Recommended | Since v2.0.0, the minimum supported ESP-IDF version is v5.5.3. |

> [!NOTE]
> The current implementation is aimed at ESP-IDF projects. Available APIs differ by selected device because each board exposes different hardware.

## Supported Devices

The current device selection in `lilygo_device_driver configuration` supports:

| Device | Kconfig option | Main driver/API |
| --- | --- | --- |
| T-Display-P4 | `CONFIG_LILYGO_DEVICE_DRIVER_T_DISPLAY_P4` | `lilygo_device_driver::TDisplayP4Driver` |
| T-Display-P4 Keyboard | `CONFIG_LILYGO_DEVICE_DRIVER_T_DISPLAY_P4` with `CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD` | `lilygo_device_driver::TDisplayP4Driver` |
| T-Display-P4-Air | `CONFIG_LILYGO_DEVICE_DRIVER_T_DISPLAY_P4_AIR` | Board constants |
| T-Glasses-P4 | `CONFIG_LILYGO_DEVICE_DRIVER_T_GLASSES_P4` | Board constants |
| T-Spe | `CONFIG_LILYGO_DEVICE_DRIVER_T_SPE` | Board constants |

## Quick Start

### Use as an ESP-IDF Component

You can place this repository in the `components` directory of your project, or you can add it to other project directory files.

```bash
your_project/
├── components/
│   └── lilygo_device_driver/
├── main/
└── CMakeLists.txt
```

Then include the unified entry header:

```cpp
#include "lilygo_device_driver_library.h"
```

### Configure the Target Device

Run:

```bash
idf.py menuconfig
```

Open `lilygo_device_driver configuration` and select the device driver you need.

Some example projects also provide board, screen, camera, and pixel-format options in their own project `Kconfig.projbuild`.

### Basic Initialization

For T-Display-P4:

```cpp
#include "lilygo_device_driver_library.h"

extern "C" void app_main(void) {
  auto& driver = lilygo_device_driver::TDisplayP4Driver::GetInstance();

  if (!driver.Init()) {
    printf("Device init failed\n");
    return;
  }

  if (driver.status().pcf8563.init_flag) {
    auto& rtc = driver.chip().pcf8563;
    // Use rtc here.
  }
}
```

Initialization can run synchronously or start peripheral initialization tasks asynchronously:

```cpp
driver.Init(lilygo_device_driver::TDisplayP4Driver::InitMode::kSync);
driver.Init(lilygo_device_driver::TDisplayP4Driver::InitMode::kAsync);
```

## Driver Access

### Buses

Use `driver.bus()` to access board bus objects created by the device driver. For T-Display-P4 this includes I2C, SPI, I2S, UART, MIPI, and board-variant-specific buses.

```cpp
auto& driver = lilygo_device_driver::TDisplayP4Driver::GetInstance();
auto& sx1262_spi = driver.bus().sx1262_spi_bus;
auto& screen_mipi = driver.bus().screen_mipi_bus;
```

### Chips

Use `driver.chip()` to access chip driver objects after initialization.

Common T-Display-P4 chip objects include:

```cpp
driver.chip().xl9535;
driver.chip().bq27220;
driver.chip().sgm38121;
driver.chip().pcf8563;
driver.chip().aw86224;
driver.chip().es8311;
driver.chip().l76k;
driver.chip().icm20948;
driver.chip().sx1262;
```

## Log Configuration

`lilygo_device_driver` provides selectable log levels for debug information, normal information, and chip/device errors.

In ESP-IDF projects, run:

```bash
idf.py menuconfig
```

Then open `lilygo_device_driver configuration` and select the log level you need.

## Development Notes

lilygo_device_driver is currently under active development. You are welcome to open Issues for problems or Feature requests.
