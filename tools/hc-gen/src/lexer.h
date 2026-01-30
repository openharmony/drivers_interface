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

#ifndef HC_GEN_LEXER_H
#define HC_GEN_LEXER_H

#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "token.h"

namespace OHOS {
namespace Hardware {
class Lexer {
public:
    Lexer();

    ~Lexer() = default;

    bool Initialize(const std::string &sourceName);

    bool Lex(Token &token);

    bool SetTokenCharacter(char c, Token &token);

    friend std::ostream &operator<<(std::ostream &stream, const Lexer &p);

    std::shared_ptr<std::string> GetSourceName() const;

    int32_t GetLineno() const;

    int32_t GetLineLoc() const;

private:
    static constexpr int BUFFER_SIZE = (1024 * 1024);

    void InitToken(Token &token) const;

    bool GetChar(char &c, bool skipSpace = true);

    void ConsumeChar();

    char GetRawChar();

    static bool IsSpace(char c);

    static bool IsNum(char c);

    bool FillBuffer();

    bool ProcessComment();

    bool LexInclude(Token &token);

    bool LexFromString(Token &token);

    bool LexFromNumber(Token &token);

    void LexHexAndBinaryNum(std::string &value, char &c, uint64_t &v);

    void LexFromLiteral(Token &token);

    bool PeekChar(char &c, bool skipSpace = true);

    static std::map<std::string, TokenType> keyWords_;

    std::ifstream src_;
    std::shared_ptr<std::string> srcName_;
    char buffer_[BUFFER_SIZE] {0};
    const char *bufferStart_ {nullptr};
    const char *bufferEnd_ {nullptr};
    int32_t lineno_;
    int32_t lineLoc_;
};

std::ostream &operator<<(std::ostream &stream, const Lexer &p);
} // namespace Hardware
} // namespace OHOS

#endif // HC_GEN_LEXER_H
