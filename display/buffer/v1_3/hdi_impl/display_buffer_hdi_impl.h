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

#ifndef OHOS_HDI_DISPLAY_V1_3_DISPLAY_BUFFER_HDI_IMPL_H
#define OHOS_HDI_DISPLAY_V1_3_DISPLAY_BUFFER_HDI_IMPL_H

#include <iproxy_broker.h>
#include <unistd.h>
#include "hdf_log.h"
#include "hilog/log.h"
#include "v1_0/imapper.h"
#include "v1_2/imapper.h"
#include "v1_3/imapper.h"
#include "v1_1/include/idisplay_buffer.h"
#include "v1_2/include/idisplay_buffer.h"
#include "v1_3/include/idisplay_buffer.h"
#include "v1_0/hdi_impl/display_buffer_hdi_impl.h"
#include "v1_1/hdi_impl/display_buffer_hdi_impl.h"
#include "v1_2/hdi_impl/display_buffer_hdi_impl.h"

#undef LOG_TAG
#define LOG_TAG "DISP_HDI_BUFF"
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002515

namespace OHOS {
namespace HDI {
namespace Display {
namespace Buffer {
namespace V1_3 {
template<typename Interface>
class DisplayBufferHdiImpl : public V1_2::DisplayBufferHdiImpl<Interface> {
public:
    explicit DisplayBufferHdiImpl(bool isAllocLocal = false) : BaseType3_0(isAllocLocal), mapper_v1_3_(nullptr)
    {
        while ((mapper_v1_3_ = IMapper::Get(true)) == nullptr) {
            // Waiting for metadata service ready
            usleep(WAIT_TIME_INTERVAL);
        }
    }

    virtual ~DisplayBufferHdiImpl() {};

    int32_t AllocMemPassThrough(const AllocInfo& info, BufferHandle*& handle) const
    {
        DISPLAY_TRACE;
        CHECK_NULLPOINTER_RETURN_VALUE(mapper_v1_3_, HDF_FAILURE);
        sptr<NativeBuffer> hdiBuffer;
        int32_t ret = mapper_v1_3_->AllocMem(info, hdiBuffer);
        if ((ret == HDF_SUCCESS) && (hdiBuffer != nullptr)) {
            handle = hdiBuffer->Move();
        } else {
            handle = nullptr;
            if (ret == HDF_SUCCESS) {
                ret = HDF_FAILURE;
            }
            HDF_LOGE("%{public}s: AllocMem error", __func__);
        }
        return ret;
    }

    int32_t AllocMemIpc(const AllocInfo& info, BufferHandle*& handle) const
    {
        DISPLAY_TRACE;
        CHECK_NULLPOINTER_RETURN_VALUE(allocator_, HDF_FAILURE);
        sptr<NativeBuffer> hdiBuffer;
        int32_t ret = allocator_->AllocMem(info, hdiBuffer);
        if ((ret == HDF_SUCCESS) && (hdiBuffer != nullptr)) {
            handle = hdiBuffer->Move();
        } else {
            handle = nullptr;
            if (ret == HDF_SUCCESS) {
                ret = HDF_FAILURE;
            }
            HDF_LOGE("%{public}s: AllocMem error", __func__);
        }
        return ret;
    }

    int32_t AllocMem(const AllocInfo& info, BufferHandle*& handle) const override
    {
        DISPLAY_TRACE;
        CHECK_NULLPOINTER_RETURN_VALUE(mapper_v1_3_, HDF_FAILURE);

        //Step1. check is support alloc passthrough
        if (mapper_v1_3_->IsSupportAllocPassthrough(info) == HDF_SUCCESS) {
            int32_t ret = AllocMemPassThrough(info, handle);
            if (ret != HDF_SUCCESS) {
                HDF_LOGW("%{public}s: AllocMem Passthrough mode failed, use allocator_host", __func__);
                return AllocMemIpc(info, handle);
            }
            return ret;
        } else {
            return AllocMemIpc(info, handle);
        }
    }

private:
    using BaseType3_0 = V1_2::DisplayBufferHdiImpl<Interface>;
protected:
    using BaseType3_0::WAIT_TIME_INTERVAL;
    using BaseType3_0::allocator_;
    sptr<IMapper> mapper_v1_3_;
};
using HdiDisplayBufferImpl = DisplayBufferHdiImpl<V1_3::IDisplayBuffer>;
} // namespace V1_3
} // namespace Buffer
} // namespace Display
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_DISPLAY_V1_3_DISPLAY_BUFFER_HDI_IMPL_H
