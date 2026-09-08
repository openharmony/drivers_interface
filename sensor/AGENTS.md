# AGENTS.md - OpenHarmony Sensor HDI 接口定义（drivers_interface_sensor）

## 1. 代码地图

本仓库定义 OpenHarmony Sensor HDI 接口（`@ohos/drivers_interface_sensor`，版本 4.1），subsystem `hdf`，part `drivers_interface_sensor`。核心职责是定义多传感器数据采集的 IDL 契约（50+ 传感器类型，含医疗传感器类型 ID 128-160）、传感器热插拔检测、以及传感器转换（convert）适配层。由 `hdi-gen` 工具自动生成 proxy/stub 代码。**本仓只维护 `.idl` 源文件，不包含任何实现代码**——实现由 `drivers/peripheral/sensor` 提供，消费方为 sensorservice。最重要的架构边界是**v3_0 是含完整回调（含热插拔回调 `ISensorPlugCallback`）的基线版本，v3_1 是最新主接口版本但不含回调（通过 `proxy_deps`/`stub_deps` 复用 v3_0 的回调），convert/v1_0 提供独立的传感器转换适配层**。

### 非本项目维护的目录

以下内容属于其他团队或生成产物，不属于本项目维护范围，修改时请跳过：

- 任何由 `hdi-gen` 生成的 proxy/stub `.h/.cpp` 文件（构建产物，不在源码树中）
- `drivers/peripheral/sensor/`：Sensor HDI 实现端（由驱动团队维护）
- sensorservice 中消费 Sensor 接口的代码（由上层服务团队维护）

### 嵌套指引

本仓目前无嵌套 AGENTS.md / CLAUDE.md / rules / skills 文件。仓库含 6 个版本目录 + 1 个 convert 子目录，版本关系较复杂；以下子目录可按需新建嵌套 AGENTS.md 以提供更具体的指引：

- `convert/v1_0/`：传感器转换适配层（独立 IDL：`ISensorConvertInterfaces.idl` + `ISensorConvertTypes.idl`），与主传感器接口解耦，可新建 `convert/AGENTS.md` 聚焦转换层契约与向后兼容性
- `v3_0/` 与 `v3_1/`：当前活跃版本，版本间复用关系是核心约束，可新建嵌套 AGENTS.md 聚焦 v3_0→v3_1 的回调复用机制

### 关键区域

- `v1_0/`：Sensor HDI 1.0 接口（**legacy，不在活跃构建中**），含 `ISensorInterface.idl` + `ISensorCallback.idl` + `SensorTypes.idl`
- `v1_1/`：Sensor HDI 1.1 接口（**legacy，不在活跃构建中**），含 `ISensorInterface.idl` + `ISensorCallback.idl` + `SensorTypes.idl`
- `v2_0/`：Sensor HDI 2.0 接口（**legacy，不在活跃构建中**），含 `ISensorInterface.idl` + `ISensorCallback.idl` + `SensorTypes.idl`
- `v2_1/`：Sensor HDI 2.1 接口（**legacy，不在活跃构建中**），含 `ISensorInterface.idl` + `ISensorCallback.idl` + `SensorTypes.idl`
- `v3_0/`：Sensor HDI 3.0 接口（**活跃构建**），含 4 个 IDL 文件 + `BUILD.gn`：
  - `ISensorInterface.idl`：主接口
  - `ISensorCallback.idl`：传感器数据上报回调
  - `ISensorPlugCallback.idl`：**热插拔回调**（v3_0 新增，用于热插拔传感器检测）
  - `SensorTypes.idl`：传感器类型定义
  - `BUILD.gn`：`hdi("sensor")` 模板，`module_name = "sensor_service"`，**`innerapi_tags = ["chipsetsdk", "platformsdk_indirect"]`**（暴露为内部平台 API），`ohos_lite` 条件分支（lite 系统下 group 为空），`defines = [ "LOG_DOMAIN=0xD002516" ]`，无 `proxy_deps`/`stub_deps`（独立基线版本）
