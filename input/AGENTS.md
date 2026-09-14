# AGENTS.md - OpenHarmony Input HDI 接口定义（drivers_interface_input）

## 1. 代码地图

本仓库定义 OpenHarmony Input HDI 接口（`@ohos/drivers_interface_input`，版本 4.0），subsystem `hdf`，part `drivers_interface_input`。核心职责是定义输入设备驱动接口和 HID DDK（Host Interface Device Driver Development Kit）接口的 IDL 契约，由 `hdi-gen` 工具自动生成 proxy/stub 代码。**本仓只维护 `.idl` 源文件，不包含任何实现代码**——实现由 `drivers/peripheral/input` 提供，消费方为 input 服务和 HID DDK 消费者。

最重要的架构边界是**本子域包含两个独立子模块**：(1) 主输入接口 `v1_0/`（`mode = "passthrough"`，含设备扫描/事件上报/热插拔回调，`@since 3.2`）和 (2) HID DDK `ddk/v1_0` + `ddk/v1_1`（`mode = "ipc"`，含设备创建/事件发送/设备操作，`@since 4.1`/`@since 5.1`）。`ddk/v1_1` 通过 IDL `extends` 继承 `ddk/v1_0.IHidDdk`，并通过 `proxy_deps`/`stub_deps` 复用 v1_0 的 proxy/stub。

### 非本项目维护的目录

以下内容属于其他团队或生成产物，不属于本项目维护范围，修改时请跳过：

- 任何由 `hdi-gen` 生成的 proxy/stub `.h/.cpp` 文件（构建产物，不在源码树中）
- `drivers/peripheral/input/`：Input HDI 实现端（由驱动团队维护）
- input service / HID DDK 消费方代码（由上层服务团队维护）

### 嵌套指引

本仓目前无嵌套 AGENTS.md / CLAUDE.md / rules / skills 文件。仓库结构为 `v1_0/`（主输入接口）+ `ddk/v1_0/` + `ddk/v1_1/`（HID DDK 两版本）+ `bundle.json`，不建议新建嵌套指引；子模块差异和版本复用关系在本文档「知识路由」中集中说明。

### 关键区域

- `v1_0/`：Input HDI 1.0 主接口（**活跃构建**，`@since 3.2`），含 3 个 IDL 文件 + `BUILD.gn`：
  - `IInputInterfaces.idl`：主接口，18 个方法（`ScanInputDevice` / `OpenInputDevice` / `CloseInputDevice` / `GetInputDevice` / `GetInputDeviceList` / `SetPowerStatus` / `GetPowerStatus` / `GetDeviceType` / `GetChipInfo` / `GetVendorName` / `GetChipName` / `SetGestureMode` / `RunCapacitanceTest` / `RunExtraCommand` / `RegisterReportCallback` / `UnregisterReportCallback` / `RegisterHotPlugCallback` / `UnregisterHotPlugCallback`）
  - `IInputCallback.idl`：回调接口（`[callback] interface`），2 个方法（`EventPkgCallback` 数据上报 + `HotPlugCallback` 热插拔事件上报）
  - `InputTypes.idl`：类型定义（`DevDesc` / `DevIdentify` / `DimensionInfo` / `DevAttr` / `DevAbility` / `DeviceInfo` / `ExtraCmd` / `HotPlugEvent` / `EventPackage`）
  - `BUILD.gn`：`hdi("input")` 模板，`module_name = "input_service"`，**`mode = "passthrough"`**（直通模式，非 IPC），`language = "cpp"`，`install_images = ["system", "updater"]`，**`ohos_lite` 条件分支**（lite 系统下 proxy 为空 group），无 `proxy_deps`/`stub_deps`（独立基线版本），无 `innerapi_tags`
- `ddk/v1_0/`：HID DDK 1.0 接口（**活跃构建**，`@since 4.1`），含 2 个 IDL 文件 + `BUILD.gn`：
  - `IHidDdk.idl`：主接口，3 个方法（`CreateDevice` / `EmitEvent` / `DestroyDevice`）
  - `HidDdkTypes.idl`：类型定义（`Hid_EmitItem` / `Hid_DeviceProp` / `Hid_Device` / `Hid_EventType` / `Hid_SynEvent` / `Hid_KeyCode`（80+ 键值）/ `Hid_AbsAxes` / `Hid_RelAxes` / `Hid_MscEvent` / `Hid_EventProperties` / `Hid_DdkErrCode`）
  - `BUILD.gn`：`hdi("hid_ddk")` 模板，`module_name = "hid_ddk"`，**`mode = "ipc"`**，`language = "cpp"`，无 `proxy_deps`/`stub_deps`（独立基线版本），无 `ohos_lite` 条件分支，无 `innerapi_tags`
