# AGENTS.md - OpenHarmony 指示灯 HDI 接口（drivers_interface_light）

## 1. 代码地图

本仓库定义 OpenHarmony 指示灯 HDI 接口（@ohos/drivers_interface_light，版本 4.0，subsystem `hdf`），是指示灯驱动与上层服务之间的 IDL 契约。核心职责是定义指示灯控制接口（ILightInterface）的开关、查询、亮度/颜色/呼吸效果等操作。最重要的架构边界是**本仓库是 interface 仓库中最简单的，单版本（v1_0）、2 个 IDL 文件，proxy/stub 由 `hdi-gen` 工具在构建时生成，被 `drivers/peripheral/light`（实现方）和 `battery_manager`（消费方）等仓引用**。

### 非本项目维护的目录

以下目录属于其他团队或生成产物，不属于本项目维护范围，修改时请跳过：

- `v1_0/` 下 `BUILD.gn` 引用的生成产物（proxy `_proxy.h/.cpp`、stub `_stub.h/.cpp`）：由 `hdi-gen` 在构建时生成，**不要手改**
- 指示灯 HDI 实现由 `drivers/peripheral/light` 仓维护，本仓是接口定义方，不包含实现
- 指示灯消费逻辑由 `battery_manager` 仓维护（电池指示灯），本仓不包含消费侧代码

### 嵌套指引

本仓目前无嵌套 AGENTS.md / CLAUDE.md / rules / skills 文件。本仓库结构极简（单版本、2 个 IDL 文件），不建议新建嵌套 AGENTS.md，所有指引在本文件中维护。

### 关键区域

- `v1_0/`：唯一版本目录，含 `ILightInterface.idl`（接口定义）和 `LightTypes.idl`（共享类型定义）
- `v1_0/ILightInterface.idl`：指示灯控制接口，定义 `TurnOn`、`TurnOff`、`GetLightInfo`、`GetLightInfoById` 等方法
- `v1_0/LightTypes.idl`：指示灯共享类型，定义 `HdfLightInfo`、`HdfLightEffect`、`HdfLightFlashEffect`、`HdfLightBrightness` 等
- `v1_0/BUILD.gn`：调用 `hdi-gen` 生成 proxy/stub/headers 的构建规则，产出 `light_idl_target` / `liblight_proxy_1.0` / `liblight_stub_1.0` / `light_idl_headers`
- `bundle.json`：组件元数据、`sub_component`（1 个活跃目标）、`inner_kits`（3 个 inner_kits 条目：proxy/stub/headers）

### Where to look

| 任务类型 | 先看哪里 |
|---|---|
| 指示灯接口变更 | `v1_0/ILightInterface.idl` -> `v1_0/LightTypes.idl` 共享类型 -> `v1_0/BUILD.gn` 生成规则 |
| 共享类型变更 | `v1_0/LightTypes.idl`（影响 `ILightInterface.idl` 所有方法） |
| 新增指示灯方法 | `v1_0/ILightInterface.idl` 追加方法（保持 ABI 兼容，不能复用已有码值） |
| 新增指示灯类型 | `v1_0/LightTypes.idl` 追加类型 |
| 构建配置 | `bundle.json`（sub_component、inner_kits、deps）+ `v1_0/BUILD.gn` |

### 架构分层

```
指示灯消费层（battery_manager 等，外部仓）
  ↓ 调用 proxy 库
IDL 代理层（本仓生成产物）
  liblight_proxy_1.0 (v1_0)
  ↓ IPC（HDF IPC 框架）
IDL 桩层（本仓生成产物）
  liblight_stub_1.0
  ↓ 由 drivers/peripheral/light 实现
HDI 实现层（drivers/peripheral/light，外部仓）
```

## 2. 知识路由

在规划或编辑前，先对任务分类，读取对应的 IDL 文件和构建配置。

### Task-based routing

