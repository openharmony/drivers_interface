# AGENTS.md - OpenHarmony Motion HDI 接口定义（drivers_interface_motion）

## 1. 代码地图

本仓库定义 OpenHarmony Motion HDI 接口（`@ohos/drivers_interface_motion`，版本 4.0），subsystem `hdf`，part `drivers_interface_motion`。核心职责是定义动作手势检测的 IDL 契约（拾起 / 翻转 / 摇晃 / 倾斜 / 旋转等动作类型），由 `hdi-gen` 工具自动生成 proxy/stub 代码。**本仓只维护 `.idl` 源文件，不包含任何实现代码**——实现由 `drivers/peripheral/motion` 提供，消费方为 miscservices。最重要的架构边界是**v1_0 与 v1_1 两个版本同时构建，v1_1 通过 `proxy_deps`/`stub_deps` 复用 v1_0 的回调接口与底层契约，向后兼容性是第一优先级**。

### 非本项目维护的目录

以下内容属于其他团队或生成产物，不属于本项目维护范围，修改时请跳过：

- 任何由 `hdi-gen` 生成的 proxy/stub `.h/.cpp` 文件（构建产物，不在源码树中）
- `drivers/peripheral/motion/`：Motion HDI 实现端（由驱动团队维护）
- miscservices 中消费 Motion 接口的代码（由上层服务团队维护）

### 嵌套指引

本仓目前无嵌套 AGENTS.md / CLAUDE.md / rules / skills 文件。仓库结构扁平（仅 `v1_0/`、`v1_1/` 两个版本目录 + `bundle.json`），不建议新建嵌套指引；版本差异在本文档「知识路由」中集中说明。

### 关键区域

- `v1_0/`：Motion HDI 1.0 接口定义（`@since 3.2`），含 3 个 IDL 文件 + `BUILD.gn`：
  - `IMotionInterface.idl`：主接口，4 个方法（`EnableMotion` / `DisableMotion` / `Register` / `Unregister`）
  - `IMotionCallback.idl`：数据上报回调接口（v1_1 复用此回调）
  - `MotionTypes.idl`：`HdfMotionTypeTag` 枚举（PICKUP/FLIP/SHAKE/ROTATION 等 12 个值）+ `HdfMotionEvent` 数据结构
  - `BUILD.gn`：`hdi("motion")` 模板，`module_name = "motion_service"`，无 `proxy_deps`/`stub_deps`（独立基线版本）
- `v1_1/`：Motion HDI 1.1 接口定义（`@since 4.0`），含 2 个 IDL 文件 + `BUILD.gn`：
  - `IMotionInterface.idl`：扩展后的主接口
  - `MotionTypes.idl`：扩展后的类型（新增 `WaveParam` 结构体 + 扩展 `HdfMotionTypeTag` 枚举：新增 TOUCH_LINK/HOVER/PUT_IN_POCKET/RESERVED 等）
  - **无 `IMotionCallback.idl`**：v1_1 复用 v1_0 的回调接口
  - `BUILD.gn`：`hdi("motion")` 模板，**`proxy_deps = [ "../v1_0:libmotion_proxy_1.0" ]` + `stub_deps = [ "../v1_0:libmotion_stub_1.0" ]`**——这是版本兼容性的关键约束
- `bundle.json`：组件清单，`adapted_system_type: ["standard", "small"]`，依赖 `ipc` / `hdf_core` / `hilog` / `c_utils`，构建 `v1_0:motion_idl_target` + `v1_1:motion_idl_target`，导出 6 个 inner_kits（v1_0/v1_1 各 3 个：proxy + stub + idl_headers）

### Where to look

