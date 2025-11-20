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

#ifndef OHOS_HDI_DISPLAY_V1_4_DISPLAY_CMD_REQUESTER_H
#define OHOS_HDI_DISPLAY_V1_4_DISPLAY_CMD_REQUESTER_H

#include "v1_3/display_command/display_cmd_requester.h"
#include "v1_4/display_command/display_cmd_utils.h"
#include "v1_4/display_composer_type.h"
#include "v1_4/idisplay_composer.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_4 {
using namespace OHOS::HDI::Base;

template <typename Transfer, typename CompHdi>
class DisplayCmdRequester : public V1_3::DisplayCmdRequester<Transfer, CompHdi> {
public:
    DisplayCmdRequester(sptr<CompHdi> hdi) : BaseType1_3(hdi), hdi_v1_4_(hdi) {}

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

    int32_t Init(uint32_t eleCnt, uint32_t devId = UINT32_MAX)
    {
        request_ = std::make_shared<Transfer>(eleCnt, SmqType::SYNCED_SMQ);
        DISPLAY_CHK_RETURN(request_ == nullptr, HDF_FAILURE,
            HDF_LOGE("%{public}s: request_ is nullptr", __func__));

        DISPLAY_CHK_RETURN(hdi_v1_4_ == nullptr, HDF_FAILURE,
            HDF_LOGE("%{public}s: hdi_v1_4_ is nullptr", __func__));

        int32_t ret = hdi_v1_4_->InitSMQInfo(devId, request_, reply_);
        DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
            HDF_LOGE("%{public}s: InitSMQInfo failure, ret=%{public}d", __func__, ret));

        DISPLAY_CHK_RETURN(requestPacker_.Init(request_->GetSize() << CmdUtils::MOVE_SIZE) == false,
            HDF_FAILURE, HDF_LOGE("%{public}s: requestPacker init failed", __func__));

        replyData_.reset(new char[CmdUtils::INIT_ELEMENT_COUNT], std::default_delete<char[]>());
        DISPLAY_CHK_RETURN(replyData_ == nullptr, HDF_FAILURE, HDF_LOGE("replyData alloc failed."));

        HDF_LOGI("%{public}s: Init request_[%{public}u] done", __func__, devId);

        ret = CmdUtils::StartPack(V1_2::CONTROL_CMD_REQUEST_BEGIN, requestPacker_);
        DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
            HDF_LOGE("%{public}s: StartPack failed", __func__));

        return HDF_SUCCESS;
    }

    int32_t CommitAndGetReleaseFence(uint32_t devId, int32_t& fence, bool isSupportSkipValidate, int32_t& skipState,
        bool& needFlush, std::vector<uint32_t>& layers, std::vector<int32_t>& fences, bool isValidated)
    {
        uint32_t replyEleCnt = 0;
        std::vector<HdifdInfo> outFds;

        int32_t ret = CmdUtils::StartSection(REQUEST_CMD_COMMIT_AND_GET_RELEASE_FENCE, requestPacker_);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = requestPacker_.WriteUint32(devId) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = requestPacker_.WriteBool(isSupportSkipValidate) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = requestPacker_.WriteBool(isValidated) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = CmdUtils::EndSection(requestPacker_);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = CmdUtils::EndPack(requestPacker_);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = DoRequest(devId, replyEleCnt, outFds);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = DoReplyResults(replyEleCnt, outFds, [&](void *data) -> int32_t {
            FenceData *fenceData = reinterpret_cast<struct FenceData *>(data);
            if (fenceData == nullptr) {
                fence = -1;
                skipState = -1;
                needFlush = false;
                layers.clear();
                fences.clear();
                return HDF_FAILURE;
            }
            fence = fenceData->fence_;
            skipState = fenceData->skipValidateState_;
            needFlush = fenceData->needFlush_;
            layers = fenceData->layers;
            fences = fenceData->fences;
            return HDF_SUCCESS;
        });
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("DoReplyResults failure, ret=%{public}d", ret);
        }

EXIT:
        return PeriodDataReset() == HDF_SUCCESS ? ret : HDF_FAILURE;
    }

    int32_t DoRequest(uint32_t devId, uint32_t &replyEleCnt, std::vector<HdifdInfo> &outFds)
    {
#ifdef DEBUG_DISPLAY_CMD_RAW_DATA
        requestPacker_.Dump();
#endif // DEBUG_DISPLAY_CMD_RAW_DATA
        uint32_t eleCnt = requestPacker_.ValidSize() >> CmdUtils::MOVE_SIZE;
        int32_t ret = request_->Write(
            reinterpret_cast<int32_t *>(requestPacker_.GetDataPtr()), eleCnt, CmdUtils::TRANSFER_WAIT_TIME);
        DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
            HDF_LOGE("%{public}s: CmdRequest write failed", __func__));

        ret = hdi_v1_4_->DoCmdRequest(devId, eleCnt, requestHdiFds_, replyEleCnt, outFds);
        DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
            HDF_LOGE("%{public}s: CmdRequest failed", __func__));

        if (replyEleCnt != 0) {
            ret = reply_->Read(reinterpret_cast<int32_t *>(replyData_.get()), replyEleCnt,
                CmdUtils::TRANSFER_WAIT_TIME);
            if (ret != HDF_SUCCESS) {
                HDF_LOGE("reply read data failure, ret=%{public}d", ret);
            }
        }

        return ret;
    }

protected:
    sptr<CompHdi> hdi_1_4_;
    sptr<CompHdi> hdi_v1_4_;
private:
    using BaseType1_3 = V1_3::DisplayCmdRequester<Transfer, CompHdi>;
    using BaseType1_3::request_;
    using BaseType1_3::reply_;
    using BaseType1_3::requestPacker_;
    using BaseType1_3::replyData_;
    using BaseType1_3::PeriodDataReset;
    using BaseType1_3::requestHdiFds_;
    using BaseType1_3::DoReplyResults;

    // CommitAndGetReleaseFence
    struct FenceData {
        int32_t fence_ = -1;
        int32_t skipValidateState_ = -1;
        bool needFlush_ = false;
        std::vector<uint32_t> layers;
        std::vector<int32_t> fences;
    };
};
using HdiDisplayCmdRequester = V1_4::DisplayCmdRequester<SharedMemQueue<int32_t>, V1_4::IDisplayComposer>;
} // namespace V1_4
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_4_DISPLAY_CMD_REQUESTER_H
