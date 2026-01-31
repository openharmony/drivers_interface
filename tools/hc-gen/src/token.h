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

#ifndef HC_GEN_TOKEN_H
#define HC_GEN_TOKEN_H

#include <memory>
#include <ostream>
#include <string>

namespace OHOS {
namespace Hardware {

enum TokenType {
    NUMBER = 256,
    TEMPLATE,
    LITERAL,
    STRING,
    REF_PATH,
    FILE_PATH,
    ROOT,
    INCLUDE,
    DELETE,
};

struct Token {
    int32_t type;
    std::string strval;
    uint64_t numval;
    std::shared_ptr<std::string> src;
    int32_t lineNo;

    Token();
    bool operator==(const Token &token) const;
    bool operator!=(const Token &token) const;
    bool operator==(int32_t otherType) const;
    bool operator!=(int32_t otherType) const;
    friend std::ostream &operator<<(std::ostream &stream, const Token &token);
};

std::ostream &operator<<(std::ostream &stream, const Token &token);
std::string TokenType2String(int32_t type);
} // namespace Hardware
} // namespace OHOS

#endif // HC_GEN_TOKEN_H