- `ddk/v1_1/`：HID DDK 1.1 接口（**活跃构建，最新版本**，`@since 5.1`），含 2 个 IDL 文件 + `BUILD.gn`：
  - `IHidDdk.idl`：扩展主接口，**`interface IHidDdk extends ohos.hdi.input.ddk.v1_0.IHidDdk`**（IDL 继承），新增 14 个方法（`Init` / `Release` / `Open` / `Close` / `Write` / `ReadTimeout` / `SetNonBlock` / `GetRawInfo` / `GetRawName` / `GetPhysicalAddress` / `GetRawUniqueId` / `SendReport` / `GetReport` / `GetReportDescriptor`）
  - `HidDdkTypes.idl`：扩展类型定义（`HidDeviceHandle` / `HidReportType` / `HidRawDevInfo` / `HidDdkErrCode`——**与 v1_0 的 `Hid_DdkErrCode` 不同**，使用 API 级错误码 201/401/27300001+）
  - `BUILD.gn`：`hdi("hid_ddk")` 模板，`module_name = "hid_ddk"`，`mode = "ipc"`，**`proxy_deps = ["../v1_0:libhid_ddk_proxy_1.0"]` + `stub_deps = ["../v1_0:libhid_ddk_stub_1.0"]`**——版本复用机制的核心
- `bundle.json`：组件清单，`adapted_system_type: ["standard", "small"]`，依赖 `c_utils` / `ipc` / `hdf_core` / `hilog`，`features: []`（无特性开关），构建 3 个 `sub_component`（`v1_0:input_idl_target` + `ddk/v1_0:hid_ddk_idl_target` + `ddk/v1_1:hid_ddk_idl_target`），导出 8 个 `inner_kits`（v1_0 2 个 + ddk/v1_0 3 个 + ddk/v1_1 3 个），**无 `sa_profile`**，**无 `innerapi_tags`**

### Where to look

| 任务类型 | 先看哪里 |
|---|---|
| 新增/修改主输入接口方法 | `v1_0/IInputInterfaces.idl` -> 重新生成 proxy/stub -> 同步实现端 `drivers/peripheral/input` |
| 输入事件回调变更 | `v1_0/IInputCallback.idl`（唯一回调定义）-> 评估对事件上报和热插拔功能的影响 |
| 输入设备类型变更 | `v1_0/InputTypes.idl`（`DeviceInfo` / `DevAbility` / `EventPackage` 等）-> 评估序列化兼容性 -> 同步实现端和消费端 |
| HID DDK 设备创建/事件发送 | `ddk/v1_0/IHidDdk.idl`（`CreateDevice` / `EmitEvent` / `DestroyDevice`）-> 评估 v1_1 继承影响 |
| HID DDK 设备操作扩展 | `ddk/v1_1/IHidDdk.idl`（`Open` / `Close` / `Write` / `ReadTimeout` / `SendReport` / `GetReport` 等）-> 确认仅影响 v1_1 消费者 |
| HID DDK 类型变更 | `ddk/v1_0/HidDdkTypes.idl` 或 `ddk/v1_1/HidDdkTypes.idl` -> 评估跨版本影响 |
| HID DDK 错误码变更 | `ddk/v1_0/HidDdkTypes.idl`（`Hid_DdkErrCode`，-1~-6）vs `ddk/v1_1/HidDdkTypes.idl`（`HidDdkErrCode`，201/401/27300001+）-> 注意两个版本错误码体系不同 |
| 版本兼容性评估 | `ddk/v1_1/BUILD.gn`（`proxy_deps`/`stub_deps` 指向 v1_0）+ `ddk/v1_1/IHidDdk.idl`（`extends` 声明）+ `bundle.json`（三目标同时构建）|
| 构建配置 | `bundle.json`（依赖、inner_kits）+ `v1_0/BUILD.gn`（`passthrough` 模式、`ohos_lite`）+ `ddk/v1_0/BUILD.gn` + `ddk/v1_1/BUILD.gn`（`ipc` 模式、`proxy_deps`/`stub_deps`）|
| lite 系统适配 | `v1_0/BUILD.gn` 中 `if (defined(ohos_lite))` 分支（lite 系统下 proxy 为空 group）；ddk 版本无 ohos_lite 条件分支 |

### 架构分层