- `v3_1/`：Sensor HDI 3.1 接口（**活跃构建，最新版本**），含 2 个 IDL 文件 + `BUILD.gn`：
  - `ISensorInterface.idl`：扩展后的主接口
  - `SensorTypes.idl`：扩展后的类型定义
  - **无 `ISensorCallback.idl` 和 `ISensorPlugCallback.idl`**：v3_1 复用 v3_0 的回调接口
  - `BUILD.gn`：`hdi("sensor")` 模板，`innerapi_tags = ["chipsetsdk", "platformsdk_indirect"]`，**`proxy_deps = [ "../v3_0:libsensor_proxy_3.0" ]` + `stub_deps = [ "../v3_0:libsensor_stub_3.0" ]`** + `no_other_header_files = true`——这是版本兼容性的关键约束
- `convert/v1_0/`：传感器转换适配层 IDL（**活跃构建**），含 2 个 IDL 文件 + `BUILD.gn`：
  - `ISensorConvertInterfaces.idl`：转换接口
  - `ISensorConvertTypes.idl`：转换类型定义
  - `BUILD.gn`：`hdi("sensor_convert")` 模板，独立的 `sensor_convert_idl_target`
- `bundle.json`：组件清单，`adapted_system_type: ["standard", "small"]`，依赖 `ipc` / `hdf_core` / `hilog` / `c_utils`，构建 `v3_0:sensor_idl_target` + `v3_1:sensor_idl_target` + `convert/v1_0:sensor_convert_idl_target`，导出 7 个 inner_kits（v3_0 3 个 + v3_1 2 个 + convert/v1_0 2 个）

### Where to look

| 任务类型 | 先看哪里 |
|---|---|
| 新增/修改传感器类型 | `v3_0/SensorTypes.idl` + `v3_1/SensorTypes.idl` -> 评估医疗传感器（类型 ID 128-160）和 `DeviceSensorInfo`（多传感器设备感知）影响 |
| 新增/修改主接口方法 | `v3_0/ISensorInterface.idl` 或 `v3_1/ISensorInterface.idl` -> 重新生成 proxy/stub -> 同步实现端 `drivers/peripheral/sensor` |
| 数据上报回调变更 | `v3_0/ISensorCallback.idl`（**v3_1 复用此回调**）-> 评估 v3_0 和 v3_1 双版本影响 |
| 热插拔回调变更 | `v3_0/ISensorPlugCallback.idl`（**v3_0 专属，v3_1 复用**）-> 评估热插拔检测影响 |
| 传感器转换适配 | `convert/v1_0/ISensorConvertInterfaces.idl` + `convert/v1_0/ISensorConvertTypes.idl`（独立适配层，向后兼容）|
| 多设备多传感器支持 | `v3_0/SensorTypes.idl` 或 `v3_1/SensorTypes.idl` 中的 `DeviceSensorInfo` -> 评估设备感知逻辑 |
| 医疗传感器支持 | `SensorTypes.idl` 中类型 ID 128-160 的定义 -> 评估医疗传感器权限和合规性 |
| 版本兼容性评估 | `v3_1/BUILD.gn`（`proxy_deps`/`stub_deps` 指向 v3_0）+ `bundle.json`（v3_0/v3_1/convert 同时构建）|
| 构建配置 | `bundle.json`（依赖、inner_kits）+ `v3_0/BUILD.gn`（`ohos_lite` 条件、`innerapi_tags`）+ `v3_1/BUILD.gn` + `convert/v1_0/BUILD.gn` |
| lite 系统适配 | `v3_0/BUILD.gn` 和 `v3_1/BUILD.gn` 中的 `if (defined(ohos_lite))` 分支（lite 系统下 proxy 为空 group）|

### 架构分层

