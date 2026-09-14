# Battery HDI 接口 (interface) — Agent 工作指南

> 给 AI 编码助手使用。本目录是 OpenHarmony 电池 HDI 接口的 **IDL 定义**（source of truth）。proxy/stub/types 由 HDI 工具链从 `.idl` 生成，**不要手改生成产物**。

## 工作目录

```bash
OHOS_ROOT=<您的OpenHarmony源码路径>
BATTERY_IFACE_ROOT=${OHOS_ROOT}/drivers/interface/battery
```

## 模块定位与职责

- **部件名**：`@ohos/drivers_interface_battery`
- **职责**：定义电池 HDI 接口（`IBatteryInterface`/`IBatteryCallback`）与类型（`Types.idl`，含 `BatteryHealthState`/`BatteryPluggedType`/`BatteryChargeState` 等），供 battery 服务（Proxy 侧）与 `drivers/peripheral/battery`（Stub 侧）共享。
- **纯 IDL 仓**：本目录只有 `.idl` + 各版本 `BUILD.gn`，**无手写 C++**。
- **版本**：`v1_0`、`v1_1`、`v1_2`、`v2_0` 多版本并存；`v2_0` 是大版本升级，v1→v2 不兼容改动须走新版本。
- **风险面**：接口/枚举是公共兼容性边界；版本管理错误致 Proxy/Stub 不一致；电池枚举语义错位致 UI/策略误判。

## 业务架构

```
┌─────────────────────────────────────────────────────────┐
│       battery_info 服务 (Proxy 调用方)                   │
└─────────────────────────────────────────────────────────┘
        │  generated Proxy                  │  generated Types
        ▼                                   ▼
┌─────────────────────────────────────────────────────────┐
│     本目录: drivers/interface/battery (IDL 真源)        │
│  ┌────────────────────────────────────────────────────┐ │
│  │ v1_0  IBatteryInterface / IBatteryCallback / Types │ │
│  │ v1_1  扩展                                          │ │
│  │ v1_2  扩展                                          │ │
│  │ v2_0  大版本升级（v1→v2 不兼容改动走 v2_0）           │ │
│  └────────────────────────────────────────────────────┘ │
│            │ ohos_hdi codegen                           │
│            ▼                                             │
│  生成产物 (out/<product>/hdf/drivers_interface_battery/):│
│    libbattery_proxy_1.*.z.so  / libbattery_stub_1.*    │
│    battery_types.cpp/h                                   │
└─────────────────────────────────────────────────────────┘
        │  Stub                                  │  Types
        ▼                                       ▼
┌─────────────────────────────────────────────────────────┐
│  drivers/peripheral/battery (Stub 实现)                  │
└─────────────────────────────────────────────────────────┘
```

### 各层职责

| 层 | 位置 | 职责 |
| --- | --- | --- |
| Proxy 调用方 | `base/powermgr/battery_info` 等 | 用 Proxy 调 HDI |
| IDL 真源 | `drivers/interface/battery`（本目录） | 定义接口/回调/类型，唯一真源 |
| 生成产物 | `out/<product>/hdf/drivers_interface_battery/` | ohos_hdi 从 IDL 生成 |
| Stub 实现 | `drivers/peripheral/battery` | 按 IDL 实现 |

### 关键流程

**新增电池属性/枚举**:
```
1. 在最新版本 v2_0 的 Types.idl 追加枚举值（BatteryHealthState/PluggedType/ChargeState，只增不复用）
2. 同步 drivers/peripheral/battery Stub 读取映射
3. 同步 battery 上层 Proxy 使用
4. 三方编译
```

**v1→v2 不兼容升级**: 已在 v2_0；新不兼容改动只走 v2_0 或新开 v2_1，不动 v1_*。

## 代码路径（关键路径与"去哪找"）

| 任务 | 去哪看 |
| --- | --- |
| 电池主接口 IDL | `v1_0/IBatteryInterface.idl`、`v1_1/IBatteryInterface.idl`、`v1_2/IBatteryInterface.idl`、`v2_0/IBatteryInterface.idl` |
| 回调 IDL | `v1_0/IBatteryCallback.idl` ~ `v2_0/IBatteryCallback.idl` |
| 类型/枚举 IDL | `v1_0/Types.idl`、`v1_1/Types.idl`、`v1_2/Types.idl`、`v2_0/Types.idl` |
| 版本构建配置 | 各 `vN_M/BUILD.gn`；顶层 `bundle.json` |
| Stub 侧实现 | `drivers/peripheral/battery` |
| Proxy 侧调用 | `base/powermgr/battery_info` 等上层 |

## 知识路由（动手前先读对应文档）

### 按任务路由

| 任务场景 | 必读 + 做法 |
| --- | --- |
| 新增 HDI 方法 | 优先在**最新版本** `v2_0` 的 `IBatteryInterface.idl` 追加；不改旧版本；同步 `drivers/peripheral/battery` 实现 + battery Proxy |
| 新增电池属性/枚举 | 在对应版本 `Types.idl` 追加枚举值（只增不删、不复用旧值）；如 `BatteryHealthState`/`BatteryPluggedType`/`BatteryChargeState` |
| 新增回调事件 | 在对应版本 `IBatteryCallback.idl` 追加；同步 Stub 注册与 Proxy 回调 |
| v1→v2 不兼容升级 | 已在 `v2_0`；新不兼容改动只走 `v2_0` 或新开 `v2_1`，不动 v1_* |
| 新开版本 | 复制最新版本目录，新增 `BUILD.gn` 引用，更新 `bundle.json` |

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
| 版本（v1_0~v2_0） | 多版本并存，新版只增不删；v2_0 是大版本 | `vN_M/` |
| BatteryHealthState/PluggedType/ChargeState | 电池枚举；只增不复用 | `Types.idl` |

