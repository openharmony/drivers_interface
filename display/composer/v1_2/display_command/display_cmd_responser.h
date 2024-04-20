/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_HDI_DISPLAY_V1_2_DISPLAY_CMD_REQUESTER_H
#define OHOS_HDI_DISPLAY_V1_2_DISPLAY_CMD_REQUESTER_H

#include "v1_1/display_command/display_cmd_responser.h"
#include "v1_2/display_composer_type.h"
#include "v1_2/display_command/display_cmd_utils.h"

#define DISPLAY_TRACE HdfTrace trace(__func__, "HDI:DISP:")

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_2 {
using namespace OHOS::HDI::Base;

template <typename Transfer, typename VdiImpl>
class DisplayCmdResponser : public V1_1::DisplayCmdResponser<Transfer, VdiImpl> {
public:
    static std::unique_ptr<DisplayCmdResponser> Create(VdiImpl* impl, std::shared_ptr<DeviceCacheManager> cacheMgr)
    {
        DISPLAY_CHK_RETURN(impl == nullptr, nullptr,
            HDF_LOGE("%{public}s: error, VdiImpl is nullptr", __func__));
        DISPLAY_CHK_RETURN(cacheMgr == nullptr, nullptr,
            HDF_LOGE("%{public}s: error, VdiImpl is nullptr", __func__));
        return std::make_unique<DisplayCmdResponser>(impl, cacheMgr);
    }

    DisplayCmdResponser(VdiImpl* impl, std::shared_ptr<DeviceCacheManager> cacheMgr) : BaseType1_1(impl, cacheMgr) {}

    virtual ~DisplayCmdResponser() {}

    int32_t ProcessRequestCmd(std::shared_ptr<CommandDataUnpacker> unpacker, int32_t cmd,
        const std::vector<HdifdInfo>& inFds, std::vector<HdifdInfo>& outFds)
    {
        int32_t ret = HDF_SUCCESS;
        std::vector<uint32_t> layers;
        std::vector<int32_t> fences;
        HDF_LOGD("%{public}s: HDI 1.2 PackSection, cmd-[%{public}d] = %{public}s",
            __func__, cmd, CmdUtils::CommandToString(cmd));
        switch (cmd) {
            case REQUEST_CMD_PREPARE_DISPLAY_LAYERS:
                OnPrepareDisplayLayers(unpacker);
                break;
            case REQUEST_CMD_SET_DISPLAY_CLIENT_BUFFER:
                OnSetDisplayClientBuffer(unpacker, inFds);
                break;
            case REQUEST_CMD_SET_DISPLAY_CLIENT_DAMAGE:
                OnSetDisplayClientDamage(unpacker);
                break;
            case REQUEST_CMD_COMMIT:
                OnCommit(unpacker, outFds);
                break;
            case REQUEST_CMD_SET_LAYER_ALPHA:
                OnSetLayerAlpha(unpacker);
                break;
            case REQUEST_CMD_SET_LAYER_REGION:
                OnSetLayerRegion(unpacker);
                break;
            case REQUEST_CMD_SET_LAYER_CROP:
                OnSetLayerCrop(unpacker);
                break;
            case REQUEST_CMD_SET_LAYER_ZORDER:
                OnSetLayerZorder(unpacker);
                break;
            case REQUEST_CMD_SET_LAYER_PREMULTI:
                OnSetLayerPreMulti(unpacker);
                break;
            case REQUEST_CMD_SET_LAYER_TRANSFORM_MODE:
                OnSetLayerTransformMode(unpacker);
                break;
            case REQUEST_CMD_SET_LAYER_DIRTY_REGION:
                OnSetLayerDirtyRegion(unpacker);
                break;
            case REQUEST_CMD_SET_LAYER_VISIBLE_REGION:
                OnSetLayerVisibleRegion(unpacker);
                break;
            case REQUEST_CMD_SET_LAYER_BUFFER:
                OnSetLayerBuffer(unpacker, inFds);
                break;
            case REQUEST_CMD_SET_LAYER_COMPOSITION_TYPE:
                OnSetLayerCompositionType(unpacker);
                break;
            case REQUEST_CMD_SET_LAYER_BLEND_TYPE:
                OnSetLayerBlendType(unpacker);
                break;
            case REQUEST_CMD_SET_LAYER_MASK_INFO:
                OnSetLayerMaskInfo(unpacker);
                break;
            case CONTROL_CMD_REQUEST_END:
                ret = OnRequestEnd(unpacker);
                break;
            case REQUEST_CMD_SET_LAYER_COLOR:
                OnSetLayerColor(unpacker);
                break;
            case REQUEST_CMD_COMMIT_AND_GET_RELEASE_FENCE:
                OnCommitAndGetReleaseFence(unpacker, outFds, layers, fences);
                break;
            default:
                HDF_LOGE("%{public}s: not support this cmd, unpacked cmd = %{public}d", __func__, cmd);
                ret = HDF_FAILURE;
                break;
        }
        return ret;
    }

    void ReplyCommitAndGetReleaseFence(std::vector<HdifdInfo>& outFds, uint32_t& devId, int32_t& fence,
        int32_t& skipRet, std::vector<uint32_t>& compLayers, std::vector<int32_t>& compTypes, bool& needFlush,
            std::vector<uint32_t>& layers, std::vector<int32_t>& fences)
    {
        int32_t ret = HDF_SUCCESS;
        uint32_t vectSize = 0;

        HdifdParcelable fdParcel(fence);
        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS,
            CmdUtils::StartSection(REPLY_CMD_COMMIT_AND_GET_RELEASE_FENCE, replyPacker_),
            HDF_LOGE("%{public}s, StartSection error", __func__));

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::FileDescriptorPack(fdParcel.GetFd(), replyPacker_, outFds),
            HDF_LOGE("%{public}s, FileDescriptorPack error", __func__));

        DISPLAY_CHECK(replyPacker_->WriteInt32(skipRet) == false,
            HDF_LOGE("%{public}s, write skip validate return value error", __func__));
        if (skipRet != HDF_SUCCESS) {
            DISPLAY_CHECK(replyPacker_->WriteUint32(devId) == false,
                HDF_LOGE("%{public}s, write devId error", __func__));

            DISPLAY_CHECK(replyPacker_->WriteBool(needFlush) == false,
                HDF_LOGE("%{public}s, write needFlush error", __func__));

            // Write compLayers vector
            vectSize = static_cast<uint32_t>(compLayers.size());
            DISPLAY_CHECK(replyPacker_->WriteUint32(vectSize) == false,
                HDF_LOGE("%{public}s, write compLayers.size error", __func__));

            for (uint32_t i = 0; i < vectSize; i++) {
                DISPLAY_CHECK(replyPacker_->WriteUint32(compLayers[i]) == false,
                    HDF_LOGE("%{public}s, write compLayers error", __func__));
            }
            // Write compTypes vector
            vectSize = static_cast<uint32_t>(compTypes.size());
            DISPLAY_CHECK(replyPacker_->WriteUint32(vectSize) == false,
                HDF_LOGE("%{public}s, write compTypes.size error", __func__));

            for (uint32_t i = 0; i < vectSize; i++) {
                DISPLAY_CHECK(replyPacker_->WriteUint32(compTypes[i]) == false,
                    HDF_LOGE("%{public}s, write compTypes error", __func__));
            }
        }

            // Write layers vector
            vectSize = static_cast<uint32_t>(layers.size());
            DISPLAY_CHECK(replyPacker_->WriteUint32(vectSize) == false,
                HDF_LOGE("%{public}s, write layers.size error", __func__));

            for (uint32_t i = 0; i < vectSize; i++) {
                DISPLAY_CHECK(replyPacker_->WriteUint32(layers[i]) == false,
                    HDF_LOGE("%{public}s, write layers error", __func__));
            }

            // Write fences vector
            vectSize = static_cast<uint32_t>(fences.size());
            DISPLAY_CHECK(replyPacker_->WriteUint32(vectSize) == false,
                HDF_LOGE("%{public}s, write fences.size error", __func__));

            for (uint32_t i = 0; i < vectSize; i++) {
                ret = CmdUtils::FileDescriptorPack(fences[i], replyPacker_, outFds);
                DISPLAY_CHECK(ret != HDF_SUCCESS,
                    HDF_LOGE("%{public}s, write fences error", __func__));
            }

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::EndSection(replyPacker_),
            HDF_LOGE("%{public}s, EndSection error", __func__));

        replyCommandCnt_++;