| 任务类型 | 先看哪里 |
|---|---|
| 新增/修改动作类型 | `v1_0/MotionTypes.idl`（`HdfMotionTypeTag` 枚举）+ `v1_1/MotionTypes.idl`（扩展枚举）-> 评估是否需要在 v1_0 修改还是仅 v1_1 |
| 新增/修改主接口方法 | `v1_0/IMotionInterface.idl` 或 `v1_1/IMotionInterface.idl` -> 重新生成 proxy/stub（由构建系统完成）-> 同步实现端 `drivers/peripheral/motion` |
| 回调接口变更 | `v1_0/IMotionCallback.idl`（**唯一回调定义**，v1_1 复用）-> 评估对 v1_0 和 v1_1 双版本的影响 |
| 动作数据结构变更 | `v1_0/MotionTypes.idl`（`HdfMotionEvent`）-> 同步 `drivers/peripheral/motion` 上报逻辑 + miscservices 消费逻辑 |
| 波形参数（v1_1 专属） | `v1_1/MotionTypes.idl`（`WaveParam` 结构体）-> 仅影响 v1_1 消费者 |
| 版本兼容性评估 | `v1_1/BUILD.gn`（`proxy_deps`/`stub_deps` 指向 v1_0）+ `bundle.json`（双版本同时构建）|
| 构建配置 | `bundle.json`（依赖、inner_kits）+ `v1_0/BUILD.gn` + `v1_1/BUILD.gn`（`hdi()` 模板）|

### 架构分层

```
IDL 源文件（本仓维护）
  ├─ v1_0/ (基线版本，@since 3.2)
  │    ├─ IMotionInterface.idl (EnableMotion/DisableMotion/Register/Unregister)
  │    ├─ IMotionCallback.idl  (数据上报回调，v1_1 复用)
  │    └─ MotionTypes.idl      (HdfMotionTypeTag 12 值 + HdfMotionEvent)
  │        ↓ hdi-gen 生成
  │    libmotion_proxy_1.0 + libmotion_stub_1.0 + motion_idl_headers
  │
  └─ v1_1/ (扩展版本，@since 4.0，proxy/stub 依赖 v1_0)
       ├─ IMotionInterface.idl (扩展后的主接口)
       ├─ MotionTypes.idl      (WaveParam + HdfMotionTypeTag 扩展枚举)
       │   proxy_deps = ["../v1_0:libmotion_proxy_1.0"]
       │   stub_deps  = ["../v1_0:libmotion_stub_1.0"]
       │       ↓ hdi-gen 生成
       └─ libmotion_proxy_1.1 + libmotion_stub_1.1 + motion_idl_headers
                ↓
           实现端：drivers/peripheral/motion
                ↓
           消费端：miscservices

命名空间：ohos.hdi.motion.v1_0 / ohos.hdi.motion.v1_1
```

## 2. 知识路由

在规划或编辑前，先对任务分类，读取对应的代码路径和文档。

### Task-based routing

| 任务类型 | 读取 |
|---|---|
| 新增动作类型 | `v1_0/MotionTypes.idl`（`HdfMotionTypeTag`）+ `v1_1/MotionTypes.idl`（扩展枚举）-> 决定在 v1_0 还是 v1_1 新增 -> 同步实现端 `drivers/peripheral/motion` |
| 修改主接口 | `v1_0/IMotionInterface.idl` 或 `v1_1/IMotionInterface.idl` -> 评估版本影响 -> 同步实现端 |
| 回调接口变更 | `v1_0/IMotionCallback.idl`（唯一回调）-> **必须评估 v1_0 和 v1_1 双版本影响**（v1_1 复用此回调）-> 同步实现端 + 消费端 |
| 数据结构变更 | `v1_0/MotionTypes.idl`（`HdfMotionEvent`）-> 评估序列化兼容性 -> 同步实现端 + 消费端 |
| 新增 v1_1 专属类型 | `v1_1/MotionTypes.idl`（如 `WaveParam`）-> 确认仅影响 v1_1 消费者 |
| 构建配置变更 | `bundle.json`（依赖、inner_kits 列表）+ `v1_0/BUILD.gn` + `v1_1/BUILD.gn`（`hdi()` 模板、`proxy_deps`/`stub_deps`）|
| 版本兼容性评估 | `v1_1/BUILD.gn` 中 `proxy_deps`/`stub_deps` 指向 v1_0 的声明 + `bundle.json` 双版本 `sub_component` |

