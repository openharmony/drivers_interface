# Power HDI 接口 (interface) — Agent 工作指南

> 给 AI 编码助手使用。本目录是 OpenHarmony 电源 HDI 接口的 **IDL 定义**（source of truth）。`proxy`/`stub`/`types` 由 HDI 工具链从 `.idl` 生成，**不要手改生成产物**。

## 工作目录

```bash
OHOS_ROOT=<您的OpenHarmony源码路径>
POWER_IFACE_ROOT=${OHOS_ROOT}/drivers/interface/power
```

## 模块定位与职责

- **部件名**：`@ohos/drivers_interface_power`
- **职责**：定义电源 HDI 接口（`IPowerInterface`/`IPowerHdiCallback`/`IPowerRunningLockCallback`）与类型（`PowerTypes`/`RunningLockTypes`），供 powermgr 服务（Proxy 侧）与 `drivers/peripheral/power`（Stub 侧）共享。
- **纯 IDL 仓**：本目录只有 `.idl` + 各版本 `BUILD.gn`，**无手写 C++**。proxy/stub/types 由 `ohos_hdi` 工具链从 `.idl` 自动生成到 `out/<product>/...`。
- **版本**：`v1_0`、`v1_1`、`v1_2`、`v1_3` 多版本并存，新版只增不删以保向后兼容。
- **风险面**：接口签名/枚举改动是**公共兼容性边界**，影响 powermgr 服务、peripheral/power、所有调用方；版本管理错误致编译/运行不一致。

## 业务架构

```
┌─────────────────────────────────────────────────────────┐
│        powermgr_service (Proxy 调用方)                    │
└─────────────────────────────────────────────────────────┘
        │  uses generated Proxy              │  uses generated Types
        ▼                                   ▼
┌─────────────────────────────────────────────────────────┐
│       本目录: drivers/interface/power (IDL 真源)        │
│  ┌────────────────────────────────────────────────────┐ │
│  │ v1_0  IPowerInterface / IPowerHdiCallback /        │ │
│  │       PowerTypes                                     │ │
│  │ v1_1  + IPowerRunningLockCallback + PowerTypes 扩展│ │
│  │ v1_2  继续扩展                                      │ │
│  │ v1_3  + IPowerHdiCallbackExt                        │ │
│  └────────────────────────────────────────────────────┘ │
│            │ ohos_hdi 工具链 (codegen)                   │
│            ▼                                             │
│  生成产物 (out/<product>/hdf/drivers_interface_power/): │
│    libpower_proxy_1.*.z.so  ← Proxy (供上层)            │
│    libpower_stub_1.*.z.so   ← Stub  (供 peripheral)    │
│    power_types.cpp/h         ← 类型 (双方共用)          │
└─────────────────────────────────────────────────────────┘
        │  Stub                                  │  Types
        ▼                                       ▼
┌─────────────────────────────────────────────────────────┐
│  drivers/peripheral/power (Stub 实现，按 IDL 实现)       │
└─────────────────────────────────────────────────────────┘
```

### 各层职责

| 层 | 位置 | 职责 |
| --- | --- | --- |
| Proxy 调用方 | `base/powermgr/powermgr_service` | 用生成的 Proxy 调 HDI |
| IDL 真源 | `drivers/interface/power`（本目录） | 定义接口/回调/类型，**唯一真源** |
| 生成产物 | `out/<product>/hdf/drivers_interface_power/` | ohos_hdi 从 IDL 生成 proxy/stub/types |
| Stub 实现 | `drivers/peripheral/power` | 按 IDL 实现 HDI Stub |

### 关键流程

**IDL → 生成 → 使用**:
```
1. 改 .idl（新增方法/枚举，优先最新版本 v1_3）
2. ohos_hdi 工具链读 .idl → 生成 proxy/stub/types 到 out/
3. powermgr 链接 libpower_proxy_1.3.so 调用新方法
4. peripheral/power 链接 libpower_stub_1.3.so 实现新方法
5. 三方（IDL/Stub/Proxy）版本须一致，否则编译过运行不一致
```

**新增能力正确流程**: 改 v1_3 IDL → 同步 peripheral/power Stub → 同步 powermgr Proxy → 三方一起编译。

## 代码路径（关键路径与"去哪找"）