#ifndef DISPLAY_COMMUNITY
        fdParcel.Move();
#endif // DISPLAY_COMMUNITY

        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_COMMIT_AND_GET_RELEASE_FENCE, ret);
        }
    }

    void OnCommitAndGetReleaseFence(std::shared_ptr<CommandDataUnpacker> unpacker,
        std::vector<HdifdInfo>& outFds, std::vector<uint32_t>& layers, std::vector<int32_t>& fences)
    {
        DISPLAY_TRACE;

        uint32_t devId = 0;
        int32_t fence = -1;
        bool isSupportSkipValidate = false;
        int32_t ret = HDF_SUCCESS;
        int32_t skipRet = HDF_FAILURE;
        bool needFlush = false;
        std::vector<uint32_t> compLayers;
        std::vector<int32_t> compTypes;
#ifdef DISPLAY_COMSPOER_DEBUG_DUMP
        const std::string SWITCH_ON = "on";
        const uint32_t DUMP_CACHE_SWITCH_LEN = 4;
        char dumpSwitch[DUMP_CACHE_SWITCH_LEN] = {0};
        GetParameter("hdi.composer.dumpcache", "off", dumpSwitch, DUMP_CACHE_SWITCH_LEN);

        if (SWITCH_ON.compare(dumpSwitch) == 0) {
            cacheMgr_->Dump();
        }
#endif
        if (!unpacker->ReadUint32(devId)) {
            HDF_LOGE("%{public}s, read devId error", __func__);
            ret = HDF_FAILURE;
            goto REPLY;
        }
        if (!unpacker->ReadBool(isSupportSkipValidate)) {
            HDF_LOGE("%{public}s, read isSupportSkipValidate error", __func__);
            ret = HDF_FAILURE;
            goto REPLY;
        }

        if (isSupportSkipValidate) {
            skipRet = impl_->Commit(devId, fence);
        }
        if (skipRet != HDF_SUCCESS) {
            ret = impl_->PrepareDisplayLayers(devId, needFlush);
            DISPLAY_CHECK(ret != HDF_SUCCESS, goto REPLY);

            ret = impl_->GetDisplayCompChange(devId, compLayers, compTypes);
            DISPLAY_CHECK(ret != HDF_SUCCESS, goto REPLY);
        }

        HDF_LOGD("%{public}s, first commit with skipRet = %{public}d, fence = %{public}d, needFlush = %{public}d",
            __func__, skipRet, fence, needFlush);
				
        ret = impl_->GetDisplayReleaseFence(devId, layers, fences);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%{public}s, GetDisplayReleaseFence failed with ret = %{public}d", __func__, ret);
        }