### Path-based routing

| 修改路径 | 需了解的上下文 |
|---|---|
| `v1_0/IMotionInterface.idl` | 基线主接口，修改后构建系统生成 `libmotion_proxy_1.0`/`libmotion_stub_1.0`，**不要直接改生成产物**；v1_1 通过 `proxy_deps`/`stub_deps` 依赖此版本，变更会传递到 v1_1 |
| `v1_0/IMotionCallback.idl` | **唯一回调定义**，v1_1 复用此回调（v1_1 目录无 `IMotionCallback.idl`），修改影响 v1_0 和 v1_1 双版本 |
| `v1_0/MotionTypes.idl` | 基线类型定义，`HdfMotionTypeTag` 枚举值顺序是 ABI 一部分，新增类型必须追加在 `HDF_MOTION_TYPE_MAX` 之前 |
| `v1_1/IMotionInterface.idl` | 扩展主接口，修改后构建系统生成 `libmotion_proxy_1.1`/`libmotion_stub_1.1`，依赖 v1_0 的 proxy/stub |
| `v1_1/MotionTypes.idl` | 扩展类型定义，新增了 `WaveParam` 和扩展 `HdfMotionTypeTag`（TOUCH_LINK/HOVER/PUT_IN_POCKET/RESERVED），**v1_1 的枚举值与 v1_0 不完全一致**，新增枚举在 v1_0 的 MAX 位置之后 |
| `v1_1/BUILD.gn` | `proxy_deps`/`stub_deps` 指向 v1_0 是版本兼容性的关键约束，移除或修改会破坏 v1_1 对 v1_0 回调的复用 |
| `bundle.json` | 双版本 `sub_component` + 6 个 inner_kits，新增版本需在此注册 |

### Vocabulary-based routing

当任务、issue、日志、API 名称中出现以下术语时，先理解其含义和风险再动手：

| 术语 | 含义与风险 | 读取 |
|---|---|---|
| Motion / motion_service | 动作手势检测 HDI，`module_name = "motion_service"` | 本文件 + `v1_0/BUILD.gn` |
| HdfMotionTypeTag | 动作类型枚举（PICKUP/FLIP/SHAKE/ROTATION/POCKET_MODE/WRIST_UP/WRIST_DOWN/WAVE/STEP_COUNTER 等），枚举值顺序是 ABI 一部分 | `v1_0/MotionTypes.idl` + `v1_1/MotionTypes.idl` |
| HdfMotionEvent | 动作事件数据结构（motion/result/status/datalen/data[]），跨进程传输 | `v1_0/MotionTypes.idl` |
| WaveParam | 波形参数（v1_1 专属：waveFrequency/waveAmplitude/isUseGyroscope）| `v1_1/MotionTypes.idl` |
| IMotionInterface | Motion 主接口，v1_0 含 4 方法，v1_1 为扩展版本 | `v1_0/IMotionInterface.idl` + `v1_1/IMotionInterface.idl` |
| IMotionCallback | 动作数据上报回调，**仅在 v1_0 定义**，v1_1 复用 | `v1_0/IMotionCallback.idl` |
| hdi-gen | HDL IDL 编译器，将 `.idl` 编译为 proxy/stub C++ 代码 | 构建系统 `//build/config/components/hdi/hdi.gni` |
| hdi() | GN 模板，用于声明 HDL 接口模块 | `v1_0/BUILD.gn` + `v1_1/BUILD.gn` |
| proxy_deps / stub_deps | v1_1 BUILD.gn 中指向 v1_0 的依赖声明，是版本复用机制的核心 | `v1_1/BUILD.gn` |
| libmotion_proxy_1.0 / libmotion_stub_1.0 | v1_0 生成的 proxy/stub 库 | 构建产物 |
| libmotion_proxy_1.1 / libmotion_stub_1.1 | v1_1 生成的 proxy/stub 库（依赖 v1_0 的 proxy/stub）| 构建产物 |
| inner_kits | 内部 SDK 接口声明，本仓导出 6 个（v1_0/v1_1 各 3 个）| `bundle.json` |
| ohos.hdi.motion.v1_0 / ohos.hdi.motion.v1_1 | IDL 包命名空间 | `MotionTypes.idl` 顶部 `package` 声明 |

