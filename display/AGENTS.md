# AGENTS.md - OpenHarmony 显示 HDI 接口（drivers_interface_display）

## 1. 代码地图

本仓库定义 OpenHarmony 显示 HDI 接口（@ohos/drivers_interface_display，版本 4.0，subsystem `hdf`），是显示驱动与图形/窗口服务层之间的 IDL 契约。核心职责是定义显示合成器（IDisplayComposer，硬件合成）、显示缓冲（IAllocator/IMapper/IMetadata，gralloc 分配与映射）以及图形通用类型（BufferHandleMetaKeyType/CMColorSpace/HdrStaticMetadata 等）的接口契约。最重要的架构边界是**本仓库是 interface 仓库中最复杂的，包含 3 个独立版本化的子域（composer、buffer、graphic/common），所有版本全部活跃构建（无历史版本排除），向后兼容性至关重要**。

### 非本项目维护的目录

以下目录属于其他团队或生成产物，不属于本项目维护范围，修改时请跳过：

- 各版本目录下 `BUILD.gn` 引用的生成产物（proxy `_proxy.h/.cpp`、stub `_stub.h/.cpp`）：由 `hdi-gen` 在构建时生成，**不要手改**
- 各版本 `hdi_impl/` 子目录中的实现适配层代码由外围仓 `drivers/peripheral/display` 维护，本仓仅提供 IDL 与 hdi_impl 接口骨架
- 显示 HDI 实现由 `drivers/peripheral/display` 仓维护，本仓是接口定义方，不包含完整实现

### 嵌套指引

本仓目前无嵌套 AGENTS.md / CLAUDE.md / rules / skills 文件。以下子目录可按需新建嵌套 AGENTS.md 以提供更具体的指引：

- `composer/`：显示合成器子域，含 6 个版本（v1_0-v1_5）+ cache_manager + command_pack + common + hdifd_parcelable，可新建 `composer/AGENTS.md` 聚焦合成器版本演进、回调接口分层、cache_manager 跨版本共享
- `buffer/`：显示缓冲子域，含 5 个版本（v1_0-v1_4），可新建 `buffer/AGENTS.md` 聚焦 gralloc 分配/映射/元数据的版本演进、IAllocator/IMapper/IMetadata 分层
- `graphic/common/`：图形通用类型子域，含 7 个版本（v1_0-v2_3），仅含类型定义无接口，可新建 `graphic/common/AGENTS.md` 聚焦跨子域共享类型、BufferHandle 元数据、HDR/色彩空间类型
- `composer/cache_manager/`：缓冲缓存管理库（`libcomposer_buffer_cache`），跨 composer 版本共享，可新建 `composer/cache_manager/AGENTS.md` 聚焦缓冲缓存策略、device/layer 缓存分层

### 关键区域

- `composer/v1_0/`：合成器初始版本，含 `IDisplayComposer.idl`、`DisplayComposerType.idl`、`IHotPlugCallback.idl`、`IRefreshCallback.idl`、`IVBlankCallback.idl`（活跃构建，产出 `display_composer_idl_target` / `libdisplay_composer_proxy_1.0` / `libdisplay_composer_stub_1.0`）
- `composer/v1_1/`：新增 `IModeCallback.idl`、`ISeamlessChangeCallback.idl`（活跃构建，额外产出 `libdisplay_composer_hdi_impl_1.1`）
- `composer/v1_2/`：新增 `IVBlankIdleCallback.idl`（活跃构建）
- `composer/v1_3/`：新增 `IHwcEventCallback.idl`（活跃构建）
- `composer/v1_4/`、`composer/v1_5/`：类型/接口扩展（活跃构建）
- `composer/cache_manager/`：缓冲缓存管理库（`libcomposer_buffer_cache`），跨 composer 版本共享，含 `cache_manager.h`、`device_cache.*`、`layer_cache.*`、`buffer_cache_utils.h`
- `composer/command_pack/`：命令打包/解包工具，含 `command_data_packer.h`、`command_data_unpacker.h`、`command_data_utils.h`
- `composer/common/`：合成器公共头文件（`include/`）
- `composer/hdifd_parcelable/`：HDI FD Parcelable 序列化（`hdifd_parcelable.h/.cpp`，活跃构建，产出 `libhdifd_parcelable` + `display_composer_common_config`）
- `buffer/v1_0/`：缓冲初始版本，含 `IAllocator.idl`、`IMapper.idl`、`DisplayBufferType.idl`（活跃构建，产出 `display_buffer_idl_target` / `libdisplay_buffer_proxy_1.0` / `libdisplay_buffer_stub_1.0` / `libdisplay_buffer_hdi_impl`）
- `buffer/v1_1/`：新增 `IMetadata.idl`（活跃构建，额外产出 `libdisplay_buffer_hdi_impl_v1_1`）
- `buffer/v1_2/`：扩展 `DisplayBufferType.idl`、`IMapper.idl`（活跃构建）
- `buffer/v1_3/`：扩展 `IMapper.idl`（活跃构建）
- `buffer/v1_4/`：扩展 `IAllocator.idl`（活跃构建）
- `graphic/common/v1_0/` 至 `graphic/common/v2_3/`：7 个版本的图形通用类型，含 `BufferHandleMetaKeyType.idl`、`CMColorSpace.idl`、`HdrStaticMetadata.idl` 等（活跃构建，仅产出 `display_commontype_idl_target` / `display_commontype_idl_headers` / `libdisplay_commontype_proxy_*`，**无接口，仅类型定义**）
- `v1_0/`：旧的 gralloc 接口（`libdisplay_gralloc`），含 `stub/libdisplay_gralloc.stub.json`，向后兼容保留
- `display_config.gni`：3 个特性开关（`drivers_interface_display_community` / `drivers_interface_display_hicollie_enable` / `drivers_interface_display_vdi_default`），`display_defines` 用于条件编译
- `bundle.json`：组件元数据、`sub_component`（30+ 个活跃目标）、`inner_kits`（50+ 个 inner_kits 条目）

