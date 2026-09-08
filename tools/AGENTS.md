# AGENTS.md - OpenHarmony HDF 配置编译器工具（drivers_tools / hc-gen）

## 1. 代码地图

本仓库是 OpenHarmony HDF 配置编译器工具（`@ohos/drivers_tools`，版本 6.0），subsystem `hdf`，part `drivers_tools`。核心职责是提供 `hc-gen` 编译器工具，将 `.hcs`（HDF Configuration Source）文本文件编译为 `.hcb`（二进制）格式，支持反编译（`.hcb` → `.hcs`），以及从配置生成 C 头文件/源文件（`.hcs` → `.h.gen` / `.c.gen`）。**本仓是独立的构建工具，不是 HDI 接口**——没有 `.idl` 文件、没有 proxy/stub、不依赖其他 OpenHarmony 组件。最重要的架构边界是**hc-gen 通过 Makefile 构建（非标准 `ohos_shared_library`/`ohos_executable`），通过 `action()` 调用 make，变更影响整个 OpenHarmony 项目中所有 `.hcs` 文件的编译**。

### 非本项目维护的目录

以下内容属于其他团队或产物，不属于本项目维护范围，修改时请跳过：

- `hc-gen/build/`：Makefile 构建产物目录（`.o` 对象文件 + `hc-gen` 二进制，由 `make` 生成，不在源码树中）
- 全项目中的 `.hcs` 配置源文件：由各子系统团队维护，hc-gen 仅负责编译
- 全项目中的 `.hcb` 二进制配置：由 hc-gen 编译 `.hcs` 生成，不在源码树中

### 嵌套指引

本仓目前无嵌套 AGENTS.md / CLAUDE.md / rules / skills 文件。仓库结构单一（`hc-gen/` 一个工具目录），不建议新建嵌套指引；编译器内部模块（lexer/parser/ast/codegen）的细节在本文档「知识路由」中集中说明。

### 关键区域

- `hc-gen/src/`：编译器源码（27 个文件），C++14 实现，命名空间 `OHOS::Hardware`：
  - `main.cpp`：程序入口
  - `lexer.cpp` / `lexer.h` + `token.cpp` / `token.h`：词法分析器与 token 定义
  - `parser.cpp` / `parser.h`：语法分析器
  - `ast.cpp` / `ast.h`：抽象语法树
  - `bytecode_gen.cpp` / `bytecode_gen.h`：二进制字节码生成（`.hcs` → `.hcb`）
  - `text_gen.cpp` / `text_gen.h`：文本输出生成
  - `decompile.cpp` / `decompile.h` + `decompile_gen.cpp` / `decompile_gen.h`：反编译（`.hcb` → `.hcs`）
  - `macro_gen.cpp` / `macro_gen.h`：宏支持生成
  - `startup_cfg_gen.cpp` / `startup_cfg_gen.h`：启动配置生成
  - `file.cpp` / `file.h`：文件操作工具
  - `option.cpp` / `option.h`：命令行选项解析
  - `opcode.h`：字节码操作码定义
  - `types.h`：类型定义
  - `logger.h`：日志工具
  - `generator.h`：生成器基类
- `hc-gen/test/`：测试用例目录（37 个测试用例目录，编号 01-35 + Python 测试运行器）：
  - `hcgen_test.py`：Python 测试运行器（非 GTest），执行所有测试用例并对比 golden 输出
  - `update_case.py`：测试用例 golden 输出更新脚本
  - 测试用例命名规则：`NN_描述`，错误用例以 `_ei` 后缀结尾（如 `01_empty_file_ei`、`17_broken_node_ei`）
  - 测试覆盖：空文件、空根节点、数值/字符串终止符、数组、节点嵌套、节点拷贝、节点引用、include 插入/修改/删除、注释、模板、嵌套模板、大文件、错误处理等
  - 每个测试用例目录含 golden 输出文件（`.hcb`、`.c.gen`、`.h.gen`、`.d.hcs`）
