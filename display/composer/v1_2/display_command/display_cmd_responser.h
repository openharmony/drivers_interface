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

#include "v1_0/display_command/display_cmd_responser.h"
#include "v1_1/display_command/display_cmd_responser.h"
#include "v1_2/display_composer_type.h"
#include "v1_2/display_command/display_cmd_utils.h"
#include "hdf_trace.h"

#define DISPLAY_TRACE HdfTrace trace(__func__, "HDI:DISP:")

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_2 {
using namespace OHOS::HDI::Base;

typedef struct CommitInfo {
    int32_t fence;
    int32_t skipRet;
    bool needFlush;
    std::vector<uint32_t> compLayers;
    std::vector<int32_t> compTypes;
    std::vector<uint32_t> layers;
    std::vector<int32_t> fences;
} CommitInfo;

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

    int32_t ProcessRequestCmd(CommandDataUnpacker& unpacker, int32_t cmd,
        const std::vector<HdifdInfo>& inFds, std::vector<HdifdInfo>& outFds)
    {
        HDF_LOGD("%{public}s: HDI 1.2 PackSection, cmd-[%{public}d] = %{public}s",
            __func__, cmd, CmdUtils::CommandToString(cmd));
        if (cmd == REQUEST_CMD_COMMIT_AND_GET_RELEASE_FENCE) {
            OnCommitAndGetReleaseFence(unpacker, outFds);
        } else if (cmd == REQUEST_CMD_SET_DISPLAY_CONSTRAINT) {
            OnSetDisplayConstraint(unpacker);
        } else if (cmd == REQUEST_CMD_SET_LAYER_PERFRAME_PARAM) {
            OnSetLayerPerFrameParam(unpacker);
        } else {
            return V1_0::DisplayCmdResponser<Transfer, VdiImpl>::ProcessRequestCmd(unpacker, cmd, inFds, outFds);
        }
        return HDF_SUCCESS;
    }

    void ReplyNotSkipInfo(uint32_t& devId, CommitInfo& commitInfo)
    {
        DISPLAY_CHECK(replyPacker_.WriteUint32(devId) == false,
            HDF_LOGE("%{public}s, write devId error", __func__));

        DISPLAY_CHECK(replyPacker_.WriteBool(commitInfo.needFlush) == false,
            HDF_LOGE("%{public}s, write needFlush error", __func__));

        // Write compLayers vector
        uint32_t vectSize = static_cast<uint32_t>(commitInfo.compLayers.size());
        DISPLAY_CHECK(replyPacker_.WriteUint32(vectSize) == false,
            HDF_LOGE("%{public}s, write compLayers.size error", __func__));

        for (uint32_t i = 0; i < vectSize; i++) {
            DISPLAY_CHECK(replyPacker_.WriteUint32(commitInfo.compLayers[i]) == false,
                HDF_LOGE("%{public}s, write compLayers error", __func__));
        }
        // Write compTypes vector
        vectSize = static_cast<uint32_t>(commitInfo.compTypes.size());
        DISPLAY_CHECK(replyPacker_.WriteUint32(vectSize) == false,
            HDF_LOGE("%{public}s, write compTypes.size error", __func__));

        for (uint32_t i = 0; i < vectSize; i++) {
            DISPLAY_CHECK(replyPacker_.WriteUint32(commitInfo.compTypes[i]) == false,
                HDF_LOGE("%{public}s, write compTypes error", __func__));
        }
    }

    void ReplyCommitAndGetReleaseFence(std::vector<HdifdInfo>& outFds, uint32_t& devId, CommitInfo& commitInfo)
    {
        int32_t ret = HDF_SUCCESS;
        uint32_t vectSize = 0;

        HdifdParcelable fdParcel(commitInfo.fence);
        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS,
            CmdUtils::StartSection(REPLY_CMD_COMMIT_AND_GET_RELEASE_FENCE, replyPacker_),
            HDF_LOGE("%{public}s, StartSection error", __func__));

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::FileDescriptorPack(fdParcel.GetFd(), replyPacker_, outFds),
            HDF_LOGE("%{public}s, FileDescriptorPack error", __func__));

        DISPLAY_CHECK(replyPacker_.WriteInt32(commitInfo.skipRet) == false,
            HDF_LOGE("%{public}s, write skip validate return value error", __func__));
        if (commitInfo.skipRet != HDF_SUCCESS) {
            ReplyNotSkipInfo(devId, commitInfo);
        } else {
            // Write layers vector
            vectSize = static_cast<uint32_t>(commitInfo.layers.size());
            DISPLAY_CHECK(replyPacker_.WriteUint32(vectSize) == false,
                HDF_LOGE("%{public}s, write layers.size error", __func__));

            for (uint32_t i = 0; i < vectSize; i++) {
                DISPLAY_CHECK(replyPacker_.WriteUint32(commitInfo.layers[i]) == false,
                    HDF_LOGE("%{public}s, write layers error", __func__));
            }

            // Write fences vector
            vectSize = static_cast<uint32_t>(commitInfo.fences.size());
            DISPLAY_CHECK(replyPacker_.WriteUint32(vectSize) == false,
                HDF_LOGE("%{public}s, write fences.size error", __func__));

            for (uint32_t i = 0; i < vectSize; i++) {
                ret = CmdUtils::FileDescriptorPack(commitInfo.fences[i], replyPacker_, outFds, false);
                DISPLAY_CHECK(ret != HDF_SUCCESS, HDF_LOGE("%{public}s, write fences error", __func__));
            }
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

    void CommitInfoDump(void)
    {
#ifdef DISPLAY_COMSPOER_DEBUG_DUMP
        const std::string SWITCH_ON = "on";
        const uint32_t DUMP_CACHE_SWITCH_LEN = 4;
        char dumpSwitch[DUMP_CACHE_SWITCH_LEN] = {0};
        GetParameter("hdi.composer.dumpcache", "off", dumpSwitch, DUMP_CACHE_SWITCH_LEN);

        if (SWITCH_ON.compare(dumpSwitch) == 0) {
            cacheMgr_->Dump();
        }
#endif
    }

    void OnCommitAndGetReleaseFence(CommandDataUnpacker& unpacker, std::vector<HdifdInfo>& outFds)
    {
        DISPLAY_TRACE;
        uint32_t devId = 0;
        bool isSupportSkipValidate = false;
        bool isValidated = false;
        int32_t ret = HDF_SUCCESS;
        CommitInfo commitInfo;
        commitInfo.fence = -1;
        commitInfo.skipRet = HDF_FAILURE;
        commitInfo.needFlush = false;

        CommitInfoDump();
        if (!unpacker.ReadUint32(devId)) {
            HDF_LOGE("%{public}s, read devId error", __func__);
            goto REPLY;
        }
        if (!unpacker.ReadBool(isSupportSkipValidate)) {
            HDF_LOGE("%{public}s, read isSupportSkipValidate error", __func__);
            goto REPLY;
        }

        if (!unpacker.ReadBool(isValidated)) {
            HDF_LOGE("%{public}s, read isValidated error", __func__);
            goto REPLY;
        }
        if (isSupportSkipValidate || isValidated) {
            HdfTrace traceVdi("Commit", "HDI:DISP:HARDWARE");
            commitInfo.skipRet = impl_->Commit(devId, commitInfo.fence);
        }

        if (commitInfo.skipRet != HDF_SUCCESS && isValidated == false) {
            {
                HdfTrace traceVdi("PrepareDisplayLayers", "HDI:DISP:HARDWARE");
                ret = impl_->PrepareDisplayLayers(devId, commitInfo.needFlush);
            }
            if (ret == HDF_SUCCESS) {
                HdfTrace traceVdi("GetDisplayCompChange", "HDI:DISP:HARDWARE");
                ret = impl_->GetDisplayCompChange(devId, commitInfo.compLayers, commitInfo.compTypes);
            }
        } else {
            HdfTrace traceVdi("GetDisplayReleaseFence", "HDI:DISP:HARDWARE");
            if (impl_->GetDisplayReleaseFence(devId, commitInfo.layers, commitInfo.fences) != HDF_SUCCESS) {
                HDF_LOGE("%{public}s, GetDisplayReleaseFence failed with ret = %{public}d", __func__, ret);
            }
        }
        HDF_LOGD("skipRet:%{public}d,fence:%{public}d,needFlush:%{public}d, ssv:%{public}d, iv:%{public}d",
            commitInfo.skipRet, commitInfo.fence, commitInfo.needFlush, isSupportSkipValidate, isValidated);
REPLY:
        ReplyCommitAndGetReleaseFence(outFds, devId, commitInfo);
    }

    int32_t CmdRequest(uint32_t inEleCnt, const std::vector<HdifdInfo>& inFds, uint32_t& outEleCnt,
        std::vector<HdifdInfo>& outFds)
    {
        std::shared_ptr<char> requestData(new char[inEleCnt * CmdUtils::ELEMENT_SIZE], std::default_delete<char[]>());
        int32_t ret = CmdRequestDataRead(requestData, inEleCnt);

        CommandDataUnpacker unpacker;
        unpacker.Init(requestData.get(), inEleCnt << CmdUtils::MOVE_SIZE);
#ifdef DEBUG_DISPLAY_CMD_RAW_DATA
        unpacker.Dump();
#endif // DEBUG_DISPLAY_CMD_RAW_DATA

        int32_t unpackCmd = -1;
        bool retBool = unpacker.PackBegin(unpackCmd);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
            HDF_LOGE("%{public}s: error: Check RequestBegin failed", __func__));
        DISPLAY_CHK_RETURN(unpackCmd != CONTROL_CMD_REQUEST_BEGIN, HDF_FAILURE,
            HDF_LOGI("error: unpacker PackBegin cmd not match, cmd(%{public}d)=%{public}s.", unpackCmd,
            CmdUtils::CommandToString(unpackCmd)));

        DISPLAY_CHK_RETURN(PeriodDataReset() == HDF_FAILURE, HDF_FAILURE,
                           HDF_LOGE("%{public}s: error: PeriodDataReset failed", __func__));
        while (ret == HDF_SUCCESS && unpacker.NextSection()) {
            if (!unpacker.BeginSection(unpackCmd)) {
                HDF_LOGE("error: PackSection failed, unpackCmd=%{public}s.",
                    CmdUtils::CommandToString(unpackCmd));
                ret = HDF_FAILURE;
                break;
            }
            ret = ProcessRequestCmd(unpacker, unpackCmd, inFds, outFds);
        }

        DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
            HDF_LOGE("%{public}s: ProcessRequestCmd failed", __func__));
        /* pack request end commands */
        replyPacker_.PackEnd(CONTROL_CMD_REPLY_END);