### Where to look

| 任务类型 | 先看哪里 |
|---|---|
| 合成器接口变更 | 对应版本 `composer/vN_0/IDisplayComposer.idl` + `DisplayComposerType.idl` + 该版本回调接口（`IHotPlugCallback` / `IRefreshCallback` / `IVBlankCallback` / `IModeCallback` / `ISeamlessChangeCallback` / `IVBlankIdleCallback` / `IHwcEventCallback`） |
| 缓冲分配/映射变更 | 对应版本 `buffer/vN_0/IAllocator.idl` + `IMapper.idl` + `IMetadata.idl`（v1_1+）+ `DisplayBufferType.idl` |
| 图形通用类型变更 | 对应版本 `graphic/common/vN_0/` 下的 `.idl`（`BufferHandleMetaKeyType.idl` / `CMColorSpace.idl` / `HdrStaticMetadata.idl`） |
| 回调接口新增 | 检查 `composer/v1_0/` 已有回调（HotPlug/Refresh/VBlank）-> `v1_1/`（Mode/SeamlessChange）-> `v1_2/`（VBlankIdle）-> `v1_3/`（HwcEvent）-> 在新版本中新增 |
| 缓冲缓存变更 | `composer/cache_manager/cache_manager.h` + `device_cache.*` + `layer_cache.*` + `buffer_cache_utils.h` |
| 命令打包变更 | `composer/command_pack/command_data_packer.h` + `command_data_unpacker.h` + `command_data_utils.h` |
| HDI FD 序列化 | `composer/hdifd_parcelable/hdifd_parcelable.h` + `hdifd_parcelable.cpp` |
| 特性开关 | `display_config.gni` 的 `declare_args()` + `bundle.json` 的 `features` 列表 |
| VDI 默认库启用 | `display_config.gni` 中 `drivers_interface_display_vdi_default` -> `COMPOSER_VDI_DEFAULT_LIBRARY_ENABLE` / `BUFFER_VDI_DEFAULT_LIBRARY_ENABLE` 宏 |
| HiCollie 启用 | `display_config.gni` 中 `drivers_interface_display_hicollie_enable`（非社区版自动启用）-> `DISPLAY_HICOLLIE_ENABLE` 宏 |
| 社区版构建 | `display_config.gni` 中 `drivers_interface_display_community` -> `display_defines` 中 `DISPLAY_COMMUNITY` 宏 |
| 构建配置 | `bundle.json`（sub_component、inner_kits、deps）+ `display_config.gni` + 各版本 `BUILD.gn` |

### 架构分层

```
图形/窗口服务层（graphic_surface / window_manager 等，外部仓）
  ↓ 调用 proxy 库
IDL 代理层（本仓生成产物）
  libdisplay_composer_proxy_1.0 ~ 1.5 (composer/v1_0 ~ v1_5)
  libdisplay_buffer_proxy_1.0 ~ 1.4 (buffer/v1_0 ~ v1_4)
  libdisplay_commontype_proxy_1.0 ~ 2.3 (graphic/common/v1_0 ~ v2_3)
  ↓ IPC（HDF IPC 框架）
IDL 桩层（本仓生成产物）
  libdisplay_composer_stub_* / libdisplay_buffer_stub_*
  ↓ 由 drivers/peripheral/display 实现
HDI 实现层（drivers/peripheral/display，外部仓）

跨版本共享层
  composer/cache_manager (libcomposer_buffer_cache)
  composer/command_pack (命令打包工具)
  composer/hdifd_parcelable (libhdifd_parcelable)
  graphic/common/* (仅类型定义，无接口)
```

## 2. 知识路由

