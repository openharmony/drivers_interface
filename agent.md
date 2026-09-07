# 外设驱动 HDI 接口定义仓 — Agent 指引

面向在 OpenHarmony `drivers_interface` 仓（子系统 `hdf`，各外设 HDI 接口定义）
工作的 OpenCode 智能体。本仓**仅定义接口**（IDL），不含服务实现；实现端在
`drivers_peripheral` 各对应仓。

> 嵌套指引：本仓内每个一级子模块目录下均设有 `agent.md`，提供该模块的版本目录、
> IDL 文件清单与约定。深度知识路由到子模块 `agent.md`（见下方「子模块导航」）。

## 仓概述

本仓管理各外设模块的 HDI（Hardware Device Interface）接口定义。接口使用 IDL
语言描述，保存为 `.idl` 文件；编译时由 `//build/config/components/hdi/hdi.gni`
模板自动生成 C/C++ 的客户端代理（proxy）与服务端桩（stub）代码，开发者只需基于
生成的 `ifoo_interface.h` 实现具体服务功能。

**接口与实现分离原则**：
- 本仓（`drivers_interface`）= 只定义接口（`.idl` + `BUILD.gn`）。
- 实现仓（`drivers_peripheral/<module>`）= 服务实现 + 驱动入口。
- HDI 框架核心（`drivers/hdf_core`）= 编译模板 `hdi.gni`、IPC/直通模式框架。

## 全局工作流

1. **新增接口**：在 `<module>/vX_Y/` 下编写 `IFoo.idl`（接口）、
   `FooTypes.idl`（类型）、`IFooCallback.idl`（回调），并写 `BUILD.gn`。
2. **编译生成**：`interface("<module>")` 模板触发 IDL→C/C++ 代码生成，
   产出 `lib<module>_proxy_vX.Y.z.so`（客户端）与
   `lib<module>_stub_vX.Y.z.so`（服务端桩）。
3. **实现服务**：在 `drivers/peripheral/<module>/` 继承生成头文件实现接口。
4. **发布服务**：在产品 hcs 配置中声明 HDI 服务（`device_info.hcs`）。
5. **调用服务**：客户端 BUILD.gn 依赖
   `//drivers/interface/<module>/vX.Y:lib<module>_proxy_vX.Y`，代码中
   `IFoo::Get()` 获取客户端实例。

## 子模块导航

本仓每个一级子目录定义一个外设模块的 HDI 接口，并配有独立的 `agent.md`。
按功能分组如下：

### 媒体与图形

| 模块 | 中文名 | 说明 | Agent 指引 |
|------|--------|------|------------|
| `audio` | 音频 | 音频适配器/录音/渲染/回调接口 | [audio/agent.md](audio/agent.md) |
| `distributed_audio` | 分布式音频 | 分布式音频扩展接口 | [distributed_audio/agent.md](distributed_audio/agent.md) |
| `camera` | 相机 | CameraHost/Device/StreamOperator 接口 | [camera/agent.md](camera/agent.md) |
| `distributed_camera` | 分布式相机 | 分布式相机接口 | [distributed_camera/agent.md](distributed_camera/agent.md) |
| `display` | 显示 | Composer/Buffer/Graphic 接口 | [display/agent.md](display/agent.md) |
| `codec` | 编解码 | 编解码器/图像/Parcel 接口 | [codec/agent.md](codec/agent.md) |
| `drm` | 数字版权管理 | DRM 接口 | [drm/agent.md](drm/agent.md) |

### 输入与交互

| 模块 | 中文名 | 说明 | Agent 指引 |
|------|--------|------|------------|
| `input` | 输入 | 输入设备 DDK 接口 | [input/agent.md](input/agent.md) |
| `vibrator` | 振动马达 | 振动马达接口 | [vibrator/agent.md](vibrator/agent.md) |
| `light` | 指示灯 | 指示灯接口 | [light/agent.md](light/agent.md) |
| `midi` | MIDI | MIDI 接口 | [midi/agent.md](midi/agent.md) |

### 传感器与感知

