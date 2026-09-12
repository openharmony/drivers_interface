# AGENTS.md - OpenHarmony 相机 HDI 接口（drivers_interface_camera）

## 1. 代码地图

本仓库定义 OpenHarmony 相机 HDI 接口（@ohos/drivers_interface_camera，版本 4.0，subsystem `hdf`），是相机驱动与相机服务层之间的 IDL 契约。核心职责是定义相机主机（ICameraHost）、相机设备（ICameraDevice）、流操作（IStreamOperator）、图像处理（IImageProcessService/IImageProcessSession）、视频处理（IVideoProcessService/IVideoProcessSession）以及序列化类型（buffer_handle/buffer_producer/map_data）和元数据（camera_metadata）的接口契约。

**`.idl` 文件是唯一真相源**，proxy/stub/headers 由 `hdi-gen` 在构建时生成。本仓额外包含 C++ 手写的 `metadata/` 库和 `sequenceable/` 序列化组件。

### 关键区域

| 子域 | 目录 | 活跃构建 | 说明 |
|------|------|----------|------|
| 基础相机接口 | `v1_0/` | 是 | ICameraHost, ICameraDevice, IStreamOperator, IOfflineStreamOperator, 回调, Types |
| v1_1 扩展 | `v1_1/` | 是 | ICameraHost/ICameraDevice/IStreamOperator 扩展, Types 扩展 |
| v1_2 图像处理 | `v1_2/` | 是 | 新增 IImageProcessService/IImageProcessSession/IImageProcessCallback |
| v1_3 视频处理 | `v1_3/` | 是 | 新增 IVideoProcessService/IVideoProcessSession/IVideoProcessCallback |
| v1_4 视频扩展 | `v1_4/` | 是 | IVideoProcessCallback/IVideoProcessService 扩展 |
| v1_5 综合 | `v1_5/` | 是 | ICameraHost/ICameraDevice/IStreamOperator 再扩展 + IVideoProcessService |
| v1_6 延续 | `v1_6/` | 是 | IImageProcessService/ICameraHost 扩展 |
| v1_7 最新 | `v1_7/` | 是 | IImageProcessService 扩展, ExtendedStreamInfoType 新增 EXTENDED_STREAM_OHOS_ENHANCED_GAINMAP |
| 元数据(手写) | `metadata/` | 是 | C++ shared library `metadata`，非 hdi-gen 产物 |
| 序列化(手写) | `sequenceable/` | 是 | buffer_handle/buffer_producer/map_data Parcelable 类型 |

### 非本项目维护的目录

- 各版本目录下 `BUILD.gn` 引用的生成产物（proxy `_proxy.h/.cpp`、stub `_stub.h/.cpp`）：由 `hdi-gen` 生成，**不要手改**
- 相机 HDI 实现由 `drivers/peripheral/camera` 仓维护，本仓是接口定义方

## 2. Build / Lint / Test 命令

```bash
# 全量构建相机接口组件（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target drivers_interface_camera

# 单独构建指定版本
./build.sh --product-name rk3568 --build-target //drivers/interface/camera/v1_0:libcamera_stub_1.0
./build.sh --product-name rk3568 --build-target //drivers/interface/camera/v1_7:libcamera_stub_1.7
./build.sh --product-name rk3568 --build-target //drivers/interface/camera/v1_7:camera_idl_target

# 构建元数据和序列化组件
./build.sh --product-name rk3568 --build-target //drivers/interface/camera/metadata:metadata
./build.sh --product-name rk3568 --build-target //drivers/interface/camera/sequenceable/buffer_handle:libbuffer_handle_sequenceable_1.0

# Lint 和静态分析
./build.sh --product-name rk3568 --build-target drivers_interface_camera --lint
gn format --check v1_0/ v1_1/ v1_2/ v1_3/ v1_4/ v1_5/ v1_6/ v1_7/ metadata/ sequenceable/
gn gen --check
hdi-gen --check

# 检查生成产物是否被手改
git diff --name-only -- '**/_proxy.h' '**/_proxy.cpp' '**/_stub.h' '**/_stub.cpp' | grep -v 'BUILD.gn'

# 运行相机 HDI 测试（外围仓）
./build.sh --product-name rk3568 --build-target //drivers/peripheral/camera --test-target camera_hdi_test

# 最小验证（仅 IDL 语法）
python3 scripts/check_idl_syntax.py --repo camera v1_0/ v1_1/ v1_2/ v1_3/ v1_4/ v1_5/ v1_6/ v1_7/
```

## 3. 版本层次结构

相机接口从 v1_0 到 v1_7 共 8 个版本并行活跃构建。每个新版本通过 `extends` 继承上一版本：

```
v1_7 → 继承 v1_6, 新增 ExtendedStreamInfoType 增强增益图
v1_6 → 继承 v1_5(via IImageProcessService), 依赖 v1_2~v1_5
v1_5 → 继承 v1_3(via ICameraHost/IStreamOperator), 依赖 v1_0~v1_3
v1_4 → 独立视频回调扩展
v1_3 → 继承 v1_0(via ICameraHost), 新增视频处理
v1_2 → 继承 v1_1(via IStreamOperator), 新增图像处理
v1_1 → 继承 v1_0, 扩展方法
v1_0 → 基础版本: ICameraHost, ICameraDevice, IStreamOperator, IOfflineStreamOperator
```

