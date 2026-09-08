# AGENTS.md - OpenHarmony 音频 HDI 接口（drivers_interface_audio）

## 1. 代码地图

本仓库定义 OpenHarmony 音频 HDI 接口（@ohos/drivers_interface_audio，版本 3.2，subsystem `hdf`），是音频驱动与服务层之间的 IDL 契约。核心职责是定义音频管理器（IAudioManager）、音频适配器（IAudioAdapter）、音频渲染（IAudioRender）、音频采集（IAudioCapture）、音频回调（IAudioCallback/IAudioCaptureCallback）以及音频效果（IEffectControl/IEffectModel）的接口契约。最重要的架构边界是**`.idl` 文件是唯一真相源，proxy/stub/服务代码由 `hdi-gen` 工具在构建时生成，本仓库不包含手写的 proxy/stub 实现，仅产出 IDL 头文件和代理/桩库**。

### 非本项目维护的目录

以下目录属于其他团队或生成产物，不属于本项目维护范围，修改时请跳过：

- `v1_0/` 至 `v5_0/`：历史版本 IDL，未在当前 `bundle.json` 的 `sub_component` 中列出，仅保留兼容性参考，不属于活跃构建
- 各版本目录下 `BUILD.gn` 引用的生成产物（proxy `_proxy.h/.cpp`、stub `_stub.h/.cpp`）：由 `hdi-gen` 在构建时生成，**不要手改**
- 音频 HDI 实现由 `drivers/peripheral/audio` 仓维护，本仓是接口定义方，不包含实现

### 嵌套指引

本仓目前无嵌套 AGENTS.md / CLAUDE.md / rules / skills 文件。以下子目录可按需新建嵌套 AGENTS.md 以提供更具体的指引：

- `v6_1/`：最新主版本 IDL（含 `IAudioCaptureCallback.idl` 独立新增），可新建 `v6_1/AGENTS.md` 聚焦 v6.1 ABI 增量、`IAudioCaptureCallback` 注册时序、与 v6.0 的差异点
- `effect/v1_0/`：音频效果子域 IDL，与主音频接口独立版本管理，可新建 `effect/v1_0/AGENTS.md` 聚焦音效链路、`IEffectControl` 与 `IEffectModel` 的关系、`EffectTypes.idl` 类型定义
- `v6_0/`：当前主稳定版本，被 `sa_profile` 与外围仓 `libaudio_proxy_6.0` 依赖，可新建 `v6_0/AGENTS.md` 聚焦 ABI 稳定性与下游消费者

### 关键区域

- `v1_0/` 至 `v5_0/`：历史版本音频 IDL（5 个版本），每个版本含 `IAudioManager.idl`、`IAudioAdapter.idl`、`IAudioRender.idl`、`IAudioCapture.idl`、`IAudioCallback.idl`、`AudioTypes.idl`，**未在活跃构建中**
- `v6_0/`：当前主稳定版本 IDL（活跃构建，产出 `libaudio_proxy_6.0` / `libaudio_stub_6.0` / `audio_idl_headers`）
- `v6_1/`：最新主版本 IDL（活跃构建，产出 `libaudio_proxy_6.1` / `libaudio_stub_6.1` / `audio_idl_headers_6.1`），新增 `IAudioCaptureCallback.idl`
- `effect/v1_0/`：音频效果子域 IDL，含 `IEffectControl.idl`、`IEffectModel.idl`、`EffectTypes.idl`（活跃构建，产出 `libeffect_proxy_1.0` / `libeffect_stub_1.0` / `effect_idl_headers`）
- `audio.gni`：3 个特性开关（`drivers_interface_audio_feature_alsa_lib` / `drivers_interface_audio_feature_offload` / `drivers_interface_audio_community`），`audio_defines` 用于条件编译
- `bundle.json`：组件元数据、`sub_component`（3 个活跃目标）、`inner_kits`（10 个 inner_kits 条目：proxy/stub/headers）
- 每个版本目录下 `BUILD.gn`：调用 `hdi-gen` 生成 proxy/stub/headers 的构建规则

### Where to look