在规划或编辑前，先对任务分类，读取对应的 IDL 文件和构建配置。本仓库因含 3 个独立版本化子域且所有版本全部活跃，路由时必须明确目标子域与版本。

### Task-based routing

| 任务类型 | 读取 |
|---|---|
| 合成器接口新增/修改 | 目标版本 `composer/vN_0/IDisplayComposer.idl` + `DisplayComposerType.idl` + 该版本回调 `.idl` + `composer/vN_0/BUILD.gn` |
| 合成器回调接口变更 | 确定回调所属版本（v1_0: HotPlug/Refresh/VBlank；v1_1: Mode/SeamlessChange；v1_2: VBlankIdle；v1_3: HwcEvent）-> 对应版本回调 `.idl` |
| 缓冲分配/映射变更 | 目标版本 `buffer/vN_0/IAllocator.idl` + `IMapper.idl` + `IMetadata.idl`（v1_1+）+ `DisplayBufferType.idl`（v1_0/v1_2）+ `buffer/vN_0/BUILD.gn` |
| 图形通用类型变更 | 目标版本 `graphic/common/vN_0/` 下的 `.idl` 文件 + `graphic/common/vN_0/BUILD.gn` + 检查 composer/buffer 是否引用该类型 |
| 缓冲缓存策略变更 | `composer/cache_manager/cache_manager.h` + `device_cache.*` + `layer_cache.*` + `buffer_cache_utils.h` + `composer/cache_manager/BUILD.gn` |
| 命令打包变更 | `composer/command_pack/command_data_packer.h` + `command_data_unpacker.h` + `command_data_utils.h` |
| HDI FD 序列化变更 | `composer/hdifd_parcelable/hdifd_parcelable.h` + `hdifd_parcelable.cpp` + `composer/hdifd_parcelable/BUILD.gn` |
| 新增合成器版本 | 最新版本（`composer/v1_5/`）作为模板 -> 新建 `composer/vN_0/` -> 修改 `bundle.json` 的 `sub_component` 与 `inner_kits` -> 同步外围仓 |
| 新增缓冲版本 | 最新版本（`buffer/v1_4/`）作为模板 -> 新建 `buffer/vN_0/` -> 修改 `bundle.json`（buffer 版本独立于 composer 版本） |
| 新增图形通用类型版本 | 最新版本（`graphic/common/v2_3/`）作为模板 -> 新建 `graphic/common/vN_0/` -> 修改 `bundle.json` |
| 特性开关变更 | `display_config.gni` 的 `declare_args()` + `bundle.json` 的 `features` 列表 + 检查 `display_defines` 在下游 `BUILD.gn` 中的使用 |
| VDI 默认库启用 | `display_config.gni` 中 `drivers_interface_display_vdi_default` + `COMPOSER_VDI_DEFAULT_LIBRARY_ENABLE` / `BUFFER_VDI_DEFAULT_LIBRARY_ENABLE` 宏 |
| HiCollie 集成 | `display_config.gni` 中 `drivers_interface_display_hicollie_enable`（非社区版自动启用为 true）+ `DISPLAY_HICOLLIE_ENABLE` 宏 |
| 构建配置 | `bundle.json`（sub_component 30+、inner_kits 50+、deps）+ `display_config.gni` + 各版本 `BUILD.gn` |

### Path-based routing

| 修改路径 | 需了解的上下文 |
|---|---|
| `composer/v1_0/` | 合成器初始版本，定义 HotPlug/Refresh/VBlank 三类回调，被 `sa_profile` 的 `min_hdi_proxy_version` 引用为 `libdisplay_composer_proxy_1.0.z.so`，破坏性变更影响所有显示服务消费者 |
| `composer/v1_1/` 至 `composer/v1_5/` | 合成器增量版本，每个版本新增回调或扩展类型，全部活跃构建，向后兼容性要求每个新版本不能破坏前一版本接口 |
| `buffer/v1_0/` | 缓冲初始版本，定义 IAllocator/IMapper/DisplayBufferType，gralloc 分配与映射的入口 |
| `buffer/v1_1/` | 新增 IMetadata（缓冲元数据），独立版本演进 |
| `buffer/v1_2/` 至 `buffer/v1_4/` | 扩展 DisplayBufferType/IMapper/IAllocator，每个版本聚焦特定接口扩展 |
| `graphic/common/v1_0/` 至 `graphic/common/v2_3/` | 7 个版本的图形通用类型，**仅类型定义无接口**，被 composer 与 buffer 引用 |
| `composer/cache_manager/` | 跨 composer 版本共享的缓冲缓存管理库，变更影响所有 composer 版本 |
| `composer/hdifd_parcelable/` | HDI FD Parcelable 序列化，跨 composer 版本共享，变更影响所有 composer 版本的 FD 传输 |
| `v1_0/`（仓库根） | 旧的 gralloc 接口（`libdisplay_gralloc`），向后兼容保留，新增功能不应在此 |
| `display_config.gni` | 特性开关源，开关变更需同步 `bundle.json` 的 `features` 列表 + 外围仓 `drivers/peripheral/display` 对应开关 |
| `bundle.json` | 活跃构建目标与 inner_kits 清单（最复杂的 bundle.json，30+ sub_component，50+ inner_kits），新增版本或新增库必须在此注册 |

