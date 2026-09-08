# AGENTS.md - OpenHarmony Vibrator HDI 接口定义（drivers_interface_vibrator）

## 1. 代码地图

本仓库定义 OpenHarmony Vibrator HDI 接口（`@ohos/drivers_interface_vibrator`，版本 4.0），subsystem `hdf`，part `drivers_interface_vibrator`。核心职责是定义振动控制的 IDL 契约，涵盖基础振动、HD 触觉反馈（Haptic）、基于会话的振动（Session-based Vibration）、热插拔振动器支持。由 `hdi-gen` 工具自动生成 proxy/stub 代码。**本仓只维护 `.idl` 源文件，不包含任何实现代码**——实现由 `drivers/peripheral/vibrator` 提供，消费方为 miscservices 和音频框架（触觉同步）。最重要的架构边界是**v2_0 是当前唯一活跃构建的「超大版本」，整合了 v1_0-v1_3 的所有能力并新增 HD Haptic、Session 振动、热插拔支持，v1_0-v1_3 均为 legacy 不在活跃构建中**。

### 非本项目维护的目录

以下内容属于其他团队或生成产物，不属于本项目维护范围，修改时请跳过：

- 任何由 `hdi-gen` 生成的 proxy/stub `.h/.cpp` 文件（构建产物，不在源码树中）
- `drivers/peripheral/vibrator/`：Vibrator HDI 实现端（由驱动团队维护）
- miscservices 中消费 Vibrator 接口的代码（由上层服务团队维护）
- 音频框架中触觉同步相关代码（由音频团队维护）

### 嵌套指引

本仓目前无嵌套 AGENTS.md / CLAUDE.md / rules / skills 文件。仓库含 5 个版本目录（v1_0-v1_3 legacy + v2_0 活跃），版本演进关系是核心约束；以下子目录可按需新建嵌套 AGENTS.md 以提供更具体的指引：

- `v2_0/`：当前唯一活跃版本，含 3 个 IDL 文件和 20+ 方法，是 v1_0-v1_3 的整合+扩展，可新建 `v2_0/AGENTS.md` 聚焦 HD Haptic / Session 振动 / 热插拔三大新能力的契约细节
- `v1_0/` - `v1_3/`：legacy 版本目录，仅用于历史兼容性参考，可新建 `AGENTS.md` 说明各版本的演进关系和 legacy 状态

### 关键区域

- `v1_0/`：Vibrator HDI 1.0 接口（**legacy，不在活跃构建中**），含 `IVibratorInterface.idl` + `VibratorTypes.idl`
- `v1_1/`：Vibrator HDI 1.1 接口（**legacy，不在活跃构建中**），含 `IVibratorInterface.idl` + `VibratorTypes.idl`
- `v1_2/`：Vibrator HDI 1.2 接口（**legacy，不在活跃构建中**），含 `IVibratorInterface.idl` + `VibratorTypes.idl`
- `v1_3/`：Vibrator HDI 1.3 接口（**legacy，不在活跃构建中**），含 `IVibratorInterface.idl` + `VibratorTypes.idl`
- `v2_0/`：Vibrator HDI 2.0 接口（**活跃构建，唯一活跃版本**，`@since 5.1`，`@version 2.0`），含 3 个 IDL 文件 + `BUILD.gn`：
  - `IVibratorInterface.idl`：主接口，20+ 方法：
    - 基础振动：`StartOnce`、`Start`、`Stop`、`StartByIntensity`
    - 振动器信息：`GetVibratorInfo`、`GetDeviceVibratorInfo`、`GetVibratorIdSingle`
    - 调制与复合效果：`EnableVibratorModulation`、`EnableCompositeEffect`
    - 效果信息：`GetEffectInfo`、`IsVibratorRunning`
    - **HD Haptic（v2_0 专属）**：`PlayHapticPattern`、`GetHapticCapacity`、`GetHapticStartUpTime`
    - 波形信息：`GetAllWaveInfo`
    - **热插拔（v2_0 专属）**：`RegVibratorPlugCallback`、`UnRegVibratorPlugCallback`
    - **Session 振动（v2_0 专属，音频框架集成）**：`PlayPatternBySessionId`、`PlayPackageBySession`、`StopVibrateBySessionId`
  - `VibratorTypes.idl`：丰富类型定义：
    - `DeviceVibratorInfo`（设备感知：deviceId + vibratorId）
    - `HdfVibratorMode`、`HdfEffectType`、`HdfVibratorInfo`、`TimeEffect`、`PrimitiveEffect`
    - `CompositeEffect` union + `HdfCompositeEffect`（复合效果：时间基 + 原始基）
    - `HdfEffectInfo`、`EVENT_TYPE`、`CurvePoint`、`HapticEvent`、`HapticPaket`（HD Haptic）
    - `VibratorPackage`、`HapticCapacity`、`HdfWaveInformation`、`HdfVibratorPlugInfo`（热插拔信息）
  - `IVibratorPlugCallback.idl`：**热插拔振动器回调**（v2_0 专属，用于热插拔振动器插拔状态上报）
  - `BUILD.gn`：`hdi("vibrator")` 模板，`module_name = "vibrator_service"`，`ohos_lite` 条件分支（lite 系统下 proxy 为空 group），无 `proxy_deps`/`stub_deps`（独立版本，不复用 legacy 版本）
