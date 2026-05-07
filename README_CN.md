<h1 align="center">lilygo_device_driver</h1>

## [English](./README.md) | **中文**

[![Release](https://img.shields.io/github/v/release/Xinyuan-LilyGO/lilygo_device_driver?style=flat-square)](https://github.com/Xinyuan-LilyGO/lilygo_device_driver/releases)
[![License](https://img.shields.io/github/license/Xinyuan-LilyGO/lilygo_device_driver?style=flat-square)](./LICENSE)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.5.3%2B-ff6f00?style=flat-square)](https://github.com/espressif/esp-idf)
[![C++](https://img.shields.io/badge/C%2B%2B-11%2B-00599c?style=flat-square)](https://isocpp.org/)

**lilygo_device_driver** 是一个面向 LILYGO 开发板的 C++ 设备级驱动库。它把板级引脚定义、电源初始化、存储挂载工具和板载外设驱动整理到统一的设备入口中，让应用代码可以先初始化整块设备，再通过一致的接口访问 bus、chip 和初始化状态。

本库通常与 [`cpp_bus_driver`](https://github.com/Llgok/cpp_bus_driver) 配合使用，底层总线和芯片驱动由 `cpp_bus_driver` 提供，`lilygo_device_driver` 负责把它们组合成具体 LILYGO 设备的板级驱动。

## 目录

- [特性](#特性)
- [支持框架](#支持框架)
- [当前支持的设备](#当前支持的设备)
- [快速开始](#快速开始)
- [驱动访问](#驱动访问)
- [存储工具](#存储工具)
- [日志配置](#日志配置)
- [开发说明](#开发说明)

## 特性

### 面向设备的初始化流程

在 `idf.py menuconfig` 中选择目标 LILYGO 设备后，只需要包含统一入口头文件，库会根据 `sdkconfig` 自动包含对应设备驱动：

```cpp
#include "lilygo_device_driver_library.h"
```

### 统一访问板级资源

对于带有设备驱动类的开发板，可以通过单例对象创建和初始化板载外设，然后访问已经创建好的 bus、chip 和 status 对象：

```cpp
auto& driver = lilygo_device_driver::TDisplayP4Driver::GetInstance();
driver.Init();

auto& es8311 = driver.chip().es8311;
auto& es8311_i2c = driver.bus().es8311_i2c_bus;
bool es8311_ready = driver.status().es8311.init_flag;
```

### 独立的板级配置文件

每个支持的设备都在 `src/device/` 下拥有独立配置头文件，用于维护 GPIO 映射、屏幕参数、存储引脚、芯片地址和其他板级常量。

## 支持框架

| 框架 | 状态 | 说明 |
| --- | --- | --- |
| ESP-IDF | 推荐 | 从v2.0.0起，最小支持的 ESP-IDF 版本为 v5.5.3 |

> [!NOTE]
> 当前实现主要面向 ESP-IDF 工程。不同设备暴露的硬件不同，因此可用 API 会随所选设备变化。

## 当前支持的设备

`lilygo_device_driver configuration` 当前支持选择以下设备：

| 设备 | Kconfig 选项 | 主要驱动/API |
| --- | --- | --- |
| T-Display-P4 | `CONFIG_LILYGO_DEVICE_DRIVER_T_DISPLAY_P4` | `lilygo_device_driver::TDisplayP4Driver` |
| T-Display-P4 Keyboard | `CONFIG_LILYGO_DEVICE_DRIVER_T_DISPLAY_P4` 搭配 `CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD` | `lilygo_device_driver::TDisplayP4Driver` |
| T-Display-P4-Air | `CONFIG_LILYGO_DEVICE_DRIVER_T_DISPLAY_P4_AIR` | 板级常量 |
| T-Glasses-P4 | `CONFIG_LILYGO_DEVICE_DRIVER_T_GLASSES_P4` | 板级常量 |
| T-Spe | `CONFIG_LILYGO_DEVICE_DRIVER_T_SPE` | 板级常量 |

## 快速开始

### 作为 ESP-IDF component 使用

可以把本仓库放入工程的 `components` 目录，你也可以添加到其他工程目录文件下。

```bash
your_project/
├── components/
│   └── lilygo_device_driver/
├── main/
└── CMakeLists.txt
```

然后在代码中包含统一入口：

```cpp
#include "lilygo_device_driver_library.h"
```

### 配置目标设备

执行：

```bash
idf.py menuconfig
```

进入 `lilygo_device_driver configuration`，选择需要的设备驱动。

部分示例工程还会在自己的 `Kconfig.projbuild` 中提供开发板类型、屏幕类型、摄像头类型和像素格式等选项。

### 基本初始化

T-Display-P4 可以这样使用：

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
    // 在这里使用 rtc
  }
}
```

初始化支持同步模式，也支持异步创建外设初始化任务：

```cpp
driver.Init(lilygo_device_driver::TDisplayP4Driver::InitMode::kSync);
driver.Init(lilygo_device_driver::TDisplayP4Driver::InitMode::kAsync);
```

## 驱动访问

### 总线对象

通过 `driver.bus()` 可以访问设备驱动创建的板级总线对象。以 T-Display-P4 为例，包含 I2C、SPI、I2S、UART、MIPI 以及部分板型专用 bus。

```cpp
auto& driver = lilygo_device_driver::TDisplayP4Driver::GetInstance();
auto& sx1262_spi = driver.bus().sx1262_spi_bus;
auto& screen_mipi = driver.bus().screen_mipi_bus;
```

### 芯片对象

初始化完成后，通过 `driver.chip()` 访问芯片驱动对象。

T-Display-P4 常见 chip 对象包括：

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

## 日志配置

`lilygo_device_driver` 提供可配置日志等级，用于控制调试信息、普通信息以及芯片/设备错误输出。

在 ESP-IDF 工程中可以通过：

```bash
idf.py menuconfig
```

进入 `lilygo_device_driver configuration`，选择需要的日志等级。

## 开发说明

lilygo_device_driver 目前仍处于活跃开发阶段，欢迎向我们提交 Issue 反馈问题或提交 Feature 请求。