在计划阶段，必须声明：
- **任务分类**（如：新增动作类型 / 修改主接口 / 回调接口变更 / 数据结构变更 / 构建配置变更）
- **已读取的代码路径和文档**（具体到文件路径）
- **发现的约束**（架构不变量、禁止事项、版本兼容性约束、跨仓依赖）
- **是否需要同步修改其他层**：
  - `.idl` 变更 -> 由构建系统重生成 proxy/stub + 同步实现端 `drivers/peripheral/motion` + 消费端 miscservices
  - `IMotionCallback.idl` 变更 -> 必须同步评估 v1_0 和 v1_1 双版本影响
  - `bundle.json` 变更 -> 同步 `inner_kits` 列表
  - `MotionTypes.idl` 枚举变更 -> 评估 ABI 兼容性（枚举值顺序）

## 3. 约束边界

### 架构不变量

- **`.idl` 文件是唯一真相源**：所有接口和类型定义必须通过 `.idl` 文件表达，proxy/stub 由 `hdi-gen` 自动生成
- **v1_1 复用 v1_0 的回调接口**：`v1_1/` 目录不含 `IMotionCallback.idl`，通过 `proxy_deps`/`stub_deps` 指向 v1_0 的 proxy/stub 实现复用
- **双版本同时构建**：v1_0 和 v1_1 都在 `bundle.json` 的 `sub_component` 中注册，均生成独立的 proxy/stub 库
- **v1_1 是 v1_0 的超集**：v1_1 在 v1_0 基础上扩展动作类型和接口方法，不删除或破坏 v1_0 已有能力
- **IDL 类型顺序是 ABI 一部分**：`HdfMotionTypeTag` 枚举值顺序、`HdfMotionEvent` 字段顺序一旦发布即固定，新增必须追加
- **本仓不含实现**：接口实现由 `drivers/peripheral/motion` 提供，本仓仅定义契约

### 禁止事项

- **不要直接编辑 `hdi-gen` 生成的 proxy/stub `.h/.cpp` 代码**，应修改 `.idl` 源文件后由构建系统重新生成
- **不要在 v1_1 目录中新增 `IMotionCallback.idl`**：v1_1 通过 `proxy_deps`/`stub_deps` 复用 v1_0 的回调，新增会破坏复用机制
- **不要删除或修改 `v1_1/BUILD.gn` 中的 `proxy_deps`/`stub_deps`**：这是 v1_1 复用 v1_0 回调的关键声明
- **不要调整 `HdfMotionTypeTag` 枚举值顺序**：已有枚举值的位置是 ABI 契约，新增类型必须追加在 `HDF_MOTION_TYPE_MAX` 之前
- **不要在 `HdfMotionEvent` 结构体中间插入字段**：跨进程序列化的字段顺序是 ABI 一部分，新增字段必须追加在末尾
- **不要移除 `bundle.json` 中任一版本的 `sub_component`**：双版本同时构建是兼容性要求
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审（当前依赖：`ipc` / `hdf_core` / `hilog` / `c_utils`）
- **不要在本仓中添加实现代码**：本仓是纯接口定义仓

### Ask before 规则

在执行以下操作前，必须先确认或询问：
- **修改 IMotionCallback.idl**：确认影响 v1_0 和 v1_1 双版本
- **调整 HdfMotionTypeTag 枚举值顺序**：确认 ABI 兼容性
- **新增 v1_1 专属类型**：确认仅影响 v1_1 消费者
- **修改 proxy_deps/stub_deps**：确认 v1_1 复用 v1_0 回调的机制