REPLY:
        ReplyCommitAndGetReleaseFence(outFds, devId, fence, skipRet, compLayers, compTypes, needFlush, layers, fences);
        return;
    }

    int32_t CmdRequest(uint32_t inEleCnt, const std::vector<HdifdInfo>& inFds, uint32_t& outEleCnt,
        std::vector<HdifdInfo>& outFds)
    {
        std::shared_ptr<char> requestData(new char[inEleCnt * CmdUtils::ELEMENT_SIZE], std::default_delete<char[]>());
        int32_t ret = request_->Read(reinterpret_cast<int32_t *>(requestData.get()), inEleCnt,
            CmdUtils::TRANSFER_WAIT_TIME);

        std::shared_ptr<CommandDataUnpacker> unpacker = std::make_shared<CommandDataUnpacker>();
        DISPLAY_CHK_RETURN(unpacker == nullptr, HDF_FAILURE,
            HDF_LOGE("%{public}s: unpacker construct failed", __func__));

        unpacker->Init(requestData.get(), inEleCnt * CmdUtils::ELEMENT_SIZE);
#ifdef DEBUG_DISPLAY_CMD_RAW_DATA
        unpacker->Dump();
#endif // DEBUG_DISPLAY_CMD_RAW_DATA

        int32_t unpackCmd = -1;
        bool retBool = unpacker->PackBegin(unpackCmd);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
            HDF_LOGE("%{public}s: error: Check RequestBegin failed", __func__));
        DISPLAY_CHK_RETURN(unpackCmd != CONTROL_CMD_REQUEST_BEGIN, HDF_FAILURE,
            HDF_LOGI("error: unpacker PackBegin cmd not match, cmd(%{public}d)=%{public}s.", unpackCmd,
            CmdUtils::CommandToString(unpackCmd)));

        while (ret == HDF_SUCCESS && unpacker->NextSection()) {
            if (!unpacker->BeginSection(unpackCmd)) {
                HDF_LOGE("error: PackSection failed, unpackCmd=%{public}s.",
                    CmdUtils::CommandToString(unpackCmd));
                ret = HDF_FAILURE;
            }
            ret = ProcessRequestCmd(unpacker, unpackCmd, inFds, outFds);
        }

        DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
            HDF_LOGE("%{public}s: ProcessRequestCmd failed", __func__));
        /* pack request end commands */
        replyPacker_->PackEnd(CONTROL_CMD_REPLY_END);