```
IDL 源文件（本仓维护）
  │
  ├─ v1_0/ (主输入接口，@since 3.2，mode = "passthrough")
  │    ├─ IInputInterfaces.idl (18 方法：设备扫描/开关/信息查询/电源/手势/测试/回调注册)
  │    ├─ IInputCallback.idl  (回调：EventPkgCallback + HotPlugCallback)
  │    └─ InputTypes.idl      (DevDesc/DevIdentify/DimensionInfo/DevAttr/DevAbility/DeviceInfo/ExtraCmd/HotPlugEvent/EventPackage)
  │        module_name = "input_service"
  │        mode = "passthrough" (直通模式，非 IPC)
  │        ohos_lite 条件分支（lite 下 proxy 为空 group）
  │        ↓ hdi-gen 生成
  │    libinput_proxy_1.0 + input_idl_headers
  │
  └─ ddk/ (HID DDK 子模块)
       ├─ v1_0/ (HID DDK 1.0，@since 4.1，mode = "ipc")
       │    ├─ IHidDdk.idl     (3 方法：CreateDevice/EmitEvent/DestroyDevice)
       │    └─ HidDdkTypes.idl (Hid_EmitItem/Hid_Device/Hid_EventType/Hid_KeyCode/Hid_AbsAxes/Hid_RelAxes/Hid_MscEvent/Hid_EventProperties/Hid_DdkErrCode)
       │        module_name = "hid_ddk"
       │        mode = "ipc"
       │        ↓ hdi-gen 生成
       │    libhid_ddk_proxy_1.0 + libhid_ddk_stub_1.0 + hid_ddk_idl_headers
       │
       └─ v1_1/ (HID DDK 1.1，@since 5.1，mode = "ipc"，extends v1_0)
            ├─ IHidDdk.idl     (14 新方法：Init/Release/Open/Close/Write/ReadTimeout/SetNonBlock/GetRawInfo/GetRawName/GetPhysicalAddress/GetRawUniqueId/SendReport/GetReport/GetReportDescriptor)
            └─ HidDdkTypes.idl (HidDeviceHandle/HidReportType/HidRawDevInfo/HidDdkErrCode)
                interface IHidDdk extends ohos.hdi.input.ddk.v1_0.IHidDdk
                proxy_deps = ["../v1_0:libhid_ddk_proxy_1.0"]
                stub_deps  = ["../v1_0:libhid_ddk_stub_1.0"]
                ↓ hdi-gen 生成
            libhid_ddk_proxy_1.1 + libhid_ddk_stub_1.1 + hid_ddk_idl_headers
                 ↓
            实现端：drivers/peripheral/input
                 ↓
            消费端：input service / HID DDK 消费者

命名空间：
  ohos.hdi.input.v1_0           (主输入接口)
  ohos.hdi.input.ddk.v1_0       (HID DDK 1.0)
  ohos.hdi.input.ddk.v1_1       (HID DDK 1.1)
```

## 2. 知识路由

在规划或编辑前，先对任务分类，读取对应的代码路径和文档。

### Task-based routing

| 任务类型 | 读取 |
|---|---|
| 新增/修改主输入接口方法 | `v1_0/IInputInterfaces.idl` -> 评估版本影响 -> 同步实现端 `drivers/peripheral/input` + 消费端 input service |
| 输入事件回调变更 | `v1_0/IInputCallback.idl`（唯一回调定义）-> 评估对事件上报和热插拔功能的影响 -> 同步实现端 + 消费端 |
| 输入设备类型变更 | `v1_0/InputTypes.idl` -> 评估序列化兼容性（`DeviceInfo` / `DevAbility` / `EventPackage` 字段顺序是 ABI 一部分）-> 同步实现端 + 消费端 |
| HID DDK 接口变更（v1_0） | `ddk/v1_0/IHidDdk.idl` -> 评估对 v1_1 继承的影响（v1_1 `extends` v1_0）-> 同步实现端 |
| HID DDK 接口扩展（v1_1） | `ddk/v1_1/IHidDdk.idl` -> 确认仅影响 v1_1 消费者 -> 同步实现端 |
| HID DDK 类型变更 | `ddk/v1_0/HidDdkTypes.idl` 或 `ddk/v1_1/HidDdkTypes.idl` -> 评估跨版本影响（v1_1 import v1_0 类型）|
| HID DDK 错误码变更 | `ddk/v1_0/HidDdkTypes.idl`（`Hid_DdkErrCode`）+ `ddk/v1_1/HidDdkTypes.idl`（`HidDdkErrCode`）-> 注意两个版本错误码体系不同 |
| HID 键值码变更 | `ddk/v1_0/HidDdkTypes.idl` 中 `Hid_KeyCode` 枚举（80+ 键值）-> 评估 ABI 兼容性 |
| 构建配置变更 | `bundle.json`（依赖、inner_kits 列表）+ `v1_0/BUILD.gn` + `ddk/v1_0/BUILD.gn` + `ddk/v1_1/BUILD.gn` |
| 版本兼容性评估 | `ddk/v1_1/BUILD.gn`（`proxy_deps`/`stub_deps`）+ `ddk/v1_1/IHidDdk.idl`（`extends` 声明）+ `bundle.json`（三目标同时构建）|
| lite 系统适配 | `v1_0/BUILD.gn` 中 `if (defined(ohos_lite))` 分支；ddk 版本无 lite 条件分支 |

### Path-based routing

