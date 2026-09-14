# AGENTS.md - OpenHarmony USB HDI 接口定义（drivers_interface_usb）

## 0. Where to look

| 遇到问题 | 读取路径 |
|----------|----------|
| 不确定哪个子模块负责 | 顶部「子模块索引」表 |
| 需要知道 IDL 定义 | `drivers/interface/usb/**/*.idl` |
| 需要知道 bundle 配置 | `drivers/interface/usb/bundle.json` |
| 需要构建命令 | 顶部「构建命令」表 |
| 需要版本依赖关系 | 顶部「版本依赖链」表 |
| 需要知道构建模式 | 顶部「构建模式」表 |
| 需要 inner_kits 变更验证 | `bundle.json` inner_kits 列表（35 条） |

## 1. 代码地图

本仓库定义 OpenHarmony USB HDI 接口（`@ohos/drivers_interface_usb`，版本 5.0），subsystem `hdf`，part `drivers_interface_usb`。核心职责是定义 USB 主机/设备/端口管理、USB DDK、串口、SCSI 外设 DDK、USB Serial DDK、USB MTP Gadget 的 IDL 契约。由 `hdi-gen` 工具自动生成 proxy/stub 代码。**本仓只维护 `.idl` 源文件，不包含任何实现代码**——实现由 `drivers/peripheral/usb` 提供，消费方为 USB Service、multimedia、miscservices 等。

### 子模块索引

| 子模块 | 功能 | 版本 | 构建模式 | 活跃构建目标 | innerapi_tags |
|--------|------|------|----------|-------------|---------------|
| `v1_0/` | USB 主机接口（基础设备管理/传输/端口/功能） | v1.0 @since 3.2 | 默认（passthrough lite / ipc standard） | `usb_idl_target` | 无 |
| `v1_1/` | USB 主机接口扩展（FD/速度/ClearHalt/Reset/Accessory） | v1.1 @since 5.0 | 默认 | `usb_idl_target` | 无 |
| `v1_2/` | USB 异步传输（UsbSubmitTransfer/UsbCancelTransfer） | v1.2 @since 5.1 | 默认 | `usb_idl_target` | 无 |
| `v2_0/` | **重构版**：拆分为 Device/Port/Host 三接口 + 设备/接口授权 | v2.0 @since 5.1 | 默认 | `usb_idl_target` | `chipsetsdk_sp` |
| `v2_1/` | 自定义控制传输数据获取 | v2.1 @since 7.0 | 默认 | `usb_idl_target` | `chipsetsdk_sp` |
| `ddk/v1_0/` | USB DDK（设备描述符/接口声明/控制传输/管道传输） | v1.0 @since 4.0 | `ipc` | `usb_ddk_idl_target` | 无 |
| `ddk/v1_1/` | USB DDK 扩展（设备枚举/驱动信息管理） | v1.1 @since 5.1 | `ipc` | `usb_ddk_idl_target` | 无 |
| `ddk/v1_2/` | USB DDK 扩展（ControlTransfer/非根 Hub 查询） | v1.2 @since 7.0 | `ipc` | `usb_ddk_idl_target` | 无 |
| `serial/v1_0/` | 串口接口（打开/关闭/读写/属性设置） | v1.0 @since 5.1 | `passthrough` | `serial_idl_target` | 无 |
| `scsi_ddk/v1_0/` | SCSI 外设 DDK（Open/Close/IO/Inquiry/容量/Sense/CDB） | v1.0 @since 5.1 | `ipc` | 无（仅 inner_kits） | 无 |
| `usb_serial_ddk/v1_0/` | USB Serial DDK（Open/Close/Read/Write/波特率/流控） | v1.0 @since 5.1 | `ipc` | 无（仅 inner_kits） | 无 |
| `gadget/mtp/v1_0/` | USB MTP/PTP Gadget（Start/Stop/Read/Write/文件传输） | v1.0 @since 4.0 | `ipc` | `usbfn_mtp_idl_target` | 无 |

### 版本依赖链

**主 USB 接口（v1_x 链：继承式扩展）：**

