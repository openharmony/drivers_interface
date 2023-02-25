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

#ifndef OHOS_HDI_DISPLAY_V1_0_DISPLAY_CMD_REQUESTER_H
#define OHOS_HDI_DISPLAY_V1_0_DISPLAY_CMD_REQUESTER_H

#include <unordered_map>

#include "base/hdi_smq.h"
#include "buffer_handle_utils.h"
#include "command_pack/command_data_packer.h"
#include "command_pack/command_data_unpacker.h"
#include "display_cmd_utils.h"
#include "hdf_base.h"
#include "hdifd_parcelable.h"
#include "hilog/log.h"
#include "idisplay_composer_vdi.h"
#include "v1_0/display_composer_type.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_0 {
using namespace OHOS::HDI::Base;
using namespace OHOS::HDI::Display::Composer::V1_0;
using HdifdSet = std::vector<std::shared_ptr<HdifdParcelable>>;

template <typename Transfer, typename VdiImpl>
class DisplayCmdResponser {
public:
    static std::unique_ptr<DisplayCmdResponser> Create(std::shared_ptr<VdiImpl> impl)
    {
        DISPLAY_CHK_RETURN(impl == nullptr, nullptr,
            HDF_LOGE("%{public}s: error, VdiImpl is nullptr", __func__));
        return std::make_unique<DisplayCmdResponser>(impl);
    }

    DisplayCmdResponser(std::shared_ptr<VdiImpl> impl)
        : impl_(impl),
        request_(nullptr),
        isReplyUpdated_(false),
        reply_(nullptr),
        replyCommandCnt_(0),
        replyPacker_(nullptr) {}

    ~DisplayCmdResponser() {}

    int32_t InitCmdRequest(const std::shared_ptr<Transfer>& request)
    {
        DISPLAY_CHK_RETURN(request == nullptr, HDF_FAILURE,
            HDF_LOGE("%{public}s: error, request is nullptr", __func__));
        if (request_ != nullptr) {
            request_.reset();
        }
        request_ = request;

        return HDF_SUCCESS;
    }

    int32_t GetCmdReply(std::shared_ptr<Transfer>& reply)
    {
        int32_t ret = HDF_SUCCESS;
        if (isReplyUpdated_ == false) {
            ret = InitReply(CmdUtils::INIT_ELEMENT_COUNT);
        }
        if (ret == HDF_SUCCESS) {
            if (reply_ != nullptr) {
                reply = reply_;
            } else {
                ret = HDF_FAILURE;
            }
        }
        isReplyUpdated_ = false;
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("error: GetCmdReply failure");
        }

        return ret;
    }

    int32_t ProcessRequestCmd(std::shared_ptr<CommandDataUnpacker> unpacker, int32_t cmd,
        const std::vector<HdifdInfo>& inFds, std::vector<HdifdInfo>& outFds)
    {
        int32_t ret = HDF_SUCCESS;
        HDF_LOGD("%{public}s: PackSection, cmd-[%{public}d] = %{public}s",
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
            case REQUEST_CMD_SET_LAYER_VISIBLE:
                OnSetLayerVisible(unpacker);
                break;
            case CONTROL_CMD_REQUEST_END:
                ret = OnRequestEnd(unpacker);
                break;
            default:
                HDF_LOGE("%{public}s: not support this cmd, unpacked cmd = %{public}d", __func__, cmd);
                ret = HDF_FAILURE;
                break;
        }
        return ret;
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
    int32_t InitReply(uint32_t size)
    {
        reply_ = std::make_shared<Transfer>(size, SmqType::SYNCED_SMQ);
        DISPLAY_CHK_RETURN(reply_ == nullptr, HDF_FAILURE,
            HDF_LOGE("%{public}s: reply_ construct failed", __func__));

        replyPacker_ = std::make_shared<CommandDataPacker>();
        DISPLAY_CHK_RETURN(replyPacker_ == nullptr, HDF_FAILURE,
            HDF_LOGE("%{public}s: replyPacker_ construct failed", __func__));

        bool retBool = replyPacker_->Init(reply_->GetSize() * CmdUtils::ELEMENT_SIZE);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
            HDF_LOGE("%{public}s: replyPacker_ init failed", __func__));

        return CmdUtils::StartPack(CONTROL_CMD_REPLY_BEGIN, replyPacker_);
    }

    int32_t OnRequestEnd(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        size_t errCnt = errMaps_.size();
        if (errCnt >= 0) {
            int32_t ret = CmdUtils::StartSection(REPLY_CMD_SET_ERROR, replyPacker_);
            DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
                HDF_LOGE("%{public}s: StartSection failed", __func__));

            bool retBool = replyPacker_->WriteUint32(errCnt);
            DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
                HDF_LOGE("%{public}s: write errCnt failed", __func__));
            for (auto it = errMaps_.begin(); it != errMaps_.end(); ++it) {
                retBool = replyPacker_->WriteInt32(it->first);
                DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
                    HDF_LOGE("%{public}s: write err-cmd failed, cmdId:%{public}s",
                    __func__, CmdUtils::CommandToString(it->first)));

                retBool = replyPacker_->WriteInt32(it->second);
                DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
                    HDF_LOGE("%{public}s: write errNo failed, errNo:%{public}d", __func__, it->second));
            }
            CmdUtils::EndSection(replyPacker_);
            replyCommandCnt_++;
        }
        return HDF_SUCCESS;
    }

    void OnPrepareDisplayLayers(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        bool needFlush = false;
        int32_t ret = unpacker->ReadUint32(devId) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = impl_->PrepareDisplayLayers(devId, needFlush);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = CmdUtils::StartSection(REPLY_CMD_PREPARE_DISPLAY_LAYERS, replyPacker_);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = replyPacker_->WriteBool(needFlush) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = CmdUtils::EndSection(replyPacker_);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);
        replyCommandCnt_++;