| 模块 | 中文名 | 说明 | Agent 指引 |
|------|--------|------|------------|
| `sensor` | 传感器 | Sensor 接口与回调 | [sensor/agent.md](sensor/agent.md) |
| `motion` | 动作 | 动作识别接口 | [motion/agent.md](motion/agent.md) |
| `activity_recognition` | 活动识别 | 活动识别接口 | [activity_recognition/agent.md](activity_recognition/agent.md) |
| `intelligent_voice` | 智能语音 | 智能语音引擎/触发接口 | [intelligent_voice/agent.md](intelligent_voice/agent.md) |
| `nnrt` | 神经网络推理 | NNRT 推理接口 | [nnrt/agent.md](nnrt/agent.md) |

### 电源与热管理

| 模块 | 中文名 | 说明 | Agent 指引 |
|------|--------|------|------------|
| `power` | 电源 | 电源管理接口 | [power/agent.md](power/agent.md) |
| `battery` | 电池 | 电池信息/充电接口 | [battery/agent.md](battery/agent.md) |
| `thermal` | 温控 | 温控接口 | [thermal/agent.md](thermal/agent.md) |
| `memorytracker` | 内存追踪 | 内存追踪接口 | [memorytracker/agent.md](memorytracker/agent.md) |

### 通信与连接

| 模块 | 中文名 | 说明 | Agent 指引 |
|------|--------|------|------------|
| `wlan` | WLAN | WLAN/芯片/WPA/Hostapd 接口 | [wlan/agent.md](wlan/agent.md) |
| `bluetooth` | 蓝牙 | A2DP/HCI/LP_BLE 接口 | [bluetooth/agent.md](bluetooth/agent.md) |
| `nearlink` | 星闪 | 星闪 HCI/OffFind 接口 | [nearlink/agent.md](nearlink/agent.md) |
| `nfc` | NFC | NFC 接口 | [nfc/agent.md](nfc/agent.md) |
| `connected_nfc_tag` | 连接式 NFC 标签 | 连接式 NFC 标签接口 | [connected_nfc_tag/agent.md](connected_nfc_tag/agent.md) |
| `secure_element` | 安全单元 | SE/SIM SE 接口 | [secure_element/agent.md](secure_element/agent.md) |
| `ethernet` | 以太网 | 以太网接口 | [ethernet/agent.md](ethernet/agent.md) |
| `ril` | RIL 无线接口 | RIL 蜂窝通信接口 | [ril/agent.md](ril/agent.md) |
| `location` | 定位 | AGNSS/Geofence/GNSS 接口 | [location/agent.md](location/agent.md) |

### USB 与串行

| 模块 | 中文名 | 说明 | Agent 指引 |
|------|--------|------|------------|
| `usb` | USB | USB DDK/Gadget/SCSI/Serial 接口 | [usb/agent.md](usb/agent.md) |
| `serial` | 串口 | 串口接口 | [serial/agent.md](serial/agent.md) |
| `udx` | UDX | UDX 接口 | [udx/agent.md](udx/agent.md) |
| `partitionslot` | 分区槽 | 分区槽接口 | [partitionslot/agent.md](partitionslot/agent.md) |

### 安全与认证

| 模块 | 中文名 | 说明 | Agent 指引 |
|------|--------|------|------------|
| `user_auth` | 用户认证 | 用户认证接口 | [user_auth/agent.md](user_auth/agent.md) |
| `pin_auth` | PIN 码认证 | PIN 码认证接口 | [pin_auth/agent.md](pin_auth/agent.md) |
| `face_auth` | 人脸认证 | 人脸认证接口 | [face_auth/agent.md](face_auth/agent.md) |
| `fingerprint_auth` | 指纹认证 | 指纹认证接口 | [fingerprint_auth/agent.md](fingerprint_auth/agent.md) |
| `huks` | 统一密钥管理 | HUKS 密钥服务接口 | [huks/agent.md](huks/agent.md) |

### 其他

| 模块 | 中文名 | 说明 | Agent 指引 |
|------|--------|------|------------|
| `low_power_player` | 低功耗播放器 | 低功耗播放器接口 | [low_power_player/agent.md](low_power_player/agent.md) |
| `tools` | 工具 | hc-gen 等工具 | [tools/agent.md](tools/agent.md) |

## 仓级关键文件

