# AGENTS.md - OpenHarmony 驱动接口层（Drivers Interface）

## 0. Where to look

| 遇到问题 | 读取路径 |
|----------|----------|
| 不确定哪个子域负责 | 顶部「子域索引」表 |
| 需要知道IDL定义 | `drivers/interface/*/*.idl` |
| 需要知道bundle配置 | `drivers/interface/*/bundle.json` |
| 需要构建命令 | 顶部「各子域构建命令」表 |
| 需要术语→文件映射 | 顶部「术语路由」表 |
| 需要版本路由 | 顶部「按接口版本路由」表 |
| 需要inner_kits变更验证 | `bundle.json` inner_kits 列表 |

## 1. 代码地图

本仓库是 OpenHarmony 驱动接口层，定义各外设驱动与上层服务之间的 IDL 契约。每个子域包含版本化的 `.idl` 接口定义文件，由 `hdi-gen` 工具在构建时生成 proxy/stub/headers。本仓是接口定义方，不包含实现代码——实现由 `drivers/peripheral/*` 仓维护。

### 任务到路径映射

| 任务类型 | 入口路径 | 说明 |
|----------|----------|------|
| 音频接口变更 | `drivers/interface/audio/` | 音频管理器/适配器/渲染/采集/效果 |
| 编解码接口变更 | `drivers/interface/codec/` | 编解码/图像/Z-Codec |
| 显示接口变更 | `drivers/interface/display/` | 合成/缓冲/图形通用 |
| 指示灯接口变更 | `drivers/interface/light/` | 指示灯控制 |
| 动作接口变更 | `drivers/interface/motion/` | 动作手势检测 |
| 传感器接口变更 | `drivers/interface/sensor/` | 传感器数据采集/转换 |
| 工具/编译器变更 | `drivers/interface/tools/` | HDF配置编译器(hc-gen) |
| 振动接口变更 | `drivers/interface/vibrator/` | 振动/HD触觉/会话/热插拔 |

### 子域索引

| 子域 | 功能 | AGENTS.md | 版本 | 内核工具 | 活跃构建 |
|------|------|-----------|------|----------|----------|
| [audio](audio/AGENTS.md) | 音频管理器/适配器/渲染/采集/效果 | [查看](audio/AGENTS.md) | v6_0/v6_1/effect/v1_0 | `audio.gni` (3个开关) | 3个活跃目标 |
| [codec](codec/AGENTS.md) | 编解码/图像/Z-Codec | [查看](codec/AGENTS.md) | v4_0/image/v2_1/zcodec/v1_0 | `codec.gni` | 3个活跃目标 |
| [display](display/AGENTS.md) | 合成/缓冲/图形通用 | [查看](display/AGENTS.md) | composer v1_0-v1_5 / buffer v1_0-v1_4 / graphic/common v1_0-v2_3 | `display_config.gni` (3个开关) | 30+ sub_component |
| [light](light/AGENTS.md) | 指示灯控制 | [查看](light/AGENTS.md) | v1_0 | 无 .gni 特性开关 | 1个活跃目标 |
| [motion](motion/AGENTS.md) | 动作手势检测 | [查看](motion/AGENTS.md) | v1_0/v1_1 | 无 .gni 特性开关 | 2个活跃目标 |
| [sensor](sensor/AGENTS.md) | 传感器数据采集/转换 | [查看](sensor/AGENTS.md) | v3_0/v3_1/convert/v1_0 | 无 .gni 特性开关 | 3个活跃目标 |
| [tools](tools/AGENTS.md) | HDF配置编译器(hc-gen) | [查看](tools/AGENTS.md) | v6.0 | 无依赖 | 1个活跃目标 |
| [vibrator](vibrator/AGENTS.md) | 振动/HD触觉/会话/热插拔 | [查看](vibrator/AGENTS.md) | v2_0 | `vibrator.gni` | 1个活跃目标 |

### 架构边界

- **`.idl` 文件是唯一真相源**：所有 proxy/stub/headers 由 `hdi-gen` 在构建时生成
- **本仓**：仅维护 `.idl` 源文件和 `bundle.json`/`BUILD.gn` 构建配置
- **实现方**：`drivers/peripheral/*` 仓维护 HDI 服务实现
- **消费方**：上层服务（audio_service、multimedia、SensorService、miscservices 等）

### 非本项目维护的目录

- 历史版本目录（`v1_0/` 至 `v5_0/` 等）：不在活跃构建中，仅保留兼容性参考
- 各版本目录下 `BUILD.gn` 引用的生成产物（proxy/stub）：由 `hdi-gen` 生成
- HDI 实现由 `drivers/peripheral/*` 仓维护

