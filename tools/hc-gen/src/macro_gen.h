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

#ifndef HC_GEN_MACRO_GEN_H
#define HC_GEN_MACRO_GEN_H

#include <fstream>
#include <map>

#include "generator.h"

namespace OHOS {
namespace Hardware {
class MacroGen : public Generator {
public:
    explicit MacroGen(std::shared_ptr<Ast> ast);

    ~MacroGen() override = default;

    bool Output() override;

private:
    bool Initialize();

    bool TemplateNodeSeparate();

    std::string GenFullName(int32_t depth, const std::shared_ptr<AstObject> &node, const std::string &sep);

    bool GenNodeForeach(int32_t depth, const std::shared_ptr<AstObject> &node);

    bool NodeWalk();

    void SetTypeData(uint32_t type, const std::shared_ptr<AstObject> &current, uint32_t &arraySize, int32_t depth);

    void SetTypeDataUinit64(const std::string &arrayName, uint32_t &arraySize, uint32_t arrayType,
        const std::shared_ptr<AstObject> &current);

    bool HeaderTopOutput();

    bool HeaderBottomOutput();

    static const std::string &ToUpperString(std::string &str);

    bool GenArray(const std::string &arrName, uint32_t &arrSize, uint32_t type, const std::shared_ptr<AstObject> &node);

    std::string GenRefObjName(int32_t depth, const std::shared_ptr<AstObject> &object);

    std::ofstream ofs_;
    std::string outFileName_;
    std::map<int, std::string> nodeNameMap_;
};
} // namespace Hardware
} // namespace OHOS
#endif // HC_GEN_MACRO_GEN_H