- `bundle.json`：组件清单，`adapted_system_type: ["standard", "small"]`，**依赖包含 `hilog_lite`（除 `hilog` 外额外依赖，是唯一依赖 hilog_lite 的接口仓）**，依赖 `ipc` / `hdf_core` / `hilog` / `hilog_lite` / `c_utils`，仅构建 `v2_0:vibrator_idl_target`，导出 3 个 inner_kits（proxy + stub + idl_headers）

### Where to look

| 任务类型 | 先看哪里 |
|---|---|
| 新增/修改振动方法 | `v2_0/IVibratorInterface.idl` -> 重新生成 proxy/stub -> 同步实现端 `drivers/peripheral/vibrator` |
| HD Haptic 相关变更 | `v2_0/IVibratorInterface.idl`（`PlayHapticPattern` / `GetHapticCapacity` / `GetHapticStartUpTime`）+ `v2_0/VibratorTypes.idl`（`HapticPaket` / `HapticEvent` / `HapticCapacity`）-> 同步实现端和 miscservices |
| Session 振动相关变更 | `v2_0/IVibratorInterface.idl`（`PlayPatternBySessionId` / `PlayPackageBySession` / `StopVibrateBySessionId`）+ `v2_0/VibratorTypes.idl`（`VibratorPackage`）-> 同步实现端和音频框架 |
| 热插拔相关变更 | `v2_0/IVibratorInterface.idl`（`RegVibratorPlugCallback` / `UnRegVibratorPlugCallback`）+ `v2_0/IVibratorPlugCallback.idl` + `v2_0/VibratorTypes.idl`（`HdfVibratorPlugInfo`）-> 同步实现端和消费端 |
| 复合效果变更 | `v2_0/VibratorTypes.idl`（`CompositeEffect` union + `HdfCompositeEffect` + `TimeEffect` + `PrimitiveEffect`）-> 同步实现端 |
| 振动器信息变更 | `v2_0/IVibratorInterface.idl`（`GetVibratorInfo` / `GetDeviceVibratorInfo` / `GetVibratorIdSingle`）+ `v2_0/VibratorTypes.idl`（`HdfVibratorInfo` / `DeviceVibratorInfo`）-> 同步实现端和消费端 |
| 设备感知变更 | `v2_0/VibratorTypes.idl`（`DeviceVibratorInfo`：deviceId + vibratorId）-> 评估多设备多振动器影响 |
| 波形信息变更 | `v2_0/IVibratorInterface.idl`（`GetAllWaveInfo`）+ `v2_0/VibratorTypes.idl`（`HdfWaveInformation`）-> 同步实现端 |
| 回调接口变更 | `v2_0/IVibratorPlugCallback.idl`（热插拔回调）-> 同步实现端和消费端 |
| 构建配置 | `bundle.json`（依赖含 `hilog_lite`、inner_kits）+ `v2_0/BUILD.gn`（`hdi()` 模板、`ohos_lite` 条件）|
| lite 系统适配 | `v2_0/BUILD.gn` 中 `if (defined(ohos_lite))` 分支（lite 系统下 proxy 为空 group）|
| 版本兼容性评估 | `bundle.json`（仅 v2_0 构建）+ legacy 版本目录（v1_0-v1_3，不在活跃构建中）|

### 架构分层