### Vocabulary-based routing

当任务、issue、日志、API 名称中出现以下术语时，先理解其含义和风险再动手：

| 术语 | 含义与风险 | 读取 |
|---|---|---|
| IDisplayComposer | 显示合成器接口，硬件合成的入口，控制图层合成与提交 | `composer/vN_0/IDisplayComposer.idl` |
| IAllocator | 缓冲分配器接口，gralloc 内存分配入口 | `buffer/v1_0/IAllocator.idl` / `buffer/v1_4/IAllocator.idl` |
| IMapper | 缏冲映射器接口，内存映射与释放 | `buffer/v1_0/IMapper.idl` / `buffer/v1_2/IMapper.idl` / `buffer/v1_3/IMapper.idl` |
| IMetadata | 缓冲元数据接口，v1_1 新增，BufferHandle 元数据读写 | `buffer/v1_1/IMetadata.idl` |
| IHotPlugCallback | 热插拔回调，v1_0，显示设备连接/断开事件 | `composer/v1_0/IHotPlugCallback.idl` |
| IRefreshCallback | 刷新回调，v1_0，刷新完成事件 | `composer/v1_0/IRefreshCallback.idl` |
| IVBlankCallback | VBlank 回调，v1_0，垂直同步事件 | `composer/v1_0/IVBlankCallback.idl` |
| IModeCallback | 模式回调，v1_1，显示模式变更事件 | `composer/v1_1/IModeCallback.idl` |
| ISeamlessChangeCallback | 无缝切换回调，v1_1，显示模式无缝切换事件 | `composer/v1_1/ISeamlessChangeCallback.idl` |
| IVBlankIdleCallback | VBlank 空闲回调，v1_2，VBlank 空闲事件 | `composer/v1_2/IVBlankIdleCallback.idl` |
| IHwcEventCallback | HWC 事件回调，v1_3，硬件合成器事件 | `composer/v1_3/IHwcEventCallback.idl` |
| DisplayComposerType | 合成器共享类型（图层、合成类型、Fence 等） | `composer/vN_0/DisplayComposerType.idl` |
| DisplayBufferType | 缓冲共享类型（缓冲描述、格式等） | `buffer/v1_0/DisplayBufferType.idl` / `buffer/v1_2/DisplayBufferType.idl` |
| BufferHandleMetaKeyType | BufferHandle 元数据键类型，graphic/common 子域 | `graphic/common/vN_0/BufferHandleMetaKeyType.idl` |
| CMColorSpace | 色彩空间类型，graphic/common 子域 | `graphic/common/v1_0/CMColorSpace.idl` |
| HdrStaticMetadata | HDR 静态元数据类型，graphic/common 子域 | `graphic/common/v1_0/HdrStaticMetadata.idl` |
| cache_manager | 缓冲缓存管理库，跨 composer 版本共享 | `composer/cache_manager/` |
| hdifd_parcelable | HDI FD Parcelable 序列化，跨 composer 版本共享 | `composer/hdifd_parcelable/` |
| hdi-gen | HDI IDL 代码生成工具 | 各版本 `BUILD.gn` 中的 `hdi_gen` 调用 |
| hdi_impl | HDI 实现适配层接口骨架，外围仓实现填充 | 各版本 `hdi_impl/` 子目录 |
| VDI default library | VDI 默认库，由 `drivers_interface_display_vdi_default` 控制 | `display_config.gni` + `COMPOSER_VDI_DEFAULT_LIBRARY_ENABLE` / `BUFFER_VDI_DEFAULT_LIBRARY_ENABLE` |
| DISPLAY_COMMUNITY | 社区版构建宏 | `display_config.gni` |
| DISPLAY_HICOLLIE_ENABLE | HiCollie 监控宏，非社区版自动启用 | `display_config.gni` |
| min_hdi_proxy_version | 外围仓 `sa_profile` 中声明的 HDI 代理版本依赖 | 外部仓 `drivers/peripheral/display` 的 `sa_profile` |

