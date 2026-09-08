# AGENTS.md - OpenHarmony 编解码 HDI 接口（drivers_interface_codec）

## 1. 代码地图

本仓库定义 OpenHarmony 编解码 HDI 接口（@ohos/drivers_interface_codec，版本 4.0，subsystem `hdf`），是编解码驱动与媒体服务层之间的 IDL 契约。核心职责是定义编解码组件管理器（ICodecComponentManager）、编解码组件（ICodecComponent）、编解码回调（ICodecCallback）、编解码图像（ICodecImage）以及 Z 编解码（HdiZFactory/HdiZComponent/HdiZCallback）的接口契约。最重要的架构边界是**`.idl` 文件是唯一真相源，proxy/stub/headers 由 `hdi-gen` 工具在构建时生成，本仓库包含三个独立版本化的子域（主 codec、image、zcodec）以及跨版本共享的 parcelable 类型层**。

### 非本项目维护的目录

以下目录属于其他团队或生成产物，不属于本项目维护范围，修改时请跳过：

- `v1_0/` 至 `v3_0/`：历史版本主 codec IDL，未在当前 `bundle.json` 的 `sub_component` 中列出，仅保留兼容性参考
- `image/v1_0/`、`image/v2_0/`：历史版本图像 IDL，未在活跃构建中
- 各版本目录下 `BUILD.gn` 引用的生成产物（proxy `_proxy.h/.cpp`、stub `_stub.h/.cpp`）：由 `hdi-gen` 在构建时生成，**不要手改**
- 编解码 HDI 实现由 `drivers/peripheral/codec` 仓维护，本仓是接口定义方，不包含实现

### 嵌套指引

本仓目前无嵌套 AGENTS.md / CLAUDE.md / rules / skills 文件。以下子目录可按需新建嵌套 AGENTS.md 以提供更具体的指引：

- `v4_0/`：当前活跃主版本，含 `CodecExtTypes.idl`（v2_0+ 扩展类型），可新建 `v4_0/AGENTS.md` 聚焦 `CodecExtTypes` 与 `CodecTypes` 的分层、ABI 稳定性
- `image/v2_1/`：当前活跃图像版本，与主 codec 独立版本管理，可新建 `image/v2_1/AGENTS.md` 聚焦图像编解码的独立版本演进、`CodecImageType.idl` 与主 `CodecTypes.idl` 的关系
- `zcodec/v1_0/`：Z 编解码子域，使用 `HdiZ` 前缀命名约定（区别于主 codec 的 `I` 前缀），可新建 `zcodec/v1_0/AGENTS.md` 聚焦 Z 编解码的工厂模式、命名约定差异
- `parcelable/`：跨版本共享的 Parcelable 类型层，可新建 `parcelable/AGENTS.md` 聚焦跨版本类型共享约束、Parcelable 序列化 ABI

### 关键区域

- `v1_0/`：历史版本，含 `ICodecComponentManager.idl`、`ICodecComponent.idl`、`ICodecCallback.idl`、`CodecTypes.idl`（无 `CodecExtTypes.idl`），**未在活跃构建中**
- `v2_0/` 至 `v3_0/`：历史版本，v2_0 起新增 `CodecExtTypes.idl`，**未在活跃构建中**
- `v4_0/`：当前活跃主版本（活跃构建，产出 `codec_idl_target` / `libcodec_proxy_4.0` / `libcodec_stub_4.0` / `codec_idl_headers`），含 `ICodecComponentManager.idl`、`ICodecComponent.idl`、`ICodecCallback.idl`、`CodecTypes.idl`、`CodecExtTypes.idl`
- `image/v1_0/`、`image/v2_0/`：历史版本图像 IDL，**未在活跃构建中**
- `image/v2_1/`：当前活跃图像版本（活跃构建，产出 `image_idl_target` / `libimage_proxy_2.1` / `libimage_stub_2.1` / `image_idl_headers`），含 `ICodecImage.idl`、`CodecImageType.idl`
- `zcodec/v1_0/`：Z 编解码 IDL（活跃构建，产出 `zcodec_idl_target` / `libzcodec_proxy_1.0` / `libzcodec_stub_1.0` / `zcodec_idl_headers`），含 `HdiZFactory.idl`、`HdiZComponent.idl`、`HdiZCallback.idl`、`HdiZTypes.idl`，使用 `HdiZ` 前缀命名约定
- `parcelable/`：跨版本共享 Parcelable 类型层（活跃构建，产出 `video_parcel_headers`），含 `error_type.h`、`key_value.h`、`parcelable_buffer.h`、`parcelable_fd.h`、`parcelable_param.h`
- `bundle.json`：组件元数据、`sub_component`（3 个活跃目标）、`inner_kits`（10 个 inner_kits 条目：proxy/stub/headers + parcelable）