```
IDL 源文件（本仓维护）
  ├─ v1_0/ (legacy，不构建)
  ├─ v1_1/ (legacy，不构建)
  ├─ v1_2/ (legacy，不构建)
  ├─ v1_3/ (legacy，不构建)
  │
  └─ v2_0/ (活跃唯一版本，@since 5.1，整合 v1_0-v1_3 + 新增能力)
       ├─ IVibratorInterface.idl (20+ 方法)
       │    ├─ 基础振动：StartOnce / Start / Stop / StartByIntensity
       │    ├─ 振动器信息：GetVibratorInfo / GetDeviceVibratorInfo / GetVibratorIdSingle
       │    ├─ 调制与复合效果：EnableVibratorModulation / EnableCompositeEffect
       │    ├─ 效果信息：GetEffectInfo / IsVibratorRunning
       │    ├─ HD Haptic (v2_0 专属)：PlayHapticPattern / GetHapticCapacity / GetHapticStartUpTime
       │    ├─ 波形信息：GetAllWaveInfo
       │    ├─ 热插拔 (v2_0 专属)：RegVibratorPlugCallback / UnRegVibratorPlugCallback
       │    └─ Session 振动 (v2_0 专属)：PlayPatternBySessionId / PlayPackageBySession / StopVibrateBySessionId
       │
       ├─ VibratorTypes.idl (丰富类型)
       │    ├─ DeviceVibratorInfo (设备感知：deviceId + vibratorId)
       │    ├─ HdfVibratorMode / HdfEffectType / HdfVibratorInfo / TimeEffect / PrimitiveEffect
       │    ├─ CompositeEffect union + HdfCompositeEffect (复合效果)
       │    ├─ HdfEffectInfo / EVENT_TYPE / CurvePoint / HapticEvent / HapticPaket (HD Haptic)
       │    ├─ VibratorPackage / HapticCapacity / HdfWaveInformation
       │    └─ HdfVibratorPlugInfo (热插拔信息)
       │
       ├─ IVibratorPlugCallback.idl (热插拔回调，v2_0 专属)
       │
       └─ BUILD.gn
            module_name = "vibrator_service"
            ohos_lite 条件分支（lite 下 proxy 为空）
            无 proxy_deps / stub_deps（独立版本）
                ↓ hdi-gen 生成
       libvibrator_proxy_2.0 + libvibrator_stub_2.0 + vibrator_idl_headers
                ↓
           实现端：drivers/peripheral/vibrator
                ↓
           消费端：miscservices + 音频框架（触觉同步）

命名空间：ohos.hdi.vibrator.v2_0
依赖：ipc / hdf_core / hilog / hilog_lite / c_utils
       （唯一依赖 hilog_lite 的接口仓）
```

## 2. 知识路由

在规划或编辑前，先对任务分类，读取对应的代码路径和文档。

### Task-based routing

| 任务类型 | 读取 |
|---|---|
| 新增/修改振动方法 | `v2_0/IVibratorInterface.idl` -> 评估方法分类（基础/HD Haptic/Session/热插拔）-> 同步实现端 `drivers/peripheral/vibrator` |
| HD Haptic 变更 | `v2_0/IVibratorInterface.idl`（`PlayHapticPattern` / `GetHapticCapacity` / `GetHapticStartUpTime`）+ `v2_0/VibratorTypes.idl`（`HapticPaket` / `HapticEvent` / `HapticCapacity` / `CurvePoint` / `EVENT_TYPE`）-> 同步实现端 + miscservices |
| Session 振动变更 | `v2_0/IVibratorInterface.idl`（`PlayPatternBySessionId` / `PlayPackageBySession` / `StopVibrateBySessionId`）+ `v2_0/VibratorTypes.idl`（`VibratorPackage`）-> 同步实现端 + 音频框架（触觉同步）|
| 热插拔变更 | `v2_0/IVibratorInterface.idl`（`RegVibratorPlugCallback` / `UnRegVibratorPlugCallback`）+ `v2_0/IVibratorPlugCallback.idl` + `v2_0/VibratorTypes.idl`（`HdfVibratorPlugInfo`）-> 同步实现端 + 消费端 |
| 复合效果变更 | `v2_0/VibratorTypes.idl`（`CompositeEffect` union + `HdfCompositeEffect` + `TimeEffect` + `PrimitiveEffect`）-> 同步实现端 |
| 振动器信息变更 | `v2_0/IVibratorInterface.idl`（`GetVibratorInfo` / `GetDeviceVibratorInfo` / `GetVibratorIdSingle`）+ `v2_0/VibratorTypes.idl`（`HdfVibratorInfo` / `DeviceVibratorInfo`）-> 同步实现端 + 消费端 |
| 设备感知变更 | `v2_0/VibratorTypes.idl`（`DeviceVibratorInfo`：deviceId + vibratorId）-> 评估多设备多振动器影响 |
| 回调接口变更 | `v2_0/IVibratorPlugCallback.idl`（热插拔回调）-> 同步实现端和消费端 |
| 构建配置变更 | `bundle.json`（依赖含 `hilog_lite`、inner_kits 列表）+ `v2_0/BUILD.gn`（`hdi()` 模板、`ohos_lite` 条件）|
| lite 系统适配 | `v2_0/BUILD.gn` 中 `if (defined(ohos_lite))` 分支（lite 系统下 proxy 为空 group）|
| 版本兼容性评估 | `bundle.json`（仅 v2_0 构建）+ legacy 版本目录（v1_0-v1_3）|