## 2. 知识路由

### 计划阶段声明

在计划阶段，必须声明以下内容：
1. 目标子域及版本
2. 是否需要新增/修改 `.idl` 方法
3. 是否需要新增版本目录或修改 `bundle.json`
4. 是否需要同步 inner_kits
5. 是否影响 ABI 契约

### 按子域路由

| 任务类型 | 子域 | 读取 |
|----------|------|------|
| 音频接口变更 | audio | [audio/AGENTS.md](audio/AGENTS.md) |
| 编解码接口变更 | codec | [codec/AGENTS.md](codec/AGENTS.md) |
| 显示接口变更 | display | [display/AGENTS.md](display/AGENTS.md) |
| 指示灯接口变更 | light | [light/AGENTS.md](light/AGENTS.md) |
| 动作接口变更 | motion | [motion/AGENTS.md](motion/AGENTS.md) |
| 传感器接口变更 | sensor | [sensor/AGENTS.md](sensor/AGENTS.md) |
| 工具/编译器变更 | tools | [tools/AGENTS.md](tools/AGENTS.md) |
| 振动接口变更 | vibrator | [vibrator/AGENTS.md](vibrator/AGENTS.md) |

### 按任务类型路由

| 任务类型 | 操作 |
|----------|------|
| 新增/修改 IDL 方法 | 读对应子域 AGENTS.md → `.idl` 文件 → `hdi-gen` 重生成 |
| 新增版本 | 读对应子域 AGENTS.md → 新建版本目录 → 修改 `bundle.json` |
| 特性开关变更 | 读对应子域 AGENTS.md → `*.gni` + `bundle.json` features |
| ABI 兼容性变更 | 读对应子域 AGENTS.md 的约束边界部分 |
| 构建配置变更 | 读对应子域 AGENTS.md 的构建配置部分 |
| inner_kits 变更 | 读对应子域 AGENTS.md → `bundle.json` inner_kits 列表 |

### 按接口版本路由

| 版本 | 子域 | 说明 |
|------|------|------|
| v6_0/v6_1 | audio | 当前主稳定版本和最新版本 |
| effect/v1_0 | audio | 音频效果子域 |
| v4_0 | codec | 当前主版本 |
| image/v2_1 | codec | 当前活跃图像版本 |
| zcodec/v1_0 | codec | Z-Codec（使用 HdiZ 前缀） |
| v1_0-v1_5 | display/composer | 全部活跃 |
| v1_0-v1_4 | display/buffer | 全部活跃 |
| v1_0-v2_3 | display/graphic/common | 仅类型定义 |
| v2_0 | vibrator | 当前唯一活跃版本 |
| v3_0/v3_1 | sensor | 当前活跃版本 |
| convert/v1_0 | sensor | 传感器转换适配层 |

### 术语路由

| 术语 | 含义 | 读取路径 |
|------|------|----------|
| `.idl` 文件 | 接口定义语言源文件 | `drivers/interface/*/*.idl` |
| `bundle.json` | 组件配置文件，含 inner_kits、sub_component | `drivers/interface/*/bundle.json` |
| `hdi-gen` | IDL代码生成工具 | 构建系统自动调用 |
| proxy | 上层调用代理 | 由 `hdi-gen` 生成 |
| stub | 下层实现桩 | 由 `hdi-gen` 生成 |
| inner_kits | 内部SDK组件列表 | `bundle.json` 内字段 |
| sub_component | 子组件列表 | `bundle.json` 内字段 |
| 特性开关 | 编译特性控制 | `*.gni` + `bundle.json` features |
| BUILD.gn | 构建配置 | `drivers/interface/*/BUILD.gn` |
| sa_profile | 安全属性配置 | `bundle.json` sa_profile 字段 |
| version | 接口版本号 | 子域目录名（如 v1_0、v2_0） |
| proxy_deps/stub_deps | 依赖回调接口 | `.idl` 文件中的依赖声明 |
| innerapi_tags | 内部API标签 | `bundle.json` innerapi_tags 字段 |
| HdiZ | Z-Codec前缀 | `drivers/interface/codec/zcodec/v1_0/` |
| hc-gen | HDF配置编译器 | `drivers/interface/tools/hc-gen` |

## 3. 约束边界

### 计划阶段必须声明

开始任何任务前，必须声明：
- 目标子域和版本
- 是否修改 `.idl` 方法签名
- 是否新增版本目录
- 是否同步 `bundle.json` inner_kits
- 是否影响 ABI 契约

### 通用约束

