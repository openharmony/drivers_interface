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

#ifndef OHOS_HDI_DISPLAY_V1_4_DISPLAY_COMPOSER_HDI_IMPL_H
#define OHOS_HDI_DISPLAY_V1_4_DISPLAY_COMPOSER_HDI_IMPL_H

#include "common/include/display_config.h"
#include "v1_3/hdi_impl/display_composer_hdi_impl.h"
#include "v1_4/display_command/display_cmd_requester.h"
#include "v1_4/display_composer_type.h"
#include "v1_4/idisplay_composer.h"
#include "v1_4/include/idisplay_composer_interface.h"
#include <cinttypes>

#undef LOG_TAG
#define LOG_TAG "DISP_HDI_COMP"
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002515

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_4 {

using namespace OHOS::HDI::Display;

template <typename Interface, typename CompHdi, typename CmdReq>
class DisplayComposerHdiImpl : public V1_3::DisplayComposerHdiImpl<Interface, CompHdi, CmdReq> {
public:
    static IDisplayComposerInterface* Create(bool needSMQ)
    {
        sptr<CompHdi> hdi;
        std::shared_ptr<CmdReq> req = nullptr;
        HDF_LOGI("%{public}s: hdi v1_4 start", __func__);
        while ((hdi = CompHdi::Get()) == nullptr) {
            // Waiting for display composer service ready
            usleep(WAIT_TIME_INTERVAL);
        }
        HDF_LOGI("%{public}s: hdi v1_4 end", __func__);
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
        : BaseType1_3(hdi, req),
        hdi_v1_4_(hdi),
        req_v1_4_(req) {
            std::lock_guard<std::mutex> lock(reqMapMutex_);
            reqMap_.insert({UINT32_MAX, req_v1_4_});
            HDF_LOGI("%{public}s: CheckEnableParallel %{public}d", __func__, GetEnableParallel());
        }

    virtual ~DisplayComposerHdiImpl() {}

    virtual int32_t GetPanelPowerStatus(uint32_t devId, PanelPowerStatus& status) override
    {
        COMPOSER_CHECK_NULLPTR_RETURN(hdi_v1_4_);
        return ToDispErrCode(hdi_v1_4_->GetPanelPowerStatus(devId, status));
    }

    virtual int32_t OnHotPlug(uint32_t outputId, bool connected) override
    {
        DISPLAY_TRACE;

        HDF_LOGI("OnHotPlug(%{public}u, %{public}u)", outputId, connected);
        int32_t ret = HDF_SUCCESS;
        if (hotPlugCb_ != nullptr) {
            hotPlugCb_(outputId, connected, hotPlugCbData_);
            PrepareParallelRequest(outputId, connected);
        } else {
            HDF_LOGE("error: hot plug callback fn is nullptr");
            ret = HDF_FAILURE;
        }

        std::lock_guard<std::mutex> lock(vsyncStatusMutex_);
        vsyncEnableStatus_[outputId] = false;
        return ret;
    }

    virtual int32_t SetLayerAlpha(uint32_t devId, uint32_t layerId, const LayerAlpha& alpha) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerAlpha(devId, layerId, alpha));
    }

    virtual int32_t SetLayerRegion(uint32_t devId, uint32_t layerId, const IRect& rect) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerRegion(devId, layerId, rect));
    }

    virtual int32_t SetLayerCrop(uint32_t devId, uint32_t layerId, const IRect& rect) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerCrop(devId, layerId, rect));
    }

    virtual int32_t SetLayerZorder(uint32_t devId, uint32_t layerId, uint32_t zorder) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerZorder(devId, layerId, zorder));
    }

    virtual int32_t SetLayerPreMulti(uint32_t devId, uint32_t layerId, bool preMul) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerPreMulti(devId, layerId, preMul));
    }

    virtual int32_t SetLayerTransformMode(uint32_t devId, uint32_t layerId, TransformType type) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerTransformMode(devId, layerId, type));
    }

    virtual int32_t SetLayerDirtyRegion(uint32_t devId, uint32_t layerId, std::vector<IRect>& rects) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerDirtyRegion(devId, layerId, rects));
    }

    virtual int32_t SetLayerVisibleRegion(uint32_t devId, uint32_t layerId, std::vector<IRect>& rects) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerVisibleRegion(devId, layerId, rects));
    }

    virtual int32_t SetLayerBuffer(uint32_t devId, uint32_t layerId, const BufferHandle* buffer, uint32_t seqNo,
        int32_t fence, const std::vector<uint32_t>& deletingList) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerBuffer(devId, layerId, buffer, seqNo, fence, deletingList));
    }

    virtual int32_t SetLayerCompositionType(uint32_t devId, uint32_t layerId, V1_0::CompositionType type) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerCompositionType(devId, layerId, type));
    }

    virtual int32_t SetLayerBlendType(uint32_t devId, uint32_t layerId, BlendType type) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerBlendType(devId, layerId, type));
    }

    virtual int32_t SetLayerMaskInfo(uint32_t devId, uint32_t layerId, const MaskInfo maskInfo) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerMaskInfo(devId, layerId, maskInfo));
    }

    virtual int32_t SetLayerColor(uint32_t devId, uint32_t layerId, const LayerColor& layerColor) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerColor(devId, layerId, layerColor));
    }

    virtual int32_t SetDisplayClientBuffer(uint32_t devId, const BufferHandle* buffer, uint32_t seqNo,
        int32_t fence) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetDisplayClientBuffer(devId, buffer, seqNo, fence));
    }

    virtual int32_t SetDisplayClientDamage(uint32_t devId, std::vector<IRect>& rects) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetDisplayClientDamage(devId, rects));
    }

    virtual int32_t PrepareDisplayLayers(uint32_t devId, bool& needFlushFb) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->PrepareDisplayLayers(devId, needFlushFb));
    }

    virtual int32_t GetDisplayCompChange(
        uint32_t devId, std::vector<uint32_t>& layers, std::vector<int32_t>& types) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->GetDisplayCompChange(devId, layers, types));
    }

    virtual int32_t Commit(uint32_t devId, int32_t& fence) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->Commit(devId, fence));
    }

    virtual int32_t CommitAndGetReleaseFence(uint32_t devId, int32_t& fence, int32_t& skipState, bool& needFlush,
        std::vector<uint32_t>& layers, std::vector<int32_t>& fences, bool isValidated) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        bool isSupportSkipValidate = false;
        {
            std::lock_guard<std::mutex> lock(skipValidateMutex_);
            auto skipValdaiteFlagIter = isSupportSkipValidate_.find(devId);
            if (skipValdaiteFlagIter != isSupportSkipValidate_.end()) {
                isSupportSkipValidate = (skipValdaiteFlagIter->second == 1) ? true : false;
            }
        }
        return ToDispErrCode(reqCur->CommitAndGetReleaseFence(devId, fence,
            isSupportSkipValidate, skipState, needFlush, layers, fences, isValidated));
    }

    virtual int32_t SetDisplayConstraint(uint32_t devId, uint64_t frameID, uint64_t ns, uint32_t type) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetDisplayConstraint(devId, frameID, ns, type));
    }

    virtual int32_t SetLayerPerFrameParameterSmq(uint32_t devId, uint32_t layerId, const std::string& key,
        const std::vector<int8_t>& value) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetLayerPerFrameParameterSmq(devId, layerId, key, value));
    }

    virtual int32_t SetDisplayPerFrameParameterSmq(uint32_t devId, const std::string& key,
        const std::vector<int8_t>& value) override
    {
        std::shared_ptr<CmdReq> reqCur = GetRequest(devId);
        COMPOSER_CHECK_NULLPTR_RETURN(reqCur);
        return ToDispErrCode(reqCur->SetDisplayPerFrameParameterSmq(devId, key, value));
    }

    int32_t GetDisplayConnectionType(uint32_t devId, DisplayConnectionType& outType) override
    {
        COMPOSER_CHECK_NULLPTR_RETURN(hdi_v1_4_);
        return ToDispErrCode(hdi_v1_4_->GetDisplayConnectionType(devId, outType));
    }

    virtual int32_t GetDisplayClientTargetProperty(uint32_t devId, int32_t& pixelFormat, int32_t& dataspace) override
    {
        COMPOSER_CHECK_NULLPTR_RETURN(hdi_v1_4_);
        return ToDispErrCode(hdi_v1_4_->GetDisplayClientTargetProperty(devId, pixelFormat, dataspace));
    }
