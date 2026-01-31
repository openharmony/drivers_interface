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

#ifndef HC_GEN_OPTION_H
#define HC_GEN_OPTION_H

#include <memory>
#include <string>

namespace OHOS {
namespace Hardware {
class Option {
public:
    Option() = default;

    ~Option() = default;

    static Option &Instance();

    Option &Parse(int argc, char *argv[]);

    static void ShowUsage();

    static void ShowVersion();

    bool ShouldShowUsage() const;

    bool OptionError() const;

    bool ShouldShowVersion() const;

    bool ShouldAlign() const;

    bool ShouldGenTextConfig() const;

    bool ShouldGenMacroConfig() const;

    bool ShouldGenBinaryConfig() const;

    bool ShouldGenStartConfig() const;

    bool ShouldGenHexDump() const;

    bool ShouldDecompile() const;

    std::string GetSymbolPrefix() const;

    std::string GetSourceName() const;

    std::string GetSourceNameBase() const;

    std::string GetOutputName() const;

    static void GetVersion(uint32_t &minor, uint32_t &major);

    bool VerboseLog() const;

    std::string GetSourceDir() const;

    static std::string RealPathSourcePath(const char *path);

private:
    static void ShowOption(const std::string &option, const std::string &helpInfo);
    bool ParseOptions(int argc, char *argv[]);
    void SetOptionError(bool shouldShowUsage = true);
    bool SetSourceOption(const char *srcName);

    bool showUsage_ = false;
    bool showVersion_ = false;
    bool shouldAlign_ = false;
    bool shouldGenTextConfig_ = false;
    bool shouldGenMacroConfig_ = false;
    bool shouldGenByteCodeConfig_ = true;
    bool genStartCfg_ = false;
    bool showGenHexDump_ = false;
    bool shouldDecompile_ = false;
    bool verboseLog_ = false;
    bool optionError_ = false;
    std::string symbolNamePrefix_;
    std::string sourceName_;
    std::string sourceNameBase_;
    std::string outputName_;
    void SetOptionData(char op);
};
} // namespace Hardware
} // namespace OHOS
#endif // HC_GEN_OPTION_H