### 需确认后再修改

- **`IMotionCallback.idl` 回调接口变更**（影响 v1_0 和 v1_1 双版本，需确认实现端和消费端兼容性）
- **`HdfMotionTypeTag` 枚举值变更**（需评估 ABI 兼容性，已发布的枚举值位置不能调整）
- **`HdfMotionEvent` 字段新增/删除**（需评估序列化兼容性，跨进程传输的 Parcelable 字段顺序是 ABI 一部分）
- **新增 HDI 版本（如 v1_2）**（需确认版本规划，在 `bundle.json` 注册 `sub_component` + `inner_kits`，决定是否复用 v1_0/v1_1 的回调）
- **`bundle.json` 中 `inner_kits` 列表变更**（需确认下游消费者依赖关系）
- **新增外部依赖**（需确认许可证、包大小、`bundle.json` 同步）
- **`adapted_system_type` 变更**（当前 `["standard", "small"]`，新增 `mini` 需评估 lite 系统兼容性）

### 项目特定陷阱

- **v1_1 复用 v1_0 回调的隐性依赖**：`v1_1/BUILD.gn` 中 `proxy_deps = [ "../v1_0:libmotion_proxy_1.0" ]` 和 `stub_deps = [ "../v1_0:libmotion_stub_1.0" ]` 意味着修改 `v1_0/IMotionCallback.idl` 会直接影响 v1_1 的回调行为，必须双版本验证
- **v1_0 与 v1_1 枚举值差异**：v1_0 的 `HdfMotionTypeTag` 在 `HDF_MOTION_TYPE_STEP_COUNTER` 后直接是 `HDF_MOTION_TYPE_MAX`；v1_1 在中间新增了 `TOUCH_LINK`/`HOVER`/`PUT_IN_POCKET`/`RESERVED`，两个版本的 `MAX` 值不同，跨版本传递枚举值时需注意
- **`HdfMotionEvent.datalen` 与 `data[]` 的一致性**：`datalen` 必须与 `data` 数组实际长度一致，否则跨进程序列化会出错
- **`module_name = "motion_service"` 的一致性**：v1_0 和 v1_1 的 `BUILD.gn` 都使用相同的 `module_name`，这是 HDI 服务名约定，不能单独修改
- **无 `.gni` 特性开关**：本仓没有 `declare_args()` 特性开关，所有能力通过版本目录表达，不要尝试添加特性开关
- **`adapted_system_type` 不含 `mini`**：本仓适配 `standard` 和 `small` 系统，不适配 `mini` 系统，与 `drivers_interface_vibrator`（同样不含 mini）一致但与 `drivers_interface_tools`（含 mini）不同

### Agent 失败模式

- **直接编辑生成产物**：Agent 可能 tempted to edit `hdi-gen` 生成的 proxy/stub `.h/.cpp` 文件以快速修复问题，而非修改 `.idl` 源文件后重新生成——这会导致生成的代码被覆盖，且无法追溯变更来源
- **忽略双版本影响**：Agent 可能只修改 `v1_0/` 或 `v1_1/` 而忘记评估对另一个版本的影响，尤其是 `IMotionCallback.idl` 变更会同时影响两个版本
- **遗漏跨层同步**：Agent 可能完成 `.idl` 修改和构建验证后，忘记同步实现端 `drivers/peripheral/motion` 和消费端 miscservices
- **忽略 ABI 兼容性**：Agent 可能新增 `HdfMotionTypeTag` 枚举值或 `HdfMotionEvent` 字段时未考虑枚举值顺序和字段顺序对 ABI 的影响
- **误改 `bundle.json`**：Agent 可能随意修改 `bundle.json` 的 `sub_component` 或 `inner_kits`，导致构建配置不一致
- **忽略版本兼容性约束**：Agent 可能在 v1_1 中新增 `IMotionCallback.idl` 或修改 `proxy_deps`/`stub_deps`，破坏 v1_1 复用 v1_0 回调的机制