### Where to look

| 任务类型 | 先看哪里 |
|---|---|
| 主 codec 接口变更 | `v4_0/ICodecComponentManager.idl` + `ICodecComponent.idl` + `ICodecCallback.idl` + `CodecTypes.idl` + `CodecExtTypes.idl` + `v4_0/BUILD.gn` |
| 图像编解码接口变更 | `image/v2_1/ICodecImage.idl` + `image/v2_1/CodecImageType.idl` + `image/v2_1/BUILD.gn` |
| Z 编解码接口变更 | `zcodec/v1_0/HdiZFactory.idl` + `HdiZComponent.idl` + `HdiZCallback.idl` + `HdiZTypes.idl` + `zcodec/v1_0/BUILD.gn` |
| 共享类型变更（主 codec） | `v4_0/CodecTypes.idl`（基础类型）+ `v4_0/CodecExtTypes.idl`（v2_0+ 扩展类型） |
| 跨版本 Parcelable 类型 | `parcelable/` 下 `.h` 文件（`parcelable_buffer.h` / `parcelable_fd.h` / `parcelable_param.h` / `key_value.h` / `error_type.h`） |
| 新增主版本 | 复制 `v4_0/` 作为模板 -> 新建 `vN_0/` -> 修改 `bundle.json` 的 `sub_component` 与 `inner_kits` -> 同步外围仓 |
| 扩展类型新增 | `CodecExtTypes.idl`（v2_0+），不要塞入 `CodecTypes.idl` |
| Z 编解码命名约定 | zcodec 使用 `HdiZ` 前缀（`HdiZFactory` / `HdiZComponent`），区别于主 codec 的 `I` 前缀（`ICodecComponentManager`） |
| 构建配置 | `bundle.json`（sub_component、inner_kits、deps）+ 各版本 `BUILD.gn` |

### 架构分层

```
媒体服务层（multimedia_player 等上层服务，外部仓）
  ↓ 调用 proxy 库
IDL 代理层（本仓生成产物）
  libcodec_proxy_4.0 (v4_0)
  libimage_proxy_2.1 (image/v2_1)
  libzcodec_proxy_1.0 (zcodec/v1_0)
  ↓ IPC（HDF IPC 框架）
IDL 桩层（本仓生成产物）
  libcodec_stub_4.0 / libimage_stub_2.1 / libzcodec_stub_1.0
  ↓ 由 drivers/peripheral/codec 实现
HDI 实现层（drivers/peripheral/codec，外部仓）

跨版本共享层
  parcelable/ (video_parcel_headers，被所有版本引用)
```

## 2. 知识路由

在规划或编辑前，先对任务分类，读取对应的 IDL 文件和构建配置。

### Task-based routing