| 修改路径 | 需了解的上下文 |
|---|---|
| `v1_0/IInputInterfaces.idl` | 主输入接口，`mode = "passthrough"`（直通模式），修改后构建系统生成 `libinput_proxy_1.0`，**不要直接改生成产物** |
| `v1_0/IInputCallback.idl` | 回调接口（`[callback] interface`），含事件上报和热插拔两个回调，修改影响事件上报和热插拔功能 |
| `v1_0/InputTypes.idl` | 主输入类型定义，结构体字段顺序是 ABI 一部分，`DevAbility` 使用 bitmap 记录支持的事件类型 |
| `v1_0/BUILD.gn` | `mode = "passthrough"`（非 IPC，直通模式）+ `ohos_lite` 条件分支 + `install_images = ["system", "updater"]`，与 ddk 版本的 `mode = "ipc"` 不同 |
| `ddk/v1_0/IHidDdk.idl` | HID DDK 基线接口，v1_1 通过 `extends` 继承此接口，修改会传递到 v1_1 |
| `ddk/v1_0/HidDdkTypes.idl` | HID DDK 基线类型，`Hid_KeyCode` 枚举（80+ 键值）的顺序是 ABI 一部分，v1_1 import 此文件 |
| `ddk/v1_0/BUILD.gn` | `mode = "ipc"` + 无 `proxy_deps`/`stub_deps`（独立基线）+ 无 `ohos_lite` 条件分支 |
| `ddk/v1_1/IHidDdk.idl` | 扩展主接口，**`extends ohos.hdi.input.ddk.v1_0.IHidDdk`**（IDL 继承），import v1_0 的 `HidDdkTypes` 和 `IHidDdk` |
| `ddk/v1_1/HidDdkTypes.idl` | 扩展类型定义，import v1_0 的 `HidDdkTypes`，**`HidDdkErrCode` 与 v1_0 的 `Hid_DdkErrCode` 不同**（注意命名和值域差异）|
| `ddk/v1_1/BUILD.gn` | `proxy_deps`/`stub_deps` 指向 v1_0 是版本复用的关键约束，移除或修改会破坏 v1_1 对 v1_0 的继承 |
| `bundle.json` | 三目标 `sub_component`（v1_0 + ddk/v1_0 + ddk/v1_1）+ 8 个 inner_kits，新增版本需在此注册；无 `sa_profile`，无 `innerapi_tags` |

### Vocabulary-based routing

当任务、issue、日志、API 名称中出现以下术语时，先理解其含义和风险再动手：

| 术语 | 含义与风险 | 读取 |
|---|---|---|
| Input / input_service | 主输入 HDI，`module_name = "input_service"`，`mode = "passthrough"` | 本文件 + `v1_0/BUILD.gn` |
| IInputInterfaces | 主输入接口，18 个方法（设备扫描/开关/信息查询/电源/手势/测试/回调注册）| `v1_0/IInputInterfaces.idl` |
| IInputCallback | 输入回调接口（`[callback] interface`），含 `EventPkgCallback` + `HotPlugCallback` | `v1_0/IInputCallback.idl` |
| EventPackage | 输入事件数据包（type/code/value/timestamp），跨进程序列化 | `v1_0/InputTypes.idl` |
| HotPlugEvent | 热插拔事件（devIndex/devType/status），用于设备上下线通知 | `v1_0/InputTypes.idl` |
| DevAbility | 设备能力位图（devProp/eventType/absCode/relCode/keyCode/ledCode 等），记录支持的事件类型 | `v1_0/InputTypes.idl` |
| DeviceInfo | 设备信息（devIndex/devType/chipInfo/vendorName/chipName/attrSet/abilitySet）| `v1_0/InputTypes.idl` |
| ExtraCmd | 额外命令（cmdCode/cmdValue），用于厂商自定义命令 | `v1_0/InputTypes.idl` |
| HID DDK / hid_ddk | HID DDK 接口，`module_name = "hid_ddk"`，`mode = "ipc"` | 本文件 + `ddk/v1_0/BUILD.gn` |
| IHidDdk | HID DDK 主接口，v1_0 含 3 方法，v1_1 `extends` v1_0 并新增 14 方法 | `ddk/v1_0/IHidDdk.idl` + `ddk/v1_1/IHidDdk.idl` |
| Hid_Device | HID 设备基本信息（deviceName/vendorId/productId/version/bustype/properties）| `ddk/v1_0/HidDdkTypes.idl` |
| Hid_EventProperties | HID 事件属性（hidEventTypes/hidKeys/hidAbs/hidRelBits/hidMiscellaneous + abs 坐标范围）| `ddk/v1_0/HidDdkTypes.idl` |
| Hid_KeyCode | HID 键值码枚举（80+ 键值：字母/数字/功能键/鼠标按钮/触摸/手写笔等），枚举值顺序是 ABI 一部分 | `ddk/v1_0/HidDdkTypes.idl` |
| Hid_EmitItem | HID 事件发送项（type/code/value）| `ddk/v1_0/HidDdkTypes.idl` |
| HidDeviceHandle | HID 设备操作句柄（fd/nonBlock），v1_1 新增 | `ddk/v1_1/HidDdkTypes.idl` |
| HidReportType | HID 报告类型（INPUT_REPORT/OUTPUT_REPORT/FEATURE_REPORT），v1_1 新增 | `ddk/v1_1/HidDdkTypes.idl` |
| HidRawDevInfo | HID 原始设备信息（busType/vendor/product），v1_1 新增 | `ddk/v1_1/HidDdkTypes.idl` |
| Hid_DdkErrCode | v1_0 错误码（HID_DDK_SUCCESS=0 / HID_DDK_FAILURE=-1 / INVALID_PARAMETER=-2 等）| `ddk/v1_0/HidDdkTypes.idl` |
| HidDdkErrCode | v1_1 错误码（HID_DDK_SUCCESS=0 / HID_DDK_NO_PERM=201 / INVALID_PARAMETER=401 / FAILURE=27300001 等），**与 v1_0 不同** | `ddk/v1_1/HidDdkTypes.idl` |
| passthrough | 直通模式，`v1_0` 使用此模式（非 IPC），与 ddk 版本的 `ipc` 模式不同 | `v1_0/BUILD.gn` |
| hdi-gen | HDL IDL 编译器，将 `.idl` 编译为 proxy/stub C++ 代码 | 构建系统 `//build/config/components/hdi/hdi.gni` |
| hdi() | GN 模板，用于声明 HDL 接口模块 | `v1_0/BUILD.gn` + `ddk/v1_0/BUILD.gn` + `ddk/v1_1/BUILD.gn` |
| proxy_deps / stub_deps | `ddk/v1_1/BUILD.gn` 中指向 v1_0 的依赖声明，是版本复用机制的核心 | `ddk/v1_1/BUILD.gn` |
| extends | IDL 继承关键字，`ddk/v1_1/IHidDdk.idl` 使用 `extends ohos.hdi.input.ddk.v1_0.IHidDdk` | `ddk/v1_1/IHidDdk.idl` |
| libinput_proxy_1.0 | v1_0 生成的 proxy 库 | 构建产物 |
| libhid_ddk_proxy_1.0 / libhid_ddk_stub_1.0 | ddk/v1_0 生成的 proxy/stub 库 | 构建产物 |
| libhid_ddk_proxy_1.1 / libhid_ddk_stub_1.1 | ddk/v1_1 生成的 proxy/stub 库（依赖 v1_0 的 proxy/stub）| 构建产物 |
| inner_kits | 内部 SDK 接口声明，本仓导出 8 个（v1_0 2 个 + ddk/v1_0 3 个 + ddk/v1_1 3 个）| `bundle.json` |
| ohos_lite | lite 系统（small/mini）条件编译标志，仅 `v1_0/BUILD.gn` 有此分支（lite 下 proxy 为空 group）| `v1_0/BUILD.gn` |
| ohos.hdi.input.v1_0 | 主输入接口包命名空间 | `v1_0/InputTypes.idl` 顶部 `package` 声明 |
| ohos.hdi.input.ddk.v1_0 / ohos.hdi.input.ddk.v1_1 | HID DDK 包命名空间 | `ddk/v1_0/HidDdkTypes.idl` + `ddk/v1_1/HidDdkTypes.idl` |