#ifdef DEBUG_DISPLAY_CMD_RAW_DATA
        /* just for debug */
        replyPacker_->Dump();
        HDF_LOGI("CmdReply command cnt=%{public}d", replyCommandCnt_);
#endif // DEBUG_DISPLAY_CMD_RAW_DATA

        /*  Write reply pack */
        outEleCnt = replyPacker_->ValidSize() / CmdUtils::ELEMENT_SIZE;
        ret = reply_->Write(reinterpret_cast<int32_t *>(replyPacker_->GetDataPtr()), outEleCnt,
            CmdUtils::TRANSFER_WAIT_TIME);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("Reply write failure, ret=%{public}d", ret);
            outEleCnt = 0;
        }
        int32_t ec = PeriodDataReset();
        return (ret == HDF_SUCCESS ? ec : ret);
    }

private:
    using BaseType1_1 = V1_1::DisplayCmdResponser<Transfer, VdiImpl>;
    using BaseType1_1::replyPacker_;
    using BaseType1_1::cacheMgr_;
    using BaseType1_1::impl_;
    using BaseType1_1::replyCommandCnt_;
    using BaseType1_1::errMaps_;
    using BaseType1_1::request_;
    using BaseType1_1::reply_;
    using BaseType1_1::PeriodDataReset;
    using BaseType1_1::OnPrepareDisplayLayers;
    using BaseType1_1::OnSetDisplayClientBuffer;
    using BaseType1_1::OnSetDisplayClientDamage;
    using BaseType1_1::OnCommit;
    using BaseType1_1::OnSetLayerAlpha;
    using BaseType1_1::OnSetLayerRegion;
    using BaseType1_1::OnSetLayerCrop;
    using BaseType1_1::OnSetLayerZorder;
    using BaseType1_1::OnSetLayerPreMulti;
    using BaseType1_1::OnSetLayerTransformMode;
    using BaseType1_1::OnSetLayerDirtyRegion;
    using BaseType1_1::OnSetLayerVisibleRegion;
    using BaseType1_1::OnSetLayerBuffer;
    using BaseType1_1::OnSetLayerCompositionType;
    using BaseType1_1::OnSetLayerBlendType;
    using BaseType1_1::OnSetLayerMaskInfo;
    using BaseType1_1::OnRequestEnd;
    using BaseType1_1::OnSetLayerColor;
};
using HdiDisplayCmdResponser = DisplayCmdResponser<SharedMemQueue<int32_t>, IDisplayComposerVdi>;
} // namespace V1_2
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_2_DISPLAY_CMD_REQUESTER_H