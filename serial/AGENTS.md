# AGENTS.md - OpenHarmony 串口 HDI 接口（drivers_interface_serial）

## 1. 代码地图

本仓库定义 OpenHarmony 串口 HDI 接口（`@ohos/drivers_interface_serial`，版本 7.0，subsystem `hdf`，part `drivers_interface_serial`），是串口驱动与上层服务之间的 IDL 契约。核心职责是定义串口设备查询、打开/关闭、读写、流控信号（RTS/CTS/DTR/DSR）控制、以及数据上报回调。最重要的架构边界是**单版本（v1_0）、4 个 IDL 文件、`mode = "passthrough"` 透传模式、`innerapi_tags = ["chipsetsdk", "platformsdk_indirect"]`，proxy/stub 由 `hdi-gen` 工具在构建时生成，被 `drivers/peripheral/serial`（实现方）和上层串口服务（消费方）引用**。

### 非本项目维护的目录

以下内容属于其他团队或生成产物，不属于本项目维护范围，修改时请跳过：

- 任何由 `hdi-gen` 生成的 proxy/stub `.h/.cpp` 文件（构建产物，不在源码树中）
- 串口 HDI 实现由 `drivers/peripheral/serial` 仓维护，本仓是接口定义方，不包含实现
- 串口消费逻辑由上层服务仓维护，本仓不包含消费侧代码

### 嵌套指引

本仓目前无嵌套 AGENTS.md / CLAUDE.md / rules / skills 文件。本仓库结构极简（单版本、4 个 IDL 文件），不建议新建嵌套 AGENTS.md，所有指引在本文件中维护。

### 关键区域

- `v1_0/`：唯一版本目录，含 4 个 IDL 文件 + `BUILD.gn`
- `v1_0/ISerials.idl`：串口管理器主接口，定义 `QueryDevices`、`OpenDevice` 方法
- `v1_0/ISerialDevice.idl`：串口设备接口，定义 `StartRead`/`StopRead`/`Close`/`Write`/`Flush`/`Drain`/`SendBrkSignal`/`SetRtsSignal`/`GetCtsSignal`/`SetDtrSignal`/`GetDsrSignal` 方法
- `v1_0/ISerialDeviceCallback.idl`：串口设备回调接口，定义 `OnDeviceOffline`、`OnReadData` 方法
- `v1_0/SerialTypes.idl`：串口共享类型，定义 `SerialDeviceInfo`、`SerialConfig`
- `v1_0/BUILD.gn`：调用 `hdi-gen` 生成 proxy/stub/headers 的构建规则，`hdi("serials")` 模板，`module_name = "serials_service"`，`mode = "passthrough"`，`branch_protector_ret = "pac_ret"`，`innerapi_tags = ["chipsetsdk", "platformsdk_indirect"]`
- `bundle.json`：组件元数据、`sub_component`（1 个活跃目标）、`inner_kits`（2 个 inner_kits 条目：proxy + headers），`adapted_system_type: ["standard"]`

### Where to look

| 任务类型 | 先看哪里 |
|---|---|
| 串口管理接口变更 | `v1_0/ISerials.idl` -> `v1_0/SerialTypes.idl` 共享类型 -> `v1_0/BUILD.gn` 生成规则 |
| 串口设备接口变更 | `v1_0/ISerialDevice.idl` -> `v1_0/SerialTypes.idl` -> `v1_0/BUILD.gn` |
| 回调接口变更 | `v1_0/ISerialDeviceCallback.idl`（数据上报与设备离线回调）-> 评估消费方影响 |
| 共享类型变更 | `v1_0/SerialTypes.idl`（影响 `ISerials.idl` 和 `ISerialDevice.idl` 所有方法） |
| 新增串口方法 | `v1_0/ISerials.idl` 或 `v1_0/ISerialDevice.idl` 追加方法（保持 ABI 兼容，不能复用已有码值） |
| 流控信号变更 | `v1_0/ISerialDevice.idl` 中 `SetRtsSignal`/`GetCtsSignal`/`SetDtrSignal`/`GetDsrSignal` |
| 构建配置 | `bundle.json`（sub_component、inner_kits、deps）+ `v1_0/BUILD.gn` |