```
v1_0 (base, 无 proxy_deps/stub_deps)
  └─ v1_1 extends v1_0.IUsbInterface (proxy_deps: v1_0, stub_deps: v1_0)
       └─ v1_2 extends v1_1.IUsbInterface (proxy_deps: v1_0+v1_1, stub_deps: v1_0+v1_1)
```

**主 USB 接口（v2_x 链：重构+扩展）：**

```
v2_0 (self-contained, 无 proxy_deps/stub_deps, 定义全部类型和回调)
  └─ v2_1 extends v2_0.IUsbDeviceInterface (proxy_deps: v2_0, stub_deps: v2_0)
```

> v2_0 不继承 v1_x，是独立重构版本，将 v1_2 的单一 `IUsbInterface` 拆分为 `IUsbDeviceInterface` + `IUsbPortInterface` + `IUsbHostInterface` 三个接口，并重新定义全部类型。

**USB DDK 链（继承式扩展）：**

```
ddk/v1_0 (base, 无 proxy_deps/stub_deps)
  └─ ddk/v1_1 extends v1_0.IUsbDdk (proxy_deps: v1_0, stub_deps: v1_0)
       └─ ddk/v1_2 extends v1_1.IUsbDdk (proxy_deps: v1_0+v1_1, stub_deps: v1_0+v1_1)
```

**独立模块（无版本依赖）：**

- `serial/v1_0`：独立，无 proxy_deps/stub_deps
- `scsi_ddk/v1_0`：独立，无 proxy_deps/stub_deps
- `usb_serial_ddk/v1_0`：独立，无 proxy_deps/stub_deps
- `gadget/mtp/v1_0`：独立，无 proxy_deps/stub_deps

### 构建模式

| 模式 | 适用子模块 | 说明 |
|------|-----------|------|
| 默认（无 `mode` 指定） | v1_0/v1_1/v1_2/v2_0/v2_1 | `ohos_lite` 下 proxy 为空 group，standard 下自动选择 passthrough/ipc |
| `ipc` | ddk/v1_0-v1_2、scsi_ddk/v1_0、usb_serial_ddk/v1_0、gadget/mtp/v1_0 | 强制 IPC 模式，无 ohos_lite 条件分支 |
| `passthrough` | serial/v1_0 | 强制直通模式，无 ohos_lite 条件分支 |

### 架构边界

- **`.idl` 文件是唯一真相源**：所有 proxy/stub/headers 由 `hdi-gen` 在构建时生成
- **本仓**：仅维护 `.idl` 源文件和 `bundle.json`/`BUILD.gn` 构建配置
- **实现方**：`drivers/peripheral/usb` 仓维护 HDI 服务实现
- **消费方**：USB Service、multimedia、miscservices 等

### 非本项目维护的目录

- 任何由 `hdi-gen` 生成的 proxy/stub `.h/.cpp` 文件（构建产物，不在源码树中）
- `drivers/peripheral/usb/`：USB HDI 实现端（由驱动团队维护）
- USB Service 中消费 USB 接口的代码（由上层服务团队维护）

### 关键区域

- `bundle.json`：组件清单，`adapted_system_type: ["standard", "small"]`，依赖 `c_utils`/`ipc`/`hdf_core`/`hilog`，10 个 `sub_component`，35 个 `inner_kits`，**无 `sa_profile` 字段**，**无 `innerapi_tags` 字段**（innerapi_tags 在各版本 BUILD.gn 中声明）
- `v1_0/`：USB HDI 1.0 基础接口（@since 3.2），4 个 IDL 文件：
  - `IUsbInterface.idl`：~30 方法（OpenDevice/CloseDevice/GetDeviceDescriptor/ClaimInterface/BulkTransferRead/ControlTransfer/RequestQueue/GetCurrentFunctions/SetPortRole/QueryPort/BindUsbdSubscriber/RegBulkCallback 等），含 3 个 `@deprecated since 5.0` 方法
  - `UsbTypes.idl`：PortInfo/UsbDev/UsbPipe/UsbCtrlTransfer/USBDeviceInfo
  - `IUsbdSubscriber.idl`：回调（DeviceEvent/PortChangedEvent）
  - `IUsbdBulkCallback.idl`：回调（OnBulkWriteCallback/OnBulkReadCallback）