### Path-based routing

| 修改路径 | 需了解的上下文 |
|---|---|
| `v2_0/IVibratorInterface.idl` | 唯一活跃主接口，20+ 方法，修改后构建系统生成 `libvibrator_proxy_2.0`/`libvibrator_stub_2.0`，**不要直接改生成产物**；方法 @since 标注跨越 2.2-6.0，反映 v1_0-v1_3 整合历史 |
| `v2_0/VibratorTypes.idl` | 丰富类型定义，含 HD Haptic（`HapticPaket`/`HapticEvent`）、Session（`VibratorPackage`）、热插拔（`HdfVibratorPlugInfo`）、复合效果（`CompositeEffect` union）等，类型顺序是 ABI 一部分 |
| `v2_0/IVibratorPlugCallback.idl` | **v2_0 专属热插拔回调**，用于热插拔振动器插拔状态上报，修改影响热插拔功能 |
| `v2_0/BUILD.gn` | `hdi("vibrator")` 模板，`module_name = "vibrator_service"`，`ohos_lite` 条件分支（lite 下 proxy 为空 group），无 `proxy_deps`/`stub_deps`（独立版本，不复用 legacy）|
| `bundle.json` | 仅构建 `v2_0:vibrator_idl_target`，导出 3 个 inner_kits，**依赖含 `hilog_lite`**（唯一依赖 hilog_lite 的接口仓）|
| `v1_0/` / `v1_1/` / `v1_2/` / `v1_3/` | **legacy 版本，不在活跃构建中**，修改无构建影响但可能影响历史兼容性分析；v2_0 整合了这些版本的全部能力 |

### Vocabulary-based routing

当任务、issue、日志、API 名称中出现以下术语时，先理解其含义和风险再动手：

