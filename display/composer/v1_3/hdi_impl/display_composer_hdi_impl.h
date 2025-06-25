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

#ifndef OHOS_HDI_DISPLAY_V1_3_DISPLAY_COMPOSER_HDI_IMPL_H
#define OHOS_HDI_DISPLAY_V1_3_DISPLAY_COMPOSER_HDI_IMPL_H

#include "v1_2/hdi_impl/display_composer_hdi_impl.h"
#include "v1_3/display_command/display_cmd_requester.h"
#include "v1_3/display_composer_type.h"
#include "v1_3/idisplay_composer.h"
#include "v1_3/include/idisplay_composer_interface.h"
#include "hdf_trace.h"
#include <cinttypes>

#undef LOG_TAG
#define LOG_TAG "DISP_HDI_COMP"
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002515

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_3 {

template <typename Interface, typename CompHdi, typename CmdReq>
class DisplayComposerHdiImpl : public V1_2::DisplayComposerHdiImpl<Interface, CompHdi, CmdReq>,
    public IHwcEventCallback {
public:
    static IDisplayComposerInterface* Create(bool needSMQ)
    {
        sptr<CompHdi> hdi;
        std::shared_ptr<CmdReq> req = nullptr;
        HDF_LOGI("%{public}s: hdi v1_3 start", __func__);
        while ((hdi = CompHdi::Get()) == nullptr) {
            // Waiting for display composer service ready
            usleep(WAIT_TIME_INTERVAL);
        }
        HDF_LOGI("%{public}s: hdi v1_3 end", __func__);
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
        : BaseType1_2(hdi, req),
        req_v1_3_(req),
        hdi_v1_3_(hdi),
        hwcEventCb_(nullptr),
        hwcEventCbData_(nullptr) {}

    virtual ~DisplayComposerHdiImpl() {}

    virtual int32_t OnHwcEvent(uint32_t devId, uint32_t eventId, const std::vector<int32_t>& eventData) override
    {
        DISPLAY_TRACE;
 
        int32_t ret = HDF_SUCCESS;
        if (hwcEventCb_) {
            hwcEventCb_(devId, eventId, eventData, hwcEventCbData_);
        } else {
            HDF_LOGE("HwcEvent callback is nullptr");
            ret = HDF_FAILURE;
        }
        return ret;
    }
 
    virtual int32_t RegHwcEventCallback(HwcEventCallback cb, void *data) override
    {
        hwcEventCb_ = cb;
        hwcEventCbData_ = data;
        COMPOSER_CHECK_NULLPTR_RETURN(hdi_v1_3_);
        return ToDispErrCode(hdi_v1_3_->RegHwcEventCallback(this));
    }

protected:
    using BaseType1_2 = V1_2::DisplayComposerHdiImpl<Interface, CompHdi, CmdReq>;
    using BaseType1_2::WAIT_TIME_INTERVAL;
    using BaseType1_2::ToDispErrCode;
    std::shared_ptr<CmdReq> req_v1_3_;
    sptr<CompHdi> hdi_v1_3_;
private:
    HwcEventCallback hwcEventCb_;
    void *hwcEventCbData_;
};
using HdiDisplayComposer = DisplayComposerHdiImpl<IDisplayComposerInterface, IDisplayComposer, HdiDisplayCmdRequester>;
} // namespace V1_3
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_3_DISPLAY_COMPOSER_HDI_IMPL_H