- `v1_1/`：USB HDI 1.1 扩展（@since 5.0），2 个 IDL 文件：
  - `IUsbInterface.idl`：extends v1_0，新增 10 方法（GetInterfaceActiveStatus/GetDeviceSpeed/GetDeviceFileDescriptor/BulkTransferReadwithLength/ClearHalt/ControlTransferReadwithLength/ResetDevice/GetAccessoryInfo/OpenAccessory/CloseAccessory）
  - `UsbTypes.idl`：新增 UsbCtrlTransferParams
- `v1_2/`：USB HDI 1.2 异步传输（@since 5.1），3 个 IDL 文件：
  - `IUsbInterface.idl`：extends v1_1，新增 2 方法（UsbSubmitTransfer/UsbCancelTransfer）
  - `UsbTypes.idl`：新增 UsbIsoPacketDescriptor/USBTransferInfo
  - `IUsbdTransferCallback.idl`：回调（OnTransferWriteCallback/OnTransferReadCallback）
- `v2_0/`：**重构版**（@since 5.1），7 个 IDL 文件，**self-contained（不依赖 v1_x）**：
  - `IUsbDeviceInterface.idl`：设备管理（GetCurrentFunctions/SetCurrentFunctions/GetAccessoryInfo/OpenAccessory/CloseAccessory/BindUsbdDeviceSubscriber/UnbindUsbdDeviceSubscriber/UsbDeviceAuthorize@since6.0/UsbInterfaceAuthorize@since6.0）
  - `IUsbPortInterface.idl`：端口管理（SetPortRole/QueryPort/QueryPorts/BindUsbdPortSubscriber/UnbindUsbdPortSubscriber）
  - `IUsbHostInterface.idl`：主机操作（~40 方法，含 OpenDevice/CloseDevice/ClaimInterface/BulkTransfer/ControlTransfer/InterruptTransfer/IsoTransfer/RequestQueue/UsbSubmitTransfer 等，含 3 个 `@deprecated` 方法）
  - `UsbTypes.idl`：完整类型定义（UsbCtrlTransferParams/UsbDev/UsbPipe/UsbCtrlTransfer/USBDeviceInfo/PortInfo/UsbPortStatus/UsbPort/UsbIsoPacketDescriptor/USBTransferInfo）
  - `IUsbdSubscriber.idl`/`IUsbdBulkCallback.idl`/`IUsbdTransferCallback.idl`：3 个回调接口
- `v2_1/`：扩展版（@since 7.0），1 个 IDL 文件：
  - `IUsbDeviceInterface.idl`：extends v2_0，新增 GetControlTransferData，`no_other_header_files = true`
- `ddk/v1_0/`：USB DDK 基础（@since 4.0），2 个 IDL 文件，`mode = "ipc"`，`LOG_DOMAIN=0xD002518`
- `ddk/v1_1/`：USB DDK 扩展（@since 5.1），2 个 IDL 文件，extends v1_0
- `ddk/v1_2/`：USB DDK 扩展（@since 7.0），1 个 IDL 文件，extends v1_1，`no_other_header_files = true`
- `serial/v1_0/`：串口接口（@since 5.1），2 个 IDL 文件，`mode = "passthrough"`
- `scsi_ddk/v1_0/`：SCSI 外设 DDK（@since 5.1），2 个 IDL 文件，`mode = "ipc"`
- `usb_serial_ddk/v1_0/`：USB Serial DDK（@since 5.1），2 个 IDL 文件，`mode = "ipc"`，`@syscap SystemCapability.Driver.UsbSerial.Extension`
- `gadget/mtp/v1_0/`：USB MTP Gadget（@since 4.0），2 个 IDL 文件，`mode = "ipc"`

## 2. 知识路由

### 计划阶段声明

在计划阶段，必须声明：
1. 任务类别及目标子模块版本（主 USB / ddk / serial / scsi_ddk / usb_serial_ddk / gadget/mtp）
2. 已读取的文档/IDL 文件清单（按「按子模块路由」表确认覆盖范围）
3. 已发现的约束（按「禁止事项」和「Ask-before 规则」逐条确认是否触发）
4. 是否需要新增/修改 `.idl` 方法
5. 是否需要新增版本目录或修改 `bundle.json`
6. 是否需要同步 inner_kits（35 条）
7. 是否影响 ABI 契约（继承链中的方法追加）
8. 是否影响 `innerapi_tags`（v2_0/v2_1 的 `chipsetsdk_sp` 标签）