### 架构分层

```
串口消费层（上层串口服务，外部仓）
  ↓ 调用 proxy 库
IDL 代理层（本仓生成产物）
  libserials_proxy_1.0 (v1_0，passthrough 模式)
  ↓ IPC（HDF IPC 框架）
IDL 桩层（本仓生成产物）
  libserials_stub_1.0
  ↓ 由 drivers/peripheral/serial 实现
HDI 实现层（drivers/peripheral/serial，外部仓）

命名空间：ohos.hdi.serial.v1_0
```

## 2. 知识路由

在规划或编辑前，先对任务分类，读取对应的 IDL 文件和构建配置。

### Task-based routing

| 任务类型 | 读取 |
|---|---|
| 串口管理接口新增/修改 | `v1_0/ISerials.idl` + `v1_0/SerialTypes.idl` + `v1_0/BUILD.gn` |
| 串口设备接口新增/修改 | `v1_0/ISerialDevice.idl` + `v1_0/SerialTypes.idl` + `v1_0/BUILD.gn` |
| 回调接口新增/修改 | `v1_0/ISerialDeviceCallback.idl` + 评估消费方影响 |
| 共享类型新增/修改 | `v1_0/SerialTypes.idl`（影响 `ISerials.idl` 和 `ISerialDevice.idl` 所有方法）+ 评估外围仓 `drivers/peripheral/serial` 实现兼容性 |
| 新增串口方法 | `v1_0/ISerials.idl` 或 `v1_0/ISerialDevice.idl` 追加方法（保持 ABI 兼容）+ 同步外围仓 `drivers/peripheral/serial` 实现 |
| 流控信号变更 | `v1_0/ISerialDevice.idl` 中 RTS/CTS/DTR/DSR 方法 + 评估硬件流控实现 |
| 构建配置变更 | `bundle.json`（sub_component、inner_kits、deps）+ `v1_0/BUILD.gn` |

### Path-based routing

| 修改路径 | 需了解的上下文 |
|---|---|
| `v1_0/ISerials.idl` | 串口管理器主接口，定义设备查询和打开方法；`OpenDevice` 接受 `ISerialDeviceCallback` 回调参数并返回 `ISerialDevice` 设备句柄，破坏性变更会影响所有串口消费者 |
| `v1_0/ISerialDevice.idl` | 串口设备操作接口，含读写、流控、信号控制；`Write` 方法限制单次写入不超过 4096 字节 |
| `v1_0/ISerialDeviceCallback.idl` | 回调接口，定义 `OnDeviceOffline`（设备离线通知）和 `OnReadData`（数据上报）；通过 `OpenDevice` 注册回调 |
| `v1_0/SerialTypes.idl` | 唯一共享类型文件，修改会扩散到 `ISerials.idl` 和 `ISerialDevice.idl` 所有方法，Parcelable 字段顺序是 ABI 一部分 |
| `v1_0/BUILD.gn` | `hdi-gen` 生成规则，`mode = "passthrough"` 透传模式，`branch_protector_ret = "pac_ret"` 返回地址保护，proxy/stub/headers 产出目标定义 |
| `bundle.json` | 活跃构建目标与 inner_kits 清单，新增库必须在此注册 |

### Vocabulary-based routing

当任务、issue、日志、API 名称中出现以下术语时，先理解其含义和风险再动手：