在计划阶段，必须声明：
- **任务分类**（如：合成器接口变更 / 缓冲接口变更 / 图形通用类型变更 / 缓存管理变更 / 序列化变更 / 新增版本 / 特性开关变更 / 构建配置变更）
- **目标子域与版本**（composer v1_0-v1_5 / buffer v1_0-v1_4 / graphic/common v1_0-v2_3 / cache_manager / hdifd_parcelable），明确所有版本全部活跃构建
- **已读取的 IDL 文件和构建配置**（具体到文件路径）
- **发现的约束**（3 子域独立版本管理、所有版本活跃构建、向后兼容性、跨版本共享层影响、外围仓同步需求）
- **是否需要同步修改其他层**：
  - IDL 变更 -> 由 `hdi-gen` 重生成 proxy/stub/headers -> 同步外围仓 `drivers/peripheral/display` 实现
  - cache_manager / hdifd_parcelable 变更 -> 影响所有 composer 版本 -> 同步外围仓
  - graphic/common 类型变更 -> 影响所有引用该版本的 composer/buffer -> 同步外围仓
  - 特性开关变更 -> 同步 `bundle.json` 的 `features` 列表 + 外围仓对应开关
  - 新增版本 -> 同步 `bundle.json` + 外围仓 `sa_profile` 的 `min_hdi_proxy_version` + 下游消费者

## 3. 约束边界

### 架构不变量

- **`.idl` 文件是唯一真相源**：所有 proxy/stub/headers 由 `hdi-gen` 在构建时生成，本仓库不包含手写的 proxy/stub 实现代码
- **3 个子域独立版本管理**：composer（v1_0-v1_5）、buffer（v1_0-v1_4）、graphic/common（v1_0-v2_3）各自独立演进，版本号不联动
- **所有版本全部活跃构建**：`bundle.json` 的 `sub_component` 列出 30+ 个目标，无历史版本排除，向后兼容性是第一优先级
- **graphic/common 仅含类型定义无接口**：该子域只产出 `display_commontype_idl_target` / headers / proxy，不包含可调用的接口方法
- **composer 与 buffer 各有 hdi_impl 库**：在 IDL 目标之外，每个版本额外产出 `libdisplay_composer_hdi_impl_*` / `libdisplay_buffer_hdi_impl_*`，由外围仓实现填充
- **cache_manager 跨 composer 版本共享**：`libcomposer_buffer_cache` 被所有 composer 版本引用，变更影响全子域
- **hdifd_parcelable 跨 composer 版本共享**：`libhdifd_parcelable` 被所有 composer 版本引用，FD 序列化变更影响全子域
- **版本号即 ABI 契约**：`v1_5` proxy 不能与 `v1_4` stub 通信，跨版本交互必须通过版本化库显式选择
- **命名空间与版本绑定**：composer 命名空间 `ohos.hdi.display.composer.vN_0`，buffer 命名空间 `ohos.hdi.display.buffer.vN_0`，graphic/common 命名空间 `ohos.hdi.display.graphic.common.vN_0`
- **特性开关在 interface 与 peripheral 双向对应**：`display_config.gni` 中的开关需与外围仓 `drivers/peripheral/display` 的开关保持一致
- **VDI 默认库启用受 `drivers_interface_display_vdi_default` 控制**：启用后定义 `COMPOSER_VDI_DEFAULT_LIBRARY_ENABLE` / `BUFFER_VDI_DEFAULT_LIBRARY_ENABLE` 宏
- **HiCollie 非社区版自动启用**：`drivers_interface_display_hicollie_enable` 在非社区版下默认 true，定义 `DISPLAY_HICOLLIE_ENABLE` 宏

### 禁止事项

- **不要手改 `hdi-gen` 生成的 proxy/stub/headers 代码**：应修改 `.idl` 源文件后由构建系统重生成
- **不要删除或重命名已发布的 IDL 方法**：IDL 方法签名是 ABI 契约，所有版本活跃构建，删除或重命名会破坏所有消费者
- **不要复用或调整已有 IDL 方法的码值**：新增接口必须追加新方法，不能复用已删除方法的码值
- **不要在 graphic/common 中定义可调用接口**：该子域仅含类型定义，接口应在 composer 或 buffer 中定义
- **不要跨子域共享类型而不显式 import**：composer、buffer、graphic/common 的类型文件不自动共享，跨子域引用需显式 import
- **不要修改 `cache_manager` / `hdifd_parcelable` 而不评估全子域影响**：跨版本共享层变更影响所有 composer 版本
- **不要修改 `bundle.json` 的 `sub_component` 而不同步 `inner_kits`**：新增构建目标必须同步注册 inner_kits 以暴露给下游（本仓 inner_kits 50+ 条目，漏注册会导致下游找不到头文件）
- **不要翻转 `display_config.gni` 特性开关默认值而不评估外围仓影响**：`community` / `hicollie_enable` / `vdi_default` 开关在 interface 与 peripheral 双向对应
- **不要在新版本中破坏前一版本接口**：所有版本活跃构建，新版本必须向后兼容前一版本
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审
- **不要在本仓库中实现完整 HDI 逻辑**：本仓是接口定义方，hdi_impl 仅提供接口骨架，实现由 `drivers/peripheral/display` 维护

### Ask before 规则