| 术语 | 含义与风险 | 读取 |
|---|---|---|
| Vibrator / vibrator_service | 振动器 HDI，`module_name = "vibrator_service"` | 本文件 + `v2_0/BUILD.gn` |
| IVibratorInterface | 振动器主接口，v2_0 含 20+ 方法 | `v2_0/IVibratorInterface.idl` |
| IVibratorPlugCallback | **热插拔振动器回调**（v2_0 专属），用于热插拔振动器插拔状态上报 | `v2_0/IVibratorPlugCallback.idl` |
| DeviceVibratorInfo | 设备振动器信息（deviceId + vibratorId），支持多设备多振动器感知 | `v2_0/VibratorTypes.idl` |
| HapticPaket / HapticEvent | HD Haptic 数据包/事件（v2_0 专属），HD 触觉反馈核心类型 | `v2_0/VibratorTypes.idl` |
| HapticCapacity | 振动能力信息（v2_0 专属），描述电机振动能力 | `v2_0/VibratorTypes.idl` |
| VibratorPackage | 振动器数据包（v2_0 专属），用于 Session 振动 | `v2_0/VibratorTypes.idl` |
| HdfVibratorPlugInfo | 热插拔振动器信息（v2_0 专属） | `v2_0/VibratorTypes.idl` |
| CompositeEffect / HdfCompositeEffect | 复合效果 union 及其包装，支持时间基和原始基效果 | `v2_0/VibratorTypes.idl` |
| TimeEffect / PrimitiveEffect | 时间基效果 / 原始基效果（复合效果的两种模式）| `v2_0/VibratorTypes.idl` |
| HdfVibratorMode | 振动模式（一次性 / 周期性）| `v2_0/VibratorTypes.idl` |
| HdfEffectType / HdfEffectInfo | 预设效果类型 / 效果信息 | `v2_0/VibratorTypes.idl` |
| HdfVibratorInfo | 振动器信息（支持强度和频率设置）| `v2_0/VibratorTypes.idl` |
| HdfWaveInformation | 波形信息 | `v2_0/VibratorTypes.idl` |
| CurvePoint / EVENT_TYPE | 曲线点 / 事件类型（HD Haptic 相关）| `v2_0/VibratorTypes.idl` |
| Session 振动 | 基于会话的振动（`PlayPatternBySessionId` / `PlayPackageBySession` / `StopVibrateBySessionId`），用于音频框架触觉同步 | `v2_0/IVibratorInterface.idl` |
| HD Haptic | HD 触觉反馈（v2_0 专属），`PlayHapticPattern` / `GetHapticCapacity` / `GetHapticStartUpTime` | `v2_0/IVibratorInterface.idl` |
| hdi-gen | HDL IDL 编译器，将 `.idl` 编译为 proxy/stub C++ 代码 | 构建系统 `//build/config/components/hdi/hdi.gni` |
| hdi() | GN 模板，用于声明 HDL 接口模块 | `v2_0/BUILD.gn` |
| libvibrator_proxy_2.0 / libvibrator_stub_2.0 | v2_0 生成的 proxy/stub 库 | 构建产物 |
| inner_kits | 内部 SDK 接口声明，本仓导出 3 个 | `bundle.json` |
| hilog_lite | lite 系统日志组件，**本仓是唯一依赖 hilog_lite 的接口仓**（除 `hilog` 外额外依赖）| `bundle.json` |
| ohos_lite | lite 系统（small/mini）条件编译标志，lite 下 v2_0 proxy 为空 group | `v2_0/BUILD.gn` |
| ohos.hdi.vibrator.v2_0 | IDL 包命名空间 | `VibratorTypes.idl` 顶部 `package` 声明 |

在计划阶段，必须声明：
- **任务分类**（如：基础振动方法变更 / HD Haptic 变更 / Session 振动变更 / 热插拔变更 / 复合效果变更 / 设备感知变更 / 构建配置变更）
- **已读取的代码路径和文档**（具体到文件路径）
- **发现的约束**（架构不变量、禁止事项、版本兼容性约束、`hilog_lite` 依赖、跨仓依赖）
- **是否需要同步修改其他层**：
  - `.idl` 变更 -> 由构建系统重生成 proxy/stub + 同步实现端 `drivers/peripheral/vibrator` + 消费端 miscservices
  - HD Haptic 变更 -> 同步 miscservices HD 触觉反馈消费逻辑
  - Session 振动变更 -> 同步音频框架触觉同步逻辑
  - 热插拔变更 -> 同步实现端和消费端的热插拔处理
  - `bundle.json` 变更 -> 同步 `inner_kits` 列表
  - `hilog_lite` 依赖变更 -> 评估 lite 系统日志兼容性

## 3. 约束边界

### 架构不变量

- **`.idl` 文件是唯一真相源**：所有接口和类型定义必须通过 `.idl` 文件表达，proxy/stub 由 `hdi-gen` 自动生成
- **v2_0 是唯一活跃构建版本**：`bundle.json` 仅注册 `v2_0:vibrator_idl_target`，v1_0-v1_3 均不在活跃构建中
- **v2_0 是 v1_0-v1_3 的整合超集**：v2_0 整合了 v1_0-v1_3 的全部能力并新增 HD Haptic、Session 振动、热插拔支持，方法 @since 标注跨越 2.2-6.0 反映整合历史
- **HD Haptic 是 v2_0 专属能力**：`HapticPaket`、`HapticEvent`、`HapticCapacity`、`PlayHapticPattern`、`GetHapticCapacity`、`GetHapticStartUpTime` 仅在 v2_0 中定义
- **Session 振动是 v2_0 专属能力**：`PlayPatternBySessionId`、`PlayPackageBySession`、`StopVibrateBySessionId` 用于音频框架触觉同步，仅在 v2_0 中定义
- **热插拔是 v2_0 专属能力**：`IVibratorPlugCallback`、`RegVibratorPlugCallback`、`UnRegVibratorPlugCallback`、`HdfVibratorPlugInfo` 仅在 v2_0 中定义
- **设备感知通过 `DeviceVibratorInfo`**：`deviceId` + `vibratorId` 支持多设备多振动器感知，大部分方法以 `DeviceVibratorInfo` 为首参数
- **IDL 类型顺序是 ABI 一部分**：`VibratorTypes.idl` 中的类型和字段顺序一旦发布即固定，新增必须追加
- **本仓不含实现**：接口实现由 `drivers/peripheral/vibrator` 提供，本仓仅定义契约
- **ohos_lite 条件编译**：lite 系统（small/mini）下 v2_0 的 proxy 为空 group，仅 standard 系统生成完整 proxy/stub