- `hc-gen/BUILD.gn`：GN 构建配置：
  - `action("build_hc_gen")`：通过 `script = "/usr/bin/env"` 调用 `make`（ohos_lite 下使用 `//build/lite/run_shell_cmd.py` 作为 shell 包装器）
  - `copy("hc_gen")`：将构建产物 `hc-gen` 二进制复制到 `$target_out_dir/bin/hc-gen`
- `hc-gen/Makefile`：Make 构建脚本：
  - `CXX := c++`，`CXX_FLAGS := -std=gnu++14 -s`，`CXX_LD_FLAGS := -lstdc++`
  - `SRCS := $(wildcard src/*.cpp)` 自动收集所有源文件
  - `LANG` 环境变量控制平台（未设置时 `-DOS_MINGW`，设置时 `-DOS_UNIX`）
  - `test` 目标：`python test/hcgen_test.py $(TARGET)`
  - `update_testcase` 目标：`python test/update_case.py $(TARGET)`
- `hc-gen/CMakeLists.txt`：CMake 替代构建（用于 IDE 开发，如 CLion 的 `cmake-build-debug`）
- `bundle.json`：组件清单，`adapted_system_type: ["standard", "small", "mini"]`（**所有驱动仓中适配系统覆盖最广**），**无依赖组件**（`deps.components = []`），构建 `hc-gen:hc_gen`，导出 1 个 inner_kits（`hc_gen`）

### Where to look

| 任务类型 | 先看哪里 |
|---|---|
| 词法分析变更 | `hc-gen/src/lexer.cpp` + `lexer.h` + `token.cpp` + `token.h` + `opcode.h` |
| 语法分析变更 | `hc-gen/src/parser.cpp` + `parser.h` + `ast.cpp` + `ast.h` |
| 二进制编译（`.hcs`→`.hcb`）变更 | `hc-gen/src/bytecode_gen.cpp` + `bytecode_gen.h` + `opcode.h` |
| 文本输出变更 | `hc-gen/src/text_gen.cpp` + `text_gen.h` |
| 反编译（`.hcb`→`.hcs`）变更 | `hc-gen/src/decompile.cpp` + `decompile.h` + `decompile_gen.cpp` + `decompile_gen.h` |
| C 代码生成（`.hcs`→`.h.gen`/`.c.gen`）变更 | `hc-gen/src/text_gen.cpp` + `text_gen.h`（C 代码生成在 text_gen 中）|
| 宏支持变更 | `hc-gen/src/macro_gen.cpp` + `macro_gen.h` |
| 启动配置生成变更 | `hc-gen/src/startup_cfg_gen.cpp` + `startup_cfg_gen.h` |
| 命令行选项变更 | `hc-gen/src/option.cpp` + `option.h` + `main.cpp` |
| 文件 IO 变更 | `hc-gen/src/file.cpp` + `file.h` |
| 类型定义变更 | `hc-gen/src/types.h` + `opcode.h` |
| 构建配置变更 | `hc-gen/BUILD.gn`（`action()` + `copy()`）+ `hc-gen/Makefile`（CXX_FLAGS、目标规则）+ `bundle.json` |
| 新增测试用例 | `hc-gen/test/`（参考现有 37 个用例的命名和组织）+ `hcgen_test.py`（测试运行器）|
| 更新 golden 输出 | `hc-gen/test/update_case.py`（`make update_testcase` 调用）|
| lite 系统适配 | `hc-gen/BUILD.gn` 中 `if (defined(ohos_lite))` 分支（使用 `//build/lite/run_shell_cmd.py`）|
| 平台适配（MingW/Unix）| `hc-gen/Makefile` 中 `LANG` 环境变量控制 `-DOS_MINGW` / `-DOS_UNIX` |

### 架构分层