| 任务类型 | 读取 |
|---|---|
| 主 codec 接口新增/修改 | `v4_0/ICodecComponentManager.idl` + `ICodecComponent.idl` + `ICodecCallback.idl` + `v4_0/CodecTypes.idl` + `v4_0/CodecExtTypes.idl` + `v4_0/BUILD.gn` |
| 图像编解码接口变更 | `image/v2_1/ICodecImage.idl` + `image/v2_1/CodecImageType.idl` + `image/v2_1/BUILD.gn` + 检查主 codec 是否引用图像类型 |
| Z 编解码接口变更 | `zcodec/v1_0/HdiZFactory.idl` + `HdiZComponent.idl` + `HdiZCallback.idl` + `HdiZTypes.idl` + `zcodec/v1_0/BUILD.gn` |
| 共享类型变更（基础） | `v4_0/CodecTypes.idl`（基础类型，影响同版本所有接口） |
| 共享类型变更（扩展） | `v4_0/CodecExtTypes.idl`（v2_0+ 扩展类型，影响同版本所有接口） |
| 跨版本 Parcelable 类型变更 | `parcelable/parcelable_buffer.h` + `parcelable_fd.h` + `parcelable_param.h` + `key_value.h` + `error_type.h` + `parcelable/BUILD.gn`（影响所有版本） |
| 新增主版本 | `v4_0/` 作为模板 -> 新建 `vN_0/` -> 修改 `bundle.json` 的 `sub_component` 与 `inner_kits` -> 同步外围仓 `drivers/peripheral/codec` |
| 新增图像版本 | `image/v2_1/` 作为模板 -> 新建 `image/vN_0/` -> 修改 `bundle.json`（image 版本独立于主 codec 版本） |
| Z 编解码命名约定适配 | zcodec 使用 `HdiZ` 前缀，与主 codec 的 `I` 前缀不同，新增接口需遵循对应子域命名约定 |
| 构建配置 | `bundle.json`（sub_component、inner_kits、deps）+ 各子域 `BUILD.gn` |

### Path-based routing

| 修改路径 | 需了解的上下文 |
|---|---|
| `v4_0/` | 当前活跃主版本，被 `sa_profile` 的 `min_hdi_proxy_version` 引用为 `libcodec_proxy_4.0.z.so`，破坏性变更会影响所有编解码服务消费者 |
| `v1_0/` 至 `v3_0/` | 历史版本，未在活跃构建中，修改无构建影响但破坏 ABI 会导致旧设备无法升级，**仅在兼容性修复时进入** |
| `v4_0/CodecExtTypes.idl` | v2_0+ 扩展类型，与 `CodecTypes.idl` 分层，扩展类型应在此新增而非塞入 `CodecTypes.idl` |
| `image/v2_1/` | 当前活跃图像版本，独立于主 codec 版本演进，`CodecImageType.idl` 与主 `CodecTypes.idl` 不共享 |
| `image/v1_0/`、`image/v2_0/` | 历史图像版本，未在活跃构建中 |
| `zcodec/v1_0/` | Z 编解码子域，使用 `HdiZ` 前缀命名约定，与主 codec 的 `I` 前缀不同，工厂模式入口 |
| `parcelable/` | 跨版本共享 Parcelable 类型层，被所有活跃版本引用，变更影响全部子域 |
| `bundle.json` | 活跃构建目标与 inner_kits 清单，新增版本或新增库必须在此注册 |

### Vocabulary-based routing

当任务、issue、日志、API 名称中出现以下术语时，先理解其含义和风险再动手：

| 术语 | 含义与风险 | 读取 |
|---|---|---|
| ICodecComponentManager | 编解码组件管理器接口，组件的创建与销毁入口 | `v4_0/ICodecComponentManager.idl` |
| ICodecComponent | 编解码组件接口，单个编解码实例的控制 | `v4_0/ICodecComponent.idl` |
| ICodecCallback | 编解码回调接口，事件上报（如 EOS、error、buffer 回调） | `v4_0/ICodecCallback.idl` |
| ICodecImage | 编解码图像接口，图像解码/编码的扩展能力 | `image/v2_1/ICodecImage.idl` |
| HdiZFactory | Z 编解码工厂接口，使用 `HdiZ` 前缀，区别于 `I` 前缀 | `zcodec/v1_0/HdiZFactory.idl` |
| HdiZComponent | Z 编解码组件接口 | `zcodec/v1_0/HdiZComponent.idl` |
| HdiZCallback | Z 编解码回调接口 | `zcodec/v1_0/HdiZCallback.idl` |
| CodecTypes | 主 codec 基础共享类型（编解码格式、参数、buffer 描述等） | `v4_0/CodecTypes.idl` |
| CodecExtTypes | 主 codec 扩展类型（v2_0+ 新增，非 v1_0 内容） | `v4_0/CodecExtTypes.idl` |
| CodecImageType | 图像编解码共享类型 | `image/v2_1/CodecImageType.idl` |
| HdiZTypes | Z 编解码共享类型 | `zcodec/v1_0/HdiZTypes.idl` |
| parcelable | 跨版本共享的 Parcelable 类型层 | `parcelable/` 下 `.h` 文件 |
| hdi-gen | HDI IDL 代码生成工具，将 `.idl` 转为 proxy/stub/headers | 各版本 `BUILD.gn` 中的 `hdi_gen` 调用 |
| ABI 契约 | 版本号即 ABI 契约，v4_0 proxy 不能与 v3_0 stub 通信 | `bundle.json` 中版本化 sub_component |
| 命名空间 | C++ 命名空间 `ohos.hdi.codec.vN_0`（主）/ `ohos.hdi.codec.image.vN_0`（图像）/ `ohos.hdi.zcodec.vN_0`（Z） | 各版本生成的头文件 |
| min_hdi_proxy_version | 外围仓 `sa_profile` 中声明的 HDI 代理版本依赖 | 外部仓 `drivers/peripheral/codec` 的 `sa_profile` |

