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

#ifndef HC_GEN_DECOMPILE_H
#define HC_GEN_DECOMPILE_H

#include <fstream>
#include <string>

#include "ast.h"

namespace OHOS {
namespace Hardware {
class Decompile {
public:
    explicit Decompile(const std::string &fileName);

    ~Decompile() = default;

    bool DoDecompile();

private:
    bool InitDecompileFile();

    bool ReadFile(char *buffer, size_t readSize);

    uint32_t GetAlignSize(uint32_t size) const
    {
        if (isAlign_) {
            return (size + ALIGN_SIZE - 1) & (~(ALIGN_SIZE - 1));
        } else {
            return size;
        }
    }

    bool ReadUint8(uint8_t &value);

    bool ReadUint16(uint16_t &value);

    bool ReadUint32(uint32_t &value);

    bool ReadUint64(uint64_t &value);

    bool ReadString(std::string &value);

    void SetAlign(bool isAlign);

    bool VerifyDecompileFile();

    bool GetNextByteCode(uint32_t &byteCode);

    std::shared_ptr<AstObject> RebuildObject(uint8_t opCode);

    std::shared_ptr<AstObject> RebuildNode();

    std::shared_ptr<AstObject> RebuildTerm();

    std::shared_ptr<AstObject> RebuildNodeRefObject();

    std::shared_ptr<AstObject> RebuildNumberObject(uint8_t opCode);

    std::shared_ptr<AstObject> RebuildArray();

    std::shared_ptr<AstObject> RebuildStringObject();

    std::shared_ptr<Ast> RebuildAst();

    bool isAlign_;
    std::string fileName_;
    std::ifstream file_;
};
} // namespace Hardware
} // namespace OHOS
#endif // HC_GEN_DECOMPILE_H