### 按子模块路由

| 任务类型 | 读取 |
|----------|------|
| 主 USB 接口变更（v1_x 链） | `v1_0/` → `v1_1/` → `v1_2/` 对应版本 IDL |
| 主 USB 接口变更（v2_x 链） | `v2_0/` 对应接口 IDL → `v2_1/`（如涉及设备接口扩展） |
| USB DDK 变更 | `ddk/v1_0/` → `ddk/v1_1/` → `ddk/v1_2/` 对应版本 IDL |
| 串口变更 | `serial/v1_0/ISerialInterface.idl` + `serial/v1_0/SerialTypes.idl` |
| SCSI DDK 变更 | `scsi_ddk/v1_0/IScsiPeripheralDdk.idl` + `scsi_ddk/v1_0/ScsiPeripheralDdkTypes.idl` |
| USB Serial DDK 变更 | `usb_serial_ddk/v1_0/IUsbSerialDdk.idl` + `usb_serial_ddk/v1_0/UsbSerialDdkTypes.idl` |
| MTP Gadget 变更 | `gadget/mtp/v1_0/IUsbfnMtpInterface.idl` + `gadget/mtp/v1_0/UsbfnMtpTypes.idl` |
| 构建配置变更 | `bundle.json` + 对应版本 `BUILD.gn` |
| innerapi_tags 变更 | `v2_0/BUILD.gn` 或 `v2_1/BUILD.gn` 的 `innerapi_tags` 字段 |

### 按任务类型路由

| 任务类型 | 操作 |
|----------|------|
| 新增/修改 IDL 方法 | 读对应版本 `.idl` → `hdi-gen` 重生成 → 同步 `drivers/peripheral/usb` |
| 新增版本 | 新建版本目录 → 修改 `bundle.json` sub_component + inner_kits → 声明 proxy_deps/stub_deps |
| 构建配置变更 | 修改 `BUILD.gn`（mode/proxy_deps/stub_deps/innerapi_tags/no_other_header_files）→ 同步 `bundle.json` |
| inner_kits 变更 | 检查 `bundle.json` 中 sub_component 和 inner_kits 列表一致性（35 条） |
| ABI 兼容性变更 | 评估继承链影响（v1_0→v1_1→v1_2 或 v2_0→v2_1 或 ddk v1_0→v1_1→v1_2） |
| innerapi_tags 变更 | 确认 `chipsetsdk_sp` 标签影响范围（仅 v2_0/v2_1） |

### 按接口版本路由

| 版本 | 子模块 | 说明 |
|------|--------|------|
| v1_0 | 主 USB | 基础版本（@since 3.2），v1_x 继承链根 |
| v1_1 | 主 USB | 扩展 FD/速度/Reset/Accessory（@since 5.0） |
| v1_2 | 主 USB | 异步传输（@since 5.1），v1_x 继承链顶端 |
| v2_0 | 主 USB | **重构版**，三接口拆分，self-contained（@since 5.1） |
| v2_1 | 主 USB | 自定义控制传输数据（@since 7.0），v2_x 继承链顶端 |
| ddk/v1_0 | USB DDK | 基础 DDK（@since 4.0），DDK 继承链根 |
| ddk/v1_1 | USB DDK | 设备枚举/驱动信息（@since 5.1） |
| ddk/v1_2 | USB DDK | ControlTransfer/非根Hub（@since 7.0），DDK 继承链顶端 |
| serial/v1_0 | 串口 | 独立版本（@since 5.1） |
| scsi_ddk/v1_0 | SCSI DDK | 独立版本（@since 5.1） |
| usb_serial_ddk/v1_0 | USB Serial DDK | 独立版本（@since 5.1） |
| gadget/mtp/v1_0 | MTP Gadget | 独立版本（@since 4.0） |

### 术语路由

