# AGENTS.md — drivers/interface/partitionslot

面向在 OpenHarmony **drivers_interface_partitionslot** 部件（PartitionSlot
服务的 HDI 接口定义——A/B 多槽位启动分区管理）工作的编码 Agent 的指南。在修改
`drivers/interface/partitionslot/` 下任何文件之前，请先阅读本文档。

> 作用范围：部件根目录（`drivers/interface/partitionslot`），位于
> `drivers/interface` git 仓库内。服务实现在独立仓库：
> `drivers/peripheral/partitionslot`（有其自己的 AGENTS.md）。英文版
> `AGENTS.md` 与本中文版内容等价；若有分歧，以本中文版为准。

## 1. 代码结构

### 1.1 本部件负责的内容

PartitionSlot HDI 接口定义——四个 API，@since 3.1：
- `GetCurrentSlot`——查询单板当前从哪个分区槽位启动、共支持几个槽位；
- `GetSlotSuffix`——槽位序号 → 分区名后缀；
- `SetActiveSlot`——切换设备下次启动使用的槽位；
- `SetSlotUnbootable`——将某槽位置为不可启动。

只有接口定义——共三个文件：`bundle.json`、`v1_0/BUILD.gn`、
`v1_0/IPartitionSlot.idl`，本目录无实现代码。`hdi()` 构建模板将 IDL 编译为
proxy/stub C++ 代码，并对外暴露三个 `inner_kits` 目标供下游链接：
`libpartitionslot_proxy_1.0`（客户端）、`libpartitionslot_stub_1.0`（服务端）、
`partitionslot_idl_headers`。典型消费方：`drivers/peripheral/partitionslot`
的服务实现、真机测试 `partitionslot_hdi_test`、`base/startup/init` 的
`begetctl partitionslot` 工具。

### 1.2 构建期生成链（依赖方向：上层生成下层；绝不手改下层）

```
v1_0/IPartitionSlot.idl            包声明 ohos.hdi.partitionslot.v1_0（唯一的 API 文件）
  → hdi() 模板                     //build/config/components/hdi/hdi.gni（代码生成与版本规则）
  → out/<product>/gen/drivers/interface/partitionslot/v1_0/   生成的 proxy/stub C++（只读）
  → libpartitionslot_proxy_1.0 / libpartitionslot_stub_1.0 / partitionslot_idl_headers   （inner_kits）
  → 消费方                         drivers/peripheral/partitionslot（服务实现）、
                                   partitionslot_hdi_test、begetctl partitionslot（startup_init）
```

| 路径 | 产物 | 职责 |
| --- | --- | --- |
| `v1_0/IPartitionSlot.idl` | —（一切之源） | 唯一的 API 文件；各方法的文档注释定义槽位语义与返回值契约 |
| `v1_0/BUILD.gn` | `partitionslot_idl_target`（及生成库） | `hdi()` 模板接线；`install_images` 在此声明 |
| `bundle.json` | — | `sub_component` + `inner_kits` 注册——对下游暴露的面 |
| `out/<product>/gen/drivers/interface/partitionslot/v1_0/` | 生成的 C++ | 精确 C++ 签名与 IPC 布局的唯一事实源——只读，重新构建以再生成 |
| *（跨仓）* `drivers/peripheral/partitionslot` | 服务实现 | 实现本接口；其 AGENTS.md 记录 bootctrl 布局与槽位编号怪癖 |
| *（跨仓）* `drivers/peripheral/partitionslot/test/unittest/hdf_partitionslot_test.cpp` | `partitionslot_hdi_test` | 消费方测试；链接 `libpartitionslot_proxy_1.0` |

高风险 / 频繁变更路径（此处改动需格外谨慎）：
`v1_0/IPartitionSlot.idl`（公开 HDI API）、`bundle.json`（`inner_kits`
是对下游暴露的面）。

嵌套指导文件：本仓库内无。实现侧有独立的
`drivers/peripheral/partitionslot/AGENTS.md`。

## 2. 知识路由——编辑前先读