| 任务类型 | 读取 |
|---|---|
| 指示灯接口新增/修改 | `v1_0/ILightInterface.idl` + `v1_0/LightTypes.idl` + `v1_0/BUILD.gn` |
| 共享类型新增/修改 | `v1_0/LightTypes.idl`（影响 `ILightInterface.idl` 所有方法）+ 评估外围仓 `drivers/peripheral/light` 实现兼容性 |
| 新增指示灯方法 | `v1_0/ILightInterface.idl` 追加方法（保持 ABI 兼容）+ 同步外围仓 `drivers/peripheral/light` 实现 + 同步消费方 `battery_manager` |
| 新增指示灯效果类型 | `v1_0/LightTypes.idl` 追加 `HdfLight*Effect` 类型 + 评估 `TurnOn` 方法是否需要扩展参数 |
| 构建配置变更 | `bundle.json`（sub_component、inner_kits、deps）+ `v1_0/BUILD.gn` |

### Path-based routing

| 修改路径 | 需了解的上下文 |
|---|---|
| `v1_0/ILightInterface.idl` | 唯一接口定义文件，被 `sa_profile` 的 `min_hdi_proxy_version` 引用为 `liblight_proxy_1.0.z.so`，破坏性变更会影响所有指示灯消费者（特别是 `battery_manager` 的电池指示灯） |
| `v1_0/LightTypes.idl` | 唯一共享类型文件，修改会扩散到 `ILightInterface.idl` 所有方法，Parcelable 字段顺序是 ABI 一部分 |
| `v1_0/BUILD.gn` | `hdi-gen` 生成规则，proxy/stub/headers 产出目标定义 |
| `bundle.json` | 活跃构建目标与 inner_kits 清单，新增库必须在此注册 |

### Vocabulary-based routing

当任务、issue、日志、API 名称中出现以下术语时，先理解其含义和风险再动手：

| 术语 | 含义与风险 | 读取 |
|---|---|---|
| ILightInterface | 指示灯控制接口，定义 TurnOn/TurnOff/GetLightInfo/GetLightInfoById 等方法 | `v1_0/ILightInterface.idl` |
| TurnOn | 开灯方法，接受 `HdfLightEffect` 参数控制亮度/颜色/闪烁效果 | `v1_0/ILightInterface.idl` |
| TurnOff | 关灯方法 | `v1_0/ILightInterface.idl` |
| GetLightInfo | 查询所有指示灯信息，返回 `HdfLightInfo` 列表 | `v1_0/ILightInterface.idl` |
| GetLightInfoById | 按灯 ID 查询指示灯信息 | `v1_0/ILightInterface.idl` |
| HdfLightInfo | 指示灯信息数据模型（灯 ID、类型、能力等），跨进程传输 | `v1_0/LightTypes.idl` |
| HdfLightEffect | 指示灯效果数据模型（亮度、颜色、呼吸效果等） | `v1_0/LightTypes.idl` |
| HdfLightFlashEffect | 闪烁效果数据模型（闪烁模式、持续时间等） | `v1_0/LightTypes.idl` |
| HdfLightBrightness | 亮度数据模型 | `v1_0/LightTypes.idl` |
| hdi-gen | HDI IDL 代码生成工具，将 `.idl` 转为 proxy/stub/headers | `v1_0/BUILD.gn` 中的 `hdi_gen` 调用 |
| ABI 契约 | 版本号即 ABI 契约，v1_0 proxy 不能与未来 v2_0 stub 通信 | `bundle.json` 中版本化 sub_component |
| 命名空间 | C++ 命名空间 `ohos.hdi.light.v1_0` | 生成的头文件 |
| min_hdi_proxy_version | 外围仓 `sa_profile` 中声明的 HDI 代理版本依赖（`liblight_proxy_1.0.z.so`） | 外部仓 `drivers/peripheral/light` 的 `sa_profile` |