```
hc-gen 编译器工具（本仓维护，独立工具，无 OpenHarmony 组件依赖）
  ├─ src/ (27 个 C++ 源文件，命名空间 OHOS::Hardware)
  │    ├─ main.cpp        (入口)
  │    ├─ option.cpp/h    (命令行选项解析)
  │    ├─ lexer.cpp/h     (词法分析)
  │    ├─ token.cpp/h     (token 定义)
  │    ├─ parser.cpp/h    (语法分析)
  │    ├─ ast.cpp/h       (抽象语法树)
  │    ├─ bytecode_gen    (二进制编译：.hcs → .hcb)
  │    ├─ text_gen        (文本输出 + C 代码生成：.hcs → .h.gen/.c.gen)
  │    ├─ decompile       (反编译：.hcb → .hcs)
  │    ├─ macro_gen       (宏支持)
  │    ├─ startup_cfg_gen (启动配置生成)
  │    ├─ file.cpp/h      (文件 IO)
  │    ├─ opcode.h        (字节码操作码)
  │    ├─ types.h         (类型定义)
  │    └─ logger.h        (日志)
  │
  ├─ test/ (37 个测试用例 + Python 测试运行器)
  │    ├─ hcgen_test.py   (测试运行器，对比 golden 输出)
  │    ├─ update_case.py  (golden 输出更新)
  │    └─ 01_empty_file_ei/ ... 35_nested_template/ (测试用例目录)
  │
  ├─ BUILD.gn (GN 构建：action() 调用 make + copy() 二进制)
  ├─ Makefile (Make 构建：C++14，MingW/Unix 平台)
  └─ CMakeLists.txt (CMake 替代构建，用于 IDE)
       ↓
  hc-gen 二进制工具
       ↓
  编译全项目 .hcs 配置文件
       ↓
  .hcb (二进制) / .h.gen / .c.gen (C 代码) / .d.hcs (反编译)

适配系统：standard / small / mini（所有驱动仓中覆盖最广）
依赖：无（独立工具，deps.components = []）
```

## 2. 知识路由

在规划或编辑前，先对任务分类，读取对应的代码路径和文档。

### Task-based routing

| 任务类型 | 读取 |
|---|---|
| 词法分析变更 | `hc-gen/src/lexer.cpp` + `lexer.h` + `token.cpp` + `token.h` + `opcode.h` -> 跑 `make test` 验证所有测试用例 |
| 语法分析变更 | `hc-gen/src/parser.cpp` + `parser.h` + `ast.cpp` + `ast.h` -> 跑 `make test` 验证所有测试用例 |
| 二进制编译变更（`.hcs`→`.hcb`）| `hc-gen/src/bytecode_gen.cpp` + `bytecode_gen.h` + `opcode.h` -> 跑 `make test` 验证 `.hcb` golden 输出 + 全项目 `.hcs` 编译验证 |
| 反编译变更（`.hcb`→`.hcs`）| `hc-gen/src/decompile.cpp` + `decompile.h` + `decompile_gen.cpp` + `decompile_gen.h` -> 跑 `make test` 验证反编译 golden 输出 |
| C 代码生成变更（`.hcs`→`.h.gen`/`.c.gen`）| `hc-gen/src/text_gen.cpp` + `text_gen.h` -> 跑 `make test` 验证 `.h.gen`/`.c.gen` golden 输出 + 全项目 C 代码生成验证 |
| 宏支持变更 | `hc-gen/src/macro_gen.cpp` + `macro_gen.h` -> 跑 `make test` 验证模板/嵌套模板用例（`34_template`、`35_nested_template`）|
| 启动配置生成变更 | `hc-gen/src/startup_cfg_gen.cpp` + `startup_cfg_gen.h` |
| 命令行选项变更 | `hc-gen/src/option.cpp` + `option.h` + `main.cpp` -> 验证所有选项组合 |
| 构建配置变更 | `hc-gen/BUILD.gn`（`action()` + `copy()`）+ `hc-gen/Makefile`（CXX_FLAGS、目标规则）+ `bundle.json` |
| 新增测试用例 | `hc-gen/test/`（参考现有 37 个用例的命名 `NN_描述` + golden 输出文件）+ `hcgen_test.py`（确认测试运行器能识别新用例）|
| 更新 golden 输出 | `hc-gen/test/update_case.py`（`make update_testcase` 调用，用于 intentional 行为变更后更新预期输出）|
| lite 系统适配 | `hc-gen/BUILD.gn` 中 `if (defined(ohos_lite))` 分支（使用 `//build/lite/run_shell_cmd.py` 作为 shell 包装器）|
| 平台适配（MingW/Unix）| `hc-gen/Makefile` 中 `LANG` 环境变量控制 `-DOS_MINGW` / `-DOS_UNIX` |