在计划阶段，必须声明：
- **任务分类**（如：主 codec 接口变更 / 图像接口变更 / Z 编解码接口变更 / 共享类型变更 / Parcelable 变更 / 新增版本 / 构建配置变更）
- **目标子域与版本**（主 codec v4_0 / image v2_1 / zcodec v1_0 / parcelable / 历史版本），明确是否为活跃构建
- **已读取的 IDL 文件和构建配置**（具体到文件路径）
- **发现的约束**（ABI 契约、命名约定差异、`CodecExtTypes` 分层、Parcelable 跨版本共享、外围仓同步需求）
- **是否需要同步修改其他层**：
  - IDL 变更 -> 由 `hdi-gen` 重生成 proxy/stub/headers -> 同步外围仓 `drivers/peripheral/codec` 实现
  - Parcelable 变更 -> 影响所有活跃版本 -> 同步外围仓与下游媒体服务
  - 新增版本 -> 同步 `bundle.json` + 外围仓 `sa_profile` 的 `min_hdi_proxy_version` + 下游消费者

## 3. 约束边界

### 架构不变量

- **`.idl` 文件是唯一真相源**：所有 proxy/stub/headers 由 `hdi-gen` 在构建时生成，本仓库不包含手写的 proxy/stub 实现代码
- **三个子域独立版本管理**：主 codec（v1_0-v4_0）、image（v1_0-v2_1）、zcodec（v1_0）各自独立演进，版本号不联动
- **版本号即 ABI 契约**：`v4_0` proxy 不能与 `v3_0` stub 通信，跨版本交互必须通过版本化库显式选择
- **命名空间与版本绑定**：主 codec 命名空间 `ohos.hdi.codec.vN_0`，image 命名空间 `ohos.hdi.codec.image.vN_0`，zcodec 命名空间 `ohos.hdi.zcodec.vN_0`
- **`CodecExtTypes.idl` 与 `CodecTypes.idl` 分层**：v2_0 起新增的扩展类型应放入 `CodecExtTypes.idl`，`CodecTypes.idl` 保留 v1_0 兼容的基础类型
- **zcodec 使用 `HdiZ` 前缀命名约定**：区别于主 codec 的 `I` 前缀，新增 zcodec 接口必须遵循 `HdiZ` 前缀
- **`parcelable/` 是跨版本共享层**：被所有活跃版本引用，变更影响全部子域，需评估全仓影响
- **活跃构建仅含 v4_0 / image v2_1 / zcodec v1_0 / parcelable**：`bundle.json` 的 `sub_component` 列出 3 个 IDL 目标，parcelable 通过 inner_kits 暴露
- **inner_kits 暴露的 headers/proxy/stub 是公共 ABI**：变更需评估所有下游消费者（媒体服务、播放器、第三方驱动）

### 禁止事项