在修改任何文件之前，先向自己说明：(1) 任务类别；(2) 已加载 §2.1 中的哪一行
（凡涉及 API 语义，还需加上 IDL 那一行）；(3) 适用 §3 的哪条约束。三者答不全，
先停下阅读。

### 2.1 按任务路由

| 任务 | 先读 | 需加载的关键概念 |
| --- | --- | --- |
| 新增或扩展 HDI API | `v1_0/IPartitionSlot.idl` + `//build/config/components/hdi/hdi.gni` + `drivers/interface/connected_nfc_tag/v1_1/` | `extends` 语法；v1_1 次版本流程（绝不向已发布的 v1_0 追加） |
| 构建接线、代码生成 | `v1_0/BUILD.gn` + `hdi.gni` 中的 `hdi()` 模板 | 模板生成什么；`install_images` 影响发布面 |
| 部件注册、对外库 | `bundle.json` + `drivers/interface/connected_nfc_tag/bundle.json` | `sub_component` + `inner_kits` 多版本注册 |
| 核对生成的 C++ 签名 / IPC 布局 | `out/<product>/gen/drivers/interface/partitionslot/v1_0/` | 生成代码是唯一事实源——不要猜布局 |
| 实现 / HAL 行为 | `drivers/peripheral/partitionslot` 及其 AGENTS.md | bootctrl 磁盘布局；槽位编号把 2 当作 B 槽 |
| 测试用例 / 预期 | `drivers/peripheral/partitionslot/test/unittest/hdf_partitionslot_test.cpp` | 每个 API 一个用例；需真机（HDF + bootctrl） |
| 下游消费者影响 | `bundle.json` 的 `inner_kits` + 全树 grep `drivers_interface_partitionslot` | 谁在链接 proxy/stub 库 |

### 2.2 按术语路由（任务/日志/issue/API 中出现下列术语时）

| 术语 / 缩写 | 概念 |
| --- | --- |
| slot、A/B 分区、active slot、unbootable、slot suffix（槽位后缀） | 各方法的定义见 `IPartitionSlot.idl`；返回值契约见 §3.1 |
| HDI、proxy、stub、`inner_kits`、`hdi()` | HDF IPC 机制；语义由 `hdi.gni` 中的 `hdi()` 模板及 `out/<product>/gen/` 下生成代码定义 |
| v1_0 / v1_1 / 大小版本规则 | 版本规则在 `hdi.gni`；`connected_nfc_tag/v1_1` 是现成的扩展范例 |
| bootctrl、`bootslots`、槽位编号 | 实现侧术语——见 `drivers/peripheral/partitionslot/AGENTS.md` |

## 3. 约束与边界（未经上报不得突破）

### 3.1 架构不变量（硬规则）

1. **包声明必须与目录路径一致。** `ohos.hdi.partitionslot.v1_0` ↔
   `drivers/interface/partitionslot/v1_0/`——不一致会破坏代码生成或生成到
   错误路径。
2. **已发布的 v1_0 已冻结。** 绝不修改已有方法的签名、参数方向标签或返回值
   语义——同大版本必须保持源码级和 IPC 级兼容。扩展一律走 `v1_1/` 次版本
   的 `extends`（见 §3.4）。
3. **返回值契约。** `0` = 成功，负数 = 失败；`-1` 特指单板不支持 A/B 分区。
   `GetCurrentSlot` 的返回值**就是**当前槽位号（`-1` 无 A/B、`0` 槽位 A、
   `1` 槽位 B、`N` 槽位 N）。
4. **每个参数必须带方向标签**：`[in]`、`[out]` 或 `[in, out]`。
5. **命名**：方法大驼峰（UpperCamelCase），参数小驼峰（lowerCamelCase），
   常量/枚举全大写（ALL_CAPS）。

### 3.2 禁止事项

- 绝不编辑 `out/<product>/gen/` 下的生成代码——应通过重新构建再生成；
  手改的"修复"下次构建即丢失。
- 绝不修改 `v1_0` 已有方法的签名、方向标签或返回值语义（§3.1.2）。
- 绝不向已发布的 `v1_0` 添加方法——改为创建 `v1_1/` 次版本。
- 绝不改变返回值含义，包括 `-1` 特指单板不支持 A/B 分区这一特殊语义
  （§3.1.3）。