### Path-based routing

| 修改路径 | 需了解的上下文 |
|---|---|
| `hc-gen/src/lexer.cpp` / `lexer.h` / `token.cpp` / `token.h` | 词法分析器，变更影响所有 `.hcs` 文件的 token 识别，需跑全部测试用例 |
| `hc-gen/src/parser.cpp` / `parser.h` | 语法分析器，变更影响 AST 构建，需跑全部测试用例 |
| `hc-gen/src/ast.cpp` / `ast.h` | AST 数据结构，变更影响所有代码生成器（bytecode_gen / text_gen / decompile / macro_gen / startup_cfg_gen）|
| `hc-gen/src/bytecode_gen.cpp` / `bytecode_gen.h` / `opcode.h` | 二进制编译核心，变更影响 `.hcb` 格式，**`.hcb` 格式变更会破坏全项目已编译的配置**，需评估向后兼容性 |
| `hc-gen/src/text_gen.cpp` / `text_gen.h` | 文本输出 + C 代码生成，变更影响 `.h.gen` / `.c.gen` 输出，影响所有依赖生成的 C 头文件/源文件的子系统 |
| `hc-gen/src/decompile.cpp` / `decompile.h` / `decompile_gen.cpp` / `decompile_gen.h` | 反编译逻辑，需与 `bytecode_gen` 保持格式对称 |
| `hc-gen/src/macro_gen.cpp` / `macro_gen.h` | 宏支持，影响模板和嵌套模板的处理 |
| `hc-gen/src/startup_cfg_gen.cpp` / `startup_cfg_gen.h` | 启动配置生成，影响 HDF 启动流程 |
| `hc-gen/src/option.cpp` / `option.h` / `main.cpp` | 命令行接口，变更影响所有调用 hc-gen 的构建脚本 |
| `hc-gen/src/opcode.h` | **字节码操作码定义**，变更影响 `.hcb` 二进制格式，是 ABI 契约 |
| `hc-gen/src/types.h` | 类型定义，变更可能影响所有模块 |
| `hc-gen/BUILD.gn` | GN 构建配置，`action()` 调用 make + `copy()` 复制二进制，ohos_lite 下使用 `run_shell_cmd.py` |
| `hc-gen/Makefile` | Make 构建，`CXX_FLAGS := -std=gnu++14 -s`，`LANG` 控制平台宏，`test`/`update_testcase` 目标 |
| `hc-gen/CMakeLists.txt` | CMake 替代构建，用于 IDE 开发，变更不影响正式构建 |
| `hc-gen/test/` | 测试用例，golden 输出文件是预期行为契约，**不要为通过测试而修改 golden 输出**（除非行为变更是有意的）|
| `bundle.json` | 组件清单，`adapted_system_type: ["standard", "small", "mini"]`，无依赖组件 |

### Vocabulary-based routing

当任务、issue、日志、API 名称中出现以下术语时，先理解其含义和风险再动手：