```
IDL 源文件（本仓维护）
  ├─ v1_0/ (legacy，不构建)
  ├─ v1_1/ (legacy，不构建)
  ├─ v2_0/ (legacy，不构建)
  ├─ v2_1/ (legacy，不构建)
  │
  ├─ v3_0/ (活跃基线版本，含完整回调)
  │    ├─ ISensorInterface.idl    (主接口)
  │    ├─ ISensorCallback.idl     (数据上报回调，v3_1 复用)
  │    ├─ ISensorPlugCallback.idl (热插拔回调，v3_0 新增，v3_1 复用)
  │    └─ SensorTypes.idl         (类型定义，含 DeviceSensorInfo)
  │        innerapi_tags = ["chipsetsdk", "platformsdk_indirect"]
  │        ohos_lite 条件分支（lite 下 proxy 为空）
  │        ↓ hdi-gen 生成
  │    libsensor_proxy_3.0 + libsensor_stub_3.0 + sensor_idl_headers
  │
  ├─ v3_1/ (活跃最新版本，proxy/stub 依赖 v3_0)
  │    ├─ ISensorInterface.idl (扩展主接口)
  │    └─ SensorTypes.idl      (扩展类型定义)
  │        proxy_deps = ["../v3_0:libsensor_proxy_3.0"]
  │        stub_deps  = ["../v3_0:libsensor_stub_3.0"]
  │        no_other_header_files = true
  │        innerapi_tags = ["chipsetsdk", "platformsdk_indirect"]
  │        ↓ hdi-gen 生成
  │    libsensor_proxy_3.1 + libsensor_stub_3.1
  │
  └─ convert/v1_0/ (独立的传感器转换适配层)
       ├─ ISensorConvertInterfaces.idl (转换接口)
       └─ ISensorConvertTypes.idl      (转换类型)
           ↓ hdi-gen 生成
       libsensor_convert_proxy_1.0 + sensor_convert_idl_headers_1.0
                ↓
           实现端：drivers/peripheral/sensor
                ↓
           消费端：sensorservice

命名空间：ohos.hdi.sensor.v3_0 / ohos.hdi.sensor.v3_1
```

## 2. 知识路由

在规划或编辑前，先对任务分类，读取对应的代码路径和文档。

### Task-based routing

| 任务类型 | 读取 |
|---|---|
| 新增传感器类型 | `v3_0/SensorTypes.idl` + `v3_1/SensorTypes.idl` -> 决定在 v3_0 还是 v3_1 新增 -> 评估医疗传感器（类型 ID 128-160）合规性 -> 同步实现端 |
| 修改主接口 | `v3_0/ISensorInterface.idl` 或 `v3_1/ISensorInterface.idl` -> 评估版本影响 -> 同步实现端 `drivers/peripheral/sensor` |
| 数据上报回调变更 | `v3_0/ISensorCallback.idl`（v3_1 复用）-> **必须评估 v3_0 和 v3_1 双版本影响** -> 同步实现端 + 消费端 sensorservice |
| 热插拔回调变更 | `v3_0/ISensorPlugCallback.idl`（v3_0 专属，v3_1 复用）-> 评估热插拔检测影响 -> 同步实现端 |
| 传感器转换适配 | `convert/v1_0/ISensorConvertInterfaces.idl` + `convert/v1_0/ISensorConvertTypes.idl`（独立适配层，向后兼容）|
| 多设备多传感器支持 | `SensorTypes.idl` 中 `DeviceSensorInfo` -> 评估设备感知逻辑 -> 同步实现端 |
| 医疗传感器支持 | `SensorTypes.idl` 中类型 ID 128-160 -> 评估权限和合规性 -> 同步 sensorservice 权限校验 |
| 构建配置变更 | `bundle.json`（依赖、inner_kits 列表）+ `v3_0/BUILD.gn` + `v3_1/BUILD.gn` + `convert/v1_0/BUILD.gn` |
| lite 系统适配 | `v3_0/BUILD.gn` + `v3_1/BUILD.gn` 中 `if (defined(ohos_lite))` 分支 |
| 版本兼容性评估 | `v3_1/BUILD.gn` 中 `proxy_deps`/`stub_deps` 指向 v3_0 + `bundle.json` 三目标同时构建 |
| innerapi 标签变更 | `v3_0/BUILD.gn` + `v3_1/BUILD.gn` 中 `innerapi_tags = ["chipsetsdk", "platformsdk_indirect"]` |

### Path-based routing