- **不要手改 `hdi-gen` 生成的 proxy/stub/headers 代码**：应修改 `.idl` 源文件后由构建系统重生成
- **不要删除或重命名已发布的 IDL 方法**：IDL 方法签名是 ABI 契约，删除或重命名会破坏旧客户端
- **不要将扩展类型塞入 `CodecTypes.idl`**：v2_0+ 扩展类型应放入 `CodecExtTypes.idl`，保持基础类型的 v1_0 兼容性
- **不要在 zcodec 中使用 `I` 前缀命名**：zcodec 子域统一使用 `HdiZ` 前缀，混用会破坏命名一致性
- **不要在历史版本（v1_0-v3_0、image v1_0-v2_0）中引入新特性**：历史版本仅用于兼容性，新特性应在新版本中开发
- **不要跨子域共享类型而不显式 import**：主 codec、image、zcodec 的类型文件不自动共享，跨子域引用需显式 import
- **不要修改 `parcelable/` 而不评估全仓影响**：Parcelable 类型被所有活跃版本引用，变更影响全部子域
- **不要修改 `bundle.json` 的 `sub_component` 而不同步 `inner_kits`**：新增构建目标必须同步注册 inner_kits 以暴露给下游
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审
- **不要在本仓库中实现 HDI 逻辑**：本仓是接口定义方，实现由 `drivers/peripheral/codec` 维护

### Ask before 规则

在执行以下操作前，必须先确认或询问：
- **修改 IDL 方法签名**：确认 ABI 兼容性和版本策略
- **修改 CodecTypes.idl/CodecExtTypes.idl**：确认跨版本类型分层
- **修改 parcelable 跨版本共享层**：确认所有活跃版本引用
- **新增版本**：确认外围仓 sa_profile 的 min_hdi_proxy_version 更新
- **zcodec 命名约定变更**：确认所有 HdiZ 前缀接口同步调整

### 需确认后再修改

- **IDL 方法签名变更**（需确认 ABI 兼容性影响和版本策略，评估所有下游消费者）
- **`CodecTypes.idl` / `CodecExtTypes.idl` / `CodecImageType.idl` / `HdiZTypes.idl` 类型新增/删除/字段调整**（需评估同版本所有接口的引用，Parcelable 字段顺序是 ABI 一部分）
- **`parcelable/` 下 `.h` 文件变更**（需评估所有活跃版本的引用，跨版本共享层变更影响面最大）
- **新增主版本 / 图像版本 / Z 编解码版本**（需确认外围仓 `drivers/peripheral/codec` 的实现计划、`sa_profile` 的 `min_hdi_proxy_version` 更新、下游消费者适配）
- **`bundle.json` 的 `sub_component` 或 `inner_kits` 调整**（需确认下游消费者是否依赖被移除的 inner_kits）
- **zcodec 命名约定变更**（需确认所有 zcodec 接口是否同步调整，`HdiZ` 前缀是子域约定）
- **历史版本的兼容性修复**（需确认修复是否需要回溯到所有受影响版本）
- **新增外部依赖**（需确认许可证、包大小、`bundle.json` 同步）

### 项目特定陷阱

- **`CodecExtTypes.idl` 仅存在于 v2_0+**：v1_0 没有 `CodecExtTypes.idl`，跨版本引用扩展类型会导致 v1_0 编译失败
- **zcodec 的 `HdiZ` 前缀与主 codec 的 `I` 前缀混用风险**：新增 zcodec 接口时若误用 `I` 前缀会破坏命名约定，下游消费者依赖命名约定区分子域
- **image 版本独立于主 codec 版本**：image v2_1 不对应主 codec v2_1，两者的版本号无联动关系，不要假设版本对应
- **`parcelable/` 跨版本共享的 ABI 风险**：Parcelable 类型的字段顺序是 ABI 一部分，新增字段必须追加在末尾，否则旧二进制反序列化失败
- **三个子域并行活跃构建**：主 codec v4_0、image v2_1、zcodec v1_0 同时产出 proxy/stub 库，外围仓需明确选择依赖子域与版本
- **历史版本 IDL 仍占命名空间**：v1_0-v3_0 与 image v1_0-v2_0 虽未活跃构建，但其命名空间仍被旧二进制引用，删除会导致符号冲突
- **`CodecTypes.idl` 与 `CodecExtTypes.idl` 的分层边界模糊**：开发者可能误将扩展类型塞入基础类型文件，需在 code review 时严格区分
- **`hdi-gen` 工具版本与 IDL 语法兼容性**：升级 `hdi-gen` 工具可能引入新语法支持，但旧 IDL 语法必须保持向后兼容