| 术语 | 含义与风险 | 读取 |
|---|---|---|
| hc-gen | HDF 配置编译器工具，本仓的核心产物 | 本文件 + `hc-gen/src/main.cpp` |
| .hcs | HDF Configuration Source，HDF 配置源文件（文本格式），hc-gen 的输入 | 全项目 `.hcs` 文件 |
| .hcb | HDF Configuration Binary，HDF 配置二进制文件，hc-gen 编译 `.hcs` 的输出，**格式变更是破坏性变更** | `hc-gen/src/bytecode_gen.cpp` + `opcode.h` |
| .h.gen / .c.gen | hc-gen 从 `.hcs` 生成的 C 头文件/源文件 | `hc-gen/src/text_gen.cpp` |
| .d.hcs | 反编译输出（`.hcb` → `.hcs`）| `hc-gen/src/decompile.cpp` + `decompile_gen.cpp` |
| OHOS::Hardware | hc-gen 的 C++ 命名空间 | `hc-gen/src/` 所有源文件 |
| opcode | 字节码操作码定义，`.hcb` 格式的核心契约 | `hc-gen/src/opcode.h` |
| AST | 抽象语法树，parser 构建、各 codegen 消费 | `hc-gen/src/ast.cpp` + `ast.h` |
| lexer / token | 词法分析器 / token 定义 | `hc-gen/src/lexer.cpp` + `token.cpp` |
| parser | 语法分析器 | `hc-gen/src/parser.cpp` |
| bytecode_gen | 二进制字节码生成器（`.hcs` → `.hcb`）| `hc-gen/src/bytecode_gen.cpp` |
| text_gen | 文本输出 + C 代码生成器 | `hc-gen/src/text_gen.cpp` |
| decompile | 反编译器（`.hcb` → `.hcs`）| `hc-gen/src/decompile.cpp` |
| macro_gen | 宏支持生成器 | `hc-gen/src/macro_gen.cpp` |
| startup_cfg_gen | 启动配置生成器 | `hc-gen/src/startup_cfg_gen.cpp` |
| golden 输出 | 测试用例的预期输出文件（`.hcb`、`.c.gen`、`.h.gen`、`.d.hcs`），是行为契约 | `hc-gen/test/` 各用例目录 |
| _ei 后缀 | 错误注入（error injection）测试用例后缀，如 `01_empty_file_ei`、`17_broken_node_ei` | `hc-gen/test/` |
| hcgen_test.py | Python 测试运行器（非 GTest），对比实际输出与 golden 输出 | `hc-gen/test/hcgen_test.py` |
| update_case.py | golden 输出更新脚本，用于 intentional 行为变更后更新预期输出 | `hc-gen/test/update_case.py` |
| run_shell_cmd.py | ohos_lite 下替代 `/usr/bin/env` 的 shell 包装器 | `//build/lite/run_shell_cmd.py` |
| OS_MINGW / OS_UNIX | Makefile 中由 `LANG` 环境变量控制的平台宏 | `hc-gen/Makefile` |
| action() | GN 动作模板，本仓用 `action("build_hc_gen")` 调用 make | `hc-gen/BUILD.gn` |
| copy() | GN 复制模板，本仓用 `copy("hc_gen")` 复制二进制 | `hc-gen/BUILD.gn` |
| drivers_tools | 本仓的 part 名称 | `bundle.json` |

在计划阶段，必须声明：
- **任务分类**（如：词法/语法分析变更 / 二进制编译变更 / 反编译变更 / C 代码生成变更 / 构建配置变更 / 测试用例变更）
- **已读取的代码路径和文档**（具体到文件路径）
- **发现的约束**（架构不变量、禁止事项、`.hcb` 格式兼容性、全项目影响范围）
- **是否需要同步修改其他层**：
  - `opcode.h` / `bytecode_gen` 变更 -> 评估 `.hcb` 格式向后兼容性 + 全项目 `.hcs` 重新编译
  - `text_gen` 变更 -> 评估 `.h.gen`/`.c.gen` 输出对依赖子系统的影压
  - `decompile` 变更 -> 与 `bytecode_gen` 保持格式对称
  - golden 输出变更 -> 确认行为变更是有意的，使用 `update_case.py` 更新
  - `BUILD.gn` / `Makefile` 变更 -> 评估 lite 系统和平台适配影响

## 3. 约束边界

### 架构不变量