| 术语 | 含义 | 读取路径 |
|------|------|----------|
| `IUsbInterface` | v1_x 主 USB 接口（单一接口） | `v1_0/`→`v1_1/`→`v1_2/IUsbInterface.idl` |
| `IUsbDeviceInterface` | v2_x 设备管理接口 | `v2_0/IUsbDeviceInterface.idl`、`v2_1/IUsbDeviceInterface.idl` |
| `IUsbPortInterface` | v2_x 端口管理接口 | `v2_0/IUsbPortInterface.idl` |
| `IUsbHostInterface` | v2_x 主机操作接口 | `v2_0/IUsbHostInterface.idl` |
| `IUsbDdk` | USB DDK 接口 | `ddk/v1_0/`→`v1_1/`→`v1_2/IUsbDdk.idl` |
| `ISerialInterface` | USB 串口通信接口 | `serial/v1_0/ISerialInterface.idl` |
| `IScsiPeripheralDdk` | SCSI 外设 DDK 接口 | `scsi_ddk/v1_0/IScsiPeripheralDdk.idl` |
| `IUsbSerialDdk` | USB Serial DDK 接口 | `usb_serial_ddk/v1_0/IUsbSerialDdk.idl` |
| `IUsbfnMtpInterface` | USB MTP/PTP Gadget 接口 | `gadget/mtp/v1_0/IUsbfnMtpInterface.idl` |
| `proxy_deps`/`stub_deps` | 版本间依赖声明 | 各版本 `BUILD.gn` |
| `innerapi_tags` | 内部 API 标签（`chipsetsdk_sp`） | `v2_0/BUILD.gn`、`v2_1/BUILD.gn` |
| `no_other_header_files` | 禁止额外头文件导出 | `v2_1/BUILD.gn`、`ddk/v1_2/BUILD.gn` |
| `module_name` | HDI 模块名 | 各 `BUILD.gn`（`usbd`/`usb_ddk`/`serial`/`scsi_ddk`/`usb_serial_ddk`/`usbfn_mtp`） |
| `LOG_DOMAIN` | 日志域 | `0xD002518`（除 scsi_ddk/usb_serial_ddk 外均有） |
| `ohos_lite` | lite 系统条件编译 | v1_0-v2_1 的 `BUILD.gn` |

## 3. 约束边界

### 架构不变量

- **`.idl` 文件是唯一真相源**：不要手改 `hdi-gen` 生成的 proxy/stub/headers
- **版本号即 ABI 契约**：跨版本交互必须通过版本化库显式选择
- **v2_0 是 self-contained 重构版本**：不依赖 v1_x，重新定义全部类型和回调，将单一 `IUsbInterface` 拆分为三个接口
- **继承链通过 proxy_deps/stub_deps 实现**：v1_1 依赖 v1_0，v1_2 依赖 v1_0+v1_1，v2_1 依赖 v2_0，ddk/v1_1 依赖 v1_0，ddk/v1_2 依赖 v1_0+v1_1
- **innerapi_tags 仅在 v2_0/v2_1 声明**：`chipsetsdk_sp` 标签影响芯片 SDK 兼容性
- **no_other_header_files 在 v2_1 和 ddk/v1_2 启用**：限制头文件导出范围
- **ohos_lite 条件分支仅存在于主 USB 接口（v1_0-v2_1）**：ddk/serial/scsi_ddk/usb_serial_ddk/gadget 无此分支
- **serial 使用 passthrough 模式**：与 ddk/scsi_ddk/usb_serial_ddk/gadget 的 `ipc` 模式不同
- **本仓不含实现**：接口实现由 `drivers/peripheral/usb` 提供

### 禁止事项