| 任务 | 去哪看 |
| --- | --- |
| 电源主接口 IDL | `v1_0/IPowerInterface.idl`、`v1_1/IPowerInterface.idl`、`v1_2/IPowerInterface.idl`、`v1_3/IPowerInterface.idl` |
| 回调 IDL | `v1_0/IPowerHdiCallback.idl`、`v1_1|v1_2/IPowerHdiCallback.idl`、`v1_1|v1_2/IPowerRunningLockCallback.idl`；`v1_3/IPowerHdiCallbackExt.idl` |
| 类型/枚举 IDL | `v1_0/PowerTypes.idl`、`v1_1|v1_2/PowerTypes.idl` + `RunningLockTypes.idl` |
| 版本构建配置 | 各 `vN_M/BUILD.gn`；顶层 `bundle.json` |
| Stub 侧实现 | `drivers/peripheral/power`（按 IDL 实现） |
| Proxy 侧调用 | `base/powermgr/powermgr_service`（用生成的 Proxy） |

## 知识路由（动手前先读对应文档）

### 按任务路由

| 任务场景 | 必读 + 做法 |
| --- | --- |
| 新增 HDI 方法 | 优先在**最新版本** `v1_3` 的 `IPowerInterface.idl` 追加方法；不改旧版本；同步在 `drivers/peripheral/power` 实现；同步 powermgr Proxy 调用 |
| 新增回调事件 | 在对应版本 `IPowerHdiCallback.idl`/`IPowerHdiCallbackExt.idl` 追加；同步 Stub 注册与 Proxy 回调 |
| 新增/改枚举 | 在对应版本 `PowerTypes.idl`/`RunningLockTypes.idl` 追加枚举值（只增不删、不复用旧值） |
| 新开大版本（如 v1_4） | 复制最新版本目录为 `v1_4`，新增顶层 `BUILD.gn` 引用，更新 `bundle.json` |
| 编译/生成失败 | 确认 `.idl` 语法、`BUILD.gn` 的 `ohos_hdi` 目标、HDI 工具链版本 |

### 按路径路由

| 改动路径 | 风险提示 |
| --- | --- |
| `vN_M/*.idl` | IDL 真源；改签名/枚举属兼容性边界，须同步 Proxy+Stub+调用方 |
| `vN_M/BUILD.gn` | 代码生成目标；改致生成产物路径/名变 |
| `bundle.json` | 部件元信息；改致依赖与构建范围变 |

### 按术语路由

| 术语 | 含义/风险 | 去哪看 |
| --- | --- | --- |
| IDL | 接口定义语言，HDI 真源 | `vN_M/*.idl` |
| proxy/stub | 由 IDL 生成，跨进程调用桩；**勿手改** | `out/<product>/.../` 生成产物 |
| 版本（v1_0~v1_3） | 多版本并存，新版只增不删 | `vN_M/` |
| PowerTypes/RunningLockTypes | 电源/锁枚举；只增不删不复用 | `*Types.idl` |

### 规划声明（动手编辑前必须明确）

先说明：①是新增方法/回调/枚举还是新开版本；②是否必须在新版本而非旧版本；③已读 Stub 侧（`drivers/peripheral/power`）与 Proxy 侧（powermgr）影响；④是否需"Ask before"。

## 约束与边界

### 架构与业务不变量

- `.idl` 是唯一真源；proxy/stub/types 是生成物，**不得手改生成产物**（改了会被下次生成覆盖或致不一致）。
- 版本只增不删：旧版本方法/枚举**不得删除或改签名**，只能在新版本追加。
- 枚举值只增不复用：已用值不得改语义，避免 Proxy/Stub 版本错配时值含义漂移。
- 接口改动须三处同步：IDL（本目录）+ Stub 实现（`drivers/peripheral/power`）+ Proxy 调用（powermgr）。

### Do not

- 不得手改 `out/` 下的生成 proxy/stub/types。
- 不得删除或改旧版本（v1_0~v1_2）的接口方法/枚举签名。
- 不得复用已废弃的枚举值。
- 不得在 IDL 里直接写实现逻辑（IDL 只声明）。
- 不得只改 IDL 而不同步 Stub 与 Proxy。

### Ask before

- 删除/废弃旧版本接口（属公共兼容性边界，需评审）。
- 新开大版本（如 v1_4）。
- 改 `bundle.json` 或 `BUILD.gn` 的代码生成目标。
- 新增第三方依赖。

### 已知易错点

- 只在 IDL 加方法未同步 Stub/Proxy → 编译过但运行调不到/不一致。
- 在旧版本改签名而非新版本追加 → 破坏向后兼容。
- 手改生成 proxy/stub → 下次生成覆盖或行为不一致。
- 枚举值复用旧值 → 版本错配时语义漂移。
- 新版本目录未加 `BUILD.gn` 引用 → 不参与生成。