- **hc-gen 是独立构建工具**：不依赖任何 OpenHarmony 组件（`deps.components = []`），通过 Makefile 构建，非标准 `ohos_shared_library`/`ohos_executable`
- **`.hcs` 文件是配置源文件，`.hcb` 是编译产物**：hc-gen 负责 `.hcs` → `.hcb` 编译、`.hcb` → `.hcs` 反编译、`.hcs` → `.h.gen`/`.c.gen` C 代码生成
- **`.hcb` 二进制格式是全项目契约**：`opcode.h` 定义的操作码和 `bytecode_gen` 的输出格式一旦发布即固定，变更会破坏全项目已编译的配置
- **编译器内部模块分层清晰**：lexer → parser → AST → codegen（bytecode_gen / text_gen / decompile / macro_gen / startup_cfg_gen），各 codegen 消费同一 AST
- **测试用例的 golden 输出是行为契约**：37 个测试用例的 golden 输出文件定义了 hc-gen 的预期行为，变更需通过 `update_case.py` 并确认行为变更是有意的
- **适配系统覆盖最广**：`adapted_system_type: ["standard", "small", "mini"]`，是所有驱动仓中适配系统覆盖最广的

### 禁止事项

- **不要修改 `opcode.h` 中的已有操作码值**：操作码是 `.hcb` 二进制格式的契约，调整已有值会破坏全项目已编译的配置
- **不要修改 `bytecode_gen` 的输出格式而不评估向后兼容性**：`.hcb` 格式变更会破坏全项目配置
- **不要为通过测试而修改 golden 输出文件**：golden 输出是预期行为契约，行为变更必须是有意的，使用 `update_case.py` 更新
- **不要将 hc-gen 改为标准 `ohos_shared_library`/`ohos_executable` 构建**：本仓通过 Makefile 构建，GN 的 `action()` 调用 make，改为标准模板会破坏独立工具的构建方式
- **不要引入对其他 OpenHarmony 组件的依赖**：hc-gen 是独立工具（`deps.components = []`），引入依赖会破坏独立性
- **不要在 `decompile` 中引入与 `bytecode_gen` 不对称的格式处理**：反编译必须与编译格式对称
- **不要跳过测试验证**：任何源码变更后必须跑 `make test` 验证 37 个测试用例
- **不要删除测试用例的 golden 输出文件**：golden 输出（`.hcb`、`.c.gen`、`.h.gen`、`.d.hcs`）是行为契约

### Ask before 规则

在执行以下操作前，必须先确认或询问：
- **修改 opcode.h 操作码值**：确认 .hcb 格式向后兼容性
- **修改 bytecode_gen 输出格式**：确认全项目 .hcs 配置不受破坏
- **修改 golden 输出文件**：确认行为变更是有意的，使用 update_case.py
- **修改 BUILD.gn 的 action() 或 copy()**：确认 lite 系统下 run_shell_cmd.py 兼容性
- **引入新依赖**：确认不破坏工具独立性（deps.components = []）

### 需确认后再修改

- **`opcode.h` 操作码变更**（需评估 `.hcb` 格式向后兼容性，确认全项目配置重新编译的影响，新增操作码追加而非调整已有值）
- **`bytecode_gen` 输出格式变更**（需评估全项目已编译配置的兼容性，可能需要版本化 `.hcb` 格式）
- **`text_gen` C 代码生成变更**（需评估 `.h.gen`/`.c.gen` 输出对所有依赖子系统的影响）
- **`decompile` 反编译逻辑变更**（需与 `bytecode_gen` 保持格式对称，确认反编译 golden 输出一致）
- **golden 输出文件批量更新**（需确认行为变更是有意的，使用 `update_case.py`，评估对全项目的影响）
- **`Makefile` 中 `CXX_FLAGS` 或平台宏（`OS_MINGW`/`OS_UNIX`）变更**（需评估跨平台兼容性）
- **`BUILD.gn` 中 `action()` 或 `copy()` 逻辑变更**（需评估 lite 系统下 `run_shell_cmd.py` 的兼容性）
- **新增外部依赖**（需确认是否破坏工具独立性，`deps.components` 当前为空）
- **`adapted_system_type` 变更**（当前 `["standard", "small", "mini"]`，移除任一系统需评估下游影响）
- **测试运行器 `hcgen_test.py` 逻辑变更**（需确认所有 37 个测试用例仍被正确执行）

### 项目特定陷阱