在计划阶段，必须声明：
- **任务分类**（如：指示灯接口变更 / 共享类型变更 / 新增方法 / 构建配置变更）
- **已读取的 IDL 文件和构建配置**（具体到文件路径）
- **发现的约束**（ABI 契约、命名空间、Parcelable 字段顺序、外围仓与消费方同步需求）
- **是否需要同步修改其他层**：
  - IDL 变更 -> 由 `hdi-gen` 重生成 proxy/stub/headers -> 同步外围仓 `drivers/peripheral/light` 实现 -> 同步消费方 `battery_manager` 的 `BatteryLight` 模块
  - `LightTypes.idl` 类型变更 -> 评估 `ILightInterface.idl` 所有方法的引用
  - 新增版本 -> 同步 `bundle.json` + 外围仓 `sa_profile` 的 `min_hdi_proxy_version` + 消费方适配

## 3. 约束边界

### 架构不变量

- **`.idl` 文件是唯一真相源**：所有 proxy/stub/headers 由 `hdi-gen` 在构建时生成，本仓库不包含手写的 proxy/stub 实现代码
- **单版本 v1_0**：本仓库仅含 v1_0 一个版本，所有接口与类型集中在此，无版本分裂
- **版本号即 ABI 契约**：v1_0 proxy 不能与未来 v2_0 stub 通信，跨版本交互必须通过版本化库显式选择
- **命名空间与版本绑定**：C++ 命名空间为 `ohos.hdi.light.v1_0`，版本号嵌入命名空间
- **`LightTypes.idl` 是版本内共享类型源**：`ILightInterface.idl` 所有方法共享同一 `LightTypes.idl`，类型变更会扩散到所有方法
- **无 .gni 特性开关**：本仓库无 `*.gni` 文件，无特性开关，构建配置集中在 `bundle.json`
- **inner_kits 暴露的 headers/proxy/stub 是公共 ABI**：变更需评估所有下游消费者（`battery_manager` 的 `BatteryLight` 模块、其他指示灯消费方）
- **本仓是接口定义方**：实现由 `drivers/peripheral/light` 维护，消费逻辑由 `battery_manager` 等仓维护

### 禁止事项

- **不要手改 `hdi-gen` 生成的 proxy/stub/headers 代码**：应修改 `.idl` 源文件后由构建系统重生成
- **不要删除或重命名已发布的 IDL 方法**：IDL 方法签名是 ABI 契约，删除或重命名会破坏旧客户端（特别是 `battery_manager` 的电池指示灯）
- **不要复用或调整已有 IDL 方法的码值**：新增接口必须追加新方法，不能复用已删除方法的码值
- **不要修改 `bundle.json` 的 `sub_component` 而不同步 `inner_kits`**：新增构建目标必须同步注册 inner_kits 以暴露给下游
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审
- **不要在本仓库中实现 HDI 逻辑**：本仓是接口定义方，实现由 `drivers/peripheral/light` 维护
- **不要在本仓库中实现消费逻辑**：指示灯消费逻辑由 `battery_manager` 等仓维护

### Ask before 规则

在执行以下操作前，必须先确认或询问：
- **修改 ILightInterface.idl 方法签名**：确认下游 battery_manager 兼容性
- **修改 LightTypes.idl 类型**：确认 Parcelable 字段顺序（ABI 一部分）
- **新增主版本 v2_0**：确认外围仓 sa_profile 的 min_hdi_proxy_version 更新
- **修改 bundle.json 的 sub_component 或 inner_kits**：确认下游消费者依赖

### 需确认后再修改

- **IDL 方法签名变更**（需确认 ABI 兼容性影响和版本策略，评估所有下游消费者特别是 `battery_manager`）
- **`LightTypes.idl` 类型新增/删除/字段调整**（需评估 `ILightInterface.idl` 所有方法的引用，Parcelable 字段顺序是 ABI 一部分，`HdfLightInfo` / `HdfLightEffect` 跨进程传输）
- **新增主版本（如 v2_0）**（需确认外围仓 `drivers/peripheral/light` 的实现计划、`sa_profile` 的 `min_hdi_proxy_version` 更新、消费方 `battery_manager` 适配）
- **`bundle.json` 的 `sub_component` 或 `inner_kits` 调整**（需确认下游消费者是否依赖被移除的 inner_kits）
- **新增外部依赖**（需确认许可证、包大小、`bundle.json` 同步）