| 任务类型 | 先看哪里 |
|---|---|
| 音频主接口变更 | 对应版本目录 `IAudioManager.idl` / `IAudioAdapter.idl` / `IAudioRender.idl` / `IAudioCapture.idl` -> `AudioTypes.idl` 共享类型 -> `BUILD.gn` 生成规则 |
| 音频回调变更 | `IAudioCallback.idl`（v1_0-v6_1 通用）-> v6_1 独有的 `IAudioCaptureCallback.idl` |
| 音频效果接口变更 | `effect/v1_0/IEffectControl.idl` + `IEffectModel.idl` + `EffectTypes.idl` |
| 共享类型变更 | 对应版本的 `AudioTypes.idl`（影响该版本所有接口） |
| 新增版本 | 在仓库根创建 `vN_0/` 目录，复制上一版本 IDL，修改 `bundle.json` 的 `sub_component` 与 `inner_kits` |
| 特性开关 | `audio.gni` 的 `declare_args()` + `bundle.json` 的 `features` 列表 |
| ALSA lib / offload 适配 | `audio.gni` 中 `drivers_interface_audio_feature_alsa_lib` / `drivers_interface_audio_feature_offload` + 外围仓 `drivers/peripheral/audio` 对应适配层 |
| 社区版构建 | `audio.gni` 中 `drivers_interface_audio_community` -> `audio_defines` 中的 `AUDIO_COMMUNITY` 宏 |
| 构建配置 | `bundle.json`（依赖、rom/ram、sub_component、inner_kits）+ `audio.gni` + 各版本 `BUILD.gn` |

### 架构分层

```
音频服务层（drivers/peripheral/audio + 上层音频服务，外部仓）
  ↓ 调用 proxy 库
IDL 代理层（本仓生成产物）
  libaudio_proxy_6.0 (v6_0)
  libaudio_proxy_6.1 (v6_1)
  libeffect_proxy_1.0 (effect/v1_0)
  ↓ IPC（HDF IPC 框架）
IDL 桩层（本仓生成产物）
  libaudio_stub_6.0 / libaudio_stub_6.1 / libeffect_stub_1.0
  ↓ 由 drivers/peripheral/audio 实现
HDI 实现层（drivers/peripheral/audio，外部仓）
```

## 2. 知识路由

在规划或编辑前，先对任务分类，读取对应的 IDL 文件和构建配置。

### Task-based routing

| 任务类型 | 读取 |
|---|---|
| 音频主接口新增/修改 | 目标版本目录 `IAudioManager.idl` + `IAudioAdapter.idl` + `IAudioRender.idl` + `IAudioCapture.idl` + `AudioTypes.idl` + 该版本 `BUILD.gn` |
| 音频回调接口变更 | `IAudioCallback.idl`（所有版本）+ v6_1 的 `IAudioCaptureCallback.idl` + `bundle.json` 中 v6_1 inner_kits |
| 音频效果接口变更 | `effect/v1_0/IEffectControl.idl` + `effect/v1_0/IEffectModel.idl` + `effect/v1_0/EffectTypes.idl` + `effect/v1_0/BUILD.gn` |
| 共享类型新增/修改 | 目标版本的 `AudioTypes.idl`（影响该版本全部接口，需评估所有引用该类型的 .idl） |
| 新增主版本 | 最新版本目录（如 `v6_1/`）作为模板 -> 新建 `vN_0/` -> 修改 `bundle.json` 的 `sub_component` 与 `inner_kits` -> 修改外围仓 `drivers/peripheral/audio` 的 `min_hdi_proxy_version` |
| 特性开关变更 | `audio.gni` 的 `declare_args()` + `bundle.json` 的 `features` 列表 + 检查 `audio_defines` 在下游 `BUILD.gn` 中的使用 |
| ALSA lib 适配 | `audio.gni` 中 `drivers_interface_audio_feature_alsa_lib` + 外围仓 `drivers/peripheral/audio` 与 `drivers/peripheral/audio/alsa_lib` 适配层 |
| offload 特性 | `audio.gni` 中 `drivers_interface_audio_feature_offload` + 外围仓 offload 实现 |
| 社区版裁剪 | `audio.gni` 中 `drivers_interface_audio_community` + `audio_defines` 中 `AUDIO_COMMUNITY` 宏 + 检查下游条件编译 |
| 构建配置 | `bundle.json`（sub_component、inner_kits、deps）+ `audio.gni` + 各版本 `BUILD.gn` |

### Path-based routing