- 不引入 `bundle.json` 未声明的第三方依赖。
- 不删除 Apache 2.0 许可证头；新文件必须携带（范例见
  `IPartitionSlot.idl` 文件头）。

### 3.3 需先确认（Ask before）

- 对 `v1_0` 文件做注释或拼写修正之外的任何修改。
- 对 `bundle.json` 的 `sub_component` 或 `inner_kits` 的任何修改——那是
  下游部件链接的内容。
- 引入新的大版本（`v2_0/`）或删除/废弃某个 API。
- 修改 `v1_0/BUILD.gn` 中的 `install_images`（影响库随哪些分区镜像发布）。
- `SetActiveSlot` / `SetSlotUnbootable` 的任何语义变更——它们改变设备从
  哪个分区启动，需维护者拍板。

### 3.4 本地约定

- 次版本扩展语法：
  `interface IPartitionSlot extends ohos.hdi.partitionslot.v1_0.IPartitionSlot { ... }`，
  随后在 `bundle.json` 注册新目标（`sub_component` + `inner_kits`），以
  `drivers/interface/connected_nfc_tag/v1_1` 为参考。只建 `v1_1/` 目录而不
  更新 `bundle.json`，会静默产生一个什么都不构建的目标。
- IDL 文档注释遵循 `IPartitionSlot.idl` 现有风格：每个方法上方
  `@brief` / `@param` / `@return` / `@since` 注释块。
- 本指导为中英双语对：`AGENTS.md`（英文）与 `AGENTS_zh.md`（中文）内容
  等价；编辑时保持两版同步。

### 3.5 本处常见的 Agent 失败模式

- 在本目录运行 `build.sh`——只能在 OHOS 源码根目录（含 `build.sh` 的祖先
  目录）运行。
- 只建 `v1_1/` 不更新 `bundle.json`——静默死目标。
- 通过编辑 `out/` 生成文件"修复"签名——下次构建即丢失。
- 期望本仓库描述运行时行为——实现语义（bootctrl 布局、槽位编号怪癖）在
  `drivers/peripheral/partitionslot/AGENTS.md`。

## 4. 验证闭环

### 4.1 构建部件（在 OHOS 源码根目录运行，不是本目录）

```bash
./build.sh --product-name rk3568 --build-target partitionslot_idl_target   # IDL + 生成的 proxy/stub
```

### 4.2 构建下游消费方测试

```bash
./build.sh --product-name rk3568 --build-target partitionslot_hdi_test
# 二进制：out/rk3568/tests/unittest/drivers_peripheral_partitionslot/drivers_peripheral_partition_slot/partitionslot_hdi_test
```

该测试链接 `libpartitionslot_proxy_1.0`——构建通过即证明接口仍可端到端
编译、链接。运行二进制需要真机（HDF 环境 + bootctrl）；无设备时诚实的上限
就是构建检查——不得把构建通过当作运行验证。

### 4.3 宣布完成前的最低检查

- [ ] `--build-target partitionslot_idl_target` 构建通过。
- [ ] 接口变更：`--build-target partitionslot_hdi_test` 也构建通过。
- [ ] `git diff` 未修改已发布的 `v1_0` 签名（仅注释/拼写修正），未改
      `bundle.json` 的 `inner_kits` / `install_images`（除非已获明确批准，
      见 §3.3）。
- [ ] 任何 IDL 变更之后：读过 `out/<product>/gen/drivers/interface/partitionslot/v1_0/`
      下重新生成的代码，确认 diff 只含预期变更。
- [ ] 新文件带 Apache 2.0 头；新建 `v1_1/` 已在 `bundle.json` 注册。

### 4.4 完成判据与最终回复

报告：(1) 改了哪些文件，标注关键改动的 `path:line`；(2) 适用了 §3 的哪条
约束、如何遵守；(3) 实际运行的构建/测试命令及通过/失败结果。若无法运行
构建/测试（无工具链或设备），需明确说明，将工作标记为 `NOT VERIFIED`，并列
出应由人工执行的命令——不得在没有证据的情况下宣称成功。
