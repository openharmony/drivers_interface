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

#include "bytecode_gen.h"
#include "decompile.h"
#include "startup_cfg_gen.h"
#include "macro_gen.h"
#include "option.h"
#include "parser.h"
#include "text_gen.h"

using namespace OHOS::Hardware;

int main(int argc, char *argv[])
{
    auto option = Option::Instance().Parse(argc, argv);
    if (option.OptionError()) {
        return EFAIL;
    }

    if (option.ShouldShowUsage()) {
        option.ShowUsage();
        return option.OptionError();
    }

    if (option.ShouldShowVersion()) {
        option.ShowVersion();
        return 0;
    }

    if (option.ShouldDecompile()) {
        Decompile decompile(option.GetSourceName());
        return decompile.DoDecompile() ? 0 : EFAIL;
    }

    Parser parser;
    if (!parser.Parse()) {
        return EFAIL;
    }

    if (option.ShouldGenBinaryConfig()) {
        if (!ByteCodeGen(parser.GetAst()).Output()) {
            return EFAIL;
        }
        return 0;
    }

    if (option.ShouldGenTextConfig()) {
        if (!TextGen(parser.GetAst()).Output()) {
            return EFAIL;
        }
    }
    if (option.ShouldGenMacroConfig()) {
        if (!MacroGen(parser.GetAst()).Output()) {
            return EFAIL;
        }
    }

    if (option.ShouldGenStartConfig()) {
        if (!StartupCfgGen(parser.GetAst()).Output()) {
            return EFAIL;
        }
    }

    return 0;
}