| 修改路径 | 需了解的上下文 |
|---|---|
| `v6_0/` | 当前主稳定版本，ABI 已固化，被 `sa_profile` 的 `min_hdi_proxy_version` 引用为 `libaudio_proxy_6.0.z.so`，破坏性变更会影响所有音频服务消费者 |
| `v6_1/` | 最新主版本，新增 `IAudioCaptureCallback.idl`，是当前迭代的活跃版本，变更需同步 `bundle.json` 中 v6_1 相关 inner_kits |
| `v1_0/` 至 `v5_0/` | 历史版本，未在活跃构建中，修改无构建影响但破坏 ABI 会导致旧设备无法升级，**仅在兼容性修复时进入** |
| `effect/v1_0/` | 音频效果子域，独立版本管理，与主音频接口解耦，但 `EffectTypes.idl` 类型可能被主音频引用 |
| `AudioTypes.idl`（任一版本） | 该版本的共享类型源，修改会扩散到同版本所有接口的 IDL，需评估全部引用 |
| `audio.gni` | 特性开关源，开关变更需同步 `bundle.json` 的 `features` 列表，并确认外围仓 `drivers/peripheral/audio` 的对应特性开关 |
| `bundle.json` | 活跃构建目标与 inner_kits 清单，新增版本或新增库必须在此注册 |

### Vocabulary-based routing

当任务、issue、日志、API 名称中出现以下术语时，先理解其含义和风险再动手：

| 术语 | 含义与风险 | 读取 |
|---|---|---|
| IAudioManager | 音频管理器接口，音频适配器的发现与创建入口 | 各版本 `IAudioManager.idl` |
| IAudioAdapter | 音频适配器接口，管理端口与创建 Render/Capture | 各版本 `IAudioAdapter.idl` |
| IAudioRender | 音频渲染接口，音频播放数据通路 | 各版本 `IAudioRender.idl` |
| IAudioCapture | 音频采集接口，音频录音数据通路 | 各版本 `IAudioCapture.idl` |
| IAudioCallback | 音频回调接口，事件上报（如参数更新、流事件） | 各版本 `IAudioCallback.idl` |
| IAudioCaptureCallback | 采集专用回调接口，v6_1 新增，独立于 IAudioCallback | `v6_1/IAudioCaptureCallback.idl` |
| IEffectControl | 音效控制接口，单个音效实例的控制 | `effect/v1_0/IEffectControl.idl` |
| IEffectModel | 音效模型接口，音效的创建与管理 | `effect/v1_0/IEffectModel.idl` |
| AudioTypes | 音频共享类型（采样格式、声道布局、设备描述、流描述等） | 各版本 `AudioTypes.idl` |
| EffectTypes | 音效共享类型（音效类型、参数描述等） | `effect/v1_0/EffectTypes.idl` |
| hdi-gen | HDI IDL 代码生成工具，将 `.idl` 转为 proxy/stub/headers | 各版本 `BUILD.gn` 中的 `hdi_gen` 调用 |
| ABI 契约 | 版本号即 ABI 契约，v6_0 proxy 不能与 v5_0 stub 通信 | `bundle.json` 中版本化 sub_component |
| 命名空间 | C++ 命名空间 `ohos.hdi.audio.vN_0`，版本号嵌入命名空间 | 各版本生成的头文件 |
| min_hdi_proxy_version | 外围仓 `sa_profile` 中声明的 HDI 代理版本依赖 | 外部仓 `drivers/peripheral/audio` 的 `sa_profile` |
| AUDIO_COMMUNITY | 社区版构建宏，由 `audio.gni` 中 `drivers_interface_audio_community` 控制 | `audio.gni` |
| alsa_lib | ALSA 库特性开关，启用后外围仓引入 ALSA 适配 | `audio.gni` + 外围仓 |
| offload | 卸载播放特性，将音频处理卸载到 DSP | `audio.gni` + 外围仓 |

在计划阶段，必须声明：
- **任务分类**（如：音频主接口变更 / 回调接口变更 / 音效接口变更 / 共享类型变更 / 新增版本 / 特性开关变更 / 构建配置变更）
- **目标版本**（v6_0 / v6_1 / effect/v1_0 / 历史版本），明确是否为活跃构建
- **已读取的 IDL 文件和构建配置**（具体到文件路径）
- **发现的约束**（ABI 契约、命名空间、共享类型扩散影响、外围仓同步需求）
- **是否需要同步修改其他层**：
  - IDL 变更 -> 由 `hdi-gen` 重生成 proxy/stub/headers -> 同步外围仓 `drivers/peripheral/audio` 实现
  - 特性开关变更 -> 同步 `bundle.json` 的 `features` 列表 + 外围仓对应开关
  - 新增版本 -> 同步 `bundle.json` + 外围仓 `sa_profile` 的 `min_hdi_proxy_version` + 下游消费者