**关键规则**：v1_7 的 `stub_deps` 依赖 v1_2~v1_6；v1_6 的 `stub_deps` 依赖 v1_2~v1_5。各版本通过 `import` 语句引用旧版本类型，不跨版本共享 Types.idl。

## 4. 代码风格指南

### 版权头

所有文件以 Apache 2.0 版权头开头（`/*` 多行注释块）。

### IDL 风格

- **接口命名**：`I` 前缀（ICameraHost, ICameraDevice, IStreamOperator, IImageProcessService）
- **包声明**：`package ohos.hdi.camera.vX_Y;`
- **共享类型**：版本内 `Types.idl` 定义 enum 和 struct；新版本通过 `import ohos.hdi.camera.vPrev.Types` 引入旧类型
- **enum 值**：负数表示错误码（如 `CAMERA_BUSY = -1`），非负数表示状态/类型
- **struct 字段**：snake_case 带尾缀下划线（`streamId_`, `width_`, `height_`）
- **方向注解**：`[in]`, `[out]` 显式标记每个参数
- **扩展方法**：新版本同名方法加 `_V1_6` 后缀（如 `CreateImageProcessSession_V1_6`）
- **Javadoc 风格注释**：`@brief`, `@since`, `@version`, `@param`, `@return`

### C++ 风格

- **命名空间**：`OHOS::HDI::Camera::V1_0`（四层嵌套），或 `OHOS::Camera`（metadata）
- **包含守卫**：`#ifndef OHOS_XXX_H / #define / #endif`
- **类命名**：PascalCase（`BufferHandleSequenceable`, `CameraMetadata`）
- **方法命名**：camelCase（`Marshalling`, `Unmarshalling`, `GetBufferHandle`）
- **变量命名**：snake_case（`bufferHandleWrap_`, `nativeBuffer_`），成员变量尾缀下划线
- **常量**：UPPER_SNAKE_CASE（`MAX_SUPPORTED_TAGS`, `METADATA_HEADER_DATA_SIZE`）
- **宏**：UPPER_SNAKE_CASE（`HDI_CAMERA_LOGE`, `METADATA_ERR_LOG`）
- **头文件顺序**：本模块头 → 标准库 → 系统库 → 外部库，每组空行分隔
- **空指针检查**：`if (ptr == nullptr) { HDI_CAMERA_LOGE("..."); return false; }`
- **日志宏**：sequenceable 用 `HDI_CAMERA_LOGE/LOGW/LOGI/LOGV/LOGD`，metadata 用 `METADATA_ERR_LOG/WARNING_LOG/INFO_LOG/FATAL_LOG/DEBUG_LOG`
- **生命周期**：`sptr` 引用计数指针；`new (std::nothrow)` 无异常分配
- **覆写**：虚方法显式标注 `override`
- **静态工具方法**：用于工厂/工具函数

### 构建风格

- `camera.gni`：两个特性开关 `drivers_interface_camera_feature_vendor_tag`（默认 true）和 `drivers_interface_camera_feature_coverage`（默认 false）
- `bundle.json`：`innerapi_tags` 含 `chipsetsdk` 和 `platformsdk_indirect`

## 5. 约束边界

### 架构不变量

- **`.idl` 是唯一真相源**：所有 proxy/stub/headers 由 `hdi-gen` 生成，本仓不包含手写 proxy/stub
- **metadata 和 sequenceable 是手写 C++ 库**：非 hdi-gen 产物，直接维护源码
- **8 个版本并行活跃构建**：v1_0~v1_7 全部在 `bundle.json` `sub_component` 中列出
- **版本链式依赖**：v1_7 → v1_6 → v1_5 → v1_4 → v1_3 → v1_2 → v1_1 → v1_0
- **扩展方法使用 `_V1_X` 后缀**：避免与基版本方法签名冲突
- **Types.idl 不跨版本共享**：每个版本有自己的 `Types.idl`，通过 `import` 引用旧版本类型

### 禁止事项

- **不要手改 `hdi-gen` 生成的 proxy/stub/headers**
- **不要删除或重命名已发布的 IDL 方法**
- **不要在 IDL struct 中间插入字段**（破坏 Parcelable ABI）
- **不要修改 `bundle.json` 的 `sub_component` 而不同步 `inner_kits`**
- **不要在本仓库中实现 HDI 逻辑**（实现在 `drivers/peripheral/camera`）
- **不要让 v1_7 跳过 v1_6 直接依赖旧版本**（须沿版本链传递）

### Agent 常见错误

- **手改 `_proxy.h/.cpp`、`_stub.h/.cpp`** → 应修改 `.idl` 源文件后重构建
- **在 IDL struct 中间插入字段** → 字段必须追加末尾以保持 ABI 兼容性
- **遗漏 `bundle.json` 同步** → 新增版本必须更新 `sub_component` 和 `inner_kits`
- **误改 v1_0 基础类型** → Types.idl 变更影响叶版本，需评估所有上游版本引用

## 6. 计划声明

任务开始前必须声明：
- **目标版本**（v1_0~v1_7），是否涉及 metadata 或 sequenceable
- **是否修改 `.idl` 方法签名** → ABI 影响
- **是否新增版本目录** → 需同步 bundle.json 和版本链
- **是否修改特性开关**（camera.gni）
- **是否同步 `drivers/peripheral/camera`** 外围仓实现