- **`.idl` 文件是唯一真相源**：不要手改 `hdi-gen` 生成的 proxy/stub/headers
- **版本号即 ABI 契约**：跨版本交互必须通过版本化库显式选择
- **不要删除或重命名已发布的 IDL 方法**：IDL 方法签名是 ABI 契约
- **不要复用或调整已有 IDL 方法的码值**：新增接口必须追加新方法
- **不要修改 `bundle.json` 的 `sub_component` 而不同步 `inner_kits`**
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审
- **不要在本仓库中实现 HDI 逻辑**：本仓是接口定义方

### Ask-before 规则

在执行以下操作前，必须先确认：

| 操作 | 确认内容 | 确认对象 |
|------|----------|----------|
| 修改 `bundle.json` 的 `sub_component` | `inner_kits` 是否同步更新 | 项目负责人 |
| 新增/删除 IDL 方法 | 是否影响 ABI 契约 | 接口评审团队 |
| 新增版本目录 | 版本号是否与 `bundle.json` 一致 | 项目负责人 |
| 修改特性开关默认值 | 是否影响其他子域构建 | 构建团队 |
| 修改 `inner_kits` 列表 | 是否同步 `sub_component` 和 `sa_profile` | 项目负责人 |
| 引入新的生产依赖 | 是否经过 `bundle.json` 评审 | 依赖评审团队 |
| 在 community 构建中启用功能 | 是否已检查 `rich_device` 专属功能标记 | 构建团队 |
| 修改 `BUILD.gn` 的 deps | 是否引入新的外部依赖 | 依赖评审团队 |
| 修改 `innerapi_tags` | 是否影响芯片SDK/平台SDK兼容性 | 芯片适配团队 |
| 重命名 IDL 文件 | 是否影响所有引用该文件的消费者 | 所有消费方 |

### Agent 失败模式

| 失败模式 | 描述 | 预防 |
|----------|------|------|
| 手改生成代码 | 直接修改 `hdi-gen` 生成的 proxy/stub/headers | 始终修改 `.idl` 源文件后由构建系统重生成 |
| 版本混淆 | 在 v1_0 中引入 v2_0 特有功能，或在 legacy 版本中新增能力 | 确认目标版本活跃性，参考「按接口版本路由」 |
| ABI 破坏 | 修改 IDL 中字段顺序或方法签名 | 跨版本交互必须通过版本化库显式选择 |
| 跨层同步遗漏 | 修改了 IDL 但未同步 `drivers/peripheral/*` 实现 | 修改 IDL 后必须验证外围仓实现 |
| inner_kits 不同步 | 修改 `sub_component` 但未同步 `inner_kits` | 修改后必须验证 `bundle.json` 字段一致性 |
| 依赖遗漏 | 新增依赖但未在 `bundle.json` 中声明 | 所有生产依赖必须经过 `bundle.json` 评审 |
| 构建类型混淆 | 在 community 构建中引入 rich_device 专属功能 | 检查 `OAT.xml` 和 `BUILD.gn` 的构建类型 |
| sa_profile 遗漏 | 修改 `bundle.json` 但未同步 `sa_profile` | 修改 `bundle.json` 后验证 `sa_profile` 字段 |
| 码值复用 | 复用已有 IDL 方法的码值 | 新增接口必须追加新方法 |
| innerapi_tags 遗漏 | 修改 `innerapi_tags` 但未考虑芯片SDK/平台SDK兼容性 | 修改前确认 `innerapi_tags` 影响范围 |

### 子域特定约束

- **audio**: 特性开关在 interface 与 peripheral 双向对应（`audio.gni` ↔ `drivers/peripheral/audio/audio.gni`）
- **codec**: zcodec 使用 `HdiZ` 前缀区别于主 codec 的 `I` 前缀；`CodecExtTypes.idl` 与 `CodecTypes.idl` 分层
- **display**: 所有版本全部活跃构建，3 子域版本号不联动；`cache_manager`/`hdifd_parcelable` 跨版本共享
- **motion**: v1_1 复用 v1_0 的 `IMotionCallback.idl`（通过 `proxy_deps`/`stub_deps`），不要在 v1_1 目录新增回调文件
- **sensor**: v3_1 复用 v3_0 的回调接口（通过 `proxy_deps`/`stub_deps`），`innerapi_tags = ["chipsetsdk", "platformsdk_indirect"]`
- **tools**: `hc-gen` 是独立构建工具，`deps.components = []`，不要引入对 OpenHarmony 组件的依赖
- **vibrator**: `hilog_lite` 是唯一依赖 hilog_lite 的接口仓；v2_0 是 v1_0-v1_3 的整合超集

## 4. 验证闭环

### 各子域构建命令

