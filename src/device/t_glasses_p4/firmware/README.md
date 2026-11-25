# T-Glasses-P4 BHI260AP 固件

`Bosch_Shuttle3_BHI260_BMM350C_BME68x.fw` 是 Bosch Sensortec 在
`BHI260AP_Generic_Custom_FW_V1.1.17.1` 固件包中提供的原始 RAM 镜像，未经修改。

- 文件大小：128832 字节
- SHA-256：`8770d67678e2e17d89d98b570f8d61bf0c179709dbb60bb14cf40ccc355e0402`

`InitBhi260ap()` 将镜像加载到 BHI260AP 的 RAM 并启动固件，不会启用数据输出。
调用方可通过 `chip().bhi260ap` 配置所需的虚拟传感器，再从 BHI260AP 的 FIFO
读取加速度、角速度和 BMM350 磁场数据。固件启动成功不代表 BMM350 已开始输出数据。

此镜像独立于公开的 BHY2 SensorAPI 源码。对外发布前，请确认 Bosch 的再分发条款。