EXIT:
        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_PREPARE_DISPLAY_LAYERS, ret);
        }
        return;
    }

    void OnSetDisplayClientBuffer(std::shared_ptr<CommandDataUnpacker> unpacker, const std::vector<HdifdInfo>& inFds)
    {
        uint32_t devId;
        int32_t ret = unpacker->ReadUint32(devId) ? HDF_SUCCESS : HDF_FAILURE;

        BufferHandle *buffer = nullptr;
        if (ret == HDF_SUCCESS) {
            ret = CmdUtils::BufferHandleUnpack(unpacker, inFds, buffer);
        } else {
            HDF_LOGE("%{public}s, read devId error", __func__);
        }

        int32_t fence = -1;
        if (ret == HDF_SUCCESS) {
            ret = CmdUtils::FileDescriptorUnpack(unpacker, inFds, fence);
        } else {
            HDF_LOGE("%{public}s, BufferHandleUnpack error", __func__);
        }
        HdifdParcelable fdParcel(fence);

        if (ret == HDF_SUCCESS) {
            ret = impl_->SetDisplayClientBuffer(devId, *buffer, fdParcel.GetFd());
        } else {
            HDF_LOGE("%{public}s, FileDescriptorUnpack error", __func__);
        }
#ifdef DISPLAY_COMMUNITY
        FreeBufferHandle(buffer);
#else
        fdParcel.Move();
#endif //DISPLAY_COMMUNITY
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%{public}s, SetDisplayClientBuffer error", __func__);
            errMaps_.emplace(REQUEST_CMD_SET_DISPLAY_CLIENT_BUFFER, ret);
        }
        return;
    }

    void OnSetDisplayClientDamage(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        uint32_t vectSize = 0;
        bool retBool = true;
        DISPLAY_CHK_CONDITION(retBool, true, unpacker->ReadUint32(devId),
            HDF_LOGE("%{public}s, read devId error", __func__));

        DISPLAY_CHK_CONDITION(retBool, true, unpacker->ReadUint32(vectSize),
            HDF_LOGE("%{public}s, read vectSize error", __func__));

        int32_t ret = (retBool ? HDF_SUCCESS : HDF_FAILURE);
        std::vector<IRect> rects(vectSize);
        if (ret == HDF_SUCCESS) {
            for (int32_t i = 0; i < vectSize; i++) {
                DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::RectUnpack(unpacker, rects[i]),
                    HDF_LOGE("%{public}s, read vect error", __func__); break);
            }
        }
        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, impl_->SetDisplayClientDamage(devId, rects),
            HDF_LOGE("%{public}s, SetDisplayClientDamage error", __func__));

        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_DISPLAY_CLIENT_DAMAGE, ret);
        }
        return;
    }

    void OnCommit(std::shared_ptr<CommandDataUnpacker> unpacker, std::vector<HdifdInfo>& outFds)
    {
        uint32_t devId = -1;
        int32_t fence = -1;

        bool retBool = true;
        DISPLAY_CHK_CONDITION(retBool, true, unpacker->ReadUint32(devId),
            HDF_LOGE("%{public}s, read devId error", __func__));

        int32_t ret = (retBool ? HDF_SUCCESS : HDF_FAILURE);
        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, impl_->Commit(devId, fence),
            HDF_LOGE("%{public}s, commit error", __func__));

        HdifdParcelable fdParcel(fence);
        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::StartSection(REPLY_CMD_COMMIT, replyPacker_),
            HDF_LOGE("%{public}s, StartSection error", __func__));

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::FileDescriptorPack(fdParcel.GetFd(), replyPacker_, outFds),
            HDF_LOGE("%{public}s, FileDescriptorPack error", __func__));

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::EndSection(replyPacker_),
            HDF_LOGE("%{public}s, EndSection error", __func__));

        replyCommandCnt_++;

