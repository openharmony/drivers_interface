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

#ifndef HC_GEN_DECOMPILE_GEN_H
#define HC_GEN_DECOMPILE_GEN_H

#include <fstream>
#include <string>

#include "ast.h"

namespace OHOS {
namespace Hardware {
class DecompileGen {
public:
    DecompileGen(std::shared_ptr<Ast> ast, std::string outPutFileName);

    ~DecompileGen() = default;

    bool OutPut();

private:
    bool Init();

    void WriteFile(const std::string &str);

    int32_t PrintBaseType(const std::shared_ptr<AstObject> &astObj);

    std::string GetNodeRefPath(uint32_t value);

    int32_t PrintArrayType(const std::shared_ptr<AstObject> &astObj);

    uint32_t OutPutWalk(const std::shared_ptr<AstObject> &astObj, int32_t walkDepth);

    uint32_t CloseBrace(const std::shared_ptr<AstObject> &astObj, int32_t walkDepth);

    const std::string fileHeader_;
    std::string outPutFileName_;
    std::ofstream file_;
    std::shared_ptr<Ast> ast_;
};
} // namespace Hardware
} // namespace OHOS
#endif // HC_GEN_DECOMPILE_GEN_H
