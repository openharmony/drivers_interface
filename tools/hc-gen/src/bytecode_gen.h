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

#ifndef HC_GEN_BYTECODE_GEN_H
#define HC_GEN_BYTECODE_GEN_H

#include <fstream>
#include "generator.h"

namespace OHOS {
namespace Hardware {
struct OpCode {
    OpCode() : opCode(0), size(0) {}

    OpCode(uint8_t code, uint32_t s, const std::string &str) : opCode(code), size(s), opStr(str) {}

    ~OpCode() = default;

    uint8_t opCode;
    uint32_t size;
    const std::string opStr;
};

class ByteCodeGen : public Generator {
public:
    explicit ByteCodeGen(std::shared_ptr<Ast> ast);

    ~ByteCodeGen() override = default;

    bool Output() override;

private:
    bool Initialize();

    bool ByteCodeConvert();

    uint32_t Align(uint32_t size) const;

    void CalculateSize(const std::shared_ptr<AstObject> &object) const;

    bool ByteCodeWrite(bool dummy);

    bool ByteCodeWriteWalk();

    template <typename T>
    void Write(T &data);

    void Write(const char *data, uint32_t size);

    void Write(const std::string &data);

    void FsWrite(const char *data, uint32_t size);

    static const OpCode &ToOpCode(uint32_t objectType);

    bool WriteBad();

    bool HexdumpInitialize(FILE *&in, FILE *&out);

    static bool HexdumpOutput(FILE *in, FILE *out);

    bool Hexdump();

    bool needAlign_;
    std::ofstream ofs_;
    std::string outFileName_;
    bool dummyOutput_;
    uint32_t writeSize_;
};
} // namespace Hardware
} // namespace OHOS
#endif // HC_GEN_BYTECODE_GEN_H