| 修改路径 | 需了解的上下文 |
|---|---|
| `v3_0/ISensorInterface.idl` | 基线主接口，修改后构建系统生成 `libsensor_proxy_3.0`/`libsensor_stub_3.0`，**不要直接改生成产物**；v3_1 通过 `proxy_deps`/`stub_deps` 依赖此版本，变更会传递到 v3_1 |
| `v3_0/ISensorCallback.idl` | 数据上报回调，**v3_1 复用此回调**（v3_1 目录无 `ISensorCallback.idl`），修改影响 v3_0 和 v3_1 双版本 |
| `v3_0/ISensorPlugCallback.idl` | **v3_0 新增的热插拔回调**，v3_1 复用，用于热插拔传感器检测，修改影响热插拔功能 |
| `v3_0/SensorTypes.idl` | 基线类型定义，`DeviceSensorInfo` 支持多传感器设备感知，传感器类型枚举值顺序是 ABI 一部分，医疗传感器类型 ID 128-160 |
| `v3_1/ISensorInterface.idl` | 扩展主接口，修改后构建系统生成 `libsensor_proxy_3.1`/`libsensor_stub_3.1`，依赖 v3_0 的 proxy/stub |
| `v3_1/SensorTypes.idl` | 扩展类型定义，`no_other_header_files = true` 意味着不导出额外头文件 |
| `v3_1/BUILD.gn` | `proxy_deps`/`stub_deps` 指向 v3_0 + `no_other_header_files = true`，是版本兼容性的关键约束 |
| `convert/v1_0/` | 独立的传感器转换适配层，与主传感器接口解耦，提供向后兼容的转换能力 |
| `v3_0/BUILD.gn` | `ohos_lite` 条件分支（lite 系统下 proxy 为空 group）+ `innerapi_tags` + `defines = [ "LOG_DOMAIN=0xD002516" ]` |
| `bundle.json` | 三目标 `sub_component`（v3_0 + v3_1 + convert/v1_0）+ 7 个 inner_kits，新增版本需在此注册 |
| `v1_0/` / `v1_1/` / `v2_0/` / `v2_1/` | **legacy 版本，不在活跃构建中**，修改无构建影响但可能影响历史兼容性分析 |

### Vocabulary-based routing

当任务、issue、日志、API 名称中出现以下术语时，先理解其含义和风险再动手：

| 术语 | 含义与风险 | 读取 |
|---|---|---|
| Sensor / sensor_service | 传感器 HDI，`module_name = "sensor_service"` | 本文件 + `v3_0/BUILD.gn` |
| ISensorInterface | 传感器主接口，v3_0 和 v3_1 各有版本 | `v3_0/ISensorInterface.idl` + `v3_1/ISensorInterface.idl` |
| ISensorCallback | 传感器数据上报回调，**仅在 v3_0 定义**，v3_1 复用 | `v3_0/ISensorCallback.idl` |
| ISensorPlugCallback | **热插拔回调**（v3_0 新增），用于热插拔传感器检测，v3_1 复用 | `v3_0/ISensorPlugCallback.idl` |
| DeviceSensorInfo | 设备传感器信息，支持多传感器设备感知（deviceId + sensorId 等）| `v3_0/SensorTypes.idl` 或 `v3_1/SensorTypes.idl` |
| 医疗传感器 | 类型 ID 128-160 的传感器，涉及医疗合规性和权限校验 | `SensorTypes.idl` 中类型 ID 128-160 定义 |
| SensorConvert | 传感器转换适配层，独立于主接口 | `convert/v1_0/ISensorConvertInterfaces.idl` |
| hdi-gen | HDL IDL 编译器，将 `.idl` 编译为 proxy/stub C++ 代码 | 构建系统 `//build/config/components/hdi/hdi.gni` |
| hdi() | GN 模板，用于声明 HDL 接口模块 | `v3_0/BUILD.gn` + `v3_1/BUILD.gn` + `convert/v1_0/BUILD.gn` |
| proxy_deps / stub_deps | v3_1 BUILD.gn 中指向 v3_0 的依赖声明，是版本复用机制的核心 | `v3_1/BUILD.gn` |
| no_other_header_files | v3_1 BUILD.gn 中的标志，表示不导出额外头文件 | `v3_1/BUILD.gn` |
| innerapi_tags | 内部 API 标签，v3_0 和 v3_1 均为 `["chipsetsdk", "platformsdk_indirect"]`，暴露为芯片 SDK 和间接平台 SDK | `v3_0/BUILD.gn` + `v3_1/BUILD.gn` |
| chipsetsdk | 芯片 SDK 标签，表示该接口对芯片厂商暴露 | `v3_0/BUILD.gn` |
| platformsdk_indirect | 间接平台 SDK 标签，表示该接口间接对平台应用暴露 | `v3_0/BUILD.gn` |
| libsensor_proxy_3.0 / libsensor_stub_3.0 | v3_0 生成的 proxy/stub 库 | 构建产物 |
| libsensor_proxy_3.1 / libsensor_stub_3.1 | v3_1 生成的 proxy/stub 库（依赖 v3_0 的 proxy/stub）| 构建产物 |
| libsensor_convert_proxy_1.0 | 转换层生成的 proxy 库 | 构建产物 |
| ohos_lite | lite 系统（small/mini）条件编译标志，lite 下 v3_0/v3_1 proxy 为空 group | `v3_0/BUILD.gn` + `v3_1/BUILD.gn` |
| LOG_DOMAIN=0xD002516 | 传感器 HDI 的日志域定义 | `v3_0/BUILD.gn` + `v3_1/BUILD.gn` |
| ohos.hdi.sensor.v3_0 / ohos.hdi.sensor.v3_1 | IDL 包命名空间 | `SensorTypes.idl` 顶部 `package` 声明 |