| 术语 | 含义与风险 | 读取 |
|---|---|---|
| ISerials | 串口管理器主接口，定义 `QueryDevices`/`OpenDevice` 方法 | `v1_0/ISerials.idl` |
| ISerialDevice | 串口设备操作接口，含读写/流控/信号控制方法 | `v1_0/ISerialDevice.idl` |
| ISerialDeviceCallback | 串口设备回调接口，含 `OnDeviceOffline`/`OnReadData` 方法 | `v1_0/ISerialDeviceCallback.idl` |
| SerialDeviceInfo | 串口设备信息数据模型（端口名/制造商/序列号/产品ID/厂商ID） | `v1_0/SerialTypes.idl` |
| SerialConfig | 串口配置数据模型（波特率/数据位/停止位/校验/流控） | `v1_0/SerialTypes.idl` |
| QueryDevices | 查询可用串口设备列表方法 | `v1_0/ISerials.idl` |
| OpenDevice | 打开串口设备方法，接受端口名/配置/回调，返回设备句柄 | `v1_0/ISerials.idl` |
| StartRead / StopRead | 启动/停止读取串口设备 | `v1_0/ISerialDevice.idl` |
| Write | 写入数据方法，单次写入不超过 4096 字节 | `v1_0/ISerialDevice.idl` |
| Flush / Drain | 刷新缓冲区/等待写入完成 | `v1_0/ISerialDevice.idl` |
| SendBrkSignal | 发送 BRK（Break）信号 | `v1_0/ISerialDevice.idl` |
| SetRtsSignal / GetCtsSignal | 设置 RTS / 获取 CTS 硬件流控信号 | `v1_0/ISerialDevice.idl` |
| SetDtrSignal / GetDsrSignal | 设置 DTR / 获取 DSR 硬件流控信号 | `v1_0/ISerialDevice.idl` |
| OnDeviceOffline | 设备离线回调通知 | `v1_0/ISerialDeviceCallback.idl` |
| OnReadData | 数据上报回调，接受 byte 数组和数据长度 | `v1_0/ISerialDeviceCallback.idl` |
| hdi-gen | HDI IDL 代码生成工具，将 `.idl` 转为 proxy/stub/headers | `v1_0/BUILD.gn` 中的 `hdi` 模板调用 |
| passthrough | 透传模式，proxy/stub 在同进程直接调用，不经过 IPC 序列化 | `v1_0/BUILD.gn` 中 `mode = "passthrough"` |
| pac_ret | 返回地址保护（PAC），`branch_protector_ret` 编译选项 | `v1_0/BUILD.gn` |
| innerapi_tags | 内部 API 标签，`["chipsetsdk", "platformsdk_indirect"]`，暴露为芯片 SDK 和间接平台 SDK | `v1_0/BUILD.gn` |
| chipsetsdk | 芯片 SDK 标签，表示该接口对芯片厂商暴露 | `v1_0/BUILD.gn` |
| platformsdk_indirect | 间接平台 SDK 标签，表示该接口间接对平台应用暴露 | `v1_0/BUILD.gn` |
| libserials_proxy_1.0 / libserials_stub_1.0 | v1_0 生成的 proxy/stub 库 | 构建产物 |
| serials_idl_target | v1_0 的 IDL 构建目标 | `v1_0/BUILD.gn` |
| serials_idl_headers | v1_0 导出的 IDL 头文件目标 | `v1_0/BUILD.gn` + `bundle.json` inner_kits |
| ABI 契约 | 版本号即 ABI 契约，v1_0 proxy 不能与未来 v2_0 stub 通信 | `bundle.json` 中版本化 sub_component |
| 命名空间 | C++ 命名空间 `ohos.hdi.serial.v1_0` | 生成的头文件 |

在计划阶段，必须声明：
- **任务分类**（如：串口管理接口变更 / 串口设备接口变更 / 回调接口变更 / 共享类型变更 / 构建配置变更）
- **已读取的 IDL 文件和构建配置**（具体到文件路径）
- **发现的约束**（ABI 契约、命名空间、Parcelable 字段顺序、passthrough 模式、外围仓同步需求）
- **是否需要同步修改其他层**：
  - IDL 变更 -> 由 `hdi-gen` 重生成 proxy/stub/headers -> 同步外围仓 `drivers/peripheral/serial` 实现 -> 同步消费方
  - `SerialTypes.idl` 类型变更 -> 评估 `ISerials.idl` 和 `ISerialDevice.idl` 所有方法的引用
  - 新增版本 -> 同步 `bundle.json` + 外围仓实现 + 消费方适配

## 3. 约束边界

### 架构不变量

