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

#ifndef OHOS_HDI_DISPLAY_V1_5_DISPLAY_COMPOSER_HDI_IMPL_H
#define OHOS_HDI_DISPLAY_V1_5_DISPLAY_COMPOSER_HDI_IMPL_H

#include "common/include/display_config.h"
#include "v1_4/hdi_impl/display_composer_hdi_impl.h"
#include "v1_5/display_command/display_cmd_requester.h"
#include "v1_5/display_composer_type.h"
#include "v1_5/idisplay_composer.h"
#include "v1_5/include/idisplay_composer_interface.h"
#include <cinttypes>

#undef LOG_TAG
#define LOG_TAG "DISP_HDI_COMP"
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002515

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_5 {

using namespace OHOS::HDI::Display;

template <typename Interface, typename CompHdi, typename CmdReq>
class DisplayComposerHdiImpl : public V1_4::DisplayComposerHdiImpl<Interface, CompHdi, CmdReq> {
public:
    static IDisplayComposerInterface* Create(bool needSMQ)
    {
        sptr<CompHdi> hdi;
        std::shared_ptr<CmdReq> req = nullptr;
        HDF_LOGI("%{public}s: hdi v1_5 start", __func__);
        if ((hdi = CompHdi::Get()) == nullptr) {
            // Let the caller handle the retry logic
            return nullptr;
        }
        HDF_LOGI("%{public}s: hdi v1_5 end", __func__);
        if (needSMQ) {
            req = CmdReq::Create(hdi);
            if (req == nullptr) {
                HDF_LOGE("%{public}s: Create DisplayCmdRequester failed %{public}d", __func__, __LINE__);
                return nullptr;
            }
        }
        return new DisplayComposerHdiImpl(hdi, req);
    }

    DisplayComposerHdiImpl(sptr<CompHdi> hdi, std::shared_ptr<CmdReq> req)
        : BaseType1_4(hdi, req), hdi_v1_5_(hdi), req_v1_5_(req) {}

    virtual ~DisplayComposerHdiImpl() {}

    virtual int32_t GetDisplayVCPFeature(uint32_t devId, uint8_t vcpCode,
        uint16_t& currentValue, uint16_t& maximumValue, int32_t& replyErrorCode) override
    {
        COMPOSER_CHECK_NULLPTR_RETURN(hdi_v1_5_);
        return ToDispErrCode(hdi_v1_5_->GetDisplayVCPFeature(
            devId, vcpCode, currentValue, maximumValue, replyErrorCode));
    }
    
    virtual int32_t SetDisplayVCPFeature(uint32_t devId, uint8_t vcpCode, uint16_t currentValue) override
    {
        COMPOSER_CHECK_NULLPTR_RETURN(hdi_v1_5_);
        return ToDispErrCode(hdi_v1_5_->SetDisplayVCPFeature(devId, vcpCode, currentValue));
    }

    virtual int32_t GetLayerColor(uint32_t devId, uint32_t layerId, LayerColor &color) override
    {
        COMPOSER_CHECK_NULLPTR_RETURN(hdi_v1_5_);
        return ToDispErrCode(hdi_v1_5_->GetLayerColor(devId, layerId, color));
    }

protected:
    using BaseType1_4 = V1_4::DisplayComposerHdiImpl<Interface, CompHdi, CmdReq>;
    using BaseType1_4::WAIT_TIME_INTERVAL;
    using BaseType1_4::ToDispErrCode;

    sptr<CompHdi> hdi_v1_5_;
    std::shared_ptr<CmdReq> req_v1_5_;
};
using HdiDisplayComposer = DisplayComposerHdiImpl<IDisplayComposerInterface, IDisplayComposer, HdiDisplayCmdRequester>;
} // namespace V1_5
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_5_DISPLAY_COMPOSER_HDI_IMPL_H