| 子域 | 构建命令 | 单独构建活跃版本 |
|------|----------|-----------------|
| audio | `./build.sh --product-name rk3568 --build-target drivers_interface_audio` | `//drivers/interface/audio/v6_0:libaudio_proxy_6.0` |
| codec | `./build.sh --product-name rk3568 --build-target drivers_interface_codec` | `//drivers/interface/codec/v4_0:codec_idl_target` |
| display | `./build.sh --product-name rk3568 --build-target drivers_interface_display` | `//drivers/interface/display/composer/v1_5:display_composer_idl_target` |
| light | `./build.sh --product-name rk3568 --build-target drivers_interface_light` | `//drivers/interface/light/v1_0:light_idl_target` |
| motion | `./build.sh --product-name rk3568 --build-target drivers_interface_motion` | `//drivers/interface/motion/v1_0:motion_idl_target` |
| sensor | `./build.sh --product-name rk3568 --build-target drivers_interface_sensor` | `//drivers/interface/sensor/v3_0:sensor_idl_target` |
| tools | `./build.sh --product-name rk3568 --build-target drivers_tools` | `//drivers/interface/tools/hc-gen:hc_gen` |
| vibrator | `./build.sh --product-name rk3568 --build-target drivers_interface_vibrator` | `//drivers/interface/vibrator/v2_0:vibrator_idl_target` |

### 任务特定验证表

| 任务类型 | 验证命令 | 验证目标 |
|----------|----------|----------|
| 新增/修改 IDL 方法 | `./build.sh --product-name rk3568 --build-target drivers_interface_[module] --lint && hdi-gen --check` | IDL 生成一致性 |
| 新增版本目录 | `./build.sh --product-name rk3568 --build-target drivers_interface_[module]` | 构建通过，版本号一致 |
| 特性开关变更 | `./build.sh --product-name rk3568 --build-target drivers_interface_[module] --lint` | bundle.json 同步 |
| inner_kits 变更 | 检查 `bundle.json` 中 sub_component 和 inner_kits 列表 | 字段一致性 |
| ABI 兼容性变更 | `gn gen --check && hdi-gen --check` | 配置正确性 |
| 构建配置变更 | `gn gen --check && hdi-gen --check` | 配置正确性 |
| hc-gen 工具变更 | `./build.sh --product-name rk3568 --build-target drivers_tools` | 工具构建通过 |
| innerapi_tags 变更 | 检查 `bundle.json` innerapi_tags 字段 | 芯片SDK/平台SDK兼容性 |

### 通用验证步骤

```bash
# 构建接口组件
./build.sh --product-name rk3568 --build-target drivers_interface_[module]

# 静态分析（Lint）
./build.sh --product-name rk3568 --build-target drivers_interface_[module] --lint
gn gen --check
hdi-gen --check
clang-format --dry-run --Werror <子域目录>
cppcheck --enable=all --error-exitcode=1 <子域目录>

# 验证 inner_kits
# 检查 bundle.json 中 sub_component 和 inner_kits 列表
python3 -c "import json; d=json.load(open('bundle.json')); assert 'inner_kits' in d; print('inner_kits:', d['inner_kits'])"

# 验证 sa_profile
# 检查 bundle.json 中 sa_profile 字段
python3 -c "import json; d=json.load(open('bundle.json')); assert 'sa_profile' in d; print('sa_profile:', d['sa_profile'])"

# 验证 IDL 一致性
hdi-gen --check

# 验证版本号与 bundle.json 一致
grep -r "version" bundle.json
```

### Done 定义

任务完成的判定标准：
1. 构建命令执行成功（`./build.sh --product-name rk3568 --build-target drivers_interface_[module]` 返回 0）
2. 静态分析通过（`--lint` 返回 0，`gn gen --check` 通过，`hdi-gen --check` 通过）
3. `bundle.json` 中 `sub_component` 和 `inner_kits` 同步更新
4. `sa_profile` 字段已同步更新（如适用）
5. `innerapi_tags` 影响范围已确认（如适用）
6. 跨层同步已验证（IDL 与外围仓 `drivers/peripheral/*` 实现匹配）
7. 最终报告已输出（包含文件清单、验证结果、确认事项）

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 目标子域与版本及是否为活跃构建
4. 是否触发跨层同步修改（IDL 重生成 / 外围仓 `drivers/peripheral/*` / `bundle.json` / `sa_profile`）
5. 是否影响特性开关默认值或 ABI 契约
6. 是否触及架构不变量或需确认事项
7. 所有验证命令的执行结果（构建/测试/lint/静态分析）

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样。涉及外围仓 `drivers/peripheral/*` 的变更，必须人工复核 IDL 与实现的匹配性并说明无法在沙箱验证的限制。