在计划阶段，必须声明：
- **任务分类**（如：修改主输入接口 / 回调接口变更 / 类型变更 / HID DDK 接口变更 / HID DDK 类型变更 / 构建配置变更）
- **目标子模块**（主输入接口 `v1_0/` vs HID DDK `ddk/v1_0/` 或 `ddk/v1_1/`）
- **已读取的代码路径和文档**（具体到文件路径）
- **发现的约束**（架构不变量、禁止事项、版本兼容性约束、跨仓依赖）
- **是否需要同步修改其他层**：
  - `.idl` 变更 -> 由构建系统重生成 proxy/stub + 同步实现端 `drivers/peripheral/input` + 消费端
  - `ddk/v1_0` 变更 -> 必须评估对 v1_1 继承的影响（v1_1 `extends` v1_0）
  - `bundle.json` 变更 -> 同步 `inner_kits` 列表
  - 枚举/结构体字段变更 -> 评估 ABI 兼容性

## 3. 约束边界

### 架构不变量

- **`.idl` 文件是唯一真相源**：所有接口和类型定义必须通过 `.idl` 文件表达，proxy/stub 由 `hdi-gen` 自动生成
- **两个独立子模块**：主输入接口（`v1_0/`，`mode = "passthrough"`）和 HID DDK（`ddk/`，`mode = "ipc"`）是独立的子模块，`module_name` 不同（`input_service` vs `hid_ddk`），模式不同，不要混淆
- **ddk/v1_1 继承 ddk/v1_0**：`ddk/v1_1/IHidDdk.idl` 使用 `extends ohos.hdi.input.ddk.v1_0.IHidDdk` 进行 IDL 继承，同时通过 `proxy_deps`/`stub_deps` 复用 v1_0 的 proxy/stub
- **三目标同时构建**：`v1_0`、`ddk/v1_0`、`ddk/v1_1` 都在 `bundle.json` 的 `sub_component` 中注册，均生成独立的 proxy/stub 库
- **IDL 类型顺序是 ABI 一部分**：`Hid_KeyCode` 枚举值顺序、`DeviceInfo` / `EventPackage` 等结构体字段顺序一旦发布即固定，新增必须追加
- **本仓不含实现**：接口实现由 `drivers/peripheral/input` 提供，本仓仅定义契约
- **无 `innerapi_tags`**：与 sensor（`chipsetsdk` / `platformsdk_indirect`）不同，本子域无 `innerapi_tags` 声明
- **无 `sa_profile`**：本子域 `bundle.json` 无 `sa_profile` 字段
- **无 `.gni` 特性开关**：本子域无 `declare_args()` 特性开关，所有能力通过版本目录表达
- **ohos_lite 仅影响 v1_0**：仅 `v1_0/BUILD.gn` 有 `ohos_lite` 条件分支（lite 下 proxy 为空 group），ddk 版本无此分支

