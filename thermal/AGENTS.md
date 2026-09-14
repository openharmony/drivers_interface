# Thermal HDI 接口 (interface) — Agent 工作指南

> 给 AI 编码助手使用。本目录是 OpenHarmony 温控 HDI 接口的 **IDL 定义**（source of truth）。proxy/stub/types 由 HDI 工具链从 `.idl` 生成，**不要手改生成产物**。

## 工作目录

```bash
OHOS_ROOT=<您的OpenHarmony源码路径>
THERMAL_IFACE_ROOT=${OHOS_ROOT}/drivers/interface/thermal
```

## 模块定位与职责

- **部件名**：`@ohos/drivers_interface_thermal`
- **职责**：定义温控 HDI 接口（`IThermalInterface`/`IThermalCallback`/`IFanCallback`）与类型（`ThermalTypes.idl`），供 thermal 服务（Proxy 侧）与 `drivers/peripheral/thermal`（Stub 侧）共享。
- **纯 IDL 仓**：本目录只有 `.idl` + 各版本 `BUILD.gn`，**无手写 C++**。
- **版本**：`v1_0`、`v1_1` 多版本并存；`v1_1` 引入 `IFanCallback`（风扇回调）。
- **风险面**：接口/枚举是公共兼容性边界；温控方法语义错位致限频/限流失控（过热损坏或性能骤降）。

## 业务架构

```
┌─────────────────────────────────────────────────────────┐
│      thermal_manager 服务 (Proxy 调用方 · 策略决策)       │
└─────────────────────────────────────────────────────────┘
        │  generated Proxy                  │  generated Types
        ▼                                   ▼
┌─────────────────────────────────────────────────────────┐
│     本目录: drivers/interface/thermal (IDL 真源)        │
│  ┌────────────────────────────────────────────────────┐ │
│  │ v1_0  IThermalInterface / IThermalCallback /       │ │
│  │       ThermalTypes                                   │ │
│  │ v1_1  + IFanCallback（风扇回调） + 扩展             │ │
│  └────────────────────────────────────────────────────┘ │
│            │ ohos_hdi codegen                           │
│            ▼                                             │
│  生成产物 (out/<product>/hdf/drivers_interface_thermal/):│
│    libthermal_proxy_1.*.z.so / libthermal_stub_1.*      │
│    thermal_types.cpp/h                                  │
└─────────────────────────────────────────────────────────┘
        │  Stub                                  │  Types
        ▼                                       ▼
┌─────────────────────────────────────────────────────────┐
│  drivers/peripheral/thermal (Stub 实现 · 采集+限频)     │
└─────────────────────────────────────────────────────────┘
```

### 各层职责

| 层 | 位置 | 职责 |
| --- | --- | --- |
| Proxy 调用方 | `base/powermgr/thermal_manager` | 策略决策，用 Proxy 调 HDI |
| IDL 真源 | `drivers/interface/thermal`（本目录） | 定义接口/回调/类型，唯一真源 |
| 生成产物 | `out/<product>/hdf/drivers_interface_thermal/` | ohos_hdi 从 IDL 生成 |
| Stub 实现 | `drivers/peripheral/thermal` | 按 IDL 实现（采集+限频） |

### 关键流程

**新增温控方法/回调**:
```
1. 在 v1_1 的 IThermalInterface.idl / IThermalCallback.idl 追加
2. 同步 drivers/peripheral/thermal Stub 实现（采集/限频）
3. 同步 thermal_manager Proxy 调用
4. 三方编译
```

**温控方法语义敏感**: `SetCpuFreq/SetGpuFreq/SetBatteryCurrent` 的参数单位（Hz/kHz）、电流方向须严格一致，改单位须整体评审。

## 代码路径（关键路径与"去哪找"）