### 规划声明（动手编辑前必须明确）

先说明：①新增方法/枚举/回调还是新开版本；②是否必须新版本；③已读 Stub（peripheral/battery）与 Proxy（battery 上层）影响；④是否需"Ask before"。

## 约束与边界

### 架构与业务不变量

- `.idl` 是唯一真源；proxy/stub/types 是生成物，**不得手改**。
- 版本只增不删：v1_0/v1_1/v1_2/v2_0 旧版本接口**不得删除或改签名**。
- 枚举值只增不复用，避免版本错配时语义漂移。
- 接口改动须三处同步：IDL（本目录）+ Stub（`drivers/peripheral/battery`）+ Proxy（battery 上层）。

### Do not

- 不得手改 `out/` 下的生成 proxy/stub/types。
- 不得删除或改旧版本（v1_0~v1_2）的方法/枚举签名。
- 不得复用已废弃枚举值。
- 不得在 IDL 写实现逻辑。
- 不得只改 IDL 而不同步 Stub 与 Proxy。

### Ask before

- 删除/废弃旧版本接口（公共兼容性边界，需评审）。
- 新开大版本（如 v2_1）。
- 改 `bundle.json` 或 `BUILD.gn` 生成目标。
- 新增第三方依赖。

### 已知易错点

- 只改 IDL 未同步 Stub/Proxy → 运行不一致。
- 在旧版本改签名 → 破坏向后兼容。
- 手改生成 proxy/stub → 下次生成覆盖或行为不一致。
- 枚举值复用旧值 → 版本错配语义漂移。
- 新版本目录未加 `BUILD.gn` → 不参与生成。

## 调试技巧

### 确认生成产物与版本一致

```bash
cd ${OHOS_ROOT}
ls out/rk3568/hdf/drivers_interface_battery/
nm out/rk3568/hdf/drivers_interface_battery/libbattery_proxy_2.0.z.so 2>/dev/null | grep <新方法符号>
# 三方版本对齐
grep -r "v2_0\|BATTERY_HDI" base/powermgr/battery_info drivers/peripheral/battery | head
```

### IDL 语法核对（无构建链时人工回退）

```bash
python3 -c "import glob; [print(f) for f in glob.glob('v*/*.idl')]"
# 人工核对：方法/枚举只增不删、v1/v2 版本边界、[in]/[out] 方向一致
```

## 常见问题

| 现象 | 可能原因 | 处理 |
| --- | --- | --- |
| 电量显示枚举错位 | 枚举值复用旧值 | 确认 Types.idl 枚举只增不复用 |
| Proxy/Stub 不一致 | 版本不匹配 | 三方用同版本 |
| 生成产物无新成员 | IDL 未在最新版本 | 改到 v2_0 |
| v1 调用方编译失败 | 在 v1 改了签名 | 改回，新方法只在 v2_0 |
| 新版本不参与生成 | 未加 BUILD.gn | 新版本目录加 BUILD.gn + 顶层引用 |

## 验证闭环

### 最小验证命令（从源码根）

```bash
cd ${OHOS_ROOT}
./build.sh --product-name rk3568 --build-target drivers_interface_battery
./build.sh --product-name rk3568 --build-target drivers_peripheral_battery   # 同步 Stub
# IDL 格式/语法核对（人工：方法/枚举只增不删、v1/v2 版本边界、参数 [in]/[out] 一致）
python3 -c "import glob; [print(f) for f in glob.glob('v*/*.idl')]"
```

### 按变更类型的最小验证

| 变更类型 | 最小验证 |
| --- | --- |
| 新增方法/回调/枚举（最新版本） | 编译 interface + Stub + Proxy 三方；确认生成产物含新成员 |
| 新开版本 | 编译 interface + 确认新版本参与生成 + 旧版本仍可编译 |
| 改 BUILD.gn/bundle.json | 编译 interface + 确认生成产物路径/名与调用方一致 |

### Done 定义

任务完成须：①IDL 已改且必要时新版本目录+BUILD.gn 已加；②Stub（peripheral/battery）与 Proxy（battery 上层）已同步；③三方编译通过或说明无法运行原因；④`git diff` 仅含预期改动；⑤未手改生成产物；⑥枚举/方法只增不删。

### 最终回复须包含

- 改了哪些 IDL/版本/BUILD.gn；②是否同步 Stub 与 Proxy；③运行了哪些验证及结果；④哪些验证因环境限制未运行及残留风险。

### 验证无法运行时的回退

环境缺 OHOS 构建链时不伪造构建结果：明确未运行命令与原因；至少对 `.idl` 做语法/格式人工核对（IDL 声明一致性），说明为非生成级校验。

## 代码仓依赖

| 路径 | 部件 | 作用 |
| --- | --- | --- |
| `drivers/interface/battery` | `drivers_interface_battery` | 电池 HDI IDL（v1_0/v1_1/v1_2/v2_0） |
| `drivers/peripheral/battery` | `drivers_peripheral_battery` | 电池 HDI Stub 实现 |
| `base/powermgr/battery_info` 等上层 | — | 电池服务（Proxy 调用方） |

## 构建产物（生成，勿手改）

```bash
${OHOS_ROOT}/out/rk3568/hdf/drivers_interface_battery/
# 生成：libbattery_proxy_1.*.z.so、libbattery_stub_1.*.z.so、battery_types.cpp/h 等
```