### 禁止事项

- **不要直接编辑 `hdi-gen` 生成的 proxy/stub `.h/.cpp` 代码**，应修改 `.idl` 源文件后由构建系统重新生成
- **不要在 legacy 版本（v1_0-v1_3）中新增能力**：这些版本不在活跃构建中，新增能力应在 v2_0 中进行
- **不要调整 `VibratorTypes.idl` 中的类型/字段顺序**：已有类型和字段的位置是 ABI 契约，新增必须追加
- **不要移除 `bundle.json` 中 v2_0 的 `sub_component`**：v2_0 是唯一活跃构建版本
- **不要移除 `hilog_lite` 依赖**：本仓是唯一依赖 `hilog_lite` 的接口仓，移除会破坏 lite 系统日志兼容性
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审（当前依赖：`ipc` / `hdf_core` / `hilog` / `hilog_lite` / `c_utils`）
- **不要在本仓中添加实现代码**：本仓是纯接口定义仓
- **不要修改 legacy 版本的 IDL**：v1_0-v1_3 不在活跃构建中，修改无构建验证且可能误导历史兼容性分析

### Ask before 规则

在执行以下操作前，必须先确认或询问：
- **修改 IVibratorInterface.idl 方法签名**：确认 20+ 方法兼容性影响
- **修改 IVibratorPlugCallback.idl**：确认影响热插拔功能
- **修改 HD Haptic 类型**：确认 HapticPaket/HapticEvent/HapticCapacity 兼容性
- **修改 Session 振动方法**：确认音频框架触觉同步兼容性
- **移除 hilog_lite 依赖**：本仓是唯一依赖 hilog_lite 的接口仓，移除会破坏 lite 系统

### 需确认后再修改

- **`IVibratorInterface.idl` 方法签名变更**（需评估 20+ 方法的兼容性影响，同步实现端和消费端）
- **`IVibratorPlugCallback.idl` 热插拔回调变更**（影响热插拔功能，需确认实现端和消费端）
- **HD Haptic 类型变更**（`HapticPaket` / `HapticEvent` / `HapticCapacity` / `CurvePoint` / `EVENT_TYPE`，需评估 HD 触觉反馈兼容性，同步 miscservices）
- **Session 振动方法变更**（`PlayPatternBySessionId` / `PlayPackageBySession` / `StopVibrateBySessionId`，需评估音频框架触觉同步兼容性）
- **`DeviceVibratorInfo` 字段变更**（需评估多设备多振动器感知兼容性）
- **`CompositeEffect` union 变更**（需评估复合效果（时间基 + 原始基）的兼容性）
- **新增 HDI 版本（如 v2_1）**（需确认版本规划，在 `bundle.json` 注册 `sub_component` + `inner_kits`，决定是否复用 v2_0 的回调）
- **`bundle.json` 中 `inner_kits` 列表变更**（需确认下游消费者依赖关系）
- **`hilog_lite` 依赖变更**（需评估 lite 系统日志兼容性，本仓是唯一依赖 hilog_lite 的接口仓）
- **新增外部依赖**（需确认许可证、包大小、`bundle.json` 同步）
- **`adapted_system_type` 变更**（当前 `["standard", "small"]`，新增 `mini` 需评估 lite 系统兼容性）
- **`ohos_lite` 条件分支变更**（需确认 lite 系统下 proxy 为空 group 的行为是否符合预期）

### 项目特定陷阱