### 项目特定陷阱

- **`HdfLightInfo` / `HdfLightEffect` 字段顺序是 ABI**：IDL 中 struct 字段顺序决定 Parcelable 序列化顺序，新增字段必须追加在末尾，否则 `battery_manager` 反序列化失败导致电池指示灯异常
- **单版本无回退路径**：本仓库仅 v1_0，破坏性变更没有历史版本作为回退，需格外谨慎
- **`battery_manager` 是关键消费方**：电池指示灯依赖 `TurnOn`/`TurnOff`，接口变更需同步 `battery_manager` 的 `BatteryLight` 模块（`services/native/include/battery_light.h`）
- **`TurnOn` 的 `HdfLightEffect` 参数扩展风险**：扩展效果参数需修改 `HdfLightEffect` 类型，影响所有 `TurnOn` 调用方
- **无 .gni 特性开关**：本仓库无特性开关，所有构建配置集中在 `bundle.json`，不要期望通过 `*.gni` 裁剪功能
- **`hdi-gen` 工具版本与 IDL 语法兼容性**：升级 `hdi-gen` 工具可能引入新语法支持，但旧 IDL 语法必须保持向后兼容

### Agent failure patterns

- **Agent 在 v1_0 中复用已有码值**：Agent 可能复用已删除 IDL 方法的码值而非追加新方法，破坏 ABI 契约。必须追加新方法
- **Agent 修改 `LightTypes.idl` 字段顺序**：Agent 新增 struct 字段时可能插入到中间而非追加末尾，导致 `battery_manager` 反序列化失败。字段顺序是 ABI 一部分
- **Agent 忽略 `battery_manager` 消费方同步**：Agent 修改 `ILightInterface.idl` 时可能忘记同步 `battery_manager` 的 `BatteryLight` 模块，导致消费方编译失败
- **Agent 期望通过 .gni 裁剪功能**：本仓无 `.gni` 特性开关，Agent 可能尝试修改或添加特性开关而不理解构建配置集中在 `bundle.json`
- **Agent 假设存在历史版本回退路径**：本仓仅 v1_0，破坏性变更没有历史版本可回退，Agent 不应假设可以通过降级恢复
- **Agent 遗漏 `TurnOn` 的 `HdfLightEffect` 参数扩展影响**：Agent 扩展 `TurnOn` 参数时可能忘记修改 `HdfLightEffect` 类型，影响所有调用方

## 4. 验证闭环

### 最小验证

```bash
# 构建 light 接口组件（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target drivers_interface_light

# 单独构建活跃版本
./build.sh --product-name rk3568 --build-target //drivers/interface/light/v1_0:light_idl_target
./build.sh --product-name rk3568 --build-target //drivers/interface/light/v1_0:liblight_proxy_1.0
./build.sh --product-name rk3568 --build-target //drivers/interface/light/v1_0:liblight_stub_1.0
```

### Lint and static analysis

```bash
# 格式检查
gn format --check v1_0/

# IDL 语法检查（验证 .idl 文件格式是否符合 hdi-gen 要求）
python3 scripts/check_idl_syntax.py --repo light v1_0/

# 构建目标 lint 检查
./build.sh --product-name rk3568 --build-target drivers_interface_light --lint

# 检查生成产物是否被手改
git diff --name-only -- '**/_proxy.h' '**/_proxy.cpp' '**/_stub.h' '**/_stub.cpp' | grep -v 'BUILD.gn' | head -20

# 检查 bundle.json 与 inner_kits 一致性
python3 scripts/check_bundle_consistency.py --repo light v1_0/
```

