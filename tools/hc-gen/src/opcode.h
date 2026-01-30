/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HC_GEN_OPCODE_H
#define HC_GEN_OPCODE_H

#include <cstdint>

namespace OHOS {
namespace Hardware {
constexpr uint32_t HCB_MAGIC_NUM = 0xA00AA00A;

enum OpCodeType {
    HCS_NODE_OP = 0x01,
    HCS_TERM_OP = 0x02,
    HCS_NODEREF_OP = 0x03,
    HCS_ARRAY_OP = 0x04,
    HCS_BYTE_OP = 0x10,
    HCS_WORD_OP = 0x11,
    HCS_DWORD_OP = 0x12,
    HCS_QWORD_OP = 0x13,
    HCS_STRING_OP = 0x14,
};

struct HcbHeader {
    uint32_t magicNumber;
    uint32_t versionMajor;
    uint32_t versionMinor;
    uint32_t checkSum;
    int32_t totalSize;
};
} // namespace Hardware
} // namespace OHOS

#endif // HC_GEN_OPCODE_H