#ifndef DISPLAY_COMMUNITY
        fdParcel.Move();
#endif //DISPLAY_COMMUNITY

        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_COMMIT, ret);
        }

        return;
    }

    void OnSetLayerAlpha(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        uint32_t layerId = -1;
        LayerAlpha alpha = {0};
        bool retBool = true;

        int32_t ret = CmdUtils::SetupDeviceUnpack(unpacker, devId, layerId);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        retBool = unpacker->ReadBool(alpha.enGlobalAlpha);
        DISPLAY_CHECK(retBool == false, goto EXIT);

        retBool = unpacker->ReadBool(alpha.enPixelAlpha);
        DISPLAY_CHECK(retBool == false, goto EXIT);

        retBool = unpacker->ReadUint8(alpha.alpha0);
        DISPLAY_CHECK(retBool == false, goto EXIT);

        retBool = unpacker->ReadUint8(alpha.alpha1);
        DISPLAY_CHECK(retBool == false, goto EXIT);

        retBool = unpacker->ReadUint8(alpha.gAlpha);
        DISPLAY_CHECK(retBool == false, goto EXIT);

        ret = impl_->SetLayerAlpha(devId, layerId, alpha);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

EXIT:
        if (ret != HDF_SUCCESS || retBool == false) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_ALPHA, ret);
        }
        return;
    }

    void OnSetLayerRegion(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        uint32_t layerId = -1;
        IRect rect = {0};

        int32_t ret = CmdUtils::SetupDeviceUnpack(unpacker, devId, layerId);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = CmdUtils::RectUnpack(unpacker, rect);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = impl_->SetLayerPosition(devId, layerId, rect);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);
EXIT:
        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_REGION, ret);
        }
        return;
    }

    void OnSetLayerCrop(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        uint32_t layerId = -1;
        IRect rect = {0};

        int32_t ret = CmdUtils::SetupDeviceUnpack(unpacker, devId, layerId);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = CmdUtils::RectUnpack(unpacker, rect);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = impl_->SetLayerCrop(devId, layerId, rect);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);
EXIT:
        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_CROP, ret);
        }
        return;
    }

    void OnSetLayerZorder(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        uint32_t layerId = -1;
        uint32_t zorder = 0;

        int32_t ret = CmdUtils::SetupDeviceUnpack(unpacker, devId, layerId);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = unpacker->ReadUint32(zorder) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = impl_->SetLayerZorder(devId, layerId, zorder);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);
EXIT:
        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_ZORDER, ret);
        }
        return;
    }

    void OnSetLayerPreMulti(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        uint32_t layerId = -1;
        bool preMulti = false;

        int32_t ret = CmdUtils::SetupDeviceUnpack(unpacker, devId, layerId);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = unpacker->ReadBool(preMulti) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = impl_->SetLayerPreMulti(devId, layerId, preMulti);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);