- **不要直接编辑 `hdi-gen` 生成的 proxy/stub 代码**，应修改 `.idl` 源文件
- **不要删除或重命名已发布的 IDL 方法**：IDL 方法签名是 ABI 契约
- **不要复用已有 IDL 方法的码值**：新增接口必须追加新方法
- **不要修改 `bundle.json` 的 `sub_component` 而不同步 `inner_kits`**（当前 10 个 sub_component / 35 个 inner_kits）
- **不要在 v1_x 链中跳版本继承**：v1_2 必须同时声明 v1_0+v1_1 的 proxy_deps/stub_deps
- **不要在 v2_0 中引用 v1_x 的类型**：v2_0 是 self-contained，类型独立定义
- **不要移除 v2_0/v2_1 的 `innerapi_tags`**：`chipsetsdk_sp` 影响芯片 SDK 兼容性
- **不要在 ddk/serial/scsi_ddk/usb_serial_ddk/gadget 的 BUILD.gn 中添加 ohos_lite 条件分支**：这些模块无此分支设计
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审（当前依赖：`c_utils`/`ipc`/`hdf_core`/`hilog`）
- **不要在本仓中添加实现代码**：本仓是纯接口定义仓
- **不要修改授权接口的安全语义**：`v2_0` 的 `UsbDeviceAuthorize`/`UsbInterfaceAuthorize`（@since 6.0）涉及设备/接口授权状态，修改签名或参数语义需安全评审
- **不要降低 DFX 能力**：`LOG_DOMAIN=0xD002518`（除 scsi_ddk/usb_serial_ddk 外）是日志归属标识，不要移除或篡改
- **不要执行破坏性设备命令**：本仓仅定义 IDL 契约，不要在修改 IDL 时模拟或执行实际 USB 设备操作（ResetDevice/ClearHalt 等）

### Ask-before 规则

| 操作 | 确认内容 | 确认对象 |
|------|----------|----------|
| 修改 `bundle.json` 的 `sub_component` | `inner_kits` 是否同步更新（35 条） | 项目负责人 |
| 新增/删除 IDL 方法 | 是否影响 ABI 契约（继承链） | 接口评审团队 |
| 新增版本目录 | 版本号与 `bundle.json` 一致 + proxy_deps/stub_deps 声明 | 项目负责人 |
| 修改 `innerapi_tags` | 是否影响芯片 SDK 兼容性（`chipsetsdk_sp`） | 芯片适配团队 |
| 修改 `no_other_header_files` | 是否影响头文件导出范围 | 项目负责人 |
| 修改构建模式（`mode`） | passthrough↔ipc 切换影响 IPC 行为 | 构建团队 |
| 修改 `proxy_deps`/`stub_deps` | 是否影响继承链完整性 | 接口评审团队 |
| 重命名 IDL 文件 | 是否影响所有引用该文件的消费者 | 所有消费方 |
| 引入新的生产依赖 | 是否经过 `bundle.json` 评审 | 依赖评审团队 |
| 修改 v2_0 接口拆分结构 | 是否影响 Device/Port/Host 三接口消费者 | 所有消费方 |
| 修改授权接口（UsbDeviceAuthorize/UsbInterfaceAuthorize） | 是否影响设备/接口授权安全语义 | 安全评审团队 |

### 子域特定约束

- **v2_0 接口拆分**：v2_0 将 v1_x 的单一 `IUsbInterface` 拆分为 `IUsbDeviceInterface`（设备/功能/授权）、`IUsbPortInterface`（端口/角色）、`IUsbHostInterface`（主机操作/传输），修改时需确认目标接口
- **v2_0 self-contained**：v2_0 的 `UsbTypes.idl` 独立定义全部类型（与 v1_0 的 `UsbTypes.idl` 字段相同但独立），不通过 import 引用 v1_x
- **v2_0 PortInfo 扩展**：v2_0 的 `PortInfo` 比 v1_0 多 `supportedModes` 字段，v2_0 新增 `UsbPortStatus`/`UsbPort` 类型
- **deprecated 方法保留**：v1_0 的 `GetFileDescriptor`/`BulkTransferRead`/`ControlTransferRead` 标记 `@deprecated since 5.0`，v2_0 的 `GetFileDescriptor`/`BulkTransferRead`/`ControlTransferRead` 标记 `@deprecated since 5.1`，不可删除
- **v2_0 UsbDeviceAuthorize/UsbInterfaceAuthorize**：@since 6.0，是 v2_0 中后期新增的设备/接口授权能力
- **ddk/v1_0 的 mode = "ipc"**：与主 USB 接口的默认模式不同，DDK 强制 IPC
- **serial/v1_0 的 mode = "passthrough"**：串口强制直通模式，无 IPC 开销
- **scsi_ddk/usb_serial_ddk 无 LOG_DOMAIN**：这两个模块的 `BUILD.gn` 未设置 `LOG_DOMAIN`
- **usb_serial_ddk 的 syscap**：声明 `@syscap SystemCapability.Driver.UsbSerial.Extension`
- **gadget/mtp 的 module_name**：`usbfn_mtp`（注意不是 `mtp`）