### 测试命令

```bash
# 运行指示灯服务接口测试
./build.sh --product-name rk3568 --build-target tests --test-target light_service_test

# 运行 ABI 兼容性检查
python3 scripts/check_abi_compatibility.py --repo light --compare v1_0

# 运行 battery_manager 消费方集成测试
./build.sh --product-name rk3568 --build-target //foundation/powermgr/battery_manager --test-target battery_light_test

# 验证 TurnOn/TurnOff 接口契约
python3 scripts/check_abi_compatibility.py --repo light --verify-methods TurnOn TurnOff GetLightInfo GetLightInfoById
```

### 任务特定验证

| 任务类型 | 验证命令 |
|---|---|
| 指示灯接口变更 | `./build.sh --build-target drivers_interface_light` + 同步构建外围仓 `drivers/peripheral/light` 验证实现兼容 + 同步构建消费方 `battery_manager` 验证 `BatteryLight` 模块 + 运行指示灯服务测试 |
| 共享类型变更 | 全量构建 `drivers_interface_light` + 验证 `ILightInterface.idl` 所有方法的引用编译通过 + 跑下游消费者 ABI 检查 + `python3 scripts/check_parcelable_fields.py --repo light` |
| 新增指示灯方法 | 重新生成 proxy/stub + 同步外围仓 `drivers/peripheral/light` 实现 + 同步消费方 `battery_manager` 调用方 + 运行 ABI 兼容性检查 |
| 新增主版本 | 新建 `vN_0/` 目录 + 修改 `bundle.json` + 全量构建 + 验证外围仓 `sa_profile` 的 `min_hdi_proxy_version` 更新 + 消费方 `battery_manager` 适配 |
| 构建配置变更 | `./build.sh --build-target drivers_interface_light` + 验证 `inner_kits` 暴露的 headers 可被下游引用 |

### 静态分析 / Lint

```bash
# 代码格式与静态检查
./build.sh --product-name rk3568 --build-target drivers_interface_light --lint
gn gen --check
hdi-gen --check
```

### Done 定义

- 构建通过（`drivers_interface_light` 子系统 + v1_0 目标 + 外围仓 `drivers/peripheral/light` + 消费方 `battery_manager`）
- Lint/格式检查通过（`gn format --check` 无格式差异）
- IDL 语法检查通过（`scripts/check_idl_syntax.py` 无错误）
- bundle.json 一致性检查通过（`scripts/check_bundle_consistency.py` 无遗漏）
- 无新增编译警告
- 变更范围与任务要求一致，未夹带未关联的重构
- IDL 变更已由 `hdi-gen` 重生成 proxy/stub/headers，无手改生成产物
- 新增版本已同步 `bundle.json` 的 `sub_component` 与 `inner_kits` + 外围仓 `sa_profile` 的 `min_hdi_proxy_version`
- `LightTypes.idl` 类型变更已评估 `ILightInterface.idl` 所有方法的引用
- 活跃版本（v1_0）的变更已评估 ABI 兼容性，未删除或重命名已发布方法
- 涉及 `battery_manager` 消费方的变更已同步 `BatteryLight` 模块
- 生成产物未被手改（`git diff` 对比无 `_proxy.h/.cpp`、`_stub.h/.cpp` 变更）

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 目标版本（v1_0）及是否为活跃构建
4. 是否触发跨层同步修改（IDL 重生成 / 外围仓 `drivers/peripheral/light` / 消费方 `battery_manager` / `bundle.json` / `sa_profile`）
5. 是否影响 ABI 契约或 `battery_manager` 的电池指示灯功能
6. 是否触及架构不变量或需确认事项

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样，不能假称已通过。涉及外围仓 `drivers/peripheral/light` 或消费方 `battery_manager` 的变更，必须人工复核 IDL 与实现/消费方的匹配性并说明无法在沙箱验证的限制。