### Agent failure patterns

- **Agent 手改生成产物**：Agent 可能直接编辑各版本目录下 `BUILD.gn` 引用的 `_proxy.h/.cpp`、`_stub.h/.cpp` 文件。必须拒绝此行为，应修改 `.idl` 源文件后由构建系统重生成
- **Agent 混淆三个子域的版本号**：Agent 可能假设 image v2_1 对应主 codec v2_1，或 zcodec v1_0 对应主 codec v1_0。实际上三子域独立演进，版本号无联动关系
- **Agent 将扩展类型塞入 CodecTypes.idl**：Agent 可能将 v2_0+ 新增的扩展类型放入 `CodecTypes.idl` 而非 `CodecExtTypes.idl`，破坏 v1_0 兼容性
- **Agent 在 zcodec 中使用 `I` 前缀**：Agent 可能为 zcodec 新增接口使用 `ICodec...` 命名而非 `HdiZ...` 前缀，破坏子域命名约定
- **Agent 遗漏 Parcelable 字段顺序**：Agent 修改 `parcelable/` 下 `.h` 文件时可能在中间插入字段，导致所有活跃版本反序列化失败
- **Agent 跨子域共享类型而不显式 import**：Agent 可能假设主 codec、image、zcodec 类型自动共享，导致缺少 import 编译失败
- **Agent 忽略三个子域并行构建**：Agent 可能只构建主 codec 而忽略 image/zcodec/parcelable，导致下游依赖缺失

## 4. 验证闭环

### 最小验证

```bash
# 构建 codec 接口组件（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target drivers_interface_codec

# 单独构建活跃子域
./build.sh --product-name rk3568 --build-target //drivers/interface/codec/v4_0:codec_idl_target
./build.sh --product-name rk3568 --build-target //drivers/interface/codec/image/v2_1:image_idl_target
./build.sh --product-name rk3568 --build-target //drivers/interface/codec/zcodec/v1_0:zcodec_idl_target
./build.sh --product-name rk3568 --build-target //drivers/interface/codec/parcelable:video_parcel_headers
```

### Lint and static analysis

```bash
# 格式检查
gn format --check v1_0/ v2_0/ v3_0/ v4_0/ image/v1_0/ image/v2_0/ image/v2_1/ zcodec/v1_0/ parcelable/

# IDL 语法检查（验证 .idl 文件格式是否符合 hdi-gen 要求）
python3 scripts/check_idl_syntax.py --repo codec v1_0/ v2_0/ v3_0/ v4_0/ image/v2_1/ zcodec/v1_0/

# 构建目标 lint 检查
./build.sh --product-name rk3568 --build-target drivers_interface_codec --lint

# 检查 Parcelable 类型字段顺序（确保新增字段追加在末尾）
python3 scripts/check_parcelable_fields.py --repo codec parcelable/

# 检查生成产物是否被手改
git diff --name-only -- '**/_proxy.h' '**/_proxy.cpp' '**/_stub.h' '**/_stub.cpp' | grep -v 'BUILD.gn' | head -20
```

### 测试命令

```bash
# 运行编解码服务接口测试
./build.sh --product-name rk3568 --build-target tests --test-target codec_service_test

# 运行 ABI 兼容性检查
python3 scripts/check_abi_compatibility.py --repo codec --compare v4_0 v3_0

# 运行 Parcelable 跨版本共享层测试
python3 scripts/check_parcelable_fields.py --repo codec parcelable/

# 运行 zcodec 命名约定验证
python3 scripts/check_naming_convention.py --repo codec --subdomain zcodec --prefix HdiZ

# 运行下游媒体服务集成测试
./build.sh --product-name rk3568 --build-target //drivers/peripheral/codec --test-target codec_hdi_test
```

### 任务特定验证