在执行以下操作前，必须先确认或询问：
- **修改任何版本的 IDL 方法**：确认所有版本活跃构建的向后兼容性
- **修改 cache_manager 或 hdifd_parcelable**：确认影响所有 composer 版本
- **修改 graphic/common 类型**：确认所有引用该版本的 composer/buffer
- **翻转 display_config.gni 特性开关**：确认外围仓 `drivers/peripheral/display` 同步
- **修改 bundle.json 的 sub_component 或 inner_kits**：确认 50+ 条目漏注册风险

### 需确认后再修改

- **IDL 方法签名变更**（需确认 ABI 兼容性影响和版本策略，评估所有下游消费者，所有版本活跃构建影响面最大）
- **`DisplayComposerType.idl` / `DisplayBufferType.idl` / graphic/common 类型文件变更**（需评估同版本所有接口的引用 + 跨子域引用，Parcelable 字段顺序是 ABI 一部分）
- **`cache_manager` / `hdifd_parcelable` 变更**（需评估所有 composer 版本的影响，跨版本共享层变更影响面最大）
- **新增合成器 / 缓冲 / 图形通用类型版本**（需确认外围仓 `drivers/peripheral/display` 的实现计划、`sa_profile` 的 `min_hdi_proxy_version` 更新、下游消费者适配）
- **`bundle.json` 的 `sub_component` 或 `inner_kits` 调整**（需确认下游消费者是否依赖被移除的 inner_kits，本仓 inner_kits 50+ 条目）
- **`display_config.gni` 特性开关默认值翻转**（特别是 `drivers_interface_display_vdi_default` / `drivers_interface_display_hicollie_enable`，需确认外围仓同步）
- **`drivers_interface_display_community` 默认值翻转**（影响 `DISPLAY_COMMUNITY` 宏的编译范围 + `hicollie_enable` 的自动启用逻辑）
- **新增外部依赖**（需确认许可证、包大小、`bundle.json` 同步）

### 项目特定陷阱

- **所有版本全部活跃构建**：本仓无历史版本排除，composer 6 个版本 + buffer 5 个版本 + graphic/common 7 个版本全部在 `sub_component` 中，任何 IDL 变更都需评估是否影响其他版本
- **3 子域版本号不联动**：composer v1_5 不对应 buffer v1_5 或 graphic/common v2_5，三子域独立演进，不要假设版本对应
- **回调接口按版本分层**：HotPlug/Refresh/VBlank 在 v1_0，Mode/SeamlessChange 在 v1_1，VBlankIdle 在 v1_2，HwcEvent 在 v1_3，新增回调应在最新版本中追加，不能回溯到旧版本
- **buffer 的 IAllocator/IMapper/IMetadata 按版本分散扩展**：v1_0 有 IAllocator/IMapper，v1_1 新增 IMetadata，v1_2 扩展 IMapper/DisplayBufferType，v1_3 扩展 IMapper，v1_4 扩展 IAllocator，变更需定位到正确版本
- **graphic/common 仅类型无接口**：误在 graphic/common 中定义接口方法会导致构建失败，接口必须在 composer 或 buffer 中
- **`cache_manager` 的 device/layer 缓存分层**：`device_cache.*` 与 `layer_cache.*` 分别管理设备级与图层级缓存，变更需同步两层
- **`hdifd_parcelable` 的 FD 序列化 ABI**：FD 跨进程传输的序列化格式是 ABI 一部分，变更会导致所有 composer 版本的 FD 传输失败
- **`v1_0/`（仓库根）旧 gralloc 接口**：`libdisplay_gralloc` 是旧接口，与 `buffer/v1_0/` 的 IAllocator/IMapper 并存，新增功能应在 `buffer/` 子域而非仓库根 `v1_0/`
- **`display_defines` 的条件编译扩散**：`DISPLAY_COMMUNITY` / `COMPOSER_VDI_DEFAULT_LIBRARY_ENABLE` / `BUFFER_VDI_DEFAULT_LIBRARY_ENABLE` / `DISPLAY_HICOLLIE_ENABLE` 宏会扩散到下游 `BUILD.gn` 的 `defines`，漏掉一处会导致特性开关失效
- **`hicollie_enable` 的非社区版自动启用**：`drivers_interface_display_hicollie_enable` 在 `!drivers_interface_display_community` 时自动设为 true，手动翻转 community 开关会间接影响 hicollie
- **`bundle.json` 的规模**：30+ sub_component + 50+ inner_kits，新增版本需在正确位置插入，漏插入 inner_kits 会导致下游找不到头文件
- **`hdi-gen` 工具版本与 IDL 语法兼容性**：升级 `hdi-gen` 工具可能引入新语法支持，但旧 IDL 语法必须保持向后兼容

### Agent failure patterns

