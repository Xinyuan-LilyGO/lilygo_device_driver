# 命名约定

按硬件语义命名板级 GPIO 常量，不根据网络名或芯片封装脚号猜测名称。

- 表示电源轨或外设供电开关控制脚的常量，名称必须以 `PowerEn` 结尾，例如 `kTypeAUsbPowerEn`、`kSdPowerEn`。不要使用 `kTypeAUsbVbusEn`、`kEnableTypeAUsb` 或 `kTypeAUsbEn` 代替。
- `PowerEn` 只用于表示供电使能的硬件引脚常量；软件状态、使能操作函数，以及非供电用途的芯片 `EN`、复位或协议控制脚按各自语义命名。
- 网络名可能与 MCU GPIO 编号不同。定义 GPIO 常量前，应以原理图中的器件引脚连接为准，确认网络实际连到的 MCU GPIO 或 IO 扩展器引脚。