EXIT:
        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_PREMULTI, ret);
        }
        return;
    }

    void OnSetLayerTransformMode(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        uint32_t layerId = -1;
        int32_t type = 0;

        int32_t ret = CmdUtils::SetupDeviceUnpack(unpacker, devId, layerId);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = unpacker->ReadInt32(type) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = impl_->SetTransformMode(devId, layerId, static_cast<TransformType>(type));
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);
EXIT:
        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_TRANSFORM_MODE, ret);
        }
        return;
    }

    void OnSetLayerDirtyRegion(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        uint32_t layerId = -1;
        uint32_t vectSize = 0;
        int32_t ret = HDF_SUCCESS;

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::SetupDeviceUnpack(unpacker, devId, layerId),
            HDF_LOGE("%{public}s, read devId error", __func__));

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, unpacker->ReadUint32(vectSize) ? HDF_SUCCESS : HDF_FAILURE,
            HDF_LOGE("%{public}s, read vectSize error", __func__));

        std::vector<IRect> rects(vectSize);
        if (ret == HDF_SUCCESS) {
            for (int32_t i = 0; i < vectSize; i++) {
                DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::RectUnpack(unpacker, rects[i]),
                    HDF_LOGE("%{public}s, read vect error", __func__));
            }
        }

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, impl_->SetLayerDirtyRegion(devId, layerId, rects),
            HDF_LOGE("%{public}s, SetLayerDirtyRegion error", __func__));

        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_DIRTY_REGION, ret);
        }
        return;
    }

    void OnSetLayerVisibleRegion(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        uint32_t layerId = -1;
        uint32_t vectSize = 0;
        int32_t ret = HDF_SUCCESS;

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::SetupDeviceUnpack(unpacker, devId, layerId),
            HDF_LOGE("%{public}s, read devId error", __func__));

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, unpacker->ReadUint32(vectSize) ? HDF_SUCCESS : HDF_FAILURE,
            HDF_LOGE("%{public}s, read vectSize error", __func__));

        std::vector<IRect> rects(vectSize);
        if (ret == HDF_SUCCESS) {
            for (int32_t i = 0; i < vectSize; i++) {
                DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::RectUnpack(unpacker, rects[i]),
                    HDF_LOGE("%{public}s, read vect error", __func__));
            }
        }

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, impl_->SetLayerVisibleRegion(devId, layerId, rects),
            HDF_LOGE("%{public}s, SetLayerDirtyRegion error", __func__));

        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_VISIBLE_REGION, ret);
        }
        return;
    }

    void OnSetLayerBuffer(std::shared_ptr<CommandDataUnpacker> unpacker, const std::vector<HdifdInfo>& inFds)
    {
        uint32_t devId = -1;
        uint32_t layerId = -1;
        BufferHandle *buffer = nullptr;
        int32_t ret = HDF_SUCCESS;

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::SetupDeviceUnpack(unpacker, devId, layerId),
            HDF_LOGE("%{public}s, read devId error", __func__));

        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::BufferHandleUnpack(unpacker, inFds, buffer),
            HDF_LOGE("%{public}s, read BufferHandleUnpack error", __func__));

        int32_t fence = -1;
        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, CmdUtils::FileDescriptorUnpack(unpacker, inFds, fence),
            HDF_LOGE("%{public}s, FileDescriptorUnpack error", __func__));

        HdifdParcelable fdParcel(fence);
        DISPLAY_CHK_CONDITION(ret, HDF_SUCCESS, impl_->SetLayerBuffer(devId, layerId, *buffer, fdParcel.GetFd()),
            HDF_LOGE("%{public}s, SetLayerBuffer error", __func__));

#ifdef DISPLAY_COMMUNITY
        FreeBufferHandle(buffer);
#else
        fdParcel.Move();
#endif // DISPLAY_COMMUNITY
        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_BUFFER, ret);
        }
        return;
    }

    void OnSetLayerCompositionType(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        uint32_t layerId = -1;
        int32_t type;
        int32_t ret = CmdUtils::SetupDeviceUnpack(unpacker, devId, layerId);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = unpacker->ReadInt32(type) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = impl_->SetLayerCompositionType(devId, layerId, static_cast<CompositionType>(type));
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);
EXIT:
        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_COMPOSITION_TYPE, ret);
        }
        return;
    }

    void OnSetLayerBlendType(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        uint32_t layerId = -1;
        int32_t type;
        int32_t ret = CmdUtils::SetupDeviceUnpack(unpacker, devId, layerId);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = unpacker->ReadInt32(type) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = impl_->SetLayerBlendType(devId, layerId, static_cast<BlendType>(type));
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);
EXIT:
        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_BLEND_TYPE, ret);
        }
        return;
    }

    void OnSetLayerVisible(std::shared_ptr<CommandDataUnpacker> unpacker)
    {
        uint32_t devId = -1;
        uint32_t layerId = -1;
        bool visible = false;
        int32_t ret = CmdUtils::SetupDeviceUnpack(unpacker, devId, layerId);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = unpacker->ReadBool(visible) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = impl_->SetLayerVisible(devId, layerId, visible);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);
EXIT:
        if (ret != HDF_SUCCESS) {
            errMaps_.emplace(REQUEST_CMD_SET_LAYER_PREMULTI, ret);
        }
        return;
    }

    int32_t PeriodDataReset()
    {
        replyCommandCnt_ = 0;
        errMaps_.clear();

        int32_t ret = CmdUtils::StartPack(CONTROL_CMD_REPLY_BEGIN, replyPacker_);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("PackBegin failure, ret=%{public}d", ret);
        }
        return ret;
    }

private:
    std::shared_ptr<VdiImpl> impl_;
    std::shared_ptr<Transfer> request_;
    bool isReplyUpdated_;
    std::shared_ptr<Transfer> reply_;
    /* period data */
    uint32_t replyCommandCnt_;
    std::shared_ptr<CommandDataPacker> replyPacker_;
    std::unordered_map<int32_t, int32_t> errMaps_;
};
using HdiDisplayCmdResponser = DisplayCmdResponser<SharedMemQueue<int32_t>, IDisplayComposerVdi>;
} // namespace V1_0
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_0_DISPLAY_CMD_REQUESTER_H