## 3. 约束边界

### 架构不变量

- **`.idl` 文件是唯一真相源**：所有 proxy/stub/headers 由 `hdi-gen` 在构建时生成，本仓库不包含手写的 proxy/stub 实现代码
- **版本号即 ABI 契约**：`v6_0` proxy 不能与 `v5_0` stub 通信，跨版本交互必须通过版本化库显式选择
- **命名空间与版本绑定**：C++ 命名空间为 `ohos.hdi.audio.vN_0`，版本号嵌入命名空间，避免跨版本符号冲突
- **`AudioTypes.idl` 是版本内共享类型源**：该版本所有接口共享同一 `AudioTypes.idl`，类型变更会扩散到同版本全部接口
- **活跃构建仅含 v6_0 / v6_1 / effect/v1_0**：`bundle.json` 的 `sub_component` 列出 3 个目标，历史版本不在活跃构建中
- **音效子域独立版本管理**：`effect/v1_0` 与主音频接口版本解耦，可独立演进
- **特性开关在 interface 与 peripheral 双向对应**：`audio.gni` 中的开关需与外围仓 `drivers/peripheral/audio` 的开关保持一致
- **inner_kits 暴露的 headers/proxy/stub 是公共 ABI**：变更需评估所有下游消费者（音频服务、媒体服务、第三方驱动）

### 禁止事项

- **不要手改 `hdi-gen` 生成的 proxy/stub/headers 代码**：应修改 `.idl` 源文件后由构建系统重生成
- **不要删除或重命名已发布的 IDL 方法**：IDL 方法签名是 ABI 契约，删除或重命名会破坏旧客户端
- **不要复用或调整已有 IDL 方法的码值**：新增接口必须追加新方法，不能复用已删除方法的码值
- **不要在历史版本（v1_0-v5_0）中引入新特性**：历史版本仅用于兼容性，新特性应在新版本中开发
- **不要跨版本共享 `AudioTypes.idl`**：每个版本有独立的 `AudioTypes.idl`，跨版本类型变更需在每个版本单独修改
- **不要修改 `bundle.json` 的 `sub_component` 而不同步 `inner_kits`**：新增构建目标必须同步注册 inner_kits 以暴露给下游
- **不要翻转 `audio.gni` 特性开关默认值而不评估外围仓影响**：`alsa_lib` / `offload` / `community` 开关在 interface 与 peripheral 双向对应
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审
- **不要在本仓库中实现 HDI 逻辑**：本仓是接口定义方，实现由 `drivers/peripheral/audio` 维护

### Ask before 规则

在执行以下操作前，必须先确认或询问：
- **修改 IDL 方法签名**：确认 ABI 兼容性和版本策略
- **修改 AudioTypes.idl 类型**：确认同版本所有接口的引用，Parcelable 字段顺序
- **新增主版本目录**：确认外围仓 sa_profile 的 min_hdi_proxy_version 更新
- **翻转 audio.gni 特性开关**：确认与外围仓 `drivers/peripheral/audio` 双向同步

### 需确认后再修改

- **IDL 方法签名变更**（需确认 ABI 兼容性影响和版本策略，评估所有下游消费者）
- **`AudioTypes.idl` 类型新增/删除/字段调整**（需评估同版本所有接口的引用，Parcelable 字段顺序是 ABI 一部分）
- **新增主版本**（需确认外围仓 `drivers/peripheral/audio` 的实现计划、`sa_profile` 的 `min_hdi_proxy_version` 更新、下游消费者适配）
- **`bundle.json` 的 `sub_component` 或 `inner_kits` 调整**（需确认下游消费者是否依赖被移除的 inner_kits）
- **`audio.gni` 特性开关默认值翻转**（特别是 `drivers_interface_audio_feature_alsa_lib` / `drivers_interface_audio_feature_offload`，需确认外围仓同步）
- **`drivers_interface_audio_community` 默认值翻转**（影响 `AUDIO_COMMUNITY` 宏的编译范围）
- **历史版本（v1_0-v5_0）的兼容性修复**（需确认修复是否需要回溯到所有受影响版本）
- **新增外部依赖**（需确认许可证、包大小、`bundle.json` 同步）