在计划阶段，必须声明：
- **任务分类**（如：新增传感器类型 / 修改主接口 / 回调接口变更 / 热插拔回调变更 / 转换适配层变更 / 构建配置变更）
- **已读取的代码路径和文档**（具体到文件路径）
- **发现的约束**（架构不变量、禁止事项、版本兼容性约束、innerapi 约束、跨仓依赖）
- **是否需要同步修改其他层**：
  - `.idl` 变更 -> 由构建系统重生成 proxy/stub + 同步实现端 `drivers/peripheral/sensor` + 消费端 sensorservice
  - `ISensorCallback.idl` / `ISensorPlugCallback.idl` 变更 -> 必须同步评估 v3_0 和 v3_1 双版本影响
  - `convert/v1_0/` 变更 -> 评估转换适配层对向后兼容性的影响
  - `bundle.json` 变更 -> 同步 `inner_kits` 列表
  - 医疗传感器类型变更 -> 评估权限校验和合规性

## 3. 约束边界

### 架构不变量

- **`.idl` 文件是唯一真相源**：所有接口和类型定义必须通过 `.idl` 文件表达，proxy/stub 由 `hdi-gen` 自动生成
- **v3_1 复用 v3_0 的回调接口**：`v3_1/` 目录不含 `ISensorCallback.idl` 和 `ISensorPlugCallback.idl`，通过 `proxy_deps`/`stub_deps` 指向 v3_0 的 proxy/stub 实现复用
- **v3_0 是含完整回调的基线版本**：v3_0 同时包含 `ISensorCallback`（数据上报）和 `ISensorPlugCallback`（热插拔检测），v3_1 在此基础上扩展主接口但不重复定义回调
- **v3_0 和 v3_1 同时构建且均为 innerapi**：两个版本都在 `bundle.json` 的 `sub_component` 中注册，均标记 `innerapi_tags = ["chipsetsdk", "platformsdk_indirect"]`，暴露为芯片 SDK 和间接平台 SDK
- **convert/v1_0 是独立的转换适配层**：与主传感器接口解耦，提供向后兼容的转换能力，独立构建为 `sensor_convert_idl_target`
- **IDL 类型顺序是 ABI 一部分**：传感器类型枚举值顺序、数据结构字段顺序一旦发布即固定，新增必须追加
- **本仓不含实现**：接口实现由 `drivers/peripheral/sensor` 提供，本仓仅定义契约
- **ohos_lite 条件编译**：lite 系统（small/mini）下 v3_0 和 v3_1 的 proxy 为空 group，仅 standard 系统生成完整 proxy/stub

### 禁止事项