## 4. 验证闭环

### 最小验证

```bash
# 构建 drivers_interface_motion 子系统（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target drivers_interface_motion

# 单独构建 v1_0
./build.sh --product-name rk3568 --build-target //drivers/interface/motion/v1_0:motion_idl_target

# 单独构建 v1_1（依赖 v1_0，会自动拉起 v1_0 构建）
./build.sh --product-name rk3568 --build-target //drivers/interface/motion/v1_1:motion_idl_target

# 静态分析：验证 GN 构建配置语法
gn gen --check //drivers/interface/motion

# 静态分析：检查 IDL 文件语法（若 hdi-gen 提供 --check 模式）
# hdi-gen --check v1_0/*.idl v1_1/*.idl
```

### 任务特定验证

| 任务类型 | 验证命令 |
|---|---|
| `.idl` 接口变更 | `./build.sh --product-name rk3568 --build-target drivers_interface_motion` + 验证 proxy/stub 重新生成 + 同步构建实现端 `drivers/peripheral/motion` |
| `IMotionCallback.idl` 变更 | 构建双版本 `v1_0` + `v1_1` + 验证 v1_1 通过 `proxy_deps`/`stub_deps` 正确复用 v1_0 回调 + 同步实现端和消费端 |
| `MotionTypes.idl` 枚举变更 | 构建双版本 + 验证枚举值顺序未破坏 ABI + 同步 `drivers/peripheral/motion` 和 miscservices |
| `HdfMotionEvent` 结构体变更 | 构建双版本 + 验证序列化/反序列化一致性 + 同步实现端 |
| 新增 v1_1 专属类型（如 `WaveParam`）| 构建 v1_1 + 验证 v1_0 不受影响 |
| 构建配置变更 | `./build.sh --product-name rk3568 --build-target drivers_interface_motion` + 验证 `bundle.json` 中 6 个 `inner_kits` 均正常导出 |
| 新增 HDI 版本 | 构建全量 `drivers_interface_motion` + 验证新版本 `sub_component` 和 `inner_kits` 注册正确 |

### 静态分析 / Lint

```bash
# 代码格式与静态检查
./build.sh --product-name rk3568 --build-target drivers_interface_motion --lint
gn gen --check
hdi-gen --check
```

### Done 定义

- 构建通过（`drivers_interface_motion` 子系统 + v1_0 + v1_1 双版本）
- `gn gen --check` 验证通过，无构建配置语法错误
- IDL 文件语法检查通过（若 hdi-gen 提供 `--check` 模式）
- proxy/stub 由 `hdi-gen` 自动重新生成，无手改生成产物
- 无新增编译警告
- 变更范围与任务要求一致，未夹带未关联的重构
- `.idl` 变更已同步实现端 `drivers/peripheral/motion` + 消费端 miscservices
- `IMotionCallback.idl` 变更已评估 v1_0 和 v1_1 双版本影响
- 枚举值/结构体字段顺序变更已评估 ABI 兼容性
- `bundle.json` 变更已同步 `sub_component` 和 `inner_kits` 列表
- 未触及 `gn gen --check` 或静态分析发现的新问题

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 是否触发跨层同步修改（实现端 `drivers/peripheral/motion` / 消费端 miscservices / `bundle.json`）
4. 是否影响版本兼容性（v1_0 与 v1_1 之间的复用关系）
5. 是否触及架构不变量或需确认事项
6. 涉及回调接口 `IMotionCallback.idl` 的变更需额外说明双版本影响验证

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样，不能假称已通过。涉及 `.idl` 变更的，必须人工复核 IDL 语法和版本兼容性并说明无法在沙箱验证的限制。涉及构建配置的变更，必须人工复核 `gn gen --check` 的输出和 IDL 文件语法检查结果。