### 禁止事项

- **不要直接编辑 `hdi-gen` 生成的 proxy/stub `.h/.cpp` 代码**，应修改 `.idl` 源文件后由构建系统重新生成
- **不要在 `ddk/v1_1` 中重复定义 v1_0 已有的方法**：v1_1 通过 `extends` 继承 v1_0 的 `CreateDevice` / `EmitEvent` / `DestroyDevice`，重复定义会破坏继承机制
- **不要删除或修改 `ddk/v1_1/BUILD.gn` 中的 `proxy_deps`/`stub_deps`**：这是 v1_1 复用 v1_0 proxy/stub 的关键声明
- **不要删除 `ddk/v1_1/IHidDdk.idl` 中的 `extends` 声明**：这是 v1_1 继承 v1_0 接口的 IDL 声明
- **不要调整 `Hid_KeyCode` 枚举值顺序**：已有 80+ 键值的位置是 ABI 契约，新增键值必须追加
- **不要在 `EventPackage` / `DeviceInfo` 等结构体中间插入字段**：跨进程序列化的字段顺序是 ABI 一部分，新增字段必须追加在末尾
- **不要混淆 `passthrough` 和 `ipc` 模式**：`v1_0` 使用 `passthrough`（直通），ddk 使用 `ipc`，两者调用机制不同
- **不要混淆 `Hid_DdkErrCode`（v1_0）和 `HidDdkErrCode`（v1_1）**：两个版本的错误码枚举命名和值域不同
- **不要移除 `bundle.json` 中任一活跃目标的 `sub_component`**：三目标同时构建是兼容性要求
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审（当前依赖：`c_utils` / `ipc` / `hdf_core` / `hilog`）
- **不要在本仓中添加实现代码**：本仓是纯接口定义仓

### Ask before 规则

在执行以下操作前，必须先确认或询问：

| 操作 | 确认内容 | 确认对象 |
|------|----------|----------|
| 修改 `bundle.json` 的 `sub_component` | `inner_kits` 是否同步更新 | 项目负责人 |
| 新增/删除 IDL 方法 | 是否影响 ABI 契约 | 接口评审团队 |
| 新增版本目录 | 版本号是否与 `bundle.json` 一致 | 项目负责人 |
| 修改 `ddk/v1_1` 的 `proxy_deps`/`stub_deps` | 是否破坏 v1_1 对 v1_0 的继承复用 | 接口评审团队 |
| 修改 `ddk/v1_1` 的 `extends` 声明 | 是否影响 v1_1 对 v1_0 接口的继承 | 接口评审团队 |
| 修改 `v1_0/BUILD.gn` 的 `mode` | 是否影响 passthrough 调用机制 | 构建团队 |
| 修改 `Hid_KeyCode` 枚举 | 是否影响 ABI 兼容性（80+ 键值）| 接口评审团队 |
| 引入新的生产依赖 | 是否经过 `bundle.json` 评审 | 依赖评审团队 |
| 修改 `BUILD.gn` 的 deps | 是否引入新的外部依赖 | 依赖评审团队 |
| 修改 `ohos_lite` 条件分支 | 是否影响 lite 系统下 proxy 为空 group 的行为 | 构建团队 |
| 重命名 IDL 文件 | 是否影响所有引用该文件的消费者 | 所有消费方 |

### 需确认后再修改

- **`IInputCallback.idl` 回调接口变更**（影响事件上报和热插拔功能，需确认实现端和消费端兼容性）
- **`ddk/v1_0/IHidDdk.idl` 接口变更**（v1_1 `extends` 继承此接口，修改会传递到 v1_1）
- **`Hid_KeyCode` 枚举值变更**（80+ 键值，需评估 ABI 兼容性，已发布的枚举值位置不能调整）
- **`EventPackage` / `DeviceInfo` / `DevAbility` 字段变更**（需评估序列化兼容性，跨进程传输的 Parcelable 字段顺序是 ABI 一部分）
- **`Hid_DdkErrCode`（v1_0）与 `HidDdkErrCode`（v1_1）变更**（两个版本错误码体系不同，需分别评估）
- **新增 HDI 版本**（需确认版本规划，在 `bundle.json` 注册 `sub_component` + `inner_kits`，决定是否 `extends` 已有版本）
- **`bundle.json` 中 `inner_kits` 列表变更**（需确认下游消费者依赖关系）
- **新增外部依赖**（需确认许可证、包大小、`bundle.json` 同步）
- **`adapted_system_type` 变更**（当前 `["standard", "small"]`，新增 `mini` 需评估 lite 系统兼容性）
- **`mode` 变更**（`passthrough` vs `ipc`，影响调用机制，需确认实现端和消费端兼容性）