- **不要直接编辑 `hdi-gen` 生成的 proxy/stub `.h/.cpp` 代码**，应修改 `.idl` 源文件后由构建系统重新生成
- **不要在 v3_1 目录中新增 `ISensorCallback.idl` 或 `ISensorPlugCallback.idl`**：v3_1 通过 `proxy_deps`/`stub_deps` 复用 v3_0 的回调，新增会破坏复用机制
- **不要删除或修改 `v3_1/BUILD.gn` 中的 `proxy_deps`/`stub_deps`**：这是 v3_1 复用 v3_0 回调的关键声明
- **不要移除 `v3_1/BUILD.gn` 中的 `no_other_header_files = true`**：该标志限制头文件导出范围
- **不要调整传感器类型枚举值顺序**：已有枚举值的位置是 ABI 契约，新增类型必须追加
- **不要移除 `bundle.json` 中任一活跃版本的 `sub_component`**：v3_0、v3_1、convert/v1_0 同时构建是兼容性要求
- **不要移除 `innerapi_tags`**：v3_0 和 v3_1 暴露为 `chipsetsdk` / `platformsdk_indirect`，移除标签会破坏下游 SDK 依赖
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审（当前依赖：`ipc` / `hdf_core` / `hilog` / `c_utils`）
- **不要在本仓中添加实现代码**：本仓是纯接口定义仓
- **不要修改 legacy 版本（v1_0/v1_1/v2_0/v2_1）的 IDL**：这些版本不在活跃构建中，修改无构建验证且可能误导历史兼容性分析

### Ask before 规则

在执行以下操作前，必须先确认或询问：
- **修改 ISensorCallback.idl**：确认影响 v3_0 和 v3_1 双版本
- **修改 ISensorPlugCallback.idl**：确认影响热插拔检测功能
- **修改 innerapi_tags**：确认芯片厂商和平台 SDK 团队影响
- **修改 ohos_lite 条件分支**：确认 lite 系统下 proxy 为空 group 的行为
- **修改医疗传感器类型**：确认类型 ID 128-160 权限校验和合规性

### 需确认后再修改

- **`ISensorCallback.idl` 回调接口变更**（影响 v3_0 和 v3_1 双版本，需确认实现端和消费端兼容性）
- **`ISensorPlugCallback.idl` 热插拔回调变更**（影响热插拔检测功能，需确认实现端和消费端）
- **传感器类型枚举值变更**（需评估 ABI 兼容性，已发布的枚举值位置不能调整）
- **医疗传感器类型（ID 128-160）变更**（需评估医疗合规性和权限校验，确认 sensorservice 权限逻辑同步）
- **`DeviceSensorInfo` 字段变更**（需评估多设备多传感器感知兼容性）
- **新增 HDI 版本（如 v3_2）**（需确认版本规划，在 `bundle.json` 注册 `sub_component` + `inner_kits`，决定是否复用 v3_0 回调，是否需要 `innerapi_tags`）
- **`innerapi_tags` 变更**（需确认芯片厂商和平台 SDK 团队，`chipsetsdk`/`platformsdk_indirect` 标签影响下游依赖）
- **`bundle.json` 中 `inner_kits` 列表变更**（需确认下游消费者依赖关系）
- **新增外部依赖**（需确认许可证、包大小、`bundle.json` 同步）
- **`adapted_system_type` 变更**（当前 `["standard", "small"]`，新增 `mini` 需评估 lite 系统兼容性）
- **`ohos_lite` 条件分支变更**（需确认 lite 系统下 proxy 为空 group 的行为是否符合预期）

### 项目特定陷阱

