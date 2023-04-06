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

#ifndef OHOS_HDI_DISPLAY_V1_0_DISPLAY_CMD_UTILS_H
#define OHOS_HDI_DISPLAY_V1_0_DISPLAY_CMD_UTILS_H

#include "buffer_handle_utils.h"
#include "command_pack/command_data_utils.h"
#include "command_pack/command_data_packer.h"
#include "command_pack/command_data_unpacker.h"
#include "v1_0/display_composer_type.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_0 {
using namespace OHOS::HDI::Display::Composer::V1_0;

class DisplayCmdUtils {
public:
    static constexpr int32_t MAX_INT = 0x7fffffff;
    static constexpr int32_t MIN_INT = 0x80000000;
    static constexpr uint32_t ELEMENT_SIZE = sizeof(int32_t);
    static constexpr uint32_t TRANSFER_WAIT_TIME = 100000000; // ms
    static constexpr uint32_t INIT_ELEMENT_COUNT = 32 * 1024;

    #define SWITCHCASE(x) case (x): {return #x;}
    static const char *CommandToString(int32_t cmdId)
    {
        switch (cmdId) {
            /* request cmd */
            SWITCHCASE(REQUEST_CMD_PREPARE_DISPLAY_LAYERS);
            SWITCHCASE(REQUEST_CMD_SET_DISPLAY_CLIENT_BUFFER);
            SWITCHCASE(REQUEST_CMD_SET_DISPLAY_CLIENT_DAMAGE);
            SWITCHCASE(REQUEST_CMD_COMMIT);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_ALPHA);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_REGION);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_CROP);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_ZORDER);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_PREMULTI);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_TRANSFORM_MODE);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_DIRTY_REGION);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_VISIBLE_REGION);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_BUFFER);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_COMPOSITION_TYPE);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_BLEND_TYPE);
            /* reply cmd */
            SWITCHCASE(REPLY_CMD_SET_ERROR);
            SWITCHCASE(REPLY_CMD_PREPARE_DISPLAY_LAYERS);
            SWITCHCASE(REPLY_CMD_COMMIT);
            /* pack control cmd */
            SWITCHCASE(CONTROL_CMD_REQUEST_BEGIN);
            SWITCHCASE(CONTROL_CMD_REPLY_BEGIN);
            SWITCHCASE(CONTROL_CMD_REQUEST_END);
            SWITCHCASE(CONTROL_CMD_REPLY_END);
            default:
                return "unknow command id.";
        }
    }

    static int32_t StartPack(int32_t cmdId, std::shared_ptr<CommandDataPacker> packer)
    {
        return packer->PackBegin(cmdId) ? HDF_SUCCESS : HDF_FAILURE;
    }

    static int32_t EndPack(std::shared_ptr<CommandDataPacker> packer)
    {
        return packer->PackEnd(CONTROL_CMD_REQUEST_END) ? HDF_SUCCESS : HDF_FAILURE;
    }

    static int32_t StartSection(int32_t cmdId, std::shared_ptr<CommandDataPacker> packer)
    {
        return packer->BeginSection(cmdId) ? HDF_SUCCESS : HDF_FAILURE;
    }

    static int32_t SetupDevice(uint32_t devId, uint32_t layerId, std::shared_ptr<CommandDataPacker> packer)
    {
        DISPLAY_CHK_RETURN(packer->WriteUint32(devId) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write devId error", __func__));
        DISPLAY_CHK_RETURN(packer->WriteUint32(layerId) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write layerId error", __func__));
        return HDF_SUCCESS;
    }

    static int32_t EndSection(std::shared_ptr<CommandDataPacker> packer)
    {
        return packer->EndSection() ? HDF_SUCCESS : HDF_FAILURE;
    }

    static int32_t GenerateHdifdSeqid()
    {
        static int32_t HdifdSeqidCursor = 0;

        if (HdifdSeqidCursor <= MAX_INT) {
            ++HdifdSeqidCursor;
            return HdifdSeqidCursor;
        } else {
            return 0;
        }
    }

    static bool MatchHdiFd(int32_t id, std::vector<HdifdInfo> hdiFds, int32_t& fd)
    {
        for (uint32_t i = 0; i < hdiFds.size(); ++i) {
            if (hdiFds[i].id == id) {
                fd = hdiFds[i].hdiFd->Move();
                return true;
            }
        }
        return false;
    }

    static int32_t RectPack(const IRect& rect, std::shared_ptr<CommandDataPacker> packer)
    {
        DISPLAY_CHK_RETURN(packer->WriteInt32(rect.x) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write rect.x error", __func__));
        DISPLAY_CHK_RETURN(packer->WriteInt32(rect.y) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write rect.y error", __func__));
        DISPLAY_CHK_RETURN(packer->WriteInt32(rect.w) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write rect.w error", __func__));
        DISPLAY_CHK_RETURN(packer->WriteInt32(rect.h) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write rect.h error", __func__));
        return HDF_SUCCESS;
    }

    static int32_t FileDescriptorPack(
        const int32_t fd, std::shared_ptr<CommandDataPacker> packer, std::vector<HdifdInfo>& hdiFds)
    {
        HdifdInfo hdifdInfo;
        hdifdInfo.id = GenerateHdifdSeqid();
        hdifdInfo.hdiFd = new HdifdParcelable();
        DISPLAY_CHK_RETURN(hdifdInfo.hdiFd == nullptr, HDF_FAILURE,
            HDF_LOGE("%{public}s, new HdifdParcelable failed", __func__));
        if (fd >= 0) {
            // A normal fd is transfered by binder, here just write id for unpacking to match fd.
            DISPLAY_CHK_RETURN(hdifdInfo.hdiFd->Init(fd) == false, HDF_FAILURE,
                HDF_LOGE("%{public}s, hdiFd init failed", __func__));
            hdiFds.push_back(hdifdInfo);
            DISPLAY_CHK_RETURN(packer->WriteInt32(hdifdInfo.id) == false, HDF_FAILURE,
                HDF_LOGE("%{public}s, hdiFd init failed", __func__));
        } else {
            // A illegal fd is transfered by smq directly.
            DISPLAY_CHK_RETURN(packer->WriteInt32(fd) == false, HDF_FAILURE,
                HDF_LOGE("%{public}s, write fd error", __func__));
        }
        return HDF_SUCCESS;
    }

    static int32_t BufferHandlePack(const BufferHandle& buffer, std::shared_ptr<CommandDataPacker> packer,
        std::vector<HdifdInfo>& hdiFds)
    {
        DISPLAY_CHK_RETURN(packer->WriteUint32(buffer.reserveFds) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write buffer.reserveFds error", __func__));
        DISPLAY_CHK_RETURN(packer->WriteUint32(buffer.reserveInts) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write buffer.reserveInts error", __func__));
        int32_t ret = FileDescriptorPack(buffer.fd, packer, hdiFds);
        if (ret != HDF_SUCCESS) {
            return ret;
        }
        DISPLAY_CHK_RETURN(packer->WriteInt32(buffer.width) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write buffer.width failed", __func__));
        DISPLAY_CHK_RETURN(packer->WriteInt32(buffer.stride) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write buffer.stride failed", __func__));
        DISPLAY_CHK_RETURN(packer->WriteInt32(buffer.height) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write buffer.height failed", __func__));
        DISPLAY_CHK_RETURN(packer->WriteInt32(buffer.size) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write buffer.size failed", __func__));
        DISPLAY_CHK_RETURN(packer->WriteInt32(buffer.format) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write buffer.format failed", __func__));
        DISPLAY_CHK_RETURN(packer->WriteUint64(buffer.usage) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write buffer.usage failed", __func__));
        DISPLAY_CHK_RETURN(packer->WriteUint64(buffer.phyAddr) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write buffer.phyAddr failed", __func__));
        DISPLAY_CHK_RETURN(packer->WriteInt32(buffer.key) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write buffer.key failed", __func__));
        uint32_t i = 0;
        for (i = 0; i < buffer.reserveFds; i++) {
            ret = FileDescriptorPack(buffer.reserve[i], packer, hdiFds);
            if (ret != HDF_SUCCESS) {
                return ret;
            }
        }
        for (uint32_t j = 0; j < buffer.reserveInts; j++) {
            DISPLAY_CHK_RETURN(packer->WriteInt32(buffer.reserve[i++]) == false, HDF_FAILURE,
                HDF_LOGE("%{public}s, write buffer.reserve failed", __func__));
        }
        return HDF_SUCCESS;
    }

    static int32_t SetupDeviceUnpack(std::shared_ptr<CommandDataUnpacker> unpacker, uint32_t& devId, uint32_t& layerId)
    {
        DISPLAY_CHK_RETURN(unpacker->ReadUint32(devId) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, read devId failed", __func__));
        DISPLAY_CHK_RETURN(unpacker->ReadUint32(layerId) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, read layerId failed", __func__));
        return HDF_SUCCESS;
    }

    static int32_t RectUnpack(std::shared_ptr<CommandDataUnpacker> unpacker, IRect& rect)
    {
        DISPLAY_CHK_RETURN(unpacker->ReadInt32(rect.x) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, read rect.x failed", __func__));
        DISPLAY_CHK_RETURN(unpacker->ReadInt32(rect.y) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, read rect.y failed", __func__));
        DISPLAY_CHK_RETURN(unpacker->ReadInt32(rect.w) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, read rect.w failed", __func__));
        DISPLAY_CHK_RETURN(unpacker->ReadInt32(rect.h) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, read rect.h failed", __func__));
        return HDF_SUCCESS;
    }

    static int32_t FileDescriptorUnpack(
        std::shared_ptr<CommandDataUnpacker> unpacker, const std::vector<HdifdInfo>& hdiFds, int32_t& fd)
    {
        int32_t fdId = -1;
        DISPLAY_CHK_RETURN(unpacker->ReadInt32(fdId) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, read fdId failed", __func__));
        if (fdId < 0 || MatchHdiFd(fdId, hdiFds, fd) == false) {
            // If matching failure, the illegal fd is transfered by smq directly, not by binder IPC.
            fd = fdId;
        }
        return HDF_SUCCESS;
    }

    static int32_t BufferHandleUnpack(std::shared_ptr<CommandDataUnpacker> unpacker,
        const std::vector<HdifdInfo>& hdiFds, BufferHandle *&buffer)
    {
        uint32_t fdsNum = 0;
        uint32_t intsNum = 0;
        DISPLAY_CHK_RETURN(unpacker->ReadUint32(fdsNum) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, read fdsNum error", __func__));
        DISPLAY_CHK_RETURN(unpacker->ReadUint32(intsNum) == false, HDF_FAILURE,
            HDF_LOGE("%{public}s, write intsNum error", __func__));
        BufferHandle *handle = AllocateBufferHandle(fdsNum, intsNum);
        if (handle == nullptr) {
            return HDF_FAILURE;
        }
        handle->reserveFds = fdsNum;
        handle->reserveInts = intsNum;
        int32_t ret = FileDescriptorUnpack(unpacker, hdiFds, handle->fd);
        bool retVal = (ret == HDF_SUCCESS ? true : false);
        DISPLAY_CHK_CONDITION(retVal, true, unpacker->ReadInt32(handle->width),
            HDF_LOGE("%{public}s, read handle->width error", __func__));
        DISPLAY_CHK_CONDITION(retVal, true, unpacker->ReadInt32(handle->stride),
            HDF_LOGE("%{public}s, read handle->stride error", __func__));
        DISPLAY_CHK_CONDITION(retVal, true, unpacker->ReadInt32(handle->height),
            HDF_LOGE("%{public}s, read handle->height error", __func__));
        DISPLAY_CHK_CONDITION(retVal, true, unpacker->ReadInt32(handle->size),
            HDF_LOGE("%{public}s, read handle->size error", __func__));
        DISPLAY_CHK_CONDITION(retVal, true, unpacker->ReadInt32(handle->format),
            HDF_LOGE("%{public}s, read handle->format error", __func__));
        DISPLAY_CHK_CONDITION(retVal, true, unpacker->ReadUint64(handle->usage),
            HDF_LOGE("%{public}s, read handle->usage error", __func__));
        DISPLAY_CHK_CONDITION(retVal, true, unpacker->ReadUint64(handle->phyAddr),
            HDF_LOGE("%{public}s, read handle->phyAddr error", __func__));
        DISPLAY_CHK_CONDITION(retVal, true, unpacker->ReadInt32(handle->key),
            HDF_LOGE("%{public}s, read handle->key error", __func__));
        if (retVal) {
            uint32_t i = 0;
            for (i = 0; i < handle->reserveFds; i++) {
                ret = FileDescriptorUnpack(unpacker, hdiFds, handle->reserve[i]);
                if (ret != HDF_SUCCESS) {
                    retVal = false;
                    break;
                }
            }
            for (uint32_t j = 0; j < handle->reserveInts; j++) {
                DISPLAY_CHK_CONDITION(retVal, true, unpacker->ReadInt32(handle->reserve[i++]),
                    HDF_LOGE("%{public}s, read handle->reserve error", __func__));
                if (!retVal) {
                    HDF_LOGE("%{public}s, get reserve date error, i:%{public}u, j:%{public}u",
                    __func__, i, j);
                    break;
                }
            }
        }
        if (!retVal) {
            if (handle != nullptr) {
                FreeBufferHandle(handle);
                handle = nullptr;
            }
            HDF_LOGE("%{public}s: buffer handle unpack failed", __func__);
        }
        buffer = handle;
        return retVal ? HDF_SUCCESS : HDF_FAILURE;
    }
};
using CmdUtils = DisplayCmdUtils;
} // namespace V1_0
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_0_DISPLAY_CMD_UTILS_H