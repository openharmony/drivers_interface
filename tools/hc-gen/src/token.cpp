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

#include <iomanip>
#include <map>

#include "token.h"

using namespace OHOS::Hardware;

static constexpr int WIDTH_ZERO = 0;
static constexpr int WIDTH_EIGHT = 8;
static constexpr int WIDTH_EIGHTEEN = 18;
static constexpr int WIDTH_TWENTY = 20;

std::string OHOS::Hardware::TokenType2String(int32_t type)
{
    static std::map<int32_t, std::string> tokenTypeMap = {
        {NUMBER,    "NUMBER"   },
        {TEMPLATE,  "TEMPLATE" },
        {LITERAL,   "LITERAL"  },
        {ROOT,      "ROOT"     },
        {INCLUDE,   "INCLUDE"  },
        {DELETE,    "DELETE"   },
        {STRING,    "STRING"   },
        {REF_PATH,  "REF_PATH" },
        {FILE_PATH, "FILE_PATH"}
    };

    std::string str;
    if (type < '~') {
        str.push_back(static_cast<char>(type));
        return str;
    } else if (tokenTypeMap.find(type) != tokenTypeMap.end()) {
        str = tokenTypeMap[type];
    }

    return str;
}

std::ostream &OHOS::Hardware::operator<<(std::ostream &stream, const OHOS::Hardware::Token &token)
{
    stream << "Token: type: " << std::setw(WIDTH_EIGHT) << ::std::left << TokenType2String(token.type);
    stream << " value: " << std::setw(WIDTH_EIGHT) << ::std::left;
    token.type != NUMBER ? stream << std::setw(WIDTH_TWENTY) << token.strval
                     : stream << std::setw(WIDTH_ZERO) << "0x" << std::setw(WIDTH_EIGHTEEN) <<
                       std::hex << token.numval;
    stream << " lineno:" << token.lineNo;
    return stream;
}

Token::Token() : type(0), strval(), numval(0), src(), lineNo(0) {}

bool Token::operator==(int32_t otherType) const
{
    return otherType == type;
}

bool Token::operator!=(int32_t otherType) const
{
    return otherType != type;
}
bool Token::operator==(const Token &token) const
{
    return token.type == type && token.numval == numval && token.strval == strval;
}

bool Token::operator!=(const Token &token) const
{
    return token.type != type || token.numval != numval || token.strval != strval;
}