#ifdef DEBUG_DISPLAY_CMD_RAW_DATA
        /* just for debug */
        replyPacker_.Dump();
        HDF_LOGI("CmdReply command cnt=%{public}d", replyCommandCnt_);
#endif // DEBUG_DISPLAY_CMD_RAW_DATA

        /*  Write reply pack */
        outEleCnt = replyPacker_.ValidSize() >> CmdUtils::MOVE_SIZE;
        ret = CmdRequestDataWrite(outEleCnt);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("Reply write failure, ret=%{public}d", ret);
            outEleCnt = 0;
        }
        return ret;
    }

    int32_t OnSetDisplayConstraint(CommandDataUnpacker& unpacker)
    {
        DISPLAY_TRACE;
        uint32_t devId = 0;
        uint64_t frameID = 0;
        uint64_t ns = 0;
        uint32_t type = 0;

        int32_t ret = unpacker.ReadUint32(devId) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = unpacker.ReadUint64(frameID) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = unpacker.ReadUint64(ns) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = unpacker.ReadUint32(type) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        if (impl_ != nullptr && impl_->SetDisplayConstraint != nullptr) {
            ret = impl_->SetDisplayConstraint(devId, frameID, ns, type);
        }

        if (ret != HDF_SUCCESS && ret != DISPLAY_NOT_SUPPORT && ret != HDF_ERR_NOT_SUPPORT) {
            HDF_LOGE("SetDisplayConstraint failed with ret = %{public}d", ret);
        }
EXIT:
        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_DISPLAY_CONSTRAINT, ret);
        }
        return ret;
    }

    int32_t OnSetLayerPerFrameParam(CommandDataUnpacker& unpacker)
    {
        DISPLAY_TRACE;
        uint32_t devId = 0;
        uint32_t layerId = 0;
        std::string key;
        std::vector<int8_t> value;
        uint32_t vectSize = 0;
        uint32_t vectStrSize = 0;

        int32_t ret = unpacker.ReadUint32(devId) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto UNPACKER_EXIT);

        ret = unpacker.ReadUint32(layerId) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto UNPACKER_EXIT);

        ret = unpacker.ReadUint32(vectStrSize) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto UNPACKER_EXIT);

        for (uint32_t i = 0; i < vectStrSize; i++) {
            int32_t tmpChar;
            ret = unpacker.ReadInt32(tmpChar) ? HDF_SUCCESS : HDF_FAILURE;
            if (ret != HDF_SUCCESS) {
                break;
            }
            key.push_back(static_cast<char>(tmpChar));
        }
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto UNPACKER_EXIT);

        ret = unpacker.ReadUint32(vectSize) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto UNPACKER_EXIT);

        for (uint32_t i = 0; i < vectSize; i++) {
            uint8_t tmpValue;
            ret = unpacker.ReadUint8(tmpValue) ? HDF_SUCCESS : HDF_FAILURE;
            if (ret != HDF_SUCCESS) {
                break;
            }
            value.push_back(static_cast<int8_t>(tmpValue));
        }
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto UNPACKER_EXIT);

        HDF_LOGD("OnSetLayerPerFrameParam %{public}s, %{public}d, %{public}d",
            key.c_str(), devId, layerId);

        if (impl_ != nullptr && impl_->SetLayerPerFrameParameter != nullptr) {
            ret = impl_->SetLayerPerFrameParameter(devId, layerId, key, value);
        }

        if (ret != HDF_SUCCESS && ret != DISPLAY_NOT_SUPPORT && ret != HDF_ERR_NOT_SUPPORT) {
            HDF_LOGE("OnSetLayerPerFrameParam %{public}s, %{public}d, %{public}d, %{public}d",
                key.c_str(), devId, layerId, ret);
        }
UNPACKER_EXIT:
        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_PERFRAME_PARAM, ret);
        }
        return ret;
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
    using BaseType1_1::CmdRequestDataRead;
    using BaseType1_1::CmdRequestDataWrite;
    using BaseType1_1::requestMutex_;
    using BaseType1_1::replyMutex_;
};

using HdiDisplayCmdResponser = DisplayCmdResponser<SharedMemQueue<int32_t>, DisplayComposerVdiAdapter>;

} // namespace V1_2
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_2_DISPLAY_CMD_REQUESTER_H