### 项目特定陷阱

- **v6_1 的 `IAudioCaptureCallback.idl` 独立新增**：v6_1 将采集回调从 `IAudioCallback` 拆分为独立的 `IAudioCaptureCallback`，下游消费者若未区分会导致采集事件丢失
- **v6_0 与 v6_1 并行活跃构建**：两个版本同时产出 proxy/stub 库，外围仓需明确选择依赖版本，不能混用
- **`AudioTypes.idl` 字段顺序是 ABI**：IDL 中 struct 字段顺序决定 Parcelable 序列化顺序，新增字段必须追加在末尾
- **effect 子域类型与主音频类型独立**：`EffectTypes.idl` 与 `AudioTypes.idl` 不共享，跨子域引用类型需显式 import
- **`audio_defines` 的条件编译扩散**：`AUDIO_COMMUNITY` 宏会扩散到下游 `BUILD.gn` 的 `defines`，漏掉一处会导致特性开关失效
- **`alsa_lib` 与 `offload` 双仓联动**：`audio.gni` 中的开关需与外围仓 `drivers/peripheral/audio` 的对应开关同时翻转，否则构建不匹配
- **历史版本 IDL 仍占命名空间**：v1_0-v5_0 虽未活跃构建，但其 `ohos.hdi.audio.vN_0` 命名空间仍被旧二进制引用，删除会导致符号冲突
- **`hdi-gen` 工具版本与 IDL 语法兼容性**：升级 `hdi-gen` 工具可能引入新语法支持，但旧 IDL 语法必须保持向后兼容

### Agent failure patterns

- **Agent 手改生成产物**：Agent 可能直接编辑 `v1_0/` 至 `v5_0/` 目录下 `BUILD.gn` 引用的 `_proxy.h/.cpp`、`_stub.h/.cpp` 文件。必须拒绝此行为，应修改 `.idl` 源文件后由构建系统重生成
- **Agent 混淆活跃版本与历史版本**：Agent 可能将变更应用到 `v1_0`-`v5_0` 历史版本而非 `v6_0`/`v6_1` 活跃版本，导致无构建影响但破坏 ABI 兼容性。历史版本仅在兼容性修复时进入
- **Agent 忽略 `AudioTypes.idl` 字段顺序**：Agent 新增 struct 字段时可能插入到中间而非追加末尾，导致 Parcelable 反序列化失败
- **Agent 遗漏双仓联动**：Agent 修改 `audio.gni` 开关时可能忘记同步外围仓 `drivers/peripheral/audio` 的对应开关，导致构建不匹配
- **Agent 跨版本混用库**：Agent 可能将 `libaudio_proxy_6.0` 与 `libaudio_proxy_6.1` 混用，或让外围仓同时依赖两个版本而不明确选择
- **Agent 遗漏 `IAudioCaptureCallback` 时序**：v6_1 中采集回调已从 `IAudioCallback` 拆分，Agent 可能仍按 v6_0 逻辑处理采集事件导致事件丢失

## 4. 验证闭环

### 最小验证

```bash
# 构建 audio 接口组件（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target drivers_interface_audio

# 单独构建活跃版本
./build.sh --product-name rk3568 --build-target //drivers/interface/audio/v6_0:libaudio_proxy_6.0
./build.sh --product-name rk3568 --build-target //drivers/interface/audio/v6_1:libaudio_proxy_6.1
./build.sh --product-name rk3568 --build-target //drivers/interface/audio/effect/v1_0:libeffect_proxy_1.0
```

### Lint and static analysis

```bash
# 格式检查
gn format --check v1_0/ v6_0/ v6_1/ effect/v1_0/

# IDL 语法检查（验证 .idl 文件格式是否符合 hdi-gen 要求）
python3 scripts/check_idl_syntax.py --repo audio v1_0/ v6_0/ v6_1/ effect/v1_0/

# 构建目标 lint 检查
./build.sh --product-name rk3568 --build-target drivers_interface_audio --lint

# 检查生成产物是否被手改（对比 git diff 与 hdi-gen 输出）
git diff --name-only -- '**/_proxy.h' '**/_proxy.cpp' '**/_stub.h' '**/_stub.cpp' | grep -v 'BUILD.gn' | head -20
```

### 测试命令

