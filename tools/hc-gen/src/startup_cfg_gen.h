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

#ifndef HC_STARTUP_CFG_GEN_H
#define HC_STARTUP_CFG_GEN_H

#include <fstream>
#include <map>
#include <vector>
#include <set>

#include "generator.h"

namespace OHOS {
namespace Hardware {
struct HostInfo {
    std::string hostCaps;
    std::string hostUID;
    std::string hostGID;
    std::string hostCritical;
    std::vector<std::string> initConfig;
    std::vector<std::pair<std::string, std::string>> mallocOpt;
    uint32_t hostPriority;
    int32_t processPriority;
    int32_t threadPriority;
    uint32_t hostId;
    uint32_t sandBox;
    bool dynamicLoad;
};

enum EXPECT_TYPE {
    EXPECT_HOST_NAME,
    EXPECT_HOST_PRIORITY,
    EXPECT_HOST_CAPS,
    EXPECT_DEVICE_PROLOAD,
    EXPECT_TYPE_BUTT,
};

class StartupCfgGen : public Generator {
public:
    explicit StartupCfgGen(const std::shared_ptr<Ast> &ast);

    ~StartupCfgGen() override = default;

    bool Output() override;

private:
    bool Initialize();

    static void InitHostInfo(HostInfo &hostData);

    bool TemplateNodeSeparate();

    bool GetHostInfo();

    void HostInfoOutput(const std::string &name, bool end);

    void HeaderTopOutput();

    void HeaderBottomOutput();

    void HostInfosOutput();

    void EmitDynamicLoad(const std::string &name, std::set<std::string> &configedKeywords);

    void EmitPathInfo(const std::string &name, std::set<std::string> &configedKeywords);

    void EmitIdInfo(const std::string &name, std::set<std::string> &configedKeywords);

    void EmitHostCapsInfo(const std::string &name, std::set<std::string> &configedKeywords);

    void EmitHostCriticalInfo(const std::string &name, std::set<std::string> &configedKeywords);

    void EmitSandBoxInfo(const std::string &name, std::set<std::string> &configedKeywords);

    void EmitSeconInfo(const std::string &name, std::set<std::string> &configedKeywords);

    void EmitInitConfigInfo(const std::string &name);

    static void GetConfigArray(const std::shared_ptr<AstObject> &term, std::string &config);

    static void GetConfigVector(const std::shared_ptr<AstObject> &term, std::vector<std::string> &config);

    static void GetConfigIntArray(const std::shared_ptr<AstObject> &term, std::string &config);

    static void GetHostLoadMode(const std::shared_ptr<AstObject> &hostInfo, HostInfo &hostData);

    static void GetHostGID(const std::shared_ptr<AstObject> &term, std::string &config, const std::string &name);

    static void GetProcessPriority(const std::shared_ptr<AstObject> &term, HostInfo &hostData);

    static void GetMallocOpt(const std::shared_ptr<AstObject> &hostInfo,
        std::vector<std::pair<std::string, std::string>> &config);

    std::ofstream ofs_;
    std::string outFileName_;
    std::map<std::string, HostInfo> hostInfoMap_;
};
} // namespace Hardware
} // namespace OHOS
#endif // HC_STARTUP_CFG_GEN_H