- **`.idl` 文件是唯一真相源**：所有 proxy/stub/headers 由 `hdi-gen` 在构建时生成，本仓库不包含手写的 proxy/stub 实现代码
- **单版本 v1_0**：本仓库仅含 v1_0 一个版本，所有接口与类型集中在此，无版本分裂
- **版本号即 ABI 契约**：v1_0 proxy 不能与未来 v2_0 stub 通信，跨版本交互必须通过版本化库显式选择
- **命名空间与版本绑定**：C++ 命名空间为 `ohos.hdi.serial.v1_0`，版本号嵌入命名空间
- **`SerialTypes.idl` 是版本内共享类型源**：`ISerials.idl` 和 `ISerialDevice.idl` 共享同一 `SerialTypes.idl`，类型变更会扩散到所有方法
- **`passthrough` 透传模式**：`mode = "passthrough"`，proxy/stub 在同进程直接调用，不经过 IPC 序列化，变更需评估透传模式下的调用路径
- **`branch_protector_ret = "pac_ret"`**：启用返回地址保护（PAC），生成代码需兼容此安全特性
- **`no_other_header_files = true`**：不导出额外头文件，仅导出 IDL 定义的头文件
- **`Write` 方法 4096 字节限制**：`ISerialDevice.Write` 单次写入数据不超过 4096 字节，这是接口契约约束
- **无 .gni 特性开关**：本仓库无 `*.gni` 文件，无特性开关，构建配置集中在 `bundle.json`
- **inner_kits 暴露的 headers/proxy 是公共 ABI**：变更需评估所有下游消费者
- **本仓是接口定义方**：实现由 `drivers/peripheral/serial` 维护

### 禁止事项

- **不要手改 `hdi-gen` 生成的 proxy/stub/headers 代码**：应修改 `.idl` 源文件后由构建系统重生成
- **不要删除或重命名已发布的 IDL 方法**：IDL 方法签名是 ABI 契约，删除或重命名会破坏旧客户端
- **不要复用或调整已有 IDL 方法的码值**：新增接口必须追加新方法，不能复用已删除方法的码值
- **不要修改 `bundle.json` 的 `sub_component` 而不同步 `inner_kits`**：新增构建目标必须同步注册 inner_kits 以暴露给下游
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审（当前依赖：`ipc` / `hdf_core` / `hilog` / `c_utils`）
- **不要在本仓库中实现 HDI 逻辑**：本仓是接口定义方，实现由 `drivers/peripheral/serial` 维护
- **不要移除 `innerapi_tags`**：v1_0 暴露为 `chipsetsdk` / `platformsdk_indirect`，移除标签会破坏下游 SDK 依赖
- **不要修改 `mode = "passthrough"` 而不评估影响**：透传模式决定 proxy/stub 调用路径，切换模式影响所有消费者
- **不要移除 `no_other_header_files = true`**：该标志限制头文件导出范围，移除会扩大 ABI 暴露面

### Ask before 规则

在执行以下操作前，必须先确认或询问：

| 操作 | 确认内容 | 确认对象 |
|------|----------|----------|
| 修改 `ISerials.idl` 方法签名 | 确认 ABI 兼容性和下游消费者依赖 | 接口评审团队 |
| 修改 `ISerialDevice.idl` 方法签名 | 确认 ABI 兼容性和下游消费者依赖 | 接口评审团队 |
| 修改 `ISerialDeviceCallback.idl` 回调 | 确认消费方回调处理逻辑兼容性 | 消费方团队 |
| 修改 `SerialTypes.idl` 类型 | 确认 Parcelable 字段顺序（ABI 一部分） | 接口评审团队 |
| 新增主版本（如 v2_0） | 确认外围仓 `drivers/peripheral/serial` 实现计划 + 消费方适配 | 项目负责人 |
| 修改 `bundle.json` 的 `sub_component` 或 `inner_kits` | 确认下游消费者依赖 | 项目负责人 |
| 修改 `innerapi_tags` | 确认芯片 SDK 和平台 SDK 兼容性 | 芯片适配团队 |
| 修改 `mode = "passthrough"` | 确认透传模式切换对调用路径的影响 | 构建团队 |
| 引入新的生产依赖 | 确认许可证、包大小、`bundle.json` 同步 | 依赖评审团队 |