### Agent 失败模式

| 失败模式 | 描述 | 预防 |
|----------|------|------|
| 版本链混淆 | 在 v1_x 中引入 v2_x 特有功能，或在 v2_0 中引用 v1_x 类型 | 确认 v2_0 是 self-contained，v1_x 和 v2_x 是两条独立链 |
| 继承链遗漏 | 新增 v1_3 但未声明 v1_0+v1_1+v1_2 的 proxy_deps/stub_deps | 参考版本依赖链表，声明所有前序版本依赖 |
| 接口拆分混淆 | 在 v2_0 中修改 `IUsbHostInterface` 但误操作 `IUsbDeviceInterface` | 确认 v2_0 三接口职责：Device=设备/功能/授权，Port=端口/角色，Host=主机操作/传输 |
| inner_kits 不同步 | 修改 sub_component 但未同步 35 条 inner_kits | 修改后必须验证 `bundle.json` 字段一致性 |
| 构建模式误改 | 将 serial 的 passthrough 改为 ipc 或反之 | 构建模式是 ABI 契约，不可随意修改 |
| ohos_lite 误加 | 在 ddk/serial 等模块添加 ohos_lite 条件分支 | 仅主 USB 接口（v1_0-v2_1）有此分支设计 |
| innerapi_tags 遗漏 | 修改 v2_0/v2_1 但未确认 `chipsetsdk_sp` 影响 | 修改前确认 innerapi_tags 影响芯片 SDK 兼容性 |
| 跨层同步遗漏 | 修改 IDL 但未同步 `drivers/peripheral/usb` 实现 | 修改 IDL 后必须验证外围仓实现 |
| 码值复用 | 复用已有 IDL 方法的码值 | 新增接口必须追加新方法 |
| 手改生成代码 | 直接修改 `hdi-gen` 生成的 proxy/stub/headers | 始终修改 `.idl` 源文件后由构建系统重生成 |

## 4. 验证闭环

### 构建命令

| 子模块 | 构建命令 | 单独构建目标 |
|--------|----------|-------------|
| 全量 USB | `./build.sh --product-name rk3568 --build-target drivers_interface_usb` | — |
| 主 USB v1_0 | 同上 | `//drivers/interface/usb/v1_0:usb_idl_target` |
| 主 USB v1_1 | 同上 | `//drivers/interface/usb/v1_1:usb_idl_target` |
| 主 USB v1_2 | 同上 | `//drivers/interface/usb/v1_2:usb_idl_target` |
| 主 USB v2_0 | 同上 | `//drivers/interface/usb/v2_0:usb_idl_target` |
| 主 USB v2_1 | 同上 | `//drivers/interface/usb/v2_1:usb_idl_target` |
| USB DDK v1_0 | 同上 | `//drivers/interface/usb/ddk/v1_0:usb_ddk_idl_target` |
| USB DDK v1_1 | 同上 | `//drivers/interface/usb/ddk/v1_1:usb_ddk_idl_target` |
| USB DDK v1_2 | 同上 | `//drivers/interface/usb/ddk/v1_2:usb_ddk_idl_target` |
| Serial | 同上 | `//drivers/interface/usb/serial/v1_0:serial_idl_target` |
| SCSI DDK（仅 inner_kits，不在 sub_component 中） | 同上 | `//drivers/interface/usb/scsi_ddk/v1_0:scsi_ddk_idl_target` |
| USB Serial DDK（仅 inner_kits，不在 sub_component 中） | 同上 | `//drivers/interface/usb/usb_serial_ddk/v1_0:usb_serial_ddk_idl_target` |
| MTP Gadget | 同上 | `//drivers/interface/usb/gadget/mtp/v1_0:usbfn_mtp_idl_target` |

### 任务特定验证

