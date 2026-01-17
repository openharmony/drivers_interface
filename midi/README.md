# OpenHarmony MIDI HDI 南向适配指南

## —— MIDI 1.0 字节流与 UMP 格式转换规范

### 1. 概述

在 OpenHarmony 的 MIDI 架构中，HDI 接口层统一采用 **UMP (Universal MIDI Packet)** 格式进行数据传输。为了兼容存量的 MIDI 1.0 硬件生态，南向 HAL 层需在驱动边界处完成协议转换。

本指南规范了 HAL 层在 **上报（字节流 -> UMP）** 和 **下发（UMP -> 字节流）** 两个方向上的转换逻辑。

### 2. 核心映射关系表

| 消息类别 | 典型消息 | 字节流特征 (MIDI 1.0) | UMP 封装特征 (MIDI 2.0) | 字长 |
| --- | --- | --- | --- | --- |
| **短消息** (Channel Voice) | Note On/Off, Control Change | 1-3 字节(如 `90 3C 64`) | **Message Type 2**(MIDI 1.0 Channel Voice) | 32-bit(1 Word) |
| **长消息** (SysEx) | System Exclusive | F0 开头，F7 结尾(不定长流) | **Message Type 3**<br>(Data Message 64-bit) | 64-bit(2 Words) |

---

### 3. 上行处理：字节流封装为 UMP (Bytes to UMP)

**场景**：HAL 从 ALSA 驱动读取到 `uint8_t` 字节流，需封装为 `uint32_t` 数组上报。

#### 3.1 短消息处理 (Channel Voice -> Type 2)

针对 1~3 字节的常规 MIDI 消息。

* **输入**：字节流 `Status`, `Data1`, `Data2`
* **输出**：单个 32-bit UMP Word
* **结构图解**：
```text
[31-28] [27-24] [23-20] [19-16] | [15-08] [07-00]
  MT=2   Group   Status  Chan   |  Data1   Data2

```


* **代码逻辑**：
```cpp
// 假设读取到 0x90, 0x3C, 0x64
uint32_t ump = 0;
ump |= (0x2 << 28);                  // MT = 0x2
ump |= ((bytes[0] >> 4) << 20);      // Status (High nibble)
ump |= ((bytes[0] & 0xF) << 16);     // Channel (Low nibble)
ump |= (bytes[1] << 8);              // Data1
ump |= (bytes[2] << 0);              // Data2

```



#### 3.2 长消息处理 (SysEx -> Type 3)

针对以 `F0` 开头、`F7` 结尾的不定长消息，需切片为多个 64-bit UMP 包。

* **输入**：不定长字节流
* **输出**：UMP Type 3 包序列 (每个包含 2 个 Word)
* **状态控制 (Status)**：
* `0x0` (Complete): 单包能装完 (<= 6 字节)
* `0x1` (Start): 起始包
* `0x2` (Continue): 中间包
* `0x3` (End): 结束包


* **结构图解**：
```text
Word 1: [MT=3][Grp][Status][Count] [Data1][Data2]
Word 2: [Data3][Data4][Data5][Data6]

```


* **示例逻辑**：
* 收到 `F0 01 02 03 04 05 06 07 F7` (共9字节)
* **包 1 (Start)**: 填入前6字节 `F0`~`05`。设置 Status=`0x1`, Count=`6`。
* **包 2 (End)**: 填入剩3字节 `06 07 F7`。设置 Status=`0x3`, Count=`3`，其余位补零。



---

### 4. 下行处理：UMP 解包为字节流 (UMP to Bytes)

**场景**：HAL 收到 Service 下发的 `uint32_t` UMP 数组，需还原为 `uint8_t` 字节流写入驱动。

#### 4.1 短消息处理 (Type 2 -> Channel Voice)

解析 32-bit UMP Word，还原为 1~3 字节。

* **输入**：单个 32-bit UMP Word (MT=2)
* **输出**：字节流 `StatusByte`, `Data1`, `Data2`
* **解析逻辑**：
1. **校验**：检查 `(ump >> 28) == 0x2`。
2. **还原 Status Byte**：`((ump >> 20) & 0xF) << 4 | ((ump >> 16) & 0xF)`。
3. **还原 Data Bytes**：提取 `[15-08]` 和 `[07-00]`。


* **代码逻辑**：
```cpp
std::vector<uint8_t> buffer;
uint8_t status = ((ump >> 20) & 0xF); // e.g., 0x9 (NoteOn)
uint8_t chan   = ((ump >> 16) & 0xF); // e.g., 0x0 (Ch 1)

buffer.push_back((status << 4) | chan); // 0x90
buffer.push_back((ump >> 8) & 0x7F);    // Note
buffer.push_back(ump & 0x7F);           // Velocity

```



#### 4.2 长消息处理 (Type 3 -> SysEx)

解析 64-bit UMP 包序列，提取有效载荷拼接成流。

* **输入**：UMP Type 3 包序列 (MT=3)
* **输出**：连续字节流
* **解析逻辑**：
1. **校验**：检查 `(word1 >> 28) == 0x3`。
2. **获取长度**：读取 `Count` 字段 `(word1 >> 16) & 0xF`。
3. **提取载荷**：根据 `Count` 值，依次从 Word1 的低16位和 Word2 的32位中提取数据。


* *注：转换回字节流时，无需关心 Start/Continue/End 状态位，只需提取数据，因为 F0/F7 已包含在数据字节中。*


* **代码逻辑**：
```cpp
// 输入为 word1, word2
int count = (word1 >> 16) & 0xF; // 获取有效字节数 (0-6)

// 按序提取并压入 buffer
if (count >= 1) buffer.push_back((word1 >> 8) & 0xFF);
if (count >= 2) buffer.push_back(word1 & 0xFF);
if (count >= 3) buffer.push_back((word2 >> 24) & 0xFF);
// ... 依此类推至 count

```



---

### 5. 附录：其它注意事项

1. **实时消息 (Realtime Messages)**
* **Type 1 (System Common)**：结构与 Type 2 类似，处理逻辑同“短消息处理”。
* **位置**：若字节流中夹杂 `F8` (Timing Clock)，需单独封装为 Type 1 UMP 插队上报。


2. **Jitter Reduction (Type 0)**
* **上行**：若底层不支持硬件时间戳，HAL **无需**生成 Type 0 包。
* **下行**：若收到 Type 0 包，HAL 应对其进行**丢弃**处理，不产生字节流。


3. **异常保护**
* **上行超时**：若字节流中 `F0` 后迟迟未收到 `F7`，HAL 应丢弃缓冲区并复位状态机。
* **下行校验**：若 UMP Type 3 的 `Count` 字段 > 6，视为非法包丢弃。