- **`.hcb` 格式的全项目影响**：hc-gen 编译全项目的 `.hcs` 配置文件，`opcode.h` 或 `bytecode_gen` 的变更会破坏全项目已编译的配置，必须评估向后兼容性
- **`text_gen` 的双重职责**：`text_gen.cpp` 同时负责文本输出和 C 代码生成（`.h.gen`/`.c.gen`），修改时需同时评估两条输出路径
- **`decompile` 与 `bytecode_gen` 的对称性**：反编译必须与编译格式对称，修改一方时需同步另一方
- **测试用例的 `_ei` 后缀**：以 `_ei` 结尾的测试用例是错误注入用例（如 `01_empty_file_ei`、`17_broken_node_ei`），验证 hc-gen 对错误输入的处理，golden 输出可能是错误码而非正常输出
- **golden 输出的平台差异**：golden 输出文件可能因平台（MingW/Unix）不同而有行尾差异（`\r\n` vs `\n`），`hcgen_test.py` 中的 `text_file_compare` 会做 `\r\n` → `\n` 归一化
- **`Makefile` 的 `LANG` 环境变量**：未设置 `LANG` 时使用 `-DOS_MINGW`，设置时使用 `-DOS_UNIX`，跨平台构建时需注意
- **`BUILD.gn` 的 `action()` 脚本差异**：standard 系统使用 `/usr/bin/env`，ohos_lite 系统使用 `//build/lite/run_shell_cmd.py` 作为 shell 包装器
- **`CMakeLists.txt` 不是正式构建**：CMake 构建仅用于 IDE 开发（如 CLion 的 `cmake-build-debug`），正式构建使用 Makefile，修改 CMakeLists.txt 不影响正式构建
- **`bundle.json` 的无依赖特性**：`deps.components = []` 是工具独立性的体现，引入依赖需谨慎评估
- **`adapted_system_type` 含 `mini`**：本仓适配 `standard`/`small`/`mini` 三个系统，是所有驱动仓中覆盖最广的，与 `drivers_interface_motion`（不含 mini）和 `drivers_interface_vibrator`（不含 mini）不同
- **`make test` 依赖 `python`**：测试运行器 `hcgen_test.py` 使用 `python` 命令，需确认构建环境中 Python 可用
- **`update_testcase` 的谨慎使用**：`make update_testcase` 会更新所有 golden 输出，仅在 intentional 行为变更后使用，不要用于「修复」测试失败

### Agent 失败模式

- **修改操作码破坏 `.hcb` 格式**：Agent 可能修改 `opcode.h` 中的操作码值以适配新需求，但这会直接破坏 `.hcb` 二进制格式，导致全项目已编译配置失效——这是最危险的失败模式
- **修改 golden 输出通过测试**：Agent 可能修改 golden 输出文件以使失败测试通过，而非修复源码逻辑——golden 输出是行为契约，必须通过 `update_case.py` 更新
- **修改 AST 影响所有 codegen**：Agent 可能修改 `ast.cpp`/`ast.h` 时未意识到会影响所有代码生成器（bytecode_gen/text_gen/decompile/macro_gen/startup_cfg_gen），导致隐蔽的回归
- **忽略 `.hcb` 格式向后兼容性**：Agent 可能修改 `bytecode_gen` 输出格式时未评估全项目影响，`.hcb` 格式变更会破坏所有已编译配置
- **修改 `text_gen` 遗漏 C 代码生成**：Agent 可能只关注文本输出路径而忽略 C 代码生成路径（`text_gen` 同时负责两者），导致 `.h.gen`/`.c.gen` 输出不一致
- **修改 `BUILD.gn` 破坏 lite 适配**：Agent 可能修改 `action()` 或 `copy()` 逻辑时未考虑 ohos_lite 下 `run_shell_cmd.py` 的兼容性
- **跳过测试验证**：Agent 可能在源码变更后未运行 `make test` 验证 37 个测试用例，直接假设变更正确
- **引入外部依赖破坏独立性**：Agent 可能添加对其他 OpenHarmony 组件的依赖，破坏 `deps.components = []` 的工具独立性承诺

## 4. 验证闭环

### 最小验证