| 任务类型 | 验证命令 | 验证目标 |
|----------|----------|----------|
| 新增/修改 IDL 方法 | `./build.sh --product-name rk3568 --build-target drivers_interface_usb --lint` | IDL 生成一致性 |
| 新增版本目录 | `./build.sh --product-name rk3568 --build-target drivers_interface_usb` | 构建通过，版本号一致 |
| inner_kits 变更 | 检查 `bundle.json` 中 sub_component（10 条）和 inner_kits（35 条） | 字段一致性 |
| innerapi_tags 变更 | 检查 `v2_0/BUILD.gn` 或 `v2_1/BUILD.gn` 的 innerapi_tags 字段 | 芯片 SDK 兼容性 |
| 构建模式变更 | `gn gen --check` + 构建 | passthrough/ipc 模式正确性 |
| proxy_deps/stub_deps 变更 | `gn gen --check` + 构建 | 继承链完整性 |
| ABI 兼容性变更 | `gn gen --check && hdi-gen --check` | 配置正确性 |
| IDL 方法签名变更 | 对比修改前后的 `.idl` 方法签名（参数类型/方向/顺序）+ 检查 `drivers/peripheral/usb` 实现是否匹配 | 跨层 IDL-实现签名一致性 |

### 通用验证步骤

```bash
# 构建 USB 接口组件（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target drivers_interface_usb

# 静态分析（Lint）
./build.sh --product-name rk3568 --build-target drivers_interface_usb --lint
gn gen --check
hdi-gen --check

# 验证 inner_kits 一致性（10 sub_component / 35 inner_kits）
python3 -c "import json; d=json.load(open('drivers/interface/usb/bundle.json')); b=d['component']['build']; print('sub_component:', len(b['sub_component'])); print('inner_kits:', len(b['inner_kits']))"  # 预期输出：sub_component: 10, inner_kits: 35

# 验证 innerapi_tags（仅 v2_0/v2_1）
# 检查 v2_0/BUILD.gn 和 v2_1/BUILD.gn 中 innerapi_tags = ["chipsetsdk_sp"]

# 验证版本依赖链
# v1_x: v1_0(无deps) -> v1_1(deps v1_0) -> v1_2(deps v1_0+v1_1)
# v2_x: v2_0(无deps) -> v2_1(deps v2_0)
# ddk:  v1_0(无deps) -> v1_1(deps v1_0) -> v1_2(deps v1_0+v1_1)

# IDL 签名对比（修改前后）
# 1. 记录修改前的 .idl 方法签名（参数类型/方向[in,out]/顺序/返回值）
# 2. 修改后重新读取，逐方法比对签名一致性
# 3. 交叉检查 drivers/peripheral/usb 实现端是否同步更新
# 示例：grep -n "methodName" drivers/interface/usb/v*/I*.idl
```

### Done 定义

任务完成的判定标准：
1. 构建命令执行成功（`./build.sh --product-name rk3568 --build-target drivers_interface_usb` 返回 0）
2. 静态分析通过（`--lint` 返回 0，`gn gen --check` 通过，`hdi-gen --check` 通过）
3. `bundle.json` 中 `sub_component`（10 条）和 `inner_kits`（35 条）同步更新
4. `innerapi_tags` 影响范围已确认（如涉及 v2_0/v2_1）
5. 版本依赖链完整（proxy_deps/stub_deps 声明正确）
6. 跨层同步已验证（IDL 与外围仓 `drivers/peripheral/usb` 实现匹配，IDL 签名对比通过）
7. 最终报告已输出（包含文件清单、验证结果、确认事项）

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 目标子模块及版本及是否为活跃构建
4. 是否触发跨层同步修改（IDL 重生成 / 外围仓 `drivers/peripheral/usb` / `bundle.json`）
5. 是否影响继承链（proxy_deps/stub_deps）或 ABI 契约
6. 是否影响 `innerapi_tags`（`chipsetsdk_sp`）或 `no_other_header_files`
7. 是否影响构建模式（passthrough/ipc）
8. 所有验证命令的执行结果（构建/测试/lint/静态分析）

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样。涉及外围仓 `drivers/peripheral/usb` 的变更，必须人工复核 IDL 与实现的匹配性并说明无法在沙箱验证的限制。