- **v3_1 复用 v3_0 回调的隐性依赖**：`v3_1/BUILD.gn` 中 `proxy_deps = [ "../v3_0:libsensor_proxy_3.0" ]` 和 `stub_deps = [ "../v3_0:libsensor_stub_3.0" ]` 意味着修改 `v3_0/ISensorCallback.idl` 或 `v3_0/ISensorPlugCallback.idl` 会直接影响 v3_1 的回调行为，必须双版本验证
- **v3_1 无回调文件但依赖 v3_0 回调**：v3_1 目录只有 `ISensorInterface.idl` 和 `SensorTypes.idl`，开发者可能误以为 v3_1 无回调支持，实际上通过 `proxy_deps`/`stub_deps` 复用 v3_0 的 `ISensorCallback` 和 `ISensorPlugCallback`
- **`no_other_header_files = true` 的限制**：v3_1 设置此标志表示不导出额外头文件，新增需要导出头文件的类型时需评估此限制
- **innerapi_tags 的双重标签**：v3_0 和 v3_1 均标记 `chipsetsdk` + `platformsdk_indirect`，意味着这些接口同时对芯片厂商和间接平台应用暴露，变更影响范围广
- **ohos_lite 条件分支的空 group**：lite 系统下 v3_0 和 v3_1 的 `libsensor_proxy_3.0`/`libsensor_proxy_3.1` 是空 group（`deps = []`），lite 系统不生成完整 proxy/stub
- **convert/v1_0 的独立性**：转换适配层与主传感器接口解耦，修改主接口不一定需要修改转换层，但需评估转换层是否引用了主接口类型
- **legacy 版本的误导风险**：v1_0/v1_1/v2_0/v2_1 目录仍在源码树中但不在活跃构建中，修改这些目录的 IDL 不会被构建验证，可能引入未发现的语法错误
- **医疗传感器的合规性**：类型 ID 128-160 的医疗传感器涉及医疗合规性和权限校验，新增或修改需同步 sensorservice 的权限逻辑
- **`LOG_DOMAIN=0xD002516` 的一致性**：v3_0 和 v3_1 的 `BUILD.gn` 都使用相同的 `LOG_DOMAIN`，这是日志域约定，不能单独修改
- **`module_name = "sensor_service"` 的一致性**：v3_0 和 v3_1 的 `BUILD.gn` 都使用相同的 `module_name`，这是 HDI 服务名约定，不能单独修改

### Agent 失败模式

- **直接编辑生成产物**：Agent 可能 tempted to edit `hdi-gen` 生成的 proxy/stub `.h/.cpp` 文件以快速修复问题，而非修改 `.idl` 源文件后重新生成——这会导致生成的代码被覆盖，且无法追溯变更来源
- **忽略双版本回调影响**：Agent 可能只修改 `v3_0/ISensorCallback.idl` 或 `v3_0/ISensorPlugCallback.idl` 而忘记评估对 v3_1 的影响，因为 v3_1 通过 `proxy_deps`/`stub_deps` 复用这些回调
- **遗漏跨层同步**：Agent 可能完成 `.idl` 修改和构建验证后，忘记同步实现端 `drivers/peripheral/sensor` 和消费端 sensorservice
- **忽略 ABI 兼容性**：Agent 可能新增传感器类型枚举值或 `DeviceSensorInfo` 字段时未考虑枚举值顺序和字段顺序对 ABI 的影响
- **误改 `innerapi_tags`**：Agent 可能随意修改 `v3_0` 或 `v3_1` 的 `innerapi_tags`，破坏芯片 SDK 和平台 SDK 的依赖关系
- **修改 legacy 版本**：Agent 可能修改 v1_0/v1_1/v2_0/v2_1 目录的 IDL，这些版本不在活跃构建中，修改不会被验证且可能误导历史兼容性分析
- **忽略 `no_other_header_files` 限制**：Agent 可能在 v3_1 中新增需要导出头文件的类型，但 v3_1 设置了 `no_other_header_files = true`，会限制头文件导出
- **忽略医疗传感器合规性**：Agent 可能新增或修改类型 ID 128-160 的医疗传感器类型时，未评估医疗合规性和权限校验

## 4. 验证闭环

### 最小验证

```bash
# 构建 drivers_interface_sensor 子系统（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target drivers_interface_sensor

# 单独构建 v3_0
./build.sh --product-name rk3568 --build-target //drivers/interface/sensor/v3_0:sensor_idl_target

# 单独构建 v3_1（依赖 v3_0，会自动拉起 v3_0 构建）
./build.sh --product-name rk3568 --build-target //drivers/interface/sensor/v3_1:sensor_idl_target

# 单独构建 convert/v1_0
./build.sh --product-name rk3568 --build-target //drivers/interface/sensor/convert/v1_0:sensor_convert_idl_target

# 静态分析：验证 GN 构建配置语法
gn gen --check //drivers/interface/sensor

# 静态分析：检查 IDL 文件语法
# hdi-gen --check v3_0/*.idl v3_1/*.idl convert/v1_0/*.idl
```