| 任务 | 去哪看 |
| --- | --- |
| 温控主接口 IDL | `v1_0/IThermalInterface.idl`、`v1_1/IThermalInterface.idl` |
| 温控回调 IDL | `v1_0/IThermalCallback.idl`、`v1_1/IThermalCallback.idl` |
| 风扇回调 IDL（v1_1 新增） | `v1_1/IFanCallback.idl` |
| 类型/枚举 IDL | `v1_0/ThermalTypes.idl`、`v1_1/ThermalTypes.idl` |
| 版本构建配置 | `vN_M/BUILD.gn`；顶层 `bundle.json` |
| Stub 侧实现 | `drivers/peripheral/thermal` |
| Proxy 侧调用 | `base/powermgr/thermal_manager` 等上层 |

## 知识路由（动手前先读对应文档）

### 按任务路由

| 任务场景 | 必读 + 做法 |
| --- | --- |
| 新增 HDI 方法 | 优先在**最新版本** `v1_1` 的 `IThermalInterface.idl` 追加；不改旧版本；同步 `drivers/peripheral/thermal` 实现 + thermal Proxy |
| 新增温控回调事件 | 在 `v1_1/IThermalCallback.idl` 追加；同步 Stub 注册与 Proxy 回调 |
| 新增风扇回调 | `v1_1/IFanCallback.idl` 已存在；在其追加，或新开版本 |
| 新增/改枚举 | 在对应版本 `ThermalTypes.idl` 追加枚举值（只增不删不复用） |
| 新开版本 | 复制 `v1_1` 为 `v1_2`，新增 `BUILD.gn` 引用，更新 `bundle.json` |

### 按路径路由

| 改动路径 | 风险提示 |
| --- | --- |
| `vN_M/*.idl` | IDL 真源；改签名/枚举属兼容性边界，须同步 Proxy+Stub+调用方 |
| `vN_M/BUILD.gn` | 代码生成目标 |
| `bundle.json` | 部件元信息 |

### 按术语路由

| 术语 | 含义/风险 | 去哪看 |
| --- | --- | --- |
| IDL | 接口定义语言，HDI 真源 | `vN_M/*.idl` |
| proxy/stub | 由 IDL 生成；**勿手改** | `out/<product>/.../` 生成产物 |
| 版本（v1_0/v1_1） | 多版本并存，新版只增不删 | `vN_M/` |
| IFanCallback | 风扇回调，v1_1 引入 | `v1_1/IFanCallback.idl` |
| ThermalTypes | 温控枚举；只增不复用 | `ThermalTypes.idl` |

### 规划声明（动手编辑前必须明确）

先说明：①新增方法/回调/枚举还是新开版本；②是否必须新版本；③已读 Stub（peripheral/thermal）与 Proxy（thermal 上层）影响；④是否需"Ask before"。

## 约束与边界

### 架构与业务不变量

- `.idl` 是唯一真源；proxy/stub/types 是生成物，**不得手改**。
- 版本只增不删：`v1_0` 接口**不得删除或改签名**，只能在新版本追加。
- 枚举值只增不复用，避免版本错配时语义漂移。
- 接口改动须三处同步：IDL（本目录）+ Stub（`drivers/peripheral/thermal`）+ Proxy（thermal 上层）。
- 温控方法（SetCpuFreq/SetGpuFreq/SetBatteryCurrent）语义错位会致限频失控，签名/单位须严格一致。

### Do not

- 不得手改 `out/` 下的生成 proxy/stub/types。
- 不得删除或改旧版本（v1_0）的方法/枚举签名。
- 不得复用已废弃枚举值。
- 不得在 IDL 写实现逻辑。
- 不得只改 IDL 而不同步 Stub 与 Proxy。
- 不得改温控方法的参数单位/语义（如 freq 单位、current 方向）而不整体评审。

### Ask before

- 删除/废弃旧版本接口（公共兼容性边界，需评审）。
- 新开大版本（如 v1_2）。
- 改温控方法参数单位/语义。
- 改 `bundle.json` 或 `BUILD.gn` 生成目标。
- 新增第三方依赖。

### 已知易错点

- 只改 IDL 未同步 Stub/Proxy → 运行不一致。
- 在旧版本改签名 → 破坏向后兼容。
- 手改生成 proxy/stub → 下次生成覆盖或行为不一致。
- 枚举值复用旧值 → 版本错配语义漂移。
- 改温控方法参数单位（如 Hz/kHz、电流方向）未整体评审 → 限频失控致过热或性能骤降。
- 新版本目录未加 `BUILD.gn` → 不参与生成。