- **Agent 在所有版本中应用同一变更**：Agent 可能将 IDL 修改应用到所有 composer/buffer/graphic/common 版本而不区分影响范围，导致无意义的构建失败或 ABI 破坏。应定位到正确版本
- **Agent 混淆 3 子域的版本号**：Agent 可能假设 composer v1_5 对应 buffer v1_5 或 graphic/common v2_5。实际上三子域独立演进，版本号无联动
- **Agent 在 graphic/common 中定义接口**：Agent 可能误在 graphic/common 子域中定义可调用接口方法，导致构建失败。graphic/common 仅含类型定义
- **Agent 遗漏 cache_manager / hdifd_parcelable 影响**：Agent 修改跨版本共享层时可能只验证单个 composer 版本而忽略其他版本
- **Agent 回溯到旧版本新增回调**：Agent 可能将新回调接口添加到旧版本（如 v1_0）而非最新版本，破坏向后兼容性
- **Agent 遗漏 bundle.json 50+ inner_kits 条目**：Agent 新增版本时可能漏注册 inner_kits，导致下游找不到头文件
- **Agent 忽略 VDI/HiCollie/Community 开关联动**：Agent 修改 `display_config.gni` 开关时可能忘记 `hicollie_enable` 在非社区版自动启用的逻辑，或 `community` 开关对 hicollie 的间接影响
- **Agent 修改旧 gralloc 接口 `v1_0/`**：Agent 可能在仓库根 `v1_0/` 目录新增功能而非在 `buffer/` 子域，新增功能应在 `buffer/` 子域

## 4. 验证闭环

### 最小验证

```bash
# 构建 display 接口组件（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target drivers_interface_display

# 单独构建各子域
./build.sh --product-name rk3568 --build-target //drivers/interface/display/composer/v1_5:display_composer_idl_target
./build.sh --product-name rk3568 --build-target //drivers/interface/display/buffer/v1_4:display_buffer_idl_target
./build.sh --product-name rk3568 --build-target //drivers/interface/display/graphic/common/v2_3:display_commontype_idl_target
./build.sh --product-name rk3568 --build-target //drivers/interface/display/composer/cache_manager:libcomposer_buffer_cache
./build.sh --product-name rk3568 --build-target //drivers/interface/display/composer/hdifd_parcelable:libhdifd_parcelable
```

### Lint and static analysis

```bash
# 格式检查
gn format --check composer/ buffer/ graphic/common/ v1_0/

# IDL 语法检查（验证 .idl 文件格式是否符合 hdi-gen 要求）
python3 scripts/check_idl_syntax.py --repo display composer/ buffer/ graphic/common/

# 构建目标 lint 检查
./build.sh --product-name rk3568 --build-target drivers_interface_display --lint

# 检查所有活跃版本的 composer/buffer/graphic/common 编译一致性
python3 scripts/check_version_completeness.py --repo display --subsystem drivers_interface_display

# 检查生成产物是否被手改
git diff --name-only -- '**/_proxy.h' '**/_proxy.cpp' '**/_stub.h' '**/_stub.cpp' | grep -v 'BUILD.gn' | head -20

# 检查 inner_kits 完整性（验证 50+ 条目无遗漏）
python3 scripts/check_inner_kits.py --repo display bundle.json
```

### 测试命令

```bash
# 运行显示服务接口测试
./build.sh --product-name rk3568 --build-target tests --test-target display_service_test

# 运行 ABI 兼容性检查（所有活跃版本）
python3 scripts/check_abi_compatibility.py --repo display --all-versions composer v1_0-v1_5 buffer v1_0-v1_4 graphic/common v1_0-v2_3

# 运行跨版本共享层一致性检查
python3 scripts/check_version_completeness.py --repo display --subsystem drivers_interface_display

# 运行 composer 版本向后兼容性检查
python3 scripts/check_backward_compatibility.py --repo display --subsystem composer

# 运行下游图形/窗口服务集成测试
./build.sh --product-name rk3568 --build-target //drivers/peripheral/display --test-target display_hdi_test
```

### 任务特定验证

