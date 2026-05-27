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

#ifndef OHOS_HDI_DISPLAY_V1_5_DISPLAY_CMD_REQUESTER_H
#define OHOS_HDI_DISPLAY_V1_5_DISPLAY_CMD_REQUESTER_H

#include "v1_4/display_command/display_cmd_requester.h"
#include "v1_5/display_command/display_cmd_utils.h"
#include "v1_5/display_composer_type.h"
#include "v1_5/idisplay_composer.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_5 {
using namespace OHOS::HDI::Base;

template <typename Transfer, typename CompHdi>
class DisplayCmdRequester : public V1_4::DisplayCmdRequester<Transfer, CompHdi> {
public:
    DisplayCmdRequester(sptr<CompHdi> hdi) : BaseType1_4(hdi) {}

    // UINT32_MAX IS THE DEFAULT DEVID.
    static std::unique_ptr<DisplayCmdRequester> Create(sptr<CompHdi> hdi, uint32_t devId = UINT32_MAX)
    {
        DISPLAY_CHK_RETURN(hdi == nullptr, nullptr, HDF_LOGE("%{public}s: hdi is nullptr", __func__));
        auto requester = std::make_unique<DisplayCmdRequester>(hdi);
        DISPLAY_CHK_RETURN(requester == nullptr, nullptr,
            HDF_LOGE("%{public}s: CmdRequester is nullptr", __func__));
        auto ret = requester->Init(CmdUtils::INIT_ELEMENT_COUNT, devId);
        DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, nullptr,
            HDF_LOGE("%{public}s: DisplayCmdRequester init failed", __func__));
        return requester;
    }

private:
    using BaseType1_4 = V1_4::DisplayCmdRequester<Transfer, CompHdi>;
};
using HdiDisplayCmdRequester = V1_5::DisplayCmdRequester<SharedMemQueue<int32_t>, V1_5::IDisplayComposer>;
} // namespace V1_5
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_5_DISPLAY_CMD_REQUESTER_H