## 调试技巧

### 确认生成产物与版本一致

```bash
cd ${OHOS_ROOT}
ls out/rk3568/hdf/drivers_interface_thermal/
nm out/rk3568/hdf/drivers_interface_thermal/libthermal_proxy_1.1.z.so 2>/dev/null | grep <新方法符号>
# 三方版本对齐 + 温控方法单位核对
grep -r "v1_1\|THERMAL_HDI" base/powermgr/thermal_manager drivers/peripheral/thermal | head
```

### IDL 语法核对（无构建链时人工回退）

```bash
python3 -c "import glob; [print(f) for f in glob.glob('v*/*.idl')]"
# 人工核对：方法/枚举只增不删、温控方法参数单位/语义一致、v1_0/v1_1 版本边界、IFanCallback 仅 v1_1
```

## 常见问题

| 现象 | 可能原因 | 处理 |
| --- | --- | --- |
| 限频失控（过热/性能骤降） | 方法参数单位错位 | 核对 SetCpuFreq/SetGpuFreq/SetBatteryCurrent 单位与 Stub 一致 |
| Proxy/Stub 不一致 | 版本不匹配 | 三方用同版本 |
| 生成产物无新成员 | IDL 未在最新版本 | 改到 v1_1 |
| 风扇回调调不到 | IFanCallback 版本错 | IFanCallback 仅在 v1_1，确认 Stub/Proxy 用 v1_1 |
| v1_0 调用方编译失败 | 在 v1_0 改签名 | 改回，新方法只在 v1_1 |

## 验证闭环

### 最小验证命令（从源码根）

```bash
cd ${OHOS_ROOT}
./build.sh --product-name rk3568 --build-target drivers_interface_thermal
./build.sh --product-name rk3568 --build-target drivers_peripheral_thermal   # 同步 Stub
# IDL 格式/语法核对（人工：方法/枚举只增不删、温控方法参数单位/语义一致、v1_0/v1_1 版本边界）
python3 -c "import glob; [print(f) for f in glob.glob('v*/*.idl')]"
```

### 按变更类型的最小验证

| 变更类型 | 最小验证 |
| --- | --- |
| 新增方法/回调/枚举（最新版本） | 编译 interface + Stub + Proxy 三方；确认生成产物含新成员 |
| 新开版本 | 编译 interface + 确认新版本参与生成 + 旧版本仍可编译 |
| 改 BUILD.gn/bundle.json | 编译 interface + 确认生成产物路径/名与调用方一致 |

### Done 定义

任务完成须：①IDL 已改且必要时新版本目录+BUILD.gn 已加；②Stub（peripheral/thermal）与 Proxy（thermal 上层）已同步；③三方编译通过或说明无法运行原因；④`git diff` 仅含预期改动；⑤未手改生成产物；⑥枚举/方法只增不删；⑦温控方法单位/语义未随意改。

### 最终回复须包含

- 改了哪些 IDL/版本/BUILD.gn；②是否同步 Stub 与 Proxy；③运行了哪些验证及结果；④哪些验证因环境限制未运行及残留风险。

### 验证无法运行时的回退

环境缺 OHOS 构建链时不伪造构建结果：明确未运行命令与原因；至少对 `.idl` 做语法/格式人工核对（IDL 声明一致性 + 温控方法单位/语义核对），说明为非生成级校验。

## 代码仓依赖

| 路径 | 部件 | 作用 |
| --- | --- | --- |
| `drivers/interface/thermal` | `drivers_interface_thermal` | 温控 HDI IDL（v1_0/v1_1） |
| `drivers/peripheral/thermal` | `drivers_peripheral_thermal` | 温控 HDI Stub 实现 |
| `base/powermgr/thermal_manager` 等上层 | — | 温控服务（Proxy 调用方） |

## 构建产物（生成，勿手改）

```bash
${OHOS_ROOT}/out/rk3568/hdf/drivers_interface_thermal/
# 生成：libthermal_proxy_1.*.z.so、libthermal_stub_1.*.z.so、thermal_types.cpp/h 等
```