| 文件 | 说明 |
|------|------|
| `interface.gni` | `interface()` 模板封装，按系统类型（standard/small/mini）分发到 `hdi.gni`/`hdi_small.gni`/`hdi_mini.gni` |
| `README.md` / `README_ZH.md` | 仓级中英文说明（IDL 语法、用法、命名规则） |
| `OAT.xml` | 开源合规扫描规则 |
| `LICENSE` | Apache-2.0 许可证 |

## IDL 与版本约定（全仓通用）

- **文件命名**：IDL 文件大驼峰，接口文件以 `I` 开头（`IFoo.idl`）；类型文件
  `FooTypes.idl`；回调接口 `IFooCallback.idl`（`[callback]` 修饰）。
- **版本号**：`[major].[minor]`。major 不同=不兼容；minor 不同但 major 相同=
  兼容，**不得**修改已有接口的名称、参数类型/个数、返回值类型/个数。
- **初始版本**：`v1_0`；新增版本目录为 `v1_1`、`v2_0` 等。
- **package 名**：必须与 IDL 路径匹配，如 `ohos.hdi.<module>.v1_0`。
- **module_name**：控制驱动描述符 `struct HdfDriverEntry` 的 `moduleName`。

## 约束与边界

### 不可破坏（Do not）

- **不要**修改已发布版本目录（`vX_Y/`）内已有 `.idl` 接口的名称、参数、返回值
  ——这是已冻结的对外 API，破坏即破坏 ABI 兼容。如需变更，新增更高版本目录。
- **不要**在本仓实现服务逻辑——本仓只定义接口；实现写到 `drivers_peripheral`。
- **不要**改 `interface.gni` 的模板分发逻辑（standard/small/mini 三分支）。
- **不要**手改生成文件：IDL 编译产出的 `*_proxy.h`、`*_stub.h`、`ifoo_interface.h`
  等均在 `out/<product>/gen/` 下，生成器是唯一真源。
- **不要**让 `package` 名与 IDL 路径不匹配——编译模板据此定位。

### 改动前必须请示（Ask before）

- 新增一个外设模块目录（涉及 `subsystem_config.json` 注册与部件声明）。
- 新增一个 major 版本（`v2_0`）——属于不兼容变更，需评估全 OS 影响。
- 修改任一版本目录的 `BUILD.gn` `sources` 列表（影响生成代码范围）。
- 修改 `interface.gni`。

### 不变量（Invariants）

- **依赖方向**：`drivers_peripheral` import 本仓；本仓**不**反向 import 实现仓。
- **接口冻结**：已发布版本的 IDL 接口签名不可变；演进只能通过新增版本目录。
- **三系统适配**：`interface.gni` 必须同时支持 standard（IPC/直通）、
  small（passthrough）、mini（lite）三类系统。

## 验证闭环

### 最小检查（任何接口改动后必跑）

1. **IDL 语法**：通过编译模板生成，确认 `gn gen` 不报错。
2. **GN 生成**：`./build.sh --product-name rk3568 --build-only-gn`。
3. **相关模块编译**：
   `./build.sh --product-name rk3568 --build-target drivers_interface_<module>`。

### Done 定义

任务完成须满足：
1. 上述最小检查全部通过。
2. 未触碰「不可破坏」项；新增版本/接口已说明兼容性影响。
3. 未手改任何生成文件。
4. 若改了已发布版本接口签名——这属于破坏性变更，须停止并请示。

## 知识路由

| 任务 | 先读 |
|------|------|
| 新增一个外设 HDI 模块 | `README_ZH.md`（IDL 语法、用法）、对应模块子目录 `agent.md` |
| 新增接口版本 | 本文件「IDL 与版本约定」、对应模块 `agent.md` 的「接口版本」表 |
| IDL 语法/编译模板 | `README_ZH.md`、`interface.gni`、`//build/config/components/hdi/hdi.gni` |
| 实现一个 HDI 服务 | 本仓接口定义 + `drivers_peripheral/<module>/` 实现仓 |
| 三系统适配（standard/small/mini） | `interface.gni`、`//drivers/hdf_core/adapter/` |

## 约定

- 文档主要为中文（`README_ZH.md`）；英文版 `README.md`。
- 仓库托管于 Gitee（openharmony/drivers_interface）。
- 版权头：Huawei Device Co., Ltd.，Apache 2.0（多数文件）。