protected:
    void PrepareParallelRequest(uint32_t outputId, bool connected)
    {
        if (!GetEnableParallel()) {
            return;
        }

        std::lock_guard<std::mutex> lock(reqMapMutex_);
        if (connected) {
            if ((req_v1_4_ != nullptr) && (reqMap_.size() == 1)) {
                reqMap_.insert({outputId, req_v1_4_});
                return;
            }

            std::shared_ptr<CmdReq> ReqCur = CmdReq::Create(hdi_v1_4_, outputId);
            if (ReqCur == nullptr) {
                HDF_LOGE("%{public}s: Create DisplayCmdRequester for devId[%{public}u] failed.", __func__, outputId);
                return;
            }
            reqMap_.insert({outputId, ReqCur});
        } else {
            auto reqItem = reqMap_.find(outputId);
            if (reqItem == reqMap_.end()) {
                HDF_LOGE("%{public}s: Cannot find DisplayCmdRequester for devId[%{public}u].", __func__, outputId);
                return;
            }
            reqMap_.erase(reqItem);
        }
        HDF_LOGI("%{public}s: the size of reqMap_: %{public}zu.", __func__, reqMap_.size());
    }

    std::shared_ptr<CmdReq> GetRequest(uint32_t devId)
    {
        if (!GetEnableParallel()) {
            return req_v1_4_;
        }

        std::lock_guard<std::mutex> lock(reqMapMutex_);
        if (reqMap_.find(devId) == reqMap_.end()) {
            HDF_LOGE("%{public}s: Cannot find DisplayCmdRequester for devId[%{public}u].", __func__, devId);
            return nullptr;
        }
        return reqMap_[devId];
    }

protected:
    using BaseType1_3 = V1_3::DisplayComposerHdiImpl<Interface, CompHdi, CmdReq>;
    using BaseType1_3::WAIT_TIME_INTERVAL;
    using BaseType1_3::ToDispErrCode;
    using BaseType1_3::vsyncStatusMutex_;
    using BaseType1_3::vsyncEnableStatus_;
    using BaseType1_3::skipValidateMutex_;
    using BaseType1_3::isSupportSkipValidate_;
    sptr<CompHdi> hdi_v1_4_;
    std::shared_ptr<CmdReq> req_v1_4_;
    std::mutex reqMapMutex_;
    std::unordered_map<uint32_t, std::shared_ptr<CmdReq>> reqMap_;

private:
    using BaseType1_3::hotPlugCb_;
    using BaseType1_3::hotPlugCbData_;
};
using HdiDisplayComposer = DisplayComposerHdiImpl<IDisplayComposerInterface, IDisplayComposer, HdiDisplayCmdRequester>;
} // namespace V1_4
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_4_DISPLAY_COMPOSER_HDI_IMPL_H