| 任务类型 | 验证命令 |
|---|---|
| 主 codec 接口变更 | `./build.sh --build-target drivers_interface_codec` + 同步构建外围仓 `drivers/peripheral/codec` 验证实现兼容 + 跑下游媒体服务的接口测试 + `python3 scripts/check_abi_compatibility.py --repo codec` |
| 图像编解码接口变更 | `./build.sh --build-target //drivers/interface/codec/image/v2_1:image_idl_target` + 同步构建外围仓图像实现 + 验证主 codec 是否引用图像类型 + 运行图像编解码集成测试 |
| Z 编解码接口变更 | `./build.sh --build-target //drivers/interface/codec/zcodec/v1_0:zcodec_idl_target` + 同步构建外围仓 Z 编解码实现 + 验证 `HdiZ` 前缀命名约定 + `python3 scripts/check_naming_convention.py --repo codec --subdomain zcodec --prefix HdiZ` |
| 共享类型变更（基础） | 全量构建 `drivers_interface_codec` + 验证同版本所有接口的引用编译通过 + 跑下游消费者 ABI 检查 |
| 共享类型变更（扩展） | 全量构建 `drivers_interface_codec` + 验证 `CodecExtTypes.idl` 仅在 v2_0+ 版本中引用 + 验证 v1_0 编译不失败 |
| Parcelable 类型变更 | 全量构建 `drivers_interface_codec` + 验证所有活跃版本引用编译通过 + 跑下游消费者 ABI 检查 + `python3 scripts/check_parcelable_fields.py --repo codec parcelable/` |
| 新增版本 | 新建版本目录 + 修改 `bundle.json` + 全量构建 + 验证外围仓 `sa_profile` 的 `min_hdi_proxy_version` 更新 + 下游消费者适配 |
| 构建配置变更 | `./build.sh --build-target drivers_interface_codec` + 验证 `inner_kits` 暴露的 headers 可被下游引用 |

### 静态分析 / Lint

```bash
# 代码格式与静态检查
./build.sh --product-name rk3568 --build-target drivers_interface_codec --lint
gn gen --check
hdi-gen --check
```

### Done 定义

- 构建通过（`drivers_interface_codec` 子系统 + 3 个活跃 IDL 目标 + parcelable headers + 外围仓 `drivers/peripheral/codec`）
- Lint/格式检查通过（`gn format --check` 无格式差异）
- IDL 语法检查通过（`scripts/check_idl_syntax.py` 无错误）
- Parcelable 字段顺序检查通过（`scripts/check_parcelable_fields.py` 无错误）
- zcodec 命名约定检查通过（`scripts/check_naming_convention.py` 验证 `HdiZ` 前缀）
- 无新增编译警告
- 变更范围与任务要求一致，未夹带未关联的重构
- IDL 变更已由 `hdi-gen` 重生成 proxy/stub/headers，无手改生成产物
- Parcelable 变更已评估所有活跃版本的引用
- 新增版本已同步 `bundle.json` 的 `sub_component` 与 `inner_kits` + 外围仓 `sa_profile` 的 `min_hdi_proxy_version`
- `CodecTypes.idl` / `CodecExtTypes.idl` / `CodecImageType.idl` / `HdiZTypes.idl` 类型变更已评估同版本所有接口的引用
- zcodec 接口变更已遵循 `HdiZ` 前缀命名约定
- 活跃版本（v4_0 / image v2_1 / zcodec v1_0）的变更已评估 ABI 兼容性，未删除或重命名已发布方法
- 生成产物未被手改（`git diff` 对比无 `_proxy.h/.cpp`、`_stub.h/.cpp` 变更）

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 目标子域与版本（主 codec v4_0 / image v2_1 / zcodec v1_0 / parcelable / 历史版本）及是否为活跃构建
4. 是否触发跨层同步修改（IDL 重生成 / 外围仓 `drivers/peripheral/codec` / `bundle.json` / `sa_profile`）
5. 是否影响 Parcelable 跨版本共享层或 ABI 契约
6. 是否触及架构不变量或需确认事项（特别是 `CodecExtTypes` 分层、`HdiZ` 命名约定、子域独立版本管理）

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样，不能假称已通过。涉及外围仓 `drivers/peripheral/codec` 的变更，必须人工复核 IDL 与实现的匹配性并说明无法在沙箱验证的限制。
