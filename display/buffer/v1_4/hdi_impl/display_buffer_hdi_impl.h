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

#ifndef OHOS_HDI_DISPLAY_V1_4_DISPLAY_BUFFER_HDI_IMPL_H
#define OHOS_HDI_DISPLAY_V1_4_DISPLAY_BUFFER_HDI_IMPL_H

#include <iproxy_broker.h>
#include <unistd.h>
#include <mutex>
#include "hdf_log.h"
#include "hilog/log.h"
#include "v1_4/iallocator.h"
#include "v1_4/include/idisplay_buffer.h"
#include "v1_3/hdi_impl/display_buffer_hdi_impl.h"

#undef LOG_TAG
#define LOG_TAG "DISP_HDI_BUFF"
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002515

namespace OHOS {
namespace HDI {
namespace Display {
namespace Buffer {
namespace V1_4 {
template<typename Interface>
class DisplayBufferHdiImpl : public V1_3::DisplayBufferHdiImpl<Interface> {
public:
    explicit DisplayBufferHdiImpl(sptr<IAllocator> allocator, sptr<V1_3::IMapper> mapper,
        sptr<V1_1::IMetadata> metadata)
        : BaseType4_0(allocator, mapper, metadata), allocator_v1_4_(allocator)
    {}

    virtual ~DisplayBufferHdiImpl() {}

    void CheckAllocator() const
    {
        if (allocator_v1_4_ != nullptr) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        if (allocator_v1_4_ == nullptr) {
            allocator_v1_4_ = IAllocator::Get(false);
        }
    }

    virtual int32_t CloneDmaBufferHandle(const BufferHandle& inHandle, BufferHandle*& outHandle)const override
    {
        DISPLAY_TRACE;
        CheckAllocator();
        CHECK_NULLPOINTER_RETURN_VALUE(allocator_v1_4_, HDF_FAILURE);

        sptr<NativeBuffer> hdiInBuffer = new NativeBuffer();
        CHECK_NULLPOINTER_RETURN_VALUE(hdiInBuffer, HDF_FAILURE);
        sptr<NativeBuffer> hdiOutBuffer;

        hdiInBuffer->SetBufferHandle(const_cast<BufferHandle*>(&inHandle));
        int32_t ret = allocator_v1_4_->CloneDmaBufferHandle(hdiInBuffer, hdiOutBuffer);
        if ((ret == HDF_SUCCESS) && (hdiOutBuffer != nullptr)) {
            outHandle = hdiOutBuffer->Move();
        }
        if (ret != HDF_SUCCESS && ret != HDF_ERR_NOT_SUPPORT) {
            HDF_LOGE("%{public}s:CloneDmaBufferHandle failed, ret : %{public}d", __func__, ret);
        }
        return ret;
    }

    using V1_3::DisplayBufferHdiImpl<Interface>::WAIT_TIME_INTERVAL;

private:
    using BaseType4_0 = V1_3::DisplayBufferHdiImpl<Interface>;
protected:
    mutable sptr<IAllocator> allocator_v1_4_;
    mutable std::mutex mutex_;
};
using HdiDisplayBufferImpl = DisplayBufferHdiImpl<V1_4::IDisplayBuffer>;
} // namespace V1_4
} // namespace Buffer
} // namespace Display
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_DISPLAY_V1_4_DISPLAY_BUFFER_HDI_IMPL_H