### 项目特定陷阱

- **`SerialDeviceInfo` / `SerialConfig` 字段顺序是 ABI**：IDL 中 struct 字段顺序决定 Parcelable 序列化顺序，新增字段必须追加在末尾，否则反序列化失败
- **单版本无回退路径**：本仓库仅 v1_0，破坏性变更没有历史版本作为回退，需格外谨慎
- **`Write` 方法 4096 字节限制**：`ISerialDevice.Write` 单次写入数据不超过 4096 字节，消费方需分片写入大数据
- **`passthrough` 模式的调用路径**：透传模式下 proxy/stub 在同进程直接调用，不经过 IPC 序列化，调试时需注意调用路径与 IPC 模式不同
- **`OpenDevice` 的回调注册**：`OpenDevice` 接受 `ISerialDeviceCallback` 回调参数，回调的生命周期与设备句柄绑定，关闭设备需注销回调
- **`branch_protector_ret = "pac_ret"` 的安全约束**：启用 PAC 返回地址保护，生成代码和消费者代码需兼容此特性
- **`innerapi_tags` 的双重标签**：v1_0 标记 `chipsetsdk` + `platformsdk_indirect`，意味着这些接口同时对芯片厂商和间接平台应用暴露，变更影响范围广
- **无 .gni 特性开关**：本仓库无特性开关，所有构建配置集中在 `bundle.json`，不要期望通过 `*.gni` 裁剪功能
- **`adapted_system_type: ["standard"]`**：仅适配 standard 系统，不包含 small/mini 系统适配，无 `ohos_lite` 条件分支

### Agent 失败模式

| 失败模式 | 描述 | 预防 |
|----------|------|------|
| 手改生成代码 | 直接修改 `hdi-gen` 生成的 proxy/stub/headers | 始终修改 `.idl` 源文件后由构建系统重生成 |
| 版本混淆 | 假设存在历史版本可回退，在 v1_0 中引入破坏性变更 | 确认本仓仅 v1_0 单版本，破坏性变更无回退路径 |
| ABI 破坏 | 修改 IDL 中字段顺序或方法签名 | 跨版本交互必须通过版本化库显式选择，字段顺序固定 |
| 跨层同步遗漏 | 修改了 IDL 但未同步 `drivers/peripheral/serial` 实现 | 修改 IDL 后必须验证外围仓实现 |
| inner_kits 不同步 | 修改 `sub_component` 但未同步 `inner_kits` | 修改后必须验证 `bundle.json` 字段一致性 |
| innerapi_tags 误改 | 随意修改 `chipsetsdk`/`platformsdk_indirect` 标签 | 修改前确认芯片 SDK 和平台 SDK 影响 |
| 码值复用 | 复用已有 IDL 方法的码值 | 新增接口必须追加新方法 |
| 忽略 4096 限制 | 在 `Write` 方法中忽略单次写入 4096 字节限制 | 确认消费方分片写入逻辑 |
| passthrough 模式误解 | 假设 proxy/stub 经过 IPC 序列化 | 确认透传模式下同进程直接调用 |

## 4. 验证闭环

### 最小验证

```bash
# 构建 serial 接口组件（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target drivers_interface_serial

# 单独构建活跃版本
./build.sh --product-name rk3568 --build-target //drivers/interface/serial/v1_0:serials_idl_target
./build.sh --product-name rk3568 --build-target //drivers/interface/serial/v1_0:libserials_proxy_1.0
./build.sh --product-name rk3568 --build-target //drivers/interface/serial/v1_0:libserials_stub_1.0
```

### Lint and static analysis

```bash
# 格式检查
gn format --check v1_0/

# 构建目标 lint 检查
./build.sh --product-name rk3568 --build-target drivers_interface_serial --lint

# 静态分析：验证 GN 构建配置语法
gn gen --check

# IDL 语法检查
hdi-gen --check

# 检查生成产物是否被手改
git diff --name-only -- '**/_proxy.h' '**/_proxy.cpp' '**/_stub.h' '**/_stub.cpp' | grep -v 'BUILD.gn' | head -20
```