## 调试技巧

### 确认生成产物与版本一致

```bash
cd ${OHOS_ROOT}
# 编译后查生成产物
ls out/rk3568/hdf/drivers_interface_power/
# 确认 Proxy/Stub 库与版本
nm out/rk3568/hdf/drivers_interface_power/libpower_proxy_1.3.z.so 2>/dev/null | grep <新方法符号>
# 确认三方版本对齐
grep -r "POWER_HDI_VERSION\|v1_3" base/powermgr/powermgr_service drivers/peripheral/power | head
```

### IDL 语法核对（无构建链时的人工回退）

```bash
# 列所有 IDL + 人工核对：方法签名只增不删、[in]/[out] 方向一致、枚举值只增不复用
python3 -c "import glob; [print(f) for f in glob.glob('v*/*.idl')]"
```

## 常见问题

| 现象 | 可能原因 | 处理 |
| --- | --- | --- |
| Proxy 调到的方法 Stub 没实现 | 只改 IDL 未同步 Stub | 同步 `drivers/peripheral/power` 实现 |
| 编译过但运行报错/不一致 | Proxy/Stub 版本不匹配 | 确认三方（IDL/Stub/Proxy）用同版本 |
| 生成产物无新成员 | IDL 未改到最新版本/工具链未跑 | 确认改在 v1_3，重跑 ohos_hdi |
| 旧调用方编译失败 | 在旧版本改了签名 | 改回旧版本，新方法只在新版本追加 |
| 新版本目录不参与生成 | 未加 BUILD.gn 引用 | 新版本目录加 `BUILD.gn` + 顶层引用 |

## 验证闭环

### 最小验证命令（从源码根）

```bash
cd ${OHOS_ROOT}
# 生成 + 编译 interface（生成 proxy/stub/types）
./build.sh --product-name rk3568 --build-target drivers_interface_power
# 同步编译 peripheral Stub 与 powermgr Proxy 确认三方一致
./build.sh --product-name rk3568 --build-target drivers_peripheral_power
# IDL 格式/语法核对（人工：方法签名/枚举值只增不删、参数方向 [in]/[out] 一致）
python3 -c "import re,sys; [print(f) for f in __import__('glob').glob('v*/*.idl') if not re.match(r'^\s*(interface|enum|package|\]|\[|[A-Za-z_]\w*\s*\[)',open(f).read())]"
```

### 按变更类型的最小验证

| 变更类型 | 最小验证 |
| --- | --- |
| 新增方法/回调/枚举（最新版本） | 编译 interface + Stub + Proxy 三方；确认生成产物含新成员 |
| 新开版本 | 编译 interface + 确认新版本目录参与生成 + 旧版本仍可编译 |
| 改 BUILD.gn/bundle.json | 编译 interface + 确认生成产物路径/名与调用方一致 |

### Done 定义

任务完成须：①IDL 已改且必要时新版本目录+BUILD.gn 已加；②Stub（peripheral/power）与 Proxy（powermgr）已同步；③三方编译通过或说明无法运行原因；④`git diff` 仅含预期改动；⑤未手改生成产物；⑥枚举/方法只增不删。

### 最终回复须包含

- 改了哪些 IDL/版本/BUILD.gn；②是否同步了 Stub 与 Proxy；③运行了哪些验证及结果；④哪些验证因环境限制未运行及残留风险。

### 验证无法运行时的回退

环境缺 OHOS 构建链（`hb`/ninja/产品配置/HDI 工具链）时不伪造构建结果：明确写出未运行的命令与原因；至少对 `.idl` 做语法/格式人工核对（IDL 声明一致性），并说明这是非生成级校验。

## 代码仓依赖

| 路径 | 部件 | 作用 |
| --- | --- | --- |
| `drivers/interface/power` | `drivers_interface_power` | 电源 HDI IDL（v1_0~v1_3） |
| `drivers/peripheral/power` | `drivers_peripheral_power` | 电源 HDI Stub 实现 |
| `base/powermgr/powermgr_service` | — | 电源服务（Proxy 调用方） |

## 构建产物（生成，勿手改）

```bash
${OHOS_ROOT}/out/rk3568/hdf/drivers_interface_power/
# 生成：libpower_proxy_1.*.z.so、libpower_stub_1.*.z.so、power_types.cpp/h 等
```