- **v2_0 方法的 @since 标注跨越多个版本**：`IVibratorInterface.idl` 中方法的 `@since` 标注从 2.2 到 6.0（如 `StartOnce` @since 2.2、`PlayHapticPattern` @since 4.1、`RegVibratorPlugCallback` @since 5.1、`PlayPatternBySessionId` @since 6.0），反映 v2_0 整合了 v1_0-v1_3 的能力历史，新增方法需正确标注 @since
- **v2_0 是「超大版本」**：v2_0 整合了 v1_0-v1_3 的全部能力并新增三大能力（HD Haptic / Session 振动 / 热插拔），20+ 方法集中在一个接口中，修改时需评估对全部能力的影响
- **legacy 版本的误导风险**：v1_0-v1_3 目录仍在源码树中但不在活跃构建中，修改这些目录的 IDL 不会被构建验证，可能引入未发现的语法错误
- **`hilog_lite` 的唯一性**：本仓是唯一依赖 `hilog_lite` 的接口仓（除 `hilog` 外额外依赖），与 `drivers_interface_motion`（仅 `hilog`）、`drivers_interface_sensor`（仅 `hilog`）不同，移除 `hilog_lite` 会破坏 lite 系统日志兼容性
- **`DeviceVibratorInfo` 的默认值约定**：`deviceId` 默认值为 -1，`vibratorId` 按电机数量累加，跨方法传递时需注意默认值语义
- **Session 振动的音频框架集成**：`sessionId` 由音频框架发送，用于触觉同步，修改 Session 振动方法需同步音频框架
- **复合效果的两种模式**：`CompositeEffect` union 支持时间基（`TimeEffect`）和原始基（`PrimitiveEffect`）两种效果，修改时需同时评估两种模式
- **ohos_lite 条件分支的空 group**：lite 系统下 v2_0 的 `libvibrator_proxy_2.0` 是空 group（`deps = []`），lite 系统不生成完整 proxy/stub
- **`module_name = "vibrator_service"` 的一致性**：v2_0 的 `BUILD.gn` 使用此 `module_name`，是 HDI 服务名约定，不能修改
- **`adapted_system_type` 不含 `mini`**：本仓适配 `standard` 和 `small` 系统，不适配 `mini` 系统，与 `drivers_interface_motion`（不含 mini）一致但与 `drivers_interface_tools`（含 mini）不同

### Agent 失败模式

- **忽略 legacy 版本影响**：Agent 可能修改 v1_0-v1_3 目录的 IDL，认为这些版本不在活跃构建中就无害，但修改会引入未发现的语法错误且可能误导历史兼容性分析
- **遗漏跨层同步**：Agent 可能完成 `.idl` 修改和构建验证后，忘记同步实现端 `drivers/peripheral/vibrator`、消费端 miscservices 和音频框架
- **忽略 `hilog_lite` 依赖**：Agent 可能移除或修改 `hilog_lite` 依赖（本仓是唯一依赖 hilog_lite 的接口仓），破坏 lite 系统日志兼容性
- **忽略 HD Haptic/Session 专用类型**：Agent 可能修改 `VibratorTypes.idl` 时未区分基础类型与 HD Haptic/Session 专用类型（`HapticPaket`、`VibratorPackage` 等），导致类型归属混乱
- **修改 `@since` 标注**：Agent 可能新增方法时未正确标注 `@since` 版本号（v2_0 方法的 `@since` 跨越 2.2-6.0），导致版本追踪失效
- **忽略复合效果两种模式**：Agent 可能修改 `CompositeEffect` union 时只评估一种模式（时间基或原始基），忽略另一种模式的兼容性影响
- **忽略设备感知默认值**：Agent 可能修改 `DeviceVibratorInfo` 时未考虑 `deviceId` 默认值为 -1 和 `vibratorId` 按电机数量累加的约定
- **遗漏 Session 振动音频框架同步**：Agent 可能修改 Session 振动方法时忘记同步音频框架的触觉同步逻辑
- **热插拔回调未验证**：Agent 可能修改 `IVibratorPlugCallback` 或 `RegVibratorPlugCallback`/`UnRegVibratorPlugCallback` 时未验证热插拔回调机制的完整性

## 4. 验证闭环

### 最小验证

```bash
# 构建 drivers_interface_vibrator 子系统（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target drivers_interface_vibrator

# 单独构建 v2_0
./build.sh --product-name rk3568 --build-target //drivers/interface/vibrator/v2_0:vibrator_idl_target

# 静态分析：验证 GN 构建配置语法
gn gen --check //drivers/interface/vibrator

# 静态分析：检查 IDL 文件语法
# hdi-gen --check v2_0/*.idl
```

### 任务特定验证