```bash
# 运行音频服务接口测试
./build.sh --product-name rk3568 --build-target tests --test-target audio_service_test

# 运行 ABI 兼容性检查
python3 scripts/check_abi_compatibility.py --repo audio --compare v6_0 v6_1

# 运行下游消费者集成测试
./build.sh --product-name rk3568 --build-target //drivers/peripheral/audio --test-target audio_hdi_test
```

### 任务特定验证

| 任务类型 | 验证命令 |
|---|---|
| 音频主接口变更 | `./build.sh --build-target drivers_interface_audio` + 同步构建外围仓 `drivers/peripheral/audio` 验证实现兼容 + 跑下游音频服务的接口测试 + `python3 scripts/check_abi_compatibility.py --repo audio --compare v6_0 v6_1` |
| 回调接口变更 | 重新生成 proxy/stub + 同步外围仓 `BatteryCallback`/音频回调实现 + 验证 v6_1 的 `IAudioCaptureCallback` 注册时序 + 运行回调时序测试 |
| 音效接口变更 | `./build.sh --build-target //drivers/interface/audio/effect/v1_0:libeffect_proxy_1.0` + 同步构建外围仓音效实现 + 验证 `EffectTypes.idl` 与 `AudioTypes.idl` 不共享 |
| 共享类型变更 | 全量构建 `drivers_interface_audio` + 验证同版本所有接口的引用编译通过 + 跑下游消费者 ABI 检查 + `python3 scripts/check_parcelable_fields.py --repo audio` |
| 新增版本 | 新建版本目录 + 修改 `bundle.json` + 全量构建 + 验证外围仓 `sa_profile` 的 `min_hdi_proxy_version` 更新 + 下游消费者适配 |
| 特性开关翻转 | 重新构建 `drivers_interface_audio` 全量 + 同步外围仓 `drivers/peripheral/audio` 构建 + 验证 `bundle.json` 的 `features` 同步 + 真机验证 ALSA/offload 通路 |
| ALSA lib / offload 适配 | 翻转 `audio.gni` 开关 + 同步外围仓构建 + 真机验证 ALSA/offload 通路 + `gn format --check audio.gni` |
| 社区版构建 | 设置 `drivers_interface_audio_community=true` + 全量构建 + 验证 `AUDIO_COMMUNITY` 宏裁剪生效 + 检查下游 `BUILD.gn` 的 `defines` |
| 构建配置变更 | `./build.sh --build-target drivers_interface_audio` + 验证 `inner_kits` 暴露的 headers 可被下游引用 |

### 静态分析 / Lint

```bash
# 代码格式与静态检查
./build.sh --product-name rk3568 --build-target drivers_interface_audio --lint
gn gen --check
hdi-gen --check
```

### Done 定义

- 构建通过（`drivers_interface_audio` 子系统 + 3 个活跃版本目标 + 外围仓 `drivers/peripheral/audio`）
- Lint/格式检查通过（`gn format --check` 无格式差异）
- IDL 语法检查通过（`scripts/check_idl_syntax.py` 无错误）
- 无新增编译警告
- 变更范围与任务要求一致，未夹带未关联的重构
- IDL 变更已由 `hdi-gen` 重生成 proxy/stub/headers，无手改生成产物
- 特性开关变更已同步 `audio.gni` + `bundle.json` + 外围仓 `drivers/peripheral/audio`
- 新增版本已同步 `bundle.json` 的 `sub_component` 与 `inner_kits` + 外围仓 `sa_profile` 的 `min_hdi_proxy_version`
- `AudioTypes.idl` / `EffectTypes.idl` 类型变更已评估同版本所有接口的引用
- 活跃版本（v6_0 / v6_1）的变更已评估 ABI 兼容性，未删除或重命名已发布方法
- 生成产物未被手改（`git diff` 对比无 `_proxy.h/.cpp`、`_stub.h/.cpp` 变更）

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 目标版本（v6_0 / v6_1 / effect/v1_0 / 历史版本）及是否为活跃构建
4. 是否触发跨层同步修改（IDL 重生成 / 外围仓 `drivers/peripheral/audio` / `bundle.json` / `audio.gni` / `sa_profile`）
5. 是否影响特性开关默认值或 ABI 契约
6. 是否触及架构不变量或需确认事项

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样，不能假称已通过。涉及外围仓 `drivers/peripheral/audio` 的变更，必须人工复核 IDL 与实现的匹配性并说明无法在沙箱验证的限制。