### 项目特定陷阱

- **两个子模块的 `mode` 差异**：`v1_0` 使用 `mode = "passthrough"`（直通模式，无 IPC 开销），ddk 使用 `mode = "ipc"`（IPC 通信），两者调用机制完全不同，不要混淆
- **ddk/v1_1 的双重依赖机制**：v1_1 同时使用 IDL `extends`（接口继承）+ `proxy_deps`/`stub_deps`（proxy/stub 复用），两者缺一不可；只改其中一个会破坏版本复用
- **ddk/v1_1 的 import 链**：`ddk/v1_1/IHidDdk.idl` import 了 `v1_1.HidDdkTypes` + `v1_0.HidDdkTypes` + `v1_0.IHidDdk`，三个 import 缺一不可
- **错误码枚举命名差异**：v1_0 是 `Hid_DdkErrCode`（下划线分隔，值 -1~-6），v1_1 是 `HidDdkErrCode`（驼峰式，值 201/401/27300001+），命名和值域均不同，不要混淆
- **`Hid_KeyCode` 的键值范围跨度大**：枚举值从 1（ESC）到 0x150（BTN_WHEEL），中间有间断，新增键值需注意值域不连续的特性
- **`module_name` 的一致性**：v1_0 = `"input_service"`，ddk/v1_0 和 ddk/v1_1 = `"hid_ddk"`，两个子模块的 `module_name` 不同，不能混淆
- **`ohos_lite` 仅影响 v1_0**：仅 `v1_0/BUILD.gn` 有 `if (defined(ohos_lite))` 分支，ddk 版本无此分支，lite 系统下 ddk 仍正常构建
- **`install_images` 仅 v1_0 有**：`v1_0/BUILD.gn` 设置 `install_images = ["system", "updater"]`，ddk 版本无此设置
- **无 `innerapi_tags`**：本子域不暴露为 `chipsetsdk` 或 `platformsdk_indirect`，与 sensor 不同
- **无 `sa_profile`**：本子域 `bundle.json` 无 `sa_profile` 字段，与部分其他子域不同
- **`DevAbility` 的 bitmap 设计**：使用 `unsigned long[]` 数组作为 bitmap 记录支持的事件类型，跨进程序列化时需注意数组长度一致性
- **`adapted_system_type` 不含 `mini`**：本仓适配 `standard` 和 `small` 系统，不适配 `mini` 系统

### Agent 失败模式

| 失败模式 | 描述 | 预防 |
|----------|------|------|
| 直接编辑生成产物 | 直接修改 `hdi-gen` 生成的 proxy/stub `.h/.cpp` 文件 | 始终修改 `.idl` 源文件后由构建系统重生成 |
| 子模块混淆 | 在 `v1_0`（passthrough）和 `ddk`（ipc）之间混淆 `mode` / `module_name` / 调用机制 | 修改前确认目标子模块和 `BUILD.gn` 的 `mode` 设置 |
| 版本继承遗漏 | 修改 `ddk/v1_0` 时忘记评估对 v1_1 继承的影响 | 修改 `ddk/v1_0` 后必须验证 v1_1 的 `extends` 和 `proxy_deps`/`stub_deps` |
| 错误码混淆 | 混淆 `Hid_DdkErrCode`（v1_0）和 `HidDdkErrCode`（v1_1） | 修改前确认目标版本和错误码枚举命名 |
| ABI 破坏 | 修改 `Hid_KeyCode` 枚举值顺序或结构体字段顺序 | 已有枚举值和字段顺序是 ABI 契约，新增必须追加 |
| 跨层同步遗漏 | 修改了 IDL 但未同步 `drivers/peripheral/input` 实现 | 修改 IDL 后必须验证外围仓实现 |
| inner_kits 不同步 | 修改 `sub_component` 但未同步 `inner_kits` | 修改后必须验证 `bundle.json` 字段一致性 |
| 依赖遗漏 | 新增依赖但未在 `bundle.json` 中声明 | 所有生产依赖必须经过 `bundle.json` 评审 |
| ohos_lite 误判 | 以为 ddk 版本也有 `ohos_lite` 条件分支 | 仅 `v1_0/BUILD.gn` 有此分支 |
| extends 声明误删 | 删除 `ddk/v1_1/IHidDdk.idl` 中的 `extends` 声明 | `extends` 是 v1_1 继承 v1_0 的 IDL 声明，不能删除 |

## 4. 验证闭环

### 构建命令

| 子模块 | 构建命令 | 单独构建活跃版本 |
|--------|----------|-----------------|
| input（全量） | `./build.sh --product-name rk3568 --build-target drivers_interface_input` | - |
| 主输入接口 v1_0 | 同上 | `//drivers/interface/input/v1_0:input_idl_target` |
| HID DDK v1_0 | 同上 | `//drivers/interface/input/ddk/v1_0:hid_ddk_idl_target` |
| HID DDK v1_1 | 同上 | `//drivers/interface/input/ddk/v1_1:hid_ddk_idl_target` |