### 任务特定验证

| 任务类型 | 验证命令 |
|---|---|
| `.idl` 接口变更 | `./build.sh --product-name rk3568 --build-target drivers_interface_sensor` + 验证 proxy/stub 重新生成 + 同步构建实现端 `drivers/peripheral/sensor` |
| `ISensorCallback.idl` 变更 | 构建双版本 `v3_0` + `v3_1` + 验证 v3_1 通过 `proxy_deps`/`stub_deps` 正确复用 v3_0 回调 + 同步实现端和消费端 sensorservice |
| `ISensorPlugCallback.idl` 变更 | 构建双版本 + 验证热插拔检测逻辑 + 同步实现端和消费端 |
| `SensorTypes.idl` 枚举变更 | 构建双版本 + 验证枚举值顺序未破坏 ABI + 同步 `drivers/peripheral/sensor` 和 sensorservice |
| 医疗传感器类型变更 | 构建双版本 + 验证类型 ID 128-160 范围 + 同步 sensorservice 权限校验逻辑 |
| `DeviceSensorInfo` 变更 | 构建双版本 + 验证多设备多传感器感知 + 同步实现端 |
| convert/v1_0 变更 | 构建 `convert/v1_0:sensor_convert_idl_target` + 验证转换适配层向后兼容性 |
| 构建配置变更 | `./build.sh --product-name rk3568 --build-target drivers_interface_sensor` + 验证 `bundle.json` 中 7 个 `inner_kits` 均正常导出 |
| innerapi_tags 变更 | 构建双版本 + 验证芯片 SDK 和平台 SDK 依赖未破坏 + 同步下游 SDK 团队 |
| ohos_lite 适配 | 在 lite 系统配置下构建 + 验证 v3_0/v3_1 proxy 为空 group 的行为 |
| 新增 HDI 版本 | 构建全量 `drivers_interface_sensor` + 验证新版本 `sub_component` 和 `inner_kits` 注册正确 |

### 静态分析 / Lint

```bash
# 代码格式与静态检查
./build.sh --product-name rk3568 --build-target drivers_interface_sensor --lint
gn gen --check
hdi-gen --check
```

### Done 定义

- 构建通过（`drivers_interface_sensor` 子系统 + v3_0 + v3_1 + convert/v1_0）
- `gn gen --check` 验证通过，无构建配置语法错误
- IDL 文件语法检查通过（若 hdi-gen 提供 `--check` 模式）
- proxy/stub 由 `hdi-gen` 自动重新生成，无手改生成产物
- 无新增编译警告
- 变更范围与任务要求一致，未夹带未关联的重构
- `.idl` 变更已同步实现端 `drivers/peripheral/sensor` + 消费端 sensorservice
- `ISensorCallback.idl` / `ISensorPlugCallback.idl` 变更已评估 v3_0 和 v3_1 双版本影响
- 枚举值/结构体字段顺序变更已评估 ABI 兼容性
- 医疗传感器类型变更已评估合规性和权限校验
- `bundle.json` 变更已同步 `sub_component` 和 `inner_kits` 列表
- `innerapi_tags` 变更已确认芯片厂商和平台 SDK 团队

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 是否触发跨层同步修改（实现端 `drivers/peripheral/sensor` / 消费端 sensorservice / `bundle.json`）
4. 是否影响版本兼容性（v3_0 与 v3_1 之间的复用关系，convert/v1_0 的独立性）
5. 是否影响 innerapi 暴露范围（`chipsetsdk` / `platformsdk_indirect` 标签）
6. 是否触及架构不变量或需确认事项
7. 涉及回调接口（`ISensorCallback.idl` / `ISensorPlugCallback.idl`）的变更需额外说明双版本影响验证
8. 涉及医疗传感器的变更需额外说明合规性和权限校验评估

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样，不能假称已通过。涉及 `.idl` 变更的，必须人工复核 IDL 语法和版本兼容性并说明无法在沙箱验证的限制。涉及医疗传感器的变更，必须人工复核类型 ID 范围（128-160）和权限校验逻辑。涉及构建配置的变更，必须人工复核 `gn gen --check` 的输出和 IDL 文件语法检查结果。
