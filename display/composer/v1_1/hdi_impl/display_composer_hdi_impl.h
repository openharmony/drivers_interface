/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_HDI_DISPLAY_V1_1_DISPLAY_COMPOSER_HDI_IMPL_H
#define OHOS_HDI_DISPLAY_V1_1_DISPLAY_COMPOSER_HDI_IMPL_H

#include "v1_0/hdi_impl/display_composer_hdi_impl.h"
#include "v1_1/display_command/display_cmd_requester.h"
#include "v1_1/display_composer_type.h"
#include "v1_1/idisplay_composer.h"
#include "v1_1/include/idisplay_composer_interface.h"

#undef LOG_TAG
#define LOG_TAG "DISP_HDI_COMP"
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002500

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_1 {

template <typename Interface, typename CompHdi, typename CmdReq>
class DisplayComposerHdiImpl : public V1_0::DisplayComposerHdiImpl<Interface, CompHdi, CmdReq>,
    public ISeamlessChangeCallback, public IModeCallback {
public:
    static IDisplayComposerInterface* Create()
    {
        sptr<CompHdi> hdi;
        uint32_t count = 0;

        while ((hdi = CompHdi::Get()) == nullptr) {
            HDF_LOGE("%{public}s: get display_composer_service, count = %{public}d, %{public}d",
                __func__, ++count, __LINE__);
            // Waiting for display composer service ready
            usleep(WAIT_TIME_INTERVAL);
        }

        std::shared_ptr<CmdReq> req = CmdReq::Create(hdi);
        if (req == nullptr) {
            HDF_LOGE("%{public}s: Create DisplayCmdRequester failed %{public}d", __func__, __LINE__);
            return nullptr;
        }
        return new DisplayComposerHdiImpl(hdi, req);
    }

    DisplayComposerHdiImpl(sptr<CompHdi> hdi, std::shared_ptr<CmdReq> req): BaseType1_0(hdi, req), hdi_v1_1_(hdi) {}

    virtual ~DisplayComposerHdiImpl() {}

    virtual int32_t RegSeamlessChangeCallback(SeamlessChangeCallback cb, void *data) override
    {
        seamlessChangeCb_ = cb;
        seamlessChangeCbData_ = data;
        COMPOSER_CHECK_NULLPTR(hdi_v1_1_);
        return ToDispErrCode(hdi_v1_1_->RegSeamlessChangeCallback(this));
    }

    virtual int32_t SetDisplayModeAsync(uint32_t devId, uint32_t modeId, ModeCallback cb) override
    {
        modeCb_ = cb;
        COMPOSER_CHECK_NULLPTR(hdi_v1_1_);
        return ToDispErrCode(hdi_v1_1_->SetDisplayModeAsync(devId, modeId, this));
    }

    virtual int32_t GetDisplaySupportedModesExt(unsigned int devId, std::vector<DisplayModeInfoExt> &modes) override
    {
        COMPOSER_CHECK_NULLPTR(hdi_v1_1_);
        return ToDispErrCode(hdi_v1_1_->GetDisplaySupportedModesExt(devId, modes));
    }

    virtual int32_t GetDisplayVBlankPeriod(uint32_t devId, uint64_t &period) override
    {
        COMPOSER_CHECK_NULLPTR(hdi_v1_1_);
        return ToDispErrCode(hdi_v1_1_->GetDisplayVBlankPeriod(devId, period));
    }

    virtual int32_t OnSeamlessChange(uint32_t outputId) override
    {
        int32_t ret = HDF_SUCCESS;
        if (seamlessChangeCb_) {
            seamlessChangeCb_(outputId, seamlessChangeCbData_);
        } else {
            HDF_LOGE("error: seamlessChange callback is nullptr");
            ret = HDF_FAILURE;
        }
        return ret;
    }

    virtual int32_t OnMode(uint32_t modeId, uint64_t vBlankPeriod) override
    {
        int32_t ret = HDF_SUCCESS;
        if (modeCb_) {
            modeCb_(modeId, vBlankPeriod, nullptr);
        } else {
            HDF_LOGE("error: seamlessChange callback is nullptr");
            ret = HDF_FAILURE;
        }
        return ret;
    }

    private:
    using BaseType1_0 = V1_0::DisplayComposerHdiImpl<Interface, CompHdi, CmdReq>;
    using BaseType1_0::WAIT_TIME_INTERVAL;
    using BaseType1_0::ToDispErrCode;
    sptr<CompHdi> hdi_v1_1_;
    SeamlessChangeCallback seamlessChangeCb_;
    void *seamlessChangeCbData_;
    ModeCallback modeCb_;
};
using HdiDisplayComposer = DisplayComposerHdiImpl<IDisplayComposerInterface, IDisplayComposer, HdiDisplayCmdRequester>;
} // namespace V1_1
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_1_DISPLAY_COMPOSER_HDI_IMPL_H