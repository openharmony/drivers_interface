/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_HDI_DISPLAY_V1_3_DISPLAY_CMD_REQUESTER_H
#define OHOS_HDI_DISPLAY_V1_3_DISPLAY_CMD_REQUESTER_H

#include "v1_0/display_command/display_cmd_responser.h"
#include "v1_1/display_command/display_cmd_responser.h"
#include "v1_2/display_command/display_cmd_responser.h"
#include "v1_3/display_composer_type.h"
#include "v1_3/display_command/display_cmd_utils.h"

#define DISPLAY_TRACE HdfTrace trace(__func__, "HDI:DISP:")

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_3 {
using namespace OHOS::HDI::Base;

template <typename Transfer, typename VdiImpl>
class DisplayCmdResponser : public V1_2::DisplayCmdResponser<Transfer, VdiImpl> {
public:
    static std::unique_ptr<DisplayCmdResponser> Create(VdiImpl* impl, std::shared_ptr<DeviceCacheManager> cacheMgr)
    {
        DISPLAY_CHK_RETURN(impl == nullptr, nullptr,
            HDF_LOGE("%{public}s: error, VdiImpl is nullptr", __func__));
        DISPLAY_CHK_RETURN(cacheMgr == nullptr, nullptr,
            HDF_LOGE("%{public}s: error, cacheMgr is nullptr", __func__));
        return std::make_unique<DisplayCmdResponser>(impl, cacheMgr);
    }

    DisplayCmdResponser(VdiImpl* impl, std::shared_ptr<DeviceCacheManager> cacheMgr) : BaseType1_2(impl, cacheMgr) {}

    virtual ~DisplayCmdResponser() {}

private:
    using BaseType1_2 = V1_2::DisplayCmdResponser<Transfer, VdiImpl>;
};

using HdiDisplayCmdResponser = DisplayCmdResponser<SharedMemQueue<int32_t>, DisplayComposerVdiAdapter>;
} // namespace V1_3
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_3_DISPLAY_CMD_REQUESTER_H