### 任务特定验证表

| 任务类型 | 验证命令 | 验证目标 |
|----------|----------|----------|
| 新增/修改主输入接口方法 | `./build.sh --product-name rk3568 --build-target drivers_interface_input --lint` | IDL 生成一致性 + passthrough 模式 |
| `IInputCallback.idl` 变更 | 构建全量 + 验证事件上报和热插拔功能 + 同步实现端和消费端 | 回调接口一致性 |
| `InputTypes.idl` 类型变更 | 构建全量 + 验证枚举值/字段顺序未破坏 ABI + 同步 `drivers/peripheral/input` | ABI 兼容性 |
| HID DDK v1_0 接口变更 | 构建 `ddk/v1_0` + `ddk/v1_1` + 验证 v1_1 `extends` 继承正确 + 同步实现端 | 版本继承一致性 |
| HID DDK v1_1 接口扩展 | 构建 `ddk/v1_1` + 验证 v1_0 不受影响 + 同步实现端 | 版本独立性 |
| `Hid_KeyCode` 枚举变更 | 构建 `ddk/v1_0` + `ddk/v1_1` + 验证枚举值顺序未破坏 ABI | ABI 兼容性 |
| 错误码变更 | 构建对应版本 + 验证 `Hid_DdkErrCode`（v1_0）/ `HidDdkErrCode`（v1_1）一致性 | 错误码体系一致性 |
| 构建配置变更 | `./build.sh --product-name rk3568 --build-target drivers_interface_input` + 验证 8 个 `inner_kits` 均正常导出 | inner_kits 一致性 |
| inner_kits 变更 | 检查 `bundle.json` 中 `sub_component` 和 `inner_kits` 列表 | 字段一致性 |
| ohos_lite 适配 | 在 lite 系统配置下构建 + 验证 `v1_0` proxy 为空 group 的行为；ddk 版本正常构建 | lite 系统兼容性 |
| ABI 兼容性变更 | `gn gen --check && hdi-gen --check` | 配置正确性 |

### 通用验证步骤

```bash
# 构建 input 接口组件（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target drivers_interface_input

# 单独构建主输入接口 v1_0
./build.sh --product-name rk3568 --build-target //drivers/interface/input/v1_0:input_idl_target

# 单独构建 HID DDK v1_0
./build.sh --product-name rk3568 --build-target //drivers/interface/input/ddk/v1_0:hid_ddk_idl_target

# 单独构建 HID DDK v1_1（依赖 v1_0，会自动拉起 v1_0 构建）
./build.sh --product-name rk3568 --build-target //drivers/interface/input/ddk/v1_1:hid_ddk_idl_target

# 静态分析（Lint）
./build.sh --product-name rk3568 --build-target drivers_interface_input --lint
gn gen --check
hdi-gen --check

# 验证 inner_kits
# 检查 bundle.json 中 sub_component 和 inner_kits 列表
python3 -c "import json; d=json.load(open('bundle.json')); assert 'inner_kits' in d; print('inner_kits:', d['inner_kits'])"

# 验证 IDL 一致性
hdi-gen --check
```

### Done 定义

任务完成的判定标准：
1. 构建命令执行成功（`./build.sh --product-name rk3568 --build-target drivers_interface_input` 返回 0）
2. 静态分析通过（`--lint` 返回 0，`gn gen --check` 通过，`hdi-gen --check` 通过）
3. `bundle.json` 中 `sub_component` 和 `inner_kits` 同步更新
4. 跨层同步已验证（IDL 与外围仓 `drivers/peripheral/input` 实现匹配）
5. `ddk/v1_0` 变更已评估对 `ddk/v1_1` 继承的影响
6. 枚举值/结构体字段顺序变更已评估 ABI 兼容性
7. 错误码变更已确认目标版本（v1_0 `Hid_DdkErrCode` vs v1_1 `HidDdkErrCode`）
8. proxy/stub 由 `hdi-gen` 自动重新生成，无手改生成产物
9. 最终报告已输出（包含文件清单、验证结果、确认事项）

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 目标子模块（主输入接口 `v1_0/` vs HID DDK `ddk/v1_0/` 或 `ddk/v1_1/`）及是否为活跃构建
4. 是否触发跨层同步修改（IDL 重生成 / 外围仓 `drivers/peripheral/input` / `bundle.json`）
5. 是否影响版本兼容性（ddk/v1_1 对 ddk/v1_0 的 `extends` 继承 + `proxy_deps`/`stub_deps` 复用）
6. 是否影响特性开关默认值或 ABI 契约
7. 是否触及架构不变量或需确认事项
8. 所有验证命令的执行结果（构建/测试/lint/静态分析）

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样。涉及外围仓 `drivers/peripheral/input` 的变更，必须人工复核 IDL 与实现的匹配性并说明无法在沙箱验证的限制。涉及 `ddk/v1_1` 继承关系的变更，必须人工复核 `extends` 声明和 `proxy_deps`/`stub_deps` 的一致性。