```bash
# 构建 drivers_tools / hc-gen（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target drivers_tools

# 单独构建 hc-gen
./build.sh --product-name rk3568 --build-target //drivers/interface/tools/hc-gen:hc_gen

# 静态分析：检查 C++ 代码风格
cpplint hc-gen/src/*.cpp hc-gen/src/*.h 2>/dev/null || true
# 或使用 clang-format 检查格式
# clang-format --dry-run --Werror hc-gen/src/*.cpp hc-gen/src/*.h
```

### 任务特定验证

| 任务类型 | 验证命令 |
|---|---|
| 词法/语法分析变更 | `make test -C drivers/interface/tools/hc-gen`（跑全部 37 个测试用例）+ 全项目 `.hcs` 编译验证 |
| 二进制编译变更（`.hcs`→`.hcb`）| `make test`（验证 `.hcb` golden 输出）+ 评估 `.hcb` 格式向后兼容性 + 全项目配置重新编译 |
| 反编译变更（`.hcb`→`.hcs`）| `make test`（验证 `.d.hcs` golden 输出）+ 验证与 `bytecode_gen` 格式对称 |
| C 代码生成变更（`.hcs`→`.h.gen`/`.c.gen`）| `make test`（验证 `.h.gen`/`.c.gen` golden 输出）+ 评估对依赖子系统的影压 |
| 宏支持变更 | `make test`（验证 `34_template`、`35_nested_template` 用例）|
| 构建配置变更 | `./build.sh --product-name rk3568 --build-target drivers_tools` + 验证 lite 系统下 `run_shell_cmd.py` 兼容性 |
| 新增测试用例 | `make test`（确认新用例被识别和执行）+ 确认 golden 输出文件完整 |
| golden 输出更新 | `make update_testcase`（`update_case.py` 更新 golden 输出）+ 确认行为变更是有意的 + 评估全项目影响 |
| 平台适配变更 | 跨平台验证（MingW/Unix）+ 验证 `LANG` 环境变量控制逻辑 |
| lite 系统适配 | 在 lite 系统配置下构建 + 验证 `run_shell_cmd.py` shell 包装器 |
| `opcode.h` 变更 | `make test` + 评估 `.hcb` 格式向后兼容性 + 全项目 `.hcs` 重新编译验证 |

### Done 定义

- 构建通过（`drivers_tools` 子系统 + `hc-gen` 二进制）
- `make test` 全部 37 个测试用例通过（无 golden 输出不一致）
- `cpplint` 或 `clang-format --dry-run` 静态分析通过，无新增警告
- 无新增编译警告（`CXX_FLAGS := -std=gnu++14 -s`）
- 变更范围与任务要求一致，未夹带未关联的重构
- `.hcb` 格式变更已评估向后兼容性，确认全项目配置不受破坏
- `text_gen` 变更已评估 `.h.gen`/`.c.gen` 输出对依赖子系统的影压
- `decompile` 变更已验证与 `bytecode_gen` 格式对称
- golden 输出变更已确认行为变更是有意的，通过 `update_case.py` 更新
- `BUILD.gn` / `Makefile` 变更已评估 lite 系统和平台适配影响
- 未引入对其他 OpenHarmony 组件的依赖（保持 `deps.components = []`）

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果（特别是 `make test` 的 37 个用例结果）
3. 是否影响 `.hcb` 二进制格式（`opcode.h` / `bytecode_gen` 变更需评估全项目影响）
4. 是否影响 C 代码生成输出（`text_gen` 变更需评估 `.h.gen`/`.c.gen` 对依赖子系统的影压）
5. 是否更新了 golden 输出文件（需说明行为变更的理由）
6. 是否触及架构不变量或需确认事项
7. 是否保持了工具独立性（`deps.components = []`）

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样，不能假称已通过。涉及 `.hcb` 格式变更的，必须人工复核 `opcode.h` 和 `bytecode_gen` 的向后兼容性并说明无法在沙箱全项目验证的限制。涉及 golden 输出更新的，必须人工复核行为变更的合理性。涉及构建配置的变更，必须人工复核 `cpplint` 或 `clang-format` 静态分析结果。