| 任务类型 | 验证命令 |
|---|---|
| `.idl` 接口变更 | `./build.sh --product-name rk3568 --build-target drivers_interface_vibrator` + 验证 proxy/stub 重新生成 + 同步构建实现端 `drivers/peripheral/vibrator` |
| HD Haptic 变更 | 构建 v2_0 + 验证 `HapticPaket` / `HapticEvent` / `HapticCapacity` 序列化 + 同步实现端 + miscservices HD 触觉反馈消费逻辑 |
| Session 振动变更 | 构建 v2_0 + 验证 `PlayPatternBySessionId` / `PlayPackageBySession` / `StopVibrateBySessionId` + 同步实现端 + 音频框架触觉同步逻辑 |
| 热插拔变更 | 构建 v2_0 + 验证 `IVibratorPlugCallback` 回调 + 验证 `RegVibratorPlugCallback` / `UnRegVibratorPlugCallback` + 同步实现端和消费端 |
| 复合效果变更 | 构建 v2_0 + 验证 `CompositeEffect` union 的两种模式（时间基 + 原始基）+ 同步实现端 |
| 振动器信息变更 | 构建 v2_0 + 验证 `HdfVibratorInfo` / `DeviceVibratorInfo` + 同步实现端和消费端 |
| 设备感知变更 | 构建 v2_0 + 验证 `DeviceVibratorInfo`（deviceId + vibratorId）多设备多振动器感知 + 同步实现端 |
| 回调接口变更 | 构建 v2_0 + 验证 `IVibratorPlugCallback` 热插拔回调 + 同步实现端和消费端 |
| 构建配置变更 | `./build.sh --product-name rk3568 --build-target drivers_interface_vibrator` + 验证 `bundle.json` 中 3 个 `inner_kits` 均正常导出 + 验证 `hilog_lite` 依赖 |
| lite 系统适配 | 在 lite 系统配置下构建 + 验证 v2_0 proxy 为空 group 的行为 + 验证 `hilog_lite` 依赖 |
| 新增 HDI 版本 | 构建全量 `drivers_interface_vibrator` + 验证新版本 `sub_component` 和 `inner_kits` 注册正确 |

### 静态分析 / Lint

```bash
# 代码格式与静态检查
./build.sh --product-name rk3568 --build-target drivers_interface_vibrator --lint
gn gen --check
hdi-gen --check
```

### Done 定义

- 构建通过（`drivers_interface_vibrator` 子系统 + v2_0）
- `gn gen --check` 验证通过，无构建配置语法错误
- IDL 文件语法检查通过（若 hdi-gen 提供 `--check` 模式）
- proxy/stub 由 `hdi-gen` 自动重新生成，无手改生成产物
- 无新增编译警告
- 变更范围与任务要求一致，未夹带未关联的重构
- `.idl` 变更已同步实现端 `drivers/peripheral/vibrator` + 消费端 miscservices
- HD Haptic 变更已同步 miscservices HD 触觉反馈消费逻辑
- Session 振动变更已同步音频框架触觉同步逻辑
- 热插拔变更已同步实现端和消费端的热插拔处理
- `DeviceVibratorInfo` 变更已评估多设备多振动器感知兼容性
- 枚举值/结构体字段顺序变更已评估 ABI 兼容性
- `bundle.json` 变更已同步 `sub_component` 和 `inner_kits` 列表
- `hilog_lite` 依赖未移除（保持 lite 系统日志兼容性）

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 是否触发跨层同步修改（实现端 `drivers/peripheral/vibrator` / 消费端 miscservices / 音频框架 / `bundle.json`）
4. 是否影响版本兼容性（v2_0 与 legacy v1_0-v1_3 的关系）
5. 是否影响 v2_0 专属能力（HD Haptic / Session 振动 / 热插拔）
6. 是否影响 `hilog_lite` 依赖（本仓唯一依赖 hilog_lite 的接口仓）
7. 是否触及架构不变量或需确认事项
8. 涉及 Session 振动的变更需额外说明音频框架触觉同步验证
9. 涉及热插拔的变更需额外说明 `IVibratorPlugCallback` 回调验证

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样，不能假称已通过。涉及 `.idl` 变更的，必须人工复核 IDL 语法和版本兼容性并说明无法在沙箱验证的限制。涉及 Session 振动的变更，必须人工复核 `sessionId` 与音频框架的集成逻辑。涉及热插拔的变更，必须人工复核 `IVibratorPlugCallback` 回调机制。涉及构建配置的变更，必须人工复核 `gn gen --check` 的输出和 IDL 文件语法检查结果。