### 任务特定验证

| 任务类型 | 验证命令 | 验证目标 |
|----------|----------|----------|
| 串口管理接口变更 | `./build.sh --product-name rk3568 --build-target drivers_interface_serial` + 同步构建外围仓 `drivers/peripheral/serial` | IDL 生成一致性 + 实现兼容 |
| 串口设备接口变更 | `./build.sh --product-name rk3568 --build-target drivers_interface_serial` + 同步构建外围仓 `drivers/peripheral/serial` | IDL 生成一致性 + 实现兼容 |
| 回调接口变更 | 构建全量 + 验证 `OpenDevice` 回调注册路径 + 同步消费方回调处理逻辑 | 回调生命周期正确 |
| 共享类型变更 | 构建全量 + 验证 `ISerials.idl` 和 `ISerialDevice.idl` 所有方法引用编译通过 + Parcelable 字段顺序检查 | 类型扩散验证 |
| 新增串口方法 | 重新生成 proxy/stub + 同步外围仓 `drivers/peripheral/serial` 实现 + 运行 ABI 兼容性检查 | ABI 兼容 |
| 新增主版本 | 新建 `vN_0/` 目录 + 修改 `bundle.json` + 全量构建 + 验证外围仓实现 + 消费方适配 | 版本注册正确 |
| 构建配置变更 | `./build.sh --product-name rk3568 --build-target drivers_interface_serial` + 验证 `inner_kits` 暴露的 headers 可被下游引用 | 配置正确性 |
| innerapi_tags 变更 | 构建全量 + 验证芯片 SDK 和平台 SDK 依赖未破坏 + 同步下游 SDK 团队 | SDK 兼容性 |

### 通用验证步骤

```bash
# 构建接口组件
./build.sh --product-name rk3568 --build-target drivers_interface_serial

# 静态分析（Lint）
./build.sh --product-name rk3568 --build-target drivers_interface_serial --lint
gn gen --check
hdi-gen --check

# 验证 inner_kits
python3 -c "import json; d=json.load(open('bundle.json')); assert 'inner_kits' in d['component']['build']; print('inner_kits:', d['component']['build']['inner_kits'])"

# 验证版本号与 bundle.json 一致
python3 -c "import json; d=json.load(open('bundle.json')); print('sub_component:', d['component']['build']['sub_component'])"
```

### Done 定义

任务完成的判定标准：
1. 构建命令执行成功（`./build.sh --product-name rk3568 --build-target drivers_interface_serial` 返回 0）
2. 静态分析通过（`--lint` 返回 0，`gn gen --check` 通过，`hdi-gen --check` 通过）
3. `bundle.json` 中 `sub_component` 和 `inner_kits` 同步更新
4. `innerapi_tags` 影响范围已确认（如适用）
5. 跨层同步已验证（IDL 与外围仓 `drivers/peripheral/serial` 实现匹配）
6. proxy/stub 由 `hdi-gen` 自动重新生成，无手改生成产物
7. 无新增编译警告
8. 变更范围与任务要求一致，未夹带未关联的重构
9. `SerialTypes.idl` 类型变更已评估 `ISerials.idl` 和 `ISerialDevice.idl` 所有方法的引用
10. 活跃版本（v1_0）的变更已评估 ABI 兼容性，未删除或重命名已发布方法

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 目标版本（v1_0）及是否为活跃构建
4. 是否触发跨层同步修改（IDL 重生成 / 外围仓 `drivers/peripheral/serial` / 消费方 / `bundle.json`）
5. 是否影响 ABI 契约或 `passthrough` 透传模式
6. 是否触及架构不变量或需确认事项（`innerapi_tags` / `mode` / `branch_protector_ret`）
7. 所有验证命令的执行结果（构建/lint/静态分析）

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样，不能假称已通过。涉及外围仓 `drivers/peripheral/serial` 的变更，必须人工复核 IDL 与实现的匹配性并说明无法在沙箱验证的限制。涉及 `passthrough` 模式或 `innerapi_tags` 的变更，必须人工复核调用路径和 SDK 依赖影响。