| 任务类型 | 验证命令 |
|---|---|
| 合成器接口变更 | `./build.sh --build-target drivers_interface_display` + 同步构建外围仓 `drivers/peripheral/display` 验证实现兼容 + 跑下游图形/窗口服务的接口测试 + `python3 scripts/check_backward_compatibility.py --repo display --subsystem composer` |
| 合成器回调接口变更 | 重新生成 proxy/stub + 同步外围仓回调实现 + 验证回调注册时序（HotPlug/Refresh/VBlank/Mode/SeamlessChange/VBlankIdle/HwcEvent） + 运行回调时序测试 |
| 缓冲分配/映射变更 | `./build.sh --build-target drivers_interface_display` + 同步构建外围仓 gralloc 实现 + 验证 IAllocator/IMapper/IMetadata 版本对应 + `python3 scripts/check_abi_compatibility.py --repo display --subsystem buffer` |
| 图形通用类型变更 | 全量构建 `drivers_interface_display` + 验证所有引用该版本的 composer/buffer 编译通过 + 跑下游消费者 ABI 检查 + `python3 scripts/check_version_completeness.py --repo display` |
| 缓冲缓存策略变更 | `./build.sh --build-target //drivers/interface/display/composer/cache_manager:libcomposer_buffer_cache` + 验证所有 composer 版本引用编译通过 + 运行跨版本缓存一致性测试 |
| HDI FD 序列化变更 | `./build.sh --build-target //drivers/interface/display/composer/hdifd_parcelable:libhdifd_parcelable` + 验证所有 composer 版本的 FD 传输 + 运行 FD 序列化 ABI 测试 |
| 新增版本 | 新建版本目录 + 修改 `bundle.json` + 全量构建 + 验证外围仓 `sa_profile` 的 `min_hdi_proxy_version` 更新 + 下游消费者适配 + `python3 scripts/check_inner_kits.py --repo display` |
| 特性开关翻转 | 重新构建 `drivers_interface_display` 全量 + 同步外围仓 `drivers/peripheral/display` 构建 + 验证 `bundle.json` 的 `features` 同步 + 验证 `hicollie_enable` 联动逻辑 |
| VDI 默认库启用 | 设置 `drivers_interface_display_vdi_default=true` + 全量构建 + 验证 `COMPOSER_VDI_DEFAULT_LIBRARY_ENABLE` / `BUFFER_VDI_DEFAULT_LIBRARY_ENABLE` 宏生效 |
| HiCollie 集成 | 验证 `drivers_interface_display_hicollie_enable` 在非社区版下自动启用 + `DISPLAY_HICOLLIE_ENABLE` 宏生效 + 验证 community 关闭时 hicollie 自动关闭 |
| 社区版构建 | 设置 `drivers_interface_display_community=true` + 全量构建 + 验证 `DISPLAY_COMMUNITY` 宏裁剪生效 + 验证 hicollie 自动关闭 |
| 构建配置变更 | `./build.sh --build-target drivers_interface_display` + 验证 `inner_kits` 暴露的 headers 可被下游引用（50+ 条目） + `python3 scripts/check_inner_kits.py --repo display` |

### Done 定义

- 构建通过（`drivers_interface_display` 子系统 + 所有 composer/buffer/graphic 版本 + cache_manager + hdifd_parcelable + 外围仓 `drivers/peripheral/display`）
- Lint/格式检查通过（`gn format --check` 无格式差异）
- IDL 语法检查通过（`scripts/check_idl_syntax.py` 无错误）
- 版本完整性检查通过（`scripts/check_version_completeness.py` 所有活跃版本编译一致）
- inner_kits 完整性检查通过（`scripts/check_inner_kits.py` 50+ 条目无遗漏）
- 无新增编译警告
- 变更范围与任务要求一致，未夹带未关联的重构
- IDL 变更已由 `hdi-gen` 重生成 proxy/stub/headers，无手改生成产物
- 特性开关变更已同步 `display_config.gni` + `bundle.json` + 外围仓 `drivers/peripheral/display`
- 新增版本已同步 `bundle.json` 的 `sub_component` 与 `inner_kits`（50+ 条目）+ 外围仓 `sa_profile` 的 `min_hdi_proxy_version`
- `cache_manager` / `hdifd_parcelable` 变更已评估所有 composer 版本的影响
- graphic/common 类型变更已评估所有引用该版本的 composer/buffer
- 所有版本（composer v1_0-v1_5 / buffer v1_0-v1_4 / graphic/common v1_0-v2_3）的变更已评估 ABI 兼容性，未删除或重命名已发布方法
- 新增版本已向后兼容前一版本
- 生成产物未被手改（`git diff` 对比无 `_proxy.h/.cpp`、`_stub.h/.cpp` 变更）

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 目标子域与版本（composer v1_0-v1_5 / buffer v1_0-v1_4 / graphic/common v1_0-v2_3 / cache_manager / hdifd_parcelable）及是否为活跃构建
4. 是否触发跨层同步修改（IDL 重生成 / 外围仓 `drivers/peripheral/display` / `bundle.json` / `display_config.gni` / `sa_profile`）
5. 是否影响跨版本共享层（cache_manager / hdifd_parcelable）或 ABI 契约
6. 是否触及架构不变量或需确认事项（特别是 3 子域独立版本管理、所有版本活跃构建、向后兼容性、VDI/HiCollie/Community 开关联动）

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样，不能假称已通过。涉及外围仓 `drivers/peripheral/display` 的变更，必须人工复核 IDL 与实现的匹配性并说明无法在沙箱验证的限制。涉及跨版本共享层（cache_manager / hdifd_parcelable）的变更，必须人工复核所有 composer 版本的引用关系。
