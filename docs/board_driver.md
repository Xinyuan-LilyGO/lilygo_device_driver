# 板级驱动函数约定

以 [`TDisplayP4Driver` 的声明](../src/device/t_display_p4/driver.h)为准。**先按操作分类，再在组内按硬件和版本放置；不要按芯片把一套接口写在一起。** 函数排列是为了方便查找，不代表上电或下电时的调用顺序。

## 公开芯片名还是功能名

- **固定芯片：默认按芯片型号命名。** 例如主板电源芯片用 `InitAxp517()`。主板和键盘各有一颗 AXP517，二者可以同时工作，并非替代方案，所以键盘端用 `InitKeyboardAxp517()` 区分，不合并为一个自动选择的入口。
- **同一功能兼容多种替代芯片：只公开功能名，芯片名留在 `private`。** 例如用户只调用 `InitScreen()`；驱动自动识别屏幕，再调用私有的 `InitHi8561()` 或 `InitRm69a10()`。不要要求示例先判断芯片型号，也不要把两个芯片初始化函数一并公开供用户试。反初始化、就绪判断和模式设置有同样的兼容需求时，也按这个边界命名，不必为了凑齐一套接口而新增函数。

## 函数名字与排列顺序

公开类型、`GetInstance()` 和只读访问器在前。其后的板级操作按以下顺序找对应位置，不为单个新外设另开一组：

| 顺序 | 类别 | 命名与放置规则 |
| --- | --- | --- |
| 1 | 初始化 | 总入口 `Init()`、`InitMinimal()` 在前；单项用 `Init<对象>()`，如 `InitAxp517()`、`InitKeyboardAxp517()`。同型芯片分属主板和扩展板时，用 `Keyboard` 等限定词区分。 |
| 2 | 反初始化 | 用 `Deinit<同一对象>()`，不改用 `Release`、`Uninit` 或 `DeInit`。例如 `DeinitKeyboardAxp517()` 放在 v2 的 `Deinit` 段，不放在 `InitKeyboardAxp517()` 下一行。返回值不决定分组。 |
| 3 | 判断是否就绪 | 用 `Is<对象>Ready() const`，如 `IsAxp517Ready()`。这里查询的是驱动是否已就绪；电池是否被选中等硬件状态不叫 `Ready`。 |
| 4 | 电源、休眠与工作模式设置 | 控制供电通断用 `Set<对象>PowerEnabled(bool enabled)`，如 `SetTypeAUsbPowerEnabled()`；休眠用 `Set<对象>Sleep(...)`，多种工作模式用 `Set<对象>OperatingMode(...)`。不要用 `Deinit` 表示断电，也不要给普通模式切换加 `Power`。 |
| 5 | 整板开关机流程的准备 | 用 `Prepare<范围>For<动作>()`。现有 `PrepareDriversForPowerOff()` 与 `PrepareMinimalDriversForPowerOff()` 分别准备完整和最小驱动集合。`Prepare` 表示为后续动作做准备，不等于已经关机；开机初始化仍走 `Init`。 |
| 6 | 其他专用操作 | 名字写明真实动作，如 `EnterEsp32c5DownloadMode()`、`SetKeyboardExpansionLed()`、`SetCc1101RfSwitch()`、`IsExternalBatterySelected()`。同为 `Set` 或 `Is` 不代表要放在电源组或就绪组。 |

上述是查找和新增接口的顺序，不是把整个类按前缀机械排序。现有头文件在电源设置与关机准备附近按版本穿插了芯片专用模式、协处理器供电接口。新增同类函数时紧邻已有同类项，保留版本边界，不为整理一个接口搬动无关代码。

## 声明、实现与成员放哪里

- **声明**放在 `driver.h` 对应的 `public` 或 `private` 分组。仅 v2 使用的接口放在**该操作分组内**已有的 `CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2` 条件段；不要把所有 v2 操作堆进 `Init` 段的版本宏。
- **私有函数沿用公共区的布局。** 在 `private` 自己的范围内，依次归入 `Init...`、`Deinit...`、`Is...Ready`、设置与其他操作组。同组声明一行一个、紧排，类别之间空一行。例如私有的 `InitHi8561()` 与 `InitRm69a10()` 都归初始化组，各自的就绪判断归判断组；不要因为实现不对外暴露，就把同一颗芯片的所有函数堆在一起。
- **实现**按硬件归属选择[共用实现](../src/device/t_display_p4/driver.cpp)、[v1 实现](../src/device/t_display_p4/v1/driver.cpp)或 [v2 实现](../src/device/t_display_p4/v2/driver.cpp)。进入文件后继续找对应操作组。例如 v2 键盘 PMIC 的初始化紧邻主板 PMIC 初始化，反初始化则放在 `DeinitEs8389()` 等反初始化定义所在段。不要图方便追加到文件末尾。
- **内部对象管理**用 `Create...Drivers()` / `Destroy...Drivers()`，与硬件 `Init` / `Deinit` 区分。总线句柄放 `Bus`，芯片驱动实例放 `Chip`，已有模式下需要保存的就绪标志放 `ChipStatus`；类成员按已有布局集中在私有区，不夹在公共函数声明之间。
- **硬件常量**放在相应版本的 `config.h` 或 `keyboard_expansion_config.h`。主板层定义连接器 GPIO，扩展板层按功能引用，如 `kPort3Sda = base_gpio::ext::k1x4P2Io46`；不要在初始化函数里重写 GPIO 编号。引脚命名见[命名约定](naming.md)。

## 空行与折行

- **声明列表用空行标记类别边界。** 同类简单接口一行一个、纵向紧排，不插空行，也不逐个写函数注释。换芯片不换类别：`InitAxp517()` 与 `InitKeyboardAxp517()` 紧挨着写。出现一个空白行，表示下一行开始是另一类函数，例如从 `Init` 转到 `Deinit`。长声明的参数折行不是空白行，不表示换组。
- `#if`、`#else`、`#endif` 独占一行，分支内的同类声明仍紧排，不为每个函数重复包一层宏。只有名称不能表达的约束、副作用或调用顺序才需要[函数注释](comments.md#函数注释)，注释紧贴对应声明，不作为分组标记。
- `.cpp` 中相邻函数定义之间空一行。函数体内按实际步骤分段，例如前置检查、资源准备、硬件配置、结果处理之间可空一行；不要每条语句都空一行。接口注释紧贴声明，中间不留空行。
- 长参数列表或调用按库的 [`.clang-format`](../.clang-format) 折行：续行缩进，不用空格把参数手工对齐到左括号。保留文件现有的换行符，不因局部修改转换整份文件的 LF/CRLF。

下面只摘出三个操作组，展示空行和命名；实际插入时仍须保留所在组的版本宏：

```cpp
bool InitAxp517();
bool InitKeyboardAxp517();

void DeinitKeyboardAxp517();
bool DeinitEs8389();

bool IsAxp517Ready() const;
```
